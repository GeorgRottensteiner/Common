#include <IO\FileStream.h>

#include <GR/XJumpX/LevelMap.h>
#include <GR/XJumpX/XJump.h>

#include <debug/debugclient.h>



CLevelMap::CLevelMap() :
      m_strFileName( "" ),
      m_strDescription( "" ),
      m_dwUserData( 0 ),
      m_vectScrollOffset( 0, 0, 0 ),
      m_pMainLayer( NULL )
{
        
  m_listLayers.clear();

}




CLevelMap::~CLevelMap()
{

  m_listLayers.clear();

}



bool CLevelMap::Load( const char* szFileName )
{

  GR::IO::FileStream   File;

  m_listLayers.clear();
  if ( !File.Open( szFileName ) )
  {
    return false;
  }

  Load( File );

  File.Close();
  
  return true;

}



bool CLevelMap::Save( const char* szFileName )
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



void CLevelMap::Save( IIOStream& File )
{

  File.WriteU32( (GR::u32)m_listLayers.size() );

  // Extra-Data-Size
  File.WriteU32( (GR::u32)m_mapExtraData.size() );
  tMapExtraData::iterator   itED( m_mapExtraData.begin() );
  while ( itED != m_mapExtraData.end() )
  {
    File.WriteU32( itED->first );

    tExtraData&   ExtraData = itED->second;

    ExtraData.Save( File );

    ++itED;
  }

  // Extra-Data-Size
  File.WriteString( m_Script );

  // .. Extra-Data
  File.WriteString( m_strDescription );

  tListLayer::iterator    it( m_listLayers.begin() );
  while ( it != m_listLayers.end() )
  {
    CLevelLayer&   Layer = *it;

    Layer.Save( File );

    ++it;
  }

  File.WriteU32( m_Flags );

}



void CLevelMap::Load( IIOStream& File )
{

  int   iAnzahlLayer = File.ReadU32();

  // Extra-Data-Size
  int   iExtraDataSize = File.ReadU32();
  m_mapExtraData.clear();
  for ( int iED = 0; iED < iExtraDataSize; ++iED )
  {
    GR::u32   dwKey = File.ReadU32();

    if ( dwKey != 0 )
    {
      m_mapExtraData[dwKey].Load( File );
    }
    else
    {
      tExtraData    Data;

      Data.Load( File );
    }
  }

  m_Script = File.ReadString();

  File.ReadString( m_strDescription );

  for ( int i = 0; i < iAnzahlLayer; i++ )
  {
    CLevelLayer     dummy;

    m_listLayers.push_back( dummy );

    CLevelLayer&    Layer = m_listLayers.back();

    Layer.Load( File );

    if ( Layer.m_dwFlags & CLevelLayer::LF_MAIN_LAYER )
    {
      m_pMainLayer = &Layer;
    }
  }

  m_Flags = File.ReadU32();

}



void CLevelMap::Clear()
{

  m_listLayers.clear();
  m_mapExtraData.clear();
  m_pMainLayer = NULL;

}



CLevelLayer& CLevelMap::AddLayer()
{

  m_listLayers.push_back( CLevelLayer() );

  return m_listLayers.back();

}



int CLevelMap::CollisionTypeAt( int iX, int iY )
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    if ( iX < 0 )
    {
      return ( m_Flags & MF_OPEN_LEFT ) ? 0 : 1;
    }
    if ( iX >= Layer.m_vTileSize.x * Layer.Width() )
    {
      return ( m_Flags & MF_OPEN_RIGHT ) ? 0 : 1;
    }
    if ( iY < 0 )
    {
      return ( m_Flags & MF_OPEN_TOP ) ? 0 : 1;
    }
    if ( iY >= Layer.m_vTileSize.y * Layer.Height() )
    {
      return ( m_Flags & MF_OPEN_BOTTOM ) ? 0 : 1;
    }
    if ( Layer.m_Type == CLevelLayer::LT_COLLISION )
    {
      size_t   iTile = Layer.Field( iX / Layer.m_vTileSize.x, iY / Layer.m_vTileSize.y );

      if ( ( iTile == 65535 )
      ||   ( iTile == 0 ) )
      {
        return 0;
      }

      CXJump&   XJump = CXJump::Instance();

      if ( ( iTile < 0 )
      ||   ( iTile >= XJump.m_vectColissionTiles.size() )
      ||   ( iTile == 1 ) )
      {
        return 1;
      }
      GR::Graphic::ContextDescriptor&   cdTile = XJump.m_vectColissionTiles[iTile];

      return cdTile.GetDirectPixel( iX % Layer.m_vTileSize.x, iY % Layer.m_vTileSize.y );
    }
    /*
    else if ( Layer.m_Type == CLevelLayer::LT_BLOCKS )
    {
      if ( Layer.GetFeld( iX / Layer.m_vTileSize.x, iY / Layer.m_vTileSize.y ) == 1 )
      {
        return 1;
      }
    }
    */

    ++itLayer;
  }

  return 0;

}



