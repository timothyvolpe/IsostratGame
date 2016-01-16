#include "base.h"
#include "script\luautil.h"

int luaf_print( lua_State *pState )
{
	const char* pStr = lua_tostring( pState, 1 ); // get arg
	PrintInfo( L"%hs", pStr );

	return 0;
}
int luaf_error( lua_State *pState )
{
	const char* pStr = lua_tostring( pState, 1 ); // get arg
	PrintError( L"%hs", pStr );

	return 0;
}