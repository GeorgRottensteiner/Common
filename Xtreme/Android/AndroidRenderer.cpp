#include "AndroidRenderer.h"

#include <android/log.h>

AndroidRenderer::AndroidRenderer() :
  m_Window( NULL ),
  m_Width( 0 ),
  m_Height( 0 ),
  m_Display( EGL_NO_DISPLAY ),
  m_Context( EGL_NO_CONTEXT ),
  m_Surface( EGL_NO_SURFACE )
{
}



bool AndroidRenderer::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment )
{
  if ( m_Window == NULL )
  {
    return true;
  }

  m_Width = Width;
  m_Height = Height;

  // initialize OpenGL ES and EGL

  /*
  * Here specify the attributes of the desired configuration.
  * Below, we select an EGLConfig with at least 8 bits per color
  * component compatible with on-screen windows
  */
  const EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_NONE
  };
  EGLint w, h, format;
  EGLint numConfigs;
  EGLConfig config;
  EGLSurface surface;
  EGLContext context;

  EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );

  eglInitialize( display, 0, 0 );

  /* Here, the application chooses the configuration it desires. In this
  * sample, we have a very simplified selection process, where we pick
  * the first EGLConfig that matches our criteria */
  eglChooseConfig( display, attribs, &config, 1, &numConfigs );

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
  * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
  * As soon as we picked a EGLConfig, we can safely reconfigure the
  * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib( display, config, EGL_NATIVE_VISUAL_ID, &format );

  //ANativeWindow_setBuffersGeometry( m_Window, 0, 0, format );

  surface = eglCreateWindowSurface( display, config, m_Window, NULL );
  context = eglCreateContext( display, config, NULL, NULL );

  if ( eglMakeCurrent( display, surface, surface, context ) == EGL_FALSE )
  {
    dh::Log( "Unable to eglMakeCurrent" );
    return -1;
  }

  eglQuerySurface( display, surface, EGL_WIDTH, &w );
  eglQuerySurface( display, surface, EGL_HEIGHT, &h );

  m_Display = display;
  m_Context = context;
  m_Surface = surface;

  // Initialize GL state.
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
  glEnable( GL_CULL_FACE );
  glShadeModel( GL_SMOOTH );
  glDisable( GL_DEPTH_TEST );


  return true;
}



bool AndroidRenderer::Release()
{
  if ( m_Display != EGL_NO_DISPLAY )
  {
    eglMakeCurrent( m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    if ( m_Context != EGL_NO_CONTEXT )
    {
      eglDestroyContext( m_Display, m_Context );
    }
    if ( m_Surface != EGL_NO_SURFACE )
    {
      eglDestroySurface( m_Display, m_Surface );
    }
    eglTerminate( m_Display );
  }
  m_Display = EGL_NO_DISPLAY;
  m_Context = EGL_NO_CONTEXT;
  m_Surface = EGL_NO_SURFACE;

  return true;
}



bool AndroidRenderer::OnResized()
{
  return true;
}



bool AndroidRenderer::IsReady() const
{
  return false;
}



bool AndroidRenderer::BeginScene()
{
  if ( m_Display == EGL_NO_DISPLAY )
  {
    // No display
    return false;
  }

  return true;
}



void AndroidRenderer::EndScene()
{
}



void AndroidRenderer::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  eglSwapBuffers( m_Display, m_Surface );
}



bool AndroidRenderer::ToggleFullscreen()
{
  return true;
}



bool AndroidRenderer::SetMode( XRendererDisplayMode& DisplayMode )
{
  return true;
}



bool AndroidRenderer::IsFullscreen()
{
  return false;
}



bool AndroidRenderer::SetState( eRenderState rState, GR::u32 rValue, GR::u32 dwStage )
{
  return false;
}



bool AndroidRenderer::SetViewport( const XViewport& Viewport )
{
  return false;
}



bool AndroidRenderer::SetTrueViewport( const XViewport& Viewport )
{
  return false;
}



GR::u32 AndroidRenderer::Width()
{
  return 0;
}



