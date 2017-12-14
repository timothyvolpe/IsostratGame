#include "base.h"
#include "ui\font.h"

#include <freetype\ftglyph.h>

//////////////////
// CFontManager //
//////////////////

boost::filesystem::path CFontManager::getSystemFontDirectory()
{
#ifdef _WINDOWS
	boost::filesystem::path fontPath;
	wchar_t winDir[255];

	// geth the windows directory
	::GetWindowsDirectory( winDir, 255 );
	fontPath /= winDir;
	fontPath /= "fonts";
	return fontPath;
#endif
}

CFontManager::CFontManager() {
	m_hFreeType = NULL;
}
CFontManager::~CFontManager() {
}

bool CFontManager::initialize()
{
	FT_Error initError;

	// Initialize FreeType
	PrintInfo( L"Initializing freetype2...\n" );
	initError = FT_Init_FreeType( &m_hFreeType );
	if( initError ) {
		PrintError( L"Failed to initialize freetype2 (%i)\n", initError );
		return false;
	}

	return true;
}
void CFontManager::destroy() {
	this->unloadFonts();
	// Destroy FreeType
	if( m_hFreeType ) {
		FT_Done_FreeType( m_hFreeType );
		m_hFreeType = NULL;
	}
}

bool CFontManager::loadFonts( std::map<std::wstring, std::wstring> fontNameList, std::unordered_set<wchar_t> cacheChars )
{
	// Clear old fonts
	this->unloadFonts();
	// Load each font
	for( auto it = fontNameList.begin(); it != fontNameList.end(); it++ )
	{
		CFont *pFont;

		// Create the font
		pFont = new CFont();
		if( pFont->initializeFont( m_hFreeType, (*it).second, cacheChars ) ) {
			SAFE_DELETE( pFont );
			continue;
		}

		// Add it to the list
		m_fontList.insert( std::pair<std::wstring, CFont*>( (*it).first, pFont ) );
	}

	return true;
}

void CFontManager::unloadFonts()
{
	// Destroy all the old fonts
	for( auto it = m_fontList.begin(); it != m_fontList.end(); it++ ) {
		DESTROY_DELETE( (*it).second );
	}
}

CFont* CFontManager::getFont( std::wstring fontIdentifier ) {
	if( m_fontList.find( fontIdentifier ) != m_fontList.end() )
		return m_fontList[fontIdentifier];
	else
		return NULL;
}

FT_Library CFontManager::getFreetype() {
	return m_hFreeType;
}

///////////
// CFont //
///////////

CFont::CFont() {
	m_fontFace = NULL;
	m_fontName = L"";
	m_textureId = 0;
}
CFont::~CFont() {
}

