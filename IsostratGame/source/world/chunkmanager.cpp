#pragma warning( disable : 4996 )
#include <glm\ext.hpp>
#pragma warning( default: 4996 )
#include <algorithm>

#include "base.h"
#include "world\world.h"
#include "world\chunkmanager.h"
#include "shader\shaderbase.h"
#include "graphics.h"
#include "camera.h"
#include "debugrender.h"

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

	m_bValidSave = 0;
	m_savePath = L"";

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

	m_hasChunkInBuffer.clear();

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
	m_hasChunkInBuffer.reserve( bufferCount );
	glGenVertexArrays( bufferCount, &m_chunkVertexArrays[0] );
	glGenBuffers( bufferCount, &m_chunkVertexBuffers[0] );
	glGenBuffers( bufferCount, &m_chunkIndexBuffers[0] );

	// Fill each buffer with blank data to allocate space
	for( int i = 0; i < bufferCount; i++ )
	{
		// Calculate chunks and indices in the buffer
		if( i == bufferCount - 1 ) {
			// The last buffer wont be full size
			m_bufferChunkCounts.push_back( m_chunkCount - i*chunksPerBuffer );
			m_bufferIndexCounts.push_back( m_bufferChunkCounts[i] * CHUNK_INDEX_COUNT );
		}
		else {
			m_bufferChunkCounts.push_back( chunksPerBuffer );
			m_bufferIndexCounts.push_back( chunksPerBuffer * CHUNK_INDEX_COUNT );
		}
		m_hasChunkInBuffer.push_back( std::vector<bool>( m_bufferChunkCounts.back(), false ) );

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
	m_pBlockWater = new CBlock( 3 );
	m_pBlockWater->setBlockColor( glm::ivec3( 0, 0, 150 ) );
	this->registerBlock( m_pBlockWater );

	m_renderPos = this->getEyeChunk();

	// Start the chunk loader thread
	m_pChunkLoader = new CChunkLoader();
	if( !m_pChunkLoader->initialize() )
		return false;

	return true;
}
void CChunkManager::destroy()
{
	// Stop the chunk loader
	m_pChunkLoader->destroy();
	SAFE_DELETE( m_pChunkLoader );
	// Destroy the blocks
	SAFE_DELETE( m_pBlockGrass );
	SAFE_DELETE( m_pBlockStone );
	SAFE_DELETE( m_pBlockWater );
	m_blockClasses.clear();
}

void CChunkManager::updateChunkGrid( char movementDirection )
{
	boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
	std::vector<CChunk*> m_oldRow;
	std::vector<CChunk*> m_oldColumn;
	int rowPos;

	switch( movementDirection )
	{
	case CHUNK_DIRECTION_BACK:
		// Push a "new" row to the front, pop the old row off the back
		m_oldRow = m_chunkGrid_.back();
		m_chunkGrid_.pop_back();
		m_chunkGrid_.insert( m_chunkGrid_.begin(), m_oldRow );
		break;
	case CHUNK_DIRECTION_FRONT:
		// Push a "new" row to the back, pop the old row off the front
		m_oldRow = m_chunkGrid_.front();
		m_chunkGrid_.erase( m_chunkGrid_.begin() );
		m_chunkGrid_.push_back( m_oldRow );
		break;
	case CHUNK_DIRECTION_RIGHT:
		// Push a "new" column to the right, and pop the old column off the back
		for( auto it = m_chunkGrid_.begin(); it != m_chunkGrid_.end(); it++ ) {
			m_oldColumn.push_back( (*it).front() );
			(*it).erase( (*it).begin() );
		}
		// Insert old
		rowPos = m_chunkGrid_.size()-1;
		for( auto it = m_chunkGrid_.begin(); it != m_chunkGrid_.end(); it++, rowPos-- )
			(*it).push_back( m_oldColumn[rowPos] );
		break;
	case CHUNK_DIRECTION_LEFT:
		// Push a "new" column to the right, and pop the old column off the back
		for( auto it = m_chunkGrid_.begin(); it != m_chunkGrid_.end(); it++ ) {
			m_oldColumn.push_back( (*it).back() );
			(*it).pop_back();
		}
		// Insert old
		rowPos = 0;
		for( auto it = m_chunkGrid_.begin(); it != m_chunkGrid_.end(); it++, rowPos++ )
			(*it).insert( (*it).begin(), m_oldColumn[rowPos] );
		break;
	default:
		PrintError( L"Invalid chunk movement direction\n" );
		break;
	}
	lock.unlock();

	// Set new positions
	for( unsigned int x = 0; x < m_chunkGrid_.size(); x++ ) {
		for( unsigned int y = 0; y < m_chunkGrid_[x].size(); y++ ) {
			if( m_chunkGrid_[x][y] )
				m_chunkGrid_[x][y]->setChunkGridPos( glm::ivec2( x, y ) );
		}
	}
	// Delete/cache old chunks and request new ones to replace them
	for( auto it = m_oldRow.begin(); it != m_oldRow.end(); it++ ) {
		if( (*it) )
			this->removeChunk( (*it) );
	}
	for( auto it = m_oldColumn.begin(); it != m_oldColumn.end(); it++ ) {
		if( (*it) )
			this->removeChunk( (*it) );
	}

	boost::shared_lock_guard<boost::shared_mutex> lock2( m_mutex_ );
	switch( movementDirection )
	{
	case CHUNK_DIRECTION_FRONT:
		for( unsigned int y = 0; y < m_chunkGrid_.size(); y++ )
			m_pChunkLoader->addChunkToQueue( this->gridPosToAbsolutePos( glm::ivec2( m_chunkGrid_.size()-1, y ) ) );
		break;
	case CHUNK_DIRECTION_BACK:
		for( unsigned int y = 0; y < m_chunkGrid_.size(); y++ )
			m_pChunkLoader->addChunkToQueue( this->gridPosToAbsolutePos( glm::ivec2( 0, y ) ) );
		break;
	case CHUNK_DIRECTION_RIGHT:
		for( unsigned int x = 0; x < m_chunkGrid_.size(); x++)
			m_pChunkLoader->addChunkToQueue( this->gridPosToAbsolutePos( glm::ivec2( x, m_chunkGrid_.size()-1 ) ) );
		break;
	case CHUNK_DIRECTION_LEFT:
		for( unsigned int x = 0; x < m_chunkGrid_.size(); x++ )
			m_pChunkLoader->addChunkToQueue( this->gridPosToAbsolutePos( glm::ivec2( x, 0 ) ) );
		break;
	default:
		PrintError( L"Invalid chunk movement direction\n" );
		break;
	}	
}

