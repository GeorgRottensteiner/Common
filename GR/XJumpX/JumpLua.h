#pragma once

#include <Lua/LuaInstance.h>



class CJumpLua : public LuaInstance
{

  public:

    CJumpLua();
    virtual ~CJumpLua();

    static int                  ChangeMap( lua_State* L );
    static int                  Warp( lua_State* L );
    static int                  StartMovie( lua_State* L );
    static int                  SetFeld( lua_State* L );
    static int                  SetArea( lua_State* L );

    static int                  SetBackground( lua_State* L );
    static int                  Freeze( lua_State* L );
    static int                  MovieMode( lua_State* L );

    static int                  CreateObject( lua_State* L );
    static int                  GetObjectById( lua_State* L );
    static int                  SetControlledObjectById( lua_State* L );

    static int                  SetVar( lua_State* L );
    static int                  GetVar( lua_State* L );
    static int                  GetVarI( lua_State* L );

    static int                  EnableTrigger( lua_State* L );
    static int                  DisableTrigger( lua_State* L );

    static int                  WalkTarget( lua_State* L );
    static int                  WalkToTarget( lua_State* L );

    void                        InitInstance();

};