bool CFont::initializeFont( FT_Library hFreeType, std::wstring fontName, std::unordered_set<wchar_t> cacheChars )
{
	FT_Error ftError;
	boost::filesystem::path fontPath;
	FT_Long faceFlags;
	bool bGlyphFailed;
	std::vector<GlyphBitmap> glyphList;
	int fontMapArea;
	int texWidth, texHeight;

	m_fontName = fontName;

	// Get a path to the font
	fontPath = CFontManager::getSystemFontDirectory();
	fontPath /= fontName;
	// Attempt to load the font
	ftError = FT_New_Face( hFreeType, fontPath.string().c_str(), 0, &m_fontFace );
	if( ftError ) {
		PrintWarn( L"Failed to load font \"%s\" (open font, %i)\n", fontName.c_str(), ftError );
		return false;
	}

	// Cache the needed glyphs
	PrintInfo( L"Caching font \"%s\"...\n", fontName.c_str() );

	// Check the flags
	faceFlags = m_fontFace->face_flags;
	if( faceFlags & FT_FACE_FLAG_VERTICAL ) {
		PrintWarn( L"Failed to load font \"%s\" because vertical fonts are not supported\n", fontName.c_str() );
		FT_Done_Face( m_fontFace );
		m_fontFace = NULL;
		return false;
	}

	std::vector<int> pointSizes;
	pointSizes.push_back( 8 );
	pointSizes.push_back( 10 );
	pointSizes.push_back( 14 );
	pointSizes.push_back( 20 );
	pointSizes.push_back( 32 );

	// Load each glyph for each point size
	bGlyphFailed = false;
	fontMapArea = 0;
	for( auto it2 = pointSizes.begin(); it2 != pointSizes.end(); it2++ )
	{
		if( bGlyphFailed )
			break;
		// Set the face size (16pt)
		ftError = FT_Set_Char_Size( m_fontFace, 0, (*it2) * 64, 0, 0 );
		if( ftError ) {
			PrintWarn( L"Failed to load font \"%s\" (font size, %i)\n", fontName.c_str(), ftError );
			bGlyphFailed = true;
			break;
		}
		for( std::unordered_set<wchar_t>::iterator it = cacheChars.begin(); it != cacheChars.end(); it++ )
		{
			FT_Bitmap glyphBitmap;

			// Load the glyph
			ftError = FT_Load_Char( m_fontFace, (*it), FT_LOAD_RENDER );
			if( ftError ) {
				PrintWarn( L"Failed to load font \"%s\" (load glyph, %i)\n", fontName.c_str(), ftError );
				bGlyphFailed = true;
				break;
			}

			int advance = m_fontFace->glyph->metrics.horiAdvance / 64;
			int verticalOffset = -(m_fontFace->glyph->metrics.height - m_fontFace->glyph->metrics.horiBearingY) / 64;

			glyphBitmap = m_fontFace->glyph->bitmap;

			// Check the pixel mode
			if( glyphBitmap.pixel_mode != FT_PIXEL_MODE_GRAY ) {
				PrintWarn( L"Failed to load font \"%s\" (invalid pixel mode)\n", fontName.c_str() );
				bGlyphFailed = true;
				break;
			}

			int height = glyphBitmap.rows;
			int width = glyphBitmap.width;

			// Copy it into the glyph list
			GlyphBitmap glyphBuffer;
			glyphBuffer.charId = (*it);
			glyphBuffer.width = width;
			glyphBuffer.height = height;
			glyphBuffer.pBuffer = new GLubyte[width*height];
			glyphBuffer.pointSize = (*it2);
			glyphBuffer.advance = advance;
			glyphBuffer.verticalOffset = verticalOffset;
			memcpy( glyphBuffer.pBuffer, glyphBitmap.buffer, width*height );
			glyphList.push_back( glyphBuffer );
			// Increase required area
			fontMapArea += width*height;
		}
	}
	if( bGlyphFailed ) {
		FT_Done_Face( m_fontFace );
		m_fontFace = NULL;
		return false;
	}
	
	// Calculate the required texture size
	texWidth = texHeight = 1024;
	// Perform the bin packing
	if( !this->startBinPack( glyphList, texWidth, texHeight ) ) {
		return false;
	}

	return false;
}
void CFont::destroy() {
	if( m_textureId ) {
		glDeleteTextures( 1, &m_textureId );
		m_textureId = 0;
	}
	if( m_fontFace ) {
		FT_Done_Face( m_fontFace );
		m_fontFace = NULL;
	}
	m_fontName = L"deleted";
}

