// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#include "XJump.h"

#include <Misc/Misc.h>
#include <Misc/CloneFactory.h>

#include <Grafik/Image.h>

#include <Input/Input.h>

#include <String/Path.h>

#include <DX8/DX8Viewer.h>
#include <DX8/DX8Shader.h>

#include <IO/FileStream.h>

#include "LuaGameObject.h"

#include <debug/debugclient.h>



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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



/*-Instance-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CXJump& CXJump::Instance()
{

  static CXJump     g_XJump;

  return g_XJump;

}



bool CXJump::InitInstance()
{

  m_LuaHandler.InitInstance();

  CInputSystem::Instance().AddCommand( "left",        CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_LEFT ) );
  CInputSystem::Instance().AddCommand( "right",       CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_RIGHT ) );
  CInputSystem::Instance().AddCommand( "up",          CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_UP ) );
  CInputSystem::Instance().AddCommand( "down",        CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_DOWN ) );
  CInputSystem::Instance().AddCommand( "jumppushed",  CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_SPACE ) );
  CInputSystem::Instance().AddCommand( "jump",        CInputSystem::Instance().MapKeyToVKey( CInputSystem::KEY_SPACE ), tInputCommand::CMD_RELEASED_DOWN );

  return true;

}



/*-Clear----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Clear()
{

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

  m_AnimationManager.clear();
  m_listMaps.clear();
  m_vectAnimPositions.clear();
  m_vectColissionTiles.clear();
  m_vectInfoColissionTiles.clear();
  m_vectInfoTiles.clear();
  m_vectInfoTileSets.clear();
  m_vectInfoColissionTileSets.clear();

  m_mapTileCategories.clear();

}



/*-ClipToCurrentRange---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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



/*-Update---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Update( const float fElapsedTime )
{

  ProcessQueue();

  m_LuaHandler.ResumeThreads( fElapsedTime );

  if ( m_bFrozen )
  {
    return;
  }

  ProcessQueue();

  m_currentMap.Update( fElapsedTime );

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



/*-BlockedAt------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::BlockedAt( CGameObject* pMovingObject, int iX, int iY, eDirType dtDir, eActionType& eAction, bool bCanPush )
{

#pragma TODO( "versch. Kolissionstypen! Wasser etc." )
  int   iColType = m_currentMap.ColissionTypeAt( iX, iY );

  if ( ( pMovingObject )
  &&   ( !( pMovingObject->m_dwFlags & CGameObject::GOF_CAN_PUSH ) ) )
  {
    bCanPush = false;
  }

  eAction = AT_NONE;
  if ( iColType )
  {
    if ( iColType == 0x7c00 )
    {
      // nach unten fest (schwebende Plattformen)
      if ( dtDir != DIR_DOWN )
      {
        return false;
      }

      return !pMovingObject->IsClimbing();
    }
    else if ( iColType == 0x7c1f )
    {
      // kaputt!
      eAction = AT_KILLED;
      return true;
    }
  }

  if ( iColType != 0 )
  {
    return true;
  }

  // blockierende Objekte pr¸fen
  if ( pMovingObject )
  {
    GR::tRect     rcMovingObject = pMovingObject->ColissionRect();

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

        if ( rcMovingObject.intersects( pOtherObj->ColissionRect() ) )
        {
          bool    bCanBeMoved = false;

          if ( ( pOtherObj->m_dwFlags & CGameObject::GOF_PUSHABLE )
          &&   ( bCanPush )
          &&   ( pOtherObj->IsOnGround() ) )
          {
            // kann das andere Objekt geschoben werden?
            switch ( dtDir )
            {
              case DIR_LEFT:
                bCanBeMoved = pOtherObj->Move( math::vector3( -1, 0, 0 ), true );
                break;
              case DIR_RIGHT:
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



/*-IsAreaFree-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::IsAreaFree( int iX, int iY, int iWidth, int iHeight )
{

  CXJump::eActionType   atType = AT_NONE;

  for ( int i = 0; i < iWidth; ++i )
  {
    for ( int j = 0; j < iHeight; ++j )
    {
      if ( BlockedAt( NULL, iX + i, iY + j, CXJump::DIR_NONE, atType, false ) )
      {
        return false;
      }
    }
  }
  return true;

}



/*-StepPossible---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::StepPossible( CGameObject* pMovingObject, int iX, int iY, int iStepHeight, int& iResultStepHeight, eActionType& atType, eDirType dtDir, bool bCanPush )
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
  GR::tRect   rcColission = pMovingObject->ColissionRect();

  rcColission.offset( GR::tPoint( iX, iY ) - pMovingObject->TruePosition() );

  int   iStartStepAt = iGravity * iStepHeight;
  iResultStepHeight = 0;
  while ( iGravity * iStepHeight >= - iGravity * iStartStepAt )
  {
    if ( BlockedAt( pMovingObject, iX, iY - iGravity * iStepHeight, DIR_NONE, atType, bCanPush ) )
    {
      iResultStepHeight = iStepHeight + iGravity;
      break;
    }
    iStepHeight -= iGravity;
  }

  return IsAreaFree( rcColission.m_iLeft, rcColission.m_iTop - iGravity * iStepHeight, rcColission.width(), iGravity * iStepHeight );

}



/*-Gravity--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

math::vector3 CXJump::Gravity()
{

  return m_vGravity;

}



/*-Gravity--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Gravity( math::vector3& vGrav )
{

  m_vGravity = vGrav;

}



/*-Load-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::Load( const char* szFileName )
{

  Clear();
  CCloneFactory::Instance().Clear();

  CFileStream   File;


  if ( !File.Open( szFileName ) )
  {
    return false;
  }

  Load( File );

  File.Close();

  m_strDataPath = CPath::RemoveFileSpec( szFileName );

  return true;

}



/*-Load-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Load( IIOStream& Stream )
{

  // Version
  GR::u32   dwVersion = Stream.ReadLong();

  // TileSets
  m_vectInfoTileSets.clear();
  int   iTileSetCount = Stream.ReadLong();
  m_vectInfoTileSets.reserve( iTileSetCount );
  for ( int iTS = 0; iTS < iTileSetCount; ++iTS )
  {
    GR::String    strTileSet;

    Stream.ReadString( strTileSet );
    m_vectInfoTileSets.push_back( strTileSet );
  }

  // Colission-TileSets
  m_vectInfoColissionTileSets.clear();
  int   iCTileSetCount = Stream.ReadLong();
  m_vectInfoColissionTileSets.reserve( iTileSetCount );
  for ( int iCTS = 0; iCTS < iCTileSetCount; ++iCTS )
  {
    GR::String    strTileSet;

    Stream.ReadString( strTileSet );
    m_vectInfoColissionTileSets.push_back( strTileSet );
  }

  // Tiles
  m_vectInfoTiles.clear();
  int   iTileCount = Stream.ReadLong();
  m_vectInfoTiles.reserve( iTileCount );
  for ( int iTile = 0; iTile < iTileCount; ++iTile )
  {
    tTileInfo   TileInfo;

    Stream.ReadBlock( &TileInfo, sizeof( TileInfo ) );
    m_vectInfoTiles.push_back( TileInfo );
  }

  // Tiles
  m_vectInfoColissionTiles.clear();
  int   iClsTileCount = Stream.ReadLong();
  m_vectInfoColissionTiles.reserve( iClsTileCount );
  for ( int iClsTile = 0; iClsTile < iClsTileCount; ++iClsTile )
  {
    tTileInfo   TileInfo;

    Stream.ReadBlock( &TileInfo, sizeof( TileInfo ) );
    m_vectInfoColissionTiles.push_back( TileInfo );
  }

  // Animationen
  m_AnimationManager.Load( Stream );

  m_vectAnimPositions.resize( m_AnimationManager.m_vectAnimations.size() );
  for ( size_t i = 0; i < m_AnimationManager.m_vectAnimations.size(); ++i )
  {
    m_vectAnimPositions[i].m_dwAnimationId = (GR::u32)i;
  }

  // Dateinamen der Levels
  m_listMaps.clear();
  int   iMapCount = Stream.ReadLong();
  for ( int iMap = 0; iMap < iMapCount; ++iMap )
  {
    GR::String   strMapName;

    Stream.ReadString( strMapName );
    m_listMaps.push_back( strMapName );
  }

  // Known Objects
  int   iKnownObjectCount = Stream.ReadLong();

  m_vectKnownObjects.resize( iKnownObjectCount );
  for ( int iKO = 0; iKO < iKnownObjectCount; ++iKO )
  {
    Stream.ReadString( m_vectKnownObjects[iKO].first );

    // das sollte eigentlich direkt in die Factory?
    m_vectKnownObjects[iKO].second = new CGameObject();
    m_vectKnownObjects[iKO].second->Load( Stream );

    m_mapKnownObjects[m_vectKnownObjects[iKO].first] = m_vectKnownObjects[iKO].second;
  }

  // Preferred Colission TypesKnown Objects
  int   iPreferredColissionTypeCount = Stream.ReadLong();

  for ( int iPCT = 0; iPCT < iPreferredColissionTypeCount; ++iPCT )
  {
    GR::u32   dwTile = Stream.ReadLong();

    m_mapPreferredColissionTile[dwTile] = Stream.ReadLong();
  }

  // Object-Pfade
  int   iObjectPathCount = Stream.ReadLong();

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
  int   iTileCategories = Stream.ReadLong();
  for ( int iTC = 0; iTC < iTileCategories; ++iTC )
  {
    eTileCategory    eTC = (eTileCategory)Stream.ReadLong();

    int   iTiles = Stream.ReadLong();
    for ( int j = 0; j < iTiles; ++j )
    {
      m_mapTileCategories[eTC].insert( Stream.ReadWord() );
    }
  }

  // Extra-Data
  GR::u32   dwExtraDataLength = Stream.ReadLong();

}



/*-Save-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::Save( const char* szFileName )
{

  CFileStream   File;


  if ( !File.Open( szFileName, IIOStream::OT_WRITE_ONLY ) )
  {
    return false;
  }

  Save( File );

  File.Close();

  return true;

}



/*-Save-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Save( IIOStream& Stream )
{

  // Version
  Stream.WriteLong( 0x0001 );

  // TileSets
  Stream.WriteLong( (GR::u32)m_vectInfoTileSets.size() );
  tVectInfoTileSets::iterator   itTS( m_vectInfoTileSets.begin() );
  while ( itTS != m_vectInfoTileSets.end() )
  {
    GR::String&    strTileSet = *itTS;

    Stream.WriteString( strTileSet );

    ++itTS;
  }

  // Colission-TileSets
  Stream.WriteLong( (GR::u32)m_vectInfoColissionTileSets.size() );
  tVectInfoTileSets::iterator   itTS2( m_vectInfoColissionTileSets.begin() );
  while ( itTS2 != m_vectInfoColissionTileSets.end() )
  {
    GR::String&    strTileSet = *itTS2;

    Stream.WriteString( strTileSet );

    ++itTS2;
  }

  // Tiles
  Stream.WriteLong( (GR::u32)m_vectInfoTiles.size() );
  tVectInfoTiles::iterator   itTile( m_vectInfoTiles.begin() );
  while ( itTile != m_vectInfoTiles.end() )
  {
    tTileInfo&    TileInfo = *itTile;

    Stream.WriteBlock( &TileInfo, sizeof( TileInfo ) );

    ++itTile;
  }

  // Colission-Tiles
  Stream.WriteLong( (GR::u32)m_vectInfoColissionTiles.size() );
  tVectInfoTiles::iterator   itClsTile( m_vectInfoColissionTiles.begin() );
  while ( itClsTile != m_vectInfoColissionTiles.end() )
  {
    tTileInfo&    TileInfo = *itClsTile;

    Stream.WriteBlock( &TileInfo, sizeof( TileInfo ) );

    ++itClsTile;
  }

  // Animationen
  m_AnimationManager.Save( Stream );

  // Dateinamen der Levels
  Stream.WriteLong( (GR::u32)m_listMaps.size() );
  tListMaps::iterator   itMap( m_listMaps.begin() );
  while ( itMap != m_listMaps.end() )
  {
    GR::String&    strMap = *itMap;

    Stream.WriteString( strMap );

    ++itMap;
  }

  // Known Objects
  Stream.WriteLong( (GR::u32)m_vectKnownObjects.size() );
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
  Stream.WriteLong( (GR::u32)m_mapPreferredColissionTile.size() );
  tMapPreferredColissionTile::iterator   itPCT( m_mapPreferredColissionTile.begin() );
  while ( itPCT != m_mapPreferredColissionTile.end() )
  {
    Stream.WriteLong( itPCT->first );
    Stream.WriteLong( itPCT->second );

    ++itPCT;
  }

  // Object-Pfade
  Stream.WriteLong( (GR::u32)m_mapObjectPaths.size() );
  tMapObjectPaths::iterator   itOP( m_mapObjectPaths.begin() );
  while ( itOP != m_mapObjectPaths.end() )
  {
    Stream.WriteString( itOP->first );

    GR::GAME::CObjectPath&    OPath = itOP->second;

    OPath.Save( Stream );

    ++itOP;
  }

  // On-Load-Script
  Stream.WriteString( m_strOnLoadScript );

  // Tile-Kategorien
  Stream.WriteLong( (GR::u32)m_mapTileCategories.size() );

  tMapTileCategories::iterator    itMTC( m_mapTileCategories.begin() );
  while ( itMTC != m_mapTileCategories.end() )
  {
    Stream.WriteLong( itMTC->first );

    tSetTileCategory&    listTC = itMTC->second;

    Stream.WriteLong( (GR::u32)listTC.size() );

    tSetTileCategory::iterator   it( listTC.begin() );
    while ( it != listTC.end() )
    {
      Stream.WriteWord( *it );

      ++it;
    }
    ++itMTC;
  }

  // Extra-Data
  Stream.WriteLong( 0 );

}



/*-SaveGameState--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::SaveGameState( IIOStream& Stream )
{

  m_GameVars.SaveVars( Stream );

  Stream.WriteString( m_strCurrentMapName );

  Stream.WriteFloat( m_pControlledObject->m_vPosition.x );
  Stream.WriteFloat( m_pControlledObject->m_vPosition.y );
  Stream.WriteFloat( m_pControlledObject->m_vPosition.z );

}



/*-LoadGameState--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::LoadGameState( IIOStream& Stream )
{

  m_GameVars.LoadVars( Stream );

  Stream.ReadString( m_strCurrentMapName );

  float     fX = Stream.ReadFloat();
  float     fY = Stream.ReadFloat();
  Stream.ReadFloat();   // z

  ProcessEvent( tGameEvent( tGameEvent::GE_CHANGE_MAP, m_strCurrentMapName, 1, (int)fX, (int)fY ) );

}



/*-GetColissionTileCD---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CContextDescriptor* CXJump::GetColissionTileCD( GR::u32 dwTile )
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



/*-AddKnownObject-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CXJump::AddKnownObject( const GR::String& strName, CGameObject* pObj )
{

  tPairKnownObject    Pair;

  Pair.first = strName;
  Pair.second = pObj;

  m_vectKnownObjects.push_back( Pair );

  return (int)m_vectKnownObjects.size() - 1;

}



/*-GetKnownObject-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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



/*-RenameKnownObject----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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
  return (CGameObject*)it->second->Clone();

}



/*-RegisterObject-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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

  // wichtige Werte vom alten Objekt ¸bernehmen (irgendwie ist das nicht sauber)
  pObj->m_dwFlags         = pGameObject->m_dwFlags;
  pObj->m_animPos         = pGameObject->m_animPos;
  pObj->m_dwAnimID        = pGameObject->m_dwAnimID;
  pObj->m_dwType          = pGameObject->m_dwType;
  pObj->m_vColissionRect  = pGameObject->m_vColissionRect;
  pObj->SetAnimation( pObj->m_dwAnimID );

  m_mapKnownObjects[strName] = pObj;

}


bool CXJump::IsObjectType( CGameObject* pObject, const GR::String& strName )
{

  if ( pObject == NULL )
  {
    return false;
  }
  return ( pObject->m_strClassName == strName );

}



/*-PrepareForStart------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::PrepareForStart()
{

  m_currentMap.PrepareForStart();

}



/*-GetTileSection-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

tTextureSection CXJump::GetTileSection( GR::u32 dwTile )
{

  if ( dwTile >= m_vectTiles.size() )
  {
    return tTextureSection();
  }
  return m_vectTiles[dwTile];

}



/*-GetTileInfo----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CXJump::tTileInfo* CXJump::GetTileInfo( GR::u32 dwTile )
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



/*-Freeze---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::Freeze( bool bFreeze )
{

  m_bFrozen = bFreeze;

}



/*-MovieMode------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::MovieMode( bool bMovie )
{

  m_bMovieMode = bMovie;

}



/*-SetViewport----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::SetViewport( int iWidth, int iHeight )
{

  m_ptViewportExtents.x   = iWidth;
  m_ptViewportExtents.y   = iHeight;

}



/*-ProcessEvent---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CXJump::ProcessEvent( const tGameEvent& Event )
{

  switch ( Event.m_Type )
  {
    case tGameEvent::GE_WARP:
      if ( m_pControlledObject )
      {
        m_pControlledObject->m_vPosition.x = (float)Event.m_iEventParam[0];
        m_pControlledObject->m_vPosition.y = (float)Event.m_iEventParam[1];

        m_currentMap.CenterOnObject( m_pControlledObject );
      }
      break;
    case tGameEvent::GE_CHANGE_MAP:
      if ( !m_currentMap.Load( CPath::Normalize( CPath::Append( m_strDataPath, Event.m_strEventParam ) ).c_str() ) )
      {
        dh::Log( "Failed to Load Map (%s)\n", CPath::Normalize( CPath::Append( m_strDataPath, Event.m_strEventParam ) ).c_str() );
        return true;
      }

      m_strCurrentMapName = Event.m_strEventParam;

      m_LuaHandler.DoFile( CPath::Normalize( CPath::Append( m_strDataPath, CPath::RenameExtension( Event.m_strEventParam, "lua" ) ) ).c_str() );
      PrepareForStart();
      if ( m_currentMap.m_pMainLayer == NULL )
      {
        dh::Log( "No Main Layer found\n" );
      }
      else
      {
        m_pControlledObject = m_currentMap.m_pMainLayer->GetObject( 0 );
        if ( Event.m_iEventParam[0] != 0 )
        {
          // Level-Wechsel, Spieler auf gew¸nschte Position setzen
          m_pControlledObject->m_vPosition.x = (float)Event.m_iEventParam[1];
          m_pControlledObject->m_vPosition.y = (float)Event.m_iEventParam[2];

          m_pControlledObject->ProcessEvent( tObjectEvent( CGameObject::ET_MAP_CHANGED ) );
        }
      }
      m_Camera.SetProjectionType( CDX8Camera::OrthoLH );
      m_Camera.ViewWidth( m_ptViewportExtents.x );
      m_Camera.ViewHeight( -(int)m_ptViewportExtents.y );
      m_Camera.ZNear( 0.0f );
      m_Camera.ZFar( 100.0f );
      m_Camera.SetValues( D3DXVECTOR3( (float)m_ptViewportExtents.x * 0.5f, (float)m_ptViewportExtents.y * 0.5f, -20.0f ),
                          D3DXVECTOR3( (float)m_ptViewportExtents.x * 0.5f, (float)m_ptViewportExtents.y * 0.5f, 0.0f ),
                          D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );

      // auf kontrolliertes Objekt zentrieren
      m_currentMap.CenterOnObject( m_pControlledObject );
      return true;
  }
  return false;

}



bool CXJump::OnTopOfPlatform( CGameObject* pObject )
{

  CXJump::tPoint    ObjPos = pObject->TruePosition();

  math::tRect<int>   vecCR = pObject->m_vColissionRect;

  vecCR.offset( ObjPos );


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

    CXJump::tPoint    ObjPos = pObj->TruePosition();

    math::tRect<int>   vecCR2 = pObj->m_vColissionRect;

    vecCR2.offset( ObjPos );

    if ( ( vecCR.position().y + vecCR.size().y == vecCR2.position().y )
    &&   ( vecCR.position().x <= vecCR2.position().x + vecCR2.size().x )
    &&   ( vecCR.position().x + vecCR.size().x >= vecCR2.position().x ) )
    {
      pObject->m_pPlatform = pObj;
      if ( !( pObject->m_dwFlags & CGameObject::GOF_ON_PLATFORM ) )
      {
        pObject->m_dwFlags |= CGameObject::GOF_ON_PLATFORM;

        pObj->ProcessEvent( tObjectEvent( CGameObject::ET_CARRY_OBJECT, (GR::up)pObject ) );
      }
      return true;
    }

    ++it;
  }

  if ( pObject->m_pPlatform )
  {
    pObject->m_pPlatform->ProcessEvent( tObjectEvent( CGameObject::ET_DROP_OBJECT, (GR::up)pObject ) );
    pObject->m_pPlatform = NULL;
    pObject->m_dwFlags &= ~CGameObject::GOF_ON_PLATFORM;
  }

  return false;

}



/*-DisplayTile----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::DisplayTile( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwAlternativeFlags )
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

  tTextureSection&    Section = m_vectTiles[iTile];

  if ( ( Section.m_pTexture )
  &&   ( Section.m_pTexture->HasAlpha() ) )
  {
    m_listAlphaTiles.push_back( tTempTileDrawInfo( &Section, fX, fY, dwAlternativeFlags ) );
  }
  else
  {
    Viewer.DrawTextureSection( fX, fY, Section, -1, -1, -1, dwAlternativeFlags );
  }

}



/*-DisplayObject--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::DisplayObject( CD3DViewer& Viewer, size_t iTile, int fX, int fY, DWORD dwDisplayFlags )
{

  if ( iTile >= m_vectTiles.size() )
  {
    return;
  }

  tTextureSection&    Section = m_vectTiles[iTile];

  Viewer.SetTexture( 0, 0 );
  Viewer.DrawBox( fX - 4, fY, 9, 1, 0xffff00ff );
  Viewer.DrawBox( fX, fY - 4, 1, 9, 0xffff00ff );

  Viewer.SetTexture( 0, Section.m_pTexture );





  /*
  SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
  SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );
  */

  struct tMyVertex
  {
    D3DXVECTOR3   vectPos;
    float         fRHW;
    DWORD         dwColor;
    float         fTU;
    float         fTV;
  };

  tMyVertex    vertexData[4];

  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VBOTTOM )
  {
    fY -= Section.m_iHeight - 1;
  }
  else if ( dwDisplayFlags & CGameObject::GOF_ALIGN_VCENTER )
  {
    fY -= Section.m_iHeight / 2;
  }
  if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HRIGHT )
  {
    fX -= Section.m_iWidth - 1;
  }
  else if ( dwDisplayFlags & CGameObject::GOF_ALIGN_HCENTER )
  {
    fX -= Section.m_iWidth / 2;
  }

  float   fZ = 0.0f;

  float   fX1 = fX - 0.5f;
  float   fY1 = fY - 0.5f;
  float   fX2 = fX + Section.m_iWidth - 0.5f;
  float   fY2 = fY + Section.m_iHeight - 0.5f;

  int     iTU[2],
          iTV[2];

  iTU[0] = 0;
  iTU[1] = 1;
  iTV[0] = 0;
  iTV[1] = 1;

  if ( dwDisplayFlags & CGameObject::GOF_HMIRROR )
  {
    iTU[0] = 1;
    iTU[1] = 0;
  }
  if ( dwDisplayFlags & CGameObject::GOF_VMIRROR )
  {
    iTV[0] = 1;
    iTV[1] = 0;
  }

  vertexData[0].fRHW = vertexData[1].fRHW = vertexData[2].fRHW = vertexData[3].fRHW = 0.2f;

  vertexData[0].vectPos = D3DXVECTOR3( fX1, fY1, fZ );
  vertexData[0].dwColor = 0xffffffff;
  vertexData[0].fTU     = Section.m_fTU[iTU[0]];
  vertexData[0].fTV     = Section.m_fTV[iTV[0]];

  vertexData[1].vectPos = D3DXVECTOR3( fX2, fY1, fZ );
  vertexData[1].dwColor = 0xffffffff;
  vertexData[1].fTU     = Section.m_fTU[iTU[1]];
  vertexData[1].fTV     = Section.m_fTV[iTV[0]];

  vertexData[2].vectPos = D3DXVECTOR3( fX1, fY2, fZ );
  vertexData[2].dwColor = 0xffffffff;
  vertexData[2].fTU     = Section.m_fTU[iTU[0]];
  vertexData[2].fTV     = Section.m_fTV[iTV[1]];

  vertexData[3].vectPos = D3DXVECTOR3( fX2, fY2, fZ );
  vertexData[3].dwColor = 0xffffffff;
  vertexData[3].fTU     = Section.m_fTU[iTU[1]];
  vertexData[3].fTV     = Section.m_fTV[iTV[1]];

  Viewer.SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  Viewer.m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, &vertexData, sizeof( vertexData[0] ) );

}



