#pragma once

#include <Lua/LuaInstance.h>

#include <Lua/Lua/Luna.h>



class CGameObject;

class CLuaGameObject
{

  public:

    CGameObject*                      m_pObject;

    static const char                 className[];

    static Luna<CLuaGameObject>::tRegType  methods[];


    CLuaGameObject( CGameObject* pObj = NULL );

    int                               Event( lua_State* L );
    int                               SetPos( lua_State* L );
    int                               WalkTo( lua_State* L );
    int                               Die( lua_State* L );
    int                               TurnLeft( lua_State* L );
    int                               TurnRight( lua_State* L );
    int                               SetAnimation( lua_State* L );
    int                               EnablePath( lua_State* L );
    int                               DisablePath( lua_State* L );

};
