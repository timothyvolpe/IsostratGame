#include "base.h"
#include "world\block.h"

CBlock::CBlock( unsigned short id ) {
	m_blockId = id;
}
CBlock::~CBlock() {
}

unsigned short CBlock::getBlockId() {
	return m_blockId;
}
unsigned short CBlock::getBlockSubId() {
	return m_blockSubId;
}