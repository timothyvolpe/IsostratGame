#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "camera.h"
#include "input.h"
#include "config.h"
#include "graphics.h"
#include "debugrender.h"

/////////////
// CCamera //
/////////////

const glm::vec3 CCamera::CameraRight = glm::vec3( 1.0f, 0.0f, 0.0f );
const glm::vec3 CCamera::CameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );
const glm::vec3 CCamera::CameraForward = glm::vec3( 0.0f, 0.0f, -1.0f );

CCamera::CCamera()
{
	m_viewMatrix = glm::mat4( 1.0f );

	m_eyePosition = glm::vec3( 25.0f, 15.0f, 25.0f );

	m_cameraRight = CCamera::CameraRight;
	m_cameraUp = CCamera::CameraUp;
	m_cameraForward = CCamera::CameraForward;

	m_cameraPitch = 0.0f;
	m_cameraYaw = 0.0f;

	m_cameraSensitivity = 5.0f;
	m_cameraSpeedWalk = 1.0f;
	m_cameraSpeed = 3.0f;
	m_cameraSpeedRun = 7.0f;

	m_cameraAccelRate = 24.5f; // 24.5 m/s/s
	m_cameraAccelRateRun = 34.5f;
	m_cameraAccelRateWalk = 14.5f;
	m_cameraDeccelRate = 3.0f; // must be lower than accel
	m_cameraMoveSpeed = 0.0f;

	m_pFrustum = NULL;
}
CCamera::~CCamera() {
}

bool CCamera::initialize() {
	m_pFrustum = new CCameraFrustum();
	return true;
}
void CCamera::destroy() {
	SAFE_DELETE( m_pFrustum );
}

glm::mat4 CCamera::update()
{
	CInput *pInput = CGame::instance().getInput();
	CConfigLoader *pConfig = CGame::instance().getConfigLoader();
	double frameTime;
	int mouseX;
	int mouseY;

	bool movingForward = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_FORWARD ) );
	bool movingBackward = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK_BACKWARD ) );
	bool movingLeft = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_RIGHT ) );
	bool movingRight = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_STRAFE_LEFT ) );

	bool moving;
	bool walking = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_WALK ) );
	bool running = pInput->isKeyHeld( pConfig->getKeybind( KEYBIND_RUN ) );

	m_viewMatrix = glm::mat4( 1.0f );

	frameTime = CGame::instance().getFrameTime();
	mouseX = CGame::instance().getInput()->getMouseDeltaX();
	mouseY = CGame::instance().getInput()->getMouseDeltaY();

	// Update rotation
	// Crashes when these are too small........?

	m_cameraYaw += m_cameraSensitivity * mouseX * (float)frameTime;
	m_cameraPitch += m_cameraSensitivity * mouseY * (float)frameTime;

	// Clamp rotation
	if( m_cameraYaw > (2.0f*glm::pi<float>()) )
		m_cameraYaw -= (2.0f*glm::pi<float>());
	if( m_cameraYaw < 0.0f )
		m_cameraYaw += (2.0f*glm::pi<float>());
	m_cameraPitch = glm::clamp( m_cameraPitch, -(0.5f*glm::pi<float>()), (0.5f*glm::pi<float>()) );

	// Update matrices and vectors
	m_viewMatrix = glm::rotate( m_viewMatrix, m_cameraPitch, CCamera::CameraRight );
	m_viewMatrix = glm::rotate( m_viewMatrix, m_cameraYaw, CCamera::CameraUp );
	m_cameraForward = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraForward, 1.0f ) );
	m_cameraRight = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraRight, 1.0f ) );
	m_cameraUp = glm::vec3( glm::inverse( m_viewMatrix ) * glm::vec4( CCamera::CameraUp, 1.0f ) );

	// Calculate the move speed
	if( m_cameraMoveSpeed > 0.0f )
		m_cameraMoveSpeed -= m_cameraDeccelRate *(float)frameTime;
	if( m_cameraMoveSpeed < 0.0f )
		m_cameraMoveSpeed = 0.0f;

	// Determine if we're moving
	if( movingForward || movingBackward || movingLeft || movingRight )
		moving = true;
	else
		moving = false;

	if( moving )
	{
		if( walking )
			m_cameraMoveSpeed += m_cameraAccelRateWalk * (float)frameTime;
		else if( running )
			m_cameraMoveSpeed += m_cameraAccelRateRun * (float)frameTime;
		else
			m_cameraMoveSpeed += m_cameraAccelRate * (float)frameTime;

		if( walking )
			m_cameraMoveSpeed = glm::clamp( m_cameraMoveSpeed, 0.0f, m_cameraSpeedWalk );
		else if( running )
			m_cameraMoveSpeed = glm::clamp( m_cameraMoveSpeed, 0.0f, m_cameraSpeedRun );
		else
			m_cameraMoveSpeed = glm::clamp( m_cameraMoveSpeed, 0.0f, m_cameraSpeed );
	}

	// Move the camera
	if( movingForward )
		m_eyePosition += m_cameraForward * (m_cameraMoveSpeed * (float)frameTime);
	if( movingBackward )
		m_eyePosition += -m_cameraForward * (m_cameraMoveSpeed * (float)frameTime);
	if( movingLeft )
		m_eyePosition += m_cameraRight * (m_cameraMoveSpeed * (float)frameTime);
	if( movingRight )
		m_eyePosition += -m_cameraRight * (m_cameraMoveSpeed * (float)frameTime);
	
	m_viewMatrix = glm::translate( m_viewMatrix, -m_eyePosition );

	m_pFrustum->update();

	return m_viewMatrix;
}

