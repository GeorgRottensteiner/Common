#include <GR/XJumpX/LevelLayer.h>
#include <GR/XJumpX/GameObject.h>
#include <GR/XJumpX/XJump.h>

#include <debug/debugclient.h>

#include <Misc/Misc.h>
#include <Misc/CloneFactory.h>

#include <Game/MovementPath.h>



CLevelLayer::CLevelLayer() :
    GR::Gamebase::TileLayer2D<WORD>(),
    m_dwUserData( 0 ),
    m_vRelativScrollSpeed( 100, 100, 0 ),
    m_vTileSize( 32, 32 ),
    m_Type( LT_INVALID ),
    m_dwFlags( 0 )
{

}



CLevelLayer::~CLevelLayer()
{

  tMapSleepingObjects::iterator    it2( m_mapSleepingObjects.begin() );
  while ( it2 != m_mapSleepingObjects.end() )
  {
    tListObjects&   ListObjects = it2->second;

    tListObjects::iterator    it( ListObjects.begin() );
    while ( it != ListObjects.end() )
    {
      //dh::Log( "Delete Sleeping Obj %x", *it );
      CGameObject*  pSleeping = *it;

      if ( !( pSleeping->m_dwFlags & CGameObject::GOF_SPAWNED ) )
      {
        delete *it;
      }

      ++it;
    }

    ++it2;
  }
  m_mapSleepingObjects.clear();

  tListObjects::iterator    it( m_listAliveObjects.begin() );
  while ( it != m_listAliveObjects.end() )
  {
    //dh::Log( "Delete Alive Obj %x", *it );
    delete *it;
    ++it;
  }
  m_listAliveObjects.clear();

}



bool CLevelLayer::Create( int iWidth, int iHeight, const char* szDesc, eLayerType eType )
{

  GR::Gamebase::TileLayer2D<WORD>::InitFeld( iWidth, iHeight );
  m_Type = eType;
  m_strDesc = szDesc;

  m_rectVisualRange.position( 0, 0 );
  m_rectVisualRange.size( iWidth, iHeight );

  return true;
  
}



void CLevelLayer::Save( IIOStream& File )
{

  File.WriteU32( Width() );
  File.WriteU32( Height() );
  File.WriteU32( 2 );
  File.WriteU32( m_Type );
  File.WriteU32( m_vTileSize.x );
  File.WriteU32( m_vTileSize.y );
  File.WriteU32( m_dwFlags );

  File.WriteU32( 1 );   // Extra-Data-Length

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

  File.WriteU32( iTrueObjectCount );
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
  File.WriteU32( (DWORD)m_listTrigger.size() );
  tListTrigger::iterator    itTrigger( m_listTrigger.begin() );
  while ( itTrigger != m_listTrigger.end() )
  {
    XJump::tTriggerArea&   Trigger = *itTrigger;

    File.WriteBlock( &Trigger, sizeof( Trigger ) );

    ++itTrigger;
  }

  // Extra-Data-Byte
  File.WriteU32( 1 );      // Extra-Daten-Version

  File.WriteU32( (GR::u32)m_listRegions.size() );
  tListRegions::iterator    itRegion( m_listRegions.begin() );
  while ( itRegion != m_listRegions.end() )
  {
    GR::tRect&    rcRegion( *itRegion );

    File.WriteBlock( &rcRegion, sizeof( GR::tRect ) );

    ++itRegion;
  }

}



