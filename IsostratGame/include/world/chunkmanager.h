#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <map>

#define CHUNK_GRID_SIZE 0.25f
#define CHUNK_SIDE_LENGTH 16 // in grid squares
#define CHUNK_HEIGHT 32 // in grid squares
#define CHUNK_BLOCK_COUNT CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT
#define CHUNK_VERTEX_COUNT CHUNK_BLOCK_COUNT*36 //(CHUNK_SIDE_LENGTH+1)*(CHUNK_SIDE_LENGTH+1)*(CHUNK_HEIGHT+1)
#define CHUNK_INDEX_COUNT (CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH*CHUNK_HEIGHT*36)

#define CHUNK_BATCH_SIZE 6*1024*1024 // max size of a single chunk vertex buffer

#define TERRAIN_VERSION_A 0
#define TERRAIN_VERSION_B 3

#define TERRAIN_REGION_SIDE 32

typedef unsigned char ChunkSide;
typedef unsigned char ChunkHeight;

#pragma pack(push, 1)
typedef struct
{
	unsigned char x, y, z, w;
	unsigned char r, g, b, a;
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

ChunkVertex GenVertex( glm::ivec3 pos, glm::ivec3 color );

class CChunk;
class CBlock;

enum : char
{
	CHUNK_DIRECTION_UP,
	CHUNK_DIRECTION_DOWN,
	CHUNK_DIRECTION_FRONT,
	CHUNK_DIRECTION_BACK,
	CHUNK_DIRECTION_LEFT,
	CHUNK_DIRECTION_RIGHT
};

///////////////////
// CChunkManager //
///////////////////

class CChunkManager
{
private:
	typedef std::map<unsigned short, CBlock*> BlockIdList;
	typedef std::vector<std::vector<CChunk*>> ChunkVector;

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
	ChunkVector m_activeChunks;

	glm::vec2 m_eyePos;
	glm::ivec2 m_renderPos;

	BlockIdList m_blockClasses;

	unsigned int m_chunkCount;

	bool m_bUpdateScale;

	bool allocateChunks( unsigned char viewDistance );
	void destroyChunks();

	bool generateMeshes();
	void destroyMeshes();

	void sortChunkList();

	unsigned short* readRawChunkData( glm::ivec2 pos );
public:
	CBlock *m_pBlockGrass;
	CBlock *m_pBlockStone;

	CChunkManager();
	~CChunkManager();

	bool initialize();
	void destroy();

	void draw( glm::mat4 projection, glm::mat4 view );

	unsigned int getChunkIndex( int x, int y );

	bool openTerrainFile( std::string path );
	bool saveTerrainFile( std::string path );
	void closeTerrainFile();

	void registerBlock( CBlock* pBlock );
	CBlock* getBlockById( unsigned short id );

	CChunk* getChunkNeighbor( glm::ivec2 vectorPos, char direction );
};

////////////
// CChunk //
////////////

class CChunk
{
private:
	typedef std::vector<CBlock*> BlockList;

	glm::ivec2 m_vectorPos;

	BlockList m_blocks;

	size_t m_bufferIndex;
	GLuint m_vertexOffset, m_indexOffset;
	GLuint m_vertexCount, m_indexCount;
public:
	CChunk();
	~CChunk();

	void setRawData( unsigned short *pData );

	void setBufferPosition( glm::ivec2 vectorPos, size_t bufferIndex, GLuint vertexOffset, GLuint indexOffset );

	bool populateVertices();
	bool populateIndices();

	bool initialize();
	void destroy();

	bool isBlockVisible( glm::vec3 pos );
	bool isBlockVisible( size_t index );

	size_t getBlockIndex( glm::vec3 pos );
	CBlock* getBlockNeighbor( glm::vec3 pos, char direction );
	CBlock* getBlockNeighbor( size_t index, char direction );
	CBlock* getBlockAt( size_t index );
	CBlock* getBlockAt( glm::vec3 pos );
	GLuint getIndexCount();

	glm::ivec2 getChunkVectorPos();
	size_t getBufferIndex();
	GLuint getVertexOffset();
	GLuint getVertexCount();
};

struct ChunkComparator {
	bool operator() ( CChunk* pOne, CChunk* pTwo ) { return (pOne->getBufferIndex() < pTwo->getBufferIndex()); }
};