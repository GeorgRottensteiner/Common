#include <string>

#include <GR/XJump3d/GameObject.h>

#include <Lua/Lua/Luna.h>

#include "LuaGameObject.h"
#include "LevelLayer.h"


const char CLuaGameObject::className[] = "CGameObject";

Luna<CLuaGameObject>::tRegType CLuaGameObject::methods[] =
{
  { "Event",  CLuaGameObject::Event },
  { "SetPos", CLuaGameObject::SetPos },
  { "Die",    CLuaGameObject::Die },
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
    if ( pLayer )
    {
      pLayer->AddSleepingObject( m_pObject );
    }
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

