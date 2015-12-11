#pragma once
#include <boost\property_tree\ptree.hpp>

enum : unsigned char
{
	GAMELANGUAGE_ENGLISH,
	GAMELANGUAGE_FRENCH
};

class CLocalization
{
private:
	boost::property_tree::wptree m_langTree;

	std::wstring m_languageName;
public:
	CLocalization();
	~CLocalization();

	bool initialize();
	void destroy();

	bool loadLanguage( unsigned char language );
};