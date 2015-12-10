#include "base.h"
#include "input.h"

#include <SDL.h>

CInput::CInput() {
	for( unsigned short i = 0; i < SDL_NUM_SCANCODES; i++ ) {
		m_bKeyDown[i] = false;
		m_bKeyLocked[i] = false;
		m_bKeyLockQueue[i] = false;
	}
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
}