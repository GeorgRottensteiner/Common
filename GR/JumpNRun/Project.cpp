// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable:4786 )

#include <IO\File.h>

#include <Grafik\Image.h>

#include "Project.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJRProject::CJRProject() :
  m_strProjectPath( "" ),
  m_pCurrentMap( NULL )
{

}



CJRProject::~CJRProject()
{

  m_mapAnimatedTileSets.clear();

}



bool CJRProject::Save( const char* szFileName )
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
  pFile->WriteLong( m_mapAnimatedTileSets.size() );
  tMapAnimatedTileSets::iterator    itTiles( m_mapAnimatedTileSets.begin() );
  while ( itTiles != m_mapAnimatedTileSets.end() )
  {
    const GR::String&    strKey  = itTiles->first;
    CTileSet&       Set     = itTiles->second.m_TileSet;

    pFile->WriteString( strKey );
    pFile->WriteString( Set.m_strDescription );

    // Pfad relativieren
    GR::String   strTemp = Set.m_strSetFileName;
    if ( strcmpi( strTemp.substr( 0, m_strProjectPath.length() ).c_str(), m_strProjectPath.c_str() ) == 0 )
    {
      strTemp = strTemp.substr( m_strProjectPath.length() );
    }
    pFile->WriteString( strTemp );

    pFile->WriteLong( Set.m_iTileWidth );
    pFile->WriteLong( Set.m_iTileHeight );
    pFile->WriteLong( Set.m_bTransparent );
    pFile->WriteLong( Set.m_dwTransparentColor );

    CAnimatedTileSet&   AnimSet = itTiles->second;

    pFile->WriteLong( AnimSet.m_vectAnimatedTiles.size() );

    tVectAnimation::iterator    itAnim( AnimSet.m_vectAnimatedTiles.begin() );
    while ( itAnim != AnimSet.m_vectAnimatedTiles.end() )
    {
      tAnimation&   Anim = *itAnim;

      pFile->WriteLong( Anim.singleAnim.size() );

      tSingleAnimation::iterator    itSingle( Anim.singleAnim.begin() );
      while ( itSingle != Anim.singleAnim.end() )
      {
        tAnimationFrame&  Frame = *itSingle;

        pFile->WriteLong( Frame.dwFrameDauer );
        pFile->WriteLong( Frame.iImageIndex );

        ++itSingle;
      }

      ++itAnim;
    }

    ++itTiles;
  }

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
      pFile->WriteLong( Layer.m_iWidth );
      pFile->WriteLong( Layer.m_iHeight );
      pFile->WriteLong( Layer.m_wBytesPerField );
      pFile->WriteLong( Layer.m_wType );
      pFile->WriteLong( Layer.m_iRelativScrollSpeedX );
      pFile->WriteLong( Layer.m_iRelativScrollSpeedY );

      pFile->WriteString( Layer.m_strDesc );
      pFile->WriteString( Layer.m_strTileSet );

      for ( dwI = 0; dwI < Layer.m_iWidth; dwI++ )
      {
        for ( dwJ = 0; dwJ < Layer.m_iHeight; dwJ++ )
        {
          switch ( Layer.m_wBytesPerField )
          {
              case 1:
                pFile->WriteByte( Layer.GetFeld( dwI, dwJ ) );
                break;
              case 2:
                pFile->WriteWord( Layer.GetFeld( dwI, dwJ ) );
                break;
              case 4:
                pFile->WriteLong( Layer.GetFeld( dwI, dwJ ) );
                break;
          }
        }
      }

      itLayer++;
    }

    itMap++;
  }

  pFile->Close();
  delete pFile;

  return true;

}



