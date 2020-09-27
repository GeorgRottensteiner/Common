#include <string>

#include <GR/XJump3d/XJump.h>

#include <Misc/CloneFactory.h>

#include <DX8/DX8Viewer.h>

#include <String/Path.h>

#include <Misc/Misc.h>

#include "jumplua.h"
#include "LuaGameObject.h"



CJumpLua::CJumpLua()
{

  DoString( CMisc::printf( "XJump = %d", this ) );

  lua_pushnumber( m_LuaInstance, CGameObject::ET_USER );
  lua_setglobal( m_LuaInstance, "ET_USER" );

  Luna<CLuaGameObject>::Register( m_LuaInstance );

}

CJumpLua::~CJumpLua()
{
}



void CJumpLua::InitInstance()
{

  Register( CJumpLua::ChangeMap,     "ChangeMap" );
  Register( CJumpLua::Warp,          "Warp" );
  Register( CJumpLua::StartMovie,    "StartMovie" );
  Register( CJumpLua::SetFeld,       "SetFeld" );
  Register( CJumpLua::GetObjectById, "GetObjectById" );
  Register( CJumpLua::CreateObject,  "CreateObject" );
  Register( CJumpLua::SetBackground, "SetBackground" );
  Register( CJumpLua::Freeze,        "Freeze" );
  Register( CJumpLua::MovieMode,     "MovieMode" );
  Register( CJumpLua::WalkTarget,    "WalkTarget" );

  Register( CJumpLua::SetVar,        "SetVar" );
  Register( CJumpLua::GetVar,        "GetVar" );
  Register( CJumpLua::GetVarI,       "GetVarI" );

}



int CJumpLua::ChangeMap( lua_State* L )
{

  int   iTop = lua_gettop( L );

  if ( ( iTop != 1 )
  &&   ( iTop != 3 ) )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int     iX = 0,
          iY = 0;

  if ( iTop == 3 )
  {
    iX = (int)lua_tonumber( L, -iTop );
    iTop--;
    iY = (int)lua_tonumber( L, -iTop );
    iTop--;
  }


  GR::String     strNewMap = "";

  if ( !lua_isnil( L, -1 ) )
  {
    strNewMap = lua_tostring( L, -1 );
  }

  if ( !strNewMap.empty() )
  {
    CXJump::Instance().PostEventOnQueue( tGameEvent( tGameEvent::GE_CHANGE_MAP, strNewMap, ( iX | iY ) ? 1 : 0, iX, iY ) );
  }

  return 0;

}



int CJumpLua::Warp( lua_State* L )
{

  int   iTop = lua_gettop( L );

  if ( iTop != 2 )
  {
    lua_pop( L, iTop );
    return 0;
  }

  int     iX = 0,
          iY = 0;

  iX = (int)lua_tonumber( L, -iTop );
  iTop--;
  iY = (int)lua_tonumber( L, -iTop );
  iTop--;

  lua_pop( L, 2 );

  CXJump::Instance().PostEventOnQueue( tGameEvent( tGameEvent::GE_WARP, "", iX, iY ) );

  return 0;

}



int CJumpLua::SetBackground( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  if ( CXJump::Instance().m_pTextureBackground )
  {
    CD3DViewer::Instance().DestroyTexture( CXJump::Instance().m_pTextureBackground );
    CXJump::Instance().m_pTextureBackground = NULL;
  }

  if ( !lua_isnil( L, -1 ) )
  {
    GR::String strNewBackground = lua_tostring( L, -1 );

    GR::String     strFullPath = CPath::Normalize( CPath::Append( CXJump::Instance().m_strDataPath, strNewBackground ) );
    CXJump::Instance().m_pTextureBackground = CD3DViewer::Instance().LoadTexture( strFullPath.c_str() );
  }

  return 0;

}



int CJumpLua::StartMovie( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strMovieScript = "";

  if ( !lua_isnil( L, -1 ) )
  {
    strMovieScript = lua_tostring( L, -1 );
  }

  lua_pop( L, 1 );

  lua_getglobal( L, "XJump" );
  CJumpLua*   pJLua = (CJumpLua*)(GR::up)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  pJLua->DoFile( CPath::Normalize( CPath::Append( CXJump::Instance().m_strDataPath, strMovieScript ) ).c_str() );

  return 0;

}



