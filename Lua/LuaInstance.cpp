#include <GR/GRTypes.h>

#include <Misc/Misc.h>

#include <debug/debugclient.h>

#include "LuaInstance.h"

extern "C"
{
#include "lua\lua.h"
#include "lua\lualib.h"
#include "lua/lauxlib.h"
}

#include "lua/luna.h"



LuaInstance::LuaInstance() :
  m_LuaInstance( NULL ),
  m_CurrentTime( 0.0f )
{
  LuaInit();
}



LuaInstance::~LuaInstance()
{
  LuaShutDown();
}



void LuaInstance::LuaInit()
{
  if ( m_LuaInstance != NULL )
  {
    return;
  }

  m_LuaInstance = luaL_newstate();

  luaL_requiref( m_LuaInstance, "_G", luaopen_base, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "coroutine", luaopen_coroutine, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "string", luaopen_string, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "table", luaopen_table, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "math", luaopen_math, 1 );
  Pop();    // remove lib

  Pop( GetTop() );

  DoString( CMisc::printf( "LuaHandler = %d", this ) );

  DoString( "function LuaCreateThread( Func, UserData )\n"
            "  if ( Func == nil ) then\n"
            "    return 1;\n"
            "  end\n"
            "  if ( globalCounter == nil ) then\n"
            "    globalCounter = 0;\n"
            "  else\n"
            "    globalCounter = globalCounter + 1;\n"
            "  end\n"
            "  local newName = \"threadFuncglob\" .. globalCounter;\n"
            "  newThread = coroutine.create( Func );\n"
            "  _G[newName] = newThread;\n"
            "  local rresult, second_value = coroutine.resume( newThread, 0, UserData );\n"
            "  if ( not rresult ) then\n"
            "    --Ein Fehler!\n"
            "    _ALERT( second_value );\n"
            "    return 0;\n"
            "  end\n"
            "  if ( second_value == nil ) then\n"
            "     --Thread done\n"
            "    newThread = nil;\n"
            "    return 1;\n"
            "  end\n"
            "  LuaAddThread( newName, UserData );\n"
            "  return 0;\n"
            "end\n" );

  DoString( "function LuaContinueThread( co_event, ElapsedTime )\n"
            "  local result, second_value = coroutine.resume( co_event, ElapsedTime );\n"
            "  if ( not result ) then\n"
            "    --Ein Fehler!\n"
            "    _ALERT( second_value );\n"
            "    return 1;\n"
            "  end\n"
            "  if ( second_value == nil ) then\n"
            "    -- Thread done\n"
            "    co_event = nil;\n"
            "    return 1;\n"
            "  end\n"
            "  return 0;\n"
            "end" );

  DoString( "function Wait( Ticks )\n"
            "  local WaitStartTime = GetTime();\n"
            "  while ( true ) do\n"
            "    if ( GetTime() > WaitStartTime + Ticks ) then\n"
            "      break;\n"
            "    end\n"
            "    coroutine.yield( 1 )\n"
            "  end\n"
            "end" );

  Register( &Alert, "_ALERT" );
  Register( &AddThread,     "LuaAddThread" );
  Register( &CreateThread,  "CreateThread" );
  Register( &KillThread,    "KillThread" );
  Register( &GetTime,       "GetTime" );

}



int LuaInstance::Alert( lua_State* L )
{
  if ( !lua_isstring( L, -1 ) )
  {
    lua_pop( L, 1 );
    return 0;
  }

  const char*   pMessage = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  dh::Log( pMessage );
  return 0;
}



bool LuaInstance::DoString( const GR::String& Script )
{
  int errorCode = luaL_dostring( m_LuaInstance, Script.c_str() );

  if ( errorCode != 0 )
  {
    Alert( m_LuaInstance );
  }
  return ( errorCode == 0 );
}



bool LuaInstance::DoFile( const GR::String& Filename )
{
  int errorCode = luaL_dofile( m_LuaInstance, Filename.c_str() );

  if ( errorCode != 0 )
  {
    Alert( m_LuaInstance );
  }
  return ( errorCode == 0 );
}



void LuaInstance::Register( lua_CFunction pFunction, const char* FunctionName )
{
  if ( ( pFunction == NULL )
  ||   ( FunctionName == NULL ) )
  {
    return;
  }

  lua_pushcfunction( m_LuaInstance, pFunction );
  lua_setglobal( m_LuaInstance, FunctionName );
}



