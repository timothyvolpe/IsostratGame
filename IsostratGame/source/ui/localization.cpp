#pragma warning( disable : 4996 )

#include <boost\filesystem.hpp>
#include <boost\property_tree\info_parser.hpp>
#include <boost\algorithm\string.hpp>

#include "base.h"
#include "def.h"
#include "ui\localization.h"

CLocalization::CLocalization() {
}
CLocalization::~CLocalization() {
}

bool CLocalization::initialize() {
	return true;
}
void CLocalization::destroy() {
}

bool CLocalization::loadLanguage( unsigned char language )
{
	boost::filesystem::path langPath;
	boost::property_tree::wptree fontTree, stringTree;

	PrintInfo( L"Loading language (%i)...\n", language );

	// Get the path to the language files
	langPath = boost::filesystem::current_path();
	langPath /= FILESYSTEM_LANGDIR;
	// Get the language file
	switch( language )
	{
	case GAMELANGUAGE_ENGLISH:
		langPath /= "en.lang";
		break;
	case GAMELANGUAGE_FRENCH:
		langPath /= "fr.lang";
		break;
	default:
		PrintError( L"Failed to load language file for unknown language\n" );
		return false;
	}

	// Make sure the file exists and is valid
	if( boost::filesystem::exists( langPath ) ) {
		if( !boost::filesystem::is_regular_file( langPath ) ) {
			PrintError( L"Failed to load language file because it is not a valid file: %s\n", langPath.wstring().c_str() );
			return false;
		}
	}
	else {
		PrintError( L"Failed to load language file because it could not be found: %s\n", langPath.wstring().c_str() );
		return false;
	}

	// Read the file
	try {
		boost::property_tree::read_info( langPath.string(), m_langTree );
	}
	catch( boost::property_tree::info_parser_error &e ) {
		PrintError( L"Failed to load language file (%hs)\n", e.what() );
		return false;
	}

	// Get some info about the language
	m_languageName = m_langTree.get( L"language.info.LANGUAGE_NAME", L"unnamed" );
	PrintInfo( L"Loaded language %s\n", m_languageName.c_str() );
	// Get the font names
	try {
		fontTree = m_langTree.get_child( L"language.fonts" );
		m_fontNameList.clear();
		for( const auto &kv : fontTree ) {
			m_fontNameList.insert( std::pair<std::wstring, std::wstring>( kv.first.data(), kv.second.data() ) );
		}
	}
	catch( boost::property_tree::ptree_bad_path &e ) {
		PrintError( L"Did not find any font info in the language file (%hs)\n", e.what() );
		return false;
	}
	// Get the strings
	try {
		stringTree = m_langTree.get_child( L"language.strings" );
		m_localizedStrings.clear();
		for( const auto &kv : stringTree ) {
			m_localizedStrings.insert( std::pair<std::wstring, std::wstring>( kv.first.data(), kv.second.data() ) );
		}
	}
	catch( boost::property_tree::ptree_bad_path &e ) {
		PrintError( L"Failed to read localized strings (%hs)\n", e.what() );
		return false;
	}

	return true;
}

std::wstring CLocalization::localizeString( std::wstring str )
{
	std::wstring localized;
	std::vector<std::wstring> tokens;
	bool bInPounds;

	// Split into tokens on the #
	boost::split( tokens, str, boost::is_any_of( L"#" ) );
	bInPounds = false;
	localized = L"";
	for( auto it = tokens.begin(); it != tokens.end(); it++ )
	{
		if( !bInPounds )
			localized += (*it);
		else {
			auto localString = m_localizedStrings.find( (*it) );
			if( localString != m_localizedStrings.end() )
				localized += (*localString).second;
			else
				localized += (*it);

		}
		bInPounds = !bInPounds;
	}

	return localized;
}


FontNameList CLocalization::getFontNameList() {
	return m_fontNameList;
}
std::unordered_set<wchar_t> CLocalization::getCacheChars() // Determine what characters we need to cache
{
	std::unordered_set<wchar_t> cacheChars;
	std::wstring infoCache;
	boost::property_tree::wptree stringTree;
	wchar_t currentChar;

	// First, check LANGUAGE_CACHE_CHARS
	infoCache = m_langTree.get( L"language.info.LANGUAGE_CACHE_CHARS", L"" );
	for( unsigned int i = 0; i < infoCache.length(); i++ ) {
		if( infoCache[i] == L'\x96' || infoCache[i] == L'\x9' || infoCache[i] == '\xA' || infoCache[i] == '\xD' ) // \0 \t \n \r
			continue;
		cacheChars.insert( infoCache[i] );
	}

	// Look through all the strings and cache new characters
	try
	{
		stringTree = m_langTree.get_child( L"language.strings" );
		for( const auto &kv : stringTree ) {
			for( unsigned int i = 0; i < kv.second.data().size(); i++ )
			{
				currentChar = kv.second.data()[i];
				if( currentChar == L'\x9C' || currentChar == L'\x9' || currentChar == '\xA' || currentChar == '\xD' )
					continue;
				if( cacheChars.find( currentChar ) == cacheChars.end() )
					cacheChars.insert( currentChar );
			}
		}
	}
	catch( boost::property_tree::ptree_bad_path &e ) {
		PrintWarn( L"Error parsing language file (%hs)\n", e.what() );
		return cacheChars;
	}

	return cacheChars;
}

#pragma warning( default : 4996 )