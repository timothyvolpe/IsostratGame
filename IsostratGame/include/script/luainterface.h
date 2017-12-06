#pragma once

#define LUA_INTERFACE_EVENTTABLE "_InterfaceEventTable"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

int luaf_interface_gc( lua_State *pState );
int luaf_interface_index( lua_State *pState );
int luaf_interface_newindex( lua_State *pState );

int luaf_interface_create( lua_State *pState );
int luaf_interface_activate( lua_State *pState );
int luaf_interface_regevent( lua_State *pState );

int luaf_interface_setpos( lua_State *pState );
int luaf_interface_setsize( lua_State *pState );
int luaf_interface_setbgcol( lua_State *pState );
int luaf_interface_setvisible( lua_State *pState );
int luaf_interface_settext( lua_State *pState );
int luaf_interface_settextsize( lua_State *pState );

static const struct luaL_Reg luaf_InterfaceMetatable[] = {
	{ "__gc", luaf_interface_gc },
	{ "__index", luaf_interface_index },
	{ "__newindex", luaf_interface_newindex },
	{ NULL, NULL }
};
static const struct luaL_Reg luaf_InterfaceMethods[] ={
	{ "create", luaf_interface_create },
	{ "RegisterEvent", luaf_interface_regevent },
	{ "Activate", luaf_interface_activate },
	{ "SetPosition", luaf_interface_setpos },
	{ "SetSize", luaf_interface_setsize },
	{ "SetBackground", luaf_interface_setbgcol },
	{ "SetVisible", luaf_interface_setvisible },
	{ "SetText", luaf_interface_settext },
	{ "SetTextSize", luaf_interface_settextsize },
	{ NULL, NULL }
};