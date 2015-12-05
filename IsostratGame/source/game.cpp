#include <SDL.h>

#include "base.h"
#include "game.h"
#include "graphics.h"

#include <iostream>
#include <sstream>

CGame::CGame()
{
	m_pConsole = NULL;
	m_pGraphics = NULL;
	m_bRunning = false;
}

void CGame::displayGameInfo()
{
	PrintInfo( L"--------------------------------------------------------------------\n" );
	PrintInfo( L"\t\"%s\"\n", WINDOW_TITLE );
	PrintInfo( L"\tCreated by Timothy Volpe\n" );
	PrintInfo( L"\tCopyright 2015 (c)\n" );
	PrintInfo( L"\thttps://github.com/tvolpes/IsostratGame\n" );
	PrintInfo( L"--------------------------------------------------------------------\n\n" );
}

bool CGame::start()
{
	SDL_version sdlCompiled, sdlLinked;

	// Initialize the console
	m_pConsole = new CConsole();
	if( !m_pConsole->initialize() ) {
		this->displayMessagebox( L"could not initialize game console" );
		return false;
	}
	this->displayGameInfo();

	// Initialize SDL
	SDL_VERSION( &sdlCompiled );
	PrintInfo( L"Initializing SDL2 (compiled version %d.%d.%d)...\n", sdlCompiled.major, sdlCompiled.minor, sdlCompiled.patch );
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		this->displayMessagebox( L"failed to initialize SDL2" );
		return false;
	}
	// Get the linked version
	SDL_GetVersion( &sdlLinked );
	PrintInfo( L"Successfully linked SDL2 (version %d.%d.%d)\n", sdlLinked.major, sdlLinked.minor, sdlLinked.patch );

	// Create the window and the context, etc.
	m_pGraphics = new CGraphics();
	if( !m_pGraphics->initialize() )
		return false;

	// Enter the game loop
	if( !this->gameLoop() )
		return false;

	return true;
}

void CGame::destroy()
{
	// Destroy graphics
	DESTROY_DELETE( m_pGraphics );

	// Quit SDL
	SDL_Quit();

	DESTROY_DELETE( m_pConsole );
}

bool CGame::gameLoop()
{
	SDL_Event pollEvent;

	m_bRunning = true;
	while( m_bRunning )
	{
		// Handle all the SDL events
		while( SDL_PollEvent( &pollEvent ) )
			this->handleEvent( pollEvent );

		// Draw the scene
		m_pGraphics->draw();
	}

	return true;
}
void CGame::handleEvent( SDL_Event sdlEvent )
{
	switch( sdlEvent.type )
	{
	case SDL_QUIT:
		PrintInfo( L"Quitting...\n" );
		m_bRunning = false;
		break;
	default:
		break;
	}
}

void CGame::displayMessagebox( std::wstring message )
{
	std::wstringstream fullmsg;

	fullmsg << "The game has crashed!\n\nError:\n" << message;
#ifdef _WINDOWS
	::MessageBox( NULL, fullmsg.str().c_str(), WINDOW_TITLE, MB_OK | MB_ICONERROR | MB_DEFBUTTON1 );
#endif
	if( m_pConsole )
		PrintError( message.c_str() );
}

CConsole* CGame::getConsole() {
	return m_pConsole;
}