#include "XJump.h"

#include <Misc/Misc.h>
#include <Misc/CloneFactory.h>

#include <Grafik/Image.h>

#include <Xtreme/Input/XInput.h>

#include <String/Path.h>

#include <Xtreme/XRenderer.h>

#include <IO/FileStream.h>
#include <IO/FileUtil.h>

#include "LuaGameObject.h"

#include <debug/debugclient.h>



CXJump::CXJump() :
  m_vGravity( 0, 10.0f, 0 ),
  m_bFrozen( false ),
  m_bMovieMode( false ),
  m_fFallAsleepRange( 400.0f ),
  m_strOnLoadScript( "" ),
  m_pTextureBackground( NULL )
{

}



CXJump::~CXJump()
{

  Clear();

}



CXJump& CXJump::Instance()
{

  static CXJump     g_XJump;

  return g_XJump;

}



bool CXJump::InitInstance()
{

  m_LuaHandler.InitInstance();

  // TODO - geht das nicht besser?
  Xtreme::XInput* pInput = (Xtreme::XInput*)GR::Service::Environment::Instance().Service( "Input" );

  if ( pInput )
  {
    pInput->AddCommand( "left",        pInput->MapKeyToVKey( Xtreme::KEY_LEFT ) );
    pInput->AddCommand( "right",       pInput->MapKeyToVKey( Xtreme::KEY_RIGHT ) );
    pInput->AddCommand( "up",          pInput->MapKeyToVKey( Xtreme::KEY_UP ) );
    pInput->AddCommand( "down",        pInput->MapKeyToVKey( Xtreme::KEY_DOWN ) );
    pInput->AddCommand( "jumppushed",  pInput->MapKeyToVKey( Xtreme::KEY_SPACE ) );
    pInput->AddCommand( "jump",        pInput->MapKeyToVKey( Xtreme::KEY_SPACE ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );
  }

  return true;

}



void CXJump::Clear()
{

  m_CloneFactory.Clear();

  m_strOnLoadScript = "";

  tVectColissionTileSets::iterator    it( m_vectColissionTileSets.begin() );
  while ( it != m_vectColissionTileSets.end() )
  {
    delete *it;

    ++it;
  }
  m_vectColissionTileSets.clear();

  tVectKnownObjects::iterator    itKO( m_vectKnownObjects.begin() );
  while ( itKO != m_vectKnownObjects.end() )
  {
    tPairKnownObject&   Pair = *itKO;

    delete Pair.second;

    ++itKO;
  }
  m_vectKnownObjects.clear();

  m_AnimationManager.Clear();
  m_listMaps.clear();
  m_vectAnimPositions.clear();
  m_vectColissionTiles.clear();
  m_vectInfoColissionTiles.clear();
  m_vectInfoTiles.clear();
  m_vectInfoTileSets.clear();
  m_vectInfoColissionTileSets.clear();

  m_mapTileCategories.clear();
  m_pControlledObject = NULL;
}



void CXJump::ClipToCurrentRange( math::vector3& vectPos )
{

  CLevelLayer*   pLayer = m_currentMap.m_pMainLayer;
  if ( pLayer == NULL )
  {
    return;
  }
  if ( vectPos.x + m_ptViewportExtents.x > ( pLayer->m_rectVisualRange.position().x + pLayer->m_rectVisualRange.size().x ) * pLayer->m_vTileSize.x )
  {
    vectPos.x = (float)( pLayer->m_rectVisualRange.position().x + pLayer->m_rectVisualRange.size().x ) * pLayer->m_vTileSize.x - m_ptViewportExtents.x;
  }
  if ( vectPos.y + m_ptViewportExtents.y > ( pLayer->m_rectVisualRange.position().y + pLayer->m_rectVisualRange.size().y ) * pLayer->m_vTileSize.y )
  {
    vectPos.y = (float)( pLayer->m_rectVisualRange.position().y + pLayer->m_rectVisualRange.size().y ) * pLayer->m_vTileSize.y - m_ptViewportExtents.y;
  }

  if ( vectPos.x < pLayer->m_rectVisualRange.position().x * pLayer->m_vTileSize.x )
  {
    vectPos.x = (float)( pLayer->m_rectVisualRange.position().x * pLayer->m_vTileSize.x );
  }
  if ( vectPos.y < pLayer->m_rectVisualRange.position().y * pLayer->m_vTileSize.y )
  {
    vectPos.y = (float)( pLayer->m_rectVisualRange.position().y * pLayer->m_vTileSize.y );
  }


}



void CXJump::Update( const float fElapsedTime )
{

  ProcessQueue();

  m_LuaHandler.ResumeThreads( fElapsedTime );

  ProcessQueue();

  m_currentMap.Update( fElapsedTime );

  if ( m_bFrozen )
  {
    return;
  }

  tVectAnimPositions::iterator  itAP( m_vectAnimPositions.begin() );
  while ( itAP != m_vectAnimPositions.end() )
  {
    tAnimationPos&    AnimPos = *itAP;

    m_AnimationManager.AdvanceAnimFrame( AnimPos, fElapsedTime );

    ++itAP;
  }

  // auf kontrolliertes Objekt zentrieren
  SoftScrollViewWithObject( m_pControlledObject, fElapsedTime );

}



bool CXJump::BlockedAt( CGameObject* pMovingObject, int iX, int iY, XJump::eDirType dtDir, XJump::eActionType& eAction, bool bCanPush )
{

  if ( ( pMovingObject )
  &&   ( pMovingObject->m_dwFlags & CGameObject::GOF_NO_TILE_COLLISION ) )
  {
    return false;
  }

  int   iColType = m_currentMap.CollisionTypeAt( iX, iY );

  if ( ( pMovingObject )
  &&   ( !( pMovingObject->m_dwFlags & CGameObject::GOF_CAN_PUSH ) ) )
  {
    bCanPush = false;
  }

  eAction = XJump::AT_NONE;
  if ( iColType )
  {
    if ( iColType == 0x7c00 )
    {
      // nach unten fest (schwebende Plattformen)
      if ( dtDir != XJump::DIR_DOWN )
      {
        return false;
      }

      return !pMovingObject->IsClimbing();
    }
    else if ( iColType == 0x7c1f )
    {
      // kaputt!
      eAction = XJump::AT_KILLED;
      return true;
    }
    else if ( iColType == 0x03e0 )
    {
      // Säure
      eAction = XJump::AT_HURTS;
      return true;
    }
    else if ( iColType == 0x03ff )
    {
      // Custom Action
      eAction = XJump::AT_CUSTOM;
      return true;
    }
  }

  if ( iColType != 0 )
  {
    return true;
  }

  // blockierende Objekte prüfen
  if ( pMovingObject )
  {
    GR::tRect     rcMovingObject = pMovingObject->CollisionRect();

    rcMovingObject.offset( -(int)pMovingObject->m_vPosition.x, -(int)pMovingObject->m_vPosition.y );
    rcMovingObject.offset( iX, iY );

    CLevelMap::tListLayer::iterator    itLayer( m_currentMap.m_listLayers.begin() );
    while ( itLayer != m_currentMap.m_listLayers.end() )
    {
      CLevelLayer&    Layer = *itLayer;

      CLevelLayer::tListObjects::iterator   itObj( Layer.m_listAliveObjects.begin() );
      while ( itObj != Layer.m_listAliveObjects.end() )
      {
        CGameObject*    pOtherObj = *itObj;

        if ( ( !( pOtherObj->m_dwFlags & CGameObject::GOF_BLOCKING ) )
        ||   ( pOtherObj == pMovingObject ) )
        {
          ++itObj;
          continue;
        }

        if ( rcMovingObject.intersects( pOtherObj->CollisionRect() ) )
        {
          bool    bCanBeMoved = false;

          if ( ( pOtherObj->m_dwFlags & CGameObject::GOF_PUSHABLE )
          &&   ( bCanPush )
          &&   ( pOtherObj->IsOnGround() ) )
          {
            // kann das andere Objekt geschoben werden?
            switch ( dtDir )
            {
              case XJump::DIR_LEFT:
                bCanBeMoved = pOtherObj->Move( math::vector3( -1, 0, 0 ), true );
                break;
              case XJump::DIR_RIGHT:
                bCanBeMoved = pOtherObj->Move( math::vector3( 1, 0, 0 ), true );
                break;
            }
            if ( bCanBeMoved )
            {
              // jetzt ist Platz
              break;
            }
          }

          pMovingObject->ProcessEvent( tObjectEvent( CGameObject::ET_COLLIDE, (GR::up)pOtherObj ) );
          pOtherObj->ProcessEvent( tObjectEvent( CGameObject::ET_COLLIDE, (GR::up)pMovingObject ) );
          return true;
        }

        ++itObj;
      }

      ++itLayer;
    }
  }

  return false;

}



bool CXJump::IsAreaFree( int iX, int iY, int iWidth, int iHeight )
{

  XJump::eActionType   atType = XJump::AT_NONE;

  for ( int i = 0; i < iWidth; ++i )
  {
    for ( int j = 0; j < iHeight; ++j )
    {
      if ( BlockedAt( NULL, iX + i, iY + j, XJump::DIR_NONE, atType, false ) )
      {
        return false;
      }
    }
  }
  return true;

}



bool CXJump::StepPossible( CGameObject* pMovingObject, int iX, int iY, int iStepHeight, int& iResultStepHeight, XJump::eActionType& atType, XJump::eDirType dtDir, bool bCanPush )
{

  int     iGravity = 1;

  if ( pMovingObject->m_dwFlags & CGameObject::GOF_REVERSE_GRAVITY )
  {
    iGravity = -1;
  }

  if ( BlockedAt( pMovingObject, iX, iY - iGravity * iStepHeight, dtDir, atType, bCanPush ) )
  {
    return false;
  }
  GR::tRect   rcCollision = pMovingObject->CollisionRect();

  int   iStartStepAt = iGravity * iStepHeight;
  iResultStepHeight = 0;
  while ( iGravity * iStepHeight >= - iGravity * iStartStepAt )
  {
    if ( BlockedAt( pMovingObject, iX, iY - iGravity * iStepHeight, XJump::DIR_NONE, atType, bCanPush ) )
    {
      iResultStepHeight = iStepHeight + iGravity;
      break;
    }
    iStepHeight -= iGravity;
  }

  return IsAreaFree( rcCollision.Left, rcCollision.Top - iGravity * iStepHeight, rcCollision.width(), iGravity * iStepHeight );

}



math::vector3 CXJump::Gravity()
{

  return m_vGravity;

}



void CXJump::Gravity( math::vector3& vGrav )
{

  m_vGravity = vGrav;

}



bool CXJump::Load( const char* szFileName )
{

  Clear();
  
  GR::IO::FileStream   File;


  if ( !File.Open( szFileName ) )
  {
    return false;
  }

  Load( File );

  File.Close();

  m_strDataPath = Path::RemoveFileSpec( szFileName );

  return true;

}



void CXJump::Load( IIOStream& Stream )
{

  // Version
  GR::u32   dwVersion = Stream.ReadU32();

  // TileSets
  m_vectInfoTileSets.clear();
  int   iTileSetCount = Stream.ReadU32();
  m_vectInfoTileSets.reserve( iTileSetCount );
  for ( int iTS = 0; iTS < iTileSetCount; ++iTS )
  {
    GR::String    strTileSet;

    Stream.ReadString( strTileSet );
    m_vectInfoTileSets.push_back( strTileSet );
  }
  
  // Colission-TileSets
  m_vectInfoColissionTileSets.clear();
  int   iCTileSetCount = Stream.ReadU32();
  m_vectInfoColissionTileSets.reserve( iTileSetCount );
  for ( int iCTS = 0; iCTS < iCTileSetCount; ++iCTS )
  {
    GR::String    strTileSet;

    Stream.ReadString( strTileSet );
    m_vectInfoColissionTileSets.push_back( strTileSet );
  }
  
  // Tiles
  m_vectInfoTiles.clear();
  int   iTileCount = Stream.ReadU32();
  m_vectInfoTiles.reserve( iTileCount );
  for ( int iTile = 0; iTile < iTileCount; ++iTile )
  {
    XJump::tTileInfo   TileInfo;

    Stream.ReadBlock( &TileInfo, sizeof( TileInfo ) );
    m_vectInfoTiles.push_back( TileInfo );
  }

  // Tiles
  m_vectInfoColissionTiles.clear();
  int   iClsTileCount = Stream.ReadU32();
  m_vectInfoColissionTiles.reserve( iClsTileCount );
  for ( int iClsTile = 0; iClsTile < iClsTileCount; ++iClsTile )
  {
    XJump::tTileInfo   TileInfo;

    Stream.ReadBlock( &TileInfo, sizeof( TileInfo ) );
    m_vectInfoColissionTiles.push_back( TileInfo );
  }

  // Animationen
  m_AnimationManager.Load( Stream );

  m_vectAnimPositions.resize( m_AnimationManager.Animations.size() );
  for ( size_t i = 0; i < m_AnimationManager.Animations.size(); ++i )
  {
    m_vectAnimPositions[i].AnimationId = (GR::u32)i;
  }

  // Dateinamen der Levels
  m_listMaps.clear();
  int   iMapCount = Stream.ReadU32();
  for ( int iMap = 0; iMap < iMapCount; ++iMap )
  {
    GR::String   strMapName;

    Stream.ReadString( strMapName );
    m_listMaps.push_back( strMapName );
  }

  // Known Objects
  int   iKnownObjectCount = Stream.ReadU32();

  m_vectKnownObjects.resize( iKnownObjectCount );
  for ( int iKO = 0; iKO < iKnownObjectCount; ++iKO )
  {
    Stream.ReadString( m_vectKnownObjects[iKO].first );

    // das sollte eigentlich direkt in die Factory?
    m_vectKnownObjects[iKO].second = new CGameObject();
    m_vectKnownObjects[iKO].second->Load( Stream );
    m_vectKnownObjects[iKO].second->m_dwType = iKO;
    m_vectKnownObjects[iKO].second->m_ClassName = m_vectKnownObjects[iKO].first;

    m_mapKnownObjects[m_vectKnownObjects[iKO].first] = m_vectKnownObjects[iKO].second;
  }

  // Preferred Colission TypesKnown Objects
  int   iPreferredColissionTypeCount = Stream.ReadU32();

  for ( int iPCT = 0; iPCT < iPreferredColissionTypeCount; ++iPCT )
  {
    GR::u32   dwTile = Stream.ReadU32();

    m_mapPreferredColissionTile[dwTile] = Stream.ReadU32();
  }

  // Object-Pfade
  int   iObjectPathCount = Stream.ReadU32();

  m_mapObjectPaths.clear();
  for ( int iOPC = 0; iOPC < iObjectPathCount; ++iOPC )
  {
    GR::String   strKey;
    
    Stream.ReadString( strKey );

    m_mapObjectPaths[strKey].Load( Stream );
  }

  // On-Load-Script
  Stream.ReadString( m_strOnLoadScript );

  // Tile-Kategorien
  int   iTileCategories = Stream.ReadU32();
  for ( int iTC = 0; iTC < iTileCategories; ++iTC )
  {
    XJump::eTileCategory    eTC = (XJump::eTileCategory)Stream.ReadU32();

    int   iTiles = Stream.ReadU32();
    for ( int j = 0; j < iTiles; ++j )
    {
      m_mapTileCategories[eTC].insert( Stream.ReadU16() );
    }
  }

  // Animationen nach String
  size_t    iAnims = Stream.ReadU32();
  m_mapAnimations.clear();

  for ( size_t i = 0; i < iAnims; ++i )
  {
    GR::String   strName;
    Stream.ReadString( strName );

    m_mapAnimations[strName] = Stream.ReadU32();
  }

  // On-GameStart-Script
  Stream.ReadString( m_strOnGameStartScript );


  // Extra-Data
  GR::u32   dwExtraDataLength = Stream.ReadU32();

}



bool CXJump::Save( const char* szFileName )
{

  GR::IO::FileStream   File;


  if ( !File.Open( szFileName, IIOStream::OT_WRITE_ONLY ) )
  {
    return false;
  }

  Save( File );

  File.Close();

  return true;

}



void CXJump::Save( IIOStream& Stream )
{

  // Version
  Stream.WriteU32( 0x0001 );

  // TileSets
  Stream.WriteU32( (GR::u32)m_vectInfoTileSets.size() );
  tVectInfoTileSets::iterator   itTS( m_vectInfoTileSets.begin() );
  while ( itTS != m_vectInfoTileSets.end() )
  {
    GR::String&    strTileSet = *itTS;

    Stream.WriteString( strTileSet );

    ++itTS;
  }
  
  // Colission-TileSets
  Stream.WriteU32( (GR::u32)m_vectInfoColissionTileSets.size() );
  tVectInfoTileSets::iterator   itTS2( m_vectInfoColissionTileSets.begin() );
  while ( itTS2 != m_vectInfoColissionTileSets.end() )
  {
    GR::String&    strTileSet = *itTS2;

    Stream.WriteString( strTileSet );

    ++itTS2;
  }
  
  // Tiles
  Stream.WriteU32( (GR::u32)m_vectInfoTiles.size() );
  tVectInfoTiles::iterator   itTile( m_vectInfoTiles.begin() );
  while ( itTile != m_vectInfoTiles.end() )
  {
    XJump::tTileInfo&    TileInfo = *itTile;

    Stream.WriteBlock( &TileInfo, sizeof( TileInfo ) );

    ++itTile;
  }

  // Colission-Tiles
  Stream.WriteU32( (GR::u32)m_vectInfoColissionTiles.size() );
  tVectInfoTiles::iterator   itClsTile( m_vectInfoColissionTiles.begin() );
  while ( itClsTile != m_vectInfoColissionTiles.end() )
  {
    XJump::tTileInfo&    TileInfo = *itClsTile;

    Stream.WriteBlock( &TileInfo, sizeof( TileInfo ) );

    ++itClsTile;
  }

  // Animationen
  m_AnimationManager.Save( Stream );

  // Dateinamen der Levels
  Stream.WriteU32( (GR::u32)m_listMaps.size() );
  tListMaps::iterator   itMap( m_listMaps.begin() );
  while ( itMap != m_listMaps.end() )
  {
    GR::String&    strMap = *itMap;

    Stream.WriteString( strMap );

    ++itMap;
  }

  // Known Objects
  Stream.WriteU32( (GR::u32)m_vectKnownObjects.size() );
  tVectKnownObjects::iterator   itKO( m_vectKnownObjects.begin() );
  while ( itKO != m_vectKnownObjects.end() )
  {
    GR::String&    strName = itKO->first;

    Stream.WriteString( strName );

    CGameObject*    pObj = itKO->second;

    pObj->Save( Stream );

    ++itKO;
  }

  // Tile-Colission-Pairs
  Stream.WriteU32( (GR::u32)m_mapPreferredColissionTile.size() );
  tMapPreferredColissionTile::iterator   itPCT( m_mapPreferredColissionTile.begin() );
  while ( itPCT != m_mapPreferredColissionTile.end() )
  {
    Stream.WriteU32( itPCT->first );
    Stream.WriteU32( itPCT->second );

    ++itPCT;
  }

  // Object-Pfade
  Stream.WriteU32( (GR::u32)m_mapObjectPaths.size() );
  tMapObjectPaths::iterator   itOP( m_mapObjectPaths.begin() );
  while ( itOP != m_mapObjectPaths.end() )
  {
    Stream.WriteString( itOP->first );

    GR::Game::Path::MovementPath&    OPath = itOP->second;

    OPath.Save( Stream );

    ++itOP;
  }

  // On-Load-Script
  Stream.WriteString( m_strOnLoadScript );

  // Tile-Kategorien
  Stream.WriteU32( (GR::u32)m_mapTileCategories.size() );

  tMapTileCategories::iterator    itMTC( m_mapTileCategories.begin() );
  while ( itMTC != m_mapTileCategories.end() )
  {
    Stream.WriteU32( itMTC->first );
    
    tSetTileCategory&    listTC = itMTC->second;

    Stream.WriteU32( (GR::u32)listTC.size() );

    tSetTileCategory::iterator   it( listTC.begin() );
    while ( it != listTC.end() )
    {
      Stream.WriteU16( *it );

      ++it;
    }
    ++itMTC;
  }

  // Animationen nach String
  Stream.WriteU32( (GR::u32)m_mapAnimations.size() );
  tMapAnimations::iterator    itAn2( m_mapAnimations.begin() );
  while ( itAn2 != m_mapAnimations.end() )
  {
    Stream.WriteString( itAn2->first );
    Stream.WriteU32( itAn2->second );

    ++itAn2;
  }

  // On-GameStart-Script
  Stream.WriteString( m_strOnGameStartScript );

  // Extra-Data
  Stream.WriteU32( 0 );

}



void CXJump::SaveGameState( IIOStream& Stream )
{

  m_GameVars.SaveVars( Stream );

  Stream.WriteString( m_strCurrentMapName );

  Stream.WriteF32( m_pControlledObject->m_vPosition.x );
  Stream.WriteF32( m_pControlledObject->m_vPosition.y );
  Stream.WriteF32( m_pControlledObject->m_vPosition.z );

}



void CXJump::LoadGameState( IIOStream& Stream )
{

  StartLoadedGame();

  m_GameVars.LoadVars( Stream );

  Stream.ReadString( m_strCurrentMapName );

  float     fX = Stream.ReadF32();
  float     fY = Stream.ReadF32();
  Stream.ReadF32();   // z


  ProcessEvent( tGameEvent( tGameEvent::GE_CHANGE_MAP, m_strCurrentMapName, 1, (int)fX, (int)fY ) );

}



GR::Graphic::ContextDescriptor* CXJump::GetColissionTileCD( GR::u32 dwTile )
{

  if ( dwTile & 0x8000 )
  {
    // BAUSTELLE
    return NULL;
  }
  else
  {
    if ( m_vectColissionTiles.size() > dwTile )
    {
      return &m_vectColissionTiles[dwTile];
    }
  }
  return NULL;

}



int CXJump::AddKnownObject( const GR::String& strName, CGameObject* pObj )
{

  tPairKnownObject    Pair;

  Pair.first = strName;
  Pair.second = pObj;

  m_vectKnownObjects.push_back( Pair );

  return (int)m_vectKnownObjects.size() - 1;

}



void CXJump::RemoveKnownObject( const GR::String& Name )
{

  m_currentMap.RemoveAllObjectsOfType( Name );

  size_t    KnownObjectIndex = 0;

  tVectKnownObjects::iterator   it( m_vectKnownObjects.begin() );
  while ( it != m_vectKnownObjects.end() )
  {
    tPairKnownObject& Pair( *it );

    if ( Pair.first == Name )
    {
      delete Pair.second;
      break;
    }
    ++KnownObjectIndex;
    ++it;
  }

  it = m_vectKnownObjects.begin();
  while ( it != m_vectKnownObjects.end() )
  {
    tPairKnownObject& Pair( *it );

    if ( Pair.second->m_dwType >= KnownObjectIndex )
    {
      Pair.second->m_dwType--;
    }
    ++it;
  }

  // now change all object's knownobjectindex if above the deleted one
  CLevelMap::tListLayer::iterator    itL( m_currentMap.m_listLayers.begin() );
  while ( itL != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer& Layer( *itL );

    CLevelLayer::tListObjects::iterator   itO( Layer.m_listAliveObjects.begin() );
    while ( itO != Layer.m_listAliveObjects.end() )
    {
      CGameObject* pObj( *itO );

      if ( pObj->m_dwType >= KnownObjectIndex )
      {
        pObj->m_dwType--;
      }

      ++itO;
    }

    CLevelLayer::tMapSleepingObjects::iterator   itSO( Layer.m_mapSleepingObjects.begin() );
    while ( itSO != Layer.m_mapSleepingObjects.end() )
    {
      CLevelLayer::tListObjects&   ListO( itSO->second );

      CLevelLayer::tListObjects::iterator   itO( ListO.begin() );
      while ( itO != ListO.end() )
      {
        CGameObject* pObj( *itO );

        if ( pObj->m_dwType >= KnownObjectIndex )
        {
          pObj->m_dwType--;
        }

        ++itO;
      }
      ++itSO;
    }

    ++itL;
  }

}



GR::String CXJump::GetObjectType( CGameObject* pObj )
{

  if ( pObj == NULL )
  {
    return GR::String();
  }

  if ( pObj->m_dwType >= (GR::u32)m_vectKnownObjects.size() )
  {
    return GR::String();
  }

  return m_vectKnownObjects[pObj->m_dwType].first;

}



CGameObject* CXJump::GetKnownObject( const GR::String& strName )
{

  tVectKnownObjects::iterator   it( m_vectKnownObjects.begin() );
  while ( it != m_vectKnownObjects.end() )
  {
    tPairKnownObject&   Pair = *it;

    if ( Pair.first == strName )
    {
      return Pair.second;
    }

    ++it;
  }

  return NULL;

}



CGameObject* CXJump::RenameKnownObject( const GR::String& strOldName, const GR::String& strNewName )
{

  tVectKnownObjects::iterator   it( m_vectKnownObjects.begin() );
  while ( it != m_vectKnownObjects.end() )
  {
    tPairKnownObject&   Pair = *it;

    if ( Pair.first == strOldName )
    {
      Pair.first = strNewName;
      return Pair.second;
    }

    ++it;
  }

  return NULL;

}



CGameObject* CXJump::CreateObject( const GR::String& strName )
{

  tMapKnownObjects::iterator    it( m_mapKnownObjects.find( strName ) );
  if ( it == m_mapKnownObjects.end() )
  {
    return NULL;
  }
  CGameObject* pObj = (CGameObject*)it->second->Clone();
  if ( pObj == NULL )
  {
    return NULL;
  }
  pObj->m_ClassName = strName;
  return pObj;

}



void CXJump::RegisterObject( const GR::String& strName, CGameObject* pObj, bool bDontKeepValues )
{

  tMapKnownObjects::iterator    it( m_mapKnownObjects.find( strName ) );
  if ( it == m_mapKnownObjects.end() )
  {
    m_mapKnownObjects[strName] = pObj;
    return;
  }

  if ( bDontKeepValues )
  {
    m_mapKnownObjects[strName] = pObj;
    return;
  }

  CGameObject*    pGameObject = it->second;

  // wichtige Werte vom alten Objekt übernehmen (irgendwie ist das nicht sauber)
  pObj->m_dwFlags         = pGameObject->m_dwFlags;
  pObj->m_animPos         = pGameObject->m_animPos;
  pObj->m_dwAnimID        = pGameObject->m_dwAnimID;
  pObj->m_dwType          = pGameObject->m_dwType;
  pObj->m_vCollisionRect  = pGameObject->m_vCollisionRect;
  pObj->SetAnimation( pObj->m_dwAnimID );

  m_mapKnownObjects[strName] = pObj;

}


bool CXJump::IsObjectType( CGameObject* pObject, const GR::String& strName )
{
  if ( pObject == NULL )
  {
    return false;
  }
  return ( pObject->m_ClassName == strName );
}



void CXJump::PrepareForStart()
{

  m_currentMap.PrepareForStart();

}



XTextureSection CXJump::GetTileSection( GR::u32 dwTile )
{

  if ( dwTile >= m_vectTiles.size() )
  {
    return XTextureSection();
  }
  return m_vectTiles[dwTile];

}



XJump::tTileInfo* CXJump::GetTileInfo( GR::u32 dwTile )
{

  if ( dwTile & 0x8000 )
  {
    if ( m_vectAnimPositions.size() <= ( dwTile & 0x7fff ) )
    {
      return NULL;
    }
    dwTile = m_AnimationManager.GetAnimFrame( m_vectAnimPositions[dwTile & 0x7fff] );
  }
  if ( m_vectInfoTiles.size() > dwTile )
  {
    return &m_vectInfoTiles[dwTile];
  }
  return NULL;

}



void CXJump::Freeze( bool bFreeze )
{

  m_bFrozen = bFreeze;

}



void CXJump::MovieMode( bool bMovie )
{

  m_bMovieMode = bMovie;

}



void CXJump::SetViewport( int iWidth, int iHeight )
{
 
  m_ptViewportExtents.x   = iWidth;
  m_ptViewportExtents.y   = iHeight;

}



bool CXJump::ProcessEvent( const tGameEvent& Event )
{

  switch ( Event.m_Type )
  {
    case tGameEvent::GE_WARP:
      if ( m_pControlledObject )
      {
        m_pControlledObject->m_vPosition.x = (float)Event.m_iEventParam[0];
        m_pControlledObject->m_vPosition.y = (float)Event.m_iEventParam[1];
        m_pControlledObject->m_pCurrentRegion = NULL;

        m_currentMap.CenterOnObject( m_pControlledObject );
      }
      break;
    case tGameEvent::GE_CHANGE_MAP:
      if ( !m_currentMap.Load( Path::Normalize( Path::Append( m_strDataPath, Event.m_strEventParam ), false ).c_str() ) )
      {
        dh::Log( "Failed to Load Map (%s)\n", Path::Normalize( Path::Append( m_strDataPath, Event.m_strEventParam ), false ).c_str() );
        return true;
      }

      m_strCurrentMapName = Event.m_strEventParam;

      PrepareForStart();

      //GR::String     MapStartScript = GR::IO::FileUtil::ReadFileAsString( Path::Normalize( Path::Append( m_strDataPath, Path::RenameExtension( Event.m_strEventParam, "lua" ) ) ).c_str() );

      GR::String     MapStartScript = m_currentMap.m_Script;
      //m_LuaHandler.DoFile( Path::Normalize( Path::Append( m_strDataPath, Path::RenameExtension( Event.m_strEventParam, "lua" ) ) ).c_str() );

      if ( m_currentMap.m_pMainLayer == NULL )
      {
        dh::Log( "No Main Layer found\n" );
      }
      else
      {
        m_pControlledObject = m_currentMap.m_pMainLayer->GetGameObject( 0 );
        if ( Event.m_iEventParam[0] != 0 )
        {
          // Level-Wechsel, Spieler auf gewünschte Position setzen
          m_pControlledObject->m_vPosition.x = (float)Event.m_iEventParam[1];
          m_pControlledObject->m_vPosition.y = (float)Event.m_iEventParam[2];

          m_pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_MAP_CHANGED ) );
        }
      }
      m_Camera.SetProjectionType( XCamera::OrthoLH );
      m_Camera.ViewWidth( m_ptViewportExtents.x );
      m_Camera.ViewHeight( -(int)m_ptViewportExtents.y );
      m_Camera.ZNear( 0.0f );
      m_Camera.ZFar( 100.0f );
      m_Camera.SetValues( GR::tVector( (float)m_ptViewportExtents.x * 0.5f, (float)m_ptViewportExtents.y * 0.5f, -20.0f ),
                          GR::tVector( (float)m_ptViewportExtents.x * 0.5f, (float)m_ptViewportExtents.y * 0.5f, 0.0f ),
                          GR::tVector( 0.0f, 1.0f, 0.0f ) );

      // auf kontrolliertes Objekt zentrieren
      m_currentMap.CenterOnObject( m_pControlledObject );

      StartScriptAsMovie( MapStartScript );

      SendEvent( Event );
      return true;
  }

  SendEvent( Event );
  return false;

}



