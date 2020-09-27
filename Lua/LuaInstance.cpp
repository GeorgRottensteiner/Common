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

  //luaopen_base( m_LuaInstance );
  luaL_requiref( m_LuaInstance, "_G", luaopen_base, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "coroutine", luaopen_coroutine, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "string", luaopen_string, 1 );
  Pop();    // remove lib
  luaL_requiref( m_LuaInstance, "table", luaopen_table, 1 );
  Pop();    // remove lib


  Pop( GetTop() );

  /*
function Wait( Ticks )  
  co_ElapsedTicks = 0;  
  while ( co_ElapsedTicks < Ticks ) do    
    local  Result = coroutine.yield( Ticks );    
    co_ElapsedTicks = co_ElapsedTicks + Result;  
  end  
  co_ElapsedTicks = co_ElapsedTicks - Ticks;
end
*/

  /*
    function LuaContinueThread( co_event, ElapsedTime )  
      local result, second_value = coroutine.resume( co_event, ElapsedTime );  
      if ( second_value == nil ) then    
        -- Thread done	
        co_event = nil;    
        return 1;  
      end  
      return 0;
    end
    */

  //lua_pushString( m_LuaInstance, "LuaHandler" );
  //lua_setglobal( m_LuaInstance, 

  DoString( CMisc::printf( "LuaHandler = %d", this ) );

  DoString( "function LuaContinueThread( co_event, ElapsedTime )\n"
            "  local result, second_value = coroutine.resume( co_event, ElapsedTime );\n"
            "  if ( not result ) then\n"
            "    --Ein Fehler!\n"
            "    _ALERT( second_value );\n"
            "    return 0;\n"
            "  end\n"
            "  if ( second_value == nil ) then\n"
            "    -- Thread done\n"
            "    co_event = nil;\n"
            "    return 1;\n"
            "  end\n"
            "  return 0;\n"
            "end" );

  DoString( "function Wait( iTicks )\n"
            "  local WaitStartTime = GetTime();\n"
            //"  _ALERT( \"Wait ticks pos a\" );\n"
            "  while ( true ) do\n"
            "    if ( GetTime() > WaitStartTime + iTicks ) then\n"
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



bool LuaInstance::DoString( const GR::String& strScript )
{
  int errorCode = luaL_dostring( m_LuaInstance, strScript.c_str() );

  if ( errorCode != 0 )
  {
    Alert( m_LuaInstance );
  }
  return ( errorCode == 0 );
}



bool LuaInstance::DoFile( const GR::String& strFilename )
{
  int errorCode = luaL_dofile( m_LuaInstance, strFilename.c_str() );

  if ( errorCode != 0 )
  {
    Alert( m_LuaInstance );
  }
  return ( errorCode == 0 );
}



void LuaInstance::Register( lua_CFunction pFunction, const char* szFunctionName )
{
  if ( ( pFunction == NULL )
  ||   ( szFunctionName == NULL ) )
  {
    return;
  }

  lua_pushcfunction( m_LuaInstance, pFunction );
  lua_setglobal( m_LuaInstance, szFunctionName );
}



bool LuaInstance::FunctionExists( const GR::String& strScript )
{

  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  if ( strScript.find( '.' ) == GR::String::npos )
  {
    // pure variable
    lua_getglobal( m_LuaInstance, strScript.c_str() );

    bool bIsFunction = lua_isfunction( m_LuaInstance, -1 );
    PopAll();
    return bIsFunction;
  }

  while ( true )
  {
    size_t      iDotPos = strScript.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        PushString( strScript );
        lua_rawget( m_LuaInstance, -1 );

        bool bIsFunction = lua_isfunction( m_LuaInstance, -1 );
        PopAll();
        return bIsFunction;
      }
      PushString( strScript.substr( iOldPos ) );
      ++iTruePushedParams;

      lua_rawget( m_LuaInstance, -2 );

      bool bIsFunction = lua_isfunction( m_LuaInstance, -1 );
      PopAll();
      return bIsFunction;
    }

    strLastTable = strScript.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:FunctionExists - no table %s", strScript.substr( 0, iDotPos ).c_str() );
      return false;
    }

    iOldPos = iDotPos + 1;
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
  GR::String     varName = "Thread";
  
  varName += ThreadName;

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
            //"    _ALERT( \"thread done already\" );\n"
            "    " + varName + " = nil;\n"
            "    return 1;\n"
            "  end\n"
            "  LuaAddThread( \"" + varName + "\" );\n"
            "  return 0;\n"
            "end\n" ).c_str() );

  return DoString( "LuaStartThread( " + Function + ");" );
}