bool CFont::startBinPack( std::vector<GlyphBitmap> glyphList, int width, int height )
{
	GLubyte *pTextureMap;
	FontMapNode *pRootNode;

	// Create the texture
	glGenTextures( 1, &m_textureId );
	glBindTexture( GL_TEXTURE_2D, m_textureId );
	// Texture parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Allocate the bin
	pTextureMap = new GLubyte[width*height];
	// FOR TEST
	//memset( pTextureMap, 0, sizeof( GLubyte )*width*height );

	// Sort by area
	std::sort( glyphList.begin(), glyphList.end() );

	// Root node
	pRootNode = new FontMapNode;
	pRootNode->pLeft = NULL;
	pRootNode->pRight = NULL;
	pRootNode->width = width;
	pRootNode->height = height;
	pRootNode->x = pRootNode->y = 0;
	pRootNode->pBuffer = NULL;
	m_binNodes.clear();
	m_binNodes.push_back( pRootNode );
	// Pack the glyphs
	for( std::vector<GlyphBitmap>::iterator it = glyphList.begin(); it != glyphList.end(); it++ ) {
		// Insert into a node
		if( !this->insertIntoBin( pRootNode, (*it) ) ) {
			PrintError( L"Font tree ran out of space\n" );
			break;//return false;
		}
	}

	// Copy each glyph into its bin spot
	for( std::vector<FontMapNode*>::iterator it = m_binNodes.begin(); it != m_binNodes.end(); it++ )
	{
		if( (*it)->pLeft || (*it)->pRight )
		{
			// Draw diagnostics
			for( int x = 0; x < (*it)->width; x++ )
			{
				for( int y = 0; y < (*it)->height; y++ )
				{
					// Copy the glyph data
					pTextureMap[(*it)->x + x + ((*it)->y + y)*height] = (*it)->pBuffer[x + y*(*it)->width];
					// Add it to the map
					FontGlyph renderedGlyph;
					renderedGlyph.width = (*it)->width;
					renderedGlyph.height = (*it)->height;
					renderedGlyph.uv = glm::vec2( (float)(*it)->x / (float)width, (float)(*it)->y / (float)height );
					renderedGlyph.uv_end = glm::vec2( (float)(*it)->width / (float)width + renderedGlyph.uv.x, (float)(*it)->height / (float)height + renderedGlyph.uv.y );
					renderedGlyph.advance = (*it)->advance;
					renderedGlyph.verticalOffset = (*it)->verticalOffset;
					m_renderedGlyphs[(*it)->pointSize][(*it)->charId] = renderedGlyph;
				}
			}
		}
		// Cleanup
		SAFE_DELETE_A( (*it)->pBuffer );
		SAFE_DELETE( (*it) );
	}
	m_binNodes.clear();
	glyphList.clear();

	// Store the pixel data
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pTextureMap );
	// Clean up
	SAFE_DELETE_A( pTextureMap );

	return true;
}
CFont::FontMapNode* CFont::insertIntoBin( FontMapNode *pNode, GlyphBitmap glyphBitmap )
{
	int newWidth, newHeight;

	// Check if this node is internal (has children)
	if( pNode->pLeft || pNode->pRight )
	{
		// Check its neighbors for space
		if( pNode->pLeft ) {
			// Try to put it in the neighbor
			FontMapNode *pNewNode = insertIntoBin( pNode->pLeft, glyphBitmap );
			if( pNewNode )
				return pNewNode;
		}
		if( pNode->pRight ) {
			// Try to put it in the neighbor
			FontMapNode *pNewNode = insertIntoBin( pNode->pRight, glyphBitmap );
			if( pNewNode )
				return pNewNode;
		}
		// No space in its neighbors
		return NULL;
	}

	// If it has no children, check if the glyph fits here
	if( glyphBitmap.width > pNode->width || glyphBitmap.height > pNode->height )
		return NULL;

	// Split the node along the short axis and 
	// insert the glyph
	newWidth = pNode->width - glyphBitmap.width;
	newHeight = pNode->height - glyphBitmap.height;
	// Create new children
	pNode->pLeft = new FontMapNode();
	pNode->pRight = new FontMapNode();
	m_binNodes.push_back( pNode->pLeft );
	m_binNodes.push_back( pNode->pRight );
	if( newWidth <= newHeight )
	{
		pNode->pLeft->x = pNode->x + glyphBitmap.width;
		pNode->pLeft->y = pNode->y;
		pNode->pLeft->width = newWidth;
		pNode->pLeft->height = glyphBitmap.height;

		pNode->pRight->x = pNode->x;
		pNode->pRight->y = pNode->y + glyphBitmap.height;
		pNode->pRight->width = pNode->width;
		pNode->pRight->height = newHeight;
	}
	else
	{
		pNode->pLeft->x = pNode->x;
		pNode->pLeft->y = pNode->y + glyphBitmap.height;
		pNode->pLeft->width = glyphBitmap.width;
		pNode->pLeft->height = newHeight;

		pNode->pRight->x = pNode->x + glyphBitmap.width;
		pNode->pRight->y = pNode->y;
		pNode->pRight->width = newWidth;
		pNode->pRight->height = pNode->height;
	}

	// Shrink the node
	pNode->width = glyphBitmap.width;
	pNode->height = glyphBitmap.height;
	pNode->advance = glyphBitmap.advance;
	pNode->verticalOffset = glyphBitmap.verticalOffset;

	pNode->charId = glyphBitmap.charId;
	pNode->pointSize = glyphBitmap.pointSize;
	pNode->pBuffer = glyphBitmap.pBuffer;

	return pNode;
}

GLuint CFont::getTextureId() {
	return m_textureId;
}

FontGlyph CFont::getGlyph( int pointSize, wchar_t character )
{
	if( m_renderedGlyphs.find( pointSize ) != m_renderedGlyphs.end() ) {
		if( m_renderedGlyphs[pointSize].find( character ) != m_renderedGlyphs[pointSize].end() )
			return m_renderedGlyphs[pointSize][character];
	}
	PrintWarn( L"Could not find character glyph in font\n" );
	return FontGlyph();
}