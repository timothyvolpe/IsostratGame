#pragma once
#include <map>
#include <unordered_set>
#include <boost\property_tree\ptree.hpp>

enum : unsigned char
{
	GAMELANGUAGE_ENGLISH,
	GAMELANGUAGE_FRENCH
};

typedef std::map<std::wstring, std::wstring> FontNameList;
typedef std::map<std::wstring, std::wstring> LocalizedStringList;

class CLocalization
{
private:
	boost::property_tree::wptree m_langTree;

	std::wstring m_languageName;
	FontNameList m_fontNameList;
	LocalizedStringList m_localizedStrings;
public:
	CLocalization();
	~CLocalization();

	bool initialize();
	void destroy();

	bool loadLanguage( unsigned char language );

	std::wstring localizeString( std::wstring str );

	FontNameList getFontNameList();
	std::unordered_set<wchar_t> getCacheChars();
};