void CLevelLayer::Load( IIOStream& File )
{

  WORD wWidth   = (WORD)File.ReadU32();
  WORD wHeight  = (WORD)File.ReadU32();

  File.ReadU32();  // sollte 2 sein (Bytetiefe des Layers)

  m_Type = (CLevelLayer::eLayerType)File.ReadU32();
  m_vTileSize.x = File.ReadU32();
  m_vTileSize.y = File.ReadU32();
  m_dwFlags = File.ReadU32();

  GR::u32   dwExtraDataLength = File.ReadU32();

  File.ReadString( m_strDesc );
  
  Create( wWidth, wHeight, m_strDesc.c_str(), m_Type );

  File.ReadBlock( m_pData, Width() * Height() * 2 );

  // Objekte
  int   iObjectCount = File.ReadU32();
  for ( int i = 0; i < iObjectCount; ++i )
  {
    CGameObject     tempObj;

    tempObj.Load( File );

    CGameObject* pGO = NULL;
    
    if ( tempObj.m_dwType < CXJump::Instance().m_vectKnownObjects.size() )
    {
      pGO = CXJump::Instance().CreateObject( CXJump::Instance().m_vectKnownObjects[tempObj.m_dwType].first );
    }
    else
    {
      dh::Log( "Discarding unknown object type" );
    }

    if ( pGO )
    {
      pGO->m_pLayer     = this;

      GR::u32           dwOrigFlags = pGO->m_dwFlags;

      pGO->m_vPosition  = tempObj.m_vPosition;
      pGO->m_vDelta     = tempObj.m_vDelta;
      pGO->m_dwAnimID   = tempObj.m_dwAnimID;
      pGO->m_dwType     = tempObj.m_dwType;
      pGO->m_dwID       = tempObj.m_dwID;
      pGO->m_dwFlags    = tempObj.m_dwFlags;
      pGO->m_iZLayer    = tempObj.m_iZLayer;
      pGO->m_dwExtraData  = tempObj.m_dwExtraData;
      pGO->SetAnimation( pGO->m_dwAnimID );

      if ( dwOrigFlags & CGameObject::GOF_ALIGN_HCENTER )
      {
        pGO->m_dwFlags |= CGameObject::GOF_ALIGN_HCENTER;
      }
      if ( dwOrigFlags & CGameObject::GOF_ALIGN_HRIGHT )
      {
        pGO->m_dwFlags |= CGameObject::GOF_ALIGN_HRIGHT;
      }
      if ( dwOrigFlags & CGameObject::GOF_ALIGN_VBOTTOM )
      {
        pGO->m_dwFlags |= CGameObject::GOF_ALIGN_VBOTTOM;
      }
      if ( dwOrigFlags & CGameObject::GOF_ALIGN_VCENTER )
      {
        pGO->m_dwFlags |= CGameObject::GOF_ALIGN_VCENTER;
      }

      //dh::Log( "Add Sleeping Obj %x", pGO );
      AddSleepingObject( pGO );
    }
  }

  // Trigger-Areas
  m_listTrigger.clear();
  int   iTriggerCount = File.ReadU32();
  for ( int i = 0; i < iTriggerCount; ++i )
  {
    XJump::tTriggerArea    Trigger;

    File.ReadBlock( &Trigger, sizeof( Trigger ) );

    m_listTrigger.push_back( Trigger );
  }

  m_listRegions.clear();
  if ( dwExtraDataLength == 1 )
  {
    // Extra-Data-Byte
    GR::u32   dwVersion = File.ReadU32();

    GR::u32   dwRegions = File.ReadU32();
    for ( GR::u32 i = 0; i < dwRegions; ++i )
    {
      GR::tRect   rcRegion;

      File.ReadBlock( &rcRegion, sizeof( GR::tRect ) );

      m_listRegions.push_back( rcRegion );
    }
  }

}



void CLevelLayer::AddSleepingObject( CGameObject* pObj )
{

  tPairCoord    Coord = std::make_pair( (int)( pObj->m_vPosition.x / 200 ), 
                                        (int)( pObj->m_vPosition.y / 200 ) );

  m_mapSleepingObjects[Coord].push_back( pObj );

}



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



void CLevelLayer::RemoveSleepingObject( CGameObject* pObj )
{

  tPairCoord    Coord = std::make_pair( (int)( pObj->m_vPosition.x / 200 ), 
                                        (int)( pObj->m_vPosition.y / 200 ) );

  m_mapSleepingObjects[Coord].remove( pObj );
  pObj->m_pLayer = NULL;

}



CGameObject* CLevelLayer::GetGameObject( GR::u32 dwType )
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

  tMapSleepingObjects::iterator   itSO( m_mapSleepingObjects.begin() );
  while ( itSO != m_mapSleepingObjects.end() )
  {
    tListObjects&   listSleepingObjects = itSO->second;

    tListObjects::iterator    it( listSleepingObjects.begin() );
    while ( it != listSleepingObjects.end() )
    {
      CGameObject*    pObj = *it;

      if ( pObj->m_dwType == dwType )
      {
        return pObj;
      }

      ++it;
    }

    ++itSO;
  }

  return NULL;

}



CGameObject* CLevelLayer::GetGameObject( const GR::String& strType )
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

  //dh::Log( "Spawning object %x (%s)", pObj, pObj->m_strClassName.c_str() );

  CGameObject*    pObjAwake = NULL;

  if ( !( pObj->m_dwFlags & CGameObject::GOF_RESPAWN ) )
  {
    pObjAwake = pObj;
    pObjAwake->m_pSpawnBase = NULL;
  }
  else
  {
    pObjAwake = (CGameObject*)pObj->Clone();
    pObjAwake->m_pSpawnBase = pObj;
  }

  pObj->m_dwFlags |= CGameObject::GOF_SPAWNED;

  AddObject( pObjAwake );
  pObjAwake->ProcessEvent( tObjectEvent( CGameObject::ET_INIT ) );

}



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

        if ( !( pObj->m_dwFlags & CGameObject::GOF_RESPAWN ) )
        {
          // Einfach-Objekt wird aus der Sleeping-List entfernt
          it = ListObjects.erase( it );
          continue;
        }

        ++it;
      }
    }
  }

}



