// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#include "TileSet.h"

#include <Grafik\ContextDescriptor.h>
#include <Grafik\Image.h>

#include <Misc\Misc.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTileSet::CTileSet()
{

  m_iTileWidth          = 40;
  m_iTileHeight         = 40;
  m_bTransparent        = false;
  m_dwTransparentColor  = 0xffff00ff;

}



CTileSet::~CTileSet()
{

  tVectImages::iterator   it( m_vectImages.begin() );
  while ( it != m_vectImages.end() )
  {
    SafeDelete( *it );

    it++;
  }
  m_vectImages.clear();

}



/*-ReloadImages---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CTileSet::ReloadImages( void )
{

  int                 i;

  char                szTemp[MAX_PATH];

  CImage           *pImage;

  tVectImages::iterator         it( m_vectImages.begin() );
  while ( it != m_vectImages.end() )
  {
    SafeDelete( *it );
    it++;
  }
  m_vectImages.clear();

  if ( ( m_iTileWidth == 0 )
  ||   ( m_iTileHeight == 0 ) )
  {
    return false;
  }

  wsprintf( szTemp, m_strSetFileName.c_str() );
  pImage = new CImage( szTemp );

  if ( pImage )
  {
    CContextDescriptor*   pCDImage = new CContextDescriptor( pImage );

    int iTileCount = ( pImage->GetWidth() / m_iTileWidth ) * ( pImage->GetHeight() / m_iTileHeight );

    int     iX,
            iY;


    iX = 0;
    iY = 0;
    for ( i = 0; i < iTileCount; i++ )
    {
      CImage*    pImageDummy = pCDImage->GetImage( iX,
                                                      iY,
                                                      m_iTileWidth,
                                                      m_iTileHeight );

      if ( m_bTransparent )
      {
        pImageDummy->SetTransparentColor( pCDImage->MapColor( 0 ) );
        pImageDummy->Compress();
      }
      m_vectImages.push_back( pImageDummy );

      iX += m_iTileWidth;
      if ( iX + m_iTileWidth > pImage->GetWidth() )
      {
        iX = 0;
        iY += m_iTileHeight;
      }
    }


    delete pCDImage;
  }
  delete pImage;

  return true;

}


typedef std::map<GR::String,CTileSet>    tMapTileSets;
