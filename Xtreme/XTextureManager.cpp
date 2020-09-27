/*----------------------------------------------------------------------------+
 | Programmname       :3d-Engine über Direct3D (Textures)                     |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "XTextureManager.h"

#include "XRenderer.h"

#include <debug/debugclient.h>



XTextureManager::XTextureManager()
{
}



XTextureManager& XTextureManager::Instance()
{

  static XTextureManager      g_Instance;


  return g_Instance;

}



XTexture* XTextureManager::Request( const GR::String& strName )
{
  tResourceMap::iterator   it( m_mapResourcen.find( strName ) );
  if ( it == m_mapResourcen.end() )
  {
    return NULL;
  }
  XTexture* pResource = it->second;
  return pResource;
}



XTexture* XTextureManager::Create( XRenderer& Renderer, const GR::String& strName, GR::u32 dwTransparentColor )
{

  XTexture*   pTex = Request( strName );
  if ( pTex )
  {
    return pTex;
  }
  pTex = Renderer.LoadTexture( strName.c_str(), GR::Graphic::IF_UNKNOWN, dwTransparentColor );
  Insert( strName, pTex );


  return pTex;

}



void XTextureManager::Insert( const GR::String& strName, XTexture* pResource )
{

  tResourceMap::iterator   it( m_mapResourcen.find( strName ) );
  if ( it != m_mapResourcen.end() )
  {
    dh::Log( "Double Resource entry %s!!", strName.c_str() );
    return;
  }

  m_mapResourcen[strName] = pResource;
}



void XTextureManager::Remove( XTexture* pResource )
{

  tResourceMap::iterator   it( m_mapResourcen.begin() );
  while ( it != m_mapResourcen.end() )
  {
    if ( it->second == pResource )
    {
      m_mapResourcen.erase( it );
      return;
    }
    ++it;
  }

}
