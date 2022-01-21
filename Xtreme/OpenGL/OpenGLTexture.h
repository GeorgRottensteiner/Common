#ifndef XTREME_OPENGL_TEXTURE_H
#define XTREME_OPENGL_TEXTURE_H


#include <Xtreme/XTexture.h>

#include <list>
#include <string>



class OpenGLRenderClass;

class OpenGLTexture : public XTexture
{

  public:

    OpenGLRenderClass*                m_pRenderer;

    GR::u32                           m_TextureID;

    std::list<GR::String>             m_listFileNames;

    std::list<GR::Graphic::ImageData> m_StoredImageData;

    bool                              m_AllowUsageAsRenderTarget;
    GR::u32                           m_RenderTargetFrameBufferID;
    GR::u32                           m_RenderBufferDepthStencil;


    OpenGLTexture( OpenGLRenderClass* pRenderer, bool AllowUsageAsRenderTarget );
    virtual ~OpenGLTexture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_OPENGL_TEXTURE_H