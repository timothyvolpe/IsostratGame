#include "base.h"
#include "ui\interface.h"
#include "ui\localization.h"

CInterfaceManager::CInterfaceManager() {
	m_pLocalization = NULL;
}
CInterfaceManager::~CInterfaceManager() {
}

bool CInterfaceManager::initialize()
{
	// Create the localization
	m_pLocalization = new CLocalization();
	if( !m_pLocalization->initialize() )
		return false;
	m_pLocalization->loadLanguage( GAMELANGUAGE_ENGLISH );

	return true;
}
void CInterfaceManager::destroy()
{
	DESTROY_DELETE( m_pLocalization );
}

CLocalization* CInterfaceManager::getLocalization() {
	return m_pLocalization;
}