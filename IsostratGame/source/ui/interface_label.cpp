#include "base.h"
#include "ui\interface.h"
#include "ui\interface_label.h"
#include "ui\font.h"

CInterfaceLabel::CInterfaceLabel() {
	m_pQuadPosition = new GLuint;
	m_pQuadOffset = new GLuint;
	m_vertexCount = 0;
}
CInterfaceLabel::~CInterfaceLabel() {
	SAFE_DELETE( m_pQuadPosition );
	SAFE_DELETE( m_pQuadOffset );
}

bool CInterfaceLabel::onCreate() {
	return true;
}
void CInterfaceLabel::onDestroy()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();

	// Remove the quad
	if( *m_pQuadPosition != INVALID_QUAD_POS )
		pManager->removeQuads( *m_pQuadPosition, 1 );
	m_vertexCount = 0;
}

void CInterfaceLabel::rebuildTextQuads()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();
	std::vector<InterfaceVertex> vertices;

	
}

bool CInterfaceLabel::onActivate()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();
	std::vector<InterfaceVertex> vertices;
	glm::vec2 pos, size;

	if( !CInterfaceBase::onActivate() )
		return false;

	// Add a quad
	pos = this->getRelativePosition();
	size = this->getRelativeSize();
	vertices.resize( 4 );
	vertices[0].relpos = glm::vec2( pos.x, pos.y + size.y );
	vertices[0].tex = glm::vec2( 0.0f, 0.0f );
	vertices[1].relpos = glm::vec2( pos.x, pos.y );
	vertices[1].tex = glm::vec2( 0.0f, 1.0f );
	vertices[2].relpos = glm::vec2( pos.x + size.x, pos.y + size.y );
	vertices[2].tex = glm::vec2( 1.0f, 0.0f );
	vertices[3].relpos = glm::vec2( pos.x + size.x, pos.y );
	vertices[3].tex = glm::vec2( 1.0f, 1.0f );
	if( !pManager->addQuads( vertices, pManager->getFontManager()->getFont( L"DEFAULTFONT" )->getTextureId(), m_pQuadPosition, m_pQuadOffset ) )
		return false;
	m_vertexCount = vertices.size();

	return true;
}

void CInterfaceLabel::onDraw()
{
	// Bind the texture
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 1 );
	// Draw
	glDrawArrays( GL_TRIANGLE_STRIP, *(this->m_pQuadOffset), m_vertexCount );
}

void CInterfaceLabel::setText( std::wstring text ) {
	m_text = text;
	m_bUpdateText = true;
}
std::wstring CInterfaceLabel::getText() {
	return m_text;
}