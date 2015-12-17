#pragma once

#include "ui\interface.h"

class CInterfaceLabel : public CInterfaceRenderable
{
private:
	GLuint m_vertexCount;
	GLuint *m_pQuadPosition, *m_pQuadOffset;

	std::wstring m_text;
	bool m_bUpdateText;

	void rebuildTextQuads();
public:
	CInterfaceLabel();
	~CInterfaceLabel();

	bool onCreate();
	void onDestroy();

	bool onActivate();

	void onDraw();

	void setText( std::wstring text );
	std::wstring getText();
};