bool CLevelLayer::SleepObject( CGameObject* pObj )
{

  if ( pObj == NULL )
  {
    return false;
  }

  //dh::Log( "Sleeping object %x (%s)", pObj, pObj->m_strClassName.c_str() );
  pObj->ProcessEvent( tObjectEvent( CGameObject::ET_ENTER_SLEEP ) );
  if ( pObj->m_pSpawnBase == NULL )
  {
    if ( !( pObj->m_dwFlags & CGameObject::GOF_RESPAWN ) )
    {
      pObj->m_dwFlags &= ~CGameObject::GOF_SPAWNED;

      return true;
    }

    pObj->Die();
    return false;
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

  return false;

}



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

      tExtraData*   pED = pObj->ExtraData();
      if ( pED )
      {
        if ( pED->m_Type == tExtraData::EX_PATH )
        {
          CXJump::tMapObjectPaths::iterator   itPath( CXJump::Instance().m_mapObjectPaths.find( pED->m_strParam ) );
          if ( itPath != CXJump::Instance().m_mapObjectPaths.end() )
          {
            GR::Game::Path::MovementPath&    OPath = itPath->second;

            pObj->m_PathPos = GR::Game::Path::tPathPos( &OPath );
            pObj->m_dwFlags |= CGameObject::GOF_ON_PATH;
          }
        }
      }

      ++itList;
    }

    ++itMap;
  }

}



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

    if ( XJump.m_bFrozen )
    {
      ++itObj;
      continue;
    }


    // ist das Objekt außerhalb?
    if ( !( pObj->m_dwFlags & CGameObject::GOF_ON_PATH ) )
    {
      if ( ( pObj->m_vPosition.x < XJump.m_currentMap.m_vectScrollOffset.x - XJump.m_fFallAsleepRange )
      ||   ( pObj->m_vPosition.y < XJump.m_currentMap.m_vectScrollOffset.y - XJump.m_fFallAsleepRange )
      ||   ( pObj->m_vPosition.x > XJump.m_currentMap.m_vectScrollOffset.x + XJump.m_ptViewportExtents.x + XJump.m_fFallAsleepRange )
      ||   ( pObj->m_vPosition.y > XJump.m_currentMap.m_vectScrollOffset.y + XJump.m_ptViewportExtents.y + XJump.m_fFallAsleepRange ) )
      {
        if ( pObj != XJump.m_pControlledObject )
        {
          if ( SleepObject( pObj ) )
          {
            itObj = m_listAliveObjects.erase( itObj );

            AddSleepingObject( pObj );
            continue;
          }
          ++itObj;
          continue;
        }
      }
    }
    pObj->Update( fElapsedTime );

    ++itObj;
  }

  // Trigger
  CGameObject*    pControlledObject = CXJump::Instance().m_pControlledObject;
  if ( pControlledObject )
  {
    math::tRect<int>                    vecCR   = pControlledObject->CollisionRect();
  
    tListTrigger::iterator    itTrigger( m_listTrigger.begin() );
    while ( itTrigger != m_listTrigger.end() )
    {
      XJump::tTriggerArea&   Trigger = *itTrigger;

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

  // Regions
  if ( pControlledObject )
  {
    GR::tPoint    ptPlayerPos( (int)pControlledObject->m_vPosition.x, (int)pControlledObject->m_vPosition.y );

    if ( ( pControlledObject->m_pCurrentRegion )
    &&   ( pControlledObject->m_pCurrentRegion->contains( ptPlayerPos ) ) )
    {
      // noch in der aktuellen Region
    }
    else
    {
      // neue Region suchen
      GR::tRect*    pNewRegion = NULL;
    
      tListRegions::iterator    itRegion( m_listRegions.begin() );
      while ( itRegion != m_listRegions.end() )
      {
        GR::tRect   rcRegion( *itRegion );

        rcRegion.Right = ( rcRegion.Left + rcRegion.width() ) * m_vTileSize.x - 1;
        rcRegion.Bottom = ( rcRegion.Top + rcRegion.height() ) * m_vTileSize.y - 1;

        rcRegion.Left *= m_vTileSize.x;
        rcRegion.Top *= m_vTileSize.y;

        if ( rcRegion.contains( ptPlayerPos ) )
        {
          pNewRegion = &( *itRegion );
        }

        ++itRegion;
      }

      if ( pNewRegion != pControlledObject->m_pCurrentRegion )
      {
        if ( pControlledObject->m_pCurrentRegion )
        {
          pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_LEAVE_REGION, (GR::up)pControlledObject->m_pCurrentRegion ) );
          pControlledObject->m_pCurrentRegion = NULL;
          SetVisualRange();
        }
        if ( pNewRegion )
        {
          // der Spieler ist voll drin
          pControlledObject->m_pCurrentRegion = pNewRegion;

          pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_ENTER_REGION, (GR::up)pNewRegion ) );

          SetVisualRange( pNewRegion->Left, pNewRegion->Top, pNewRegion->width(), pNewRegion->height() );
        }
      }
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

    math::tRect<int>        vecCR = pObj->CollisionRect();
    
    tListObjects::iterator    itObj3( m_listAliveObjects.begin() );
    while ( itObj3 != m_listAliveObjects.end() )
    {
      CGameObject*    pObj2 = *itObj3;

      if ( ( pObj != pObj2 )
      &&   ( !( pObj2->m_dwFlags & CGameObject::GOF_NO_COLISSION ) )
      &&   ( !( pObj2->m_dwFlags & CGameObject::GOF_REMOVE_ME ) ) )
      {
        CXJump::tPoint    Obj2Pos = pObj2->TruePosition();

        math::tRect<int>        vecCR2 = pObj2->CollisionRect();
    
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

    if ( ( pObj != pCollider )
    &&   ( pObj->CollisionRect().intersects( pCollider->CollisionRect() ) ) )
    {
      if ( ( strType.empty() )
      ||   ( strType == pObj->Type() ) )
      {
        return pObj;
      }
    }

    ++it;
  }
  return NULL;

}