GR::u32 AndroidRenderer::Height()
{
  return 0;
}




void AndroidRenderer::Clear( bool bClearColor, bool bClearZ )
{
}



bool AndroidRenderer::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  return true;
}



XTexture* AndroidRenderer::CreateTexture( const GR::u32 dwWidth, const GR::u32 dwHeight, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  return NULL;
}



XTexture* AndroidRenderer::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevels, bool AllowUsageAsRenderTarget )
{
  return NULL;
}



XTexture* AndroidRenderer::LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 dwColorKey, const GR::u32 dwMipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  return NULL;
}



XTexture* AndroidRenderer::LoadTexture( const GR::IString& FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 dwColorKey, const GR::u32 dwMipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  return NULL;
}



void AndroidRenderer::SetTexture( GR::u32 dwStage, XTexture* pTexture )
{
}



bool AndroidRenderer::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 dwColorKey, const GR::u32 dwMipMapLevel )
{
  return false;
}



// Lights
bool AndroidRenderer::SetLight( GR::u32 dwLightIndex, XLight& Light )
{
  return false;
}



bool AndroidRenderer::SetMaterial( const XMaterial& Material )
{
  return false;
}



XVertexBuffer* AndroidRenderer::CreateVertexBuffer( GR::u32 dwPrimitiveCount, GR::u32 dwVertexFormat, XVertexBuffer::PrimitiveType Type )
{
  return NULL;
}



XVertexBuffer* AndroidRenderer::CreateVertexBuffer( GR::u32 dwVertexFormat, XVertexBuffer::PrimitiveType Type )
{
  return NULL;
}



XVertexBuffer* AndroidRenderer::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 dwVertexFormat )
{
  return NULL;
}




void AndroidRenderer::DestroyVertexBuffer( XVertexBuffer* )
{
}



bool AndroidRenderer::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  return false;
}


void AndroidRenderer::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float fZ )
{
}



void AndroidRenderer::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
{
}



void AndroidRenderer::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3, GR::u32 dwColor4, float fZ )
{
}



void AndroidRenderer::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3, GR::u32 dwColor4, float fZ )
{
}



void AndroidRenderer::RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
  GR::f32 fX2, GR::f32 fY2,
  GR::f32 fX3, GR::f32 fY3,
  GR::f32 fX4, GR::f32 fY4,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3, GR::u32 Color4, float fZ )
{
}




void AndroidRenderer::RenderQuad( const GR::tVector& ptPos1,
  const GR::tVector& ptPos2,
  const GR::tVector& ptPos3,
  const GR::tVector& ptPos4,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3, GR::u32 dwColor4 )
{
}



void AndroidRenderer::RenderQuad( const GR::tVector& ptPos1,
  const GR::tVector& ptPos2,
  const GR::tVector& ptPos3,
  const GR::tVector& ptPos4,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3, GR::u32 dwColor4 )
{
}




void AndroidRenderer::RenderTriangle( const GR::tVector& pt1,
  const GR::tVector& pt2,
  const GR::tVector& pt3,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3 )
{
}




void AndroidRenderer::RenderTriangle2d( const GR::tPoint& pt1,
  const GR::tPoint& pt2,
  const GR::tPoint& pt3,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::u32 dwColor1, GR::u32 dwColor2,
  GR::u32 dwColor3, float fZ )
{
}



bool AndroidRenderer::SaveScreenShot( const char* FileName )
{
  return false;
}



void AndroidRenderer::ReleaseAssets()
{
}



bool AndroidRenderer::VSyncEnabled()
{
  return true;
}



void AndroidRenderer::EnableVSync( bool Enable )
{
}



GR::Graphic::eImageFormat AndroidRenderer::ImageFormat()
{
  return GR::Graphic::IF_UNKNOWN;
}



bool AndroidRenderer::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  return false;
}



bool AndroidRenderer::RenderMesh( const Mesh::IMesh& Mesh )
{
  return false;
}



void AndroidRenderer::SetRenderTarget( XTexture* pTexture )
{
}