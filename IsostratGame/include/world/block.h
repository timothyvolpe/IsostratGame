#pragma once

#include <glm\glm.hpp>

class CBlock
{
private:
	unsigned short m_blockId;
	unsigned short m_blockSubId;

	glm::ivec3 m_blockColor;
	bool m_opaque;
public:
	CBlock() {}
	CBlock( unsigned short id );
	~CBlock();

	unsigned short getBlockId();
	unsigned short getBlockSubId();

	glm::ivec3 getBlockColor();
	void setBlockColor( glm::ivec3 blockColor );
	inline bool isOpaque() { return m_opaque; }
};