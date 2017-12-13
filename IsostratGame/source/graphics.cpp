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
#include "debugrender.h"
#include "world\world.h"
#include "ui\interface.h"
#include "input.h"

CGraphics::CGraphics() {
	m_pSDLWindow = NULL;
	m_GLContext = 0;

	m_pShaderManager = NULL;
	m_pWorld = NULL;
	m_pDebugRender = NULL;

	m_pCamera = NULL;

	m_projectionMatrix = glm::mat4( 1.0f );
	m_orthoMatrix = glm::mat4( 1.0f );
	m_currentViewMat = glm::mat4( 1.0f );
	m_currentOrthoViewMat = glm::mat4( 1.0f );

	m_farZ = 0.0f;

	m_wireframeMode = WIREFRAME_MODE_UNSET;
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

	SDL_GL_SetSwapInterval( 0 ); // set to 1 for vsync

	// OpenGL attributes
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	this->setWireframeMode( WIREFRAME_MODE_SOLID );

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
	
	// Create the debug renderer
	m_pDebugRender = new CDebugRender();
	if( !m_pDebugRender->initialize() )
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
	// Destroy debug renderer
	DESTROY_DELETE( m_pDebugRender );
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

void CGraphics::update()
{
	if( CGame::instance().getInput()->isKeyPress( SDL_SCANCODE_F11 ) )
	{
		int newMode;
		newMode = m_wireframeMode + 1;
		if( newMode >= WIREFRAME_MODE_COUNT )
			newMode = 0;
		this->setWireframeMode( newMode );
	}

	// Move the camera
	m_currentViewMat = m_pCamera->update();
	m_currentOrthoViewMat = glm::lookAt( glm::vec3( 0.0f, 0.0f, 1.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
	// Update frustrum test
	if( CGame::instance().getInput()->isKeyPress( CGame::instance().getConfigLoader()->getKeybind( KEYBIND_SET_FRUSTRUM ) ) )
		m_pCamera->getFrustum()->setFrustum( m_pCamera->getEyePosition(), m_projectionMatrix*m_currentViewMat, m_nearZ, m_farZ, m_fov, m_ratio );

	// Update world
	m_pWorld->update();
	// Update debug renderer
	m_pDebugRender->update();
	// Update the interface
	CGame::instance().getInterfaceManager()->update();
}
void CGraphics::draw()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if( m_wireframeMode == WIREFRAME_MODE_WIREUI )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// Draw the world
	m_pWorld->draw( m_projectionMatrix, m_currentViewMat );
	// Draw the debug data
	m_pDebugRender->draw( m_projectionMatrix, m_currentViewMat );
	if( m_wireframeMode == WIREFRAME_MODE_WIREUI )
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	// Draw the interface
	CGame::instance().getInterfaceManager()->draw( m_orthoMatrix, m_currentOrthoViewMat );

	SDL_GL_SwapWindow( m_pSDLWindow );
}

void CGraphics::calculateProjection( int width, int height, float fov, float zFar ) {
	m_nearZ = 0.01f;
	m_farZ = zFar;
	m_fov = fov;
	m_ratio = (float)width / (float)height;
	m_projectionMatrix = glm::perspective( m_fov, m_ratio, m_nearZ, m_farZ );
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
CDebugRender* CGraphics::getDebugRender() {
	return m_pDebugRender;
}
CCamera* CGraphics::getCamera() {
	return m_pCamera;
}
void CGraphics::setWireframeMode( int mode )
{
	if( mode != m_wireframeMode ) {
		m_wireframeMode = mode;
		switch( mode )
		{
		case WIREFRAME_MODE_SOLID:
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			break;
		case WIREFRAME_MODE_WIREUI:
		case WIREFRAME_MODE_WIRE:
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			break;
		default:
			PrintWarn( L"Invalid wireframe mode\n" );
			setWireframeMode( WIREFRAME_MODE_SOLID );
		}
	}
}