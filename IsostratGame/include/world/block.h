#pragma once

class CBlock
{
private:
	unsigned short m_blockId;
	unsigned short m_blockSubId;
public:
	CBlock() {}
	CBlock( unsigned short id );
	~CBlock();

	unsigned short getBlockId();
	unsigned short getBlockSubId();
};