bool CXJump::OnTopOfPlatform( CGameObject* pObject )
{

  CXJump::tPoint    ObjPos = pObject->TruePosition();

  math::tRect<int>   vecCR = pObject->m_vCollisionRect;
  
  vecCR.offset( ObjPos );

  vecCR = pObject->CollisionRect();


  CLevelLayer::tListObjects::iterator    it( m_currentMap.m_pMainLayer->m_listAliveObjects.begin() );
  while ( it != m_currentMap.m_pMainLayer->m_listAliveObjects.end() )
  {
    CGameObject*    pObj = *it;

    if ( ( !( pObj->m_dwFlags & CGameObject::GOF_PLATFORM ) )
    ||   ( pObject == pObj ) )
    {
      ++it;
      continue;
    }

    //CXJump::tPoint    ObjPos = pObj->TruePosition();

    //math::tRect<int>   vecCR2 = pObj->m_vCollisionRect;
    math::tRect<int>   vecCR2 = pObj->CollisionRect();
    
    //vecCR2.offset( ObjPos );

    if ( ( vecCR.Bottom == vecCR2.Top )
    &&   ( vecCR.Left <= vecCR2.Right )
    &&   ( vecCR.Right >= vecCR2.Left ) )
    {
      //dh::Log( "On Platform" );
      pObject->m_pPlatform = pObj;
      if ( !( pObject->m_dwFlags & CGameObject::GOF_ON_PLATFORM ) )
      {
        pObject->m_dwFlags |= CGameObject::GOF_ON_PLATFORM;

        // fest auf die Höhe setzen
        //pObject->m_vPosition.y = (GR::f32)( vecCR2.m_iTop - vecCR.height() );
        pObject->m_vPosition.y = (GR::f32)( vecCR2.Top - 1 );
        //pObj->m_vPosition.y = (GR::f32)( vecCR.m_iTop - 1 );
        pObj->ProcessEvent( tObjectEvent( CGameObject::ET_CARRY_OBJECT, (GR::up)pObject ) );
      }
      else
      {
        pObject->m_vPosition.y = (GR::f32)( vecCR2.Top - 1 );
      }
      return true;
    }
    /*
    else if ( pObject->m_pPlatform == pObj )
    {
      dh::Log( "Fell off" );

      dh::Log( "vecCR.m_iBottom == vecCR2.m_iTop - 1 (%d==%d)", vecCR.m_iBottom, vecCR2.m_iTop - 1 );
      dh::Log( "vecCR.m_iLeft <= vecCR2.m_iRight (%d <= %d)", vecCR.m_iLeft, vecCR2.m_iRight );
      dh::Log( "vecCR.m_iRight >= vecCR2.m_iLeft (%d >= %d)", vecCR.m_iRight, vecCR2.m_iLeft);
    }
    */

    ++it;
  }

  if ( pObject->m_pPlatform )
  {
    CXJump::tPoint    ObjPos2 = pObject->m_pPlatform->TruePosition();

    math::tRect<int>   vecCR2 = pObject->m_pPlatform->m_vCollisionRect;
    
    vecCR2.offset( ObjPos2 );

    //dh::Log( "Off Platform %d (my y %d)", vecCR2.m_iTop - 1, vecCR.m_iBottom );//ObjPos.y );

    pObject->m_pPlatform->ProcessEvent( tObjectEvent( CGameObject::ET_DROP_OBJECT, (GR::up)pObject ) );
    pObject->m_pPlatform = NULL;
    pObject->m_dwFlags &= ~CGameObject::GOF_ON_PLATFORM;
  }

  return false;

}



