#pragma once

#include <SDL.h>
#include <string>

class CConsole;
class CGraphics;

class CGame
{
private:
	CConsole *m_pConsole;
	CGraphics *m_pGraphics;

	bool m_bRunning;

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

	CConsole* getConsole();
};