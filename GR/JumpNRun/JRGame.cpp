// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable:4786 )

#include <IO\File.h>
#include <Misc\Misc.h>

#include <Grafik\Image.h>

#include <Debug\debugclient.h>

#include "JRGame.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJRGame::CJRGame() :
  m_strProjectPath( "" ),
  m_pCurrentMap( NULL )
{

  CollisionType( 0, 0 );

}



CJRGame::~CJRGame()
{

}



bool CJRGame::Save( const char* szFileName )
{

  CFile      *pFile;

  char          szTemp[MAX_PATH];

  DWORD         dwI,
                dwJ;


  wsprintf( szTemp, szFileName );
  pFile = new CFile();
  pFile->Open( szTemp, FILE_WRITE_NEW );
  if ( !pFile->Check() )
  {
    delete pFile;
    return false;
  }

  pFile->WriteLong( 0xabcd );   // Magic Number
  pFile->WriteLong( 0 );        // für später

  pFile->WriteString( m_strProjectPath );

  pFile->WriteLong( m_mapTileSets.size() );

  typedef std::pair<int,int>                        tPairTile;    // Tileset/Index

  typedef std::map<CImage*,tPairTile>            tMapImages;

  tMapImages                  mapImages;


  int                         iNrTileSet = 0;

  tMapTileSets::iterator    itTileSets( m_mapTileSets.begin() );
  while ( itTileSets != m_mapTileSets.end() )
  {
    pFile->WriteString( itTileSets->first );

    CTileSet&   TileSet = itTileSets->second;

    pFile->WriteString( TileSet.m_strDescription );

    GR::String   strTemp = TileSet.m_strSetFileName;
    if ( strcmpi( strTemp.substr( 0, m_strProjectPath.length() ).c_str(), m_strProjectPath.c_str() ) == 0 )
    {
      strTemp = strTemp.substr( m_strProjectPath.length() );
    }
    pFile->WriteString( strTemp );

    pFile->WriteLong( TileSet.m_iTileWidth );
    pFile->WriteLong( TileSet.m_iTileHeight );
    pFile->WriteLong( TileSet.m_bTransparent );
    pFile->WriteLong( TileSet.m_dwTransparentColor );

    // alle Images in eine Map
    int iIndex = 0;
    tVectImages::iterator   itImages( TileSet.m_vectImages.begin() );
    while ( itImages != TileSet.m_vectImages.end() )
    {
      mapImages[*itImages].first = iNrTileSet;
      mapImages[*itImages].second = iIndex;

      ++iIndex;
      ++itImages;
    }

    ++iNrTileSet;
    ++itTileSets;
  }

  // erst alle enthaltenen Images mappen

  pFile->WriteLong( m_AnimationList.m_vectAnimations.size() );
  tVectAnimation::iterator    itAnimation( m_AnimationList.m_vectAnimations.begin() );
  while ( itAnimation != m_AnimationList.m_vectAnimations.end() )
  {
    tAnimation&   Animation = *itAnimation;

    pFile->WriteLong( Animation.singleAnim.size() );

    tSingleAnimation& SingleAnim = Animation.singleAnim;

    tSingleAnimation::iterator    itSAnim( SingleAnim.begin() );
    while ( itSAnim != SingleAnim.end() )
    {
      tAnimationFrame&    Frame = *itSAnim;

      tMapImages::iterator    itImage( mapImages.find( Frame.pImage ) );
      if ( itImage == mapImages.end() )
      {
        pFile->WriteLong( -1 );
        pFile->WriteLong( -1 );
      }
      else
      {
        tPairTile&    Pair = itImage->second;

        pFile->WriteLong( Pair.first );
        pFile->WriteLong( Pair.second );
      }
      pFile->WriteLong( Frame.dwFrameDauer );

      ++itSAnim;
    }

    ++itAnimation;
  }

  // bekannte Objekt-Typen
  pFile->WriteLong( m_knownObjects.size() );

  tMapJRObjects::iterator    itObjList( m_knownObjects.begin() );
  while ( itObjList != m_knownObjects.end() )
  {
    CJRObject&    obj = itObjList->second;

    pFile->WriteString( obj.GetVar( "Name" ) );
    pFile->WriteLong( obj.m_dwAnimation );

    ++itObjList;
  }

  // Maps
  pFile->WriteLong( m_listMaps.size() );

  tListMaps::iterator   itMap( m_listMaps.begin() );
  while ( itMap != m_listMaps.end() )
  {
    CLevelMap&  aMap = *itMap;

    pFile->WriteLong( aMap.m_listLayers.size() );
    pFile->WriteLong( 0 );  // Reserved
    pFile->WriteString( aMap.m_strDescription );

    tListLayer::iterator    itLayer( aMap.m_listLayers.begin() );
    while ( itLayer != aMap.m_listLayers.end() )
    {
      CLevelLayer&  Layer = *itLayer;
      pFile->WriteLong( Layer.Width() );
      pFile->WriteLong( Layer.Height() );
      pFile->WriteLong( 2 );
      pFile->WriteLong( Layer.m_wType );
      pFile->WriteLong( Layer.m_iRelativScrollSpeedX );
      pFile->WriteLong( Layer.m_iRelativScrollSpeedY );
      pFile->WriteLong( Layer.m_iTileWidth );
      pFile->WriteLong( Layer.m_iTileHeight );

      pFile->WriteLong( 4 );   // Extra-Data-Length
      if ( &Layer == aMap.m_pMainLayer )
      {
        pFile->WriteLong( 1 );
      }
      else
      {
        pFile->WriteLong( 0 );
      }

      pFile->WriteString( Layer.m_strDesc );

      for ( dwI = 0; dwI < Layer.Width(); dwI++ )
      {
        for ( dwJ = 0; dwJ < Layer.Height(); dwJ++ )
        {
          pFile->WriteWord( Layer.GetFeld( dwI, dwJ ) );
        }
      }

      itLayer++;
    }

    // Objekte
    pFile->WriteLong( aMap.m_mapObjects.size() );

    tMapJRObjectList::iterator    itObjList( aMap.m_mapObjects.begin() );
    while ( itObjList != aMap.m_mapObjects.end() )
    {
      const GR::String&    strKey = itObjList->first;

      pFile->WriteString( strKey );

      tListJRObjects&   List = itObjList->second;

      pFile->WriteLong( List.size() );
      tListJRObjects::iterator    itObj( List.begin() );
      while ( itObj != List.end() )
      {
        CJRObject&    Obj = *itObj;

        pFile->WriteLong( Obj.m_dwAnimation );
        pFile->WriteLong( Obj.GetVarI( "x" ) );
        pFile->WriteLong( Obj.GetVarI( "y" ) );

        pFile->WriteString( Obj.GetVar( "Name" ) );

        ++itObj;
      }

      ++itObjList;
    }

    itMap++;
  }

  pFile->Close();
  delete pFile;

  return true;

}



