#pragma once

class CInput
{
private:
	bool m_bKeyDown[SDL_NUM_SCANCODES];
	bool m_bKeyLocked[SDL_NUM_SCANCODES];
	bool m_bKeyLockQueue[SDL_NUM_SCANCODES];
public:
	CInput();
	~CInput();

	bool isKeyPress( unsigned short key );
	bool isKeyHeld( unsigned short key );

	void keyDown( unsigned short key );
	void keyUp( unsigned short key );

	void handleEvent( SDL_Event sdlEvent );
	void update();
};