glm::mat4 CCamera::getViewMatrix() {
	return m_viewMatrix;
}

glm::vec3 CCamera::getEyePosition() {
	return m_eyePosition;
}
glm::vec3 CCamera::getForward() {
	return m_cameraForward;
}

CCameraFrustum* CCamera::getFrustum() {
	return m_pFrustum;
}

////////////////////
// CCameraFrustum //
////////////////////

CCameraFrustum::CCameraFrustum() {
	m_position = glm::vec3( 0.0f, 0.0f, 0.0f );
	m_conversionMat = glm::mat4( 1.0f );
	m_nearZ = 0.0f;
	m_farZ = 0.0f;
	m_fov = 0.0f;
	m_ratio = 0.0f;
}
CCameraFrustum::~CCameraFrustum() {
}

void CCameraFrustum::update()
{
	CDebugRender *pDebugRender = CGame::instance().getGraphics()->getDebugRender();
	glm::vec4 coords;

	coords = glm::inverse( m_conversionMat ) * glm::vec4( -1, -1, 1, 1.0f );
	pDebugRender->drawLine( m_position, m_position + glm::vec3( coords / coords.w ), DEBUG_COLOR_FRUSTRUM );
	coords = glm::inverse( m_conversionMat ) * glm::vec4( -1, 1, 1, 1.0f );
	pDebugRender->drawLine( m_position, m_position + glm::vec3( coords / coords.w ), DEBUG_COLOR_FRUSTRUM );
	coords = glm::inverse( m_conversionMat ) * glm::vec4( 1, 1, 1, 1.0f );
	pDebugRender->drawLine( m_position, m_position + glm::vec3( coords / coords.w ), DEBUG_COLOR_FRUSTRUM );
	coords = glm::inverse( m_conversionMat ) * glm::vec4( 1, -1, 1, 1.0f );
	pDebugRender->drawLine( m_position, m_position + glm::vec3( coords / coords.w ), DEBUG_COLOR_FRUSTRUM );
}

void CCameraFrustum::setFrustum( glm::vec3 pos, glm::mat4 conversionMat, float nearZ, float farZ, float fov, float ratio ) {
	m_position = pos;
	m_conversionMat = conversionMat;
	m_nearZ = nearZ;
	m_farZ = farZ;
	m_fov = fov;
	m_ratio = ratio;
}