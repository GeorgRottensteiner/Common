#include <string>

#include <GR/XJumpX/GameObject.h>

#include <Lua/Lua/Luna.h>

#include "LuaGameObject.h"
#include "LevelLayer.h"


const char CLuaGameObject::className[] = "CGameObject";

Luna<CLuaGameObject>::tRegType CLuaGameObject::methods[] =
{
  { "Event",  &CLuaGameObject::Event },
  { "SetPos", &CLuaGameObject::SetPos },
  { "WalkTo", &CLuaGameObject::WalkTo },
  { "SetAnimation", &CLuaGameObject::SetAnimation },
  { "TurnLeft", &CLuaGameObject::TurnLeft },
  { "TurnRight", &CLuaGameObject::TurnRight },
  { "Die",    &CLuaGameObject::Die },
  { "DisablePath",    &CLuaGameObject::DisablePath },
  { "EnablePath",     &CLuaGameObject::EnablePath },
  { 0, 0 }
};



CLuaGameObject::CLuaGameObject( CGameObject* pObj ) :
  m_pObject( pObj )
{
}



int CLuaGameObject::Event( lua_State* L )
{

  int   iStackSize = lua_gettop( L );
  if ( ( iStackSize != 3 )
  &&   ( iStackSize != 2 ) )
  {
    lua_pop( L, lua_gettop( L ) );
    return 0;
  }

  int             iEventType = CGameObject::ET_NONE;

  iEventType = (int)lua_tonumber( L, -iStackSize );
  --iStackSize;

  GR::up iVar = 0;
  iVar = (GR::up)lua_tonumber( L, -iStackSize );
  --iStackSize;

  GR::String     strVar = "";
  if ( iStackSize >= 1 )
  {
    if ( !lua_isnil( L, -iStackSize ) )
    {
      strVar = lua_tostring( L, -iStackSize );
    }
  }

  lua_pop( L, iStackSize );

  if ( m_pObject )
  {
    m_pObject->ProcessEvent( tObjectEvent( iEventType, iVar, 0, strVar ) );
  }

  return 0;

}



int CLuaGameObject::SetPos( lua_State* L )
{

  int   iStackSize = lua_gettop( L );
  if ( iStackSize != 2 )
  {
    lua_pop( L, iStackSize );
    return 0;
  }

  int             iX = (int)lua_tonumber( L, -iStackSize );
  --iStackSize;

  int             iY = (int)lua_tonumber( L, -iStackSize );
  --iStackSize;

  lua_pop( L, 2 );

  if ( m_pObject )
  {
    CLevelLayer*    pLayer = m_pObject->m_pLayer;
    if ( pLayer )
    {
      pLayer->RemoveSleepingObject( m_pObject );
    }
    m_pObject->m_vPosition.x = (float)iX;
    m_pObject->m_vPosition.y = (float)iY;
    if ( ( pLayer )
    &&   ( m_pObject->m_pSpawnBase == NULL ) )
    {
      pLayer->AddSleepingObject( m_pObject );
    }
    if ( m_pObject->m_pSpawnBase )
    {
      // die Spawnbase auch verschieben!
      CLevelLayer*    pLayer = m_pObject->m_pSpawnBase->m_pLayer;
      if ( pLayer )
      {
        pLayer->RemoveSleepingObject( m_pObject->m_pSpawnBase );
      }
      m_pObject->m_pSpawnBase->m_vPosition.x = (float)iX;
      m_pObject->m_pSpawnBase->m_vPosition.y = (float)iY;
      if ( pLayer )
      {
        pLayer->AddSleepingObject( m_pObject->m_pSpawnBase );
      }
    }
  }

  return 0;

}



int CLuaGameObject::WalkTo( lua_State* L )
{

  int   iStackSize = lua_gettop( L );
  if ( iStackSize != 1 )
  {
    lua_pop( L, iStackSize );
    return 0;
  }

  int             iX = (int)lua_tonumber( L, -iStackSize );
  --iStackSize;

  lua_pop( L, 1 );

  if ( m_pObject )
  {
    m_pObject->m_bWalkToTarget = true;
    m_pObject->m_vTargetPos = m_pObject->m_vPosition;
    m_pObject->m_vTargetPos.x = (float)iX;
  }

  return 0;

}



int CLuaGameObject::SetAnimation( lua_State* L )
{

  int   iStackSize = lua_gettop( L );
  if ( iStackSize != 1 )
  {
    lua_pop( L, iStackSize );
    return 0;
  }

  GR::String     strAnimName = lua_tostring( L, -1 );
  lua_pop( L, 1 );

  if ( m_pObject )
  {
    m_pObject->SetAnimation( strAnimName );
  }

  return 0;

}



int CLuaGameObject::TurnLeft( lua_State* L )
{

  if ( m_pObject )
  {
    m_pObject->m_dwFlags |= CGameObject::GOF_FACING_LEFT;
  }
  return 0;

}



int CLuaGameObject::TurnRight( lua_State* L )
{

  if ( m_pObject )
  {
    m_pObject->m_dwFlags &= ~CGameObject::GOF_FACING_LEFT;
  }
  return 0;

}



int CLuaGameObject::Die( lua_State* L )
{

  if ( m_pObject )
  {
    m_pObject->Die();
  }

  return 0;

}



int CLuaGameObject::EnablePath( lua_State* L )
{

  if ( m_pObject )
  {
    m_pObject->m_dwFlags |= CGameObject::GOF_ON_PATH;

    if ( m_pObject->m_pSpawnBase )
    {
      m_pObject->m_pSpawnBase->m_dwFlags |= CGameObject::GOF_ON_PATH;
    }
  }

  return 0;

}



int CLuaGameObject::DisablePath( lua_State* L )
{

  if ( m_pObject )
  {
    m_pObject->m_dwFlags &= ~CGameObject::GOF_ON_PATH;
    if ( m_pObject->m_pSpawnBase )
    {
      m_pObject->m_pSpawnBase->m_dwFlags &= ~CGameObject::GOF_ON_PATH;
    }
  }

  return 0;

}

