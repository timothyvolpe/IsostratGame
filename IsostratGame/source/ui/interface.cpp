#include "base.h"
#include "ui\interface.h"
#include "ui\localization.h"
#include "ui\font.h"
#include "ui\interface_screen.h"

#include "graphics.h"
#include "shader\shaderbase.h"

///////////////////////
// CInterfaceManager //
///////////////////////

CInterfaceManager::CInterfaceManager() {
	m_pFontManager = NULL;
	m_pLocalization = NULL;

	m_pTestScreen = NULL;

	m_width = 800;
	m_height = 600;
	m_bUpdateDim = true;

	m_interfaceVAO = 0;
	m_interfaceVBO = 0;
}
CInterfaceManager::~CInterfaceManager() {
}

bool CInterfaceManager::initialize()
{
	// Create the font manager
	m_pFontManager = new CFontManager();
	if( !m_pFontManager->initialize() )
		return false;
	// Create the localization
	m_pLocalization = new CLocalization();
	if( !m_pLocalization->initialize() )
		return false;

	this->setLanguage( GAMELANGUAGE_ENGLISH );

	// Create the test screen
	m_pTestScreen = new CInterfaceScreen();
	if( !m_pTestScreen->onCreate() ) {
		return false;
	}

	// Create the interface VAO
	glGenVertexArrays( 1, &m_interfaceVAO );
	glBindVertexArray( m_interfaceVAO );
	glGenBuffers( 1, &m_interfaceVBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_interfaceVBO );
	// Its empty for now
	InterfaceVertex testVertices[4];
	testVertices[0].relpos = glm::vec2( 0.0f, 1.0f );
	testVertices[1].relpos = glm::vec2( 0.0f, 0.8f );
	testVertices[2].relpos = glm::vec2( 0.2f, 1.0f );
	testVertices[3].relpos = glm::vec2( 0.2f, 0.8f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( testVertices ), &testVertices[0], GL_DYNAMIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( InterfaceVertex ), (GLvoid*)offsetof( InterfaceVertex, relpos ) );
	glEnableVertexAttribArray( 0 );
	
	return true;
}
void CInterfaceManager::destroy()
{
	if( m_interfaceVAO ) {
		glDeleteVertexArrays( 1, &m_interfaceVAO );
		m_interfaceVAO = 0;
	}
	if( m_interfaceVBO ) {
		glDeleteBuffers( 1, &m_interfaceVBO );
		m_interfaceVBO = 0;
	}
	DESTROY_DELETE( m_pLocalization );
	DESTROY_DELETE( m_pFontManager );
}

bool CInterfaceManager::setLanguage( unsigned char language )
{
	// Load english as the language
	if( !m_pLocalization->loadLanguage( GAMELANGUAGE_ENGLISH ) )
		return false;
	// Load the fonts
	if( !m_pFontManager->loadFonts( m_pLocalization->getFontNameList(), m_pLocalization->getCacheChars() ) )
		return false;

	return true;
}

void CInterfaceManager::draw( glm::mat4 projection, glm::mat4 view )
{
	CShaderManager *pShaderManager = CGame::instance().getGraphics()->getShaderManager();

	// Bind the interface shader
	pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->bind();

	// Update dimensions
	if( m_bUpdateDim ) {
		int winDim[] = { m_width, m_height };
		glUniform2iv( pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->getUniform( "windowDimensions" ), 1, &winDim[0] );
	}

	// Draw the interfaces
	pShaderManager->m_ubGlobalMatrices.mvp_ortho = projection * view;
	pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );

	glBindVertexArray( m_interfaceVAO );
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

void CInterfaceManager::setDimensions( int width, int height ) {
	m_width = width;
	m_height = height;
	m_bUpdateDim = true;
}
CFontManager* CInterfaceManager::getFontManager() {
	return m_pFontManager;
}
CLocalization* CInterfaceManager::getLocalization() {
	return m_pLocalization;
}

////////////////////
// CInterfaceBase //
////////////////////

CInterfaceBase::CInterfaceBase() {
}
CInterfaceBase::~CInterfaceBase() {
}

bool CInterfaceBase::initialize()
{
	return true;
}
void CInterfaceBase::destroy()
{
}

//////////////////////////
// CInterfaceRenderable //
//////////////////////////

CInterfaceRenderable::CInterfaceRenderable() {
	m_vertexStart = 0;
}
CInterfaceRenderable::~CInterfaceRenderable() {
}