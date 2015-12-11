#include "base.h"
#include "def.h"
#include "shader/shaderbase.h"

#include <fstream>
#include <streambuf>
#include <boost\filesystem.hpp>

////////////////////
// CShaderManager //
////////////////////

CShaderManager::CShaderManager()
{
	m_ubGlobalMatrices.mvp = glm::mat4( 1.0f );
	m_ubGlobalMatrices.mvp_ortho = glm::mat4( 1.0f );
}
CShaderManager::~CShaderManager() {
}

bool CShaderManager::loadShaderObjects( ShaderFileList shadersToLoad )
{
	this->destroyShaderObjects();

	// Load each shader object
	for( auto it = shadersToLoad.begin(); it != shadersToLoad.end(); it++ ) {
		CShaderObject *pShaderObj;
		pShaderObj = new CShaderObject();
		if( !pShaderObj->initializeShader( (*it).first, (*it).second ) )
			return false;
		m_shaderObjects.push_back( pShaderObj );
	}

	return true;
}
void CShaderManager::destroyShaderObjects() {
	// Destroy each object
	for( auto it = m_shaderObjects.begin(); it != m_shaderObjects.end(); it++ ) {
		DESTROY_DELETE( (*it) );
	}
	m_shaderObjects.clear();
}

bool CShaderManager::loadPrograms( ShaderProgramDescs programDescs )
{
	// Create each program
	for( auto it = programDescs.begin(); it != programDescs.end(); it++ )
	{
		CShaderProgram *pProgram;
		CShaderObject *pVert, *pGeom, *pFrag;

		// Find the shader objects
		pVert = ((*it).vertexShader == -1) ? NULL : m_shaderObjects[(*it).vertexShader];
		pGeom = ((*it).geometryShader == -1) ? NULL : m_shaderObjects[(*it).geometryShader];
		pFrag = ((*it).fragmentShader == -1) ? NULL : m_shaderObjects[(*it).fragmentShader];

		// Create the program
		pProgram = new CShaderProgram();
		if( !pProgram->initializeProgram( (*it).name, pVert, pGeom, pFrag, (*it).uniformBlocks, (*it).uniformNames ) )
			return false;
		m_programObjects.push_back( pProgram );
	}
	return true;
}

