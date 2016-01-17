#include "base.h"
#include "script\luagame.h"
#include "game.h"
#include "config.h"

int luaf_game_getresolution( lua_State *pLuaState )
{
	int resx, resy;
	resx = CGame::instance().getConfigLoader()->getResolutionX();
	resy = CGame::instance().getConfigLoader()->getResolutionY();
	lua_newtable( pLuaState );
	lua_pushinteger( pLuaState, resx );
	lua_setfield( pLuaState, -2, "x" );
	lua_pushinteger( pLuaState, resy );
	lua_setfield( pLuaState, -2, "y" );
	return 1;
}
int luaf_game_getfov( lua_State *pLuaState )
{
	float fov;
	fov = CGame::instance().getConfigLoader()->getFieldOfView();
	lua_pushnumber( pLuaState, fov );
	return 1;
}
int luaf_game_setfov( lua_State *pLuaState )
{
	float fov;

	luaL_checktype( pLuaState, 2, LUA_TNUMBER );

	fov = (float)lua_tonumber( pLuaState, 2 );
	if( fov < 0.0f ) {
		fov = 0.0f;
		PrintWarn( L"field of view out of range, clamping [0.0f, 180.0f]\n" );
	}
	else if( fov > 180.0f ) {
		fov = 180.0f;
		PrintWarn( L"field of view out of range, clamping [0.0f, 180.0f]\n" );
	}
	CGame::instance().getConfigLoader()->setFieldOfView( fov );
	return 0;
}
int luaf_game_isfullscreen( lua_State *pLuaState )
{
	bool isFullscreen;
	isFullscreen = CGame::instance().getConfigLoader()->isFullscreen();
	lua_pushboolean( pLuaState, isFullscreen );
	return 1;
}
int luaf_game_setfullscreen( lua_State *pLuaState )
{
	int fullscreen;

	luaL_checktype( pLuaState, 2, LUA_TBOOLEAN );
	fullscreen = lua_toboolean( pLuaState, 2 );
	if( !fullscreen )
		CGame::instance().getConfigLoader()->setFullscreen( false );
	else
		CGame::instance().getConfigLoader()->setFullscreen( true );

	return 0;
}
int luaf_game_frametime( lua_State *pLuaState )
{
	double frametime;
	frametime = CGame::instance().getFrameTime();
	lua_pushnumber( pLuaState, frametime );
	return 1;
}