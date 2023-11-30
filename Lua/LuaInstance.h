#pragma once

#include <list>
#include <string>
#include <map>

#include <GR/GRTypes.h>

#include <Lang/FastDelegate.h>



struct lua_State;

typedef int (*lua_CFunction) (lua_State *L);



class LuaInstance  
{

  public:

    typedef fastdelegate::FastDelegate1<LuaInstance&,int>   tHandlerFunction;


  protected:

    typedef std::map<GR::String,tHandlerFunction>        tMapHandler;


  public:

    lua_State*              m_LuaInstance;

    std::list<GR::String>   m_Threads;

    // variable that should be assigned to "this" before resuming a thread
    std::map<GR::String, GR::String>   m_ThreadThis;

    float                   m_CurrentTime;

    tMapHandler             m_Handler;


    LuaInstance();
	  virtual ~LuaInstance();

    void                    LuaInit();
    void                    LuaShutDown();

    void                    Register( lua_CFunction pFunction, const char* FunctionName );
    void                    RegisterHandler( tHandlerFunction Function, const GR::String& FunctionName );
    void                    UnregisterHandler( const GR::String& FunctionName );

    static int              HandlerThunk( lua_State* L );

    bool                    DoString( const GR::String& Script );
    bool                    DoFile( const GR::String& Filename );
    bool                    FunctionExists( const GR::String& Function );

    bool                    StartThread( const GR::String& ThreadName, const GR::String& Function );
    void                    ResumeThreads( const float ElapsedTime );
    GR::String              StartScriptAsThread( const GR::String& Script, const GR::String& Threadname = GR::String(), const GR::String& ThisVariable = GR::String() );
    bool                    IsThreadRunning( const GR::String& ThreadName );

    int                     GetTop();
    void                    PushNIL();
    void                    PushNumber( GR::ip Number );
    void                    PushNumberF( GR::f32 Number );
    void                    PushString( const GR::String& Text );
    void                    PushBoolean( bool Bool );
    void                    Pop( int Count = 1 );
    void                    PopAll();

    GR::up                  RunningThreadCount() const;
    void                    KillAllThreads();

    GR::String              ToString( int Index );
    GR::ip                  ToNumber( int Index );
    GR::f32                 ToF32( int Index );
    bool                    ToBoolean( int Index );

    bool                    IsNumber( int Index );
    bool                    IsNil( int Index );

    GR::String              GetGlobal( const GR::String& VarName );
    GR::String              GetVar( const GR::String& VarName );
    bool                    GetTable( const GR::String& TableName );

    bool                    SetVar( const GR::String& VarName, const GR::String& Value );
    bool                    SetVar( const GR::String& VarName, const GR::ip Value );
    bool                    SetVar( const GR::String& VarName, const GR::f32 Value );

    void                    SetGlobal( const GR::String& VarName );
    void                    SetGlobal( const GR::String& VarName, const GR::String& VarValue );
    void                    SetGlobal( const GR::String& VarName, const GR::ip Value );
    void                    SetGlobal( const GR::String& VarName, const GR::f32 Value );
    void                    SetGlobal( const GR::String& VarName, const bool Value );

    static int              Alert( lua_State* L );

    // expect either a string (name of function)
    //            or a function
    static int              CreateThread( lua_State* L );
    static int              KillThread( lua_State* L );
    static int              GetTime( lua_State* L );

    static int              AddThread( lua_State* L );

    void                    DumpStack();

};

