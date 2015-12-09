#include <GL\glew.h>

#include "base.h"
#include "def.h"
#include "graphics.h"
#include "shader\shaderbase.h"

CGraphics::CGraphics() {
	m_pSDLWindow = NULL;
	m_GLContext = 0;

	m_pShaderManager = NULL;

	m_testVBO = 0;
	m_testVAO = 0;
}
CGraphics::~CGraphics() {
}

bool CGraphics::initialize()
{
	GLenum glewError;

	// Request the proper version
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, GL_VERSION_MAJOR );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, GL_VERSION_MINOR );

	// GL Attributes
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

	// Create the window
	PrintInfo( L"Creating game window...\n" );
	m_pSDLWindow = SDL_CreateWindow( WINDOW_TITLE_SHORT, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_DEFRES_X, WINDOW_DEFRES_Y, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if( !m_pSDLWindow ) {
		CGame::instance().displayMessagebox( L"failed to create game window " );
		return false;
	}

	// Create the openGL context
	PrintInfo( L"Creating openGL context (requested version %d.%d)...\n", GL_VERSION_MAJOR, GL_VERSION_MINOR );
	m_GLContext = SDL_GL_CreateContext( m_pSDLWindow );
	if( !m_GLContext ) {
		CGame::instance().displayMessagebox( L"failed to create openGL context" );
		return false;
	}

	// Initialize GLEW
	PrintInfo( L"Initializing GLEW..." );
	glewError = glewInit();
	if( glewError != GLEW_OK ) {
		CGame::instance().displayMessagebox( L"failed to initialize GLEW" );
		return false;
	}

	SDL_GL_SetSwapInterval( 1 );

	// OpenGL attributes
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// Load the shaders
	m_pShaderManager = new CShaderManager();
	if( !m_pShaderManager->initialize() )
		return false;

	// Create the test VAO
	glGenVertexArrays( 1, &m_testVAO );
	glBindVertexArray( m_testVAO );
	// Create the test VBO
	glGenBuffers( 1, &m_testVBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_testVBO );
	// Fill with basic vertex info
	float basicTriangle[] = {
		0.0f, 0.25f, 0.0f,
		0.25f, -0.25f, 0.0f,
		-0.25f, -0.25f, 0.0f
	};
	// Store the vertices
	glBufferData( GL_ARRAY_BUFFER, sizeof( basicTriangle ), &basicTriangle[0], GL_STATIC_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	return true;
}
void CGraphics::destroy()
{
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
	glClear( GL_COLOR_BUFFER_BIT );

	// Use simple shader
	m_pShaderManager->getProgram( SHADERPROGRAM_SIMPLE )->bind();

	// Render . . .
	glBindVertexArray( m_testVAO );
	glDrawArrays( GL_TRIANGLES, 0, 3 );

	SDL_GL_SwapWindow( m_pSDLWindow );
}