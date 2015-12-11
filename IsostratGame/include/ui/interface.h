#pragma once

class CLocalization;

class CInterfaceManager
{
private:
	CLocalization *m_pLocalization;
public:
	CInterfaceManager();
	~CInterfaceManager();

	bool initialize();
	void destroy();

	CLocalization* getLocalization();
};