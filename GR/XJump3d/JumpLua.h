#pragma once

#include <Lua/LuaInstance.h>



class CJumpLua : public CLuaInstance
{

  public:


    CJumpLua();
    virtual ~CJumpLua();

    static int                  ChangeMap( lua_State* L );
    static int                  Warp( lua_State* L );
    static int                  StartMovie( lua_State* L );
    static int                  SetFeld( lua_State* L );
    static int                  SetBackground( lua_State* L );
    static int                  Freeze( lua_State* L );
    static int                  MovieMode( lua_State* L );

    static int                  CreateObject( lua_State* L );
    static int                  GetObjectById( lua_State* L );

    static int                  SetVar( lua_State* L );
    static int                  GetVar( lua_State* L );
    static int                  GetVarI( lua_State* L );

    static int                  WalkTarget( lua_State* L );

    void                        InitInstance();

};
