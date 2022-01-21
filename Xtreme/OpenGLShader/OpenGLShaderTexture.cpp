#include "OpenGLShaderTexture.h"
#include "OpenGLShaderRenderClass.h"


OpenGLShaderTexture::OpenGLShaderTexture( OpenGLShaderRenderClass* pRenderer, bool AllowUsageAsRenderTarget ) :
  m_pRenderer( pRenderer ),
  m_TextureID( 0 ),
  m_AllowUsageAsRenderTarget( AllowUsageAsRenderTarget ),
  m_RenderTargetFrameBufferID( 0 ),
  m_RenderBufferDepthStencil( 0 )
{
}



OpenGLShaderTexture::~OpenGLShaderTexture()
{
  Release();
}



bool OpenGLShaderTexture::Release()
{
  m_pRenderer->ReleaseTexture( this );

  return true;
}



bool OpenGLShaderTexture::RequiresRebuild()
{
  if ( !m_AllowUsageAsRenderTarget )
  {
    return false;
  }
  return false;
}