#include "DX11Renderer.h"
#include "DX11Texture.h"



DX11Texture::DX11Texture() :
  m_pTexture( NULL ),
  m_pTextureShaderResourceView( NULL ),
  m_pTextureRenderTargetView( NULL ),
  m_MipMapLevels( 1 ),
  m_TransparentColor( 0 )
{
}



DX11Texture::~DX11Texture()
{
  Release();
}



bool DX11Texture::Release()
{
  if ( m_pTextureShaderResourceView )
  {
    m_pTextureShaderResourceView->Release();
    m_pTextureShaderResourceView = NULL;
  }
  if ( m_pTextureRenderTargetView )
  {
    m_pTextureRenderTargetView->Release();
    m_pTextureRenderTargetView = NULL;
  }
  if ( m_pTexture )
  {
    m_pTexture->Release();
    m_pTexture = NULL;
  }
  return true;
}



bool DX11Texture::RequiresRebuild()
{
  if ( !AllowUsageAsRenderTarget )
  {
    return false;
  }
  return ( m_pTexture == NULL );
}