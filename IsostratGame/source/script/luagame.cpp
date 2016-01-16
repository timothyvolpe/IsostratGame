#include "base.h"
#include "script\luagame.h"

int luaf_game_getresolution( lua_State *pLuaState )
{
	lua_pushinteger( pLuaState, 1 );
	return 1;
}