bool CJRGame::Load( const char* szFileName )
{

  m_listMaps.clear();

  CFile*   pFile;

  pFile = new CFile();
  pFile->Open( szFileName, FILE_READ_EXISTING );
  if ( !pFile->Check() )
  {
    delete pFile;
    return false;
  }

  if ( pFile->ReadLong() != 0xabcd )
  {
    pFile->Close();
    delete pFile;
    return false;
  }

  pFile->ReadLong(); // reserved für später

  pFile->ReadString( m_strProjectPath );


  std::vector<CTileSet*>      vectTileSets;


  int                         iAnzahlTileSets = pFile->ReadLong();

  m_mapTileSets.clear();
  for ( int iTileSet = 0; iTileSet < iAnzahlTileSets; ++iTileSet )
  {
    GR::String   strKey;

    pFile->ReadString( strKey );

    CTileSet&   TileSet = m_mapTileSets[strKey];

    pFile->ReadString( TileSet.m_strDescription );

    pFile->ReadString( TileSet.m_strSetFileName );

    // Pfad absolut machen
    TileSet.m_strSetFileName = m_strProjectPath + TileSet.m_strSetFileName;

    TileSet.m_iTileWidth          = pFile->ReadLong();
    TileSet.m_iTileHeight         = pFile->ReadLong();
    if ( pFile->ReadLong() )
    {
      TileSet.m_bTransparent = true;
    }
    else
    {
      TileSet.m_bTransparent = false;
    }
    TileSet.m_dwTransparentColor  = pFile->ReadLong();

    TileSet.ReloadImages();

    vectTileSets.push_back( &TileSet );
  }

  // Animationen
  int                         iAnzahlAnimationen = pFile->ReadLong();

  m_AnimationList.m_vectAnimations.clear();
  for ( int iAnim = 0; iAnim < iAnzahlAnimationen; ++iAnim )
  {
    tAnimation  dummy;

    m_AnimationList.m_vectAnimations.push_back( dummy );

    tAnimation&   Animation = m_AnimationList.m_vectAnimations.back();

    int   iAnzahlFrames = pFile->ReadLong();

    tSingleAnimation& SingleAnim = Animation.singleAnim;

    for ( int iFrame = 0; iFrame < iAnzahlFrames; ++iFrame )
    {
      tAnimationFrame    Frame;

      int   iTileSet  = pFile->ReadLong();
      int   iIndex    = pFile->ReadLong();

      if ( ( iTileSet != -1 )
      &&   ( iIndex != -1 )
      &&   ( iTileSet < vectTileSets.size() ) )
      {
        if ( iIndex < vectTileSets[iTileSet]->m_vectImages.size() )
        {
          Frame.pImage = vectTileSets[iTileSet]->m_vectImages[iIndex];
        }
        else
        {
          dh::Log( "No Image - invalid index\n" );
        }
      }
      else
      {
        dh::Log( printf( "No Image - invalid tileset %d %d\n", iTileSet, iIndex ) );
      }

      Frame.dwFrameDauer = pFile->ReadLong();

      SingleAnim.push_back( Frame );
    }
  }

  // bekannte Objekt-Typen
  int   iAnzahlKnownObjects = pFile->ReadLong();

  m_knownObjects.clear();
  for ( int iKO = 0; iKO < iAnzahlKnownObjects; ++iKO )
  {
    GR::String   strName;
    pFile->ReadString( strName );


    CJRObject&    newObj = m_knownObjects[strName];

    newObj.SetVar( "Name", strName );
    newObj.m_dwAnimation = pFile->ReadLong();
  }

  // Maps
  int   iAnzahlMaps = pFile->ReadLong();

  for ( int i = 0; i < iAnzahlMaps; i++ )
  {
    CLevelMap   dummy;
    m_listMaps.push_back( dummy );
    CLevelMap&  aMap = m_listMaps.back();

    int   iAnzahlLayer = pFile->ReadLong();

    pFile->ReadLong();  // Reserved
    pFile->ReadString( aMap.m_strDescription );

    for ( int j = 0; j < iAnzahlLayer; ++j )
    {
      CLevelLayer   newLayer;
      aMap.m_listLayers.push_back( newLayer );

      CLevelLayer&  Layer = aMap.m_listLayers.back();
      WORD      wWidth        = pFile->ReadLong();
      WORD      wHeight       = pFile->ReadLong();
      pFile->ReadLong();
      Layer.m_wType           = pFile->ReadLong();
      Layer.m_iRelativScrollSpeedX  = pFile->ReadLong();
      Layer.m_iRelativScrollSpeedY  = pFile->ReadLong();
      Layer.m_iTileWidth            = pFile->ReadLong();
      Layer.m_iTileHeight           = pFile->ReadLong();

      DWORD   dwExtraDataLength = pFile->ReadLong();

      if ( dwExtraDataLength )
      {
        dwExtraDataLength -= 4;
        DWORD   dwLayerFlags = pFile->ReadLong();

        if ( dwLayerFlags & 1 )
        {
          aMap.m_pMainLayer = &Layer;
        }
      }

      pFile->ReadString( Layer.m_strDesc );

      Layer.Create( wWidth, wHeight, Layer.m_strDesc.c_str() );

      for ( int dwI = 0; dwI < Layer.Width(); dwI++ )
      {
        for ( int dwJ = 0; dwJ < Layer.Height(); dwJ++ )
        {
          Layer.SetFeld( dwI, dwJ, pFile->ReadWord() );
        }
      }
      Layer.m_pAnimationList = &m_AnimationList;
    }

    int   iAnzahlObjektLayer = pFile->ReadLong();

    for ( int l = 0; l < iAnzahlObjektLayer; ++l )
    {
      // Objekte
      GR::String   strKey = "";
      pFile->ReadString( strKey );

      tListJRObjects   List;

      int   iAnzahlObjekte = pFile->ReadLong();


      for ( int m = 0; m < iAnzahlObjekte; ++m )
      {
        CJRObject   dummy;
        List.push_back( dummy );

        CJRObject&    Obj = List.back();

        Obj.m_dwAnimation = pFile->ReadLong();
        Obj.SetVarI( "x", (int)pFile->ReadLong() );
        Obj.SetVarI( "y", (int)pFile->ReadLong() );

        GR::String   strDummy;
        pFile->ReadString( strDummy );
        Obj.SetVar( "Name", strDummy );
      }

      bool    bLayerExists = false;
      tListLayer::iterator    itLayer( aMap.m_listLayers.begin() );
      while ( itLayer != aMap.m_listLayers.end() )
      {
        CLevelLayer&    Layer = *itLayer;

        GR::String&   strName = Layer.m_strDesc;

        if ( strName == strKey )
        {
          bLayerExists = true;
          break;
        }

        ++itLayer;
      }
      if ( bLayerExists )
      {
        aMap.m_mapObjects[strKey] = List;
      }
      else
      {
        dh::Log( CMisc::printf( "%d Objekte loaded in Map %s - disposed\n", iAnzahlObjekte, strKey.c_str() ) );
      }
    }

  }

  pFile->Close();
  delete pFile;

  return true;

}



