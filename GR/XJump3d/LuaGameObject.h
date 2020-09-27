#pragma once

#include <Lua/LuaInstance.h>


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
    int                               Die( lua_State* L );

};
