#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>

#define CHUNK_GRID_SIZE 0.25f
#define CHUNK_SIDE_LENGTH 16 // in grid squares
#define CHUNK_HEIGHT 4 // in grid squares
#define CHUNK_BLOCK_COUNT CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT
#define CHUNK_VERTEX_COUNT (CHUNK_SIDE_LENGTH+1)*(CHUNK_SIDE_LENGTH+1)*(CHUNK_HEIGHT+1)
#define CHUNK_INDEX_COUNT (CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT*36)

#define CHUNK_BATCH_SIZE 3*1024*1024 // max size of a single chunk vertex buffer

#pragma pack(push, 1)
typedef struct
{
	glm::ivec3 pos;
	glm::vec3 color;
} ChunkVertex;
#pragma pack(pop, 1)

class CChunk;
class CBlock;

///////////////////
// CChunkManager //
///////////////////

class CChunkManager
{
private:
	unsigned char m_chunkViewDistance;
	
	std::vector<GLuint> m_chunkVertexArrays;
	std::vector<GLuint> m_chunkVertexBuffers;
	std::vector<GLuint> m_chunkIndexBuffers;
	std::vector<GLuint> m_bufferChunkCounts;
	std::vector<GLuint> m_bufferIndexCounts;

	std::vector<CChunk*> m_chunks;

	unsigned int m_chunkCount;

	bool m_bUpdateScale;

	bool allocateChunks( unsigned char viewDistance );
	void destroyChunks();

	bool generateMeshes();
	void destroyMeshes();
public:
	CChunkManager();
	~CChunkManager();

	bool initialize();
	void destroy();

	void draw( glm::mat4 projection, glm::mat4 view );

	unsigned int getChunkIndex( int x, int y );

	bool openTerrainFile( std::string path );
	bool saveTerrainFile( std::string path );
	void closeTerrainFile();
};

////////////
// CChunk //
////////////

class CChunk
{
private:
	std::vector<CBlock*> m_blocks;

	int m_bufferIndex;
	GLuint m_vertexOffset, m_indexOffset;
public:
	CChunk();
	~CChunk();

	void setBufferPosition( int bufferIndex, GLuint vertexOffset, GLuint indexOffset );

	bool populateVertices();
	bool populateIndices();

	bool initialize();
	void destroy();

	CBlock* getBlockAt( glm::vec3 pos );
};