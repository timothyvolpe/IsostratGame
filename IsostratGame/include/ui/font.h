#pragma once

#include <map>
#include <ft2build.h>
#include <freetype.h>
#include <boost\filesystem.hpp>
#include <unordered_set>
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <boost\unordered_map.hpp>

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

typedef struct
{
	int width, height;
	glm::vec2 uv;
} FontGlyph;

class CFont
{
private:
	typedef boost::unordered_map<wchar_t, FontGlyph> GlyphMap;
	typedef boost::unordered_map<int, GlyphMap> GlypedSizeMap;

	struct GlyphBitmap
	{
		wchar_t charId;
		int pointSize;
		int width, height;
		GLubyte *pBuffer;

		bool operator<( const GlyphBitmap &rhs ) const { return width*height > rhs.width*rhs.height; }
	};
	struct FontMapNode
	{
		FontMapNode *pLeft, *pRight;
		int x, y;
		int width, height;
		wchar_t charId;
		int pointSize;
		GLubyte *pBuffer;
	};

	FT_Face m_fontFace;
	std::wstring m_fontName;

	GLuint m_textureId;

	// http://clb.demon.fi/projects/rectangle-bin-packing
	std::vector<FontMapNode*> m_binNodes;
	bool startBinPack( std::vector<GlyphBitmap> glyphList, int width, int height );
	FontMapNode* insertIntoBin( FontMapNode *pNode, GlyphBitmap glyphBitmap );

	GlypedSizeMap m_renderedGlyphs;
public:
	CFont();
	~CFont();

	bool initializeFont( FT_Library hFreeType, std::wstring fontName, std::unordered_set<wchar_t> cacheChars );
	void destroy();

	GLuint getTextureId();

	FontGlyph getGlyph( wchar_t character );
};