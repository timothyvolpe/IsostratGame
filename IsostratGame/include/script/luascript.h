#pragma once

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <boost\filesystem.hpp>
#include <map>

class CInterfaceBase;

class CLuaScript;
/////////////////
// CLuaManager //
/////////////////

class CLuaManager
{
private:
	typedef std::map<std::string, CLuaScript*> ScriptMap;

	lua_State *m_pLuaClientState;
	lua_State *m_pLuaServerState;
	ScriptMap m_scriptMap;

	bool createLuaMethods( lua_State *pState, bool isServer );
public:
	CLuaManager();
	~CLuaManager();

	bool initialize();
	void destroy();

	bool executeScripts( bool isServer );

	CLuaScript* loadScript( boost::filesystem::path file, bool isServer, bool execute = false );

	bool callInterfaceEvent( CInterfaceBase *pInterface, std::string eventName );

	bool isScriptLoaded( boost::filesystem::path file );
};

///////////////
// CLuaSript //
///////////////

class CLuaScript
{
public:
	CLuaScript();
	~CLuaScript();

	bool initialize( boost::filesystem::path file, lua_State *pLuaState, bool execute );
	void destroy();
};


