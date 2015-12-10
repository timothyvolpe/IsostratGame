#pragma once

#include <glm\glm.hpp>

class CCamera
{
private:
	glm::mat4 m_viewMatrix;

	glm::vec3 m_position;

	float m_cameraSpeed; // units per second
public:
	CCamera();
	~CCamera();

	bool initialize();
	void destroy();

	glm::mat4 update();
};