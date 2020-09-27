// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#include "AnimatedTileSet.h"

#include <IO\File.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAnimatedTileSet::CAnimatedTileSet()
{

}



CAnimatedTileSet::~CAnimatedTileSet()
{

}



void CAnimatedTileSet::Update( DWORD dwElapsedTicks )
{

  tVectAnimation::iterator    it( m_vectAnimatedTiles.begin() );
  while ( it != m_vectAnimatedTiles.end() )
  {
    tAnimation&   Anim = *it;

    Anim.Update( dwElapsedTicks );

    ++it;
  }


}



CImage* CAnimatedTileSet::GetImage( int iIndex )
{

  if ( iIndex >= m_vectAnimatedTiles.size() )
  {
    return NULL;
  }

  int iFrame = m_vectAnimatedTiles[iIndex].CurrentFrame();
  if ( iFrame >= m_TileSet.m_vectImages.size() )
  {
    return NULL;
  }

  return m_TileSet.m_vectImages[iFrame];

}



bool CAnimatedTileSet::Save( const char* szFileName )
{

  CFile    aFile;

  if ( !aFile.Open( szFileName, FILE_WRITE_NEW ) )
  {
    return false;
  }

  aFile.WriteString( m_TileSet.m_strDescription );
  aFile.WriteString( m_TileSet.m_strSetFileName );
  aFile.WriteLong( m_TileSet.m_iTileWidth );
  aFile.WriteLong( m_TileSet.m_iTileHeight );

  aFile.WriteLong( m_vectAnimatedTiles.size() );

  tVectAnimation::iterator    itAnim( m_vectAnimatedTiles.begin() );
  while ( itAnim != m_vectAnimatedTiles.end() )
  {
    tAnimation&   Anim = *itAnim;

    aFile.WriteLong( Anim.singleAnim.size() );

    tSingleAnimation::iterator    itSingle( Anim.singleAnim.begin() );
    while ( itSingle != Anim.singleAnim.end() )
    {
      tAnimationFrame&  Frame = *itSingle;

      aFile.WriteLong( Frame.dwFrameDauer );
      aFile.WriteLong( Frame.iImageIndex );

      ++itSingle;
    }

    ++itAnim;
  }
  aFile.Close();

  return true;

}



bool CAnimatedTileSet::Load( const char* szFileName )
{

  m_vectAnimatedTiles.clear();

  CFile    aFile;

  if ( !aFile.Open( szFileName, FILE_READ_EXISTING ) )
  {
    return false;
  }

  aFile.ReadString( m_TileSet.m_strDescription );
  aFile.ReadString( m_TileSet.m_strSetFileName );
  m_TileSet.m_iTileWidth = aFile.ReadLong();
  m_TileSet.m_iTileHeight = aFile.ReadLong();

  if ( m_TileSet.m_strDescription.empty() )
  {
    m_TileSet.m_strDescription = m_TileSet.m_strSetFileName;
  }

  int   iAnzahlAnimatedTiles = aFile.ReadLong();
  for ( int iAnim = 0; iAnim < iAnzahlAnimatedTiles; ++iAnim )
  {
    tAnimation  dummy;
    m_vectAnimatedTiles.push_back( dummy );
    tAnimation&   Anim = m_vectAnimatedTiles.back();

    int   iAnzahlFrames = aFile.ReadLong();
    for ( int iFrame = 0; iFrame < iAnzahlFrames; ++iFrame )
    {
      tAnimationFrame   Frame;

      Frame.dwFrameDauer = aFile.ReadLong();
      Frame.iImageIndex = aFile.ReadLong();
      Anim.singleAnim.push_back( Frame );
    }
  }

  aFile.Close();

  m_TileSet.ReloadImages();

  return true;

}
