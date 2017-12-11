#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>

#define DEBUG_COLOR_CHUNK_INACTIVE glm::vec3( 0.6f, 0.83921568627f, 1.0f ) // 153, 214, 255
#define DEBUG_COLOR_CHUNK_ACTIVE glm::vec3( 0.0f,  0.59607843137f, 1.0f ) // 0, 152, 255
#define DEBUG_COLOR_FRUSTRUM glm::vec3( 1.0f, 0.74901960784f, 0.0f ) // 255, 191, 0

#pragma pack( push, 1 )
typedef struct 
{
	glm::vec3 pos;
	glm::vec3 col;
} DebugVertex;
#pragma pack( pop )

class CDebugRender
{
private:
	GLuint m_vertexArrayId;
	GLuint m_vertexBufferId;

	bool m_bEnabled;

	std::vector<DebugVertex> m_vertices;
public:
	CDebugRender();
	~CDebugRender();

	bool initialize();
	void destroy();

	void update();
	void draw( glm::mat4 projection, glm::mat4 view );

	void drawLine( glm::vec3 p1, glm::vec3 p2, glm::vec3 color );
	void drawRect3d( glm::vec3 p1, glm::vec3 dim, glm::vec3 color );
};