void CChunkManager::update()
{
	int chunksCopied;
	LoadedChunk chunk;
	glm::ivec2 chunkGridPos, eyeChunk;
	char movementDirection;

	m_pChunkLoader->sendChunkQueue();

	// Copy over all the new chunks (maximum of 5 per frame)
	chunksCopied = 0;
	while( chunksCopied < 1 && m_pChunkLoader->getFinishedQueueSize() > 0 )
	{
		// Make sure we get a valid chunk (it wasnt cleared)
		chunk = m_pChunkLoader->popFinishedChunk();
		if( !chunk.pChunk )
			continue;

		// Put it in grid position
		chunkGridPos = this->absolutePosToGridPos( chunk.position );
		// Check if its out of bounds
		if( chunkGridPos.x >= (m_chunkViewDistance * 2 + 1) || chunkGridPos.y >= (m_chunkViewDistance * 2 + 1) || 
			chunkGridPos.x < 0 || chunkGridPos.y < 0 )
		{
			chunk.pChunk->destroy();
			SAFE_DELETE( chunk.pChunk );
			continue;
		}
		else {
			chunk.pChunk->setChunkGridPos( chunkGridPos );
			m_chunks.push_back( chunk.pChunk );
			boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
			m_chunkGrid_[chunkGridPos.x][chunkGridPos.y] = chunk.pChunk;
			lock.unlock();
		}

		// Find a place for it in a buffer
		unsigned int bufferIndex = 0;
		for( auto it = m_hasChunkInBuffer.begin(); it != m_hasChunkInBuffer.end(); it++, bufferIndex++ )
		{
			unsigned int chunkIndex = 0;
			for( auto it2 = (*it).begin(); it2 != (*it).end(); it2++, chunkIndex++ )
			{
				// If we found a spot
				if( (*it2) == false )
				{
					chunk.pChunk->setBufferPosition( bufferIndex, chunkIndex );
					chunk.pChunk->sendDataToBuffers();
					(*it2) = true;
					it = m_hasChunkInBuffer.end()-1;
					break;
				}
			}
		}

		chunksCopied++;
	}

	// Calculate the chunk that is under the eye
	eyeChunk = this->getEyeChunk();
	// Check if its different
	if( eyeChunk != m_renderPos )
	{
		if( eyeChunk.x == m_renderPos.x && eyeChunk.y > m_renderPos.y )
			movementDirection = CHUNK_DIRECTION_RIGHT;
		else if( eyeChunk.x == m_renderPos.x && eyeChunk.y < m_renderPos.y )
			movementDirection = CHUNK_DIRECTION_LEFT;
		else if( eyeChunk.x < m_renderPos.x && eyeChunk.y == m_renderPos.y )
			movementDirection = CHUNK_DIRECTION_BACK;
		else if( eyeChunk.x > m_renderPos.x && eyeChunk.y == m_renderPos.y )
			movementDirection = CHUNK_DIRECTION_FRONT;
		else {
			PrintError( L"Chunks move two directions??\n" );
			movementDirection = 0;
		}
		m_renderPos = eyeChunk;
		this->updateChunkGrid( movementDirection );
	}

	// Sort the chunks buffer vertex array object
	ChunkComparator comparator;
	std::sort( m_chunks.begin(), m_chunks.end(), comparator );
}
void CChunkManager::draw( glm::mat4 projection, glm::mat4 view )
{
	CShaderManager *pShaderManager = CGame::instance().getGraphics()->getShaderManager();
	CDebugRender *pDebugRender  = CGame::instance().getGraphics()->getDebugRender();
	glm::mat4 modelMatrix;
	glm::vec3 chunkOffset, chunkPos;
	glm::ivec2 chunkGridPos;
	size_t currentBufferIndex;

	// use the chunk shader program
	pShaderManager->getProgram( SHADERPROGRAM_CHUNK )->bind();
	if( m_bUpdateScale ) {
		glUniform1f( pShaderManager->getProgram( SHADERPROGRAM_CHUNK )->getUniform( "voxelScale" ), CHUNK_GRID_SIZE );
		m_bUpdateScale = false;
	}

	// Render each chunk
	currentBufferIndex = (GLuint)-1;
	chunkOffset = glm::vec3( -m_chunkViewDistance*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE, 0, -m_chunkViewDistance*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE );
	for( auto it = m_chunks.begin(); it != m_chunks.end(); it++ )
	{
		if( currentBufferIndex != (*it)->getBufferIndex() ) {
			currentBufferIndex = (*it)->getBufferIndex();
			glBindVertexArray( m_chunkVertexArrays[currentBufferIndex] );
		}
		// Update matrices
		chunkGridPos = (*it)->getChunkGridPos();
		chunkPos = glm::vec3( 
			(chunkGridPos.x*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE) + (m_renderPos.x * CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE),
			0.0f, 
			(chunkGridPos.y*CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE) + (m_renderPos.y * CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE)
			)+chunkOffset;
		modelMatrix = glm::translate( glm::mat4( 1.0f ), chunkPos );
		pShaderManager->m_ubGlobalMatrices.mvp = projection * view * modelMatrix;
		pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );
		// Calculate where in the array to draw
		glDrawArrays( GL_TRIANGLES, (*it)->getVertexOffset(), (*it)->getVertexCount() );
		// Debug outline
		pDebugRender->drawRect3d( chunkPos, glm::vec3( CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE, CHUNK_HEIGHT*CHUNK_GRID_SIZE, CHUNK_SIDE_LENGTH*CHUNK_GRID_SIZE ), DEBUG_COLOR_CHUNK_ACTIVE );
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
	// Load each chunk
	boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
	m_chunkGrid_.resize( (m_chunkViewDistance * 2 + 1) );
	for( int x = 0; x < (m_chunkViewDistance * 2 + 1); x++ ) {
		for( int z = 0; z < (m_chunkViewDistance * 2 + 1); z++ ) {
			m_pChunkLoader->addChunkToQueue( glm::ivec2( renderPosOffset.x + x, renderPosOffset.y + z ) );
			m_chunkGrid_[x].push_back( 0 );
		}
	}
	lock.unlock();

	// Generate initial chunk meshes
	if( !this->generateMeshes() )
		return false;

	return true;
}
bool CChunkManager::removeChunk( CChunk *pChunk )
{
	boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
	if( !pChunk )
		return false;
	// Remove from grid
	m_chunkGrid_[pChunk->getChunkGridPos().x][pChunk->getChunkGridPos().y] = 0;
	// Find and remove from chunk list
	auto it = std::find( m_chunks.begin(), m_chunks.end(), pChunk );
	if( it != m_chunks.end() )
		m_chunks.erase( it );
	// Remove from buffer
	if( pChunk->hasBufferPosition() ) {
		int bufferIndex = pChunk->getBufferIndex();
		int chunkIndex = pChunk->getChunkIndex();
		m_hasChunkInBuffer[bufferIndex][chunkIndex] = false;
	}

	pChunk->destroy();
	SAFE_DELETE( pChunk );

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
	boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
	m_chunkGrid_.clear();
}

