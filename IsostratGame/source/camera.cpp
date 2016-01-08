#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "camera.h"
#include "input.h"
#include "config.h"

const glm::vec3 CCamera::CameraRight = glm::vec3( 1.0f, 0.0f, 0.0f );
const glm::vec3 CCamera::CameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );
const glm::vec3 CCamera::CameraForward = glm::vec3( 0.0f, 0.0f, -1.0f );

CCamera::CCamera()
{
	m_viewMatrix = glm::mat4( 1.0f );

	m_eyePosition = glm::vec3( 0.0f, 05.0f, 0.0f );

	m_cameraRight = CCamera::CameraRight;
	m_cameraUp = CCamera::CameraUp;
	m_cameraForward = CCamera::CameraForward;

	m_cameraPitch = 0.0f;
	m_cameraYaw = 0.0f;

	m_cameraSensitivity = 0.5f;
	m_cameraSpeedWalk = 1.0f;
	m_cameraSpeed = 3.0f;
	m_cameraSpeedRun = 7.0f;
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
	double frameTime;
	float moveSpeed;
	int mouseX;
	int mouseY;

	m_viewMatrix = glm::mat4( 1.0f );

	frameTime = CGame::instance().getFrameTime();
	mouseX = CGame::instance().getInput()->getMouseDeltaX();
	mouseY = CGame::instance().getInput()->getMouseDeltaY();

	// Update rotation
	m_cameraYaw += m_cameraSensitivity * mouseX;
	m_cameraPitch += m_cameraSensitivity * mouseY;

	// Clamp rotation
	if( m_cameraPitch > 90.0f )
		m_cameraPitch = 90.0f;
	if( m_cameraPitch < -90.0f )
		m_cameraPitch = -90.0f;
	if( m_cameraYaw > 360.0f )
		m_cameraYaw -= 360.0f;
	if( m_cameraPitch < -360.0f )
		m_cameraPitch += 360.0f;

	// Update matrices and vectors
	m_viewMatrix = glm::rotate( m_viewMatrix, m_cameraPitch, CCamera::CameraRight );
	m_viewMatrix = glm::rotate( m_viewMatrix, m_cameraYaw, CCamera::CameraUp );
	m_cameraForward = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraForward, 1.0f ) );
	m_cameraRight = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraRight, 1.0f ) );
	m_cameraUp = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraUp, 1.0f ) );

	// Calculate the move speed
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK ) ) )
		moveSpeed = (float)frameTime*m_cameraSpeedWalk;
	else if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_RUN ) ) )
		moveSpeed = (float)frameTime*m_cameraSpeedRun;
	else
		moveSpeed = (float)frameTime*m_cameraSpeed;

	// Move the camera
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_FORWARD ) ) )
		m_eyePosition += m_cameraForward * moveSpeed;
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_BACKWARD ) ) )
		m_eyePosition += -m_cameraForward * moveSpeed;
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_RIGHT ) ) )
		m_eyePosition += m_cameraRight * moveSpeed;
	if( pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_LEFT ) ) )
		m_eyePosition += -m_cameraRight * moveSpeed;
	

	m_viewMatrix = glm::translate( m_viewMatrix, -m_eyePosition );

	return m_viewMatrix;
}

glm::mat4 CCamera::getViewMatrix() {
	return m_viewMatrix;
}

glm::vec3 CCamera::getEyePosition() {
	return m_eyePosition;
}