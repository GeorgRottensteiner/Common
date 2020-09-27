#include "SDLTexture.h"


SDLTexture::SDLTexture() :
  m_pSDLTexture( NULL )
{
}



SDLTexture::~SDLTexture()
{
}



bool SDLTexture::Release()
{
  if ( m_pSDLTexture != NULL )
  {
    SDL_DestroyTexture( m_pSDLTexture );
    m_pSDLTexture = NULL;
  }
  return true;
}



bool SDLTexture::RequiresRebuild()
{
  return false;
}