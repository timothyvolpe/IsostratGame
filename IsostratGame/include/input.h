#pragma once

class CInput
{
private:
	bool m_bKeyDown[SDL_NUM_SCANCODES];
	bool m_bKeyLocked[SDL_NUM_SCANCODES];
	bool m_bKeyLockQueue[SDL_NUM_SCANCODES];

	int m_mouseX, m_mouseY;
	int m_mouseDeltaX, m_mouseDeltaY;
public:
	CInput();
	~CInput();

	bool isKeyPress( unsigned short key );
	bool isKeyHeld( unsigned short key );

	void keyDown( unsigned short key );
	void keyUp( unsigned short key );

	void handleEvent( SDL_Event sdlEvent );
	void update();

	int getMouseX();
	int getMouseY();
	int getMouseDeltaX();
	int getMouseDeltaY();
};