#include "base.h"
#include "world\block.h"

CBlock::CBlock( unsigned short id ) {
	m_blockId = id;
	m_blockColor = glm::ivec3( 120, 120, 120 );
	m_opaque = true;
}
CBlock::~CBlock() {
}

unsigned short CBlock::getBlockId() {
	return m_blockId;
}
unsigned short CBlock::getBlockSubId() {
	return m_blockSubId;
}

glm::ivec3 CBlock::getBlockColor() {
	return m_blockColor;

}
void CBlock::setBlockColor( glm::ivec3 blockColor ) {
	m_blockColor = blockColor;
}