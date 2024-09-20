#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include <Xtreme/Environment/XWindow.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include "OpenGLRenderClass.h"
#include "OpenGLTexture.h"
#include "OpenGLVertexBuffer.h"


#if ( OPERATING_SYSTEM == OS_WEB ) && ( OPERATING_SUB_SYSTEM != OS_SUB_SDL )
#include <emscripten.h>
#else
#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )
//#pragma comment( lib, "glaux.lib" )
#endif

#if OPERATING_SYSTEM != OS_WEB
typedef void ( APIENTRY* PFNGLBINDBUFFERPROC ) ( GLenum target, GLuint buffer );
typedef void ( APIENTRY* PFNGLDELETEBUFFERSPROC ) ( GLsizei CountBuffersToDelete, const GLuint* buffers );
typedef void ( APIENTRY* PFNGLGENBUFFERSPROC ) ( GLsizei CountBuffersToGenerate, GLuint* buffers );
typedef void ( APIENTRY* PFNGLBUFFERDATAPROC ) ( GLenum target, int size, const GLvoid* data, GLenum usage );
typedef void ( APIENTRYP PFNGLGENFRAMEBUFFERSPROC ) ( GLsizei n, GLuint* framebuffers );
typedef void ( APIENTRYP PFNGLBINDFRAMEBUFFERPROC ) ( GLenum target, GLuint framebuffer );
typedef void ( APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC ) ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level );
typedef void ( APIENTRYP PFNGLDRAWBUFFERSPROC ) ( GLsizei n, const GLenum* bufs );
typedef GLenum( APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC ) ( GLenum target );
typedef void ( APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC ) ( GLsizei n, const GLuint* framebuffers );
typedef void ( APIENTRYP PFNGLGENRENDERBUFFERSPROC ) ( GLsizei n, GLuint* renderbuffers );
typedef void ( APIENTRYP PFNGLBINDRENDERBUFFERPROC ) ( GLenum target, GLuint renderbuffer );
typedef void ( APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC ) ( GLenum target, GLenum internalformat, GLsizei width, GLsizei height );
typedef void ( APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC ) ( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer );
typedef void ( APIENTRYP PFNGLDELETERENDERBUFFERSPROC ) ( GLsizei n, const GLuint* renderbuffers );
typedef void ( APIENTRYP PFNGLACTIVETEXTUREPROC ) ( GLenum texture );

PFNGLGENFRAMEBUFFERSPROC      glGenFramebuffers = NULL;
// VBO Extension Function Pointers
PFNGLGENBUFFERSPROC    glGenBuffers = NULL;					// VBO Name Generation Procedure
PFNGLBINDBUFFERPROC    glBindBuffer = NULL;					// VBO Bind Procedure
PFNGLBUFFERDATAPROC    glBufferData = NULL;					// VBO Data Loading Procedure
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;			// VBO Deletion Procedure

PFNGLBINDFRAMEBUFFERPROC      glBindFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC   glCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC   glDeleteFramebuffers = NULL;
PFNGLGENRENDERBUFFERSPROC     glGenRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC     glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC  glRenderbufferStorage = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC  glFramebufferRenderbuffer = NULL;
PFNGLDELETERENDERBUFFERSPROC  glDeleteRenderbuffers = NULL;

PFNGLDRAWBUFFERSPROC          glDrawBuffers = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
#endif


#if ( OPERATING_SUB_SYSTEM != OS_SUB_SDL )
PFNGLACTIVETEXTUREPROC        glActiveTexture = NULL;
#endif



OpenGLRenderClass::OpenGLRenderClass() :
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  m_pMainWindow( NULL ),
#else
  m_RC( NULL ),
  m_DC( NULL ),
#endif
  m_FullScreen( false ),
  m_Width( 0 ),
  m_Height( 0 ),
  m_AlphaRefValue( 8.0f / 255.0f ),
  m_AlphaFuncValue( GL_GEQUAL ),
  m_SrcBlend( GL_ONE ),
  m_DestBlend( GL_ZERO ),
  m_OldPixelFormat( 0 ),
  m_Depth( 0 ),
  m_Ready( false ),
  m_SupportsVBO( false ),
  m_2dModeActive( false ),
  m_pSetRenderTargetTexture( NULL )
{
  m_DirectTexelMappingOffset.Set( -0.5f, -0.5f );

  for ( int i = 0; i < 8; ++i )
  {
    m_pSetTextures[i] = NULL;
  }
}



OpenGLRenderClass::~OpenGLRenderClass()
{
  Release();
}



bool OpenGLRenderClass::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment )
{
  if ( m_Ready )
  {
    return true;
  }

  m_pEnvironment = &Environment;

  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  if ( pWindowService != NULL )
  {
    m_pMainWindow = (SDL_Window*)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }
#else
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }

  m_hwndViewport = hWnd;
#endif

  m_FullScreen = ( Flags & XRenderer::IN_FULLSCREEN );

  if ( m_FullScreen )
  {
#if OPERATING_SYSTEM == OS_WINDOWS

    DEVMODE     dmScreenSettings;					// Device Mode

		memset( &dmScreenSettings, 0, sizeof( dmScreenSettings ) );
		dmScreenSettings.dmSize       = sizeof( dmScreenSettings );
		dmScreenSettings.dmPelsWidth	= Width;
		dmScreenSettings.dmPelsHeight	= Height;
		dmScreenSettings.dmBitsPerPel	= Depth;
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if ( ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
		{
      Release();
      return false;
    }
#endif
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_SDL

#if OPERATING_SYSTEM != OS_WEB
  // MUST BE CALLED BEFORE SDL_init!! <- Emscripten absolutely does NOT like that after SDL_init
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 4 );

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
#endif

  m_GLContext = SDL_GL_CreateContext( m_pMainWindow );
#else
  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof( PIXELFORMATDESCRIPTOR ),
    1,								            // Version Number
    PFD_DRAW_TO_WINDOW |					// Format Must Support Window
    PFD_SUPPORT_OPENGL |					// Format Must Support OpenGL
    PFD_DOUBLEBUFFER,						  // Must Support Double Buffering
    PFD_TYPE_RGBA,							  // Request An RGBA Format
    (BYTE)Depth,								// Select Our Color Depth
    0, 0, 0, 0, 0, 0,						  // Color Bits Ignored
    0,								            // No Alpha Buffer
    0,								            // Shift Bit Ignored
    0,								            // No Accumulation Buffer
    0, 0, 0, 0,							      // Accumulation Bits Ignored
    16,								            // 16Bit Z-Buffer (Depth Buffer)
    0,								            // No Stencil Buffer
    0,								            // No Auxiliary Buffer
    PFD_MAIN_PLANE,							  // Main Drawing Layer
    0,								            // Reserved
    0, 0, 0								        // Layer Masks Ignored
  };

  m_DC = GetDC( m_hwndViewport );
  if ( m_DC == NULL )
	{
    dh::Log( "OpenGL: GetDC failed" );
    Release();
    return false;
	}

  m_OldPixelFormat = GetPixelFormat( m_DC );
  DescribePixelFormat( m_DC, m_OldPixelFormat, sizeof( m_pfd ), &m_pfd );

  GLuint	pixelFormat = ChoosePixelFormat( m_DC, &pfd );
  if ( pixelFormat == 0 )
  {
    dh::Log( "OpenGL: ChoosePixelFormat failed (%x)", GetLastError() );
    Release();
    return false;
  }

  if ( !SetPixelFormat( m_DC, pixelFormat, &pfd ) )
	{
    dh::Log( "OpenGL: SetPixelFormat failed" );
    Release();
    return false;
	}

  m_RC = wglCreateContext( m_DC );
  if ( m_RC == NULL )
	{
    dh::Log( "OpenGL: wglCreateContext failed" );
    Release();
    return false;
	}

  if ( !wglMakeCurrent( m_DC, m_RC ) )
	{
    dh::Log( "OpenGL: wglMakeCurrent failed" );
    Release();
		return false;
	}
#endif