bool CShaderManager::initialize()
{
	ShaderFileList shaderList;
	ShaderProgramDescs programDescList;
	boost::filesystem::path logDirectory, compileLog, linkLog;

	// Clear the shader log files
	logDirectory = boost::filesystem::current_path();
	logDirectory /= FILESYSTEM_LOGDIR;
	compileLog = logDirectory / "compile.log";
	linkLog = logDirectory / "link.log";

	try
	{
		// Clear compile log
		if( boost::filesystem::exists( compileLog ) ) {
			std::ofstream outStream( compileLog.string().c_str(), std::ios::trunc );
			outStream.close();
		}
		// Clear link log
		if( boost::filesystem::exists( linkLog ) ) {
			std::ofstream outStream( linkLog.string().c_str(), std::ios::trunc );
			outStream.close();
		}
	}
	catch( std::ofstream::failure &e ) {
		PrintWarn( L"Failed to clear the log files (%hs)\n", e.what() );
	}

	// Shader objects to load
	shaderList.push_back( std::pair<std::string, GLenum>( "simple", GL_VERTEX_SHADER ) );// 0
	shaderList.push_back( std::pair<std::string, GLenum>( "simple", GL_FRAGMENT_SHADER ) ); // 1
	shaderList.push_back( std::pair<std::string, GLenum>( "chunk", GL_VERTEX_SHADER ) ); // 2
	enum {
		NO_SHADER = -1,
		SIMPLE_VERT = 0,
		SIMPLE_FRAG = 1,
		CHUNK_VERT = 2
	};

	// Load the shader objects
	if( !this->loadShaderObjects( shaderList ) )
		return false;

	// Create programs with the shader objects
	// Shader objects are reference by created order, -1 is unused
	programDescList.resize( SHADERPROGRAM_COUNT );

	// Shader Program SIMPLE
	UniformBlockList simple_uniformBlocks;
	std::vector<std::string> simple_uniforms;
	simple_uniformBlocks.push_back( std::pair<std::string, GLuint>( "GlobalMatrices", UNIFORMBLOCK_GLOBALMATRICES ) );
	programDescList[SHADERPROGRAM_SIMPLE] = ShaderProgramDesc( L"SIMPLE", SIMPLE_VERT, NO_SHADER, SIMPLE_FRAG, simple_uniformBlocks, simple_uniforms );
	// Shader Program CHUNK
	UniformBlockList chunk_uniformBlocks;
	std::vector<std::string> chunk_uniforms;
	chunk_uniformBlocks.push_back( std::pair<std::string, GLuint>( "GlobalMatrices", UNIFORMBLOCK_GLOBALMATRICES ) );
	chunk_uniforms.push_back( "voxelScale" );
	programDescList[SHADERPROGRAM_CHUNK] = ShaderProgramDesc( L"CHUNK", CHUNK_VERT, NO_SHADER, SIMPLE_FRAG, chunk_uniformBlocks, chunk_uniforms );

	if( !this->loadPrograms( programDescList ) )
		return false;

	// Destroy the shader objects as we dont need them anymore
	this->destroyShaderObjects();

	// Create the uniform buffers
	m_uniformBuffers.resize( UNIFORMBLOCK_COUNT );
	glGenBuffers( m_uniformBuffers.size(), &m_uniformBuffers[0] );
	// GlobalMatrices
	glBindBuffer( GL_UNIFORM_BLOCK, m_uniformBuffers[UNIFORMBLOCK_GLOBALMATRICES] );
	glBufferData( GL_UNIFORM_BLOCK, sizeof( UBGlobalMatrices ), 0, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, UNIFORMBLOCK_GLOBALMATRICES, m_uniformBuffers[UNIFORMBLOCK_GLOBALMATRICES] );

	return true;
}
void CShaderManager::destroy()
{
	// Should already be destroyed, but try anyway
	this->destroyShaderObjects();
	// Destroy programs
	PrintInfo( L"Destroying shader programs...\n" );
	for( auto it = m_programObjects.begin(); it != m_programObjects.end(); it++ ) {
		DESTROY_DELETE( (*it) );
	}
	m_programObjects.clear();
}

bool CShaderManager::updateUniformBlock( GLuint index )
{
	switch( index )
	{
	case UNIFORMBLOCK_GLOBALMATRICES:
		glBindBuffer( GL_UNIFORM_BUFFER, m_uniformBuffers[UNIFORMBLOCK_GLOBALMATRICES] );
		glBufferData( GL_UNIFORM_BUFFER, sizeof( m_ubGlobalMatrices ), &m_ubGlobalMatrices, GL_DYNAMIC_DRAW );
		break;
	default:
		PrintWarn( L"Attempted to update invalid uniform block\n" );
		return false;
	}
	return true;
}

CShaderProgram* CShaderManager::getProgram( char programIndex ) {
	return m_programObjects[programIndex];
}

///////////////////
// CShaderObject //
///////////////////

CShaderObject::CShaderObject() {
	m_name = L"UNNAMED";
	m_type = 0;

	m_shaderId = 0;
}
CShaderObject::~CShaderObject() {
}