bool LuaInstance::FunctionExists( const GR::String& Script )
{
  size_t      oldPos = 0;
  int         truePushedParamCount = 0;
  GR::String  lastTable;

  if ( Script.find( '.' ) == GR::String::npos )
  {
    // pure variable
    lua_getglobal( m_LuaInstance, Script.c_str() );

    bool isFunction = lua_isfunction( m_LuaInstance, -1 );
    PopAll();

    return isFunction;
  }

  while ( true )
  {
    size_t      dotPos = Script.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( truePushedParamCount == 0 )
      {
        PushString( Script );
        lua_rawget( m_LuaInstance, -1 );

        bool isFunction = lua_isfunction( m_LuaInstance, -1 );
        PopAll();

        return isFunction;
      }
      PushString( Script.substr( oldPos ) );
      ++truePushedParamCount;

      lua_rawget( m_LuaInstance, -2 );

      bool isFunction = lua_isfunction( m_LuaInstance, -1 );
      PopAll();

      return isFunction;
    }

    lastTable = Script.substr( oldPos, dotPos - oldPos );
    ++truePushedParamCount;
    if ( truePushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:FunctionExists - no table %s", Script.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



void LuaInstance::LuaShutDown()
{
  if ( m_LuaInstance )
  {
    lua_close( m_LuaInstance );
    m_LuaInstance = NULL;
  }
  m_Threads.clear();
}



bool LuaInstance::StartThread( const GR::String& ThreadName, const GR::String& Function )
{
  static int      threadCounter2 = 0;
  GR::String      threadName = ThreadName;
  int             newValue = ++threadCounter2;

  if ( threadName.empty() )
  {
    threadName = Misc::Format( "TempThreadFunc2_%1%" ) << threadCounter2;
  }


  GR::String     varName = "Thread";
  
  varName += threadName;

  DoString( GR::String( "function LuaStartThread( Func )\n"
            "  if ( Func == nil ) then\n"
            "    return 1;\n"
            "  end\n"
            "  " + varName + " = coroutine.create( Func );\n"
            "  local rresult, second_value = coroutine.resume( " + varName + ", 0 );\n"
            "  if ( not rresult ) then\n"
            "    --Ein Fehler!\n"
            "    _ALERT( second_value );\n"
            "    return 0;\n"
            "  end\n"
            "  if ( second_value == nil ) then\n"
            "    -- Thread done\n"
            "    " + varName + " = nil;\n"
            "    return 1;\n"
            "  end\n"
            "  LuaAddThread( \"" + varName + "\" );\n"
            "  return 0;\n"
            "end\n" ).c_str() );

  return DoString( "LuaStartThread( " + Function + " );" );
}



int LuaInstance::CreateThread( lua_State* L )
{
  if ( !lua_isstring( L, -1 ) )
  {
    if ( lua_isfunction( L, -1 ) )
    {
      lua_getglobal( L, "LuaCreateThread" );
      int error = lua_pcall( L, 1, 1, 0 );
      if ( error )
      {
        GR::String   errorMessage = lua_tostring( L, -1 );
        lua_pop( L, 1 );

        dh::Log( "CreateThread error %s\n", errorMessage.c_str() );
        return 0;
      }
    }
    dh::Log( "CreateThread, expected string with function name (got %s instead)", lua_typename( L, lua_type( L, -1 ) ) );
    lua_pop( L, 1 );
    return 0;
  }

  GR::String     threadName = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  if ( !pLuaHandler->StartThread( threadName, threadName ) )
  {
    dh::Log( "StartThread %s failed", threadName.c_str() );
  }

  return 0;
}



GR::String LuaInstance::StartScriptAsThread( const GR::String& Script, const GR::String& UserThreadName, const GR::String& ThisVariable )
{
  static int      threadCounter = 0;

  GR::String      threadName = UserThreadName;
  int             newValue = ++threadCounter;

  GR::String     randomFunctionName = Misc::Format( "TempThreadFunc_%1%%2%" ) << threadCounter << ( GR::up )&Script;
  if ( threadName.empty() )
  {
    threadName = randomFunctionName;
  }

  GR::String     functionBody = "function " + randomFunctionName + "()\n" + Script + "\nend;";

  DoString( functionBody.c_str() );

  if ( !StartThread( threadName, randomFunctionName ) )
  {
    return GR::String();
  }

  if ( !ThisVariable.empty() )
  {
    if ( m_ThreadThis.find( threadName ) != m_ThreadThis.end() )
    {
      dh::Log( "StartScriptAsThread duplicate thread name %s", threadName.c_str() );
    }
    m_ThreadThis[threadName] = ThisVariable;
  }

  return threadName;
}



int LuaInstance::AddThread( lua_State* L )
{
  int     top = lua_gettop( L );

  if ( !lua_isstring( L, -top ) )
  {
    dh::Log( "AddThread, expected string with function name (got %s instead)", lua_typename( L, lua_type( L, -top ) ) );
    lua_pop( L, top );
    return 0;
  }

  GR::String     threadName = lua_tostring( L, -top );
  lua_pop( L, lua_gettop( L ) );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  pLuaHandler->m_Threads.push_back( threadName );

  return 0;
}



int LuaInstance::KillThread( lua_State* L )
{
  if ( !lua_isstring( L, -1 ) )
  {
    lua_pop( L, 1 );
    return 0;
  }

  GR::String     threadName = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  pLuaHandler->DoString( ( threadName + " = nil;" ).c_str() );
  pLuaHandler->m_Threads.remove( threadName );

  return 0;
}



int LuaInstance::GetTime( lua_State* L )
{
  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  lua_pushnumber( L, pLuaHandler->m_CurrentTime * 1000.0f );
  return 1;
}



void LuaInstance::ResumeThreads( const float ElapsedTime )
{
  m_CurrentTime += ElapsedTime;

  std::list<GR::String>::iterator    it( m_Threads.begin() );
  while ( it != m_Threads.end() )
  {
    GR::String&    threadName = *it;

    auto            itTT( m_ThreadThis.find( threadName ) );
    if ( itTT != m_ThreadThis.end() )
    {
      auto&   thisVariable = itTT->second;

      DoString( "this = " + thisVariable );
    }

    lua_getglobal( m_LuaInstance, "LuaContinueThread" );
    lua_getglobal( m_LuaInstance, threadName.c_str() );
    lua_pushnumber( m_LuaInstance, ElapsedTime );

    int error = lua_pcall( m_LuaInstance, 2, 1, 0 );
    if ( error )
    {
      GR::String   errorMsg = lua_tostring( m_LuaInstance, -1 );
      lua_pop( m_LuaInstance, 1 );

      // Thread ist kaputt
      it = m_Threads.erase( it );
      if ( itTT != m_ThreadThis.end() )
      {
        m_ThreadThis.erase( itTT );
      }
      continue;
    }

    int   returnValue = (int)lua_tonumber( m_LuaInstance, -1 );
    lua_pop( m_LuaInstance, 1 );

    if ( returnValue )
    {
      // Thread ist fertig
      it = m_Threads.erase( it );
      if ( itTT != m_ThreadThis.end() )
      {
        m_ThreadThis.erase( itTT );
      }
      continue;
    }

    ++it;
  }
}



bool LuaInstance::IsNumber( int Index )
{
  return !!lua_isnumber( m_LuaInstance, Index );
}



bool LuaInstance::IsNil( int Index )
{
  return !!lua_isnil( m_LuaInstance, Index );
}



GR::ip LuaInstance::ToNumber( int Index )
{
  return (GR::ip)lua_tonumber( m_LuaInstance, Index );
}



GR::f32 LuaInstance::ToF32( int Index )
{
  return (GR::f32)lua_tonumber( m_LuaInstance, Index );
}



int LuaInstance::GetTop()
{
  return lua_gettop( m_LuaInstance );
}



void LuaInstance::PushString( const GR::String& Text )
{
  lua_pushstring( m_LuaInstance, Text.c_str() );
}



void LuaInstance::PushNumber( GR::ip Number )
{
  lua_pushnumber( m_LuaInstance, (lua_Number)Number );
}



void LuaInstance::PushNumberF( GR::f32 Number )
{
  lua_pushnumber( m_LuaInstance, Number );
}



void LuaInstance::PushBoolean( bool Bool )
{
  lua_pushboolean( m_LuaInstance, Bool );
}



void LuaInstance::PushNIL()
{
  lua_pushnil( m_LuaInstance );
}



void LuaInstance::Pop( int Count )
{
  lua_pop( m_LuaInstance, Count );
}



void LuaInstance::PopAll()
{
  lua_pop( m_LuaInstance, lua_gettop( m_LuaInstance ) );
}



void LuaInstance::KillAllThreads()
{
  std::list<GR::String>::iterator    it( m_Threads.begin() );
  while ( it != m_Threads.end() )
  {
    GR::String&    threadName = *it;

    // auf NULL setzen
    lua_pushnil( m_LuaInstance );
    lua_setglobal( m_LuaInstance, threadName.c_str() );

    it = m_Threads.erase( it );
  }
}



GR::up LuaInstance::RunningThreadCount() const
{
  return m_Threads.size();
}



GR::String LuaInstance::ToString( int Index )
{
  if ( !lua_isstring( m_LuaInstance, Index ) )
  {
    return GR::String();
  }
  return lua_tostring( m_LuaInstance, Index );
}



bool LuaInstance::ToBoolean( int Index )
{
  return !!lua_toboolean( m_LuaInstance, Index );
}



bool LuaInstance::IsFunction( int Index )
{
  return lua_isfunction( m_LuaInstance, Index );
}



GR::String LuaInstance::GetGlobal( const GR::String& VarName )
{
  lua_getglobal( m_LuaInstance, VarName.c_str() );
  GR::String strResult = ToString( -1 );
  lua_pop( m_LuaInstance, 1 );

  return strResult;
}



bool LuaInstance::VariableExists( const GR::String& VarName )
{
  size_t      oldPos = 0;
  int         truePushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t      dotPos = VarName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( truePushedParamCount == 0 )
      {
        lua_getglobal( m_LuaInstance, VarName.c_str() );

        bool  exists = !IsNil( -1 );

        PopAll();

        return exists;
      }
      PushString( VarName.substr( oldPos ) );
      ++truePushedParamCount;

      lua_rawget( m_LuaInstance, -2 );

      bool  exists = !IsNil( -1 );

      PopAll();

      return exists;
    }

    lastTable = VarName.substr( oldPos, dotPos - oldPos );
    ++truePushedParamCount;
    if ( truePushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:GetVar - no table %s", VarName.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



GR::String LuaInstance::GetVar( const GR::String& VarName )
{
  size_t      oldPos = 0;
  int         truePushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t      dotPos = VarName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( truePushedParamCount == 0 )
      {
        return GetGlobal( VarName );
      }
      PushString( VarName.substr( oldPos ) );
      ++truePushedParamCount;

      lua_rawget( m_LuaInstance, -2 );

      GR::String   strResult = ToString( -1 );

      //dh::Log( "Type: %s", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 ) ) );

      PopAll();
      return strResult;
    }

    lastTable = VarName.substr( oldPos, dotPos - oldPos );
    ++truePushedParamCount;
    if ( truePushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:GetVar - no table %s", VarName.substr( 0, dotPos ).c_str() );
      return GR::String();
    }

    oldPos = dotPos + 1;
  }
}



bool LuaInstance::SetVar( const GR::String& VarName, const GR::String& Value )
{
  size_t      oldPos = 0;
  int         pushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t dotPos = VarName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( pushedParamCount == 0 )
      {
        SetGlobal( VarName, Value );
        return true;
      }
      PushString( VarName.substr( oldPos ) );
      PushString( Value );

      lua_rawset( m_LuaInstance, -3 );

      PopAll();
      return true;
    }

    lastTable = VarName.substr( oldPos, dotPos - oldPos );
    ++pushedParamCount;
    if ( pushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", VarName.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



bool LuaInstance::SetVar( const GR::String& VarName, const GR::ip Value )
{
  size_t      oldPos = 0;
  int         pushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t  dotPos = VarName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( pushedParamCount == 0 )
      {
        SetGlobal( VarName, Value );
        return true;
      }
      PushString( VarName.substr( oldPos ) );
      PushNumber( (int)Value );

      lua_rawset( m_LuaInstance, -3 );

      PopAll();
      return true;
    }

    lastTable = VarName.substr( oldPos, dotPos - oldPos );
    ++pushedParamCount;
    if ( pushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", VarName.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



bool LuaInstance::SetVar( const GR::String& VarName, const GR::f32 Value )
{
  size_t      oldPos = 0;
  int         pushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t  dotPos = VarName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( pushedParamCount == 0 )
      {
        SetGlobal( VarName, Value );
        return true;
      }
      PushString( VarName.substr( oldPos ) );
      lua_pushnumber( m_LuaInstance, Value );

      lua_rawset( m_LuaInstance, -3 );

      PopAll();
      return true;
    }

    lastTable = VarName.substr( oldPos, dotPos - oldPos );
    ++pushedParamCount;
    if ( pushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", VarName.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



bool LuaInstance::GetTable( const GR::String& TableName )
{
  size_t      oldPos = 0;
  int         pushedParamCount = 0;
  GR::String  lastTable;

  while ( true )
  {
    size_t  dotPos = TableName.find( '.', oldPos );

    if ( dotPos == GR::String::npos )
    {
      // reine Variable
      if ( pushedParamCount == 0 )
      {
        lua_getglobal( m_LuaInstance, TableName.c_str() );
        if ( !lua_istable( m_LuaInstance, -1 ) )
        {
          PopAll();
          return false;
        }
        return true;
      }
      PushString( TableName.substr( oldPos ) );
      ++pushedParamCount;

      lua_rawget( m_LuaInstance, -2 );

      if ( !lua_istable( m_LuaInstance, -1 ) )
      {
        PopAll();
        return false;
      }
      return true;
    }

    lastTable = TableName.substr( oldPos, dotPos - oldPos );
    ++pushedParamCount;
    if ( pushedParamCount == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, lastTable.c_str() );
    }
    else
    {
      PushString( lastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:GetTable - no table %s", TableName.substr( 0, dotPos ).c_str() );
      return false;
    }

    oldPos = dotPos + 1;
  }
}



void LuaInstance::SetGlobal( const GR::String& VarName )
{
  lua_setglobal( m_LuaInstance, VarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& VarName, const GR::String& VarValue )
{
  lua_pushstring( m_LuaInstance, VarValue.c_str() );
  lua_setglobal( m_LuaInstance, VarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& VarName, const GR::ip Value )
{
  lua_pushnumber( m_LuaInstance, (lua_Number)Value );
  lua_setglobal( m_LuaInstance, VarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& VarName, const GR::f32 Value )
{
  lua_pushnumber( m_LuaInstance, Value );
  lua_setglobal( m_LuaInstance, VarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& VarName, const bool Value )
{
  lua_pushboolean( m_LuaInstance, Value );
  lua_setglobal( m_LuaInstance, VarName.c_str() );
}



bool LuaInstance::IsThreadRunning( const GR::String& ThreadName )
{
  std::list<GR::String>::iterator    it( m_Threads.begin() );
  while ( it != m_Threads.end() )
  {
    GR::String&    listedThreadName = *it;

    if ( ThreadName == listedThreadName.substr( 6 ) )
    {
      return true;
    }
    ++it;
  }

  return false;
}



void LuaInstance::RegisterHandler( tHandlerFunction Function, const GR::String& FunctionName )
{
  if ( m_Handler.find( FunctionName ) != m_Handler.end() )
  {
    UnregisterHandler( FunctionName );
  }

  PopAll();

  m_Handler["HandlerFunc" + FunctionName] = Function;

  tHandlerFunction* pFunc = static_cast<tHandlerFunction*>( lua_newuserdata( m_LuaInstance, sizeof( tHandlerFunction ) ) );
  *pFunc = Function;
  lua_pushnumber( m_LuaInstance, (int)(GR::up)this );
  lua_pushcclosure( m_LuaInstance, &LuaInstance::HandlerThunk, 2 );
  lua_setglobal( m_LuaInstance, FunctionName.c_str() );
}



void LuaInstance::UnregisterHandler( const GR::String& FunctionName )
{
  tMapHandler::iterator   it( m_Handler.find( "HandlerFunc" + FunctionName ) );
  if ( it != m_Handler.end() )
  {
    m_Handler.erase( it );
    PushNIL();
    SetGlobal( FunctionName );
  }
}



int LuaInstance::HandlerThunk( lua_State* L )
{
  // stack has userdata
  // get member function from upvalue
  tHandlerFunction* p = static_cast<tHandlerFunction*>( lua_touserdata( L, lua_upvalueindex( 1 ) ) );
  LuaInstance* pLua = (LuaInstance*)(GR::up)lua_tonumber( L, lua_upvalueindex( 2 ) );

  // ACHTUNG: Bei coroutines ist der lua_State ein anderer!! 
  // Dieser Hack geht daneben, wenn ich jemals eine coroutine als ersten Parameter an einen gethunkten Handler übergebe!!

  // TODO - evtl. doch den lua_state übergeben statt LuaInstance?
  lua_State*    pOrigState = pLua->m_LuaInstance;

  pLua->m_LuaInstance = L;

  int   result = (*p)( *pLua );

  pLua->m_LuaInstance = pOrigState;

  return result;
}



void LuaInstance::DumpStack()
{
  for ( int i = 0; i < GetTop(); ++i )
  {
    dh::Log( "Type: (%s)", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 - i ) ) );
  }
}



bool LuaInstance::CallFunction( const GR::String& FunctionName )
{
  lua_getglobal( m_LuaInstance, FunctionName.c_str() );
  if ( !lua_isfunction( m_LuaInstance, -1 ) )
  {
    dh::Log( "Variable '%s' does either not exist or is not a function", FunctionName.c_str() );
    PopAll();
    return false;
  }

  int error = lua_pcall( m_LuaInstance, 0, LUA_MULTRET, 0 );
  if ( error )
  {
    GR::String   errorMessage = ToString( -1 );
    PopAll();

    dh::Log( "CallFunction '%s', error %s", FunctionName.c_str(), errorMessage.c_str() );
    return false;
  }
  int   numTop = GetTop();
  return true;
}


