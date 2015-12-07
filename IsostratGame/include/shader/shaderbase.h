#pragma once

#include <string>

enum : unsigned char
{
	PROGRAM_USEVERTEX		= 1 << 0,
	PROGRAM_USEFRAGMENT		= 1 << 1,
	PROGRAM_USEGEOMETRY		= 1 << 2
};

////////////////////
// CShaderManager //
////////////////////

class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();

	bool initialize();
	void destroy();
};

/////////////////
// CShaderBase //
/////////////////

class CShaderBase
{
public:
	CShaderBase();
	~CShaderBase();

	bool initializeProgram( std::string programName, unsigned char shaderFlags );
	void destroyProgram();
};