bool CShaderObject::initializeShader( std::string name, GLenum type )
{
	boost::filesystem::path fullPath;
	std::string shaderSource;
	GLint compileStatus, logLength;

	m_type = type;
	// Copy the name
	m_name.resize( name.length() );
	std::copy( name.begin(), name.end(), m_name.begin() );

	PrintInfo( L"Loading shader object \"%s\"...", m_name.c_str() );

	// Determine the shader type
	switch( type )
	{
	case GL_VERTEX_SHADER:
		PrintInfo( L"(VERT)\n" );
		name += ".vert";
		break;
	case GL_GEOMETRY_SHADER:
		PrintInfo( L"(GEOM)\n" );
		name += ".geom";
		break;
	case GL_FRAGMENT_SHADER:
		PrintInfo( L"(FRAG)\n" );
		name += ".frag";
		break;
	default:
		PrintError( L"Unknown shader type for shader \"%s\"\n", m_name.c_str() );
		return false;
	}

	// Construct the full path
	fullPath = boost::filesystem::current_path();
	fullPath /= FILESYSTEM_SHADERDIR;
	fullPath /= name;

	// Load the source
	try
	{
		// Make sure it exists and is valid
		if( boost::filesystem::exists( fullPath ) ) {
			if( !boost::filesystem::is_regular_file( fullPath ) ) {
				PrintError( L"Failed to load shader because the file is not valid \"%s\"\n", fullPath.wstring().c_str() );
				return false;
			}
		}
		else {
			PrintError( L"Failed to load shader because the file does not exists \"%s\"\n", fullPath.wstring().c_str() );
			return false;
		}

		// Read the contents of the file
		std::ifstream inStream( fullPath.string().c_str() );
		// Get length
		inStream.seekg( 0, std::ios::end );
		shaderSource.reserve( (unsigned int)inStream.tellg() );
		inStream.seekg( 0, std::ios::beg );
		// Read the file
		shaderSource.assign( std::istreambuf_iterator<char>( inStream ), std::istreambuf_iterator<char>() );
		inStream.close();
	}
	catch( std::ifstream::failure &e ) {
		PrintError( L"Failed to load shader because the file could not be read: %hs\n", e.what() );
		return false;
	}
	catch( boost::filesystem::filesystem_error &e ) {
		PrintError( L"Failed to load shader because the file could not be read: %hs\n", e.what() );
		return false;
	}

	// Create the shader
	m_shaderId = glCreateShader( m_type );
	if( !m_shaderId ) {
		PrintError( L"OpenGL failed to create shader\n" );
		return false;
	}
	// Compile the source
	const char *source_str = shaderSource.c_str();
	glShaderSource( m_shaderId, 1, &source_str, 0 );
	glCompileShader( m_shaderId );

	// Make sure the source compiled correctly
	glGetShaderiv( m_shaderId, GL_COMPILE_STATUS, &compileStatus );
	if( compileStatus == GL_FALSE )
	{
		glGetShaderiv( m_shaderId, GL_INFO_LOG_LENGTH, &logLength );
		if( logLength > 0 )
		{
			std::vector<GLchar> infoLog(logLength);
			boost::filesystem::path logPath;
			std::string compileLogHeader;

			glGetShaderInfoLog( m_shaderId, logLength, 0, &infoLog[0] );
			// Write to the log file
			logPath = boost::filesystem::current_path();
			logPath /= FILESYSTEM_LOGDIR;
			// Check if it exists and if not, create
			if( !boost::filesystem::is_directory( logPath ) )
				boost::filesystem::create_directory( logPath );
			logPath /= "compile.log";
			try {
				std::ofstream outStream( logPath.string().c_str(), std::ios::app );
				compileLogHeader = ">" + name + "\n";
				outStream.write( compileLogHeader.c_str(), compileLogHeader.length() );
				outStream.write( &infoLog[0], logLength );
				outStream.close();
			}
			catch( std::ofstream::failure &e ) {
				PrintError( L"Failed to write to compile.log (%hs)\n", e.what() );
				return false;
			}

			PrintError( L"Failed to compile shader \"%hs\", see compile.log\n", name.c_str() );
		}
		else {
			PrintError( L"Failed to compile shader \"%hs\", no log from OpenGL\n", name.c_str() );
			return false;
		}
		glDeleteShader( m_shaderId );
		m_shaderId = 0;
		return false;
	}

	return true;
}
void CShaderObject::destroy()
{
	// Delete the shader object
	if( m_shaderId ) {
		glDeleteShader( m_shaderId );
		m_shaderId = 0;
	}
	m_type = 0;
	m_name = L"DELETED";
}

GLuint CShaderObject::getShaderId() {
	return m_shaderId;
}

////////////////////
// CShaderProgram //
////////////////////

CShaderProgram::CShaderProgram() {
	m_name = L"UNNAMED";
	m_programId = 0;
}
CShaderProgram::~CShaderProgram() {
}

