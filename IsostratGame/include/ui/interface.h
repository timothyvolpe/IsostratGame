#pragma once

#include <glm\glm.hpp>
#include <gl\glew.h>

#include <vector>

#define LAYER_SIZE 0.1f

class CFontManager;
class CLocalization;
class CInterfaceBase;
class CInterfaceRenderable;

#pragma pack(push, 1)
typedef struct
{
	glm::vec2 relpos;
	glm::vec2 tex;
} InterfaceVertex;
#pragma pack(pop, 1)

///////////////////////
// CInterfaceManager //
///////////////////////

class CInterfaceManager
{
private:
	typedef std::vector<CInterfaceBase*> InterfaceList;
	typedef std::vector<CInterfaceRenderable*> InterfaceRenderableList;

	CFontManager *m_pFontManager;
	CLocalization *m_pLocalization;

	int m_width, m_height;

	std::vector<CInterfaceBase*> m_interfaceList;
	std::vector<CInterfaceRenderable*> m_interfaceRenderableList; // everything in this list is also in the interfacebase one

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

	// Creates an interface object
	template<class T>
	T* createInterfaceObject()
	{
		T* pObject = new T();
		if( !pObject->initialize() || !pObject->onCreate() ) {
			delete pObject;
			return NULL;
		}
		m_interfaceList.push_back( pObject );
		return pObject;
	}
	// Creates a renderable interface object
	template<class T>
	T* createInterfaceObjectRenderable()
	{
		T* pObject = this->createInterfaceObject<T>();
		if( pObject )
			m_interfaceRenderableList.push_back( pObject );
		return pObject;
	}
};

////////////////////
// CInterfaceBase //
////////////////////

class CInterfaceBase
{
private:
	glm::vec2 m_positionRel;
	glm::vec2 m_sizeRel;
public:
	CInterfaceBase();
	~CInterfaceBase();

	bool initialize();
	void destroy();

	virtual bool onCreate() = 0;
	virtual void onDestroy() = 0;

	virtual void onPositionChange() {}
	virtual void onResize() {}

	void setRelativePosition( glm::vec2 posRel );
	glm::vec2 getRelativePosition();
	void setRelativeSize( glm::vec2 sizeRel );
	glm::vec2 getRelativeSize();
};

//////////////////////////
// CInterfaceRenderable //
//////////////////////////

class CInterfaceRenderable : public CInterfaceBase
{
private:
	unsigned char m_layer;
public:
	CInterfaceRenderable();
	~CInterfaceRenderable();

	virtual void onLayerChange() {}

	void setLayer( unsigned char layer );
	unsigned char getLayer();
};