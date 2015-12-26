#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "world\world.h"
#include "world\chunkmanager.h"
#include "shader\shaderbase.h"
#include "graphics.h"

#include "world\block.h"

ChunkVertex GenVertex( glm::ivec3 pos, glm::ivec3 color )
{
	ChunkVertex vertex;
	vertex.x = (unsigned char)pos.x;
	vertex.y = (unsigned char)pos.y;
	vertex.z = (unsigned char)pos.z;
	vertex.w = 1;
	vertex.r = (unsigned char)color.r;
	vertex.g = (unsigned char)color.g;
	vertex.b = (unsigned char)color.b;
	vertex.a = 1;
	return vertex;
}

///////////////////
// CChunkManager //
///////////////////

CChunkManager::CChunkManager()
{
	m_chunkViewDistance = 0;
	m_chunkCount = 0;
	m_bUpdateScale = true;
	m_chunkDataSize = 0;

	m_renderPos = glm::ivec2( 13, 13 );
}
CChunkManager::~CChunkManager() {
}

// Generates all the chunk meshes based on the loaded data
bool CChunkManager::generateMeshes()
{
	int chunkSize;
	int bufferCount;
	unsigned int chunksPerBuffer;
	size_t currentChunk;

	// Determine how many buffers we need
	chunkSize = CHUNK_VERTEX_COUNT*sizeof( ChunkVertex );
	chunksPerBuffer = (int)floor( (double)CHUNK_BATCH_SIZE / (double)chunkSize );
	bufferCount = (int)ceil( (double)m_chunkCount / (double)chunksPerBuffer );
	if( bufferCount < 0 ) {
		PrintError( L"Chunk manager is generating nothing!\n" );
		return false;
	}

	// Allocate the arrays and buffers
	m_chunkVertexArrays.resize( bufferCount );
	m_chunkVertexBuffers.resize( bufferCount );
	m_chunkIndexBuffers.resize( bufferCount );
	glGenVertexArrays( bufferCount, &m_chunkVertexArrays[0] );
	glGenBuffers( bufferCount, &m_chunkVertexBuffers[0] );
	glGenBuffers( bufferCount, &m_chunkIndexBuffers[0] );

	// Fill each buffer with blank data to allocate space
	for( size_t i = 0; i < m_chunkVertexArrays.size(); i++ )
	{
		// Calculate chunks and indices in the buffer
		if( i == bufferCount - 1 ) {
			m_bufferChunkCounts.push_back( m_chunkCount - i*chunksPerBuffer );
			m_bufferIndexCounts.push_back( m_bufferChunkCounts[i] * CHUNK_INDEX_COUNT );
		}
		else {
			m_bufferChunkCounts.push_back( chunksPerBuffer );
			m_bufferIndexCounts.push_back( chunksPerBuffer * CHUNK_INDEX_COUNT );
		}

		glBindVertexArray( m_chunkVertexArrays[i] );
		// Fill the vertex buffer
		glBindBuffer( GL_ARRAY_BUFFER, m_chunkVertexBuffers[i] );
		glBufferData( GL_ARRAY_BUFFER, sizeof( ChunkVertex )*CHUNK_VERTEX_COUNT*m_bufferChunkCounts[i], NULL, GL_DYNAMIC_DRAW );
		glVertexAttribIPointer( 0, 4, GL_UNSIGNED_BYTE, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, x ) );
		glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, r ) );
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		// Fill the index buffer
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_chunkIndexBuffers[i] );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int )*CHUNK_INDEX_COUNT*m_bufferChunkCounts[i], NULL, GL_DYNAMIC_DRAW );
	}

	// Fill the mesh for each chunk
	currentChunk = 0;
	for( int i = 0; i < bufferCount; i++ )
	{
		glBindVertexArray( m_chunkVertexArrays[i] );
		// Do all vertex data for this buffer
		glBindBuffer( GL_ARRAY_BUFFER, m_chunkVertexBuffers[i] );
		for( GLuint chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ ) {
			// Set the buffer position
			m_chunks[currentChunk]->setBufferPosition( i, chunk*CHUNK_VERTEX_COUNT, chunk*CHUNK_INDEX_COUNT );
			if( !m_chunks[currentChunk]->populateVertices() )
				return false;
			currentChunk++;
		}
		currentChunk-=m_bufferChunkCounts[i];
		// Do all index data for this buffer
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_chunkIndexBuffers[i] );
		for( GLuint chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ ) {
			if( !m_chunks[currentChunk]->populateIndices() )
				return false;
			currentChunk++;
		}
	}

	return true;
}
void CChunkManager::destroyMeshes()
{
	if( m_chunkVertexArrays.size() > 0 )
		glDeleteVertexArrays( m_chunkVertexArrays.size(), &m_chunkVertexArrays[0] );
	if( m_chunkVertexBuffers.size() > 0 )
		glDeleteBuffers( m_chunkVertexBuffers.size(), &m_chunkVertexBuffers[0] );
	if( m_chunkIndexBuffers.size() > 0 )
		glDeleteBuffers( m_chunkIndexBuffers.size(), &m_chunkIndexBuffers[0] );
	m_chunkVertexArrays.clear();
	m_chunkVertexBuffers.clear();
	m_chunkIndexBuffers.clear();

	m_bufferChunkCounts.clear();
	m_bufferIndexCounts.clear();

	m_chunkCount = 0;
}