void CXJump::DisplayTile( XRenderer& Renderer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags )
{

  if ( iTile & 0x8000 )
  {
    if ( m_vectAnimPositions.size() <= ( iTile & 0x7fff ) )
    {
      return;
    }
    iTile = m_AnimationManager.GetAnimFrame( m_vectAnimPositions[iTile & 0x7fff] );
  }
  if ( ( iTile < 0 )
  ||   ( iTile >= m_vectTiles.size() ) )
  {
    return;
  }

  XTextureSection&    Section = m_vectTiles[iTile];

  if ( ( Section.m_pTexture )
  &&   ( GR::Graphic::ImageData::FormatHasAlpha( Section.m_pTexture->m_ImageFormat ) ) )
  {
    m_listAlphaTiles.push_back( tTempTileDrawInfo( &Section, fX, fY, dwAlternativeFlags ) );
  }
  else
  {
    Renderer.RenderTextureSection2d( fX, fY, Section, -1, -1, -1, dwAlternativeFlags );
  }

}



void CXJump::DisplayTileDirect( XRenderer& Renderer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags )
{

  if ( iTile & 0x8000 )
  {
    if ( m_vectAnimPositions.size() <= ( iTile & 0x7fff ) )
    {
      return;
    }
    iTile = m_AnimationManager.GetAnimFrame( m_vectAnimPositions[iTile & 0x7fff] );
  }
  if ( ( iTile < 0 )
  ||   ( iTile >= m_vectTiles.size() ) )
  {
    return;
  }

  XTextureSection&    Section = m_vectTiles[iTile];

  Renderer.RenderTextureSection2d( fX, fY, Section, -1, -1, -1, dwAlternativeFlags );

}



