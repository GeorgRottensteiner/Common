// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable:4786 )
#include <GR/XJump3d/LevelLayer.h>
#include <GR/XJump3d/GameObject.h>
#include <GR/XJump3d/XJump.h>

#include <debug/debugclient.h>

#include <Misc/Misc.h>
#include <Misc/CloneFactory.h>

#include <Spiel/ObjectPath.h>



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CLevelLayer::CLevelLayer() :
    C2DLayer<WORD>(),
    m_dwUserData( 0 ),
    m_vRelativScrollSpeed( 100, 100, 0 ),
    m_vTileSize( 32, 32 ),
    m_Type( LT_INVALID ),
    m_dwFlags( 0 )
{

}



CLevelLayer::~CLevelLayer()
{

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    delete *it;
    ++it;
  }
  m_listAliveObjects.clear();

  tMapSleepingObjects::iterator    it2( m_mapSleepingObjects.begin() );
  while ( it2 != m_mapSleepingObjects.end() )
  {
    tListObjects&   ListObjects = it2->second;

    tListObjects::iterator    it( ListObjects.begin() );
    while ( it != ListObjects.end() )
    {
      delete *it;
      ++it;
    }

    ++it2;
  }
  m_mapSleepingObjects.clear();

}



/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CLevelLayer::Create( int iWidth, int iHeight, const char* szDesc, eLayerType eType )
{

  C2DLayer<WORD>::InitFeld( iWidth, iHeight, szDesc );
  m_Type = eType;

  m_rectVisualRange.position( 0, 0 );
  m_rectVisualRange.size( iWidth, iHeight );

  return true;

}



