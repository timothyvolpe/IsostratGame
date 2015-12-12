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

FT_Library CFontManager::getFreetype() {
	return m_hFreeType;
}

///////////
// CFont //
///////////

CFont::CFont() {
	m_fontFace = NULL;
	m_fontName = L"";
}
CFont::~CFont() {
}

bool CFont::initializeFont( FT_Library hFreeType, std::wstring fontName, std::unordered_set<wchar_t> cacheChars )
{
	FT_Error ftError;
	boost::filesystem::path fontPath;

	m_fontName = fontName;

	// Get a path to the font
	fontPath = CFontManager::getSystemFontDirectory();
	fontPath /= fontName;
	// Attempt to load the font
	ftError = FT_New_Face( hFreeType, fontPath.string().c_str(), 0, &m_fontFace );
	if( ftError ) {
		PrintWarn( L"Failed to load font \"%s\" (%i)\n", fontName.c_str(), ftError );
		return false;
	}

	// Cache the needed glyphs
	PrintInfo( L"Caching font \"%s\"...\n", fontName.c_str() );

	// TODO: Load all the cached glyphs

	return false;
}
void CFont::destroy() {
	if( m_fontFace ) {
		FT_Done_Face( m_fontFace );
		m_fontFace = NULL;
	}
	m_fontName = L"deleted";
}