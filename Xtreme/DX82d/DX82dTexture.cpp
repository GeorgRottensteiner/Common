#include "DX82dRenderClass.h"

#include "DX82dTexture.h"



CDX8Texture::CDX8Texture( DX82dRenderer* pRenderer ) :
  m_pRenderer( pRenderer ),
  m_Surface( NULL )
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
  return false;
}