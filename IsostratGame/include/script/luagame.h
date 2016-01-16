#pragma once

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

int luaf_game_getresolution( lua_State *pLuaState );

static const struct luaL_Reg luaf_GameMetatable[] = {
	{ "GetResolution", luaf_game_getresolution },
	{ NULL, NULL }
};