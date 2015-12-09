#pragma once

#include <GL\glew.h>

#include <string>
#include <list>
#include <vector>

enum : unsigned char
{
	PROGRAM_USEVERTEX		= 1 << 0,
	PROGRAM_USEFRAGMENT		= 1 << 1,
	PROGRAM_USEGEOMETRY		= 1 << 2
};

enum : unsigned char
{
	SHADERPROGRAM_SIMPLE = 0,
	SHADERPROGRAM_COUNT
};

class CShaderObject;
class CShaderProgram;

////////////////////
// CShaderManager //
////////////////////

class CShaderManager
{
private:
	struct ShaderProgramDesc {
		std::wstring name;
		char vertexShader, geometryShader, fragmentShader;
		ShaderProgramDesc() {};
		ShaderProgramDesc( std::wstring n, char vert, char geom, char frag ) : name( n ), vertexShader( vert ), geometryShader( geom ), fragmentShader( frag ) {}
	};

	typedef std::list<std::pair<std::string, GLenum>> ShaderFileList;
	typedef std::vector<ShaderProgramDesc> ShaderProgramDescs;
	typedef std::vector<CShaderObject*> ShaderObjectList;
	typedef std::vector<CShaderProgram*> ShaderProgramList;

	ShaderObjectList m_shaderObjects;
	ShaderProgramList m_programObjects;

	bool loadShaderObjects( ShaderFileList shadersToLoad );
	void destroyShaderObjects();

	bool loadPrograms( ShaderProgramDescs programDescs );
public:
	CShaderManager();
	~CShaderManager();

	bool initialize();
	void destroy();

	CShaderProgram* getProgram( char programIndex );
};

///////////////////
// CShaderObject //
///////////////////

class CShaderObject
{
private:
	std::wstring m_name;
	GLenum m_type;

	GLuint m_shaderId;
public:
	CShaderObject();
	~CShaderObject();

	bool initializeShader( std::string file, GLenum type);
	void destroy();

	GLuint getShaderId();
};

////////////////////
// CShaderProgram //
////////////////////

class CShaderProgram
{
private:
	std::wstring m_name;

	GLuint m_programId;
public:
	CShaderProgram();
	~CShaderProgram();

	bool initializeProgram( std::wstring name, CShaderObject *pVertexShader, CShaderObject *pGeometryShader, CShaderObject *pFragmentShader );
	void destroy();

	void bind();
};