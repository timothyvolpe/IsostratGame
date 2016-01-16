#pragma once

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <boost\filesystem.hpp>
#include <vector>

class CLuaScript;

/////////////////
// CLuaManager //
/////////////////

class CLuaManager
{
private:
	typedef std::vector<CLuaScript*> ScriptList;

	lua_State *m_pLuaClientState;
	lua_State *m_pLuaServerState;
	ScriptList m_scriptList;

	bool createLuaMethods( lua_State *pState, bool isServer );
public:
	CLuaManager();
	~CLuaManager();

	bool initialize();
	void destroy();

	CLuaScript* loadScript( boost::filesystem::path file, bool isServer );

	lua_State* getLuaState();
};

///////////////
// CLuaSript //
///////////////

class CLuaScript
{
public:
	CLuaScript();
	~CLuaScript();

	bool initialize( boost::filesystem::path file, lua_State *pLuaState );
	void destroy();
};


