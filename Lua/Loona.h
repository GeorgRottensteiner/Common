#ifndef LOONA_H_INCLUDED_
#define LOONA_H_INCLUDED_

extern "C"
{
#include <Lua/lua/lua.h>
#include <Lua/lua/lauxlib.h>
}
#include <Lua/LuaInstance.h>

#include <debug/debugclient.h>



template <typename T> class Loona
{

  struct userdataType
  {
    T*              pT;
    LuaInstance*    pLuaInstance;
  } ;


public:

  typedef int (T::*tMemberFunction)(LuaInstance& Lua);

  struct tRegType
  {
    const char*       name;
    tMemberFunction   mfunc;
  };

  static void Register( LuaInstance& Lua )
  {
    lua_newtable( Lua.m_LuaInstance );

    int methods = Lua.GetTop();

    luaL_newmetatable( Lua.m_LuaInstance, T::s_ClassName );
    int metatable = Lua.GetTop();

    // store method table in globals so that
    // scripts can add functions written in Lua.
    //Lua.PushString( T::s_ClassName );
    lua_pushvalue( Lua.m_LuaInstance,       methods );
    lua_setglobal( Lua.m_LuaInstance, T::s_ClassName );
    //lua_settable( Lua.m_LuaInstance,        LUA_GLOBALSINDEX );

    lua_pushliteral( Lua.m_LuaInstance,     "__metatable" );
    lua_pushvalue( Lua.m_LuaInstance,       methods );
    lua_settable( Lua.m_LuaInstance,        metatable );  // hide metatable from Lua getmetatable()

    lua_pushliteral( Lua.m_LuaInstance,     "__index" );
    lua_pushvalue( Lua.m_LuaInstance,       methods );
    lua_settable( Lua.m_LuaInstance,        metatable );

    lua_pushliteral( Lua.m_LuaInstance,     "__tostring" );
    lua_pushcfunction( Lua.m_LuaInstance,   tostring_T );
    lua_settable( Lua.m_LuaInstance,        metatable);

    lua_pushliteral( Lua.m_LuaInstance,     "__gc" );
    lua_pushcfunction( Lua.m_LuaInstance,   gc_T );
    lua_settable( Lua.m_LuaInstance,        metatable );

    lua_newtable( Lua.m_LuaInstance );                    // mt for method table
    int mt = Lua.GetTop();
    lua_pushliteral( Lua.m_LuaInstance,     "__call" );
    lua_pushcfunction( Lua.m_LuaInstance,   new_T );
    lua_pushliteral( Lua.m_LuaInstance,     "new" );
    lua_pushvalue( Lua.m_LuaInstance,       -2 );         // dup new_T function
    lua_settable( Lua.m_LuaInstance,        methods );    // add new_T to method table
    lua_settable( Lua.m_LuaInstance,        mt );         // mt.__call = new_T
    lua_setmetatable( Lua.m_LuaInstance,    methods );

    // fill method table with methods from class T
    for ( tRegType* l = T::s_Methods; l->name; l++ )
    {
      Lua.PushString( l->name );
      lua_pushlightuserdata( Lua.m_LuaInstance, (void*)l );
      lua_pushcclosure( Lua.m_LuaInstance, thunk, 1 );
      lua_settable( Lua.m_LuaInstance, methods );
    }

    Lua.Pop( 2 ); // drop metatable and method table
  }



  // get userdata from Lua stack and return pointer to T object
  static userdataType* check( lua_State *L, int narg )
  {
    userdataType *ud = static_cast<userdataType*>( luaL_checkudata( L, narg, T::s_ClassName ) );
    if ( !ud )
    {
      GR::String     text = lua_tostring( L, -1 );
      luaL_argerror( L, narg, T::s_ClassName );
      return NULL;
    }
    return ud;
  }



  static T* PopObject( lua_State *L, int narg )
  {
    userdataType *ud = static_cast<userdataType*>( luaL_checkudata( L, narg, T::s_ClassName ) );
    if ( !ud )
    {
      luaL_argerror( L, narg, T::s_ClassName );
    }
    T*  pObj = ud->pT;

    lua_remove( L, narg );

    return pObj;
  }



  static void PushObject( LuaInstance& Lua, T* pObject )
  {
    userdataType* ud = static_cast<userdataType*>( lua_newuserdata( Lua.m_LuaInstance, sizeof( userdataType ) ) );

    ud->pT = pObject;  // store pointer to object in userdata
    ud->pLuaInstance = &Lua;
    luaL_getmetatable( Lua.m_LuaInstance, T::s_ClassName );  // lookup metatable in Lua registry
    lua_setmetatable( Lua.m_LuaInstance, -2 );
  }



  static void SetVar( LuaInstance& Lua, T* pObject, const char* szVarName )
  {
    PushObject( Lua, pObject );
    lua_setglobal( Lua.m_LuaInstance, szVarName );
  }

private:

  Loona();  // hide default constructor


  // member function dispatcher
  static int thunk( lua_State* L )
  {
    /*
    LuaInstance    lua;
    lua.m_LuaInstance = L;
    lua.DumpStack();*/
    // stack has userdata, followed by method args
    userdataType*  pUD = check( L, 1 );     // get 'this' pointer
    T* obj = pUD->pT;
    lua_remove( L, 1 );         // remove this so member function args start at index 1
    // get member function from upvalue
    tRegType* l = static_cast<tRegType*>( lua_touserdata( L, lua_upvalueindex( 1 ) ) );

    //dh::Log( "Thunk has top at %d", lua_gettop( L ) );

    // since coroutines may have a different luaState copy over stack...
    if ( pUD->pLuaInstance->m_LuaInstance != L )
    {
      pUD->pLuaInstance->PopAll();
      lua_xmove( L, ( *pUD->pLuaInstance ).m_LuaInstance, lua_gettop( L ) );
    }

    //dh::Log( "Thunk has top at %d", lua_gettop( ( *pUD->pLuaInstance ).m_LuaInstance ) );

    int result = (obj->*(l->mfunc))( *pUD->pLuaInstance );

    // ...and back
    if ( pUD->pLuaInstance->m_LuaInstance != L )
    {
      lua_xmove( ( *pUD->pLuaInstance ).m_LuaInstance, L, lua_gettop( ( *pUD->pLuaInstance ).m_LuaInstance ) );
    }

    return result;
  }

  // create a new T object and
  // push onto the Lua stack a userdata containing a pointer to T object
  static int new_T( lua_State* L )
  {

    lua_remove( L, 1 );   // use s_ClassName:new(), instead of s_ClassName.new()

    //T* obj = new T( L );  // call constructor for T objects
    T* obj = new T();  // call constructor for T objects
    userdataType* ud = static_cast<userdataType*>( lua_newuserdata( L, sizeof( userdataType ) ) );

    ud->pT = obj;  // store pointer to object in userdata
    luaL_getmetatable( L, T::s_ClassName );  // lookup metatable in Lua registry
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

#ifdef sprintf_s
    sprintf_s( buff, sizeof( buff ), "%p", obj );
#else
#pragma warning(push)
#pragma warning(disable: 4996) // Deprecation
    sprintf( buff, "%p", obj );
#pragma warning(pop)
#endif
    lua_pushfstring( L, "%s (%s)", T::s_ClassName, buff );
    return 1;
  }

};



#endif // LOONA_H_INCLUDED_