bool CChunkManager::setSaveFile( std::wstring saveName )
{
	// We need to verify this is a valid save

	// later...

	m_bValidSave = true;
	m_savePath = boost::filesystem::current_path();
	m_savePath /= FILESYSTEM_SAVEDIR;
	m_savePath /= saveName;
	m_savePath /= "terrain";

	m_pChunkLoader->setSavePath( m_savePath );

	// Create the space for the chunks in memory
	// Begin loading them from file and creating their vertex data
	if( !this->allocateChunks( 5 ) )
		return false;

	return true;
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

CChunk* CChunkManager::getChunkNeighbor( glm::ivec2 vectorPos, char direction )
{
	glm::ivec2 neighborPos;

	switch( direction )
	{
	case CHUNK_DIRECTION_RIGHT:
		neighborPos = vectorPos + glm::ivec2( 1, 0 );
		break;
	case CHUNK_DIRECTION_LEFT:
		neighborPos = vectorPos - glm::ivec2( 1, 0 );
		break;
	case CHUNK_DIRECTION_FRONT:
		neighborPos = vectorPos - glm::ivec2( 0, 1 );
		break;
	case CHUNK_DIRECTION_BACK:
		neighborPos = vectorPos + glm::ivec2( 0, 1 );
		break;
	default:
		PrintWarn( L"Unknown block direction to getChunkNeighbor\n" );
		return NULL;
	}

	boost::shared_lock_guard<boost::shared_mutex> lock( m_mutex_ );
	if( (size_t)neighborPos.x >= m_chunkGrid_.size() )
		return NULL;
	else if( (size_t)neighborPos.y >= m_chunkGrid_[neighborPos.x].size() )
		return NULL;
	else
		return m_chunkGrid_[neighborPos.x][neighborPos.y];
}
glm::ivec2 CChunkManager::getEyeChunk() {
	glm::vec3 eyePos = CGame::instance().getGraphics()->getCamera()->getEyePosition();
	return glm::ivec2( (int)floor( eyePos.x / (float)(CHUNK_GRID_SIZE*CHUNK_SIDE_LENGTH) ), (int)floor( eyePos.z / (float)(CHUNK_GRID_SIZE*CHUNK_SIDE_LENGTH) ) );
}
void CChunkManager::getBufferIds( int bufferIndex, unsigned int *pVertexArrayId, unsigned int *pVertexBufferId, unsigned int *pIndexBufferId ) {
	(*pVertexArrayId) = m_chunkVertexArrays[bufferIndex];
	(*pVertexBufferId) = m_chunkVertexBuffers[bufferIndex];
	(*pIndexBufferId) = m_chunkIndexBuffers[bufferIndex];
}

glm::ivec2 CChunkManager::absolutePosToGridPos( glm::ivec2 chunkPos )
{
	glm::ivec2 gridOrigin = m_renderPos - (int)m_chunkViewDistance;
	return chunkPos - gridOrigin;
}
glm::ivec2 CChunkManager::gridPosToAbsolutePos( glm::ivec2 gridPos )
{
	glm::ivec2 gridOrigin = m_renderPos - (int)m_chunkViewDistance;
	return gridPos + gridOrigin;
}

//////////////////
// CChunkLoader //
//////////////////

CChunkLoader::CChunkLoader() {
	m_running_.store( false );
	m_chunksToLoad_.store( false );
}
CChunkLoader::~CChunkLoader() {
}

void CChunkLoader::threadStart()
{
	PrintInfo( L"THREAD: Chunk loader thread started\n" );

	while( m_running_.load() )
	{
		this->loadQueue();

		boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
		while( !m_chunksToLoad_.load() && m_running_.load() ) {
			m_loadChunks_.wait( lock );
		}
		lock.unlock();
	}

	PrintInfo( L"THREAD: Chunk loader thread terminated\n" );
}

bool CChunkLoader::initialize()
{
	m_savePath = "";

	m_running_.store( true );
	m_chunksToLoad_.store( false );
	m_chunkQueue_.clear();

	m_chunkLoaderThread = boost::thread( &CChunkLoader::threadStart, this );

	return true;
}
void CChunkLoader::destroy()
{
	// Stop the thread
	m_running_.store( false );
	// Force queue to load in case we're waiting
	this->clearQueue();
	this->m_loadChunks_.notify_one();

	m_chunkLoaderThread.join();
}

void CChunkLoader::addChunkToQueue( glm::ivec2 absolutePosition ) {
	boost::lock_guard<boost::shared_mutex> lock( this->m_mutex_ );
	m_chunkQueue_.push_back( absolutePosition );
}
void CChunkLoader::clearQueue() {
	boost::lock_guard<boost::shared_mutex> lock( this->m_mutex_ );
	m_chunkQueue_.clear();
}
glm::ivec2 CChunkLoader::popChunkFromQueue( bool *pSuccess )
{
	if( this->getQueueSize() == 0 ) {
		*pSuccess = false;
		return glm::ivec2();
	}
	boost::unique_lock<boost::shared_mutex> lock( this->m_mutex_ );
	glm::ivec2 front = m_chunkQueue_.front();
	m_chunkQueue_.pop_front();
	lock.unlock();
	*pSuccess = true;
	return front;
}
int CChunkLoader::getQueueSize() {
	boost::shared_lock<boost::shared_mutex> lock( this->m_mutex_ );
	int count = m_chunkQueue_.size();
	lock.unlock();
	return count;
}
void CChunkLoader::sendChunkQueue()
{
	if( this->getQueueSize() > 0 ) {
		m_chunksToLoad_.store( true );
		this->m_loadChunks_.notify_one();
	}
}

void CChunkLoader::addFinishedChunk( glm::ivec2 position, CChunk *pChunk )
{
	LoadedChunk chunk;
	chunk.position = position;
	chunk.pChunk = pChunk;
	boost::lock_guard<boost::shared_mutex> lock( this->m_finishedMutex_ );
	m_finishedChunkQueue_.push_back( chunk );
}
LoadedChunk CChunkLoader::popFinishedChunk()
{
	LoadedChunk chunk;
	if( this->getFinishedQueueSize() == 0 ) {
		chunk.pChunk = 0;
		return chunk;
	}
	boost::unique_lock<boost::shared_mutex> lock( this->m_finishedMutex_ );
	chunk = m_finishedChunkQueue_.front();
	m_finishedChunkQueue_.pop_front();
	lock.unlock();
	return chunk;
}
int CChunkLoader::getFinishedQueueSize()
{
	boost::shared_lock<boost::shared_mutex> lock( this->m_finishedMutex_ );
	int count = m_finishedChunkQueue_.size();
	lock.unlock();
	return count;
}

bool CChunkLoader::populateChunkDataFromFile( glm::ivec2 position, CChunk *pChunk )
{
	glm::ivec2 regionFileCoords;
	glm::ivec2 positionInRegion;
	boost::filesystem::path regionFilePath;
	std::stringstream fileName;
	std::fstream regionStream;
	int chunkOffset;

	TerrainOffsetTable offsetTable;
	size_t offset;
	unsigned short *pData;

	pChunk->setChunkPos( position );

	// Open the required region file
	regionFileCoords = position / TERRAIN_REGION_SIDE;
	if( position.x < 0 )
		regionFileCoords.x--;
	if( position.y < 0 )
		regionFileCoords.y--;

	regionFilePath = m_savePath;
	fileName << regionFileCoords.x << "_" << regionFileCoords.y << ".sav";
	regionFilePath /= fileName.str();

	// open the file
	try
	{
		regionStream.exceptions( std::ios::failbit | std::ios::badbit );
		// Open the file
		regionStream.open( regionFilePath.c_str(), std::ios::in | std::ios::out | std::ios::binary );
		// Read the header
		//regionStream.read( 0, sizeof( TerrainSaveHeader ) );
		// Read the position table
		regionStream.read( reinterpret_cast<char*>(&offsetTable), sizeof( TerrainOffsetTable ) );
	}
	catch( std::ios_base::failure ) {
		PrintError( L"Failed to open terrain file \"%s\"\n", regionFilePath.c_str() );
		return false;
	}

	// Read the raw chunk data from the file

	positionInRegion = position - glm::ivec2( regionFileCoords.x*TERRAIN_REGION_SIDE, regionFileCoords.y*TERRAIN_REGION_SIDE );

	// Make sure its exists
	chunkOffset = offsetTable.offset[positionInRegion.x][positionInRegion.y];
	if( chunkOffset == -1 ) {
		PrintError( L"Tried to load chunk that does not exist\n" );
		return false;
	}

	// Move to the offset
	offset = sizeof( TerrainOffsetTable );
	offset += chunkOffset;
	// Make sure its valid
	regionStream.seekg( offset );
	// Read the data
	pData = new unsigned short[CHUNK_BLOCK_COUNT];
	regionStream.read( reinterpret_cast<char*>(pData), sizeof( unsigned short )*CHUNK_BLOCK_COUNT );
	if( regionStream.eof() ) {
		SAFE_DELETE_A( pData );
		return false;
	}
	else
		pChunk->setRawData( pData );

	SAFE_DELETE_A( pData );

	// Generate geometry
	if( !pChunk->generateVertices() )
		return false;
	//if( !pChunk->generateIndices() )
		//return false;

	return true;
}
void CChunkLoader::loadQueue()
{
	while( this->getQueueSize() > 0 )
	{
		glm::ivec2 pos;
		bool success;
		pos = this->popChunkFromQueue( &success );
		if( !success )
			continue;

		CChunk *pChunk = new CChunk();
		if( !pChunk->initialize() ) {
			SAFE_DELETE( pChunk );
			continue;
		}
		if( !this->populateChunkDataFromFile( pos, pChunk ) ) {
			pChunk->destroy();
			SAFE_DELETE( pChunk );
			continue;
		}
		this->addFinishedChunk( pos, pChunk );
	}

	m_chunksToLoad_.store( false );
}
void CChunkLoader::setSavePath( boost::filesystem::path savePath )
{
	m_savePath = savePath;
}

////////////
// CChunk //
////////////

#pragma region CChunk

bool CChunk::isOccludingBlock( CBlock* pBlock )
{
	if( !pBlock )
		return false;
	else if( !pBlock->isOpaque() )
		return false;
	return true;
}

CChunk::CChunk()
{
	m_bufferIndex = 0;
	m_chunkIndex = 0;
	m_vertexOffset = 0;
	m_indexOffset = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
	m_chunkPos = glm::ivec2( 0, 0 );
	m_chunkGridPos = glm::ivec2( 0, 0 );
	m_pVertices = 0;
	m_pIndices = 0;
	m_bHasBufferPos = false;
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
	m_chunkPos = glm::ivec2( 0, 0 );
	m_chunkGridPos = glm::ivec2( 0, 0 );
	m_blocks.clear();
	if( m_pVertices )
		SAFE_DELETE_A( m_pVertices );
	if( m_pIndices )
		SAFE_DELETE_A( m_pIndices );
}

void CChunk::setBufferPosition( size_t bufferIndex, size_t chunkIndex )
{
	m_bufferIndex = bufferIndex;
	m_chunkIndex = chunkIndex;
	m_vertexOffset = chunkIndex*CHUNK_VERTEX_COUNT;
	m_indexOffset = chunkIndex*CHUNK_INDEX_COUNT;
	m_bHasBufferPos = true;
}
bool CChunk::generateVertices()
{
	GLuint currentVertex;
	size_t currentBlock;
	glm::ivec3 currentColor, shadowColor;
	OcclusionList occlusionList;

	// Get the occlusion list
	occlusionList = this->getOcclusionList();

	// Create space for vertex buffer
	m_pVertices = new ChunkVertex[occlusionList.verticesRequired];
	if( !m_pVertices ) {
		PrintError( L"Memory error while generating chunk vertices.\n " );
		return false;
	}

	// Update the data
	currentVertex = 0;
	currentBlock = 0;
	m_vertexCount = 0;
	for( unsigned int y = 0; y < CHUNK_HEIGHT; y++ )
	{
		for( unsigned int z = 0; z < CHUNK_SIDE_LENGTH; z++ )
		{
			for( unsigned int x = 0; x < CHUNK_SIDE_LENGTH; x++ )
			{
				if( occlusionList.occlusionFlags[currentBlock] == 0 ) {
					currentBlock++;
					continue;
				}
				currentColor = m_blocks[currentBlock]->getBlockColor();
				shadowColor = glm::ivec3( currentColor.r / 2, currentColor.g / 2, currentColor.b / 2 );

				// TOP
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_UP) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z + 1 ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z ), currentColor );
					m_vertexCount += 6;
				}
				// BOTTOM
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_DOWN) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z + 1 ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z + 1 ), currentColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z + 1 ), currentColor );
					m_vertexCount += 6;
				}
				// FRONT
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_BACK) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z + 1 ), shadowColor );
					m_vertexCount += 6;
				}
				// BACK
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_FRONT) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z ), shadowColor / 2 );
					m_vertexCount += 6;
				}
				// RIGHT
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_RIGHT) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y + 1, z + 1 ), shadowColor / 2 );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x + 1, y, z + 1 ), shadowColor / 2 );
					m_vertexCount += 6;
				}
				// LEFT
				if( !(occlusionList.occlusionFlags[currentBlock] & CHUNK_DIRECTION_LEFT) ) {
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y + 1, z + 1 ), shadowColor );
					m_pVertices[currentVertex++] = GenVertex( glm::ivec3( x, y, z ), shadowColor );
					m_vertexCount += 6;
				}

				currentBlock++;
			}
		}
	}

	return true;
}
bool CChunk::generateIndices()
{
	GLuint currentIndex;
	size_t currentBlock;
	unsigned int firstIndex;
	unsigned int layerSize = (CHUNK_SIDE_LENGTH + 1)*(CHUNK_SIDE_LENGTH + 1);
	unsigned int rowSize = (CHUNK_SIDE_LENGTH + 1);

	// Create index data
	m_pIndices = new unsigned int[CHUNK_INDEX_COUNT];
	if( !m_pIndices ) {
		PrintError( L"Memory error while generating chunk indices.\n " );
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
				if( !this->isBlockVisible( currentBlock ) ) {
					firstIndex++;
					currentBlock++;
					continue;
				}
				// BOTTOM
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + rowSize + 1;
				m_pIndices[currentIndex++] = firstIndex + 1;
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + rowSize;
				m_pIndices[currentIndex++] = firstIndex + rowSize + 1;
				m_indexCount += 6;
				// TOP
				m_pIndices[currentIndex++] = firstIndex + layerSize;
				m_pIndices[currentIndex++] = firstIndex + layerSize + 1;
				m_pIndices[currentIndex++] = firstIndex + layerSize + rowSize + 1;
				m_pIndices[currentIndex++] = firstIndex + layerSize;
				m_pIndices[currentIndex++] = firstIndex + layerSize + rowSize + 1;
				m_pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				m_indexCount += 6;
				// FRONT
				m_pIndices[currentIndex++] = firstIndex + 1;
				m_pIndices[currentIndex++] = firstIndex + 1 + layerSize + rowSize;
				m_pIndices[currentIndex++] = firstIndex + 1 + layerSize;
				m_pIndices[currentIndex++] = firstIndex + 1;
				m_pIndices[currentIndex++] = firstIndex + 1 + rowSize;
				m_pIndices[currentIndex++] = firstIndex + 1 + layerSize + rowSize;
				// BACK
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + layerSize;
				m_pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + layerSize + rowSize;
				m_pIndices[currentIndex++] = firstIndex + rowSize;
				// LEFT
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + layerSize + 1;
				m_pIndices[currentIndex++] = firstIndex + layerSize;
				m_pIndices[currentIndex++] = firstIndex;
				m_pIndices[currentIndex++] = firstIndex + 1;
				m_pIndices[currentIndex++] = firstIndex + layerSize + 1;
				// RIGHT
				m_pIndices[currentIndex++] = firstIndex + rowSize + layerSize;
				m_pIndices[currentIndex++] = firstIndex + rowSize + layerSize + 1;
				m_pIndices[currentIndex++] = firstIndex + rowSize;
				m_pIndices[currentIndex++] = firstIndex + rowSize;
				m_pIndices[currentIndex++] = firstIndex + rowSize + layerSize + 1;
				m_pIndices[currentIndex++] = firstIndex + rowSize + 1;

				m_indexCount += 24;
				currentBlock++;

				firstIndex++;
			}
			firstIndex++;
		}
	}

	return true;
}
bool CChunk::sendDataToBuffers()
{
	unsigned int vaoId, vboId, iboId;
	ChunkVertex *pVertices;
	unsigned int *pIndices;

	CGame::instance().getGraphics()->getWorld()->getChunkManager()->getBufferIds( m_bufferIndex, &vaoId, &vboId, &iboId );

	glBindVertexArray( vaoId );
	// VERTICES
	// Get a pointer to the data
	glBindBuffer( GL_ARRAY_BUFFER, vboId );
	pVertices = reinterpret_cast<ChunkVertex*>(glMapBufferRange( GL_ARRAY_BUFFER, sizeof( ChunkVertex )*m_vertexOffset, sizeof( ChunkVertex )*CHUNK_VERTEX_COUNT, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT ));
	if( !pVertices ) {
		PrintError( L"Failed to update terrain\n" );
		return false;
	}
	memcpy( pVertices, m_pVertices, sizeof( ChunkVertex )*m_vertexCount );
	// Finish
	glUnmapBuffer( GL_ARRAY_BUFFER );

	// INDICES
	// Get a pointer to the data
	glBindBuffer( GL_ARRAY_BUFFER, iboId );
	pIndices = reinterpret_cast<unsigned int*>(glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, sizeof( unsigned int )*m_indexOffset, sizeof( unsigned int )*CHUNK_INDEX_COUNT, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT ));
	if( !pIndices ) {
		PrintError( L"Failed to update terrain\n" );
		return false;
	}
	memcpy( pIndices, pIndices, sizeof( unsigned int )*CHUNK_INDEX_COUNT );
	// Finish
	glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

	if( m_pVertices )
		SAFE_DELETE_A( m_pVertices );
	if( m_pIndices )
		SAFE_DELETE_A( m_pIndices );

	return true;
}

