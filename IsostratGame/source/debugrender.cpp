#include "debugrender.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "config.h"
#include "shader\shaderbase.h"

CDebugRender::CDebugRender()
{
	m_vertexArrayId = 0;
	m_vertexBufferId = 0;

	m_bEnabled = true;
}
CDebugRender::~CDebugRender()
{
}

bool CDebugRender::initialize()
{
	// Create vertex array and buffer
	glGenVertexArrays( 1, &m_vertexArrayId );
	glBindVertexArray( m_vertexArrayId );
	glGenBuffers( 1, &m_vertexBufferId );
	glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
	glBufferData( GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( DebugVertex ), (GLvoid*)offsetof( DebugVertex, pos ) );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( DebugVertex ), (GLvoid*)offsetof( DebugVertex, col ) );
	glEnableVertexAttribArray( 0 );
	glEnableVertexAttribArray( 1 );
	
	return true;
}

void CDebugRender::destroy()
{
	if( m_vertexArrayId ) {
		glDeleteVertexArrays( 1, &m_vertexArrayId );
		m_vertexArrayId = 0;
	}
	if( m_vertexBufferId ) {
		glDeleteBuffers( 1, &m_vertexBufferId );
		m_vertexBufferId = 0;
	}
	m_vertices.clear();
}

void CDebugRender::update()
{
	// Make sure it isnt toggled
	if( CGame::instance().getInput()->isKeyPress( CGame::instance().getConfigLoader()->getKeybind( KEYBIND_TOGGLE_DEBUGDRAW ) ) )
		m_bEnabled = !m_bEnabled;
}
void CDebugRender::draw( glm::mat4 projection, glm::mat4 view )
{
	CShaderManager *pShaderManager = CGame::instance().getGraphics()->getShaderManager();
	glm::mat4 modelMatrix;

	if( !m_bEnabled )
		return;
	if( m_vertices.size() == 0 )
		return;

	pShaderManager->getProgram( SHADERPROGRAM_DEBUG )->bind();
	modelMatrix = glm::mat4( 1.0f );
	pShaderManager->m_ubGlobalMatrices.mvp = projection * view * modelMatrix;
	pShaderManager->updateUniformBlock( UNIFORMBLOCK_GLOBALMATRICES );

	// Fill the vertex buffer
	glBindVertexArray( m_vertexArrayId );
	glBindBuffer( GL_ARRAY_BUFFER, m_vertexBufferId );
	glBufferData( GL_ARRAY_BUFFER, sizeof( DebugVertex ) * m_vertices.size(), &m_vertices[0], GL_DYNAMIC_DRAW );

	glLineWidth( 2.0f );
	glDrawArrays( GL_LINES, 0, m_vertices.size() );

	m_vertices.clear();
}

void CDebugRender::drawLine( glm::vec3 p1, glm::vec3 p2, glm::vec3 color )
{
	if( !m_bEnabled )
		return;

	DebugVertex v1, v2;
	v1.pos = p1;
	v1.col = color;
	v2.pos = p2;
	v2.col = color;
	m_vertices.push_back( v1 );
	m_vertices.push_back( v2 );
}

void CDebugRender::drawRect3d( glm::vec3 p1, glm::vec3 dim, glm::vec3 color )
{
	if( !m_bEnabled )
		return;

	std::vector<DebugVertex> prismVerts;

	prismVerts.resize( 12 * 2 ); // 12 edges, 2 verts per edge

	prismVerts[0].pos = p1;
	prismVerts[0].col = color;
	prismVerts[1].pos = p1 + glm::vec3( dim.x, 0.0f, 0.0f );
	prismVerts[1].col = color;
	prismVerts[2].pos = p1;
	prismVerts[2].col = color;
	prismVerts[3].pos = p1 + glm::vec3( 0.0f, dim.y, 0.0f );
	prismVerts[3].col = color;
	prismVerts[4].pos = p1;
	prismVerts[4].col = color;
	prismVerts[5].pos = p1 + glm::vec3( 0.0f, 0.0f, dim.z );
	prismVerts[5].col = color;
	prismVerts[6].pos = p1 + dim;
	prismVerts[6].col = color;
	prismVerts[7].pos = p1 + glm::vec3( dim.x, dim.y, 0.0f );
	prismVerts[7].col = color;
	prismVerts[8].pos = p1 + dim;
	prismVerts[8].col = color;
	prismVerts[9].pos = p1 + glm::vec3( 0.0f, dim.y, dim.z );
	prismVerts[9].col = color;
	prismVerts[10].pos = p1 + dim;
	prismVerts[10].col = color;
	prismVerts[11].pos = p1 + glm::vec3( dim.x, 0.0f, dim.z );
	prismVerts[11].col = color;
	prismVerts[12].pos = p1 + glm::vec3( 0.0f, dim.y, 0.0f );
	prismVerts[12].col = color;
	prismVerts[13].pos = p1 + glm::vec3( dim.x, dim.y, 0.0f );
	prismVerts[13].col = color;
	prismVerts[14].pos = p1 + glm::vec3( 0.0f, dim.y, 0.0f );
	prismVerts[14].col = color;
	prismVerts[15].pos = p1 + glm::vec3( 0.0f, dim.y, dim.z );
	prismVerts[15].col = color;
	prismVerts[16].pos = p1 + glm::vec3( dim.x, 0.0f, 0.0f );
	prismVerts[16].col = color;
	prismVerts[17].pos = p1 + glm::vec3( dim.x, dim.y, 0.0f );
	prismVerts[17].col = color;
	prismVerts[18].pos = p1 + glm::vec3( 0.0f, 0.0f, dim.z );
	prismVerts[18].col = color;
	prismVerts[19].pos = p1 + glm::vec3( 0.0f, dim.y, dim.z );
	prismVerts[19].col = color;
	prismVerts[20].pos = p1 + glm::vec3( dim.x, 0.0f, dim.z );
	prismVerts[20].col = color;
	prismVerts[21].pos = p1 + glm::vec3( 0.0f, 0.0f, dim.z );
	prismVerts[21].col = color;
	prismVerts[22].pos = p1 + glm::vec3( dim.x, 0.0f, dim.z );
	prismVerts[22].col = color;
	prismVerts[23].pos = p1 + glm::vec3( dim.x, 0.0f, 0.0f );
	prismVerts[23].col = color;

	m_vertices.insert( m_vertices.end(), prismVerts.begin(), prismVerts.end() );
}