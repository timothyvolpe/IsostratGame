#include "base.h"
#include "input.h"

#include <SDL.h>

CInput::CInput() {
	for( unsigned short i = 0; i < SDL_NUM_SCANCODES; i++ ) {
		m_bKeyDown[i] = false;
		m_bKeyLocked[i] = false;
		m_bKeyLockQueue[i] = false;
	}
	m_mouseX = 0;
	m_mouseY = 0;
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
}
CInput::~CInput() {
}

bool CInput::isKeyPress( unsigned short key ) {
	return m_bKeyDown[key] && !m_bKeyLocked[key];
}
bool CInput::isKeyHeld( unsigned short key ) {
	return m_bKeyDown[key];
}

void CInput::keyDown( unsigned short key ) {
	m_bKeyDown[key] = true;
	if( !m_bKeyLocked[key] )
		m_bKeyLockQueue[key] = true;
}
void CInput::keyUp( unsigned short key ) {
	m_bKeyDown[key] = false;
	m_bKeyLocked[key] = false;
	m_bKeyLockQueue[key] = false;
}

void CInput::handleEvent( SDL_Event sdlEvent )
{
	switch( sdlEvent.type )
	{
	case SDL_KEYDOWN:
		this->keyDown( sdlEvent.key.keysym.scancode );
		break;
	case SDL_KEYUP:
		this->keyUp( sdlEvent.key.keysym.scancode );
		break;
	case SDL_MOUSEMOTION:
		m_mouseX = sdlEvent.motion.x;
		m_mouseY = sdlEvent.motion.y;
		m_mouseDeltaX = sdlEvent.motion.xrel;
		m_mouseDeltaY = sdlEvent.motion.yrel;
	default:
		break;
	}
}
void CInput::update() {
	for( unsigned short i = 0; i < SDL_NUM_SCANCODES; i++ ) {
		if( m_bKeyLockQueue[i] ) {
			m_bKeyLocked[i] = true;
			m_bKeyLockQueue[i] = false;
		}
	}
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
}

int CInput::getMouseX() {
	return m_mouseX;
}
int CInput::getMouseY() {
	return m_mouseY;
}
int CInput::getMouseDeltaX() {
	return m_mouseDeltaX;
}
int CInput::getMouseDeltaY() {
	return m_mouseDeltaY;
}