void CXJump::DisplayField( CD3DViewer& Viewer )
{

  Viewer.SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
  Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  Viewer.SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  Viewer.SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  Viewer.SetRenderState( D3DRS_ALPHAREF,        0x08 );
  Viewer.SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  D3DXMATRIX    matWorld;

  D3DXMatrixIdentity( &matWorld );

  Viewer.SetTransform( D3DTS_WORLD, &matWorld );

  Viewer.SetTransform( D3DTS_VIEW,       &m_Camera.GetViewMatrix() );
  Viewer.SetTransform( D3DTS_PROJECTION, &m_Camera.GetProjectionMatrix() );

  if ( m_pTextureBackground )
  {
    Viewer.SetTexture( 0, m_pTextureBackground );
    Viewer.DrawQuad( 0, 0, Viewer.Width(), Viewer.Height(), 0xffffffff );
  }

  D3DXVECTOR3 vectPos = m_currentMap.m_vectScrollOffset;//m_Camera.GetPosition();

  vectPos.x = floorf( vectPos.x );
  vectPos.y = floorf( vectPos.y );

  CLevelMap::tListLayer::iterator    it( m_currentMap.m_listLayers.begin() );
  while ( it != m_currentMap.m_listLayers.end() )
  {
    CLevelLayer&    Layer = *it;

    if ( Layer.m_Type == CLevelLayer::LT_COLISSION )
    {
      ++it;
      continue;
    }

    GR::CDX8ShaderAlphaTest::Apply( Viewer );

    m_listAlphaTiles.clear();

    int   iX1 = int( vectPos.x / Layer.m_vTileSize.x - 1 ),
          iY1 = int( vectPos.y / Layer.m_vTileSize.y - 1 );
    int   iX2 = iX1 + Viewer.Width() / Layer.m_vTileSize.x + 1;
    int   iY2 = iY1 + Viewer.Height() / Layer.m_vTileSize.y + 1;

    // +4, damit die Objekte ein biﬂchen auﬂerhalb auch mitkommen
    Layer.SpawnObjects( (float)( ( iX1 - 4 ) * Layer.m_vTileSize.x ),
                        (float)( ( iY1 - 4 ) * Layer.m_vTileSize.y ),
                        (float)( ( iX2 - iX1 + 1 + 4 ) * Layer.m_vTileSize.x ),
                        (float)( ( iY2 - iY1 + 1 + 4 ) * Layer.m_vTileSize.y ) );
    for ( int i = iX1; i <= iX2; i++ )
    {
      for ( int j = iY1; j <= iY2; j++ )
      {
        DisplayTile( Viewer,
                     Layer.GetFeld( i, j ),
                     i * Layer.m_vTileSize.x - (int)vectPos.x,
                     j * Layer.m_vTileSize.y - (int)vectPos.y );
      }
    }

    GR::CDX8ShaderAlphaBlendAndTest::Apply( Viewer );

    tAlphaTileList::iterator    itAT( m_listAlphaTiles.begin() );
    while ( itAT != m_listAlphaTiles.end() )
    {
      tTempTileDrawInfo&    TTDI = *itAT;

      Viewer.DrawTextureSection( TTDI.m_iX, TTDI.m_iY, *TTDI.m_pTexSection, -1, -1, -1, TTDI.m_dwAlternativeFlags );

      ++itAT;
    }

    GR::CDX8ShaderAlphaTest::Apply( Viewer );

    m_listAlphaTiles.clear();

    CLevelLayer::tListObjects::iterator    itObj( Layer.m_listAliveObjects.begin() );
    while ( itObj != Layer.m_listAliveObjects.end() )
    {
      CGameObject*    pObj = *itObj;

      if ( pObj->m_dwFlags & CGameObject::GOF_INVISIBLE )
      {
        ++itObj;
        continue;
      }

      if ( pObj->m_dwFlags & CGameObject::GOF_TILE )
      {
        DisplayTile( Viewer,
                     pObj->m_dwAnimID,
                     (int)( pObj->m_vPosition.x - vectPos.x ),
                     (int)( pObj->m_vPosition.y - vectPos.y ) );
      }
      else
      {
        size_t   iTile = pObj->m_dwAnimID;

        iTile = m_AnimationManager.GetAnimFrame( pObj->m_animPos );

        pObj->Render( Viewer, vectPos, iTile );

        /*
        if ( CGameData::Instance().m_bShowColissionBoxes )
        {
          // Colission-Boxes
          CXJump::tPoint    ObjPos = pObj->TruePosition();

          math::tRect<int>   vecCR = pObj->m_vColissionRect;

          vecCR.offset( ObjPos );

          Viewer.DrawBox( (int)( vecCR.m_iLeft - vectPos.x ),
                          (int)( vecCR.m_iTop - vectPos.y ),
                          (int)vecCR.width(),
                          (int)vecCR.height(),
                          0xffffff00 );
        }
        */
      }

      ++itObj;
    }

    // Alpha-Objekte
    {
      GR::CDX8ShaderAlphaBlendAndTest::Apply( Viewer );

      tAlphaTileList::iterator    itAT( m_listAlphaTiles.begin() );
      while ( itAT != m_listAlphaTiles.end() )
      {
        tTempTileDrawInfo&    TTDI = *itAT;

        Viewer.DrawTextureSection( TTDI.m_iX, TTDI.m_iY, *TTDI.m_pTexSection, -1, -1, -1, TTDI.m_dwAlternativeFlags );

        ++itAT;
      }

      GR::CDX8ShaderAlphaTest::Apply( Viewer );
    }

    ++it;
  }

}



