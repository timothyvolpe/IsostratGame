#pragma once

#include <glm\glm.hpp>
#include <gl\glew.h>

class CFontManager;
class CLocalization;
class CInterfaceBase;

#pragma pack(push, 1)
typedef struct
{
	glm::vec2 relpos;
} InterfaceVertex;
#pragma pack(pop, 1)

///////////////////////
// CInterfaceManager //
///////////////////////

class CInterfaceManager
{
private:
	CFontManager *m_pFontManager;
	CLocalization *m_pLocalization;

	CInterfaceBase *m_pTestScreen;

	int m_width, m_height;
	bool m_bUpdateDim;

	GLuint m_interfaceVAO;
	GLuint m_interfaceVBO;
public:
	CInterfaceManager();
	~CInterfaceManager();

	bool initialize();
	void destroy();

	bool setLanguage( unsigned char language );

	void draw( glm::mat4 projection, glm::mat4 view );

	void setDimensions( int width, int height );
	CFontManager* getFontManager();
	CLocalization* getLocalization();
};

////////////////////
// CInterfaceBase //
////////////////////

class CInterfaceBase
{
public:
	CInterfaceBase();
	~CInterfaceBase();

	bool initialize();
	void destroy();

	virtual bool onCreate() = 0;
	virtual void onDestroy() = 0;
};

//////////////////////////
// CInterfaceRenderable //
//////////////////////////

class CInterfaceRenderable
{
private:
	GLuint m_vertexStart;
public:
	CInterfaceRenderable();
	~CInterfaceRenderable();
};