/*----------------------------------------------------------------------------+
 | Programmname       :3d-Engine über Direct3D (Textures)                     |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 +----------------------------------------------------------------------------*/



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "TextureManager.h"

#include <DX8/DX8Viewer.h>



CTextureManager::CTextureManager()
{
}



CTextureManager& CTextureManager::Instance()
{

  static CTextureManager      g_Instance;


  return g_Instance;

}



void CTextureManager::RecreateTextures()
{

  tResourceMap::iterator  itTexture( m_mapResourcen.begin() );
  while ( itTexture != m_mapResourcen.end() )
  {
    CDX8Texture*    pTextureEntry = (CDX8Texture*)itTexture->second;

    if ( pTextureEntry == NULL )
    {
      dh::Log( "RecreateTextures: Schwerer Fehler! pTextureEntry = NULL!" );
      itTexture++;
      continue;
    }

    if ( pTextureEntry->m_dwCreationFlags & DX8::TF_DONT_LOAD )
    {
      if ( pTextureEntry->m_Surface )
      {
        pTextureEntry->m_Surface->Release();
        pTextureEntry->m_Surface = NULL;
      }
      delete pTextureEntry;
      itTexture = m_mapResourcen.erase( itTexture );
      continue;
    }

    if ( pTextureEntry->m_Surface )
    {
      pTextureEntry->m_Surface->Release();
      pTextureEntry->m_Surface = NULL;
    }
    if ( pTextureEntry->m_Surface == NULL )
    {
      pTextureEntry->m_pViewer->LoadSurface( pTextureEntry,
                                             pTextureEntry->m_dwCreationFlags,
                                             pTextureEntry->m_dwTransparentColor );
    }

    itTexture++;
  }

}



void CTextureManager::ReleaseTextures()
{

  tResourceMap::iterator  itTexture( m_mapResourcen.begin() );
  while ( itTexture != m_mapResourcen.end() )
  {
    CDX8Texture*    pTextureEntry = (CDX8Texture*)itTexture->second;
    pTextureEntry->m_bIsLoaded = false;
    if ( pTextureEntry->m_Surface )
    {
      pTextureEntry->m_Surface->Release();
      pTextureEntry->m_Surface = NULL;
    }
    if ( pTextureEntry->m_dwCreationFlags & DX8::TF_DONT_LOAD )
    {
      delete pTextureEntry;
      itTexture = m_mapResourcen.erase( itTexture );
      continue;
    }

    itTexture++;
  }

}



CDX8Texture* CTextureManager::Create( CD3DViewer& Viewer, const GR::String& strName, DWORD dwFlags, DWORD dwTransparentColor, DWORD dwMipmapLevels )
{

  return Viewer.LoadTexture( strName.c_str(), dwFlags, dwTransparentColor, dwMipmapLevels );

}