bool CChunkManager::initialize()
{
	// Initialize all the blocks
	m_pBlockGrass = new CBlock( 1 );
	m_pBlockGrass->setBlockColor( glm::ivec3( 0, 255, 0) );
	this->registerBlock( m_pBlockGrass );
	m_pBlockStone = new CBlock( 2 );
	m_pBlockStone->setBlockColor( glm::ivec3( 150, 150, 150 ) );
	this->registerBlock( m_pBlockStone );

	return true;
}
void CChunkManager::destroy()
{
	// Close the terrain file
	this->closeTerrainFile();
	// Destroy the blocks
	SAFE_DELETE( m_pBlockGrass );
	SAFE_DELETE( m_pBlockStone );
	m_blockClasses.clear();
}

void CChunkManager::draw( glm::mat4 projection, glm::mat4 view )
{
	CShaderManager *pShaderManager = CGame::instance().getGraphics()->getShaderManager();
	glm::mat4 modelMatrix;
	unsigned int chunkStart, chunkLength, chunksRendered;
	int chunkRow, chunkColumn;
	glm::vec3 chunkOffset;
	size_t chunkIndex;

	// use the chunk shader program
	pShaderManager->getProgram( SHADERPROGRAM_CHUNK )->bind();
	if( m_bUpdateScale ) {
		glUniform1f( pShaderManager->getProgram( SHADERPROGRAM_CHUNK )->getUniform( "voxelScale" ), CHUNK_GRID_SIZE );
		m_bUpdateScale = false;
	}

	chunkLength = CHUNK_VERTEX_COUNT;
	chunksRendered = 0;
	chunkOffset = glm::vec3( -m_chunkViewDistance*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE, 0, -m_chunkViewDistance*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE );
	chunkIndex = 0;
	for( unsigned int i = 0; i < m_chunkVertexArrays.size(); i++ )
	{
		glBindVertexArray( m_chunkVertexArrays[i] );
		for( unsigned int chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ )
		{
			// Calculate its position in the grid
			chunkRow = (int)floor( (double)chunkIndex / (double)(m_chunkViewDistance * 2 + 1) );
			chunkColumn = chunkIndex - chunkRow*(m_chunkViewDistance * 2 + 1);

			modelMatrix = glm::translate( glm::mat4( 1.0f ), glm::vec3( chunkRow*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE, 0.0f, chunkColumn*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE )+chunkOffset );
			// Update matrices
			pShaderManager->m_ubGlobalMatrices.mvp = projection * view * modelMatrix;
			pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );

			// Calculate where in the array to draw
			chunkStart = chunk*CHUNK_VERTEX_COUNT;

			glDrawArrays( GL_TRIANGLES, chunkStart, chunkLength );
			//glDrawRangeElements( GL_TRIANGLES, chunkStart, chunkStart + chunkLength, m_chunks[chunkIndex]->getIndexCount(), GL_UNSIGNED_INT, (GLvoid*)(sizeof(unsigned int)*chunk*CHUNK_INDEX_COUNT) );
			chunkIndex++;
		}
		chunksRendered += m_bufferChunkCounts[i];
	}
}

