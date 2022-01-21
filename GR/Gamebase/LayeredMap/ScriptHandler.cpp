#include "ScriptHandler.h"

#include "TileLayer.h"
#include "JREngine.h"



ScriptEngine::ScriptEngine( GR::Gamebase::JREngine* pJREngine ) :
  m_pJREngine( pJREngine )
{
}



const char ScriptEngine::s_ClassName[] = "LuaJREngine";

Loona<ScriptEngine>::tRegType ScriptEngine::s_Methods[] =
{
  { "FindObjectByID", &ScriptEngine::FindObjectByID },
  { "FindObjectByType", &ScriptEngine::FindObjectByType },
  { "SpawnObject", &ScriptEngine::SpawnObject },
  { "GetControlledObject", &ScriptEngine::GetControlledObject },
  { "SetControlledObject", &ScriptEngine::SetControlledObject },
  { "RemoveObject", &ScriptEngine::RemoveObject },
  { "ChangeMap", &ScriptEngine::ChangeMap },
  { "CurrentMap", &ScriptEngine::CurrentMap },
  { "CenterCameraOnObject", &ScriptEngine::CenterCameraOnObject },
  { 0, 0 }
};



int ScriptEngine::SpawnObject( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 3 )
  {
    Lua.PopAll();
    return 0;
  }

  int x = Lua.ToNumber( -3 );
  int y = Lua.ToNumber( -2 );
  GR::String type = Lua.ToString( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject* pObj = m_pJREngine->SpawnObject( type, x, y );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptEngine::FindObjectByID( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::u32     id = Lua.ToNumber( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)m_pJREngine->FindObjectByID( id );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptEngine::FindObjectByType( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::String type = Lua.ToString( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)m_pJREngine->FindObjectByType( type );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptEngine::ChangeMap( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 3 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::String     mapName = Lua.ToString( -3 );
  int             posX = Lua.ToNumber( -2 );
  int             posY = Lua.ToNumber( -1 );
  Lua.PopAll();

  m_pJREngine->WarpToMap( mapName, posX, posY );
  return 0;
}



int ScriptEngine::CurrentMap( LuaInstance& Lua )
{
  Lua.PopAll();

  Lua.PushString( m_pJREngine->CurrentMap() );
  return 1;
}



int ScriptEngine::CenterCameraOnObject( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();
  if ( pObj == NULL )
  {
    return 0;
  }

  m_pJREngine->CenterCameraOnObject( pObj->m_pObject );
  return 0;
}



int ScriptEngine::RemoveObject( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();
  if ( pObj == NULL )
  {
    return 0;
  }

  m_pJREngine->RemoveObject( pObj->m_pObject );
  return 0;
}



int ScriptEngine::GetControlledObject( LuaInstance& Lua )
{
  Lua.PopAll();

  GR::Gamebase::GameObject*   pObj = m_pJREngine->ControlledObject();
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptEngine::SetControlledObject( LuaInstance& Lua )
{
  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();

  if ( pObj == NULL )
  {
    return 0;
  }

  m_pJREngine->SetControlledObject( pObj->m_pObject );
  return 0;
}



ScriptMap::ScriptMap( GR::Gamebase::JREngine* pEngine, GR::Gamebase::LayeredMap* pMap ) :
  m_pJREngine( pEngine ),
  m_pMap( pMap )
{
}



const char ScriptMap::s_ClassName[] = "LuaScriptMap";

Loona<ScriptMap>::tRegType ScriptMap::s_Methods[] =
{
  { "GetLayer", &ScriptMap::GetLayer },
  { "FindObjectByID", &ScriptMap::FindObjectByID },
  { 0, 0 }
};



int ScriptMap::GetLayer( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::String     var = Lua.ToString( -1 );
  Lua.PopAll();

  GR::Gamebase::Layer*    pLayer = m_pMap->LayerByName( var );
  if ( pLayer == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptLayer>::PushObject( Lua, new ScriptLayer( m_pJREngine, pLayer ) );
  }
  return 1;
}



int ScriptMap::FindObjectByID( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::u32     id = Lua.ToNumber( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)m_pJREngine->FindObjectByID( id );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



ScriptLayer::ScriptLayer( GR::Gamebase::JREngine* pEngine, GR::Gamebase::Layer* pLayer ) :
  m_pJREngine( pEngine ),
  m_pLayer( pLayer )
{
}



const char ScriptLayer::s_ClassName[] = "LuaScriptLayer";

Loona<ScriptLayer>::tRegType ScriptLayer::s_Methods[] =
{
  { "SetField", &ScriptLayer::SetField },
  { "Field", &ScriptLayer::Field },
  { "Width", &ScriptLayer::Width },
  { "Height", &ScriptLayer::Height },
  { "DetachObject", &ScriptLayer::DetachObject },
  { "AttachObject", &ScriptLayer::AttachObject },
  { "AwakenObject", &ScriptLayer::AwakenObject },
  { "SpawnObject", &ScriptLayer::SpawnObject },
  { "FindObjectByType", &ScriptLayer::FindObjectByType },
  { 0, 0 }
};



int ScriptLayer::FindObjectByType( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 1 )
  {
    Lua.PopAll();
    return 0;
  }

  GR::String type = Lua.ToString( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject*   pObj = (GR::Gamebase::GameObject*)( (GR::Gamebase::ObjectLayer*)m_pLayer )->FindObjectByType( type );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptLayer::SpawnObject( LuaInstance& Lua )
{
  if ( Lua.GetTop() != 3 )
  {
    Lua.PopAll();
    return 0;
  }

  int x = Lua.ToNumber( -3 );
  int y = Lua.ToNumber( -2 );
  GR::String type = Lua.ToString( -1 );
  Lua.PopAll();

  GR::Gamebase::GameObject* pObj = m_pJREngine->SpawnObject( type, x, y );
  if ( pObj == NULL )
  {
    Lua.PushNIL();
  }
  else
  {
    Loona<ScriptGameObject>::PushObject( Lua, new ScriptGameObject( m_pJREngine, pObj ) );
  }
  return 1;
}



int ScriptLayer::SetField( LuaInstance& Lua )
{
  int     X = Lua.ToNumber( -3 );
  int     Y = Lua.ToNumber( -2 );
  GR::u16 Field = (GR::u16)Lua.ToNumber( -1 );

  if ( m_pLayer->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
  {
    GR::Gamebase::TileLayer*    pTileLayer = (GR::Gamebase::TileLayer*)m_pLayer;
    pTileLayer->SetField( X, Y, Field );
  }

  Lua.PopAll();
  return 0;
}



int ScriptLayer::Field( LuaInstance& Lua )
{
  int     X = Lua.ToNumber( -2 );
  int     Y = Lua.ToNumber( -1 );

  Lua.PopAll();

  if ( m_pLayer->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
  {
    GR::Gamebase::TileLayer*    pTileLayer = (GR::Gamebase::TileLayer*)m_pLayer;
    Lua.PushNumber( pTileLayer->Field( X, Y ) );
  }
  else
  {
    Lua.PushNumber( 0 );
  }
  return 1;
}



int ScriptLayer::Width( LuaInstance& Lua )
{
  Lua.PopAll();

  if ( m_pLayer->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
  {
    GR::Gamebase::TileLayer*    pTileLayer = ( GR::Gamebase::TileLayer* )m_pLayer;
    Lua.PushNumber( pTileLayer->Width() );
  }
  else
  {
    Lua.PushNumber( 0 );
  }
  return 1;
}



int ScriptLayer::Height( LuaInstance& Lua )
{
  Lua.PopAll();

  if ( m_pLayer->Type == GR::Gamebase::Layer::LT_TILE_LAYER )
  {
    GR::Gamebase::TileLayer*    pTileLayer = ( GR::Gamebase::TileLayer* )m_pLayer;
    Lua.PushNumber( pTileLayer->Height() );
  }
  else
  {
    Lua.PushNumber( 0 );
  }
  return 1;
}



int ScriptLayer::DetachObject( LuaInstance& Lua )
{
  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();

  if ( pObj == NULL )
  {
    return 0;
  }

  GR::Gamebase::ObjectLayer*    pObjLayer = (GR::Gamebase::ObjectLayer*)m_pLayer;

  pObj->m_pObject->m_ProcessingFlags &= GR::Gamebase::ProcessingFlags::AWAKENED;
  pObjLayer->RemoveObject( pObj->m_pObject );
  return 0;
}



int ScriptLayer::AttachObject( LuaInstance& Lua )
{
  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();

  if ( pObj == NULL )
  {
    return 0;
  }

  GR::Gamebase::ObjectLayer*    pObjLayer = (GR::Gamebase::ObjectLayer*)m_pLayer;

  pObjLayer->AddObject( pObj->m_pObject );
  return 0;
}



int ScriptLayer::AwakenObject( LuaInstance& Lua )
{
  ScriptGameObject* pObj = Loona<ScriptGameObject>::PopObject( Lua.m_LuaInstance, -1 );
  Lua.PopAll();

  if ( pObj == NULL )
  {
    return 0;
  }

  m_pJREngine->AwakenObject( pObj->m_pObject );
  return 0;
}



ScriptGameObject::ScriptGameObject( GR::Gamebase::JREngine* pEngine, GR::Gamebase::GameObject* pObject ) :
  m_pJREngine( pEngine ),
  m_pObject( pObject )
{
}



const char ScriptGameObject::s_ClassName[] = "LuaScriptGameObject";

Loona<ScriptGameObject>::tRegType ScriptGameObject::s_Methods[] =
{
  { "SetPosition", &ScriptGameObject::SetPosition },
  { "SetSection", &ScriptGameObject::SetSection },
  { "GetX", &ScriptGameObject::GetX },
  { "GetY", &ScriptGameObject::GetY },
  { "Show", &ScriptGameObject::Show },
  { "Hide", &ScriptGameObject::Hide },
  { "SetOnTop", &ScriptGameObject::SetOnTop },
  { "RemoveObject", &ScriptGameObject::RemoveObject },
  { 0, 0 }
};



int ScriptGameObject::GetX( LuaInstance& Lua )
{
  Lua.PopAll();

  Lua.PushNumber( m_pObject->Position.x );
  return 1;
}



int ScriptGameObject::GetY( LuaInstance& Lua )
{
  Lua.PopAll();

  Lua.PushNumber( m_pObject->Position.y );
  return 1;
}



int ScriptGameObject::Show( LuaInstance& Lua )
{
  Lua.PopAll();

  m_pObject->Flags &= ~GR::Gamebase::LayerObject::OF_HIDDEN;
  return 0;
}



int ScriptGameObject::Hide( LuaInstance& Lua )
{
  Lua.PopAll();

  m_pObject->Flags |= GR::Gamebase::LayerObject::OF_HIDDEN;
  return 0;
}



int ScriptGameObject::SetOnTop( LuaInstance& Lua )
{
  Lua.PopAll();

  m_pJREngine->SetObjectOnTop( m_pObject );

  return 0;
}



int ScriptGameObject::RemoveObject( LuaInstance& Lua )
{
  Lua.PopAll();

  m_pObject->DeleteMe();

  return 0;
}



int ScriptGameObject::SetPosition( LuaInstance& Lua )
{
  int     X = Lua.ToNumber( -2 );
  int     Y = Lua.ToNumber( -1 );

  Lua.PopAll();

  GR::Gamebase::TileLayer   layer;

  layer.SetObjectPos( m_pObject, GR::tPoint( X, Y ) );
  return 0;
}



int ScriptGameObject::SetSection( LuaInstance& Lua )
{
  GR::String     section = Lua.ToString( -1 );

  Lua.PopAll();

  m_pObject->SetSection( section );
  return 0;
}



ScriptHandler::ScriptHandler()
{
  LuaInit();
  Loona<ScriptMap>::Register( *this );
  Loona<ScriptLayer>::Register( *this );
  Loona<ScriptGameObject>::Register( *this );
}



ScriptHandler::~ScriptHandler()
{
}
