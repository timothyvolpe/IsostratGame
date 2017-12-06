#include "base.h"
#include "def.h"
#include "platform.h"
#include "game.h"

#include <GL\glew.h>

//int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
int main( int argc, char *argv[] )
{
	// Start the game
	if( !CGame::instance().start() ) {
		CGame::instance().destroy();
		return -1;
	}
	CGame::instance().destroy();
	return 0;
}