int LuaInstance::CreateThread( lua_State* L )
{
  // TODO - wenn Funktion direkt übergeben, auch starten!
  if ( !lua_isstring( L, -1 ) )
  {
    lua_pop( L, 1 );
    return 0;
  }

  GR::String     strThreadName = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  if ( !pLuaHandler->StartThread( strThreadName, strThreadName ) )
  {

  }

  return 0;
}



GR::String LuaInstance::StartScriptAsThread( const GR::String& Script, const GR::String& UserThreadName, const GR::String& ThisVariable )
{
  GR::String     threadName = UserThreadName;

  static int  threadCounter = 0;
  int   newValue = ++threadCounter;
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
    m_ThreadThis[threadName] = ThisVariable;
  }

  return threadName;
}



int LuaInstance::AddThread( lua_State* L )
{
  if ( !lua_isstring( L, -1 ) )
  {
    lua_pop( L, 1 );
    return 0;
  }

  GR::String     strThreadName = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  pLuaHandler->m_Threads.push_back( strThreadName );

  return 0;
}



int LuaInstance::KillThread( lua_State* L )
{
  if ( !lua_isstring( L, -1 ) )
  {
    lua_pop( L, 1 );
    return 0;
  }

  GR::String     strThreadName = lua_tostring( L, -1 );

  lua_pop( L, 1 );

  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  pLuaHandler->DoString( ( strThreadName + " = nil;" ).c_str() );
  pLuaHandler->m_Threads.remove( strThreadName );

  return 0;
}



int LuaInstance::GetTime( lua_State* L )
{
  lua_getglobal( L, "LuaHandler" );
  LuaInstance*   pLuaHandler = (LuaInstance*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  lua_pushnumber( L, pLuaHandler->m_CurrentTime * 1000.0f );//GetTickCount() );
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

    //lua_call( m_LuaInstance, 2, 1 );
    int iError = lua_pcall( m_LuaInstance, 2, 1, 0 );

    if ( iError )
    {
      GR::String   strError = lua_tostring( m_LuaInstance, -1 );
      lua_pop( m_LuaInstance, 1 );
      
      dh::Log( "Thread (%s) error %s\n", threadName.c_str(), strError.c_str() );

      // Thread ist kaputt
      it = m_Threads.erase( it );
      if ( itTT != m_ThreadThis.end() )
      {
        m_ThreadThis.erase( itTT );
      }
      continue;
    }

    int   iReturnValue = (int)lua_tonumber( m_LuaInstance, -1 );
    lua_pop( m_LuaInstance, 1 );

    if ( iReturnValue )
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



GR::ip LuaInstance::ToNumber( int iIndex )
{
  return (GR::ip)lua_tonumber( m_LuaInstance, iIndex );
}



GR::f32 LuaInstance::ToF32( int iIndex )
{
  return (GR::f32)lua_tonumber( m_LuaInstance, iIndex );
}



int LuaInstance::GetTop()
{
  return lua_gettop( m_LuaInstance );
}



void LuaInstance::PushString( const GR::String& strText )
{
  lua_pushstring( m_LuaInstance, strText.c_str() );
}



void LuaInstance::PushNumber( GR::ip iNumber )
{
  lua_pushnumber( m_LuaInstance, (lua_Number)iNumber );
}



void LuaInstance::PushNumberF( GR::f32 Number )
{
  lua_pushnumber( m_LuaInstance, Number );
}



void LuaInstance::PushBoolean( bool bBool )
{
  lua_pushboolean( m_LuaInstance, bBool );
}



void LuaInstance::PushNIL()
{
  lua_pushnil( m_LuaInstance );
}



void LuaInstance::Pop( int iCount )
{
  lua_pop( m_LuaInstance, iCount );
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
    GR::String&    strThreadName = *it;

    // auf NULL setzen
    lua_pushnil( m_LuaInstance );
    lua_setglobal( m_LuaInstance, strThreadName.c_str() );

    it = m_Threads.erase( it );
  }
}



GR::up LuaInstance::RunningThreadCount() const
{
  return m_Threads.size();
}



GR::String LuaInstance::ToString( int iIndex )
{
  if ( !lua_isstring( m_LuaInstance, iIndex ) )
  {
    return GR::String();
  }
  return lua_tostring( m_LuaInstance, iIndex );
}



bool LuaInstance::ToBoolean( int iIndex )
{
  return !!lua_toboolean( m_LuaInstance, iIndex );
}



GR::String LuaInstance::GetGlobal( const GR::String& strVarName )
{
  lua_getglobal( m_LuaInstance, strVarName.c_str() );
  GR::String strResult = ToString( -1 );
  lua_pop( m_LuaInstance, 1 );

  return strResult;
}



GR::String LuaInstance::GetVar( const GR::String& strVarName )
{
  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  while ( true )
  {
    size_t      iDotPos = strVarName.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        return GetGlobal( strVarName );
      }
      PushString( strVarName.substr( iOldPos ) );
      ++iTruePushedParams;

      lua_rawget( m_LuaInstance, -2 );

      GR::String   strResult = ToString( -1 );

      //dh::Log( "Type: %s", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 ) ) );

      PopAll();
      return strResult;
    }

    strLastTable = strVarName.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:GetVar - no table %s", strVarName.substr( 0, iDotPos ).c_str() );
      return GR::String();
    }

    iOldPos = iDotPos + 1;
  }

}



