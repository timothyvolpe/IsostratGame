#include <boost\filesystem.hpp>
#include <boost\property_tree\info_parser.hpp>

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

	return true;
}