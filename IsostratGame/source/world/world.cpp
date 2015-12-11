#include "base.h"
#include "world\world.h"
#include "world\chunkmanager.h"

CWorld::CWorld() {
	m_pChunkManager = NULL;
}
CWorld::~CWorld() {
}

bool CWorld::initialize() {
	return true;
}
void CWorld::destroy() {
}

bool CWorld::loadWorld()
{
	PrintInfo( L"Creating world...\n" );

	// Create the chunk manager
	m_pChunkManager = new CChunkManager();
	if( !m_pChunkManager->initialize() )
		return false;
	return true;
}
void CWorld::destroyWorld()
{
	PrintInfo( L"Destroying world...\n" );
	DESTROY_DELETE( m_pChunkManager );
}

void CWorld::draw( glm::mat4 projection, glm::mat4 view ) {
	m_pChunkManager->draw( projection, view );
}

CChunkManager* CWorld::getChunkManager() {
	return m_pChunkManager;
}
