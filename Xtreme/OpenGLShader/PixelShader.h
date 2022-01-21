#ifndef XTREME_PIXELSHADER
#define XTREME_PIXELSHADER

#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>

#if ( OPERATING_SYSTEM == OS_WEB )
#define GL_GLEXT_PROTOTYPES
#include <emscripten.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#include <gl\glu.h>
#else
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#endif




class OpenGLShaderRenderClass;

class PixelShader
{

  public:

    GLuint                              m_ShaderID;

    OpenGLShaderRenderClass*            m_pRenderer;

    GR::u32                             m_VertexFormat;

    GR::String                          m_ShaderScript;



    PixelShader( OpenGLShaderRenderClass* pRenderer );
    ~PixelShader();


    void                                Release();


    bool CreateFromFile( const GR::String& CompiledShaderFile );
    bool CreateFromString( const GR::String& ShaderContent );

};


#endif // XTREME_PIXELSHADER