bool CChunkManager::allocateChunks( unsigned char viewDistance )
{
	glm::ivec2 renderPosOffset;

	// Allocate all the chunks based on the view distance
	m_chunkViewDistance = viewDistance;
	m_chunkCount = (m_chunkViewDistance * 2 + 1)*(m_chunkViewDistance * 2 + 1);
	// Allocate the chunks around render pos
	renderPosOffset = m_renderPos + glm::ivec2( -m_chunkViewDistance, -m_chunkViewDistance );
	m_chunks.reserve( m_chunkCount );
	for( int x = 0; x < (m_chunkViewDistance * 2 + 1); x++ )
	{
		for( int y = 0; y < (m_chunkViewDistance * 2 + 1); y++ )
		{
			// Load data from file
			CChunk *pChunk = new CChunk();
			unsigned short *pData = this->readRawChunkData( glm::ivec2( renderPosOffset.x+x, renderPosOffset.y+y ) );
			if( !pData )
				return false;
			if( !pChunk->initialize() )
				return false;
			pChunk->setRawData( pData );
			m_chunks.push_back( pChunk );
			SAFE_DELETE_A( pData );
		}
	}

	// Generate initial chunk meshes
	if( !this->generateMeshes() )
		return false;

	return true;
}
void CChunkManager::destroyChunks()
{
	// Destroy each chunk
	for( auto it = m_chunks.begin(); it != m_chunks.end(); it++ ) {
		DESTROY_DELETE( (*it) );
	}
	// Destroy the meshes
	this->destroyMeshes();
	m_chunkCount = 0;
	m_chunks.clear();
}

bool CChunkManager::openTerrainFile( std::string path )
{
	TerrainSaveHeader terrainHeader;
	TerrainPositionTable positionTable;

	// Open the chunk stream
	try
	{
		// Open the file
		m_chunkStream.open( path.c_str(), std::ios::in | std::ios::out | std::ios::binary );
		// Get the file size
		m_chunkStream.seekg( 0, std::ios::end );
		m_chunkDataSize = (size_t)m_chunkStream.tellg();
		m_chunkStream.seekg( 0, std::ios::beg );
		// Read the header
		m_chunkStream.read( reinterpret_cast<char*>( &terrainHeader ), sizeof( TerrainSaveHeader ) );
		// Read the position table
		m_chunkStream.read( reinterpret_cast<char*>( &positionTable ), sizeof( TerrainPositionTable ) );
	}
	catch( std::ios_base::failure ) {
		PrintError( L"Failed to open terrain file \"%hs\"\n", path.c_str() );
		return false;
	}
	// Make sure the version and height matches
	if( terrainHeader.version_a != TERRAIN_VERSION_A || terrainHeader.version_b != TERRAIN_VERSION_B ) {
		PrintError( L"Terrain file (version %i.%i) is not the correct version (%i.%i)\n", terrainHeader.version_a, terrainHeader.version_b, TERRAIN_VERSION_A, TERRAIN_VERSION_B );
		return false;
	}
	if( terrainHeader.chunkHeight != CHUNK_HEIGHT ) {
		PrintError( L"Terrain file (height %i) does not match program chunk height (%i)\n", terrainHeader.chunkHeight, CHUNK_HEIGHT );
		return false;
	}
	// Interpret the position table
	memset( &m_chunkOffsets, -1, sizeof( m_chunkOffsets ) );
	for( unsigned int i = 0; i < TERRAIN_REGION_SIDE*TERRAIN_REGION_SIDE; i++ ) {
		if( positionTable.x[i] == -1 || positionTable.y[i] == -1 )
			continue;
		m_chunkOffsets[positionTable.x[i]][positionTable.y[i]] = i;
	}
	// Create the chunks
	if( !this->allocateChunks( 2 ) )
		return false;

	return true;
}
bool CChunkManager::saveTerrainFile( std::string path )
{
	return true;
}
void CChunkManager::closeTerrainFile()
{
	// Close the stream
	m_chunkDataSize = 0;
	m_chunkStream.close();
	// Delete the chunks
	this->destroyChunks();
}

