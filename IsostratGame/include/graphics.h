#pragma once

#include <SDL.h>

class CShaderManager;

class CGraphics
{
private:
	SDL_Window *m_pSDLWindow;
	SDL_GLContext m_GLContext;

	CShaderManager *m_pShaderManager;
public:
	CGraphics();
	~CGraphics();

	bool initialize();
	void destroy();

	void draw();
};