#include "base.h"
#include "ui\interface.h"
#include "ui\localization.h"
#include "ui\font.h"
#include "ui\interface_screen.h"
#include "ui\interface_label.h"

#include "graphics.h"
#include "shader\shaderbase.h"

#include <functional>
#include <algorithm>
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
	m_quadCount = 0;
	m_oldQuadCount = 0;
	m_bQuadsInvalid = false;
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

	// Create the interface VAO
	glGenVertexArrays( 1, &m_interfaceVAO );
	glBindVertexArray( m_interfaceVAO );
	glGenBuffers( 1, &m_interfaceVBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_interfaceVBO );
	glBufferData( GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( InterfaceVertex ), (GLvoid*)offsetof( InterfaceVertex, relpos ) );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( InterfaceVertex ), (GLvoid*)offsetof( InterfaceVertex, tex ) );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );

	// Load the ui controls
	if( !this->loadScreens() )
		return false;

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
	m_uiScreens.clear();

	if( m_interfaceVAO ) {
		glDeleteVertexArrays( 1, &m_interfaceVAO );
		m_interfaceVAO = 0;
	}
	if( m_interfaceVBO ) {
		glDeleteBuffers( 1, &m_interfaceVBO );
		m_interfaceVBO = 0;
	}
	m_oldQuadCount = 0;
	m_quadCount = 0;
	m_quadData.clear();
	DESTROY_DELETE( m_pLocalization );
	DESTROY_DELETE( m_pFontManager );
}

bool CInterfaceManager::loadScreens()
{
	CInterfaceScreen *pHud;
	CInterfaceLabel *pLabel0, *pLabel1;

	// Create a test HUD
	pHud = this->createInterfaceObject<CInterfaceScreen>();
	if( pHud ) {
		if( !pHud->onActivate() )
			return false;
	}
	pLabel0 = this->createInterfaceObjectRenderable<CInterfaceLabel>();
	if( pLabel0 ) {
		pLabel0->setRelativePosition( glm::vec2( 0.0f, 0.0f ) );
		pLabel0->setRelativeSize( glm::vec2( 0.4f, 0.4f ) );
		pHud->addToContainer( pLabel0 );
		if( !pLabel0->onActivate() )
			return false;
	}
	// 2
	/*pLabel1 = this->createInterfaceObjectRenderable<CInterfaceLabel>();
	if( pLabel1 ) {
		pLabel1->setRelativePosition( glm::vec2( 0.5f, 0.5f ) );
		pLabel1->setRelativeSize( glm::vec2( 0.2f, 0.2f ) );
		pHud->addToContainer( pLabel1 );
		if( !pLabel1->onActivate() )
			return false;
	}*/
	m_uiScreens.push_back( pHud );

	return true;
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
	glm::vec2 windowDimensions;

	// Reconstruct quads if needed
	if( m_bQuadsInvalid )
		this->reconstructQuadData();

	// Bind the interface shader
	pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->bind();

	windowDimensions = glm::vec2( m_width, m_height );

	// Send the matrix info
	pShaderManager->m_ubGlobalMatrices.mvp_ortho = projection * view;
	pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );
	// Update the dimensions
	glUniform2fv( pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->getUniform( "resolution" ), 1, &windowDimensions[0] );

	// Set the sampler to texture0
	glUniform1i( pShaderManager->getProgram( SHADERPROGRAM_INTERFACE )->getUniform( "textureSampler" ), 0 );

	// Draw all the renderable components
	glBindVertexArray( m_interfaceVAO );
	for( auto it = m_interfaceRenderableList.begin(); it != m_interfaceRenderableList.end(); it++ ) {
		if( (*it)->isVisible() )
			(*it)->onDraw();
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

void CInterfaceManager::reconstructQuadData()
{
	std::vector<InterfaceVertex> vertices;
	GLuint quadIndex;

	// Generate the new vertex data
	vertices.reserve( m_quadCount * 4 );
	quadIndex = 0;
	for( auto it = m_quadData.begin(); it != m_quadData.end(); it++ ) {
		// Put each set of quads in the vertices
		*((*it).pQuadOffset) = vertices.size();
		vertices.insert( vertices.end(), (*it).vertices.begin(), (*it).vertices.end() );
		*((*it).pQuadIndex) = quadIndex;
		quadIndex++;
	}

	// Get rid of the old contents
	glBindBuffer( GL_ARRAY_BUFFER, m_interfaceVBO );
	glMapBufferRange( GL_ARRAY_BUFFER, 0, m_oldQuadCount*4*sizeof(InterfaceVertex), GL_MAP_INVALIDATE_BUFFER_BIT );
	glUnmapBuffer( GL_ARRAY_BUFFER );
	// Put the new data in
	glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof( InterfaceVertex ), &vertices[0], GL_DYNAMIC_DRAW );

	m_oldQuadCount = m_quadCount;
}
bool CInterfaceManager::addQuads( std::vector<InterfaceVertex> vertices, GLuint textureId, GLuint *pQuadIndex, GLuint *pQuadOffset )
{
	std::vector<int>::iterator emptySpot;
	QuadData quadData;

	if( vertices.size() == 0 || !pQuadIndex )
		return false;
	if( vertices.size() % 4 != 0 )
		return false;

	// Add the quad data
	quadData.quadCount = vertices.size() / 4;
	quadData.textureId = textureId;
	quadData.vertices = vertices;
	quadData.pQuadIndex = pQuadIndex;
	quadData.pQuadOffset = pQuadOffset;
	m_quadCount += quadData.quadCount;
	m_quadData.push_back( quadData );
	// Reconstruct the data
	m_bQuadsInvalid = true;

	// calculated on reconstruct
	*pQuadIndex = 0;
	*pQuadOffset = 0;

	return true;
}
bool CInterfaceManager::updateQuads( std::vector<InterfaceVertex> vertices, GLuint quadIndex )
{


	return true;
}
bool CInterfaceManager::removeQuads( GLuint quadIndex, GLuint count )
{
	if( quadIndex > m_quadData.size() )
		return false;
	m_quadCount -= m_quadData[quadIndex].quadCount;
	// Low the index of all the quads after
	if( quadIndex != m_quadData.size() - 1 ) {
		for( auto it = m_quadData.begin() + quadIndex + 1; it != m_quadData.end(); it++ ) {
			*((*it).pQuadIndex) = *(*it).pQuadIndex-1;
		}
	}
	// Remove the quad
	m_quadData.erase( m_quadData.begin() + quadIndex );
	// Reconstruct the data
	m_bQuadsInvalid = true;

	return true;
}