unsigned short* CChunkManager::readRawChunkData( glm::ivec2 pos )
{
	size_t offset;
	unsigned short *pData;

	// Read the raw chunk data from the file

	// Make sure its exists
	if( m_chunkOffsets[pos.x][pos.y] == -1 ) {
		PrintError( L"Tried to load chunk that does not exist\n" );
		return NULL;
	}
	// Move to the offset
	offset = sizeof( TerrainSaveHeader ) + sizeof( TerrainPositionTable );
	offset += m_chunkOffsets[pos.x][pos.y]*CHUNK_BLOCK_COUNT*sizeof( unsigned short );
	// Make sure its valid
	if( offset > m_chunkDataSize ) {
		PrintError( L"Invalid offset in terrain file\n" );
		return NULL;
	}
	m_chunkStream.seekg( offset );
	// Read the data
	pData = new unsigned short[CHUNK_BLOCK_COUNT];
	m_chunkStream.read( reinterpret_cast<char*>(pData), sizeof( unsigned short )*CHUNK_BLOCK_COUNT );

	return pData;
}

// Returns the index (0 to chunk count)
// 0,0 is in the top left corner and is index 0
// X axis is to the right
// Z axis is down
// index increases to the right
unsigned int CChunkManager::getChunkIndex( int x, int z ) {
	return (z*(m_chunkViewDistance * 2 + 1)) + x;
}

void CChunkManager::registerBlock( CBlock* pBlock ) {
	m_blockClasses.insert( std::pair<unsigned short, CBlock*>( pBlock->getBlockId(), pBlock ) );
}
CBlock* CChunkManager::getBlockById( unsigned short id )
{
	auto block = m_blockClasses.find( id );
	if( block != m_blockClasses.end() )
		return (*block).second;
	else
		return NULL; // air
}

////////////
// CChunk //
////////////

CChunk::CChunk() {
	m_bufferIndex = 0;
	m_vertexOffset = 0;
	m_indexOffset = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
}
CChunk::~CChunk() {
}

bool CChunk::initialize()
{
	m_blocks.resize( CHUNK_BLOCK_COUNT );

	return true;
}
void CChunk::destroy() 
{
	m_bufferIndex = 0;
	m_vertexOffset = 0;
	m_indexOffset = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
	m_blocks.clear();
}

