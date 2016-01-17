#include "base.h"
#include "script\luainterface.h"
#include "script\luascript.h"
#include "ui\interface.h"
#include <algorithm>
#include <boost\locale.hpp>

#include "ui\interface_screen.h"
#include "ui\interface_label.h"

int luaf_interface_gc( lua_State *pState )
{
	return 0;
}
int luaf_interface_index( lua_State *pState )
{
	//PrintWarn( L"Index?\n" );
	return 0;
}
int luaf_interface_newindex( lua_State *pState )
{
	//PrintWarn( L"New index?\n" );
	return 0;
}

int luaf_interface_create( lua_State *pState )
{
	CInterfaceManager *pManager = CGame::instance().getInterfaceManager();
	int argcount;
	const char *pType;
	std::string typestd;
	CInterfaceContainer *pParent;
	CInterfaceBase *pInterface;

	argcount = lua_gettop( pState );
	// Get the arguments
	luaL_checktype( pState, 1, LUA_TSTRING );
	pType = lua_tostring( pState, 1 );
	if( argcount > 1 )
	{
		luaL_checktype( pState, 2, LUA_TLIGHTUSERDATA );
		pParent = reinterpret_cast<CInterfaceContainer*>( const_cast<void*>( lua_topointer( pState, 2 ) ) );
		if( !pParent ) {
			PrintError( L"Invalid parent object\n" );
			return 0;
		}
	}
	else
		pParent = NULL;

	typestd = pType;
	std::transform( typestd.begin(), typestd.end(), typestd.begin(), ::tolower );
	if( typestd.compare( "screen" ) == 0 ) {
		pInterface = pManager->createInterfaceObject<CInterfaceScreen>();
		pManager->addScreen( reinterpret_cast<CInterfaceScreen*>(pInterface) );
	}
	else if( typestd.compare( "label" ) == 0 ) {
		pInterface = pManager->createInterfaceObjectRenderable<CInterfaceLabel>();
	}
	else {
		PrintError( L"Invalid interface type \"%hs\"\n", pType );
		return 0;
	}

	if( !pInterface )
		return false;
	if( pParent )
		pParent->addToContainer( pInterface );

	// Add to interface object table
	lua_getglobal( pState, LUA_INTERFACE_EVENTTABLE );
	lua_pushlightuserdata( pState, pInterface );
	lua_newtable( pState );
	lua_settable( pState, -3 );

	// Return a new interface object pointer
	lua_pushlightuserdata( pState, pInterface );
	luaL_getmetatable( pState, "Interface" );
	lua_setmetatable( pState, -2 );

	return 1;
}
int luaf_interface_activate( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}

	if( !pInterfaceObject->onActivate() )
		return 0;

	return 0;
}
int luaf_interface_regevent( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;
	const char *pEventName;
	int eventFuncRef;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}
	luaL_checktype( pState, 2, LUA_TSTRING );
	pEventName = lua_tostring( pState, 2 );
	luaL_checktype( pState, 3, LUA_TFUNCTION );
	lua_pushvalue( pState, 3 );
	eventFuncRef = luaL_ref( pState, LUA_REGISTRYINDEX );

	// format for table is
	// [interfacepointer][eventname] = func
	// find in interface object table and add event
	lua_getglobal( pState, LUA_INTERFACE_EVENTTABLE );
	lua_pushlightuserdata( pState, pInterfaceObject );
	lua_gettable( pState, -2 );
	lua_pushstring( pState, pEventName );
	lua_pushinteger( pState, eventFuncRef );
	lua_settable( pState, -3 );

	return 0;
}

int luaf_interface_setpos( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;
	float relx, rely;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}
	luaL_checktype( pState, 2, LUA_TNUMBER );
	relx = (float)lua_tonumber( pState, 2 );
	luaL_checktype( pState, 3, LUA_TNUMBER );
	rely = (float)lua_tonumber( pState, 3 );
	
	pInterfaceObject->setRelativePosition( glm::vec2( relx, rely ));

	return 0;
}
int luaf_interface_setsize( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;
	float relx, rely;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}
	luaL_checktype( pState, 2, LUA_TNUMBER );
	relx = (float)lua_tonumber( pState, 2 );
	luaL_checktype( pState, 3, LUA_TNUMBER );
	rely = (float)lua_tonumber( pState, 3 );

	pInterfaceObject->setRelativeSize( glm::vec2( relx, rely ) );
	return 0;
}

int luaf_interface_setbgcol( lua_State *pState )
{
	return 0;
}
int luaf_interface_setvisible( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;
	int isvisible;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}
	
	luaL_checktype( pState, 2, LUA_TBOOLEAN );
	isvisible = lua_toboolean( pState, 2 );
	if( isvisible == 0 )
		pInterfaceObject->setVisible( false );
	else
		pInterfaceObject->setVisible( true );

	return 0;
}
int luaf_interface_settext( lua_State *pState )
{
	CInterfaceBase *pInterfaceObject;
	const char *pText;

	luaL_checktype( pState, 1, LUA_TLIGHTUSERDATA );
	pInterfaceObject = reinterpret_cast<CInterfaceBase*>(const_cast<void*>(lua_topointer( pState, 1 )));
	if( !pInterfaceObject ) {
		PrintError( L"Invalid interface object\n" );
		return 0;
	}

	luaL_checktype( pState, 2, LUA_TSTRING );
	pText = lua_tostring( pState, 2 );
	pInterfaceObject->setText( boost::locale::conv::utf_to_utf<wchar_t>(pText) );

	return 0;
}