void CChunk::setRawData( unsigned short *pData )
{
	CChunkManager *pChunkManager = CGame::instance().getGraphics()->getWorld()->getChunkManager();

	boost::unique_lock<boost::shared_mutex> lock( m_mutex_ );
	for( unsigned int i = 0; i < CHUNK_BLOCK_COUNT; i++ ) {
		m_blocks[i] = pChunkManager->getBlockById( pData[i] );
	}
}

OcclusionList CChunk::getOcclusionList()
{
	OcclusionList occlusionList;
	std::array<unsigned char, CHUNK_BLOCK_COUNT> occlusionFlags;
	GLuint verticesRequired;

	int index = 0;
	verticesRequired = CHUNK_VERTEX_COUNT;
	for( auto it = m_blocks.begin(); it != m_blocks.end(); it++, index++ )
	{
		occlusionFlags[index] = 0;
		if( !this->isBlockVisible( index, true ) ) {
			verticesRequired -= 36;
			continue;
		}

		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_UP, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_UP;
			verticesRequired -= 6;
		}
		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_DOWN, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_DOWN;
			verticesRequired -= 6;
		}
		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_FRONT, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_FRONT;
			verticesRequired -= 6;
		}
		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_BACK, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_BACK;
			verticesRequired -= 6;
		}
		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_LEFT, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_LEFT;
			verticesRequired -= 6;
		}
		if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_RIGHT, true ) ) ) {
			occlusionFlags[index] |= CHUNK_DIRECTION_RIGHT;
			verticesRequired -= 6;
		}
	}

	occlusionList.occlusionFlags = occlusionFlags;
	occlusionList.verticesRequired = verticesRequired;

	return occlusionList;
}

