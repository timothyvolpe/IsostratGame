#pragma once

#include <map>
#include <ft2build.h>
#include <freetype.h>
#include <boost\filesystem.hpp>
#include <unordered_set>
#include <GL\glew.h>

class CFont;

#define MAX_FONTMAP_SIZE 4096

//////////////////
// CFontManager //
//////////////////

class CFontManager
{
private:
	typedef std::map<std::wstring, CFont*> FontList;

	FT_Library m_hFreeType;
	FontList m_fontList;
public:
	static boost::filesystem::path getSystemFontDirectory();

	CFontManager();
	~CFontManager();

	bool initialize();
	void destroy();

	bool loadFonts( std::map<std::wstring, std::wstring> fontNameList, std::unordered_set<wchar_t> cacheChars );
	void unloadFonts();

	CFont* getFont( std::wstring fontIdentifier );

	FT_Library getFreetype();
};

///////////
// CFont //
///////////

class CFont
{
private:
	FT_Face m_fontFace;
	std::wstring m_fontName;

	GLuint m_textureId;
public:
	CFont();
	~CFont();

	bool initializeFont( FT_Library hFreeType, std::wstring fontName, std::unordered_set<wchar_t> cacheChars );
	void destroy();

	GLuint getTextureId();
};