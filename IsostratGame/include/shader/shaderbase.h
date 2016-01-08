#pragma once

#include <GL\glew.h>
#include <glm\glm.hpp>

#include <string>
#include <list>
#include <vector>
#include <map>

enum : unsigned char
{
	PROGRAM_USEVERTEX		= 1 << 0,
	PROGRAM_USEFRAGMENT		= 1 << 1,
	PROGRAM_USEGEOMETRY		= 1 << 2
};

// Add new shader programs here
enum : unsigned char
{
	SHADERPROGRAM_SIMPLE		= 0,
	SHADERPROGRAM_CHUNK			= 1,
	SHADERPROGRAM_INTERFACE		= 2,
	SHADERPROGRAM_DEBUG			= 3,
	SHADERPROGRAM_COUNT
};

enum
{
	UNIFORMBLOCK_GLOBALMATRICES = 0,
	UNIFORMBLOCK_COUNT
};

typedef struct
{
	glm::mat4 mvp;
	glm::mat4 mvp_ortho;
} UBGlobalMatrices;

class CShaderObject;
class CShaderProgram;

////////////////////
// CShaderManager //
////////////////////

typedef std::list<std::pair<std::string, GLuint>> UniformBlockList;

class CShaderManager
{
private:
	typedef std::list<std::pair<std::string, GLenum>> ShaderFileList;
	typedef std::vector<CShaderObject*> ShaderObjectList;
	typedef std::vector<CShaderProgram*> ShaderProgramList;

	struct ShaderProgramDesc {
		std::wstring name;
		char vertexShader, geometryShader, fragmentShader;
		UniformBlockList uniformBlocks;
		std::vector<std::string> uniformNames;
		ShaderProgramDesc() {};
		ShaderProgramDesc( std::wstring n, char vert, char geom, char frag, UniformBlockList ub, std::vector<std::string> un ) : name( n ), vertexShader( vert ), geometryShader( geom ), fragmentShader( frag ), uniformBlocks( ub ), uniformNames( un ) {}
	};
	typedef std::vector<ShaderProgramDesc> ShaderProgramDescs;
	
	ShaderObjectList m_shaderObjects;
	ShaderProgramList m_programObjects;

	std::vector<GLuint> m_uniformBuffers;

	bool loadShaderObjects( ShaderFileList shadersToLoad );
	void destroyShaderObjects();

	bool loadPrograms( ShaderProgramDescs programDescs );
public:
	UBGlobalMatrices m_ubGlobalMatrices;

	CShaderManager();
	~CShaderManager();

	bool initialize();
	void destroy();

	bool updateUniformBlock( GLuint index );

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

	bool initializeShader( std::string file, GLenum type );
	void destroy();

	GLuint getShaderId();
};

////////////////////
// CShaderProgram //
////////////////////

class CShaderProgram
{
private:
	typedef std::map<GLuint, GLuint> UniformBlockIndices;
	typedef std::map<std::string, GLuint> UniformList;

	std::wstring m_name;

	GLuint m_programId;

	UniformBlockIndices m_uniformBlocks;
	UniformList m_uniforms;
public:
	CShaderProgram();
	~CShaderProgram();

	bool initializeProgram( std::wstring name, CShaderObject *pVertexShader, CShaderObject *pGeometryShader, CShaderObject *pFragmentShader, UniformBlockList uniformBlocks, std::vector<std::string> uniformNames );
	void destroy();

	void bind();

	GLuint getUniform( std::string name );
};