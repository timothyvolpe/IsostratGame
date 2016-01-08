#pragma once

#include <glm\glm.hpp>

class CCameraFrustum;

/////////////
// CCamera //
/////////////

class CCamera
{
public:
	static const glm::vec3 CameraRight, CameraUp, CameraForward;
private:
	glm::mat4 m_viewMatrix;

	glm::vec3 m_eyePosition;
	glm::vec3 m_cameraRight, m_cameraUp, m_cameraForward;

	float m_cameraPitch, m_cameraYaw;

	float m_cameraSensitivity;
	float m_cameraSpeed, m_cameraSpeedRun, m_cameraSpeedWalk; // units per second

	glm::vec3 m_eyeVector;

	CCameraFrustum *m_pFrustum;
public:
	CCamera();
	~CCamera();

	bool initialize();
	void destroy();

	glm::mat4 update();

	glm::mat4 getViewMatrix();

	glm::vec3 getEyePosition();
	glm::vec3 getForward();

	CCameraFrustum* getFrustum();
};

////////////////////
// CCameraFrustum //
////////////////////

class CCameraFrustum
{
private:
	glm::vec3 m_position;
	glm::mat4 m_conversionMat;

	float m_nearZ, m_farZ;
	float m_fov;
	float m_ratio;
public:
	CCameraFrustum();
	~CCameraFrustum();

	void update();

	void setFrustum( glm::vec3 pos, glm::mat4 conversionMat, float nearZ, float farZ, float fov, float ratio );
};