/*-AddSection-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CXJump::AddSection( CDX8Texture* pTexture, int iX, int iY, int iWidth, int iHeight, GR::u32 dwFlags )
{

  m_vectTiles.push_back( tTextureSection() );

  tTextureSection&    newSect = m_vectTiles.back();

  newSect.m_pTexture  = pTexture;
  newSect.m_iXOffset  = iX;
  newSect.m_iYOffset  = iY;
  newSect.m_iWidth    = abs( iWidth );
  newSect.m_iHeight   = abs( iHeight );
  newSect.m_Flags     = dwFlags;

  if ( pTexture )
  {
    newSect.m_fTU[0] = ( (float)iX ) / (float)pTexture->m_vSize.x;
    newSect.m_fTV[0] = ( (float)iY ) / (float)pTexture->m_vSize.y;
    newSect.m_fTU[1] = ( (float)iX + iWidth ) / (float)pTexture->m_vSize.x;
    newSect.m_fTV[1] = ( (float)iY + iHeight ) / (float)pTexture->m_vSize.y;
  }

}



void CXJump::PreloadImages( CD3DViewer& Viewer )
{

  size_t   iTileSets = m_vectInfoTileSets.size();

  std::vector<CDX8Texture*>   vectTS;

  for ( size_t i = 0; i < iTileSets; ++i )
  {
    GR::String&    strTS = m_vectInfoTileSets[i];

    GR::String     strFullPath = CPath::Normalize( CPath::Append( m_strDataPath, strTS ) );
    vectTS.push_back( Viewer.LoadTexture( strFullPath.c_str(), DX8::TF_HOLES, 0xffff00ff ) );
  }

  // Tiles erzeugen
  CXJump::tVectInfoTiles::iterator    it( m_vectInfoTiles.begin() );
  while ( it != m_vectInfoTiles.end() )
  {
    CXJump::tTileInfo&    TileInfo = *it;

    AddSection( vectTS[TileInfo.iTileSetIndex],
                TileInfo.iTileX, TileInfo.iTileY,
                TileInfo.iTileWidth, TileInfo.iTileHeight,
                TileInfo.dwFlags );

    ++it;
  }

  // ColissionTilesets laden
  for ( size_t iCT = 0; iCT < m_vectInfoColissionTileSets.size(); ++iCT )
  {
    m_vectColissionTileSets.push_back( new GR::CImage( CPath::Normalize( CPath::Append( m_strDataPath, m_vectInfoColissionTileSets[iCT] ) ).c_str() ) );
  }

  // Colission-Tiles erzeugen
  for ( size_t iCTile = 0; iCTile < m_vectInfoColissionTiles.size(); ++iCTile )
  {
    CXJump::tTileInfo&  TileInfo = m_vectInfoColissionTiles[iCTile];

    GR::CImage*    pImageTS = m_vectColissionTileSets[TileInfo.iTileSetIndex & 0x7fff];

    CContextDescriptor    cdTS( pImageTS );

    CContextDescriptor    cdTile;

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



void CXJump::Run()
{

  m_GameVars.clear();
  m_LuaHandler.DoString( m_strOnLoadScript.c_str() );

}



void CXJump::StartScriptAsMovie( const GR::String& strScript )
{

  GR::String     strCompleteScript = "function TempTalkMovie()\n" + strScript + "\nend";

  m_LuaHandler.DoString( strCompleteScript.c_str() );
  m_LuaHandler.StartThread( "Talk", "TempTalkMovie" );

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



void CXJump::AddTileToCategory( const eTileCategory eCategory, const GR::u32 dwTile )
{

  m_mapTileCategories[eCategory].insert( dwTile );

}



void CXJump::RemoveTileFromCategory( const eTileCategory eCategory, const GR::u32 dwTile )
{

  tMapTileCategories::iterator    itMap( m_mapTileCategories.find( eCategory ) );
  if ( itMap == m_mapTileCategories.end() )
  {
    return;
  }

  tSetTileCategory&    setTiles = itMap->second;

  setTiles.erase( dwTile );

}


bool CXJump::IsTileInCategory( const eTileCategory eCategory, const GR::u32 dwTile ) const
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

  // immer in Laufrichtung ein St¸ck voraus
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
  }

}