#if OPERATING_SYSTEM != OS_WEB
  GR::String  version = (char*)glGetString( GL_VERSION );
  //dh::Log( "Open GL version %s", version.c_str() );
#endif

  m_Width   = Width;
  m_Height  = Height;
  m_Depth   = Depth;

  m_Canvas.Set( 0, 0, Width, Height );
  m_VirtualSize.Set( Width, Height );

  m_Transform[TT_WORLD].Identity();
  m_Transform[TT_VIEW].Identity();
  m_Transform[TT_PROJECTION].Identity();

  glFlush();
  OnResized();

#if OPERATING_SYSTEM != OS_WEB
  glShadeModel( GL_SMOOTH );
  glClearDepth( 1.0f );
#endif
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
#if OPERATING_SYSTEM != OS_WEB
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
#endif
  glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
  glBlendFunc( m_SrcBlend, m_DestBlend );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
  glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );

  SetState( XRenderer::RS_CULLMODE, XRenderer::RSV_CULL_CCW );

#if OPERATING_SYSTEM != OS_WEB
  // both disabled for Emscripten
  glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, 1 );
  glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
#endif
  glEnable( GL_COLOR_MATERIAL );

  SetState( XRenderer::RS_NORMALIZE_NORMALS, XRenderer::RSV_ENABLE );
  SetState( XRenderer::RS_MINFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MAGFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MIPFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_FOG_ENABLE, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_FOG_TABLEMODE, XRenderer::RSV_FOG_LINEAR );
  SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_AMBIENT, 0 );

  m_2dModeActive = false;

  m_ViewPort.X = 0;
  m_ViewPort.Y = 0;
  m_ViewPort.Width = Width;
  m_ViewPort.Height = Height;
  m_ViewPort.MinZ = 0.0f;
  m_ViewPort.MaxZ = 1.0f;

  SortAndCleanDisplayModes();

#if OPERATING_SUB_SYSTEM != OS_SUB_SDL
  // Get Pointers To The GL Functions
  glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress( "glActiveTexture" );
#endif

#if OPERATING_SYSTEM != OS_WEB
  glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress( "glGenBuffers" );
  glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress( "glBindBuffer" );
  glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress( "glBufferData" );
  glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress( "glDeleteBuffers" );

  if ( ( glGenBuffers == NULL )
  ||   ( glBindBuffer == NULL )
  ||   ( glBufferData == NULL )
  ||   ( glDeleteBuffers == NULL ) )
  {
    dh::Log( "No VBO support" );
  }
  else
  {
    m_SupportsVBO = true;
  }

  glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress( "glBindFramebuffer" );
  glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress( "glGenFramebuffers" );
  glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress( "glFramebufferTexture2D" );
  glDrawBuffers             = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress( "glDrawBuffers" );
  glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress( "glCheckFramebufferStatus" );
  glDeleteFramebuffers      = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress( "glDeleteFramebuffers" );

  glGenRenderbuffers          = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress( "glGenRenderbuffers" );
  glBindRenderbuffer          = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress( "glBindRenderbuffer" );
  glRenderbufferStorage       = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress( "glRenderbufferStorage" );
  glFramebufferRenderbuffer   = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress( "glFramebufferRenderbuffer" );
  glDeleteRenderbuffers       = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress( "glDeleteRenderbuffers" );

  if ( ( glBindFramebuffer == NULL )
  ||   ( glFramebufferTexture2D == NULL )
  ||   ( glDrawBuffers == NULL )
  ||   ( glCheckFramebufferStatus == NULL )
  ||   ( glDeleteFramebuffers == NULL )
  ||   ( glGenRenderbuffers == NULL )
  ||   ( glBindRenderbuffer == NULL )
  ||   ( glRenderbufferStorage == NULL )
  ||   ( glFramebufferRenderbuffer == NULL )
  ||   ( glDeleteRenderbuffers == NULL )
  ||   ( glGenFramebuffers == NULL ) )
  {
    dh::Log( "No render to texture support" );
  }
#endif

  NotifyService( "Renderer", "Initialised" );

  CheckError( "RendererInitDone" );

  m_Ready = true;

  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu laden
      GR::up    imageCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE );
      for ( GR::up i = 0; i < imageCount; ++i )
      {
        Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );

        GR::u32         ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );
        GR::u32         ColorKeyReplacementColor = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKeyReplacementColor" ), 16 );

        GR::u32         ForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        GR::String  fileName = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" );

        std::list<GR::String>    listImages;

        GR::u32     MipMaps = 0;

        while ( true )
        {
          if ( MipMaps == 0 )
          {
            listImages.push_back( AppPath( fileName ) );
          }
          else
          {
            GR::String   strPath = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", MipMaps ) );

            if ( strPath.empty() )
            {
              break;
            }
            listImages.push_back( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", MipMaps ) ) ) );
          }
          MipMaps++;
        }

        if ( listImages.empty() )
        {
          Log( "Renderer.General", "DX8Renderer: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front().c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey, MipMaps );

          if ( pTexture )
          {
            if ( pAsset == NULL )
            {
              pAsset = new Xtreme::Asset::XAssetImage( pTexture );
            }
            else
            {
              ( ( Xtreme::Asset::XAssetImage* )pAsset )->m_pTexture = pTexture;
            }

            pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pAsset );

            // MipMaps laden
            std::list<GR::String>::iterator    it( listImages.begin() );
            it++;
            GR::u32   Level = 1;

            while ( it != listImages.end() )
            {
              GR::String& strPath( *it );

              GR::Graphic::ImageData* pData = LoadAndConvert( strPath.c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey, ColorKeyReplacementColor );
              if ( pData == NULL )
              {
                Log( "Renderer.General", CMisc::printf( "DX8Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, ColorKey, Level );
                delete pData;
              }

              ( (OpenGLTexture*)pTexture )->m_listFileNames.push_back( strPath );

              ++Level;
              ++it;
            }
          }
          else
          {
            Log( "Renderer.General", CMisc::printf( "DX8Renderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
          }
        }
      }

      GR::up    imageSectionCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < imageSectionCount; ++i )
      {
        Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = ( Xtreme::Asset::XAssetImage* )pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          Log( "Renderer.General", CMisc::printf( "DX8Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
        }
        else
        {
          XTextureSection   tsSection;

          tsSection.m_XOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "X" ) );
          tsSection.m_YOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Y" ) );
          tsSection.m_Width = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "W" ) );
          tsSection.m_Height = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "H" ) );
          tsSection.m_Flags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Flags" ) );

          tsSection.m_pTexture = pTexture->Texture();

          if ( pAsset == NULL )
          {
            pAsset = new Xtreme::Asset::XAssetImageSection( tsSection );
          }
          else
          {
            ( ( Xtreme::Asset::XAssetImageSection* )pAsset )->m_tsImage = tsSection;
          }
          pLoader->SetAsset( Xtreme::Asset::XA_IMAGE_SECTION, i, pAsset );
        }
      }

      GR::up    fontCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_FONT );
      for ( GR::up i = 0; i < fontCount; ++i )
      {
        Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_FONT, i );

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     LoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        XFont* pFont = NULL;

        if ( ( LoadFlags & XFont::FLF_SQUARED )
             || ( LoadFlags & XFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
                                  LoadFlags,
                                  ColorKey );
        }
        else
        {
          pFont = LoadFont( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), ColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          Log( "Renderer.General", "DX8Renderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }

      GR::up    meshCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_MESH );
      for ( GR::up i = 0; i < meshCount; ++i )
      {
        Xtreme::Asset::XAsset* pAsset = pLoader->Asset( Xtreme::Asset::XA_MESH, i );

        XMesh* pMesh = NULL;

        pMesh = CT3DMeshLoader::Load( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "File" ) ).c_str() );
        if ( pMesh )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
          }
          delete pAsset;

          Xtreme::Asset::XAssetMesh* pAssetMesh = new Xtreme::Asset::XAssetMesh( *pMesh );

          pAssetMesh->m_Mesh.CalculateBoundingBox();

          pAssetMesh->m_pVertexBuffer = CreateVertexBuffer( pAssetMesh->m_Mesh );

          pAsset = pAssetMesh;
          pLoader->SetAsset( Xtreme::Asset::XA_MESH, i, pAsset );

          delete pMesh;
        }
        else
        {
          Log( "Renderer.General", "DX8Renderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
        }
      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  return true;
}



