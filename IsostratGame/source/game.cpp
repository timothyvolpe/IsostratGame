#include <SDL.h>

#include "base.h"
#include "game.h"
#include "graphics.h"
#include "config.h"
#include "input.h"
#include "ui\interface.h"
#include "script\luascript.h"

#include <iostream>
#include <sstream>
#include <boost\timer\timer.hpp>
#include <boost\chrono.hpp>

CGame::CGame()
{
	m_pConsole = NULL;
	m_pGraphics = NULL;
	m_pConfigLoader = NULL;
	m_pInput = NULL;
	m_pInterfaceManager = NULL;
	m_pLuaManager = NULL;
	m_bRunning = false;
	m_bMouseLocked = false;
	m_frameTime = 0.0;
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

	// Trap mouse
	m_bMouseLocked = true;
	SDL_SetRelativeMouseMode( SDL_TRUE );

	// Load the config
	m_pConfigLoader = new CConfigLoader();
	if( !m_pConfigLoader->initializeAndLoad() )
		return false;

	// Create the window and the context, etc.
	m_pGraphics = new CGraphics();
	if( !m_pGraphics->initialize() )
		return false;

	// Create the input handler
	m_pInput = new CInput();

	// Create the lua manager
	m_pLuaManager = new CLuaManager();
	if( !m_pLuaManager->initialize() )
		return false;

	// Load the interface manager
	m_pInterfaceManager = new CInterfaceManager();
	if( !m_pInterfaceManager->initialize() )
		return false;
	m_pInterfaceManager->setDimensions( m_pConfigLoader->getResolutionX(), m_pConfigLoader->getResolutionY() );

	// Execute the client scripts
	if( !m_pLuaManager->executeScripts( false ) )
		return false;

	// Enter the game loop
	if( !this->gameLoop() )
		return false;

	return true;
}

void CGame::destroy()
{
	// Destroy interfaces
	DESTROY_DELETE( m_pInterfaceManager );
	// Destroy lua manager
	DESTROY_DELETE( m_pLuaManager );
	// Destroy input
	SAFE_DELETE( m_pInput );
	// Destroy graphics
	DESTROY_DELETE( m_pGraphics );
	// Destroy config
	DESTROY_DELETE( m_pConfigLoader );

	// Quit SDL
	SDL_Quit();

	DESTROY_DELETE( m_pConsole );
}

bool CGame::gameLoop()
{
	SDL_Event pollEvent;

	boost::timer::cpu_timer gameTimer;
	boost::chrono::duration<double> timerSeconds;

	m_bRunning = true;
	while( m_bRunning )
	{
		gameTimer.start();
		// Handle all the SDL events
		while( SDL_PollEvent( &pollEvent ) )
			this->handleEvent( pollEvent );

		// Quit if ESC is pressed
#ifdef QUICK_QUIT
		if( m_pInput->isKeyPress( SDL_SCANCODE_ESCAPE ) ) {
			SDL_Event quitEvent;
			quitEvent.type = SDL_QUIT;
			quitEvent.user.code = 0;
			SDL_PushEvent( &quitEvent );
		}
#endif

		// Draw the scene
		m_pGraphics->draw();
		// Update input
		m_pInput->update();

		// Get the frame time
		timerSeconds = boost::chrono::nanoseconds( gameTimer.elapsed().wall );
		m_frameTime = timerSeconds.count();
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
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		m_pInput->handleEvent( sdlEvent );
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
CConfigLoader* CGame::getConfigLoader() {
	return m_pConfigLoader;
}
CInput* CGame::getInput() {
	return m_pInput;
}
CGraphics* CGame::getGraphics() {
	return m_pGraphics;
}
CInterfaceManager* CGame::getInterfaceManager() {
	return m_pInterfaceManager;
}
CLuaManager* CGame::getLuaManager() {
	return m_pLuaManager;
}

double CGame::getFrameTime() {
	return m_frameTime;
}