void CXJump::DisplayObject( XRenderer& Renderer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, GR::u32 dwColor, int iWidth, int iHeight )
{

  if ( iTile >= m_vectTiles.size() )
  {
    return;
  }

  XTextureSection&    Section = m_vectTiles[iTile];

  /*
  // TODO - in Liste packen!
  if ( CImageData::FormatHasAlpha( Section.m_pTexture->m_ImageFormat ) )
  {

  }
  */

  /*
  // Kreuzchen - temporär
  Renderer.SetTexture( 0, 0 );
  Renderer.RenderQuad2d( fX - 4, fY, 9, 1, 0xffff00ff );
  Renderer.RenderQuad2d( fX, fY - 4, 1, 9, 0xffff00ff );
  */

  GR::u32     dwFlags = Section.m_Flags;

  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HCENTER )
  {
    dwFlags |= XTextureSection::TSF_HCENTER;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VCENTER )
  {
    dwFlags |= XTextureSection::TSF_VCENTER;
  }
  if ( dwDisplayFlags & CGameObject::GOF_HMIRROR )
  {
    dwFlags ^= XTextureSection::TSF_H_MIRROR;
  }
  if ( dwDisplayFlags & CGameObject::GOF_VMIRROR )
  {
    dwFlags ^= XTextureSection::TSF_V_MIRROR;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VBOTTOM )
  {
    dwFlags |= XTextureSection::TSF_ALIGN_BOTTOM;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HRIGHT )
  {
    dwFlags |= XTextureSection::TSF_ALIGN_RIGHT;
  }

  Renderer.RenderTextureSection2d( fX, fY, Section, dwColor, iWidth, iHeight, dwFlags );

}



