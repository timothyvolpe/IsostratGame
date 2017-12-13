#pragma once

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

int luaf_game_getresolution( lua_State *pLuaState );
int luaf_game_getfov( lua_State *pLuaState );
int luaf_game_setfov( lua_State *pLuaState );
int luaf_game_isfullscreen( lua_State *pLuaState );
int luaf_game_setfullscreen( lua_State *pLuaState );
int luaf_game_frametime( lua_State *pLuaState );
int luaf_game_getwireframemode( lua_State *pLuaState );
int luaf_game_getcameraposition( lua_State *pLuaState );

static const struct luaL_Reg luaf_GameMetatableClient[] = {
	{ "GetResolution", luaf_game_getresolution }, 
	{ "GetFieldOfView", luaf_game_getfov },
	{ "SetFieldOfView", luaf_game_setfov },
	{ "IsFullscreen", luaf_game_isfullscreen },
	{ "SetFullscreen", luaf_game_setfullscreen },
	{ "GetFrameTime", luaf_game_frametime },
	{ "GetWireframeMode", luaf_game_getwireframemode },
	{ "GetCameraPosition", luaf_game_getcameraposition },
	{ NULL, NULL }
};
static const struct luaL_Reg luaf_GameMetatableServer[] ={
	{ NULL, NULL }
};