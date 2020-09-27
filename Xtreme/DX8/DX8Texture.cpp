#include "DX8RenderClass.h"

#include "DX8Texture.h"



CDX8Texture::CDX8Texture( CDX8RenderClass* pRenderer ) :
  m_pRenderer( pRenderer ),
  m_Surface( NULL ),
  m_MipMapLevels( 1 ),
  AllowUsageAsRenderTarget( false ),
  RequiresRebuilding( false )
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



bool CDX8Texture::Release()
{
  if ( m_Surface )
  {
    m_Surface->Release();
    m_Surface = NULL;
  }
  return true;
}



bool CDX8Texture::RequiresRebuild()
{
  if ( !AllowUsageAsRenderTarget )
  {
    return false;
  }
  return RequiresRebuilding;
}