bool CChunk::isBlockVisible( glm::vec3 pos, bool onlyInChunk ) {
	size_t index = this->getBlockIndex( pos );
	if( index >= CHUNK_BLOCK_COUNT )
		return false;
	return this->isBlockVisible( index, onlyInChunk );
}
bool CChunk::isBlockVisible( size_t index, bool onlyInChunk )
{
	// Make sure it exists
	if( !this->getBlockAt( index ) )
		return false;
	// If the its neighbors occlude it, it isn't visible
	if( CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_UP, onlyInChunk ) ) && CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_DOWN, onlyInChunk ) ) &&
		CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_FRONT, onlyInChunk ) ) && CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_BACK, onlyInChunk ) ) &&
		CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_RIGHT, onlyInChunk ) ) && CChunk::isOccludingBlock( this->getBlockNeighbor( index, CHUNK_DIRECTION_LEFT, onlyInChunk ) ) )
		return false;

	return true;
}

size_t CChunk::getBlockIndex( glm::vec3 pos ) {
	return (size_t)(pos.y*(CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH)+pos.x*CHUNK_SIDE_LENGTH+pos.z);
}
CBlock* CChunk::getBlockNeighbor( glm::vec3 pos, char direction, bool onlyInChunk ) {
	return this->getBlockNeighbor( this->getBlockIndex( pos ), direction, onlyInChunk );
}
CBlock* CChunk::getBlockNeighbor( size_t index, char direction, bool onlyInChunk )
{
	CChunkManager *pManager = CGame::instance().getGraphics()->getWorld()->getChunkManager();
	CChunk *pNeighbor;
	size_t neighborIndex;
	int row, layer;
	glm::ivec2 gridPos = this->getChunkGridPos();

	//row = (int)(index / CHUNK_SIDE_LENGTH);
	layer = (int)(index / (CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH));
	row = (int)(index - layer*(CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH)) / CHUNK_SIDE_LENGTH;

	switch( direction )
	{
	case CHUNK_DIRECTION_RIGHT:
		// if its at the edge of the chunk
		if( index % CHUNK_SIDE_LENGTH == (CHUNK_SIDE_LENGTH-1) ) {
			pNeighbor = pManager->getChunkNeighbor( gridPos, CHUNK_DIRECTION_RIGHT );
			if( pNeighbor && !onlyInChunk )
				return pNeighbor->getBlockAt( index - (CHUNK_SIDE_LENGTH - 1) );
			return NULL; 
		}
		neighborIndex = index+1;
		break;
	case CHUNK_DIRECTION_LEFT:
		// if its at the edge of the chunk
		if( index % CHUNK_SIDE_LENGTH == 0 ) {
			pNeighbor = pManager->getChunkNeighbor( gridPos, CHUNK_DIRECTION_LEFT );
			if( pNeighbor && !onlyInChunk )
				return pNeighbor->getBlockAt( index + CHUNK_SIDE_LENGTH-1 );
			return NULL;
		} 
		neighborIndex = index-1;
		break;
	case CHUNK_DIRECTION_BACK:
		// if its at the edge of the chunk
		if( row == CHUNK_SIDE_LENGTH-1 ) { //  
			pNeighbor = pManager->getChunkNeighbor( gridPos, CHUNK_DIRECTION_BACK );
			if( pNeighbor && !onlyInChunk )
				return pNeighbor->getBlockAt( index - CHUNK_SIDE_LENGTH*(CHUNK_SIDE_LENGTH-1) );
			return NULL;
		}
		neighborIndex = index+CHUNK_SIDE_LENGTH;
		break;
	case CHUNK_DIRECTION_FRONT:
		// if its at the edge of the chunk
		if( row == 0 ) { //  
			pNeighbor = pManager->getChunkNeighbor( gridPos, CHUNK_DIRECTION_FRONT );
			if( pNeighbor && !onlyInChunk )
				return pNeighbor->getBlockAt( index + CHUNK_SIDE_LENGTH*(CHUNK_SIDE_LENGTH-1) );
			return NULL;
		}
		neighborIndex = index-CHUNK_SIDE_LENGTH;
		break;
	case CHUNK_DIRECTION_UP:
		// if its at the edge of the chunk
		if( layer == CHUNK_HEIGHT-1 )
			return NULL;
		neighborIndex = index+CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH;
		break;
	case CHUNK_DIRECTION_DOWN:
		// if its at the edge of the chunk
		if( layer == 0 )
			return NULL;
		neighborIndex = index-CHUNK_SIDE_LENGTH*CHUNK_SIDE_LENGTH;
		break;
	default:
		PrintWarn( L"Unknown block direction to getBlockNeighbor\n" );
		return NULL;
	}
	boost::shared_lock_guard<boost::shared_mutex> lock( m_mutex_ );
	if( neighborIndex < m_blocks.size() )
		return m_blocks[neighborIndex];
	return NULL;
}
CBlock* CChunk::getBlockAt( size_t index ) {
	boost::shared_lock_guard<boost::shared_mutex> lock( m_mutex_ );
	if( index < m_blocks.size() )
		return m_blocks[index];
	return NULL;
}
CBlock* CChunk::getBlockAt( glm::vec3 pos ) {
	return this->getBlockAt( this->getBlockIndex( pos ) );
}
GLuint CChunk::getIndexCount() {
	return m_indexCount;
}
size_t CChunk::getBufferIndex() {
	return m_bufferIndex;
}
GLuint CChunk::getVertexOffset() {
	return m_vertexOffset;
}
GLuint CChunk::getVertexCount() {
	return m_vertexCount;
}

#pragma endregion