#pragma once

#include <SDL.h>
#include <string>

#define QUICK_QUIT

class CConsole;
class CGraphics;
class CConfigLoader;
class CInput;
class CInterfaceManager;
class CLuaManager;

class CGame
{
private:
	CConsole *m_pConsole;
	CGraphics *m_pGraphics;
	CConfigLoader *m_pConfigLoader;
	CInput *m_pInput;

	CInterfaceManager *m_pInterfaceManager;

	CLuaManager *m_pLuaManager;

	bool m_bRunning;
	bool m_bMouseLocked;

	double m_frameTime;

	CGame();

	CGame( CGame const& ) = delete;
	void operator=( CGame const& ) = delete;

	void displayGameInfo();

	bool gameLoop();
	void handleEvent( SDL_Event sdlEvent );
public:
	static CGame& instance() {
		static CGame instance;
		return instance;
	}

	bool start();
	void destroy();

	void displayMessagebox( std::wstring message );

	CGraphics* getGraphics();
	CConsole* getConsole();
	CConfigLoader* getConfigLoader();
	CInput* getInput();
	CInterfaceManager* getInterfaceManager();
	CLuaManager *getLuaManager();

	double getFrameTime();
};