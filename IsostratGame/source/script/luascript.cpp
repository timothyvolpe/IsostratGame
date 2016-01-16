#include "base.h"
#include "script\luascript.h"
#include "script\luautil.h"
#include "script\luagame.h"

/////////////////
// CLuaManager //
/////////////////

CLuaManager::CLuaManager() {
	m_pLuaClientState = NULL;
	m_pLuaServerState = NULL;
}
CLuaManager::~CLuaManager() {
}

bool CLuaManager::createLuaMethods( lua_State *pState, bool isServer )
{
	// load default libraries
	luaL_openlibs( pState );
	// Override default lib funcs
	lua_getglobal( pState, "_G" );
	luaL_setfuncs( pState, luaf_overridelibs, 0 );
	lua_pop( pState, 1 );

	// Set default globals
	// GAMEVERSION
	lua_pushnumber( pState, GAME_VERSION );
	lua_setglobal( pState, "GAMEVERSION" );
	// SERVER
	if( isServer )
		lua_pushboolean( pState, true );
	else
		lua_pushboolean( pState, false );
	lua_setglobal( pState, "SERVER" );
	// CLIENT
	if( isServer )
		lua_pushboolean( pState, false );
	else
		lua_pushboolean( pState, true );
	lua_setglobal( pState, "CLIENT" );

	// Game globoal
	luaL_newmetatable( pState, "Game" );
	luaL_getmetatable( pState, "Game" );
	luaL_setfuncs( pState, luaf_GameMetatable, 0 );
	lua_setglobal( pState, "Game" );
	lua_pop( pState, 1 );

	return true;
}

bool CLuaManager::initialize()
{
	boost::filesystem::path scriptPath;
	int luaError;

	// Create the lua states
	m_pLuaClientState = luaL_newstate();
	m_pLuaServerState = luaL_newstate();
	// Setup the lua methods
	if( !this->createLuaMethods( m_pLuaClientState, false ) )
		return false;

	scriptPath = boost::filesystem::current_path();
	scriptPath /= FILESYSTEM_LUADIR;

	// Load the main scripts
	if( !this->loadScript( scriptPath / L"interface\\main.lua", false ) )
		return false;

	// Run the scripts
	luaError = lua_pcall( m_pLuaClientState, 0, 0, 0 );
	if( luaError ) {
		PrintError( L"%hs\n", lua_tostring( m_pLuaClientState, -1 ) );
		return false;
	}

	return true;
}
void CLuaManager::destroy()
{
	for( auto it = m_scriptList.begin(); it != m_scriptList.end(); it++ ) {
		DESTROY_DELETE( (*it) );
	}
	m_scriptList.clear();
	// Shut down lua
	lua_close( m_pLuaClientState );
	m_pLuaClientState = NULL;
	lua_close( m_pLuaServerState );
	m_pLuaServerState = NULL;
}

CLuaScript* CLuaManager::loadScript( boost::filesystem::path file, bool isServer )
{
	CLuaScript *pLuaScript;

	// Create a new script
	pLuaScript = new CLuaScript();
	if( isServer ) {
		if( !pLuaScript->initialize( file, m_pLuaServerState ) )
			return false;
	}
	else {
		if( !pLuaScript->initialize( file, m_pLuaClientState ) )
			return false;
	}
	// Add to the list
	m_scriptList.push_back( pLuaScript );

	return pLuaScript;
}

///////////////
// CLuaSript //
///////////////

CLuaScript::CLuaScript() {
}
CLuaScript::~CLuaScript() {
}

bool CLuaScript::initialize( boost::filesystem::path file, lua_State *pLuaState )
{
	int luaError;

	// Make sure the file loads and works
	luaError = luaL_loadfile( pLuaState, file.string().c_str() );
	if( luaError ) {
		switch( luaError )
		{
		case LUA_ERRSYNTAX:
			PrintError( L"Syntax error in lua file \"%s\":\n%hs\n", file.wstring().c_str(), lua_tostring( pLuaState, -1 ) );
			return false;
		default:
			PrintError( L"Failed to load lua file \"%s\" (code: %d)\n", file.wstring().c_str(), luaError );
			return false;
		}
	}

	return true;
}
void CLuaScript::destroy()
{
}