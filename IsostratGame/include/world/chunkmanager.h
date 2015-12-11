#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>

#define CHUNK_GRID_SIZE 0.25f
#define CHUNK_SIDE_LENGTH 2 // in grid squares
#define CHUNK_HEIGHT 1 // in grid squares
#define CHUNK_VERTEX_COUNT (CHUNK_SIDE_LENGTH+1)*(CHUNK_SIDE_LENGTH+1)*(CHUNK_HEIGHT+1)
#define CHUNK_INDEX_COUNT (CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT*36)

#define CHUNK_BATCH_SIZE 3*1024*1024 // max size of a single chunk vertex buffer

#pragma pack(push, 1)
typedef struct
{
	glm::vec3 pos;
	glm::vec3 color;
} ChunkVertex;
#pragma pack(pop, 1)

class CChunkManager
{
private:
	unsigned char m_chunkViewDistance;
	
	std::vector<GLuint> m_chunkVertexArrays;
	std::vector<GLuint> m_chunkVertexBuffers;
	std::vector<GLuint> m_chunkIndexBuffers;
	std::vector<GLuint> m_bufferChunkCounts;
	std::vector<GLuint> m_bufferIndexCounts;

	unsigned int m_chunkCount;

	bool generateMeshes();
	void destroyMeshes();
public:
	CChunkManager();
	~CChunkManager();

	bool initialize();
	void destroy();

	void draw( glm::mat4 projection, glm::mat4 view );

	unsigned int getChunkIndex( int x, int y );
};