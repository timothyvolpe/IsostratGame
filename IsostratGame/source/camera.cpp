#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "camera.h"
#include "input.h"
#include "config.h"

CCamera::CCamera()
{
	m_viewMatrix = glm::mat4( 1.0f );
	m_position = glm::vec3( 0.0f, 0.0f, -2.0f );

	m_cameraSpeed = 3.0f;
}
CCamera::~CCamera() {
}

bool CCamera::initialize() {
	return true;
}
void CCamera::destroy() {
}

glm::mat4 CCamera::update()
{
	CInput *pInput = CGame::instance().getInput();
	CConfigLoader *pConfig = CGame::instance().getConfigLoader();
	double frameTime = CGame::instance().getFrameTime();

	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_FORWARD ) ) ) {
		m_position.z += (float)frameTime*m_cameraSpeed;
	}
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_BACKWARD ) ) ) {
		m_position.z -= (float)frameTime*m_cameraSpeed;
	}
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_LEFT ) ) ) {
		m_position.x += (float)frameTime*m_cameraSpeed;
	}
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_RIGHT ) ) ) {
		m_position.x -= (float)frameTime*m_cameraSpeed;
	}

	m_viewMatrix = glm::translate( glm::mat4( 1.0f ), m_position );

	return m_viewMatrix;
}