/*-Save-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::Save( IIOStream& File )
{

  File.WriteLong( Width() );
  File.WriteLong( Height() );
  File.WriteLong( 2 );
  File.WriteLong( m_Type );
  File.WriteLong( m_vTileSize.x );
  File.WriteLong( m_vTileSize.y );
  File.WriteLong( m_dwFlags );

  File.WriteLong( 0 );   // Extra-Data-Length

  File.WriteString( m_strDesc );

  File.WriteBlock( m_pData, Width() * Height() * 2 );

  // Objekte
  int   iTrueObjectCount = 0;
  tMapSleepingObjects::iterator    it( m_mapSleepingObjects.begin() );
  while ( it != m_mapSleepingObjects.end() )
  {
    tListObjects&   ListObjects = it->second;

    iTrueObjectCount += (int)ListObjects.size();
    ++it;
  }

  File.WriteLong( iTrueObjectCount );
  tMapSleepingObjects::iterator    it2( m_mapSleepingObjects.begin() );
  while ( it2 != m_mapSleepingObjects.end() )
  {
    tListObjects&   ListObjects = it2->second;

    tListObjects::iterator    itList( ListObjects.begin() );
    while ( itList != ListObjects.end() )
    {
      CGameObject*    pObj = *itList;

      pObj->Save( File );
      ++itList;
    }

    ++it2;
  }

  // Trigger-Areas
  File.WriteLong( (DWORD)m_listTrigger.size() );
  tListTrigger::iterator    itTrigger( m_listTrigger.begin() );
  while ( itTrigger != m_listTrigger.end() )
  {
    tTriggerArea&   Trigger = *itTrigger;

    File.WriteBlock( &Trigger, sizeof( Trigger ) );

    ++itTrigger;
  }

}



/*-Load-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::Load( IIOStream& File )
{

  WORD wWidth   = (WORD)File.ReadLong();
  WORD wHeight  = (WORD)File.ReadLong();

  File.ReadLong();  // sollte 2 sein (Bytetiefe des Layers)

  m_Type = (CLevelLayer::eLayerType)File.ReadLong();
  m_vTileSize.x = File.ReadLong();
  m_vTileSize.y = File.ReadLong();
  m_dwFlags = File.ReadLong();

  DWORD   dwExtraDataLength = File.ReadLong();
  if ( dwExtraDataLength >= 4 )
  {
    dwExtraDataLength -= 4;
  }

  File.ReadString( m_strDesc );

  Create( wWidth, wHeight, m_strDesc.c_str(), m_Type );

  File.ReadBlock( m_pData, Width() * Height() * 2 );

  // Objekte
  int   iObjectCount = File.ReadLong();
  for ( int i = 0; i < iObjectCount; ++i )
  {
    CGameObject     tempObj;

    tempObj.Load( File );

    CGameObject* pGO = CXJump::Instance().CreateObject( CXJump::Instance().m_vectKnownObjects[tempObj.m_dwType].first );

    if ( pGO )
    {
      pGO->m_pLayer     = this;

      pGO->m_vPosition  = tempObj.m_vPosition;
      pGO->m_vDelta     = tempObj.m_vDelta;
      pGO->m_dwAnimID   = tempObj.m_dwAnimID;
      pGO->m_dwType     = tempObj.m_dwType;
      pGO->m_dwID       = tempObj.m_dwID;
      pGO->m_dwFlags    = tempObj.m_dwFlags;
      pGO->m_iZLayer    = tempObj.m_iZLayer;
      pGO->m_dwExtraData  = tempObj.m_dwExtraData;
      pGO->SetAnimation( pGO->m_dwAnimID );

      AddSleepingObject( pGO );
    }
  }

  // Trigger-Areas
  m_listTrigger.clear();
  int   iTriggerCount = File.ReadLong();
  for ( i = 0; i < iTriggerCount; ++i )
  {
    tTriggerArea    Trigger;

    File.ReadBlock( &Trigger, sizeof( Trigger ) );

    m_listTrigger.push_back( Trigger );
  }

}



/*-AddSleepingObject----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::AddSleepingObject( CGameObject* pObj )
{

  tPairCoord    Coord = std::make_pair( (int)( pObj->m_vPosition.x / 200 ),
                                        (int)( pObj->m_vPosition.y / 200 ) );

  m_mapSleepingObjects[Coord].push_back( pObj );

}



/*-AddObject------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::AddObject( CGameObject* pObj )
{

  pObj->m_pLayer = this;

  // nach Z-Order einordnen
  tListObjects::iterator   it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    CGameObject*    pOtherObj = *it;

    if ( pOtherObj->m_iZLayer >= pObj->m_iZLayer )
    {
      m_listAliveObjects.insert( it, pObj );
      return;
    }

    ++it;
  }

  m_listAliveObjects.push_back( pObj );

}



/*-RemoveSleepingObject-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::RemoveSleepingObject( CGameObject* pObj )
{

  tPairCoord    Coord = std::make_pair( (int)( pObj->m_vPosition.x / 200 ),
                                        (int)( pObj->m_vPosition.y / 200 ) );

  m_mapSleepingObjects[Coord].remove( pObj );
  pObj->m_pLayer = NULL;

}



/*-GetObject------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGameObject* CLevelLayer::GetObject( DWORD dwType )
{

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *it;

    if ( pObj->m_dwType == dwType )
    {
      return pObj;
    }

    ++it;
  }

  return NULL;

}



/*-GetObject------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGameObject* CLevelLayer::GetObject( const GR::String& strType )
{

  CGameObject* pKnownObj = CXJump::Instance().GetKnownObject( strType );
  if ( pKnownObj == NULL )
  {
    return NULL;
  }

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *it;

    if ( pObj->m_dwType == pKnownObj->m_dwType )
    {
      return pObj;
    }

    ++it;
  }

  return NULL;

}



/*-GetObjectByID--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CGameObject* CLevelLayer::GetObjectByID( GR::u32 dwID )
{

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *it;

    if ( pObj->m_dwID == dwID )
    {
      return pObj;
    }

    ++it;
  }

  tMapSleepingObjects::iterator   itSO( m_mapSleepingObjects.begin() );
  while ( itSO != m_mapSleepingObjects.end() )
  {
    tListObjects&   listSleepingObjects = itSO->second;

    tListObjects::iterator    it( listSleepingObjects.begin() );
    while ( it != listSleepingObjects.end() )
    {
      CGameObject*    pObj = *it;

      if ( pObj->m_dwID == dwID )
      {
        return pObj;
      }

      ++it;
    }

    ++itSO;
  }

  return NULL;

}



/*-SpawnObject----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::SpawnObject( CGameObject* pObj )
{

  if ( pObj == NULL )
  {
    return;
  }
  if ( ( pObj->m_dwFlags & CGameObject::GOF_SPAWNED )
  &&   ( !( pObj->m_dwFlags & CGameObject::GOF_RESPAWN ) ) )
  {
    // das Objekt wurde bereits gespawnt
    return;
  }
  CGameObject*    pObjAwake = (CGameObject*)pObj->Clone();

  pObjAwake->m_pSpawnBase = pObj;
  pObj->m_dwFlags |= CGameObject::GOF_SPAWNED;

  AddObject( pObjAwake );
  pObjAwake->ProcessEvent( tObjectEvent( CGameObject::ET_INIT ) );

}



/*-SpawnObjects---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::SpawnObjects( float fX, float fY, float fWidth, float fHeight )
{

  int   iX1 = (int)( fX / 200 - 1 ),
        iY1 = (int)( fY / 200 - 1 ),
        iX2 = (int)( ( fX + fWidth ) / 200 ),
        iY2 = (int)( ( fY + fHeight ) / 200 );

  for ( int i = iX1; i <= iX2; ++i )
  {
    for ( int j = iY1; j <= iY2; ++j )
    {
      tMapSleepingObjects::iterator   itMap( m_mapSleepingObjects.find( tPairCoord( i, j ) ) );
      if ( itMap == m_mapSleepingObjects.end() )
      {
        continue;
      }
      tListObjects&   ListObjects = itMap->second;

      tListObjects::iterator    it( ListObjects.begin() );
      while ( it != ListObjects.end() )
      {
        CGameObject*    pObj = *it;

        SpawnObject( pObj );

        ++it;
      }
    }
  }

}



/*-SleepObject----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::SleepObject( CGameObject* pObj )
{

  if ( pObj == NULL )
  {
    return;
  }
  if ( pObj->m_pSpawnBase == NULL )
  {
    pObj->Die();
    return;
  }

  /*
  if ( ( pObj->m_pSpawnBase->m_dwFlags & CGameObject::GOF_SPAWNED )
  &&   ( !( pObj->m_pSpawnBase->m_dwFlags & CGameObject::GOF_RESPAWN ) ) )
  {
    // das Objekt wurde bereits gespawnt, und soll nicht mehr gespawnt werden
    pObj->Die();
    return;
  }
  */
  pObj->m_pSpawnBase->m_vPosition = pObj->m_vPosition;
  pObj->m_pSpawnBase->m_dwFlags &= ~CGameObject::GOF_SPAWNED;
  pObj->Die();

}