bool LuaInstance::SetVar( const GR::String& strVarName, const GR::String& Value )
{

  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  while ( true )
  {
    size_t      iDotPos = strVarName.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        SetGlobal( strVarName, Value );
        return true;
      }
      PushString( strVarName.substr( iOldPos ) );
      PushString( Value );

      lua_rawset( m_LuaInstance, -3 );

      //dh::Log( "Type: %s", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 ) ) );

      PopAll();
      return true;
    }

    strLastTable = strVarName.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", strVarName.substr( 0, iDotPos ).c_str() );
      return false;
    }

    iOldPos = iDotPos + 1;
  }
}



bool LuaInstance::SetVar( const GR::String& strVarName, const GR::ip Value )
{

  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  while ( true )
  {
    size_t      iDotPos = strVarName.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        SetGlobal( strVarName, Value );
        return true;
      }
      PushString( strVarName.substr( iOldPos ) );
      PushNumber( (int)Value );

      lua_rawset( m_LuaInstance, -3 );

      //dh::Log( "Type: %s", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 ) ) );

      PopAll();
      return true;
    }

    strLastTable = strVarName.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", strVarName.substr( 0, iDotPos ).c_str() );
      return false;
    }

    iOldPos = iDotPos + 1;
  }
}



bool LuaInstance::SetVar( const GR::String& strVarName, const GR::f32 Value )
{

  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  while ( true )
  {
    size_t      iDotPos = strVarName.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        SetGlobal( strVarName, Value );
        return true;
      }
      PushString( strVarName.substr( iOldPos ) );
      lua_pushnumber( m_LuaInstance, Value );

      lua_rawset( m_LuaInstance, -3 );

      //dh::Log( "Type: %s", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 ) ) );

      PopAll();
      return true;
    }

    strLastTable = strVarName.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:SetVar - no table %s", strVarName.substr( 0, iDotPos ).c_str() );
      return false;
    }

    iOldPos = iDotPos + 1;
  }
}



