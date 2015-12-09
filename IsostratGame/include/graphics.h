#pragma once

#include <GL\glew.h>
#include <SDL.h>

class CShaderManager;

class CGraphics
{
private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_GLContext;

	CShaderManager *m_pShaderManager;

	GLuint m_testVAO;
	GLuint m_testVBO;
public:
	CGraphics();
	~CGraphics();

	bool initialize();
	void destroy();

	void draw();
};