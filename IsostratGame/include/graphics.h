#pragma once

#include <GL\glew.h>
#include <SDL.h>
#include <glm\glm.hpp>

class CShaderManager;
class CCamera;
class CWorld;
class CDebugRender;

enum
{
	WIREFRAME_MODE_UNSET = -1,
	WIREFRAME_MODE_SOLID = 0,
	WIREFRAME_MODE_WIRE,
	WIREFRAME_MODE_WIREUI,
	WIREFRAME_MODE_COUNT
};

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
	glm::mat4 m_currentViewMat, m_currentOrthoViewMat;
	float m_nearZ, m_farZ;
	float m_fov;
	float m_ratio;

	int m_wireframeMode;
public:
	CGraphics();
	~CGraphics();

	bool initialize();
	void destroy();

	void update();
	void draw();
	
	void calculateProjection( int width, int height, float fov, float zFar );

	glm::mat4 getProjectionMatrix();

	CShaderManager* getShaderManager();
	CWorld* getWorld();
	CDebugRender* getDebugRender();
	CCamera* getCamera();
	inline int getWireframeMode() { return m_wireframeMode; }
	void setWireframeMode( int mode );
};