void CLevelMap::Update( const float fElapsedTime )
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    Layer.Update( fElapsedTime );

    ++itLayer;
  }

}



void CLevelMap::PrepareForStart()
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    Layer.PrepareForStart();

    ++itLayer;
  }

}



GR::u32 CLevelMap::AddExtraData()
{

  GR::u32   dwFirstAvailableEntry = 1;

  tMapExtraData::iterator   it( m_mapExtraData.begin() );
  while ( it != m_mapExtraData.end() )
  {
    if ( it->first != dwFirstAvailableEntry )
    {
      m_mapExtraData[dwFirstAvailableEntry] = tExtraData();
      return dwFirstAvailableEntry;
    }

    dwFirstAvailableEntry++;
    ++it;
  }

  m_mapExtraData[dwFirstAvailableEntry] = tExtraData();
  return dwFirstAvailableEntry;

}



void CLevelMap::RemoveExtraData( GR::u32 dwEntry )
{

  tMapExtraData::iterator   it( m_mapExtraData.find( dwEntry ) );
  if ( it == m_mapExtraData.end() )
  {
    return;
  }
  m_mapExtraData.erase( it );

}



tExtraData* CLevelMap::GetExtraData( GR::u32 dwEntry )
{

  tMapExtraData::iterator   it( m_mapExtraData.find( dwEntry ) );
  if ( it == m_mapExtraData.end() )
  {
    return NULL;
  }
  return &it->second;

}



CGameObject* CLevelMap::FindCollidingObject( CGameObject* pCollider, const GR::String& strType )
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    CGameObject*    pResult = Layer.FindCollidingObject( pCollider, strType );
    if ( pResult )
    {
      return pResult;
    }

    ++itLayer;
  }

  return NULL;

}



CLevelLayer* CLevelMap::GetLayer( const GR::String& strLayerName )
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    if ( Layer.m_strDesc == strLayerName )
    {
      return &Layer;
    }

    ++itLayer;
  }
  return NULL;

}



CLevelLayer* CLevelMap::GetCollisionLayer()
{

  tListLayer::iterator    itLayer( m_listLayers.begin() );
  while ( itLayer != m_listLayers.end() )
  {
    CLevelLayer&    Layer = *itLayer;

    if ( Layer.m_Type == CLevelLayer::LT_COLLISION )
    {
      return &Layer;
    }

    ++itLayer;
  }
  return NULL;

}



void CLevelMap::CenterOnObject( CGameObject* pObject )
{

  if ( pObject == NULL )
  {
    return;
  }

  m_vectScrollOffset.x = pObject->m_vPosition.x - CXJump::Instance().m_ptViewportExtents.x / 2;
  m_vectScrollOffset.y = pObject->m_vPosition.y - CXJump::Instance().m_ptViewportExtents.y / 2;
  CXJump::Instance().ClipToCurrentRange( m_vectScrollOffset );

}



void CLevelMap::RemoveAllObjectsOfType( const GR::String& Type )
{

  tListLayer::iterator    it( m_listLayers.begin() );
  while ( it != m_listLayers.end() )
  {
    it->RemoveAllObjectsOfType( Type );

    ++it;
  }

}