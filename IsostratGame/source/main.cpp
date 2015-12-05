#include "base.h"
#include "def.h"
#include "platform.h"
#include "game.h"

#include <GL\glew.h>

// SDL 2.0.3 libraries
#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )

// Glew
#ifdef _DEBUG
#pragma comment( lib, "glew32sd.lib" )
#else
#pragma comment( lib, "glew32s.lib" )
#endif

// OpenGL
#pragma comment( lib, "opengl32.lib" )

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// Start the game
	if( !CGame::instance().start() )
		return -1;

	return 0;
}