bool LuaInstance::GetTable( const GR::String& strTableName )
{

  size_t      iOldPos = 0;

  int         iPushedParams = 0;
  int         iTruePushedParams = 0;

  GR::String strLastTable;

  while ( true )
  {
    size_t      iDotPos = strTableName.find( '.', iOldPos );

    if ( iDotPos == GR::String::npos )
    {
      // reine Variable
      if ( iTruePushedParams == 0 )
      {
        lua_getglobal( m_LuaInstance, strTableName.c_str() );
        if ( !lua_istable( m_LuaInstance, -1 ) )
        {
          PopAll();
          return false;
        }
        return true;
      }
      PushString( strTableName.substr( iOldPos ) );
      ++iTruePushedParams;

      lua_rawget( m_LuaInstance, -2 );

      if ( !lua_istable( m_LuaInstance, -1 ) )
      {
        PopAll();
        return false;
      }
      return true;
    }

    strLastTable = strTableName.substr( iOldPos, iDotPos - iOldPos );
    ++iTruePushedParams;
    if ( iTruePushedParams == 1 )
    {
      // fetch from the global table
      lua_getglobal( m_LuaInstance, strLastTable.c_str() );
      //lua_gettable( m_LuaInstance, LUA_GLOBALSINDEX );
    }
    else
    {
      PushString( strLastTable );
      lua_rawget( m_LuaInstance, -2 );
    }
    if ( !lua_istable( m_LuaInstance, -1 ) )
    {
      PopAll();
      dh::Log( "LuaInstance:GetTable - no table %s", strTableName.substr( 0, iDotPos ).c_str() );
      return false;
    }

    iOldPos = iDotPos + 1;
  }
}



void LuaInstance::SetGlobal( const GR::String& strVarName )
{
  lua_setglobal( m_LuaInstance, strVarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& strVarName, const GR::String& strVarValue )
{
  lua_pushstring( m_LuaInstance, strVarValue.c_str() );
  lua_setglobal( m_LuaInstance, strVarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& strVarName, const GR::ip iValue )
{
  lua_pushnumber( m_LuaInstance, (lua_Number)iValue );
  lua_setglobal( m_LuaInstance, strVarName.c_str() );
}



void LuaInstance::SetGlobal( const GR::String& VarName, const GR::f32 Value )
{

  lua_pushnumber( m_LuaInstance, Value );
  lua_setglobal( m_LuaInstance, VarName.c_str() );

}



void LuaInstance::SetGlobal( const GR::String& strVarName, const bool Value )
{

  lua_pushboolean( m_LuaInstance, Value );
  lua_setglobal( m_LuaInstance, strVarName.c_str() );

}



bool LuaInstance::IsThreadRunning( const GR::String& strThreadName )
{

  std::list<GR::String>::iterator    it( m_Threads.begin() );
  while ( it != m_Threads.end() )
  {
    GR::String&    strListedThreadName = *it;

    if ( strThreadName == strListedThreadName.substr( 6 ) )
    {
      return true;
    }
    ++it;
  }

  return false;

}



void LuaInstance::RegisterHandler( tHandlerFunction Function, const GR::String& strFunctionName )
{
  if ( m_Handler.find( strFunctionName ) != m_Handler.end() )
  {
    UnregisterHandler( strFunctionName );
  }

  PopAll();

  m_Handler["HandlerFunc" + strFunctionName] = Function;

  tHandlerFunction* pFunc = static_cast<tHandlerFunction*>( lua_newuserdata( m_LuaInstance, sizeof( tHandlerFunction ) ) );
  *pFunc = Function;
  lua_pushnumber( m_LuaInstance, (int)(GR::up)this );
  lua_pushcclosure( m_LuaInstance, &LuaInstance::HandlerThunk, 2 );
  lua_setglobal( m_LuaInstance, strFunctionName.c_str() );
}



void LuaInstance::UnregisterHandler( const GR::String& strFunctionName )
{
  tMapHandler::iterator   it( m_Handler.find( "HandlerFunc" + strFunctionName ) );
  if ( it != m_Handler.end() )
  {
    m_Handler.erase( it );
    PushNIL();
    SetGlobal( strFunctionName ); 
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

  int   iResult = (*p)( *pLua );

  pLua->m_LuaInstance = pOrigState;

  return iResult;
}



void LuaInstance::DumpStack()
{
  for ( int i = 0; i < GetTop(); ++i )
  {
    dh::Log( "Type: (%s)", lua_typename( m_LuaInstance, lua_type( m_LuaInstance, -1 - i ) ) );
  }
}