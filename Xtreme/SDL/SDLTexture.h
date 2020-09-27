#pragma once


#include <SDL.h>

#include <vector>

#include <Xtreme/XTexture.h>



struct SDL_Texture;

class SDLTexture : public XTexture
{

  public:

    std::vector<GR::String>       m_FileNames;

    SDL_Texture*                  m_pSDLTexture;


    SDLTexture();
    virtual ~SDLTexture();


    virtual bool                  Release();

    // used for render target textures
    virtual bool                  RequiresRebuild();

};