void CLevelLayer::EnableTrigger( GR::u32 dwTriggerID, bool bEnable )
{

  if ( dwTriggerID - 1 >= (GR::u32)m_listTrigger.size() )
  {
    return;
  }

  tListTrigger::iterator    itTrigger( m_listTrigger.begin() );
  std::advance( itTrigger, dwTriggerID - 1 );

  XJump::tTriggerArea&   Trigger( *itTrigger );

  if ( ( ( Trigger.m_Rect.Left < 0 )
  &&     ( bEnable ) )
  ||   ( ( Trigger.m_Rect.Left >= 0 )
  &&     ( !bEnable ) ) )
  {
    Trigger.m_Rect.Left    = -Trigger.m_Rect.Left;
    Trigger.m_Rect.Right   = -Trigger.m_Rect.Right;
    Trigger.m_Rect.Top     = -Trigger.m_Rect.Top;
    Trigger.m_Rect.Bottom  = -Trigger.m_Rect.Bottom;
  }

}



GR::u32 CLevelLayer::GetObjectCountByType( const GR::String& strType )
{

  GR::u32   dwCount = 0;

  CGameObject*    pKnownObj = CXJump::Instance().GetKnownObject( strType );
  if ( pKnownObj == NULL )
  {
    return 0;
  }

  tMapSleepingObjects::iterator   it( m_mapSleepingObjects.begin() );
  while ( it != m_mapSleepingObjects.end() )
  {
    const tListObjects& List( it->second );

    tListObjects::const_iterator    itO( List.begin() );
    while ( itO != List.end() )
    {
      CGameObject*    pObj( *itO );

      if ( pObj->m_dwType == pKnownObj->m_dwType )
      {
        ++dwCount;
      }

      ++itO;
    }

    ++it;
  }

  return dwCount;

}



void CLevelLayer::RemoveAllObjectsOfType( const GR::String& Type )
{

  CGameObject*    pKnownObj = CXJump::Instance().GetKnownObject( Type );
  if ( pKnownObj == NULL )
  {
    return;
  }

  tListObjects::iterator    itAO( m_listAliveObjects.begin() );
  while ( itAO != m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *itAO;

    if ( pObj->m_dwType == pKnownObj->m_dwType )
    {
      delete pObj;
      itAO = m_listAliveObjects.erase( itAO );
    }
    else
    {
      ++itAO;
    }
  }

  tMapSleepingObjects::iterator   it( m_mapSleepingObjects.begin() );
  while ( it != m_mapSleepingObjects.end() )
  {
    tListObjects& List( it->second );

    tListObjects::iterator    itO( List.begin() );
    while ( itO != List.end() )
    {
      CGameObject*    pObj( *itO );

      if ( pObj->m_dwType == pKnownObj->m_dwType )
      {
        delete pObj;
        itO = List.erase( itO );
      }
      else
      {
        ++itO;
      }
    }

    ++it;
  }

}