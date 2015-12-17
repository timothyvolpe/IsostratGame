#include "base.h"
#include "ui\interface.h"
#include "ui\localization.h"
#include "ui\font.h"
#include "ui\interface_screen.h"

#include "graphics.h"
#include "shader\shaderbase.h"

#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

///////////////////////
// CInterfaceManager //
///////////////////////

CInterfaceManager::CInterfaceManager() {
	m_pFontManager = NULL;
	m_pLocalization = NULL;

	m_width = 800;
	m_height = 600;

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

	// Create the test screens
	// 1
	CInterfaceScreen *pTestScreen0, *pTestScreen1;
	pTestScreen0 = this->createInterfaceObjectRenderable<CInterfaceScreen>();
	if( pTestScreen0 ) {
		pTestScreen0->setRelativePosition( glm::vec2( 0.0f, 0.0f ) );
		pTestScreen0->setRelativeSize( glm::vec2( 0.7f, 0.7f ) );
	}
	// 2
	/*pTestScreen1 = this->createInterfaceObjectRenderable<CInterfaceScreen>();
	if( pTestScreen1 ) {
		pTestScreen1->setRelativePosition( glm::vec2( 0.5f, 0.5f ) );
		pTestScreen1->setRelativeSize( glm::vec2( 0.2f, 0.2f ) );
	}*/

	// Create the interface VAO
	glGenVertexArrays( 1, &m_interfaceVAO );
	glBindVertexArray( m_interfaceVAO );
	glGenBuffers( 1, &m_interfaceVBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_interfaceVBO );
	InterfaceVertex testVertices[4];
	testVertices[0].relpos = glm::vec2( 0.0f, 1.0f );
	testVertices[0].tex = glm::vec2( 0.0f, 0.0f );
	testVertices[1].relpos = glm::vec2( 0.0f, 0.0f );
	testVertices[1].tex = glm::vec2( 0.0f, 1.0f );
	testVertices[2].relpos = glm::vec2( 1.0f, 1.0f );
	testVertices[2].tex = glm::vec2( 1.0f, 0.0f );
	testVertices[3].relpos = glm::vec2( 1.0f, 0.0f );
	testVertices[3].tex = glm::vec2( 1.0f, 1.0f );
	glBufferData( GL_ARRAY_BUFFER, sizeof( testVertices ), &testVertices[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( InterfaceVertex ), (GLvoid*)offsetof( InterfaceVertex, relpos ) );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( InterfaceVertex ), (GLvoid*)offsetof( InterfaceVertex, tex ) );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	
	return true;
}
void CInterfaceManager::destroy()
{
	// Destroy all the interface objects
	for( InterfaceList::iterator it = m_interfaceList.begin(); it != m_interfaceList.end(); it++ ) {
		(*it)->onDestroy();
		DESTROY_DELETE( (*it) );
	}
	m_interfaceList.clear();
	m_interfaceRenderableList.clear();

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
	glm::mat4 modelMatrix;
	glm::vec2 windowDimensions, textureCoords;

	// Bind the interface shader
	pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->bind();

	windowDimensions = glm::vec2( m_width, m_height );

	// Set the sampler to texture0
	glUniform1i( pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->getUniform( "textureSampler" ), 0 );

	// Draw each interface object
	for( InterfaceRenderableList::iterator it = m_interfaceRenderableList.begin(); it != m_interfaceRenderableList.end(); it++ )
	{
		// Update matrices
		modelMatrix = glm::translate( glm::mat4( 1.0f ), glm::vec3( (*it)->getRelativePosition()*windowDimensions, (*it)->getLayer()*LAYER_SIZE ) );
		modelMatrix = glm::scale( modelMatrix, glm::vec3( (*it)->getRelativeSize()*windowDimensions, 1.0f ) );
		pShaderManager->m_ubGlobalMatrices.mvp_ortho = projection * view * modelMatrix;
		pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );
		// Update uniforms
		textureCoords = glm::vec2( 0.0f, 0.0f );
		glUniform2fv( pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->getUniform( "textureCoords" ), 1, &textureCoords[0] );
		// Bind the texture
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, m_pFontManager->getFont( L"DEFAULTFONT" )->getTextureId() );
		// Draw
		glBindVertexArray( m_interfaceVAO );
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}
}

void CInterfaceManager::setDimensions( int width, int height ) {
	m_width = width;
	m_height = height;
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
	m_positionRel = glm::vec2( 0.0f, 0.0f );
	m_sizeRel = glm::vec2( 0.0f, 0.0f );
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

void CInterfaceBase::setRelativePosition( glm::vec2 posRel ) {
	m_positionRel = posRel;
	this->onPositionChange();
}
glm::vec2 CInterfaceBase::getRelativePosition() {
	return m_positionRel;
}
void CInterfaceBase::setRelativeSize( glm::vec2 sizeRel ) {
	m_sizeRel = sizeRel;
	this->onResize();
}
glm::vec2 CInterfaceBase::getRelativeSize() {
	return m_sizeRel;
}

//////////////////////////
// CInterfaceRenderable //
//////////////////////////

CInterfaceRenderable::CInterfaceRenderable() {
	m_layer = 0;
}
CInterfaceRenderable::~CInterfaceRenderable() {
}
void CInterfaceRenderable::setLayer( unsigned char layer ) {
	m_layer = layer;
	this->onLayerChange();
}
unsigned char CInterfaceRenderable::getLayer() {
	return m_layer;
}