bool CJRProject::Load( const char* szFileName )
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

  int   iAnzahlTileSets = pFile->ReadLong();
  for ( int k = 0; k < iAnzahlTileSets; ++k )
  {
    GR::String   strKey,
                  strDescription;

    pFile->ReadString( strKey );

    CTileSet&   newSet = m_mapAnimatedTileSets[strKey].m_TileSet;

    pFile->ReadString( newSet.m_strDescription );
    pFile->ReadString( newSet.m_strSetFileName );

    // Pfad absolut machen
    newSet.m_strSetFileName = m_strProjectPath + newSet.m_strSetFileName;

    newSet.m_iTileWidth = pFile->ReadLong();
    newSet.m_iTileHeight = pFile->ReadLong();
    if ( pFile->ReadLong() )
    {
      newSet.m_bTransparent = true;
    }
    else
    {
      newSet.m_bTransparent = false;
    }
    newSet.m_dwTransparentColor = pFile->ReadLong();

    if ( newSet.m_strDescription.empty() )
    {
      newSet.m_strDescription = newSet.m_strSetFileName;
    }

    CAnimatedTileSet&   AnimSet = m_mapAnimatedTileSets[strKey];

    int   iAnzahlAnimatedTiles = pFile->ReadLong();
    for ( int iAnim = 0; iAnim < iAnzahlAnimatedTiles; ++iAnim )
    {
      tAnimation  dummy;
      AnimSet.m_vectAnimatedTiles.push_back( dummy );
      tAnimation&   Anim = AnimSet.m_vectAnimatedTiles.back();

      int   iAnzahlFrames = pFile->ReadLong();
      for ( int iFrame = 0; iFrame < iAnzahlFrames; ++iFrame )
      {
        tAnimationFrame   Frame;

        Frame.dwFrameDauer = pFile->ReadLong();
        Frame.iImageIndex = pFile->ReadLong();
        Anim.singleAnim.push_back( Frame );
      }
    }

    newSet.ReloadImages();
  }


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
      Layer.m_iWidth          = pFile->ReadLong();
      Layer.m_iHeight         = pFile->ReadLong();
      Layer.m_wBytesPerField  = pFile->ReadLong();
      Layer.m_wType           = pFile->ReadLong();
      Layer.m_iRelativScrollSpeedX  = pFile->ReadLong();
      Layer.m_iRelativScrollSpeedY  = pFile->ReadLong();

      pFile->ReadString( Layer.m_strDesc );
      pFile->ReadString( Layer.m_strTileSet );

      Layer.Create( Layer.m_iWidth, Layer.m_iHeight, Layer.m_wBytesPerField, Layer.m_strDesc.c_str() );

      for ( int dwI = 0; dwI < Layer.m_iWidth; dwI++ )
      {
        for ( int dwJ = 0; dwJ < Layer.m_iHeight; dwJ++ )
        {
          switch ( Layer.m_wBytesPerField )
          {
              case 1:
                Layer.SetFeld( dwI, dwJ, pFile->ReadByte() );
                break;
              case 2:
                Layer.SetFeld( dwI, dwJ, pFile->ReadWord() );
                break;
              case 4:
                Layer.SetFeld( dwI, dwJ, pFile->ReadLong() );
                break;
          }
        }
      }

      tMapAnimatedTileSets::iterator    it( m_mapAnimatedTileSets.begin() );
      while ( it != m_mapAnimatedTileSets.end() )
      {
        CAnimatedTileSet&   AnimSet = it->second;

        if ( AnimSet.m_TileSet.m_strDescription == Layer.m_strTileSet )
        {
          Layer.m_pTileSet = &AnimSet;
          break;
        }

        ++it;
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

bool CJRProject::SetCurrentMap( const char* szName )
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



/*-BlockedAt------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRProject::BlockedAt( int iX, int iY )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }
  tListLayer::iterator    it( m_pCurrentMap->m_listLayers.begin() );
  while ( it != m_pCurrentMap->m_listLayers.end() )
  {
    CLevelLayer& Layer = *it;

    if ( Layer.m_wType == JNR::LAYER::TYPE::COLLISION )
    {
      CImage*  pImage = Layer.m_pTileSet->GetImage(
            Layer.GetFeld( iX / Layer.m_pTileSet->m_TileSet.m_iTileWidth,
                           iY / Layer.m_pTileSet->m_TileSet.m_iTileHeight ) );
      if ( pImage == NULL )
      {
        return false;
      }
      WORD*   pData = (WORD*)pImage->GetData();
      pData += ( iX % Layer.m_pTileSet->m_TileSet.m_iTileWidth ) +  ( iY % Layer.m_pTileSet->m_TileSet.m_iTileHeight ) * Layer.m_pTileSet->m_TileSet.m_iTileWidth;
      if ( *pData )
      {
        return true;
      }
      return false;
    }
    ++it;
  }

  return false;

}



/*-Display--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRProject::Display( CGfxPage *pPage, DWORD dwElapsedTicks )
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

    int   iTileX = -iOffsetX / Layer.m_pTileSet->m_TileSet.m_iTileWidth;
    int   iTileY = -iOffsetY / Layer.m_pTileSet->m_TileSet.m_iTileHeight;

    if ( ( Layer.m_wType == JNR::LAYER::TYPE::BLOCKS )
    ||   ( ( Layer.m_wType == JNR::LAYER::TYPE::COLLISION )
    &&     ( GetVarI( "ShowCollisionLayer" ) ) ) )
    {
      int   iX1 = iTileX,
            iY1 = iTileY,
            iX2 = iTileX + pPage->GetWidth() / Layer.m_pTileSet->m_TileSet.m_iTileWidth + 1,
            iY2 = iTileY + pPage->GetHeight() / Layer.m_pTileSet->m_TileSet.m_iTileHeight + 1;

      CImage*  pImage;

      for ( int i = iX1; i <= iX2; ++i )
      {
        for ( int j = iY1; j <= iY2; ++j )
        {
          pImage = Layer.m_pTileSet->GetImage( Layer.GetFeld( i, j ) );
          if ( pImage )
          {
            pImage->PutImage(
                      pPage,
                      iOffsetX + i * Layer.m_pTileSet->m_TileSet.m_iTileWidth,
                      iOffsetY + j * Layer.m_pTileSet->m_TileSet.m_iTileHeight );
          }

        }
      }
    }

    ++it;
  }

  return true;

}



/*-ScrollTo-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CJRProject::ScrollTo( float fX, float fY )
{

  if ( m_pCurrentMap == NULL )
  {
    return false;
  }

  SetVarF( "LevelOffsetX", fX );
  SetVarF( "LevelOffsetY", fY );

  return true;

}
