#pragma once

#include <map>
#include <ft2build.h>
#include <freetype.h>
#include <boost\filesystem.hpp>
#include <unordered_set>

class CFont;

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
public:
	CFont();
	~CFont();

	bool initializeFont( FT_Library hFreeType, std::wstring fontName, std::unordered_set<wchar_t> cacheChars );
	void destroy();
};