void CChunk::setBufferPosition( int bufferIndex, GLuint vertexOffset, GLuint indexOffset )
{
	m_bufferIndex = bufferIndex;
	m_vertexOffset = vertexOffset;
	m_indexOffset = indexOffset;
}
bool CChunk::populateVertices()
{
	GLuint currentVertex;
	size_t currentBlock;
	ChunkVertex *pVertices;
	glm::ivec3 currentColor;

	// The vertex buffer should already be bound

	// Get a pointer to the data
	pVertices = reinterpret_cast<ChunkVertex*>(glMapBufferRange( GL_ARRAY_BUFFER, sizeof( ChunkVertex )*m_vertexOffset, sizeof( ChunkVertex )*CHUNK_VERTEX_COUNT, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT ));
	if( !pVertices ) {
		PrintError( L"Failed to update terrain\n" );
		return false;
	}
	// Update the data
	currentVertex = 0;
	currentBlock = 0;
	m_vertexCount = 0;
	/*for( unsigned int y = 0; y < CHUNK_HEIGHT + 1; y++ ) {
		for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH + 1; x++ ) {
			for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH + 1; z++ ) {
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y,z ), glm::ivec3( 200, 200, 200 ) );
				m_vertexCount++;
			}
		}
	}*/
	for( unsigned int y = 0; y < CHUNK_HEIGHT; y++ ) {
		for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH; x++ ) {
			for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH; z++ ) {
				if( !m_blocks[currentBlock] ) {
					currentBlock++;
					continue;
				}
				currentColor = m_blocks[currentBlock]->getBlockColor();

				// TOP
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1, z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1, z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1, z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1, z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1, z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1, z ), currentColor );
				// BOTTOM
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z+1 ), currentColor );
				// FRONT
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z+1 ), currentColor );
				// BACK
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1,z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z ), currentColor );
				// RIGHT
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x+1, y,	 z+1 ), currentColor );
				// LEFT
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1,z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y+1,z+1 ), currentColor );
				pVertices[currentVertex++] = GenVertex( glm::ivec3( x,	 y,	 z ), currentColor );

				currentBlock++;
			}
		}
	}
	// Finish
	glUnmapBuffer( GL_ARRAY_BUFFER );

	return true;
}
bool CChunk::populateIndices()
{
	GLuint currentIndex;
	size_t currentBlock;
	unsigned int *pIndices;
	unsigned int firstIndex;
	unsigned int layerSize = (CHUNK_SIDE_LENGTH + 1)*(CHUNK_SIDE_LENGTH + 1);
	unsigned int rowSize = (CHUNK_SIDE_LENGTH + 1);

	// The index buffer should already be bound

	// Get a pointer to the data
	pIndices = reinterpret_cast<unsigned int*>(glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int )*m_indexOffset, sizeof( unsigned int )*CHUNK_INDEX_COUNT, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT ));
	if( !pIndices ) {
		PrintError( L"Failed to update terrain\n" );
		return false;
	}

	// Update the data
	currentIndex = 0;
	currentBlock = 0;
	m_indexCount = 0;
	for( unsigned int y = 0; y < CHUNK_HEIGHT; y++ )
	{
		firstIndex = y*layerSize;
		for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH; x++ )
		{
			for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH; z++ )
			{
				if( !m_blocks[currentBlock] ) {
					firstIndex++;
					currentBlock++;
					continue;
				}

				// BOTTOM
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + rowSize + 1;
				pIndices[currentIndex++] = firstIndex + 1;
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + rowSize;
				pIndices[currentIndex++] = firstIndex + rowSize + 1;
				// TOP
				pIndices[currentIndex++] = firstIndex + layerSize;
				pIndices[currentIndex++] = firstIndex + layerSize + 1;
				pIndices[currentIndex++] = firstIndex + layerSize + rowSize + 1;
				pIndices[currentIndex++] = firstIndex + layerSize;
				pIndices[currentIndex++] = firstIndex + layerSize + rowSize + 1;
				pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				// FRONT
				pIndices[currentIndex++] = firstIndex + 1;
				pIndices[currentIndex++] = firstIndex + 1 + layerSize + rowSize;
				pIndices[currentIndex++] = firstIndex + 1 + layerSize;
				pIndices[currentIndex++] = firstIndex + 1;
				pIndices[currentIndex++] = firstIndex + 1 + rowSize;
				pIndices[currentIndex++] = firstIndex + 1 + layerSize + rowSize;
				// BACK
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + layerSize;
				pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				pIndices[currentIndex++] = firstIndex + rowSize;
				// LEFT
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + layerSize + 1;
				pIndices[currentIndex++] = firstIndex + layerSize;
				pIndices[currentIndex++] = firstIndex;
				pIndices[currentIndex++] = firstIndex + 1;
				pIndices[currentIndex++] = firstIndex + layerSize + 1;
				// RIGHT
				pIndices[currentIndex++] = firstIndex + rowSize + layerSize;
				pIndices[currentIndex++] = firstIndex + rowSize + layerSize + 1;
				pIndices[currentIndex++] = firstIndex + rowSize;
				pIndices[currentIndex++] = firstIndex + rowSize;
				pIndices[currentIndex++] = firstIndex + rowSize + layerSize + 1;
				pIndices[currentIndex++] = firstIndex + rowSize + 1;

				m_indexCount += 36;
				currentBlock++;

				firstIndex++;
			}
			firstIndex++;
		}
	}

	// Finisn
	glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

	return true;
}

void CChunk::setRawData( unsigned short *pData )
{
	CChunkManager *pChunkManager = CGame::instance().getGraphics()->getWorld()->getChunkManager();

	for( unsigned int i = 0; i < CHUNK_BLOCK_COUNT; i++ ) {
		m_blocks[i] = pChunkManager->getBlockById( pData[i] );
	}
}

CBlock* CChunk::getBlockAt( glm::vec3 pos ) {
	return NULL;
}
GLuint CChunk::getIndexCount() {
	return m_indexCount;
}