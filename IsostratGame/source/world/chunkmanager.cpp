#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )

#include "base.h"
#include "world\chunkmanager.h"
#include "shader\shaderbase.h"
#include "graphics.h"

CChunkManager::CChunkManager()
{
	m_chunkViewDistance = 2;
	m_chunkCount = 0;
	m_bUpdateScale = true;
}
CChunkManager::~CChunkManager() {
}

bool CChunkManager::generateMeshes()
{
	int chunkSize;
	int bufferCount;
	unsigned int chunksGenerated;
	unsigned int firstIndex, chunkIndex;
	unsigned int chunksPerBuffer, verticesPerBuffer, indicesPerBuffer;

	// Determine how many chunks we'll have
	m_chunkCount = (m_chunkViewDistance * 2 + 1)*(m_chunkViewDistance * 2 + 1);
	// Determine how many buffers we need1
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

	verticesPerBuffer = CHUNK_VERTEX_COUNT*chunksPerBuffer;
	indicesPerBuffer = chunksPerBuffer*CHUNK_INDEX_COUNT;

	// Populate each vertex array
	chunksGenerated = 0;
	firstIndex = 0;
	for( int i = 0; i < bufferCount; i++ )
	{
		std::vector<ChunkVertex> chunkVertices;
		std::vector<unsigned int> chunkIndices;

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

		// Vertex buffer
		glBindBuffer( GL_ARRAY_BUFFER, m_chunkVertexBuffers[i] );
		chunkVertices.reserve( m_bufferChunkCounts[i]*CHUNK_VERTEX_COUNT );
		for( unsigned int chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ )
		{
			// For each chunk
			for( unsigned int y = 0; y < CHUNK_HEIGHT + 1; y++ ) {
				for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH + 1; x++ ) {
					for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH + 1; z++ ) {
						ChunkVertex vertex;
						vertex.pos = glm::ivec3( x, y, z );
						if( chunk % 2 == 0 )
							vertex.color = glm::vec3( 0.0f, 1.0f, 0.0f );
						else
							vertex.color = glm::vec3( 0.0f, 1.0f, 1.0f );
						chunkVertices.push_back( vertex );
					}
				}
			}
		}
		// Set the data
		glBufferData( GL_ARRAY_BUFFER, sizeof( ChunkVertex )*chunkVertices.size(), &chunkVertices[0], GL_STATIC_DRAW );
		glVertexAttribIPointer( 0, 3, GL_INT, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, pos ) );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( ChunkVertex ), (GLvoid*)offsetof( ChunkVertex, color ) );
		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );

		// Index buffer
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_chunkIndexBuffers[i] );
		chunkIndices.resize( m_bufferChunkCounts[i]*CHUNK_INDEX_COUNT );
		for( unsigned int chunk = 0; chunk < m_bufferChunkCounts[i]; chunk++ )
		{
			unsigned int firstVertex;
			unsigned int layerSize = (CHUNK_SIDE_LENGTH + 1)*(CHUNK_SIDE_LENGTH + 1);
			unsigned int rowSize = (CHUNK_SIDE_LENGTH + 1);

			chunkIndex = 0;
			firstVertex = chunk*CHUNK_VERTEX_COUNT;
			for( unsigned int y = 0; y < CHUNK_HEIGHT; y++ )
			{
				for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH; x++ )
				{
					for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH; z++ )
					{
						// BOTTOM
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + 1;
						// TOP
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + rowSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + rowSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + rowSize;
						// FRONT
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1 + layerSize + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1 + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1 + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1 + layerSize + rowSize;
						// BACK
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize;
						// LEFT
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + layerSize + 1;
						// RIGHT
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + layerSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + layerSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + layerSize + 1;
						chunkIndices[firstIndex + (chunkIndex++)] = firstVertex + rowSize + 1;

						firstVertex++;
					}
					firstVertex++;
				}
			}
			firstIndex += CHUNK_INDEX_COUNT;
		}
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int )*chunkIndices.size(), &chunkIndices[0], GL_STATIC_DRAW );

		chunksGenerated += chunksPerBuffer;
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
	// Generate the chunk meshes
	this->generateMeshes();

	return true;
}
void CChunkManager::destroy()
{
	this->destroyMeshes();
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

// Returns the index (0 to chunk count)
// 0,0 is in the top left corner and is index 0
// X axis is to the right
// Z axis is down
// index increases to the right
unsigned int CChunkManager::getChunkIndex( int x, int z ) {
	return (z*(m_chunkViewDistance * 2 + 1)) + x;
}