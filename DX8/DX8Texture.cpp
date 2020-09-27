/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8/DX8Viewer.h>

#include "DX8Texture.h"



CDX8Texture::CDX8Texture( const GR::String& strFileName,
                          DWORD dwCreationFlags,
                          DWORD dwTransparentColor,
                          DWORD dwMipMapLevels ) :
  IResource(),
  m_Surface( NULL ),
  m_pViewer( NULL ),
  m_strFileName( strFileName ),
  m_dwCreationFlags( dwCreationFlags ),
  m_dwMipmapLevels( dwMipMapLevels )
{
}



CDX8Texture::~CDX8Texture()
{

  if ( m_Surface )
  {
    m_Surface->Release();
    m_Surface = NULL;
  }

}



/*-RecreateSurface------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CDX8Texture::RecreateSurface()
{

  if ( !m_pViewer )
  {
    return false;
  }

  if ( m_Surface != NULL )
  {
    return true;
  }

  if ( !m_pViewer->LoadSurface( this, m_dwCreationFlags, m_dwTransparentColor ) )
  {
    return false;
  }
  return true;

}



bool CDX8Texture::Load()
{

  if ( m_pViewer == NULL )
  {
    return false;
  }
  if ( !RecreateSurface() )
  {
    return false;
  }
  m_bIsLoaded = true;
  return true;

}



bool CDX8Texture::Release()
{

  if ( m_Surface )
  {
    m_Surface->Release();
    m_Surface = NULL;
  }
  m_bIsLoaded = false;
  return true;

}



bool CDX8Texture::Destroy()
{

  return Release();

}



bool CDX8Texture::HasAlpha() const
{

  return !!( m_dwCreationFlags & DX8::TF_ALPHA_MASK );

}