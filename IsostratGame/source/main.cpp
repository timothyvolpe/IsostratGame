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

// FreeType
#ifdef _DEBUG
#pragma comment( lib, "freetype253MT_D.lib" )
#else
#pragma comment( lib, "freetype253MT.lib" )
#endif

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	// Start the game
	if( !CGame::instance().start() ) {
		CGame::instance().destroy();
		return -1;
	}
	CGame::instance().destroy();
	return 0;
}