int CJumpLua::SetFeld( lua_State* L )
{

  if ( lua_gettop( L ) != 4 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strMapName = "";

  if ( !lua_isnil( L, -4 ) )
  {
    strMapName = lua_tostring( L, -4 );
  }


  int iX = (int)lua_tonumber( L, -3 );
  int iY = (int)lua_tonumber( L, -2 );
  int iValue = (int)lua_tonumber( L, -1 );

  lua_pop( L, 4 );

  CLevelLayer*    pLayer = CXJump::Instance().m_currentMap.GetLayer( strMapName );

  if ( pLayer )
  {
    pLayer->SetFeld( iX, iY, iValue );
  }

  return 0;

}



int CJumpLua::SetVar( lua_State* L )
{

  if ( lua_gettop( L ) != 2 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strVarName  = "";
  GR::String     strVarValue = "";

  if ( lua_isstring( L, -2 ) )
  {
    strVarName = lua_tostring( L, -2 );
  }
  if ( lua_isstring( L, -1 ) )
  {
    strVarValue = lua_tostring( L, -1 );
  }

  lua_pop( L, 2 );

  if ( strVarName.empty() )
  {
    return 0;
  }

  CXJump::Instance().m_GameVars.SetVar( strVarName, strVarValue );
  return 0;

}



int CJumpLua::GetVar( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strVarName  = "";

  if ( lua_isstring( L, -1 ) )
  {
    strVarName = lua_tostring( L, -1 );
  }

  lua_pop( L, 1 );

  if ( strVarName.empty() )
  {
    lua_pushstring( L, "" );
  }
  else
  {
    lua_pushstring( L, CXJump::Instance().m_GameVars.GetVar( strVarName ).c_str() );
  }

  return 1;

}



int CJumpLua::GetVarI( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strVarName  = "";

  if ( lua_isstring( L, -1 ) )
  {
    strVarName = lua_tostring( L, -1 );
  }

  lua_pop( L, 1 );

  if ( strVarName.empty() )
  {
    lua_pushnumber( L, 0 );
  }
  else
  {
    lua_pushnumber( L, CXJump::Instance().m_GameVars.GetVarI( strVarName ) );
  }

  return 1;

}



int CJumpLua::CreateObject( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  GR::String     strObjType = "";

  if ( lua_isstring( L, -1 ) )
  {
    strObjType = lua_tostring( L, -1 );
  }

  lua_pop( L, 1 );

  CGameObject*    pGameObject = (CGameObject*)CXJump::Instance().CreateObject( strObjType );

  if ( pGameObject == NULL )
  {
    lua_pushnil( L );
  }
  else
  {
    CXJump::Instance().m_currentMap.m_pMainLayer->AddObject( pGameObject );
    Luna<CLuaGameObject>::PushObject( L, new CLuaGameObject( pGameObject ) );
  }

  return 1;

}



int CJumpLua::GetObjectById( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int iID = (int)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  CGameObject*    pObj = CXJump::Instance().GetObjectByID( iID );
  if ( pObj == NULL )
  {
    lua_pushnil( L );
    return 1;
  }
  Luna<CLuaGameObject>::PushObject( L, new CLuaGameObject( pObj ) );
  return 1;

}



int CJumpLua::Freeze( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int   iFreeze = (int)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  CXJump::Instance().Freeze( !!iFreeze );
  return 0;

}



int CJumpLua::MovieMode( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int   iMode = (int)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  CXJump::Instance().MovieMode( !!iMode );
  return 0;

}



int CJumpLua::WalkTarget( lua_State* L )
{

  if ( lua_gettop( L ) != 1 )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int   iDX = (int)lua_tonumber( L, -1 );
  lua_pop( L, 1 );

  CXJump::Instance().m_pControlledObject->m_vTargetPos = CXJump::Instance().m_pControlledObject->m_vPosition;
  CXJump::Instance().m_pControlledObject->m_vTargetPos.x += iDX;
  CXJump::Instance().m_pControlledObject->m_vDelta = math::vector3( 0, 0, 0 );
  CXJump::Instance().m_pControlledObject->m_bWalkToTarget = true;

  return 0;

}