void CXJump::DisplayObjectMasked( XRenderer& Renderer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags, GR::u32 dwColor, int iWidth, int iHeight )
{

  if ( iTile >= m_vectTiles.size() )
  {
    return;
  }

  XTextureSection&    Section = m_vectTiles[iTile];

  /*
  // Kreuzchen - temporär
  Renderer.SetTexture( 0, 0 );
  Renderer.RenderQuad2d( fX - 4, fY, 9, 1, 0xffff00ff );
  Renderer.RenderQuad2d( fX, fY - 4, 1, 9, 0xffff00ff );
  */

  GR::u32     dwFlags = Section.m_Flags;

  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HCENTER )
  {
    dwFlags |= XTextureSection::TSF_HCENTER;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VCENTER )
  {
    dwFlags |= XTextureSection::TSF_VCENTER;
  }
  if ( dwDisplayFlags & CGameObject::GOF_HMIRROR )
  {
    dwFlags |= XTextureSection::TSF_H_MIRROR;
  }
  if ( dwDisplayFlags & CGameObject::GOF_VMIRROR )
  {
    dwFlags |= XTextureSection::TSF_V_MIRROR;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VBOTTOM )
  {
    dwFlags |= XTextureSection::TSF_ALIGN_BOTTOM;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HRIGHT )
  {
    dwFlags |= XTextureSection::TSF_ALIGN_RIGHT;
  }

  Renderer.SetState( XRenderer::RS_COLOR_OP, XRenderer::RSV_SELECT_ARG_1 );
  Renderer.SetState( XRenderer::RS_COLOR_ARG_1, XRenderer::RSV_DIFFUSE );

  Renderer.RenderTextureSection2d( fX, fY, Section, dwColor, iWidth, iHeight, dwFlags );

  Renderer.SetState( XRenderer::RS_COLOR_OP, XRenderer::RSV_MODULATE );

}