void OpenGLRenderClass::ReleaseTexture( OpenGLTexture* pTexture )
{
  if ( pTexture->m_AllowUsageAsRenderTarget )
  {
    if ( pTexture->m_RenderTargetFrameBufferID != 0 )
    {
      glDeleteFramebuffers( 1, &pTexture->m_RenderTargetFrameBufferID );
      pTexture->m_RenderTargetFrameBufferID = 0;
    }
    if ( pTexture->m_RenderBufferDepthStencil != 0 )
    {
      glDeleteRenderbuffers( 1, &pTexture->m_RenderBufferDepthStencil );
      pTexture->m_RenderBufferDepthStencil = 0;
    }
  }
}



bool OpenGLRenderClass::Release()
{
  if ( !m_Ready )
  {
    return true;
  }
  DestroyAllFonts();
  DestroyAllTextures();
  DestroyAllVertexBuffers();

#if OPERATING_SUB_SYSTEM != OS_SUB_SDL
  if ( m_RC )
  {
    if ( !wglMakeCurrent( NULL, NULL ) )
    {
      dh::Log( "Release: wglMakeCurrent failed" );
    }
    if ( !wglDeleteContext( m_RC ) )
    {
      dh::Log( "Release: wglDeleteContext failed" );
    }
    m_RC = NULL;
  }
  if ( m_DC )
  {
    SetPixelFormat( m_DC, m_OldPixelFormat, &m_pfd );

    ReleaseDC( m_hwndViewport, m_DC );
    m_DC = NULL;
  }
  m_hwndViewport = NULL;
#endif

  m_Ready = false;
  NotifyService( "Renderer", "Released" );
  return true;
}



void OpenGLRenderClass::Set2DMode()
{
  if ( m_2dModeActive )
  {
    return;
  }
  m_2dModeActive = true;

  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_FOG );

  SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, 0 )], m_RenderStates[std::make_pair( RS_ALPHABLENDING, 0 )] );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, VisibleWidth(), VisibleHeight(), 0, -1.0f, 1.0f );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glScissor( m_ViewPort.X, VisibleHeight() - m_ViewPort.Height - m_ViewPort.Y, m_ViewPort.Width, m_ViewPort.Height );
  // viewport needs to be full size, otherwise everything is scaled (facepalm)
  //glViewport( m_ViewPort.X, Height() - m_ViewPort.Height - m_ViewPort.Y, m_ViewPort.Width, m_ViewPort.Height );
  glViewport( 0, 0, VisibleWidth(), VisibleHeight() );
  glDepthRange( m_ViewPort.MinZ, m_ViewPort.MaxZ );
  if ( ( m_ViewPort.X != 0 )
  ||   ( m_ViewPort.Y != 0 )
  ||   ( m_ViewPort.Width != VisibleWidth() )
  ||   ( m_ViewPort.Height != VisibleHeight() ) )
  {
    glEnable( GL_SCISSOR_TEST );
  }
  else
  {
    glDisable( GL_SCISSOR_TEST );
  }
  CheckError( "Set2dMode" );
}



void OpenGLRenderClass::Set3DMode()
{
  if ( !m_2dModeActive )
  {
    return;
  }

  m_2dModeActive = false;
  SetTransform( XRenderer::TT_WORLD, m_Transform[XRenderer::TT_WORLD] );
  SetTransform( XRenderer::TT_VIEW, m_Transform[XRenderer::TT_VIEW] );
  SetTransform( XRenderer::TT_PROJECTION, m_Transform[XRenderer::TT_PROJECTION] );

  if ( m_RenderStates[std::make_pair( RS_MINFILTER, 0 )] == RSV_FILTER_POINT )
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  }
  else
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  }
  if ( m_RenderStates[std::make_pair( RS_MAGFILTER, 0 )] == RSV_FILTER_POINT )
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  }
  else
  {
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  }


  SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, 0 )], m_RenderStates[std::make_pair( RS_ALPHABLENDING, 0 )] );

  glViewport( m_ViewPort.X, VisibleHeight() - m_ViewPort.Height - m_ViewPort.Y, m_ViewPort.Width, m_ViewPort.Height );
  glScissor( m_ViewPort.X, VisibleHeight() - m_ViewPort.Height - m_ViewPort.Y, m_ViewPort.Width, m_ViewPort.Height );
  glDepthRange( m_ViewPort.MinZ, m_ViewPort.MaxZ );
  if ( ( m_ViewPort.X != 0 )
  ||   ( m_ViewPort.Y != 0 )
  ||   ( m_ViewPort.Width != VisibleWidth() )
  ||   ( m_ViewPort.Height != VisibleHeight() ) )
  {
    glEnable( GL_SCISSOR_TEST );
  }
  else
  {
    glDisable( GL_SCISSOR_TEST );
  }
  if ( m_RenderStates[std::make_pair( RS_LIGHTING, 0 )] == XRenderer::RSV_ENABLE )
  {
    glEnable( GL_LIGHTING );
    for ( int i = 0; i < 8; ++i )
    {
      if ( m_RenderStates[std::make_pair( RS_LIGHT, i )] == XRenderer::RSV_ENABLE )
      {
        glEnable( GL_LIGHT0 + i );
      }
    }
  }

  if ( m_RenderStates[std::make_pair( RS_ZBUFFER, 0 )] == XRenderer::RSV_ENABLE )
  {
    glEnable( GL_DEPTH_TEST );
  }
  if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == XRenderer::RSV_ENABLE )
  {
    glEnable( GL_FOG );
  }

  CheckError( "Set3dMode" );
}



int OpenGLRenderClass::VisibleWidth()
{
  if ( m_pSetRenderTargetTexture != NULL )
  {
    return m_SetRenderTargetTextureSize.x;
  }
  return Width();
}



int OpenGLRenderClass::VisibleHeight()
{
  if ( m_pSetRenderTargetTexture != NULL )
  {
    return m_SetRenderTargetTextureSize.y;
  }
  return Height();
}



bool OpenGLRenderClass::IsExtensionSupported( const GR::String& TargetExtension )
{
  const unsigned char*  pszExtensions = NULL;
  const unsigned char*  pszStart;
  unsigned char*        pszWhere;
  unsigned char*        pszTerminator;


  // Extension names should not have spaces
  pszWhere = (unsigned char*)strchr( TargetExtension.c_str(), ' ' );
  if ( ( pszWhere )
  ||   ( TargetExtension.empty() ) )
  {
    return false;
  }

  // Get Extensions String
  pszExtensions = glGetString( GL_EXTENSIONS );

  // Search The Extensions String For An Exact Copy
  pszStart = pszExtensions;
  for ( ;; )
  {
    pszWhere = (unsigned char*)strstr( (const char*)pszStart, TargetExtension.c_str() );
    if ( !pszWhere )
    {
      break;
    }
    pszTerminator = pszWhere + TargetExtension.length();
    if ( ( pszWhere == pszStart )
    ||   ( *( pszWhere - 1 ) == ' ' ) )
    {
      if ( ( *pszTerminator == ' ' )
      ||   ( *pszTerminator == '\0' ) )
      {
        return true;
      }
    }
    pszStart = pszTerminator;
  }
  return false;
}



void OpenGLRenderClass::RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                       GR::f32 TU1, GR::f32 TV1,
                                       GR::f32 TU2, GR::f32 TV2,
                                       GR::f32 TU3, GR::f32 TV3,
                                       GR::f32 TU4, GR::f32 TV4,
                                       GR::u32 Color1, GR::u32 Color2,
                                       GR::u32 Color3, GR::u32 Color4, float Z )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  X += m_DisplayOffset.x;
  Y += m_DisplayOffset.y;

  Set2DMode();
  CheckError( "RenderQuad2d a" );

  glBegin( GL_QUADS );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
	glVertex3i( X, Y, (int)Z );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
  glVertex3i( X + Width, Y, (int)Z );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU4, TV4 );
  }
  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color4 & 0xff000000 ) >> 24 );
  glVertex3i( X + Width, Y + Height, (int)Z );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
  glVertex3i( X, Y + Height, (int)Z );

  glEnd();
  CheckError( "RenderQuad2d x" );
}



