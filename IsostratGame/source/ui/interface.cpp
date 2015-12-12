#include "base.h"
#include "ui\interface.h"
#include "ui\localization.h"
#include "ui\font.h"

CInterfaceManager::CInterfaceManager() {
	m_pFontManager = NULL;
	m_pLocalization = NULL;
}
CInterfaceManager::~CInterfaceManager() {
}

bool CInterfaceManager::initialize()
{
	// Create the font manager
	m_pFontManager = new CFontManager();
	if( !m_pFontManager->initialize() )
		return false;
	// Create the localization
	m_pLocalization = new CLocalization();
	if( !m_pLocalization->initialize() )
		return false;

	this->setLanguage( GAMELANGUAGE_ENGLISH );
	
	return true;
}
void CInterfaceManager::destroy()
{
	DESTROY_DELETE( m_pLocalization );
	DESTROY_DELETE( m_pFontManager );
}

bool CInterfaceManager::setLanguage( unsigned char language )
{
	// Load english as the language
	if( !m_pLocalization->loadLanguage( GAMELANGUAGE_ENGLISH ) )
		return false;
	// Load the fonts
	if( !m_pFontManager->loadFonts( m_pLocalization->getFontNameList(), m_pLocalization->getCacheChars() ) )
		return false;

	return true;
}

CFontManager* CInterfaceManager::getFontManager() {
	return m_pFontManager;
}
CLocalization* CInterfaceManager::getLocalization() {
	return m_pLocalization;
}