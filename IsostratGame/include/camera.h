#pragma once

#include <glm\glm.hpp>

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
public:
	CCamera();
	~CCamera();

	bool initialize();
	void destroy();

	glm::mat4 update();
};