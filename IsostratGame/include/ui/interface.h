#pragma once

class CFontManager;
class CLocalization;

class CInterfaceManager
{
private:
	CFontManager *m_pFontManager;
	CLocalization *m_pLocalization;
public:
	CInterfaceManager();
	~CInterfaceManager();

	bool initialize();
	void destroy();

	bool setLanguage( unsigned char language );

	CFontManager* getFontManager();
	CLocalization* getLocalization();
};