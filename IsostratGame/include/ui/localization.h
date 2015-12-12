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

class CLocalization
{
private:
	boost::property_tree::wptree m_langTree;

	std::wstring m_languageName;
	FontNameList m_fontNameList;
public:
	CLocalization();
	~CLocalization();

	bool initialize();
	void destroy();

	bool loadLanguage( unsigned char language );

	FontNameList getFontNameList();
	std::unordered_set<wchar_t> getCacheChars();
};