void OpenGLRenderClass::RenderQuadDetail2d( GR::f32 X, GR::f32 Y, GR::f32 Width, GR::f32 Height,
                                       GR::f32 TU1, GR::f32 TV1,
                                       GR::f32 TU2, GR::f32 TV2,
                                       GR::f32 TU3, GR::f32 TV3,
                                       GR::f32 TU4, GR::f32 TV4,
                                       GR::u32 Color1, GR::u32 Color2,
                                       GR::u32 Color3, GR::u32 Color4, float Z )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  Set2DMode();

  X += m_DisplayOffset.x;
  Y += m_DisplayOffset.y;

  glBegin( GL_QUADS );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
	glVertex3f( (GLfloat)X, (GLfloat)Y, Z );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glVertex3f( (GLfloat)X + Width, (GLfloat)Y, Z );

  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU4, TV4 );
  }
  glVertex3f( (GLfloat)X + Width, (GLfloat)Y + Height, Z );

  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glVertex3f( (GLfloat)X, (GLfloat)Y, Z );

  glEnd();
  CheckError( "RenderQuadDetail2d" );
}



void OpenGLRenderClass::RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1,
                                             GR::f32 X2, GR::f32 Y2,
                                             GR::f32 X3, GR::f32 Y3,
                                             GR::f32 X4, GR::f32 Y4,
                                             GR::f32 TU1, GR::f32 TV1,
                                             GR::f32 TU2, GR::f32 TV2,
                                             GR::f32 TU3, GR::f32 TV3,
                                             GR::f32 TU4, GR::f32 TV4,
                                             GR::u32 Color1, GR::u32 Color2,
                                             GR::u32 Color3, GR::u32 Color4, float Z )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  Set2DMode();

  glBegin( GL_QUADS );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
	glVertex3f( X1, Y1, Z );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
	glVertex3f( X2, Y2, Z );

  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU4, TV4 );
  }
	glVertex3f( X4, Y4, Z );

  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
	glVertex3f( X3, Y3, Z );

  glEnd();
  CheckError( "RenderQuadDetail2d 2" );
}



void OpenGLRenderClass::RenderQuad( const GR::tVector& ptPos1,
                                    const GR::tVector& ptPos2,
                                    const GR::tVector& ptPos3,
                                    const GR::tVector& ptPos4,
                                    GR::f32 TU1, GR::f32 TV1,
                                    GR::f32 TU2, GR::f32 TV2,
                                    GR::u32 Color1, GR::u32 Color2,
                                    GR::u32 Color3, GR::u32 Color4 )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );

  normal.normalize();

  Set3DMode();
  glBegin( GL_QUADS );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos1.x, ptPos1.y, ptPos1.z );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV1 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos2.x, ptPos2.y, ptPos2.z );

  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos4.x, ptPos4.y, ptPos4.z );

  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV2 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos3.x, ptPos3.y, ptPos3.z );

  glEnd();
  CheckError( "RenderQuad" );
}



void OpenGLRenderClass::RenderQuad( const GR::tVector& ptPos1,
                                    const GR::tVector& ptPos2,
                                    const GR::tVector& ptPos3,
                                    const GR::tVector& ptPos4,
                                    GR::f32 TU1, GR::f32 TV1,
                                    GR::f32 TU2, GR::f32 TV2,
                                    GR::f32 TU3, GR::f32 TV3,
                                    GR::f32 TU4, GR::f32 TV4,
                                    GR::u32 Color1, GR::u32 Color2,
                                    GR::u32 Color3, GR::u32 Color4 )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );

  normal.normalize();

  Set3DMode();
  glBegin( GL_QUADS );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos1.x, ptPos1.y, ptPos1.z );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos2.x, ptPos2.y, ptPos2.z );

  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU4, TV4 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos4.x, ptPos4.y, ptPos4.z );

  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos3.x, ptPos3.y, ptPos3.z );

  glEnd();
  CheckError( "RenderQuad 2" );
}



void OpenGLRenderClass::RenderTriangle2d( const GR::tPoint& pt1, const GR::tPoint& pt2, const GR::tPoint& pt3,
                                           GR::f32 TU1, GR::f32 TV1,
                                           GR::f32 TU2, GR::f32 TV2,
                                           GR::f32 TU3, GR::f32 TV3,
                                           GR::u32 Color1, GR::u32 Color2,
                                           GR::u32 Color3, float Z )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  Set2DMode();

  glBegin( GL_TRIANGLES );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  glVertex2f( m_DisplayOffset.x + (GLfloat)pt1.x, m_DisplayOffset.y + (GLfloat)pt1.y );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
  glVertex2f( m_DisplayOffset.x + (GLfloat)pt2.x, m_DisplayOffset.y + (GLfloat)pt2.y );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
  glVertex2f( m_DisplayOffset.x + (GLfloat)pt3.x, m_DisplayOffset.y + (GLfloat)pt3.y );

  glEnd();
  CheckError( "RenderTriangle2d" );
}



void OpenGLRenderClass::RenderTriangle( const GR::tVector& ptPos1,
                                         const GR::tVector& ptPos2,
                                         const GR::tVector& ptPos3,
                                         GR::f32 TU1, GR::f32 TV1,
                                         GR::f32 TU2, GR::f32 TV2,
                                         GR::f32 TU3, GR::f32 TV3,
                                         GR::u32 Color1, GR::u32 Color2,
                                         GR::u32 Color3 )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );

  normal.normalize();

  Set3DMode();

  glBegin( GL_TRIANGLES );
  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU1, TV1 );
  }
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos1.x, ptPos1.y, ptPos1.z );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU2, TV2 );
  }
  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos2.x, ptPos2.y, ptPos2.z );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
  glNormal3f( normal.x, normal.y, normal.z );
  glVertex3f( ptPos3.x, ptPos3.y, ptPos3.z );

  glEnd();
  CheckError( "RenderTriangle2d" );
}



void OpenGLRenderClass::CheckError( const GR::String& Function )
{
  int error = glGetError();
  if ( error != 0 )
  {
#if OPERATING_SYSTEM == OS_WEB 
    dh::Log( "OpenGL error %d at %s", error, Function.c_str() );
#else
    GR::String errMessage = (GR::Char*)gluErrorString( error );
    dh::Log( "OpenGL error %d (%s) at %s", error, errMessage.c_str(), Function.c_str() );
#endif
  }
}



XTexture* OpenGLRenderClass::LoadTexture( const char* FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  GR::String     path = FileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), FormatToConvert, ColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData, MipMapLevels );
  if ( pTexture == NULL )
  {
    delete pData;
    return NULL;
  }

  Log( "Renderer.Full", "Texture (%s) loaded, loaded format %d, created format %d",
       path.c_str(), pData->ImageFormat(), pTexture->m_ImageFormat );

  pTexture->m_ColorKey = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  pTexture->m_LoadedFromFile = path;

  delete pData;
  return pTexture;
}