/*-PrepareForStart------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::PrepareForStart()
{

  // ist ein Spieler hier drin?
  CGameObject*    pObjPlayer = CXJump::Instance().GetKnownObject( "Player" );
  if ( pObjPlayer == NULL )
  {
    // es gibt keine Spieler-Vorlage
    dh::Log( "PrepareForStart - no Player Object found\n" );
    return;
  }

  tMapSleepingObjects::iterator   itMap( m_mapSleepingObjects.begin() );
  while ( itMap != m_mapSleepingObjects.end() )
  {
    tListObjects&   ListObjects = itMap->second;

    tListObjects::iterator    itList( ListObjects.begin() );
    while ( itList != ListObjects.end() )
    {
      CGameObject*    pObj = *itList;

      if ( pObjPlayer->m_dwType == pObj->m_dwType )
      {
        SpawnObject( pObj );
      }

      tExtraData*   pED = CXJump::Instance().m_currentMap.GetExtraData( pObj->m_dwExtraData );
      if ( pED )
      {
        if ( pED->m_Type == tExtraData::EX_PATH )
        {
          CXJump::tMapObjectPaths::iterator   itPath( CXJump::Instance().m_mapObjectPaths.find( pED->m_strParam ) );
          if ( itPath != CXJump::Instance().m_mapObjectPaths.end() )
          {
            GR::GAME::CObjectPath&    OPath = itPath->second;

            pObj->m_pathPos = OPath.begin();
            pObj->m_dwFlags |= CGameObject::GOF_ON_PATH;
          }
        }
      }

      ++itList;
    }

    ++itMap;
  }

}



/*-Update---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::Update( const float fElapsedTime )
{

  CXJump&   XJump = CXJump::Instance();

  tListObjects::iterator    itObj( m_listAliveObjects.begin() );
  while ( itObj != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *itObj;

    if ( pObj->m_dwFlags & CGameObject::GOF_REMOVE_ME )
    {
      if ( pObj == XJump.m_pControlledObject )
      {
        XJump.m_pControlledObject = NULL;
      }
      delete pObj;
      itObj = m_listAliveObjects.erase( itObj );
      continue;
    }

    // ist das Objekt außerhalb?
    if ( ( pObj->m_vPosition.x < XJump.m_currentMap.m_vectScrollOffset.x - XJump.m_fFallAsleepRange )
    ||   ( pObj->m_vPosition.y < XJump.m_currentMap.m_vectScrollOffset.y - XJump.m_fFallAsleepRange )
    ||   ( pObj->m_vPosition.x > XJump.m_currentMap.m_vectScrollOffset.x + XJump.m_ptViewportExtents.x + XJump.m_fFallAsleepRange )
    ||   ( pObj->m_vPosition.y > XJump.m_currentMap.m_vectScrollOffset.y + XJump.m_ptViewportExtents.y + XJump.m_fFallAsleepRange ) )
    {
      if ( pObj != XJump.m_pControlledObject )
      {
        SleepObject( pObj );
        ++itObj;
        continue;
      }
    }
    pObj->Update( fElapsedTime );

    ++itObj;
  }

  // Trigger
  CGameObject*    pControlledObject = CXJump::Instance().m_pControlledObject;
  if ( pControlledObject )
  {
    math::tRect<int>                    vecCR   = pControlledObject->ColissionRect();

    tListTrigger::iterator    itTrigger( m_listTrigger.begin() );
    while ( itTrigger != m_listTrigger.end() )
    {
      tTriggerArea&   Trigger = *itTrigger;

      if ( Trigger.m_Rect.intersects( vecCR ) )
      {
        if ( pControlledObject->m_pCurrentTrigger != &Trigger )
        {
          // der Spieler ist voll drin
          pControlledObject->m_pCurrentTrigger = &Trigger;

          pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_TRIGGER, (GR::up)&Trigger ) );
          // der Trigger wird nicht mehr benötigt
          //itTrigger = m_listTrigger.erase( itTrigger );
          //continue;
        }
      }
      else if ( pControlledObject->m_pCurrentTrigger == &Trigger )
      {
        pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_LEAVE_TRIGGER, (GR::up)pControlledObject->m_pCurrentTrigger ) );
        pControlledObject->m_pCurrentTrigger = NULL;
      }

      ++itTrigger;
    }
  }

  tListObjects::iterator    itObj2( m_listAliveObjects.begin() );
  while ( itObj2 != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *itObj2;

    if ( ( pObj->m_dwFlags & CGameObject::GOF_NO_COLISSION )
    ||   ( pObj->m_dwFlags & CGameObject::GOF_REMOVE_ME ) )
    {
      ++itObj2;
      continue;
    }

    CXJump::tPoint    ObjPos = pObj->TruePosition();

    math::tRect<int>        vecCR = pObj->m_vColissionRect;

    vecCR.offset( ObjPos );

    tListObjects::iterator    itObj3( m_listAliveObjects.begin() );
    while ( itObj3 != m_listAliveObjects.end() )
    {
      CGameObject*    pObj2 = *itObj3;

      if ( ( pObj != pObj2 )
      &&   ( !( pObj2->m_dwFlags & CGameObject::GOF_NO_COLISSION ) )
      &&   ( !( pObj2->m_dwFlags & CGameObject::GOF_REMOVE_ME ) ) )
      {
        CXJump::tPoint    Obj2Pos = pObj2->TruePosition();

        math::tRect<int>        vecCR2 = pObj2->m_vColissionRect;

        vecCR2.offset( Obj2Pos );

        if ( vecCR.intersects( vecCR2 ) )
        {
          pObj->ProcessEvent( tObjectEvent( CGameObject::ET_COLLIDE, (GR::up)pObj2 ) );
          pObj2->ProcessEvent( tObjectEvent( CGameObject::ET_COLLIDE, (GR::up)pObj ) );
        }
      }

      ++itObj3;
    }

    ++itObj2;
  }

}



/*-SetVisualRange-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLevelLayer::SetVisualRange( int iX, int iY, int iWidth, int iHeight )
{

  if ( ( iX == -1 )
  &&   ( iY == -1 )
  &&   ( iWidth == -1 )
  &&   ( iHeight == -1 ) )
  {
    m_rectVisualRange.position( 0, 0 );
    m_rectVisualRange.size( Width(), Height() );
    return;
  }
  m_rectVisualRange.position( iX, iY );
  m_rectVisualRange.size( iWidth, iHeight );

}



CGameObject* CLevelLayer::FindCollidingObject( CGameObject* pCollider, const GR::String& strType )
{

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *it;

    if ( ( pObj->Type() == strType )
    &&   ( pObj->ColissionRect().intersects( pCollider->ColissionRect() ) ) )
    {
      return pObj;
    }

    ++it;
  }
  return NULL;

}