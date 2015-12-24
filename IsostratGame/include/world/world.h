#pragma once

#include <glm\glm.hpp>

class CChunkManager;

class CWorld
{
private:
	CChunkManager *m_pChunkManager;
public:
	CWorld();
	~CWorld();

	bool initialize();
	void destroy();

	bool loadWorld();
	void destroyWorld();

	bool loadSave( std::wstring saveName );

	void draw( glm::mat4 projection, glm::mat4 view );

	CChunkManager* getChunkManager();
};