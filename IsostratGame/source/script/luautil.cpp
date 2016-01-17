#include "base.h"
#include "script\luascript.h"
#include "script\luautil.h"
#include <boost\filesystem.hpp>
#include "game.h"

int luaf_print( lua_State *pState )
{
	const char* pStr = lua_tostring( pState, 1 );
	PrintInfo( L"%hs", pStr );

	return 0;
}
int luaf_error( lua_State *pState )
{
	const char* pStr = lua_tostring( pState, 1 ); 
	PrintError( L"%hs", pStr );

	return 0;
}
int luaf_include( lua_State *pState )
{
	const char *pFile;
	boost::filesystem::path fullpath;
	bool isServer;

	luaL_checktype( pState, 1, LUA_TSTRING );
	pFile = lua_tostring( pState, 1 );

	// Include the file pointed to by the arg
	fullpath = boost::filesystem::current_path();
	fullpath /= FILESYSTEM_LUADIR;
	fullpath /= pFile;

	// if it exists load it
	if( boost::filesystem::exists( fullpath ) ) {
		if( boost::filesystem::is_regular_file( fullpath ) )
		{
			// Check if server
			lua_getglobal( pState, "SERVER" );
			isServer = (lua_toboolean( pState, -1 ) > 0 ? true : false);
			lua_pop( pState, 1 );
			if( CGame::instance().getLuaManager()->isScriptLoaded( fullpath ) )
				return 0;
			else {
				if( !CGame::instance().getLuaManager()->loadScript( fullpath, isServer, true ) ) {
					PrintError( L"Failed to include file because the file could not be loaded \"%hs\"\n", pFile );
				}
				return 0;
			}
			return 0;
		}
	}
	PrintError( L"Failed to include file because the file was not found \"%hs\"\n", pFile );

	return 0;
}