XTexture* OpenGLRenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  CheckError( "CreateTexture pre pre 1" );

  OpenGLTexture*   pTexture = new OpenGLTexture( this, AllowUsageAsRenderTarget );

  glGenTextures( 1, &pTexture->m_TextureID );
  if ( pTexture->m_TextureID == 0 )
  {
    return NULL;
  }

  pTexture->m_ImageFormat = ImageData.ImageFormat();
  pTexture->m_SurfaceSize.Set( ImageData.Width(), ImageData.Height() );
  pTexture->m_ImageSourceSize.Set( ImageData.Width(), ImageData.Height() );

  if ( AllowUsageAsRenderTarget )
  {
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers( 1, &pTexture->m_RenderTargetFrameBufferID );
    glBindFramebuffer( GL_FRAMEBUFFER, pTexture->m_RenderTargetFrameBufferID );

    // Give an empty image to OpenGL ( the last "0" means "empty" )
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ImageData.Width(), ImageData.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );

    // Poor filtering
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glGenRenderbuffers( 1, &pTexture->m_RenderBufferDepthStencil );
    glBindRenderbuffer( GL_RENDERBUFFER, pTexture->m_RenderBufferDepthStencil );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, ImageData.Width(), ImageData.Height() );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pTexture->m_RenderBufferDepthStencil );

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture->m_TextureID, 0 );

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, DrawBuffers ); // "1" is the size of DrawBuffers

    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      dh::Log( "glCheckFramebufferStatus returns not complete!" );
    }

    CheckError( "glCheckFramebufferStatus" );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
  else
  {
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

    CheckError( "CreateTexture 1" );

    // Give the image to OpenGL
    switch ( ImageData.ImageFormat() )
    {
      case GR::Graphic::IF_R8G8B8:
#if OPERATING_SYSTEM == OS_WEB
        {
          GR::Graphic::ImageData      convertedImage( ImageData );
          convertedImage.ConvertSelfTo( GR::Graphic::IF_B8G8R8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, convertedImage.Data() );
        }
#else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ImageData.Width(), ImageData.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, ImageData.Data() );
#endif
        break;
      case GR::Graphic::IF_A8R8G8B8:
#if OPERATING_SYSTEM == OS_WEB
        {
          GR::Graphic::ImageData      convertedImage( ImageData );
          convertedImage.ConvertSelfTo( GR::Graphic::IF_A8B8G8R8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage.Data() );
        }
#else
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ImageData.Width(), ImageData.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, ImageData.Data() );
#endif
        break;
      default:
        {
          GR::Graphic::ImageData      convertedImage( ImageData );

          if ( GR::Graphic::ImageData::FormatHasAlpha( ImageData.ImageFormat() ) )
          {
#if OPERATING_SYSTEM == OS_WEB
            convertedImage.ConvertSelfTo( GR::Graphic::IF_A8B8G8R8 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage.Data() );
#else
            convertedImage.ConvertSelfTo( GR::Graphic::IF_A8R8G8B8 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, convertedImage.Data() );
#endif
          }
          else
          {
#if OPERATING_SYSTEM == OS_WEB
            convertedImage.ConvertSelfTo( GR::Graphic::IF_B8G8R8 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, convertedImage.Data() );
#else
            convertedImage.ConvertSelfTo( GR::Graphic::IF_R8G8B8 );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, convertedImage.Data() );
#endif
          }
        }
        break;
    }
    CheckError( "CreateTexture 2x" );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
  CheckError( "CreateTexture" );
  return pTexture;
}



XTexture* OpenGLRenderClass::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  OpenGLTexture*   pTexture = new OpenGLTexture( this, AllowUsageAsRenderTarget );

  glGenTextures( 1, &pTexture->m_TextureID );

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

  GR::Graphic::ContextDescriptor    dummyImage;

  dummyImage.CreateData( Width, Height, imgFormat );

  pTexture->m_ImageFormat = imgFormat;
  pTexture->m_SurfaceSize.Set( Width, Height );
  pTexture->m_ImageSourceSize.Set( Width, Height );

  if ( AllowUsageAsRenderTarget )
  {
    dummyImage.CreateData( Width, Height, GR::Graphic::IF_A8R8G8B8 );
    dummyImage.Box( 0, 0, Width, Height, 0x00000000 );

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers( 1, &pTexture->m_RenderTargetFrameBufferID );

    glBindFramebuffer( GL_FRAMEBUFFER, pTexture->m_RenderTargetFrameBufferID );

    // Give an empty image to OpenGL ( the last "0" means "empty" )
#if OPERATING_SYSTEM == OS_WEB
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
#else
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_BGR, GL_UNSIGNED_BYTE, 0 );
#endif

    // Poor filtering
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

#if OPERATING_SYSTEM != OS_WEB
    glGenRenderbuffers( 1, &pTexture->m_RenderBufferDepthStencil );
    glBindRenderbuffer( GL_RENDERBUFFER, pTexture->m_RenderBufferDepthStencil );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width, Height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pTexture->m_RenderBufferDepthStencil );
#endif
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture->m_TextureID, 0 );

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, DrawBuffers ); // "1" is the size of DrawBuffers

    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
      dh::Log( "glCheckFramebufferStatus returns not complete!" );
    }

    CheckError( "glCheckFramebufferStatus" );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  // Give the image to OpenGL
  CopyDataToTexture( pTexture, dummyImage );

  CheckError( "CreateTexture 2" );
  return pTexture;
}



void OpenGLRenderClass::SetTexture( GR::u32 Stage, XTexture* pTexture )
{
  if ( m_pSetTextures[Stage] == pTexture )
  {
    return;
  }
  m_pSetTextures[Stage] = pTexture;
  if ( pTexture == NULL )
  {
    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable( GL_TEXTURE_2D );
  }
  else
  {
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, ( (OpenGLTexture*)pTexture )->m_TextureID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  }
  CheckError( "SetTexture" );
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  OpenGLVertexBuffer* pBuffer = new OpenGLVertexBuffer( this );

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    CheckError( "CreateVertexBuffer" );

    delete pBuffer;
    return NULL;
  }

  CheckError( "CreateVertexBuffer" );
  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  OpenGLVertexBuffer* pBuffer = new OpenGLVertexBuffer( this );
  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    CheckError( "CreateVertexBuffer 2" );
    delete pBuffer;
    return NULL;
  }
  CheckError( "CreateVertexBuffer 2" );
  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( const Mesh::IMesh& Mesh, GR::u32 VertexFormat )
{
  XVertexBuffer* pBuffer = CreateVertexBuffer( Mesh.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( Mesh );

  return pBuffer;
}



void OpenGLRenderClass::DestroyVertexBuffer( XVertexBuffer* pBuffer )
{
  if ( pBuffer == NULL )
  {
    dh::Log( "OpenGLRenderClass::DestroyVertexBuffer Buffer is NULL!" );
    return;
  }
  pBuffer->Release();
  delete pBuffer;
  m_VertexBuffers.remove( pBuffer );
}



bool OpenGLRenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  if ( pBuffer == NULL )
  {
    dh::Log( "OpenGLRenderClass::RenderVertexBuffer Buffer is NULL!" );
    return false;
  }

  OpenGLVertexBuffer* pOpenGLBuffer = (OpenGLVertexBuffer*)pBuffer;

  Set3DMode();
  return pOpenGLBuffer->Display( Index, Count );
}



bool OpenGLRenderClass::BeginScene()
{
  return m_Ready;
}



void OpenGLRenderClass::EndScene()
{
}



void OpenGLRenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  glFlush();
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  SDL_GL_SwapWindow( m_pMainWindow );
#else
  SwapBuffers( m_DC );
#endif
}



bool OpenGLRenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  return false;
}



bool OpenGLRenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, ( (OpenGLTexture*)pTexture )->m_TextureID );
  CheckError( "CopyDataToTexture bind" );

  // Give the image to OpenGL
  switch ( ImageData.ImageFormat() )
  {
    case GR::Graphic::IF_R8G8B8:
#if OPERATING_SYSTEM == OS_WEB
      {
        GR::Graphic::ImageData    convertedImage( ImageData );
        convertedImage.ConvertSelfTo( GR::Graphic::IF_B8G8R8 );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, convertedImage.Data() );
      }
#else
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ImageData.Width(), ImageData.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, ImageData.Data() );
#endif
      CheckError( "CopyDataToTexture teximage2d r8g8b8" );
      break;
    case GR::Graphic::IF_A8R8G8B8:
#if OPERATING_SYSTEM == OS_WEB
      {
        GR::Graphic::ImageData    convertedImage( ImageData );
        convertedImage.ConvertSelfTo( GR::Graphic::IF_A8B8G8R8 );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage.Data() );
      }
