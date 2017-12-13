#pragma once

#include <GL\glew.h>
#include <vector>
#include <glm\glm.hpp>
#include <iostream>
#include <map>
#include <boost\thread.hpp>
#include <boost\atomic.hpp>
#include <boost\filesystem.hpp>

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
	int offset[TERRAIN_REGION_SIDE][TERRAIN_REGION_SIDE];
} TerrainOffsetTable;
#pragma pack(pop, 1)

ChunkVertex GenVertex( glm::ivec3 pos, glm::ivec3 color );

class CChunk;
class CBlock;
class CChunkLoader;

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

	boost::filesystem::path m_savePath;
	bool m_bValidSave;
	CChunkLoader *m_pChunkLoader;

	// A vector of bools that are true if there is
	// currently a chunk stored at that part of the
	// buffer
	// [bufferIndex][chunkIndex]
	std::vector<std::vector<bool>> m_hasChunkInBuffer;
	// A vector containing all the chunks to be rendered
	std::vector<CChunk*> m_chunks;
	// A vector containing all the chunks to be rendered,
	// but in grid order [x][y]
	ChunkVector m_chunkGrid;

	glm::ivec2 m_renderPos; // the chunk the eye is in

	BlockIdList m_blockClasses;

	unsigned int m_chunkCount;

	bool m_bUpdateScale;

	bool allocateChunks( unsigned char viewDistance );
	void destroyChunks();

	bool generateMeshes();
	void destroyMeshes();
public:
	boost::shared_mutex m_mutex_;

	CBlock *m_pBlockGrass;
	CBlock *m_pBlockStone;

	CChunkManager();
	~CChunkManager();

	bool initialize();
	void destroy();

	void update();
	void draw( glm::mat4 projection, glm::mat4 view );

	unsigned int getChunkIndex( int x, int y );

	bool setSaveFile( std::wstring saveName );

	void registerBlock( CBlock* pBlock );
	CBlock* getBlockById( unsigned short id );

	CChunk* getChunkNeighbor( glm::ivec2 vectorPos, char direction );

	void getBufferIds( int bufferIndex, unsigned int *pVertexArrayId, unsigned int *pVertexBufferId, unsigned int *pIndexBufferId );
	// Get the chunk that the camera eye is in
	glm::ivec2 getEyeChunk();
};

//////////////////
// CChunkLoader //
//////////////////

struct LoadedChunk
{
	glm::ivec2 position;
	CChunk *pChunk;
};

class CChunkLoader
{
private:
	boost::filesystem::path m_savePath;

	boost::atomic<bool> m_running_;
	boost::atomic<bool> m_chunksToLoad_;

	std::deque<glm::ivec2> m_chunkQueue_;
	std::deque<LoadedChunk> m_finishedChunkQueue_;

	boost::thread m_chunkLoaderThread;

	void threadStart();
	void loadQueue();

	void addFinishedChunk( glm::ivec2 position, CChunk *pChunk );

	bool populateChunkDataFromFile( glm::ivec2 position, CChunk *pChunk );
public:
	boost::shared_mutex m_mutex_;
	boost::shared_mutex m_finishedMutex_;
	boost::condition_variable_any m_loadChunks_;

	CChunkLoader();
	~CChunkLoader();

	bool initialize();
	void destroy();
	
	void addChunkToQueue( glm::ivec2 absolutePosition );
	void clearQueue();
	glm::ivec2 popChunkFromQueue( bool *pSuccess );
	int getQueueSize();

	void sendChunkQueue();

	LoadedChunk popFinishedChunk();
	int getFinishedQueueSize();

	void setSavePath( boost::filesystem::path savePath );
};

////////////
// CChunk //
////////////

class CChunk
{
private:
	typedef std::vector<CBlock*> BlockList;

	glm::ivec2 m_chunkPos, m_chunkGridPos;

	BlockList m_blocks;

	size_t m_bufferIndex;
	GLuint m_vertexOffset, m_indexOffset;
	GLuint m_vertexCount, m_indexCount;

	ChunkVertex *m_pVertices;
	unsigned int *m_pIndices;
public:
	boost::shared_mutex m_mutex_;

	static bool isOccludingBlock( CBlock* pBlock );

	CChunk();
	~CChunk();

	void setRawData( unsigned short *pData );

	void setBufferPosition( size_t bufferIndex, GLuint vertexOffset, GLuint indexOffset );

	// These might not be thread safe! If the chunk grid vector changes
	// there could be issues!
	bool generateVertices();
	bool generateIndices();
	bool sendDataToBuffers();

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

	void setChunkPos( glm::ivec2 pos ) { m_chunkPos = pos; }
	glm::ivec2 getChunkPos() { return m_chunkPos; }
	void setChunkGridPos( glm::ivec2 pos ) { m_chunkGridPos = pos; }
	glm::ivec2 getChunkGridPos() { return m_chunkGridPos; }
	size_t getBufferIndex();
	GLuint getVertexOffset();
	GLuint getVertexCount();
};

struct ChunkComparator {
	bool operator() ( CChunk* pOne, CChunk* pTwo ) { return (pOne->getBufferIndex() < pTwo->getBufferIndex()); }
};