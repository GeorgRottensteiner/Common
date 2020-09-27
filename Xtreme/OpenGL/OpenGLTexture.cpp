#include "OpenGLTexture.h"



OpenGLTexture::OpenGLTexture() :
  m_TextureID( 0 ),
  AllowUsageAsRenderTarget( false )
{
}



OpenGLTexture::~OpenGLTexture()
{
  Release();
}



bool OpenGLTexture::Release()
{
  //TODO
  return true;
}



bool OpenGLTexture::RequiresRebuild()
{
  if ( !AllowUsageAsRenderTarget )
  {
    return false;
  }
  return false;
}