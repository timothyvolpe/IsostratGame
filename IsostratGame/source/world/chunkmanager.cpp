#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "world\chunkmanager.h"
#include "shader\shaderbase.h"
#include "graphics.h"

#include "world\block.h"

///////////////////
// CChunkManager //
///////////////////

CChunkManager::CChunkManager()
{
	m_chunkViewDistance = 0;
	m_chunkCount = 0;
	m_bUpdateScale = true;
}
CChunkManager::~CChunkManager() {
}

// Generates all the chunk meshes based on the loaded data
bool CChunkManager::generateMeshes()
{
	int chunkSize;
	int bufferCount;
	unsigned int chunksPerBuffer;

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
		glVertexAttribIPointer( 0, 3, GL_INT, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, pos ) );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, color ) );
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		// Fill the index buffer
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_chunkIndexBuffers[i] );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int )*CHUNK_INDEX_COUNT*m_bufferChunkCounts[i], NULL, GL_DYNAMIC_DRAW );
	}

	// Fill the mesh for each chunk
	for( int i = 0; i < bufferCount; i++ )
	{
		glBindVertexArray( m_chunkVertexArrays[i] );
		// Do all vertex data for this buffer
		glBindBuffer( GL_ARRAY_BUFFER, m_chunkVertexBuffers[i] );
		for( GLuint chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ ) {
			// Set the buffer position
			m_chunks[chunk]->setBufferPosition( i, chunk*CHUNK_VERTEX_COUNT, chunk*CHUNK_INDEX_COUNT );
			if( !m_chunks[chunk]->populateVertices() )
				return false;
		}
		// Do all index data for this buffer
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_chunkIndexBuffers[i] );
		for( GLuint chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ ) {
			if( !m_chunks[chunk]->populateIndices() )
				return false;
		}
	}

	return true;
}
void CChunkManager::destroyMeshes()
{
	glDeleteVertexArrays( m_chunkVertexArrays.size(), &m_chunkVertexArrays[0] );
	glDeleteBuffers( m_chunkVertexBuffers.size(), &m_chunkVertexBuffers[0] );
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
	// Load a terrain file
	if( !this->openTerrainFile( "test.ter" ) )
		return false;

	return true;
}
void CChunkManager::destroy()
{
	// Close the terrain file
	this->closeTerrainFile();
}

void CChunkManager::draw( glm::mat4 projection, glm::mat4 view )
{
	CShaderManager *pShaderManager = CGame::instance().getGraphics()->getShaderManager();
	glm::mat4 modelMatrix;
	unsigned int chunkStart, chunkLength, chunksRendered;
	int chunkIndex, chunkRow, chunkColumn;
	glm::vec3 chunkOffset;

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

			//glDrawArrays( GL_TRIANGLES, chunkStart, chunkLength );
			glDrawRangeElements( GL_TRIANGLES, chunkStart, chunkStart + chunkLength, CHUNK_INDEX_COUNT, GL_UNSIGNED_INT, (GLvoid*)(sizeof(unsigned int)*chunk*CHUNK_INDEX_COUNT) );
			chunkIndex++;
		}
		chunksRendered += m_bufferChunkCounts[i];
	}
}

bool CChunkManager::allocateChunks( unsigned char viewDistance )
{
	// Allocate all the chunks based on the view distance
	m_chunkViewDistance = viewDistance;
	m_chunkCount = (m_chunkViewDistance * 2 + 1)*(m_chunkViewDistance * 2 + 1);
	// Allocate the chunks
	m_chunks.reserve( m_chunkCount );
	for( unsigned int i = 0; i < m_chunkCount; i++ ) {
		m_chunks.push_back( new CChunk() );
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
	// Delete the chunks
	this->destroyChunks();
}

// Returns the index (0 to chunk count)
// 0,0 is in the top left corner and is index 0
// X axis is to the right
// Z axis is down
// index increases to the right
unsigned int CChunkManager::getChunkIndex( int x, int z ) {
	return (z*(m_chunkViewDistance * 2 + 1)) + x;
}

////////////
// CChunk //
////////////

CChunk::CChunk() {
	m_bufferIndex = 0;
	m_vertexOffset = 0;
	m_indexOffset = 0;
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
	ChunkVertex *pVertices;

	// The vertex buffer should already be bound

	// Get a pointer to the data
	pVertices = reinterpret_cast<ChunkVertex*>(glMapBufferRange( GL_ARRAY_BUFFER, sizeof( ChunkVertex )*m_vertexOffset, sizeof( ChunkVertex )*CHUNK_VERTEX_COUNT, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT ));
	if( !pVertices ) {
		PrintError( L"Failed to update terrain\n" );
		return false;
	}
	// Update the data
	currentVertex = 0;
	for( unsigned int y = 0; y < CHUNK_HEIGHT + 1; y++ ) {
		for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH + 1; x++ ) {
			for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH + 1; z++ ) {
				ChunkVertex vertex;
				vertex.pos = glm::ivec3( x, y, z );
				vertex.color = glm::vec3( 0.5f, (x % 2 == 0) ? 0.0f : 1.0f, (z % 2 != 0) ? 0.0f : 1.0f );
				pVertices[currentVertex++] = vertex;
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
	for( unsigned int y = 0; y < CHUNK_HEIGHT; y++ )
	{
		firstIndex = y*layerSize;
		for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH; x++ )
		{
			for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH; z++ )
			{
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

				firstIndex++;
			}
			firstIndex++;
		}
	}

	// Finisn
	glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

	return true;
}

CBlock* CChunk::getBlockAt( glm::vec3 pos ) {
	return NULL;
}