#pragma once

#include "ui\interface.h"

class CInterfaceLabel : public CInterfaceRenderable
{
private:
	GLuint m_vertexCount;
	GLuint *m_pQuadPosition, *m_pQuadOffset;

	bool m_bUpdateText;

	void rebuildTextQuads();
	void destroyTextQuads();
public:
	CInterfaceLabel();
	~CInterfaceLabel();

	bool onCreate();
	void onDestroy();

	void onUpdate();

	bool onActivate();

	void onDraw();

	void onTextChange();
};