#else
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ImageData.Width(), ImageData.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, ImageData.Data() );
#endif
      CheckError( "CopyDataToTexture teximage2d a8r8g8b8" );
      break;
    default:
      {
        GR::Graphic::ImageData      convertedImage( ImageData );

        if ( GR::Graphic::ImageData::FormatHasAlpha( ImageData.ImageFormat() ) )
        {
#if OPERATING_SYSTEM == OS_WEB
          convertedImage.ConvertSelfTo( GR::Graphic::IF_A8B8G8R8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, convertedImage.Data() );
#else
          convertedImage.ConvertSelfTo( GR::Graphic::IF_A8R8G8B8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, convertedImage.Data() );
#endif
          CheckError( "CopyDataToTexture teximage2d ar8g8b8 1" );
        }
        else
        {
#if OPERATING_SYSTEM == OS_WEB
          convertedImage.ConvertSelfTo( GR::Graphic::IF_B8G8R8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, convertedImage.Data() );
#else
          convertedImage.ConvertSelfTo( GR::Graphic::IF_R8G8B8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, convertedImage.Data() );
#endif
          CheckError( "CopyDataToTexture teximage2d r8g8b8 2" );
        }
      }
      break;
  }
  glBindTexture( GL_TEXTURE_2D, 0 );
  return true;
}



void OpenGLRenderClass::RestoreAllTextures()
{
}



bool OpenGLRenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  if ( ( imgFormat == GR::Graphic::IF_A8R8G8B8 )
  ||   ( imgFormat == GR::Graphic::IF_R8G8B8 ) )
  {
    return true;
  }
  return false;
}



bool OpenGLRenderClass::OnResized()
{
#if OPERATING_SYSTEM == OS_WINDOWS
  RECT    rc;

  GetClientRect( m_hwndViewport, &rc );


  glViewport( 0, 0, rc.right - rc.left, rc.bottom - rc.top );
#endif
  return true;
}



GR::Graphic::eImageFormat OpenGLRenderClass::ImageFormat()
{
  return GR::Graphic::ImageData::ImageFormatFromDepth( m_Depth );
}



GR::u32 OpenGLRenderClass::Width()
{
  return m_Width;
}



GR::u32 OpenGLRenderClass::Height()
{
  return m_Height;
}



bool OpenGLRenderClass::IsReady() const
{
  return m_Ready;
}



void OpenGLRenderClass::Clear( bool bClearColor, bool bClearZ )
{
  int   flags = 0;

  if ( bClearColor )
  {
    flags |= GL_COLOR_BUFFER_BIT;
  }
  if ( bClearZ )
  {
    flags |= GL_DEPTH_BUFFER_BIT;
  }
  if ( flags )
  {
    glClearColor( ( ( m_ClearColor & 0x00ff0000 ) >> 16 ) / 255.0f,
                  ( ( m_ClearColor & 0x0000ff00 ) >>  8 ) / 255.0f,
                  ( ( m_ClearColor & 0x000000ff )       ) / 255.0f,
                  0.0f );
    glClearDepth( 1.0 );
    glClear( flags );
    CheckError( "Clear" );
  }
}



