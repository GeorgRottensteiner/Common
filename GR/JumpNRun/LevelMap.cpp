// TileSet.cpp: implementation of the CTileSet class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning( disable:4786 )


#include <IO\File.h>

#include <GR\JumpNRun\LevelMap.h>



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CLevelLayer::CLevelLayer() :
    C2dLayer<WORD>(),
    m_dwUserData( 0 ),
    m_iRelativScrollSpeedX( 100 ),
    m_iRelativScrollSpeedY( 100 ),
    m_iTileWidth( 32 ),
    m_iTileHeight( 32 ),
    m_wType( LT_INVALID ),
    m_pAnimationList( NULL )
{

}



CLevelLayer::~CLevelLayer()
{

}



/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CLevelLayer::Create( int iWidth, int iHeight, const char* szDesc )
{

  C2dLayer<WORD>::InitFeld( iWidth, iHeight, szDesc );

  return true;

}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLevelMap::CLevelMap() :
      m_strFileName( "" ),
      m_strDescription( "" ),
      m_dwUserData( 0 ),
      m_iScrollX( 0 ),
      m_iScrollY( 0 ),
      m_pMainLayer( NULL )
{

  m_listLayers.clear();

}




CLevelMap::~CLevelMap()
{

  m_listLayers.clear();

}



/*-LoadMap--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CLevelMap::Load( const char* szFileName )
{

  CFile      aFile;

  m_listLayers.clear();
  if ( !aFile.Open( szFileName, FILE_READ_EXISTING ) )
  {
    return false;
  }

  int   iAnzahlLayer = aFile.ReadLong();

  int   iReserved = aFile.ReadLong();

  aFile.ReadString( m_strDescription );

  for ( int i = 0; i < iAnzahlLayer; i++ )
  {
    CLevelLayer     dummy;

    m_listLayers.push_back( dummy );

    CLevelLayer&    Layer = m_listLayers.back();



    WORD wWidth = aFile.ReadLong();
    WORD wHeight = aFile.ReadLong();
    aFile.ReadLong();
    Layer.m_wType = aFile.ReadLong();
    Layer.m_iTileWidth = aFile.ReadLong();
    Layer.m_iTileHeight = aFile.ReadLong();

    DWORD   dwExtraDataLength = aFile.ReadLong();

    if ( dwExtraDataLength )
    {
      dwExtraDataLength -= 4;
      DWORD   dwLayerFlags = aFile.ReadLong();

      if ( dwLayerFlags & 1 )
      {
        m_pMainLayer = &Layer;
      }
    }

    aFile.ReadString( Layer.m_strDesc );


    Layer.Create( wWidth, wHeight, Layer.m_strDesc.c_str() );

    aFile.ReadBlock( Layer.m_pData, Layer.Width() * Layer.Height() * 2 );
  }

  aFile.Close();

  return true;

}



bool CLevelMap::Save( const char* szFileName )
{

  CFile      aFile;

  if ( !aFile.Open( szFileName, FILE_WRITE_NEW ) )
  {
    return false;
  }

  aFile.WriteLong( m_listLayers.size() );
  aFile.WriteLong( 0 );
  aFile.WriteString( m_strDescription );

  tListLayer::iterator    it( m_listLayers.begin() );
  while ( it != m_listLayers.end() )
  {
    CLevelLayer&   Layer = *it;

    aFile.WriteLong( Layer.Width() );
    aFile.WriteLong( Layer.Height() );
    aFile.WriteLong( 2 );
    aFile.WriteLong( Layer.m_wType );
    aFile.WriteLong( Layer.m_iTileWidth );
    aFile.WriteLong( Layer.m_iTileHeight );
    aFile.WriteLong( 4 );   // Extra-Data-Length
    if ( &Layer == m_pMainLayer )
    {
      aFile.WriteLong( 1 );
    }
    else
    {
      aFile.WriteLong( 0 );
    }

    aFile.WriteString( Layer.m_strDesc );

    aFile.WriteBlock( Layer.m_pData, Layer.Width() * Layer.Height() * 2 );
    ++it;
  }

  aFile.Close();

  return true;

}