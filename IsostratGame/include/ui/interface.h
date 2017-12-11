#pragma once

#include <glm\glm.hpp>
#include <gl\glew.h>

#include <vector>

#define LAYER_SIZE 0.1f

#define INVALID_QUAD_POS ((unsigned int)0)-1

class CFontManager;
class CLocalization;
class CInterfaceBase;
class CInterfaceRenderable;

class CInterfaceScreen;
class CInterfaceLabel;

#pragma pack(push, 1)
typedef struct
{
	glm::vec2 relpos;
	glm::vec2 tex;
} InterfaceVertex;
#pragma pack(pop, 1)

enum : unsigned short
{
	INTERFACE_TYPE_UNKNOWN = 0,
	INTERFACE_TYPE_SCREEN = 1,
	INTERFACE_TYPE_LABEL
};

///////////////////////
// CInterfaceManager //
///////////////////////

class CInterfaceManager
{
private:
	typedef struct
	{
		GLuint quadCount;
		GLuint textureId;
		std::vector<InterfaceVertex> vertices;
		GLuint *pQuadIndex;
		GLuint *pQuadOffset;
	} QuadData;

	typedef std::vector<CInterfaceBase*> InterfaceList;
	typedef std::vector<CInterfaceRenderable*> InterfaceRenderableList;
	typedef std::vector<CInterfaceScreen*> ScreenList;

	CFontManager *m_pFontManager;
	CLocalization *m_pLocalization;

	int m_width, m_height;
	float m_horizDpi, m_vertDpi;

	std::vector<CInterfaceBase*> m_interfaceList;
	std::vector<CInterfaceRenderable*> m_interfaceRenderableList; // everything in this list is also in the interfacebase one

	GLuint m_interfaceVAO;
	GLuint m_interfaceVBO;
	std::vector<QuadData> m_quadData;
	GLuint m_oldQuadCount;
	GLuint m_quadCount;
	bool m_bQuadsInvalid;
	void reconstructQuadData();

	ScreenList m_uiScreens;
	bool loadScreens();
public:
	CInterfaceManager();
	~CInterfaceManager();

	bool initialize();
	void destroy();

	bool setLanguage( unsigned char language );

	void update();
	void draw( glm::mat4 projection, glm::mat4 view );

	void setDimensions( int width, int height );
	int getWidth();
	int getHeight();
	CFontManager* getFontManager();
	CLocalization* getLocalization();

	bool addQuads( std::vector<InterfaceVertex> vertices, GLuint textureId, GLuint *pQuadIndex, GLuint *pQuadOffset );
	bool updateQuads( std::vector<InterfaceVertex> vertices, GLuint quadIndex );
	bool removeQuads( GLuint quadIndex, GLuint count );

	void addScreen( CInterfaceScreen *pScreen );

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
		if( pObject ) {
			m_interfaceRenderableList.push_back( pObject );
			return pObject;
		}
		return 0;
	}
};

////////////////////
// CInterfaceBase //
////////////////////

class CInterfaceContainer;

class CInterfaceBase
{
private:
	glm::vec2 m_positionRel;
	glm::vec2 m_sizeRel;

	CInterfaceContainer *m_pParent;

	bool m_bVisible;
protected:
	int m_type;
	std::wstring m_text;
public:
	CInterfaceBase();
	virtual ~CInterfaceBase();

	bool initialize();
	void destroy();

	virtual bool onCreate() = 0;
	virtual void onDestroy() = 0;

	virtual void onUpdate();

	virtual bool onActivate() { return true; };

	virtual void onPositionChange() {}
	virtual void onResize() {}
	virtual void onVisibilityChange() {}
	virtual void onTextChange() {}

	virtual void onParentChange() {}

	void setRelativePosition( glm::vec2 posRel );
	glm::vec2 getRelativePosition();
	void setRelativeSize( glm::vec2 sizeRel );
	glm::vec2 getRelativeSize();
	bool isVisible();
	void setVisible( bool visible );
	void setText( std::wstring text );
	std::wstring getText();
	
	int getType();

	virtual bool isContainer() { return false; }
	virtual bool isRenderable() { return false; }

	friend class CInterfaceContainer;
};

/////////////////////////
// CInterfaceContainer //
/////////////////////////

class CInterfaceContainer : public CInterfaceBase
{
private:
	typedef std::vector<CInterfaceBase*> ChildrenList;

	ChildrenList m_children;
public:
	CInterfaceContainer();
	virtual ~CInterfaceContainer();

	bool onCreate();
	void onDestroy();

	bool addToContainer( CInterfaceBase *pControl );
	bool removeFromContainer( CInterfaceBase *pControl );
	void clearContainer();

	bool isContainer() { return true; }
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
	virtual ~CInterfaceRenderable();

	virtual void onDraw() = 0;

	virtual void onLayerChange() {}

	void setLayer( unsigned char layer );
	unsigned char getLayer();

	bool isRenderable() { return true; }
};