void OpenGLRenderClass::SetAlpha( GR::u32 AlphaTest, GR::u32 AlphaBlend )
{
  if ( ( AlphaBlend == RSV_ENABLE )
  ||   ( AlphaTest == RSV_ENABLE ) )
  {
    glEnable( GL_BLEND );
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
    //glAlphaFunc( GL_GREATER, 0.f );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  else
  {
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glBlendFunc( GL_ONE, GL_ZERO );
  }
}



bool OpenGLRenderClass::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
{
  tMapRenderStates::iterator    it( m_RenderStates.find( std::make_pair( rState, Stage ) ) );
  if ( it != m_RenderStates.end() )
  {
    if ( it->second == rValue )
    {
      return true;
    }
  }

  bool    bSupported = true;

  switch ( rState )
  {
    case RS_ZWRITE:
      if ( rValue == RSV_ENABLE )
      {
        glDepthMask( GL_TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        glDepthMask( GL_FALSE );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_MINFILTER:
      if ( m_pSetTextures[0] )
      {
        if ( rValue == RSV_FILTER_POINT )
        {
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        }
        else
        {
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        }
      }
      break;
    case RS_MAGFILTER:
      if ( m_pSetTextures[0] )
      {
        if ( rValue == RSV_FILTER_POINT )
        {
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        }
        else
        {
          glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }
      }
      break;
    case RS_FOG_COLOR:
      {
        GR::f32     color[4];

        color[0] = ( rValue & 0xff ) / 255.0f;
        color[1] = ( ( rValue >> 8 ) & 0xff ) / 255.0f;
        color[2] = ( ( rValue >> 16 ) & 0xff ) / 255.0f;
        color[3] = ( ( rValue >> 24 ) & 0xff ) / 255.0f;

        glFogfv( GL_FOG_COLOR, color );
      }
      break;
    case RS_FOG_DENSITY:
      glFogf( GL_FOG_DENSITY, *( GR::f32* ) & rValue );
      break;
    case RS_FOG_START:
      {
        GR::f32   fogValue = *( GR::f32* ) & rValue;
        glFogf( GL_FOG_START, fogValue );

        glFogf( GL_FOG_MODE, GL_LINEAR );
      }
      break;
    case RS_FOG_END:
      {
        GR::f32   fogValue = *( GR::f32* ) & rValue;
        glFogf( GL_FOG_END, fogValue );
      }
      break;
    case RS_FOG_ENABLE:
      if ( rValue == RSV_ENABLE )
      {
        glEnable( GL_FOG );
      }
      else if ( rValue == RSV_DISABLE )
      {
        glDisable( GL_FOG );
      }
      break;
    case RS_FOG_TABLEMODE:
      if ( rValue == RSV_FOG_NONE )
      {
        glFogf( GL_FOG_MODE, GL_NONE );
      }
      else if ( rValue == RSV_FOG_EXP )
      {
        glFogf( GL_FOG_MODE, GL_EXP );
      }
      else if ( rValue == RSV_FOG_EXP_SQUARED )
      {
        glFogf( GL_FOG_MODE, GL_EXP2 );
      }
      else if ( rValue == RSV_FOG_LINEAR )
      {
        glFogf( GL_FOG_MODE, GL_LINEAR );
      }
#if OPERATING_SYSTEM != OS_WEB
      glHint( GL_FOG_HINT, GL_NICEST );
#endif
      break;
    case RS_FOG_VERTEXMODE:
      glHint( GL_FOG_HINT, GL_FASTEST );
      /*
      // no way to match?
      if ( rValue == RSV_FOG_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_NONE );
      }
      else if ( rValue == RSV_FOG_EXP )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_EXP );
      }
      else if ( rValue == RSV_FOG_EXP_SQUARED )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_EXP2 );
      }
      else if ( rValue == RSV_FOG_LINEAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR );
      }*/
      break;
    case RS_TEXTURE_TRANSFORM:
      if ( rValue == RSV_TEXTURE_TRANSFORM_COUNT2 )
      {
      }
      else if ( rValue == RSV_DISABLE )
      {
        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ZBUFFER:
      if ( rValue == RSV_ENABLE )
      {
        glEnable( GL_DEPTH_TEST );
      }
      else if ( rValue == RSV_DISABLE )
      {
        glDisable( GL_DEPTH_TEST );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_LIGHT:
      if ( rValue == RSV_ENABLE )
      {
        glEnable( GL_LIGHT0 + Stage );
      }
      else if ( rValue == RSV_DISABLE )
      {
        glDisable( GL_LIGHT0 + Stage );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_LIGHTING:
      if ( rValue == RSV_ENABLE )
      {
        glEnable( GL_LIGHTING );
      }
      else if ( rValue == RSV_DISABLE )
      {
        glDisable( GL_LIGHTING );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_AMBIENT:
      // no support for this in legacy opengl in emscripten
#if OPERATING_SYSTEM != OS_WEB
      {
        GR::f32     color[4];

        color[2] =   ( rValue & 0xff ) / 255.0f;
        color[1] = ( ( rValue >> 8 ) & 0xff ) / 255.0f;
        color[0] = ( ( rValue >> 16 ) & 0xff ) / 255.0f;
        color[3] = ( ( rValue >> 24 ) & 0xff ) / 255.0f;

        glLightModelfv( GL_LIGHT_MODEL_AMBIENT, color );
      }
#endif
      break;
    case RS_COLOR_OP:
      if ( rValue == RSV_MODULATE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE );
        
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
      }
      else if ( rValue == RSV_DISABLE )
      {
      }
      /*
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
      }
      */
      else
      {
        bSupported = false;
      }
      break;
    case RS_COLOR_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_COLOR_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_CLEAR_COLOR:
      m_ClearColor = rValue;
      break;
    case RS_ALPHA_OP:
      if ( rValue == RSV_MODULATE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE );
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
      }
      else if ( rValue == RSV_DISABLE )
      {
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ALPHA_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ALPHA_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PREVIOUS );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE );
        glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_SRC_BLEND:
      if ( rValue == RSV_SRC_ALPHA )
      {
        m_SrcBlend = GL_SRC_ALPHA;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ONE )
      {
        m_SrcBlend = GL_ONE;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ZERO )
      {
        m_SrcBlend = GL_ZERO;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_DEST_BLEND:
      if ( rValue == RSV_INV_SRC_ALPHA )
      {
        m_DestBlend = GL_ONE_MINUS_SRC_ALPHA;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ONE )
      {
        m_DestBlend = GL_ONE;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ZERO )
      {
        m_DestBlend = GL_ZERO;
        glEnable( GL_BLEND );
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_CULLMODE:
      if ( rValue == RSV_CULL_NONE )
      {
        glDisable( GL_CULL_FACE );
      }
      else if ( rValue == RSV_CULL_CCW )
      {
        CheckError( "CullMode a" );
        glEnable( GL_CULL_FACE );
        CheckError( "CullMode a2" );
        glFrontFace( GL_CW );
        CheckError( "CullMode a3" );
      }
      else if ( rValue == RSV_CULL_CW )
      {
        glEnable( GL_CULL_FACE );
        glFrontFace( GL_CCW );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ALPHATEST:
      if ( rValue == RSV_ENABLE )
      {
        SetAlpha( rValue, m_RenderStates[std::make_pair( RS_ALPHABLENDING, Stage )] );
      }
      else if ( rValue == RSV_DISABLE )
      {
        SetAlpha( rValue, m_RenderStates[std::make_pair( RS_ALPHABLENDING, Stage )] );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ALPHABLENDING:
      if ( rValue == RSV_ENABLE )
      {
        SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, Stage )], rValue );
      }
      else if ( rValue == RSV_DISABLE )
      {
        SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, Stage )], rValue );
      }
      else
      {
        bSupported = false;
      }
      break;
    case RS_ALPHAREF:
      m_AlphaRefValue = (float)rValue / 255.0f;
      glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
      break;
    case RS_ALPHAFUNC:
      if ( rValue == RSV_COMPARE_GREATEREQUAL )
      {
        m_AlphaFuncValue = GL_GEQUAL;
        glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
      }
      else
      {
        bSupported = false;
      }
      break;
  }

  m_RenderStates[std::make_pair( rState, Stage )] = rValue;
  if ( !bSupported )
  {
    dh::Log( "OpenGLRenderClass::SetState not supported (%d - %d - Stage %d)", rState, rValue, Stage );
  }

  CheckError( Misc::Format( "OpenGLRenderClass::SetState (%1% - %2% - Stage %3%)" ) << rState << rValue << Stage );
  return bSupported;
}



void OpenGLRenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{
  m_Transform[tType] = matTrix;
  switch ( tType )
  {
    case TT_PROJECTION:
      if ( !m_2dModeActive )
      {
        glMatrixMode( GL_PROJECTION );

        if ( m_pSetRenderTargetTexture != NULL )
        {
          math::matrix4   matProj = matTrix;

          matProj.ms._12 = -matProj.ms._12;
          matProj.ms._22 = -matProj.ms._22;
          matProj.ms._32 = -matProj.ms._32;
          matProj.ms._42 = -matProj.ms._42;

          glLoadMatrixf( (GLfloat*)&matProj );
        }
        else
        {
          glLoadMatrixf( (GLfloat*)&matTrix );
        }
      }
      break;
    case TT_WORLD:
    case TT_VIEW:
      if ( !m_2dModeActive )
      {
        glMatrixMode( GL_MODELVIEW );

        math::matrix4   matModelView = m_Transform[TT_WORLD] * m_Transform[TT_VIEW];

        glLoadMatrixf( (GLfloat*)&matModelView );
      }
      break;
    case TT_TEXTURE_STAGE_0:
      {
        glMatrixMode( GL_TEXTURE );

        math::matrix4   matTrans;

        matTrans.Translation( matTrix.ms._31, matTrix.ms._32, 0.0f );

        glLoadMatrixf( (GLfloat*)&matTrans );
      }
      break;
  }
}



bool OpenGLRenderClass::SetViewport( const XViewport& Viewport )
{
  //dh::Log( "SetViewport %d,%d %dx%d", Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );
  if ( !m_2dModeActive )
  {
    glViewport( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
    CheckError( "SetViewport glViewport 3d" );
  }
  else
  {
    // viewport needs to be full size, otherwise everything is scaled (facepalm)
    glViewport( 0, 0, VisibleWidth(), VisibleHeight() );
    CheckError( "SetViewport glViewport 2d" );

    glScissor( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
    CheckError( "SetViewport glScissor" );
    if ( ( Viewport.X != 0 )
    ||   ( Viewport.Y != 0 )
    ||   ( Viewport.Width != VisibleWidth() )
    ||   ( Viewport.Height != VisibleHeight() ) )
    {
      glEnable( GL_SCISSOR_TEST );
    }
    else
    {
      glDisable( GL_SCISSOR_TEST );
    }
  }

  m_ViewPort.X      = Viewport.X;
  m_ViewPort.Y      = Viewport.Y;
  m_ViewPort.Width  = Viewport.Width;
  m_ViewPort.Height = Viewport.Height;

  m_ViewPort.MinZ   = Viewport.MinZ;
  m_ViewPort.MaxZ   = Viewport.MaxZ;
  return true;
}



bool OpenGLRenderClass::SetTrueViewport( const XViewport& Viewport )
{
  dh::Log( "SetTrueViewport %d,%d %dx%d", Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );

  if ( !m_2dModeActive )
  {
    glViewport( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
  }
  else
  {
    glViewport( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
    glScissor( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
    if ( ( Viewport.X != 0 )
    ||   ( Viewport.Y != 0 )
    ||   ( Viewport.Width != VisibleWidth() )
    ||   ( Viewport.Height != VisibleHeight() ) )
    {
      glEnable( GL_SCISSOR_TEST );
    }
    else
    {
      glDisable( GL_SCISSOR_TEST );
    }
  }

  m_ViewPort.X      = Viewport.X;
  m_ViewPort.Y      = Viewport.Y;
  m_ViewPort.Width  = Viewport.Width;
  m_ViewPort.Height = Viewport.Height;

  m_ViewPort.MinZ   = Viewport.MinZ;
  m_ViewPort.MaxZ   = Viewport.MaxZ;
  return true;
}



bool OpenGLRenderClass::ToggleFullscreen()
{
  return false;
}



bool OpenGLRenderClass::IsFullscreen()
{
  return m_FullScreen;
}



bool OpenGLRenderClass::SetMode( XRendererDisplayMode& DisplayMode )
{
  return false;
}



void OpenGLRenderClass::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float Z )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  Set2DMode();

  glBegin( GL_LINES );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
	glVertex2f( m_DisplayOffset.x + m_Canvas.Left + pt1.x * virtualX, m_DisplayOffset.y + m_Canvas.Top + pt1.y * virtualY );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
	glVertex2f( m_DisplayOffset.x + m_Canvas.Left + pt2.x * virtualX, m_DisplayOffset.y + m_Canvas.Top + pt2.y * virtualY );

  glEnd();
  CheckError( "RenderLine2d" );
}



void OpenGLRenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
{
}




bool OpenGLRenderClass::SetLight( GR::u32 LightIndex, XLight& Light )
{
  CheckError( "SetLight start" );

  glMatrixMode( GL_MODELVIEW );
  math::matrix4   matModelView = m_Transform[TT_VIEW];
  glLoadMatrixf( (GLfloat*)&matModelView );

  switch ( Light.m_Type )
  {
    case XLight::LT_DIRECTIONAL:
      {
        GR::f32   dir[4];

        memcpy( dir, &Light.m_Direction, 3 * sizeof( GR::f32 ) );
        //TODO - Direction!
        dir[0] = -dir[0];
        dir[1] = -dir[1];
        dir[2] = -dir[2];
        // w = 0.0f means directional
        dir[3] = 0.0f;
        glLightfv( GL_LIGHT0 + LightIndex, GL_POSITION, dir );

        // Emscripten has no glLightf
        GR::f32     exponent = 0.0f;
        GR::f32     cutOff = 180.0f;
        glLightfv( GL_LIGHT0 + LightIndex, GL_SPOT_EXPONENT, &exponent );
        glLightfv( GL_LIGHT0 + LightIndex, GL_SPOT_CUTOFF, &cutOff );
      }
      break;
    case XLight::LT_POINT:
      {
        GR::f32   dir[4];

        memcpy( dir, &Light.m_Position, 3 * sizeof( GR::f32 ) );
        dir[3] = 1.0f;
        glLightfv( GL_LIGHT0 + LightIndex, GL_POSITION, dir );
      }
      break;
    case XLight::LT_SPOT:
      {
        GR::f32   dir[4];

        memcpy( dir, &Light.m_Position, 3 * sizeof( GR::f32 ) );
        dir[3] = 1.0f;
        glLightfv( GL_LIGHT0 + LightIndex, GL_POSITION, dir );
      }
      break;
  }

  CheckError( "SetLight glLightfv pos" );

  GR::f32   color[4];
  color[0] = ( ( Light.m_Ambient >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Ambient >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Ambient & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Ambient >> 24 ) & 0xff ) / 255.0f;
  glLightfv( GL_LIGHT0 + LightIndex, GL_AMBIENT, color );
  CheckError( "SetLight GL_AMBIENT" );

  color[0] = ( ( Light.m_Diffuse >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Diffuse >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Diffuse & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Diffuse >> 24 ) & 0xff ) / 255.0f;
  glLightfv( GL_LIGHT0 + LightIndex, GL_DIFFUSE, color );
  CheckError( "SetLight GL_DIFFUSE" );

  color[0] = ( ( Light.m_Specular >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Specular >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Specular & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Specular >> 24 ) & 0xff ) / 255.0f;
  glLightfv( GL_LIGHT0 + LightIndex, GL_SPECULAR, color );
  CheckError( "SetLight GL_SPECULAR" );

  CheckError( "SetLight GL_SPECULAR" );

  // restore matrices !??
  glMatrixMode( GL_MODELVIEW );
  if ( m_2dModeActive )
  {
    glLoadIdentity();
  }
  else
  {
    math::matrix4   matModelView = m_Transform[TT_WORLD] * m_Transform[TT_VIEW];

    glLoadMatrixf( (GLfloat*)&matModelView );
  }
  return false;
}



bool OpenGLRenderClass::SetMaterial( const XMaterial& Material )
{
#if OPERATING_SYSTEM == OS_WEB
  // no support for glMaterialfv
  return true;
#endif

  GR::f32     color[4];

  color[2] =   ( Material.Ambient & 0xff ) / 255.0f;
  color[1] = ( ( Material.Ambient >> 8 ) & 0xff ) / 255.0f;
  color[0] = ( ( Material.Ambient >> 16 ) & 0xff ) / 255.0f;
  color[3] = ( ( Material.Ambient >> 24 ) & 0xff ) / 255.0f;
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, color );

  color[2] =   ( Material.Diffuse & 0xff ) / 255.0f;
  color[1] = ( ( Material.Diffuse >> 8 ) & 0xff ) / 255.0f;
  color[0] = ( ( Material.Diffuse >> 16 ) & 0xff ) / 255.0f;
  color[3] = ( ( Material.Diffuse >> 24 ) & 0xff ) / 255.0f;
  glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, color );

  color[2] =   ( Material.Specular & 0xff ) / 255.0f;
  color[1] = ( ( Material.Specular >> 8 ) & 0xff ) / 255.0f;
  color[0] = ( ( Material.Specular >> 16 ) & 0xff ) / 255.0f;
  color[3] = ( ( Material.Specular >> 24 ) & 0xff ) / 255.0f;
  glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, color );

  // is that correct?
  GR::f32     power = Material.Power * 128.0f;
  glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, &power );

  color[2] = ( Material.Emissive & 0xff ) / 255.0f;
  color[1] = ( ( Material.Emissive >> 8 ) & 0xff ) / 255.0f;
  color[0] = ( ( Material.Emissive >> 16 ) & 0xff ) / 255.0f;
  color[3] = ( ( Material.Emissive >> 24 ) & 0xff ) / 255.0f;
  glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, color );

  return false;
}