void CXJump::DisplayField( XRenderer& Renderer )
{

  math::matrix4     matWorld;

  matWorld.Identity();

  Renderer.SetTransform( XRenderer::TT_WORLD, matWorld );

  Renderer.SetTransform( XRenderer::TT_VIEW,       m_Camera.GetViewMatrix() );
  Renderer.SetTransform( XRenderer::TT_PROJECTION, m_Camera.GetProjectionMatrix() );

  Renderer.SetShader( XRenderer::ST_FLAT );

  Renderer.SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );

  if ( m_pTextureBackground )
  {
    Renderer.SetTexture( 0, m_pTextureBackground );
    Renderer.RenderQuad2d( 0, 0, Renderer.Width(), Renderer.Height(), 
                           0.0f, 0.0f, 1.0f, 0.0f,
                           0.0f, 1.0f, 1.0f, 1.0f,
                           0xffffffff );
  }

  GR::tVector vectPos = m_currentMap.m_vectScrollOffset;//m_Camera.GetPosition();

  vectPos.x = floorf( vectPos.x );
  vectPos.y = floorf( vectPos.y );

  CLevelMap::tListLayer::iterator    it( m_currentMap.m_listLayers.begin() );
  while ( it != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer&    Layer = *it;

    if ( Layer.m_Type == CLevelLayer::LT_COLLISION )
    {
      ++it;
      continue;
    }

    Renderer.SetShader( XRenderer::ST_ALPHA_TEST );

    m_listAlphaTiles.clear();

    int   iX1 = int( vectPos.x / Layer.m_vTileSize.x - 1 ),
          iY1 = int( vectPos.y / Layer.m_vTileSize.y - 1 );
    int   iX2 = iX1 + Renderer.Width() / Layer.m_vTileSize.x + 2;
    int   iY2 = iY1 + Renderer.Height() / Layer.m_vTileSize.y + 2;

    // +4, damit die Objekte ein bißchen außerhalb auch mitkommen
    Layer.SpawnObjects( (float)( ( iX1 - 4 ) * Layer.m_vTileSize.x ), 
                        (float)( ( iY1 - 4 ) * Layer.m_vTileSize.y ),
                        (float)( ( iX2 - iX1 + 1 + 4 ) * Layer.m_vTileSize.x ),
                        (float)( ( iY2 - iY1 + 1 + 4 ) * Layer.m_vTileSize.y ) );

    // behind-Tiles
    CLevelLayer::tListObjects::iterator    itObj( Layer.m_listAliveObjects.begin() );
    while ( itObj != Layer.m_listAliveObjects.end() )
    {
      CGameObject*    pObj = *itObj;
      
      if ( pObj->m_dwFlags & CGameObject::GOF_INVISIBLE )
      {
        ++itObj;
        continue;
      }
      if ( !( pObj->m_SecondaryFlags & CGameObject::GOF_EX_BEHIND_TILES ) )
      {
        ++itObj;
        continue;
      }

      if ( pObj->m_dwFlags & CGameObject::GOF_TILE )
      {
        DisplayTile( Renderer,
                     pObj->m_dwAnimID,
                     (int)( pObj->m_vPosition.x - vectPos.x ),
                     (int)( pObj->m_vPosition.y - vectPos.y ) );
      }
      else
      {
        size_t   iTile = pObj->m_dwAnimID;

        iTile = m_AnimationManager.GetAnimFrame( pObj->m_animPos );

        pObj->Render( Renderer, vectPos, iTile );

        /*
        //if ( CGameData::Instance().m_bShowColissionBoxes )
        {
          // Colission-Boxes
          math::tRect<int>   vecCR = pObj->CollisionRect();

          Renderer.RenderRect2d( vecCR.position() - GR::tPoint( vectPos.x, vectPos.y ), 
                                 vecCR.size(),
                                 0xffffff00 );
        }
        */
      }

      ++itObj;
    }

    // Alpha-Objekte
    if ( !m_listAlphaTiles.empty() )
    {
      Renderer.SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

      tAlphaTileList::iterator    itAT( m_listAlphaTiles.begin() );
      while ( itAT != m_listAlphaTiles.end() )
      {
        tTempTileDrawInfo&    TTDI = *itAT;

        Renderer.RenderTextureSection2d( TTDI.m_iX, TTDI.m_iY, *TTDI.m_pTexSection, -1, -1, -1, TTDI.m_dwAlternativeFlags );

        ++itAT;
      }

      Renderer.SetShader( XRenderer::ST_ALPHA_TEST );
    }

    // Tiles
    for ( int i = iX1; i <= iX2; i++ )
    {
      for ( int j = iY1; j <= iY2; j++ )
      {
        DisplayTile( Renderer,
                     Layer.Field( i, j ), 
                     i * Layer.m_vTileSize.x - (int)vectPos.x,
                     j * Layer.m_vTileSize.y - (int)vectPos.y );
      }
    }

    Renderer.SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

    tAlphaTileList::iterator    itAT( m_listAlphaTiles.begin() );
    while ( itAT != m_listAlphaTiles.end() )
    {
      tTempTileDrawInfo&    TTDI = *itAT;

      Renderer.RenderTextureSection2d( TTDI.m_iX, TTDI.m_iY, *TTDI.m_pTexSection, -1, -1, -1, TTDI.m_dwAlternativeFlags );

      ++itAT;
    }

    Renderer.SetShader( XRenderer::ST_ALPHA_TEST );

    m_listAlphaTiles.clear();

    itObj = Layer.m_listAliveObjects.begin();
    while ( itObj != Layer.m_listAliveObjects.end() )
    {
      CGameObject*    pObj = *itObj;
      
      if ( ( pObj->m_dwFlags & CGameObject::GOF_INVISIBLE )
      ||   ( pObj->m_SecondaryFlags & CGameObject::GOF_EX_BEHIND_TILES ) )
      {
        ++itObj;
        continue;
      }

      if ( pObj->m_dwFlags & CGameObject::GOF_TILE )
      {
        DisplayTile( Renderer,
                     pObj->m_dwAnimID,
                     (int)( pObj->m_vPosition.x - vectPos.x ),
                     (int)( pObj->m_vPosition.y - vectPos.y ) );
      }
      else
      {
        size_t   iTile = pObj->m_dwAnimID;

        iTile = m_AnimationManager.GetAnimFrame( pObj->m_animPos );

        pObj->Render( Renderer, vectPos, iTile );

        /*
        //if ( CGameData::Instance().m_bShowColissionBoxes )
        {
          // Colission-Boxes
          math::tRect<int>   vecCR = pObj->CollisionRect();

          Renderer.RenderRect2d( vecCR.position() - GR::tPoint( vectPos.x, vectPos.y ), 
                                 vecCR.size(),
                                 0xffffff00 );
        }
        */
      }

      ++itObj;
    }

    // Alpha-Objekte
    if ( !m_listAlphaTiles.empty() )
    {
      Renderer.SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

      tAlphaTileList::iterator    itAT( m_listAlphaTiles.begin() );
      while ( itAT != m_listAlphaTiles.end() )
      {
        tTempTileDrawInfo&    TTDI = *itAT;

        Renderer.RenderTextureSection2d( TTDI.m_iX, TTDI.m_iY, *TTDI.m_pTexSection, -1, -1, -1, TTDI.m_dwAlternativeFlags );

        ++itAT;
      }

      Renderer.SetShader( XRenderer::ST_ALPHA_TEST );
    }

    ++it;
  }

}



