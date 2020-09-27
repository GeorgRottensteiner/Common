#include "DX9RenderClass.h"

#include "DX9Texture.h"



CDX9Texture::CDX9Texture() :
  m_Surface( NULL ),
  m_MipMapLevels( 1 )
{
}



CDX9Texture::~CDX9Texture()
{

  if ( m_Surface )
  {
    m_Surface->Release();
    m_Surface = NULL;
  }

}



bool CDX9Texture::Release()
{
  if ( m_Surface )
  {
    m_Surface->Release();
    m_Surface = NULL;
  }
  return true;
}



bool CDX9Texture::RequiresRebuild()
{
  if ( !AllowUsageAsRenderTarget )
  {
    return false;
  }
  return ( m_Surface == NULL );
}