#include "OpenGLTexture.h"
#include "OpenGLRenderClass.h"


OpenGLTexture::OpenGLTexture( OpenGLRenderClass* pRenderer, bool AllowUsageAsRenderTarget ) :
  m_pRenderer( pRenderer ),
  m_TextureID( 0 ),
  m_AllowUsageAsRenderTarget( AllowUsageAsRenderTarget ),
  m_RenderTargetFrameBufferID( 0 ),
  m_RenderBufferDepthStencil( 0 )
{
}



OpenGLTexture::~OpenGLTexture()
{
  Release();
}



bool OpenGLTexture::Release()
{
  m_pRenderer->ReleaseTexture( this );

  return true;
}



bool OpenGLTexture::RequiresRebuild()
{
  if ( !m_AllowUsageAsRenderTarget )
  {
    return false;
  }
  return false;
}