#ifndef _LUNA_H_INCLUDED_
#define _LUNA_H_INCLUDED_

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "ldebug.h"
}

#include <debug/debugclient.h>

template <typename T> class Luna 
{

  struct userdataType
  { 
    T*      pT; 
  } ;


public:

  typedef int (T::*tMemberFunction)(lua_State *L);

  struct tRegType
  { 
    const char*       name; 
    tMemberFunction   mfunc; 
  };

  static void Register( lua_State* L ) 
  {

    lua_newtable( L );

    int methods = lua_gettop( L );

    luaL_newmetatable( L, T::className );
    int metatable = lua_gettop( L );

    // store method table in globals so that
    // scripts can add functions written in Lua.
    lua_pushvalue( L, methods );
    lua_setglobal( L, T::className );
    //lua_pushstring( L,      T::className );
    //lua_pushvalue( L,       methods );
    //lua_settable( L,        LUA_GLOBALSINDEX );

    lua_pushliteral( L,     "__metatable" );
    lua_pushvalue( L,       methods );
    lua_settable( L,        metatable );  // hide metatable from Lua getmetatable()

    lua_pushliteral( L,     "__index" );
    lua_pushvalue( L,       methods );
    lua_settable( L,        metatable );

    lua_pushliteral( L,     "__tostring" );
    lua_pushcfunction( L,   tostring_T );
    lua_settable( L,        metatable);

    lua_pushliteral( L,     "__gc" );
    lua_pushcfunction( L,   gc_T );
    lua_settable( L,        metatable );

    lua_newtable( L );                    // mt for method table
    int mt = lua_gettop( L );
    lua_pushliteral( L,     "__call" );
    lua_pushcfunction( L,   new_T );
    lua_pushliteral( L,     "new" );
    lua_pushvalue( L,       -2 );         // dup new_T function
    lua_settable( L,        methods );    // add new_T to method table
    lua_settable( L,        mt );         // mt.__call = new_T
    lua_setmetatable( L,    methods );

    // fill method table with methods from class T
    for ( tRegType* l = T::methods; l->name; l++ ) 
    {
      lua_pushstring( L, l->name );
      lua_pushlightuserdata( L, (void*)l );
      lua_pushcclosure( L, thunk, 1 );
      lua_settable( L, methods );
    }

    lua_pop( L, 2 );  // drop metatable and method table

  }

  // get userdata from Lua stack and return pointer to T object
  static T* check( lua_State *L, int narg ) 
  {
    userdataType *ud = static_cast<userdataType*>( luaL_checkudata( L, narg, T::className ) );
    if ( !ud )
    {
      //luaG_typeerror( L, narg, T::className );
      dh::Log( "User Data is wrong type!" );
    }
    return ud->pT;  // pointer to T object
  }

  static T* PopObject( lua_State *L, int narg ) 
  {
    userdataType *ud = static_cast<userdataType*>( luaL_checkudata( L, narg, T::className ) );
    if ( !ud )
    {
      //luaG_typeerror( L, narg, T::className );
      dh::Log( "User Data is wrong type!" );
    }
    T*  pObj = ud->pT;

    lua_remove( L, narg );

    return pObj;
  }

  static void PushObject( lua_State* L, T* pObject ) 
  {
    userdataType* ud = static_cast<userdataType*>( lua_newuserdata( L, sizeof( userdataType ) ) );

    ud->pT = pObject;  // store pointer to object in userdata
    luaL_getmetatable( L, T::className );  // lookup metatable in Lua registry
    lua_setmetatable( L, -2 );
  }

  static void SetVar( lua_State* L, T* pObject, const char* szVarName ) 
  {
    PushObject( L, pObject );
    lua_setglobal( L, szVarName );
  }

private:

  Luna();  // hide default constructor


  // member function dispatcher
  static int thunk( lua_State* L ) 
  {
    // stack has userdata, followed by method args
    T* obj = check( L, 1 );     // get 'this' pointer
    lua_remove( L, 1 );         // remove this so member function args start at index 1
    // get member function from upvalue
    tRegType* l = static_cast<tRegType*>( lua_touserdata( L, lua_upvalueindex( 1 ) ) );

    // call member function
    return (obj->*(l->mfunc))( L );  
  }

  // create a new T object and
  // push onto the Lua stack a userdata containing a pointer to T object
  static int new_T( lua_State* L ) 
  {

    lua_remove( L, 1 );   // use classname:new(), instead of classname.new()

    //T* obj = new T( L );  // call constructor for T objects
    T* obj = new T();  // call constructor for T objects
    userdataType* ud = static_cast<userdataType*>( lua_newuserdata( L, sizeof( userdataType ) ) );

    ud->pT = obj;  // store pointer to object in userdata
    luaL_getmetatable( L, T::className );  // lookup metatable in Lua registry
    lua_setmetatable( L, -2 );

    // userdata containing pointer to T object
    return 1;  

  }

  // garbage collection metamethod
  static int gc_T( lua_State* L ) 
  {
    userdataType* ud = static_cast<userdataType*>( lua_touserdata( L, 1 ) );

    T* obj = ud->pT;
    delete obj;  // call destructor for T objects
    return 0;
  }

  static int tostring_T( lua_State* L ) 
  {

    char buff[32];

    userdataType* ud = static_cast<userdataType*>( lua_touserdata( L, 1 ) );
    T* obj = ud->pT;
    sprintf( buff, "%p", obj );
    lua_pushfstring( L, "%s (%s)", T::className, buff );
    return 1;
  }

};



#endif // _LUNA_H_INCLUDED_