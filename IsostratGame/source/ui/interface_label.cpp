#include "base.h"
#include "ui\interface.h"
#include "ui\interface_label.h"
#include "ui\font.h"
#include "ui\localization.h"

CInterfaceLabel::CInterfaceLabel() {
	m_pQuadPosition = new GLuint;
	m_pQuadOffset = new GLuint;
	*m_pQuadPosition = INVALID_QUAD_POS;
	*m_pQuadOffset = INVALID_QUAD_POS;
	m_vertexCount = 0;
	m_type = INTERFACE_TYPE_LABEL;
}
CInterfaceLabel::~CInterfaceLabel() {
	SAFE_DELETE( m_pQuadPosition );
	SAFE_DELETE( m_pQuadOffset );
}

bool CInterfaceLabel::onCreate() {
	return true;
}
void CInterfaceLabel::onDestroy() {
	this->destroyTextQuads();
}

void CInterfaceLabel::rebuildTextQuads()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();
	CFont *pFont;
	std::vector<InterfaceVertex> vertices;
	InterfaceVertex currentVertex;
	glm::vec2 pos;
	glm::vec2 scaledSize;
	float offsetx;
	float relVericalOffset;

	this->destroyTextQuads();

	pFont = pManager->getFontManager()->getFont( L"DEFAULTFONT" );
	pos = this->getPosRespectiveToParent();

	// Create a quad for each character
	offsetx = 0;
	for( size_t i = 0; i < m_text.length(); i++ )
	{
		// Make sure its not a special characters
		switch( m_text[i] )
		{
		case L' ':
		case L'\x96':
		case L'\x9':
		case L'\xA':
		case L'\xD':
			continue;
		}
		FontGlyph glyph = pFont->getGlyph( 20, m_text[i] );
		relVericalOffset = glyph.verticalOffset / (float)pManager->getHeight();

		scaledSize = glm::vec2( glyph.width, glyph.height ) / glm::vec2( pManager->getWidth(), pManager->getHeight() );

		currentVertex.relpos = glm::vec2( pos.x + offsetx, pos.y + scaledSize.y + relVericalOffset );
		currentVertex.tex = glm::vec2( glyph.uv.x, glyph.uv.y );
		vertices.push_back( currentVertex );

		currentVertex.relpos = glm::vec2( pos.x + offsetx, pos.y + relVericalOffset );
		currentVertex.tex = glm::vec2( glyph.uv.x, glyph.uv_end.y );
		vertices.push_back( currentVertex );

		currentVertex.relpos = glm::vec2( pos.x + offsetx + scaledSize.x, pos.y + scaledSize.y + relVericalOffset );
		currentVertex.tex = glm::vec2( glyph.uv_end.x, glyph.uv.y );
		vertices.push_back( currentVertex );

		currentVertex.relpos = glm::vec2( pos.x + offsetx + scaledSize.x, pos.y + relVericalOffset );
		currentVertex.tex = glm::vec2( glyph.uv_end.x, glyph.uv_end.y );
		vertices.push_back( currentVertex );

		offsetx += (float)glyph.advance / (float)pManager->getWidth();
		//offsetx += scaledSize.x;
	}
	if( vertices.size() > 0 ) {
		if( !pManager->addQuads( vertices, pFont->getTextureId(), m_pQuadPosition, m_pQuadOffset ) ) {
			PrintWarn( L"Failed to update label text\n" );
			this->destroyTextQuads();
			return;
		}
		m_vertexCount = vertices.size();
	}
}
void CInterfaceLabel::destroyTextQuads()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();

	// Remove the quad
	if( m_pQuadPosition ) {
		if( *m_pQuadPosition != INVALID_QUAD_POS )
			pManager->removeQuads( *m_pQuadPosition, m_vertexCount / 4 );
	}
	m_vertexCount = 0;
}

void CInterfaceLabel::onUpdate()
{
	CInterfaceBase::onUpdate();

	if( m_bUpdateText ) {
		this->rebuildTextQuads();
		m_bUpdateText = false;
	}
}

bool CInterfaceLabel::onActivate()
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();

	if( !CInterfaceBase::onActivate() )
		return false;

	this->rebuildTextQuads();

	return true;
}

void CInterfaceLabel::onDraw()
{
	if( m_pQuadOffset )
	{
		// Bind the texture
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, 1 );
		// Draw
		glDrawArrays( GL_LINES_ADJACENCY, *(this->m_pQuadOffset), m_vertexCount );
	}
}

void CInterfaceLabel::onTextChange() {
	m_bUpdateText = true;
}