////////////////////
// CInterfaceBase //
////////////////////

CInterfaceBase::CInterfaceBase() {
	m_positionRel = glm::vec2( 0.0f, 0.0f );
	m_sizeRel = glm::vec2( 0.0f, 0.0f );
	m_pParent = NULL;
	m_bVisible = true;
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
bool CInterfaceBase::isVisible() {
	return m_bVisible;
}
void CInterfaceBase::setVisible( bool visible ) {
	m_bVisible = visible;
	this->onVisibilityChange();
}

/////////////////////////
// CInterfaceContainer //
/////////////////////////

CInterfaceContainer::CInterfaceContainer() {

}
CInterfaceContainer::~CInterfaceContainer() {
}

bool CInterfaceContainer::onCreate()
{
	return true;
}
void CInterfaceContainer::onDestroy()
{
	this->clearContainer();
}

bool CInterfaceContainer::addToContainer( CInterfaceBase *pControl )
{
	// Make sure it doesn't already have a parent
	if( pControl->m_pParent ) {
		PrintWarn( L"Cannot set parent of control that already has a parent\n" );
		return false;
	}
	pControl->m_pParent = this;
	pControl->onParentChange();
	m_children.push_back( pControl );

	return true;
}
bool CInterfaceContainer::removeFromContainer( CInterfaceBase *pControl )
{
	ChildrenList::iterator child;

	// Make sure we're its parent
	if( pControl->m_pParent != this ) {
		PrintWarn( L"Cannot remove control from container because the container was not its parent\n" );
		return false;
	}
	// Find it
	child = std::find( m_children.begin(), m_children.end(), pControl );
	if( child == m_children.end() ) {
		PrintError( L"child not in children list!!\n" );
		return false;
	}
	// Erase
	m_children.erase( child );
	pControl->m_pParent = NULL;
	pControl->onParentChange();

	return true;
}
void CInterfaceContainer::clearContainer()
{
	// Clear all the children
	for( auto it = m_children.begin(); it != m_children.end(); it++ )
	{
		(*it)->m_pParent = NULL;
		(*it)->onParentChange();
	}
	m_children.clear();
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