/*-SetCurrentMap--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRGame::SetCurrentMap( const char* szName )
{

  tListMaps::iterator   it( m_listMaps.begin() );
  while ( it != m_listMaps.end() )
  {
    CLevelMap&  aMap = *it;

    if ( aMap.m_strDescription.compare( szName ) == 0 )
    {
      m_pCurrentMap = &aMap;
      return true;
    }

    ++it;
  }
  m_pCurrentMap = NULL;
  return false;

}



/*-SetMainLayer---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRGame::SetMainLayer( const char* szName )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }
  tListLayer::iterator    it( m_pCurrentMap->m_listLayers.begin() );
  while ( it != m_pCurrentMap->m_listLayers.end() )
  {
    CLevelLayer&  Layer = *it;

    if ( Layer.m_strDesc.compare( szName ) == 0 )
    {
      m_pCurrentMap->m_pMainLayer = &Layer;
      return true;
    }

    ++it;
  }
  m_pCurrentMap->m_pMainLayer = NULL;
  return false;

}



/*-BlockedAt------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRGame::BlockedAt( int iX, int iY )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }
  tListLayer::iterator    it( m_pCurrentMap->m_listLayers.begin() );
  while ( it != m_pCurrentMap->m_listLayers.end() )
  {
    CLevelLayer& Layer = *it;

    if ( Layer.m_wType == LT_COLLISION )
    {
      CImage*  pImage = m_AnimationList.GetImage(
            Layer.GetFeld( iX / Layer.m_iTileWidth,
                           iY / Layer.m_iTileHeight ) );
      if ( pImage == NULL )
      {
        return false;
      }
      WORD*   pData = (WORD*)pImage->GetData();
      pData += ( iX % Layer.m_iTileWidth ) + ( iY % Layer.m_iTileHeight ) * Layer.m_iTileWidth;
      if ( *pData == m_mapCollisionTypesReverse[1] )
      {
        return true;
      }
      return false;
    }
    ++it;
  }

  return false;

}



/*-CollideValueAt-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

DWORD CJRGame::CollideValueAt( int iX, int iY )
{

  if ( m_pCurrentMap == NULL )
  {
    return 0;
  }
  tListLayer::iterator    it( m_pCurrentMap->m_listLayers.begin() );
  while ( it != m_pCurrentMap->m_listLayers.end() )
  {
    CLevelLayer& Layer = *it;

    if ( Layer.m_wType == LT_COLLISION )
    {
      CImage*  pImage = m_AnimationList.GetImage(
            Layer.GetFeld( iX / Layer.m_iTileWidth,
                           iY / Layer.m_iTileHeight ) );
      if ( pImage == NULL )
      {
        return 0;
      }
      WORD*   pData = (WORD*)pImage->GetData();
      pData += ( iX % Layer.m_iTileWidth ) + ( iY % Layer.m_iTileHeight ) * Layer.m_iTileWidth;
      return *pData;
    }
    ++it;
  }

  return 0;

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRGame::Display( CGfxPage *pPage, DWORD dwElapsedTicks )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }

  int   iOffsetX = (int)GetVarF( "LevelOffsetX" );
  int   iOffsetY = (int)GetVarF( "LevelOffsetY" );

  tListLayer::iterator    it( m_pCurrentMap->m_listLayers.begin() );
  while ( it != m_pCurrentMap->m_listLayers.end() )
  {
    CLevelLayer& Layer = *it;

    int   iTileX = iOffsetX / Layer.m_iTileWidth;
    int   iTileY = iOffsetY / Layer.m_iTileHeight;

    if ( ( Layer.m_wType == LT_BLOCKS )
    ||   ( ( Layer.m_wType == LT_COLLISION )
    &&     ( GetVarI( "ShowCollisionLayer" ) ) ) )
    {
      int   iX1 = iTileX,
            iY1 = iTileY,
            iX2 = iTileX + pPage->GetWidth() / Layer.m_iTileWidth + 1,
            iY2 = iTileY + pPage->GetHeight() / Layer.m_iTileHeight + 1;

      CImage*  pImage;

      for ( int i = iX1; i <= iX2; ++i )
      {
        for ( int j = iY1; j <= iY2; ++j )
        {
          pImage = m_AnimationList.GetImage( Layer.GetFeld( i, j ) );
          if ( pImage )
          {
            pImage->PutImage(
                      pPage,
                      i * Layer.m_iTileWidth - iOffsetX,
                      j * Layer.m_iTileHeight - iOffsetY );
          }

        }
      }
    }

    // Objekte
    tMapJRObjectList::iterator    itObj( m_pCurrentMap->m_mapObjects.find( Layer.m_strDesc ) );
    if ( itObj != m_pCurrentMap->m_mapObjects.end() )
    {
      tListJRObjects&   List = itObj->second;

      tListJRObjects::iterator    itList( List.begin() );
      while ( itList != List.end() )
      {
        CJRObject&    obj = *itList;

        CImage*    pImage = m_AnimationList.GetImage( obj.m_dwAnimation );
        if ( pImage )
        {
          pImage->PutImage( pPage,
                            obj.GetVarF( "x" ) - pImage->GetWidth() / 2 - iOffsetX,
                            obj.GetVarF( "y" ) - pImage->GetHeight() + 1 - iOffsetY );
        }

        ++itList;
      }
    }


    ++it;
  }

  return true;

}



/*-ScrollTo-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRGame::ScrollTo( float fX, float fY )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }
  if ( m_pCurrentMap->m_pMainLayer == NULL )
  {
    return false;
  }

  // BAUSTELLE - Scroll-Regions
  if ( fX < 0 )
  {
    fX = 0;
  }
  if ( fY < 0 )
  {
    fY = 0;
  }
  if ( fX + GetVarI( "VisibleWidth" ) > m_pCurrentMap->m_pMainLayer->Width() * m_pCurrentMap->m_pMainLayer->m_iTileWidth )
  {
    fX = m_pCurrentMap->m_pMainLayer->Width() * m_pCurrentMap->m_pMainLayer->m_iTileWidth - GetVarI( "VisibleWidth" );
  }
  if ( fY + GetVarI( "VisibleHeight" ) > m_pCurrentMap->m_pMainLayer->Height() * m_pCurrentMap->m_pMainLayer->m_iTileHeight )
  {
    fY = m_pCurrentMap->m_pMainLayer->Height() * m_pCurrentMap->m_pMainLayer->m_iTileHeight - GetVarI( "VisibleHeight" );
  }

  SetVarF( "LevelOffsetX", fX );
  SetVarF( "LevelOffsetY", fY );

  //dh::Log( printf( "ScrollTo %d,%d\n", GetVarI( "LevelOffsetX" ), GetVarI( "LevelOffsetY" ) ) );

  return true;

}



/*-CollisionType--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CJRGame::CollisionType( DWORD dwUserCollisionType, DWORD dwColorValue )
{

  m_mapCollisionTypes[dwColorValue] = dwUserCollisionType;
  m_mapCollisionTypesReverse[dwUserCollisionType] = dwColorValue;

}



/*-GetCollisionTypeAt---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

DWORD CJRGame::GetCollisionTypeAt( int iX, int iY )
{

  return m_mapCollisionTypes[CollideValueAt( iX, iY )];

}
