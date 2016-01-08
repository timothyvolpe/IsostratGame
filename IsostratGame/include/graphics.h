#pragma once

#include <GL\glew.h>
#include <SDL.h>
#include <glm\glm.hpp>

class CShaderManager;
class CCamera;
class CWorld;
class CDebugRender;

class CGraphics
{
private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_GLContext;

	CShaderManager *m_pShaderManager;
	CWorld *m_pWorld;
	CDebugRender *m_pDebugRender;
	
	CCamera *m_pCamera;

	glm::mat4 m_projectionMatrix, m_orthoMatrix;
public:
	CGraphics();
	~CGraphics();

	bool initialize();
	void destroy();

	void draw();
	
	void calculateProjection( int width, int height, float fov, float zFar );

	glm::mat4 getProjectionMatrix();

	CShaderManager* getShaderManager();
	CWorld* getWorld();
	CDebugRender* getDebugRender();
	CCamera* getCamera();
};