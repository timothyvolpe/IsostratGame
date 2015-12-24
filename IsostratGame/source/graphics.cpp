#include <GL\glew.h>
#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include <sstream>

#include "base.h"
#include "def.h"
#include "graphics.h"
#include "config.h"
#include "shader\shaderbase.h"
#include "camera.h"
#include "world\world.h"
#include "ui\interface.h"

CGraphics::CGraphics() {
	m_pSDLWindow = NULL;
	m_GLContext = 0;

	m_pShaderManager = NULL;
	m_pWorld = NULL;

	m_pCamera = NULL;

	m_projectionMatrix = glm::mat4( 1.0f );
	m_orthoMatrix = glm::mat4( 1.0f );
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	GLenum glewError;
	int windowX, windowY, resX, resY;
	float fov;

	// Request the proper version
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR );

	// GL Attributes
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

	// Create the window
	PrintInfo( L"Creating game window...\n" );
	windowX = CGame::instance().getConfigLoader()->getWindowX();
	windowY = CGame::instance().getConfigLoader()->getWindowY();
	m_pSDLWindow = SDL_CreateWindow( WINDOW_TITLE_SHORT, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowX, windowY, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if( !m_pSDLWindow ) {
		CGame::instance().displayMessagebox( L"failed to create game window " );
		return false;
	}
	resX = CGame::instance().getConfigLoader()->getResolutionX();
	resY = CGame::instance().getConfigLoader()->getResolutionY();
	fov = CGame::instance().getConfigLoader()->getFieldOfView();
	this->calculateProjection( resX, resY, fov, 100.0f );

	// Create the openGL context
	PrintInfo( L"Creating openGL context (requested version %d.%d)...\n", GL_VERSION_MAJOR, GL_VERSION_MINOR );
	m_GLContext = SDL_GL_CreateContext( m_pSDLWindow );
	if( !m_GLContext ) {
		CGame::instance().displayMessagebox( L"failed to create openGL context" );
		return false;
	}

	// Initialize GLEW
	PrintInfo( L"Initializing GLEW...\n" );
	glewError = glewInit();
	if( glewError != GLEW_OK ) {
		CGame::instance().displayMessagebox( L"failed to initialize GLEW" );
		return false;
	}

	// Print some GL information
	PrintInfo( L"Graphics Information:\n > GL Version: %hs\n > GLSL Version: %hs\n > Hardware Vendor: %hs\n > Renderer: %hs\n", glGetString( GL_VERSION ), glGetString( GL_SHADING_LANGUAGE_VERSION ), glGetString( GL_VENDOR ), glGetString( GL_RENDERER ) );

	SDL_GL_SetSwapInterval( 1 );

	// OpenGL attributes
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );

	// Load the shaders
	m_pShaderManager = new CShaderManager();
	if( !m_pShaderManager->initialize() )
		return false;

	// Create the camera
	m_pCamera = new CCamera();
	if( !m_pCamera->initialize() )
		return false;

	// Create the world
	m_pWorld = new CWorld();
	if( !m_pWorld->initialize() )
		return false;
	// Load the world
	if( !m_pWorld->loadWorld() )
		return false;
	if( !m_pWorld->loadSave( L"testSave" ) )
		return false;

	return true;
}
void CGraphics::destroy()
{
	// Destroy the world
	if (m_pWorld )
		m_pWorld->destroyWorld();
	DESTROY_DELETE( m_pWorld );
	// Destory camera
	DESTROY_DELETE( m_pCamera );
	// Destroy the shaders
	DESTROY_DELETE( m_pShaderManager );
	// Destroy the graphics
	SDL_GL_DeleteContext( m_GLContext );
	m_GLContext = 0;
	if( m_pSDLWindow ) {
		SDL_DestroyWindow( m_pSDLWindow );
		m_pSDLWindow = NULL;
	}
}

void CGraphics::draw()
{
	glm::mat4 viewMatrix, ortherViewMatrix, modelMatrix;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Update the camera
	viewMatrix = m_pCamera->update();
	ortherViewMatrix = glm::lookAt( glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

	// Draw the world
	m_pWorld->draw( m_projectionMatrix, viewMatrix );
	// Draw the interface
	CGame::instance().getInterfaceManager()->draw( m_orthoMatrix, ortherViewMatrix );

	SDL_GL_SwapWindow( m_pSDLWindow );

	// TEST: add FPS to window title
	std::stringstream titleStream;
	titleStream << WINDOW_TITLE_SHORT << " Frametime: " << CGame::instance().getFrameTime();
	SDL_SetWindowTitle( m_pSDLWindow, titleStream.str().c_str() );
}

void CGraphics::calculateProjection( int width, int height, float fov, float zFar ) {
	m_projectionMatrix = glm::perspective( 45.0f, (float)width / (float)height, 0.01f, 100.0f );
	m_orthoMatrix = glm::ortho( 0.0f, (float)width, 0.0f, (float)height, 0.1f, LAYER_SIZE*255 );
}

glm::mat4 CGraphics::getProjectionMatrix() {
	return m_projectionMatrix;
}

CShaderManager* CGraphics::getShaderManager() {
	return m_pShaderManager;
}
CWorld* CGraphics::getWorld() {
	return m_pWorld;
}