bool CShaderProgram::initializeProgram( std::wstring name, CShaderObject *pVertexShader, CShaderObject *pGeometryShader, CShaderObject *pFragmentShader, UniformBlockList uniformBlocks, std::vector<std::string> uniformNames )
{
	GLint linkStatus, logLength;

	m_name = name;
	PrintInfo( L"Linking shader program \"%s\"...\n", m_name.c_str() );

	// Create the program
	m_programId = glCreateProgram();
	if( !m_programId ) {
		PrintError( L"OpenGL failed to create a shader program\n" );
		return false;
	}
	// Attach the shaders
	if( pVertexShader )
		glAttachShader( m_programId, pVertexShader->getShaderId() );
	if( pGeometryShader )
		glAttachShader( m_programId, pGeometryShader->getShaderId() );
	if( pFragmentShader )
		glAttachShader( m_programId, pFragmentShader->getShaderId() );

	// Link the program
	glLinkProgram( m_programId );
	// Make sure it was successful
	glGetProgramiv( m_programId, GL_LINK_STATUS, &linkStatus );
	if( linkStatus == GL_FALSE )
	{
		glGetProgramiv( m_programId, GL_INFO_LOG_LENGTH, &logLength );
		if( logLength > 0 )
		{
			std::vector<GLchar> infoLog( logLength );
			boost::filesystem::path logPath;

			glGetProgramInfoLog( m_programId, logLength, 0, &infoLog[0] );
			// Write to the log file
			logPath = boost::filesystem::current_path();
			logPath /= FILESYSTEM_LOGDIR;
			// Check if it exists and if not, create
			if( !boost::filesystem::is_directory( logPath ) )
				boost::filesystem::create_directory( logPath );
			logPath /= "link.log";
			try {
				std::ofstream outStream( logPath.string().c_str(), std::ios::app );
				outStream.write( &infoLog[0], logLength );
				outStream.close();
			}
			catch( std::ofstream::failure &e ) {
				PrintError( L"Failed to write to link.log (%hs)\n", e.what() );
				return false;
			}

			PrintError( L"Failed to link shader program \"%s\", see link.log\n", m_name.c_str() );
			return false;
		}
		else {
			PrintError( L"Failed to link shader program \"%s\", no log from OpenGL\n", m_name.c_str() );
			return false;
		}
		glDeleteProgram( m_programId );
		m_programId = 0;
	}

	// Find the uniform blocks
	for( auto it = uniformBlocks.begin(); it != uniformBlocks.end(); it++ )
	{
		unsigned int index;

		// Get the location
		index = glGetUniformBlockIndex( m_programId, (*it).first.c_str() );
		if( index == GL_INVALID_INDEX ) {
			PrintWarn( L"Could not find uniform block \"%hs\", ignoring!\n", (*it).first.c_str() );
			continue;
		}
		// Bind it to the correct index
		glUniformBlockBinding( m_programId, index, (*it).second );
		// Add it to the list
		m_uniformBlocks.insert( std::pair<GLuint, GLuint>( index, (*it).second ) );
	}
	// Find uniforms
	for( auto it = uniformNames.begin(); it != uniformNames.end(); it++ )
	{
		unsigned int location;

		location = glGetUniformLocation( m_programId, (*it).c_str() );
		if( !location ) {
			PrintWarn( L"Could not find uniform \"%hs\", ignoring!\n", (*it).c_str() );
			continue;
		}
		// Save the location
		m_uniforms.insert( std::pair<std::string, GLuint>( (*it), location ) );
	}

	// Detach the shaders
	if( pVertexShader )
		glDetachShader( m_programId, pVertexShader->getShaderId() );
	if( pGeometryShader )
		glDetachShader( m_programId, pGeometryShader->getShaderId() );
	if( pFragmentShader )
		glDetachShader( m_programId, pFragmentShader->getShaderId() );

	return true;
}
void CShaderProgram::destroy()
{
	m_name = L"DELETED";
	if( m_programId ) {
		glDeleteProgram( m_programId );
		m_programId = 0;
	}
	m_uniformBlocks.clear();
	m_uniforms.clear();
}

void CShaderProgram::bind() {
	// TODO: Add a way to remove redundant calls
	glUseProgram( m_programId );
}

GLuint CShaderProgram::getUniform( std::string name ) {
	if( m_uniforms.find( name ) != m_uniforms.end() )
		return m_uniforms[name];
	else {
		PrintWarn( L"Could not find uniform \"%hs\"\n", name.c_str() );
		return 0;
	}
}