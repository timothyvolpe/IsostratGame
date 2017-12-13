#include "base.h"
#include "def.h"
#include "world\world.h"
#include "world\chunkmanager.h"

#include <boost\filesystem.hpp>

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

bool CWorld::loadSave( std::wstring saveName )
{
	boost::filesystem::path fullPath, terrainPath;

	// Get the save folder
	fullPath = boost::filesystem::current_path();
	fullPath /= FILESYSTEM_SAVEDIR;
	fullPath /= saveName;

	PrintInfo( L"Loading save file %s...\n", saveName.c_str() );

	if( !m_pChunkManager->setSaveFile( saveName ) ) {
		PrintError( L"Failed to load save file %s\n", saveName.c_str() );
		return false;
	}

	return true;
}

void CWorld::update() {
	m_pChunkManager->update();
}
void CWorld::draw( glm::mat4 projection, glm::mat4 view ) {
	m_pChunkManager->draw( projection, view );
}

CChunkManager* CWorld::getChunkManager() {
	return m_pChunkManager;
}
