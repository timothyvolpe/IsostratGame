#include "base.h"
#include "script\luascript.h"
#include "script\luautil.h"
#include "script\luagame.h"
#include "script\luainterface.h"
#include <algorithm>

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
	lua_pushboolean( pState, isServer );
	lua_setglobal( pState, "SERVER" );
	// CLIENT
	lua_pushboolean( pState, !isServer );
	lua_setglobal( pState, "CLIENT" );

	// Game globsal
	luaL_newmetatable( pState, "Game" );
	luaL_getmetatable( pState, "Game" );
	if( isServer)
		luaL_setfuncs( pState, luaf_GameMetatableServer, 0 );
	else
		luaL_setfuncs( pState, luaf_GameMetatableClient, 0 );
	lua_setglobal( pState, "Game" );
	lua_pop( pState, 1 );

	// Interface class
	if( !isServer )
	{
		int libid, metaid;

		lua_createtable( pState, 0, 0 );
		libid = lua_gettop( pState );

		luaL_newmetatable( pState, "Interface" );
		metaid = lua_gettop( pState );
		luaL_setfuncs( pState, luaf_InterfaceMetatable, 0 );

		luaL_newlib( pState, luaf_InterfaceMethods );
		lua_setfield( pState, metaid, "__index" );

		luaL_newlib( pState, luaf_InterfaceMetatable );
		lua_setfield( pState, metaid, "__metatable" );

		lua_setmetatable( pState, libid );
		lua_setglobal( pState, "Interface" );

		// Interface table
		lua_newtable( pState );
		lua_setglobal( pState, LUA_INTERFACE_EVENTTABLE );
	}

	return true;
}

bool CLuaManager::initialize()
{
	boost::filesystem::path scriptPath;

	// Create the lua states
	m_pLuaClientState = luaL_newstate();
	m_pLuaServerState = luaL_newstate();
	// Setup the lua methods
	if( !this->createLuaMethods( m_pLuaClientState, false ) )
		return false;

	return true;
}
void CLuaManager::destroy()
{
	for( auto it = m_scriptMap.begin(); it != m_scriptMap.end(); it++ ) {
		DESTROY_DELETE( (*it).second );
	}
	m_scriptMap.clear();
	// Shut down lua
	lua_close( m_pLuaClientState );
	m_pLuaClientState = NULL;
	lua_close( m_pLuaServerState );
	m_pLuaServerState = NULL;
}

bool CLuaManager::executeScripts( bool isServer )
{
	int luaError;

	// Run the scripts
	if( !isServer )
	{
		luaError = lua_pcall( m_pLuaClientState, 0, 0, 0 );
		if( luaError ) {
			PrintError( L"%hs\n", lua_tostring( m_pLuaClientState, -1 ) );
			return false;
		}
	}
	return true;
}

CLuaScript* CLuaManager::loadScript( boost::filesystem::path file, bool isServer, bool execute )
{
	CLuaScript *pLuaScript;
	std::string scriptKey;

	// Check if its already loaded
	scriptKey = file.string();
	std::transform( scriptKey.begin(), scriptKey.end(), scriptKey.begin(), ::tolower );
	if( m_scriptMap.find( scriptKey ) != m_scriptMap.end() ) {
		PrintWarn( L"Attempted to reload script \"%hs\"", scriptKey.c_str() );
		return false;
	}

	// Create a new script
	pLuaScript = new CLuaScript();
	if( isServer ) {
		if( !pLuaScript->initialize( file, m_pLuaServerState, execute ) )
			return false;
	}
	else {
		if( !pLuaScript->initialize( file, m_pLuaClientState, execute ) )
			return false;
	}
	// Add to the list
	m_scriptMap.insert( std::pair<std::string, CLuaScript*>( scriptKey, pLuaScript ) );

	return pLuaScript;
}

bool CLuaManager::callInterfaceEvent( CInterfaceBase *pInterface, std::string eventName )
{
	int eventFuncRef;

	// Find the object in the table
	lua_getglobal( m_pLuaClientState, LUA_INTERFACE_EVENTTABLE );
	lua_pushlightuserdata( m_pLuaClientState, pInterface );
	lua_gettable( m_pLuaClientState, -2 );
	if( lua_isnil( m_pLuaClientState, -1 ) ) {
		lua_pop( m_pLuaClientState, 2 );
		return false;
	}
	lua_getfield( m_pLuaClientState, -1, eventName.c_str() );
	if( lua_isnil( m_pLuaClientState, -1 ) ) {
		lua_pop( m_pLuaClientState, 3 );
		return false;
	}
	eventFuncRef = lua_tointeger( m_pLuaClientState, -1 );
	lua_pop( m_pLuaClientState, 3 );
	// Call the callback
	lua_rawgeti( m_pLuaClientState, LUA_REGISTRYINDEX, eventFuncRef );
	lua_pcall( m_pLuaClientState, 0, 0, 0 );

	return true;
}

bool CLuaManager::isScriptLoaded( boost::filesystem::path file )
{
	std::string scriptKey;

	// Check if its already loaded
	scriptKey = file.string();
	std::transform( scriptKey.begin(), scriptKey.end(), scriptKey.begin(), ::tolower );
	if( m_scriptMap.find( scriptKey ) != m_scriptMap.end() )
		return true;
	return false;
}

///////////////
// CLuaSript //
///////////////

CLuaScript::CLuaScript() {
}
CLuaScript::~CLuaScript() {
}

bool CLuaScript::initialize( boost::filesystem::path file, lua_State *pLuaState, bool execute )
{
	int luaError;

	// Make sure the file loads and works
	if( execute )
		luaError = luaL_dofile( pLuaState, file.string().c_str() );
	else
		luaError = luaL_loadfile( pLuaState, file.string().c_str() );
	if( luaError ) {
		switch( luaError )
		{
		case LUA_YIELD:
			PrintError( L"Lua yield \"%s\":\n%hs\n", file.wstring().c_str(), lua_tostring( pLuaState, -1 ) );
			return false;
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