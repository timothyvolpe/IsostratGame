#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>

#define CHUNK_GRID_SIZE 0.25f
#define CHUNK_SIDE_LENGTH 16 // in grid squares
#define CHUNK_HEIGHT 16 // in grid squares
#define CHUNK_BLOCK_COUNT CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT
#define CHUNK_VERTEX_COUNT (CHUNK_SIDE_LENGTH+1)*(CHUNK_SIDE_LENGTH+1)*(CHUNK_HEIGHT+1)
#define CHUNK_INDEX_COUNT (CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT*36)

#define CHUNK_BATCH_SIZE 3*1024*1024 // max size of a single chunk vertex buffer

#define TERRAIN_VERSION_A 0
#define TERRAIN_VERSION_B 3

#define TERRAIN_REGION_SIDE 32

typedef unsigned char ChunkSide;
typedef unsigned char ChunkHeight;

#pragma pack(push, 1)
typedef struct
{
	glm::ivec3 pos;
	glm::vec3 color;
} ChunkVertex;

typedef struct
{
	unsigned char version_a, version_b;
	ChunkSide chunkSide;
	ChunkHeight chunkHeight;
} TerrainSaveHeader;

typedef struct
{
	int x[TERRAIN_REGION_SIDE*TERRAIN_REGION_SIDE];
	int y[TERRAIN_REGION_SIDE*TERRAIN_REGION_SIDE];
} TerrainPositionTable;
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

	std::fstream m_chunkStream;
	size_t m_chunkDataSize;
	int m_chunkOffsets[TERRAIN_REGION_SIDE][TERRAIN_REGION_SIDE];

	std::vector<CChunk*> m_chunks;
	glm::ivec2 m_renderPos;

	unsigned int m_chunkCount;

	bool m_bUpdateScale;

	bool allocateChunks( unsigned char viewDistance );
	void destroyChunks();

	bool generateMeshes();
	void destroyMeshes();

	unsigned short* readRawChunkData( glm::ivec2 pos );
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
	typedef std::vector<bool> BlockList;

	BlockList m_blocks;

	int m_bufferIndex;
	GLuint m_vertexOffset, m_indexOffset;
	GLuint m_vertexCount, m_indexCount;
public:
	CChunk();
	~CChunk();

	void setRawData( unsigned short *pData );

	void setBufferPosition( int bufferIndex, GLuint vertexOffset, GLuint indexOffset );

	bool populateVertices();
	bool populateIndices();

	bool initialize();
	void destroy();

	CBlock* getBlockAt( glm::vec3 pos );
	GLuint getIndexCount();
};