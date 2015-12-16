#include "base.h"
#include "ui\font.h"

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

	// Set the face size
	ftError = FT_Set_Char_Size( m_fontFace, 0, 16 * 64, 300, 300 );
	if( ftError ) {
		PrintWarn( L"Failed to load font \"%s\" (font size, %i)\n", fontName.c_str(), ftError );
		FT_Done_Face( m_fontFace );
		m_fontFace = NULL;
		return false;
	}

	// Create the texture
	glGenTextures( 1, &m_textureId );
	glBindTexture( GL_TEXTURE_2D, m_textureId );
	// Texture parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// Load each glyph
	for( std::unordered_set<wchar_t>::iterator it = cacheChars.begin(); it != cacheChars.end(); it++ )
	{
		FT_UInt glyphIndex;
		FT_GlyphSlot glyph;
		
		// Get the index
		glyphIndex = FT_Get_Char_Index( m_fontFace, (*it) );
		// Load and render the glyph
		ftError = FT_Load_Glyph( m_fontFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_MONOCHROME );
		if( ftError ) {
			PrintWarn( L"Failed to load font \"%s\" (load glyph, %i)\n", fontName.c_str(), ftError );
			FT_Done_Face( m_fontFace );
			m_fontFace = NULL;
			return false;
		}

		// Add it to the texture map
		glyph = m_fontFace->glyph;

		int height = glyph->bitmap.rows;
		int width = glyph->bitmap.width;

		// Add it to the texture
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer );

		break;
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

GLuint CFont::getTextureId() {
	return m_textureId;
}