void CXJump::AddSection( XTexture* pTexture, int iX, int iY, int iWidth, int iHeight, GR::u32 dwFlags )
{

  m_vectTiles.push_back( XTextureSection( pTexture, iX, iY, iWidth, iHeight, dwFlags ) );

}



void CXJump::PreloadImages( XRenderer& Renderer )
{
  size_t   iTileSets = m_vectInfoTileSets.size();

  std::vector<XTexture*>   vectTS;

  for ( size_t i = 0; i < iTileSets; ++i )
  {
    GR::String&    strTS = m_vectInfoTileSets[i];

    GR::String     strFullPath = Path::Normalize( Path::Append( m_strDataPath, strTS ), false );
    vectTS.push_back( Renderer.LoadTexture( strFullPath.c_str(), GR::Graphic::IF_UNKNOWN, 0xffff00ff ) );
  }

  // Tiles erzeugen
  CXJump::tVectInfoTiles::iterator    it( m_vectInfoTiles.begin() );
  while ( it != m_vectInfoTiles.end() )
  {
    XJump::tTileInfo&    TileInfo = *it;

    AddSection( vectTS[TileInfo.iTileSetIndex],
                TileInfo.iTileX, TileInfo.iTileY,
                TileInfo.iTileWidth, TileInfo.iTileHeight,
                TileInfo.dwFlags );

    ++it;
  }

  // ColissionTilesets laden
  for ( size_t iCT = 0; iCT < m_vectInfoColissionTileSets.size(); ++iCT )
  {
    m_vectColissionTileSets.push_back( new GR::Graphic::Image( Path::Normalize( Path::Append( m_strDataPath, m_vectInfoColissionTileSets[iCT] ), false ).c_str() ) );
  }

  // Colission-Tiles erzeugen
  for ( size_t iCTile = 0; iCTile < m_vectInfoColissionTiles.size(); ++iCTile )
  {
    XJump::tTileInfo&  TileInfo = m_vectInfoColissionTiles[iCTile];

    GR::Graphic::Image*    pImageTS = m_vectColissionTileSets[TileInfo.iTileSetIndex & 0x7fff];

    GR::Graphic::ContextDescriptor    cdTS( pImageTS );

    GR::Graphic::ContextDescriptor    cdTile;

    cdTile.Attach( TileInfo.iTileWidth, TileInfo.iTileHeight, pImageTS->GetLineSize(), 
                 cdTS.ImageFormat(), 
                 ( (BYTE*)cdTS.Data() ) + TileInfo.iTileX * cdTS.BytesProPixel() + TileInfo.iTileY * cdTS.BytesPerLine() );

    m_vectColissionTiles.push_back( cdTile );
  }

}



CGameObject* CXJump::FindCollidingObject( CGameObject* pCollider, const GR::String& strType )
{

  return m_currentMap.FindCollidingObject( pCollider, strType );

}



void CXJump::StartGame()
{

  m_GameVars.Clear();
  m_LuaHandler.DoString( m_strOnGameStartScript.c_str() );

  if ( m_pControlledObject )
  {
    // auf kontrolliertes Objekt zentrieren
    m_currentMap.CenterOnObject( m_pControlledObject );
  }

}



void CXJump::StartLoadedGame()
{

  m_GameVars.Clear();
  m_LuaHandler.DoString( m_strOnLoadScript.c_str() );

}



void CXJump::StartScriptAsMovie( const GR::String& strScript )
{

  GR::String     strScriptName = Misc::Format( "TempMovie%1%" ) << (GR::u32)GetTickCount();
  GR::String     strCompleteScript = "function " + strScriptName + "()\n" + strScript + "\nend"; 

  m_LuaHandler.DoString( strCompleteScript.c_str() );
  m_LuaHandler.StartThread( strScriptName.c_str(), strScriptName.c_str() );

}



CGameObject* CXJump::GetObjectByType( const GR::String& strType )
{

  CLevelMap::tListLayer::iterator   itLayer( m_currentMap.m_listLayers.begin() );
  while ( itLayer != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    CLevelLayer::tListObjects::iterator   it( Layer.m_listAliveObjects.begin() );
    while ( it != Layer.m_listAliveObjects.end() )
    {
      CGameObject* pObj = *it;

      if ( GetObjectType( pObj ) == strType )
      {
        return pObj;
      }

      ++it;
    }

    CLevelLayer::tMapSleepingObjects::iterator   itSO( Layer.m_mapSleepingObjects.begin() );
    while ( itSO != Layer.m_mapSleepingObjects.end() )
    {
      CLevelLayer::tListObjects&    listSO = itSO->second;

      CLevelLayer::tListObjects::iterator   it( listSO.begin() );
      while ( it != listSO.end() )
      {
        CGameObject* pObj = *it;

        if ( GetObjectType( pObj ) == strType )
        {
          return pObj;
        }
        ++it;
      }

      ++itSO;
    }

    ++itLayer;
  }

  return NULL;

}