bool OpenGLRenderClass::SaveScreenShot( const char* szFileName )
{
  return false;
}



bool OpenGLRenderClass::RenderMesh( const Mesh::IMesh& Mesh )
{
  return false;
}



void OpenGLRenderClass::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu entladen
      UnloadAssets( Xtreme::Asset::XA_IMAGE );
      UnloadAssets( Xtreme::Asset::XA_IMAGE_SECTION );
      UnloadAssets( Xtreme::Asset::XA_FONT );
      UnloadAssets( Xtreme::Asset::XA_MESH );
      pLoader->NotifyService( "GUI", "AssetsUnloaded" );
    }
  }
}




bool OpenGLRenderClass::VSyncEnabled()
{
  //return m_VSyncEnabled;
  return false;
}



void OpenGLRenderClass::EnableVSync( bool Enable )
{
  /*
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
  }
  */
}



void OpenGLRenderClass::SetRenderTarget( XTexture* pTexture )
{
  if ( pTexture == NULL )
  {
    m_pSetRenderTargetTexture = NULL;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    InvertCullMode();
    return;
  }
  if ( !( (OpenGLTexture*)pTexture )->m_AllowUsageAsRenderTarget )
  {
    dh::Log( "SetRenderTarget called with non render target texture!" );
    return;
  }

  m_SetRenderTargetTextureSize = pTexture->m_SurfaceSize;
  glBindFramebuffer( GL_FRAMEBUFFER, ( (OpenGLTexture*)pTexture )->m_RenderTargetFrameBufferID );
  glEnable( GL_TEXTURE_2D );
  glViewport( 0, 0, pTexture->m_SurfaceSize.x, pTexture->m_SurfaceSize.y );
  glScissor( 0, 0, pTexture->m_SurfaceSize.x, pTexture->m_SurfaceSize.y );
  //Clear();

  SetTransform( XRenderer::TT_WORLD, m_Transform[XRenderer::TT_WORLD] );
  SetTransform( XRenderer::TT_VIEW, m_Transform[XRenderer::TT_VIEW] );
  SetTransform( XRenderer::TT_PROJECTION, m_Transform[XRenderer::TT_PROJECTION] );

  if ( pTexture != m_pSetRenderTargetTexture )
  {
    InvertCullMode();
    m_pSetRenderTargetTexture = pTexture;
  }
}



void OpenGLRenderClass::InvertCullMode()
{
  XRenderer::eRenderStateValue    cullMode = (XRenderer::eRenderStateValue)m_RenderStates[std::make_pair( XRenderer::RS_CULLMODE, 0 )];

  switch ( cullMode )
  {
    case XRenderer::RSV_CULL_NONE:
      return;
    case XRenderer::RSV_CULL_CCW:
      SetState( XRenderer::RS_CULLMODE, XRenderer::RSV_CULL_CW );
      break;
    case XRenderer::RSV_CULL_CW:
      SetState( XRenderer::RS_CULLMODE, XRenderer::RSV_CULL_CCW );
      break;
  }
}