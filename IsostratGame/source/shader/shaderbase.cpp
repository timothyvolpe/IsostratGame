#include "base.h"
#include "shader/shaderbase.h"

////////////////////
// CShaderManager //
////////////////////

CShaderManager::CShaderManager() {
}
CShaderManager::~CShaderManager() {
}

bool CShaderManager::initialize()
{
	// TODO: Load the shaders

	// Create a list of all the shaders to load, and then create a
	// list of all the programs and which shaders they use, using
	// an integer to reference by index

	return true;
}
void CShaderManager::destroy()
{

}

/////////////////
// CShaderBase //
/////////////////

CShaderBase::CShaderBase() {
}
CShaderBase::~CShaderBase() {
}

bool CShaderBase::initializeProgram( std::string programName, unsigned char shaderFlags )
{
	PrintInfo( L"Loading shader program \"%hs\"...", programName.c_str() );

	// Load each shader

	return true;
}
void CShaderBase::destroyProgram()
{
}