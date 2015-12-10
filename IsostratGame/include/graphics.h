#pragma once

#include <GL\glew.h>
#include <SDL.h>
#include <glm\glm.hpp>

class CShaderManager;

class CGraphics
{
private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_GLContext;

	CShaderManager *m_pShaderManager;

	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewMatrix;

	GLuint m_testVAO;
	GLuint m_testVBO;
public:
	CGraphics();
	~CGraphics();

	bool initialize();
	void destroy();

	void draw();
	
	void calculateProjection( int width, int height, float fov, float zFar );

	glm::mat4 getProjectionMatrix();
};