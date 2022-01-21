#ifndef XTREME_OPENGL_TEXTURE_H
#define XTREME_OPENGL_TEXTURE_H


#include <Xtreme/XTexture.h>

#include <list>
#include <string>



class OpenGLShaderRenderClass;

class OpenGLShaderTexture : public XTexture
{

  public:

    OpenGLShaderRenderClass*            m_pRenderer;

    GR::u32                           m_TextureID;

    std::list<GR::String>             m_listFileNames;

    std::list<GR::Graphic::ImageData> m_StoredImageData;

    bool                              m_AllowUsageAsRenderTarget;
    GR::u32                           m_RenderTargetFrameBufferID;
    GR::u32                           m_RenderBufferDepthStencil;


    OpenGLShaderTexture( OpenGLShaderRenderClass* pRenderer, bool AllowUsageAsRenderTarget );
    virtual ~OpenGLShaderTexture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_OPENGL_TEXTURE_H