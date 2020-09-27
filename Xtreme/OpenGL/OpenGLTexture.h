#ifndef XTREME_OPENGL_TEXTURE_H
#define XTREME_OPENGL_TEXTURE_H


#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>

#include <Xtreme/XTexture.h>

#include <list>
#include <string>



class OpenGLTexture : public XTexture
{

  public:

    GLuint                            m_TextureID;

    std::list<GR::String>            m_listFileNames;

    std::list<GR::Graphic::ImageData> m_StoredImageData;

    bool                              AllowUsageAsRenderTarget;


    OpenGLTexture();
    virtual ~OpenGLTexture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_OPENGL_TEXTURE_H