CGameObject* CXJump::GetObjectByID( GR::u32 iID )
{

  CLevelMap::tListLayer::iterator   itLayer( m_currentMap.m_listLayers.begin() );
  while ( itLayer != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    CLevelLayer::tListObjects::iterator   it( Layer.m_listAliveObjects.begin() );
    while ( it != Layer.m_listAliveObjects.end() )
    {
      CGameObject* pObj = *it;

      if ( pObj->m_dwID == iID )
      {
        return pObj;
      }

      ++it;
    }

    CLevelLayer::tMapSleepingObjects::iterator   itSO( Layer.m_mapSleepingObjects.begin() );
    while ( itSO != Layer.m_mapSleepingObjects.end() )
    {
      CLevelLayer::tListObjects&    listSO = itSO->second;

      CLevelLayer::tListObjects::iterator   it( listSO.begin() );
      while ( it != listSO.end() )
      {
        CGameObject* pObj = *it;

        if ( pObj->m_dwID == iID )
        {
          return pObj;
        }
        ++it;
      }

      ++itSO;
    }

    ++itLayer;
  }

  return NULL;

}



void CXJump::AddTileToCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile )
{

  m_mapTileCategories[eCategory].insert( dwTile );

}



void CXJump::RemoveTileFromCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile )
{

  tMapTileCategories::iterator    itMap( m_mapTileCategories.find( eCategory ) );
  if ( itMap == m_mapTileCategories.end() )
  {
    return;
  }

  tSetTileCategory&    setTiles = itMap->second;

  setTiles.erase( dwTile );

}


bool CXJump::IsTileInCategory( const XJump::eTileCategory eCategory, const GR::u32 dwTile ) const
{

  tMapTileCategories::const_iterator    itMap( m_mapTileCategories.find( eCategory ) );
  if ( itMap == m_mapTileCategories.end() )
  {
    return false;
  }

  const tSetTileCategory&    setTiles = itMap->second;

  tSetTileCategory::const_iterator   it( setTiles.find( dwTile ) );
  if ( it != setTiles.end() )
  {
    return true;
  }

  return false;

}



void CXJump::SoftScrollViewWithObject( CGameObject* pObject, const float fElapsedTime )
{

  if ( pObject == NULL )
  {
    return;
  }

  math::vector3   vectSollPos;

  vectSollPos.x = pObject->m_vPosition.x - m_ptViewportExtents.x / 2;
  vectSollPos.y = pObject->m_vPosition.y - m_ptViewportExtents.y / 2;
  vectSollPos.z = m_currentMap.m_vectScrollOffset.z;

  math::vector3   vectMovement = pObject->m_vDelta;
  vectMovement.normalize();

  // immer in Laufrichtung ein Stück voraus
  //vectSollPos += vectMovement * 150.0f;

  ClipToCurrentRange( vectSollPos );

  math::vector3   vectDiff = vectSollPos - m_currentMap.m_vectScrollOffset;

  math::vector3   vectDir = vectDiff;

  vectDir.normalize();

  vectDir *= 200.0f * fElapsedTime;

  if ( vectDir.length_squared() >= vectDiff.length_squared() )
  {
    m_currentMap.m_vectScrollOffset = vectSollPos;
  }
  else
  {
    m_currentMap.m_vectScrollOffset += vectDir;
  }

  if ( vectSollPos.x - m_currentMap.m_vectScrollOffset.x > m_ptViewportExtents.x / 3 )
  {
    m_currentMap.m_vectScrollOffset.x = vectSollPos.x - m_ptViewportExtents.x / 3;
  }
  if ( vectSollPos.x - m_currentMap.m_vectScrollOffset.x < -m_ptViewportExtents.x / 3 )
  {
    m_currentMap.m_vectScrollOffset.x = vectSollPos.x + m_ptViewportExtents.x / 3;
  }
  if ( vectSollPos.y - m_currentMap.m_vectScrollOffset.y > m_ptViewportExtents.y / 3 )
  {
    m_currentMap.m_vectScrollOffset.y = vectSollPos.y - m_ptViewportExtents.y / 3;
  }
  if ( vectSollPos.y - m_currentMap.m_vectScrollOffset.y < -m_ptViewportExtents.y / 3 )
  {
    m_currentMap.m_vectScrollOffset.y = vectSollPos.y + m_ptViewportExtents.y / 3;
  }

  // vorerst
  m_currentMap.m_vectScrollOffset = vectSollPos;

}



void CXJump::ProcessExtraData( CGameObject* pObject )
{

  if ( pObject == NULL )
  {
    return;
  }
  if ( !( pObject->m_dwFlags & CGameObject::GOF_EXTRA_DATA ) )
  {
    return;
  }

  tExtraData*   pData = CXJump::Instance().m_currentMap.GetExtraData( pObject->m_dwExtraData );
  if ( pData )
  {
    if ( pData->m_Type == tExtraData::EX_SCRIPT )
    {
      Luna<CLuaGameObject>::PushObject( m_LuaHandler.m_LuaInstance, new CLuaGameObject( pObject ) );
      lua_setglobal( m_LuaHandler.m_LuaInstance, "Self" );
      StartScriptAsMovie( pData->m_strParam );
    }
    else if ( pData->m_Type == tExtraData::EX_WARP_TARGET )
    {
      CXJump::Instance().ProcessEvent( tGameEvent( tGameEvent::GE_WARP, "", pData->m_iParam[0], pData->m_iParam[1] ) );
    }
  }

}



GR::u32 CXJump::Animation( const GR::String& strAnim ) const
{

  tMapAnimations::const_iterator    it( m_mapAnimations.find( strAnim ) );
  if ( it == m_mapAnimations.end() )
  {
    return 0;
  }
  return it->second;

}



GR::tPoint CXJump::ObjectLocationOnScreen( CGameObject* pThisObj )
{

  if ( pThisObj == NULL )
  {
    return GR::tPoint();
  }

  GR::tVector vectPos = m_currentMap.m_vectScrollOffset;//m_Camera.GetPosition();

  vectPos.x = floorf( vectPos.x );
  vectPos.y = floorf( vectPos.y );

  CLevelMap::tListLayer::iterator    it( m_currentMap.m_listLayers.begin() );
  while ( it != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer&    Layer = *it;

    if ( Layer.m_Type == CLevelLayer::LT_COLLISION )
    {
      ++it;
      continue;
    }

    CLevelLayer::tListObjects::iterator    itObj( Layer.m_listAliveObjects.begin() );
    while ( itObj != Layer.m_listAliveObjects.end() )
    {
      CGameObject*    pObj = *itObj;

      if ( pObj == pThisObj )
      {
        return GR::tPoint( (int)( pObj->m_vPosition.x - vectPos.x ),
                           (int)( pObj->m_vPosition.y - vectPos.y ) );
      }
      ++itObj;
    }

    CLevelLayer::tMapSleepingObjects::iterator   itSO( Layer.m_mapSleepingObjects.begin() );
    while ( itSO != Layer.m_mapSleepingObjects.end() )
    {
      CLevelLayer::tListObjects&    listSO = itSO->second;

      CLevelLayer::tListObjects::iterator   it( listSO.begin() );
      while ( it != listSO.end() )
      {
        CGameObject* pObj = *it;

        if ( pObj == pThisObj )
        {
          return GR::tPoint( (int)( pObj->m_vPosition.x - vectPos.x ),
                             (int)( pObj->m_vPosition.y - vectPos.y ) );
        }
        ++it;
      }

      ++itSO;
    }

    ++it;
  }

  return GR::tPoint();

}



void CXJump::EnableTrigger( GR::u32 dwTriggerID, bool bEnable )
{

  m_currentMap.m_pMainLayer->EnableTrigger( dwTriggerID, bEnable );

}



void CXJump::RemoveMap( const GR::String& MapName )
{

  if ( m_strCurrentMapName == MapName )
  {
    m_strCurrentMapName.erase();
    m_currentMap.Clear();
  }
  m_listMaps.remove( MapName );

}