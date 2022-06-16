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

#include "OpenGLShaderRenderClass.h"
#include "OpenGLShaderTexture.h"
#include "OpenGLShaderVertexBuffer.h"
#include "QuadCache.h"

#include "VertexShader.h"
#include "PixelShader.h"



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

typedef void ( APIENTRYP PFNGLATTACHSHADERPROC ) ( GLuint program, GLuint shader );
typedef void ( APIENTRYP PFNGLCOMPILESHADERPROC ) ( GLuint shader );
typedef GLuint( APIENTRYP PFNGLCREATEPROGRAMPROC ) ( void );
typedef GLuint( APIENTRYP PFNGLCREATESHADERPROC ) ( GLenum type );
typedef void ( APIENTRYP PFNGLDELETEPROGRAMPROC ) ( GLuint program );
typedef void ( APIENTRYP PFNGLDELETESHADERPROC ) ( GLuint shader );
typedef void ( APIENTRYP PFNGLDETACHSHADERPROC ) ( GLuint program, GLuint shader );
typedef void ( APIENTRYP PFNGLSHADERSOURCEPROC ) ( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length );
typedef void ( APIENTRYP PFNGLGETSHADERIVPROC ) ( GLuint shader, GLenum pname, GLint* params );
typedef void ( APIENTRYP PFNGLGETSHADERINFOLOGPROC ) ( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog );
typedef void ( APIENTRYP PFNGLGETPROGRAMIVPROC ) ( GLuint program, GLenum pname, GLint* params );
typedef void ( APIENTRYP PFNGLLINKPROGRAMPROC ) ( GLuint program );
typedef void ( APIENTRYP PFNGLGETPROGRAMINFOLOGPROC ) ( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog );
typedef void ( APIENTRYP PFNGLUSEPROGRAMPROC ) ( GLuint program );
typedef GLuint( APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC ) ( GLuint program, const GLchar* uniformBlockName );
typedef void ( APIENTRYP PFNGLBINDBUFFERBASEPROC ) ( GLenum target, GLuint index, GLuint buffer );
typedef void ( APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC ) ( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding );
typedef void ( APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC ) ( GLuint index );
typedef void ( APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC ) ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer );
typedef void ( APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC ) ( GLuint index );
typedef void ( APIENTRYP PFNGLGENVERTEXARRAYSPROC ) ( GLsizei n, GLuint* arrays );
typedef void ( APIENTRYP PFNGLBINDVERTEXARRAYPROC ) ( GLuint array );
typedef void ( APIENTRYP PFNGLDELETEVERTEXARRAYSPROC ) ( GLsizei n, const GLuint* arrays );

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

PFNGLATTACHSHADERPROC         glAttachShader = NULL;
PFNGLCOMPILESHADERPROC        glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC        glCreateProgram = NULL;
PFNGLCREATESHADERPROC         glCreateShader = NULL;
PFNGLDELETEPROGRAMPROC        glDeleteProgram = NULL;
PFNGLDELETESHADERPROC         glDeleteShader = NULL;
PFNGLDETACHSHADERPROC         glDetachShader = NULL;
PFNGLSHADERSOURCEPROC         glShaderSource = NULL;
PFNGLGETSHADERIVPROC          glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC     glGetShaderInfoLog = NULL;
PFNGLGETPROGRAMIVPROC         glGetProgramiv = NULL;
PFNGLLINKPROGRAMPROC          glLinkProgram = NULL;
PFNGLGETPROGRAMINFOLOGPROC    glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAMPROC           glUseProgram = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = NULL;
PFNGLBINDBUFFERBASENVPROC     glBindBufferBase = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC  glUniformBlockBinding = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC  glVertexAttribPointer = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLGENVERTEXARRAYSPROC      glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC      glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC   glDeleteVertexArrays = NULL;

#endif


#if ( OPERATING_SUB_SYSTEM != OS_SUB_SDL )
PFNGLACTIVETEXTUREPROC        glActiveTexture = NULL;
#endif

#if ( OPERATING_SYSTEM != OS_WEB )
// hacky crap for double GL context hop
HGLRC WINAPI wglarb_CreateContextAttribsARB(
  HDC hDC,
  HGLRC hShareContext,
  const int* attribList );

BOOL WINAPI wglarb_ChoosePixelFormatARB(
  HDC hdc,
  const int* piAttribIList,
  const FLOAT* pfAttribFList,
  UINT nMaxFormats,
  int* piFormats,
  UINT* nNumFormats );


#define wglarb_BuildAssert(cond) ((void)sizeof(char[1 - 2*!(cond)]))

static HANDLE wglarb_intermediary_mutex = NULL;

static DWORD wglarb_intermediary_lock( void )
{
  wglarb_BuildAssert( sizeof( PVOID ) == sizeof( HANDLE ) );

  if ( !wglarb_intermediary_mutex )
  {
    // Between testing for the validity of the mutex handle,
    // creating a new mutex handle and using the interlocked
    // exchange there is a race...

    // //// START

    HANDLE const new_mutex =
      CreateMutex( NULL, TRUE, NULL );

    HANDLE const dst_mutex =
      InterlockedCompareExchangePointer(
        &wglarb_intermediary_mutex,
        new_mutex,
        NULL );

    // //// FINISH \\\\ 

    if ( !dst_mutex )
    {
      // mutex created in one time initialization and held by calling thread. Return signaled status.
      return WAIT_OBJECT_0;
    }
    // In this case we lost the race and another thread
    // beat this thread in creating a mutex object.
    // Clean up and wait for the proper mutex.
    ReleaseMutex( new_mutex );
    CloseHandle( new_mutex );
  }
  return WaitForSingleObject( wglarb_intermediary_mutex, INFINITE );
}

static BOOL wglarb_intermediary_unlock( void )
{
  return ReleaseMutex( wglarb_intermediary_mutex );
}

#define WGLARB_INTERMEDIARY_CLASS	L"wglarb intermediary"
#define WGLARB_INTERMEDIARY_STYLE	(WS_CLIPSIBLINGS|WS_CLIPCHILDREN)
#define WGLARB_INTERMEDIARY_EXSTYLE	0

static HWND wglarb_intermediary_hWnd = 0;

static BOOL wglarb_intermediary_create_Wnd( void )
{
  HINSTANCE const hInstance = GetModuleHandle( NULL );

  WNDCLASS wc;
  memset( &wc, 0, sizeof( wc ) );
  wc.hInstance = hInstance;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = DefWindowProc;
  wc.lpszClassName = WGLARB_INTERMEDIARY_CLASS;
  RegisterClass( &wc );

  wglarb_intermediary_hWnd =
    CreateWindowEx(
      WGLARB_INTERMEDIARY_EXSTYLE,
      WGLARB_INTERMEDIARY_CLASS,
      NULL,
      WGLARB_INTERMEDIARY_STYLE,
      0, 0, 0, 0,
      NULL, NULL,
      hInstance,
      NULL );

  if ( !wglarb_intermediary_hWnd )
  {
    return FALSE;
  }

  return TRUE;
}

static HDC wglarb_intermediary_hDC = 0;

static BOOL wglarb_intermediary_create_DC( void )
{
  if ( !wglarb_intermediary_hWnd
       && !wglarb_intermediary_create_Wnd() )
  {
    return FALSE;
  }

  wglarb_intermediary_hDC = GetDC( wglarb_intermediary_hWnd );
  if ( !wglarb_intermediary_hDC )
  {
    return FALSE;
  }

  return TRUE;
}

static HGLRC wglarb_intermediary_hRC = 0;

static BOOL wglarb_intermediary_create_RC( void )
{
  if ( !wglarb_intermediary_hDC
       && !wglarb_intermediary_create_DC() )
  {
    return FALSE;
  }

  PIXELFORMATDESCRIPTOR pfd;
  memset( &pfd, 0, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int iPF;
  if ( !( iPF = ChoosePixelFormat( wglarb_intermediary_hDC, &pfd ) )
       || !( SetPixelFormat( wglarb_intermediary_hDC, iPF, &pfd ) )
       || !( wglarb_intermediary_hRC = wglCreateContext( wglarb_intermediary_hDC ) )
       )
  {
    return FALSE;
  }

  return TRUE;
}

static BOOL wglarb_intermediary_makecurrent( HDC* hOrigDC, HGLRC* hOrigRC )
{
  *hOrigDC = wglGetCurrentDC();
  *hOrigRC = wglGetCurrentContext();

  if ( !wglarb_intermediary_hRC
       && !wglarb_intermediary_create_RC() )
  {
    return FALSE;
  }

  return wglMakeCurrent( wglarb_intermediary_hDC, wglarb_intermediary_hRC );
}

HGLRC WINAPI wglarb_CreateContextAttribsARB(
  HDC hDC,
  HGLRC hShareContext,
  const int* attribList )
{
  if ( WAIT_OBJECT_0 != wglarb_intermediary_lock() )
  {
    return NULL;
  }

  HDC   hOrigDC;
  HGLRC hOrigRC;
  if ( !wglarb_intermediary_makecurrent( &hOrigDC, &hOrigRC ) )
  {
    wglarb_intermediary_unlock();
    return NULL;
  }

  PFNWGLCREATECONTEXTATTRIBSARBPROC impl =
    (PFNWGLCREATECONTEXTATTRIBSARBPROC)
    wglGetProcAddress( "wglCreateContextAttribsARB" );

  HGLRC ret = NULL;
  if ( impl )
  {
    ret = impl( hDC, hShareContext, attribList );
  }

  wglMakeCurrent( hOrigDC, hOrigRC );
  wglarb_intermediary_unlock();
  return ret;
}

BOOL WINAPI wglarb_ChoosePixelFormatARB(
  HDC hdc,
  const int* piAttribIList,
  const FLOAT* pfAttribFList,
  UINT nMaxFormats,
  int* piFormats,
  UINT* nNumFormats )
{
  if ( WAIT_OBJECT_0 != wglarb_intermediary_lock() )
  {
    return FALSE;
  }

  HDC   hOrigDC;
  HGLRC hOrigRC;
  if ( !wglarb_intermediary_makecurrent( &hOrigDC, &hOrigRC ) )
  {
    wglarb_intermediary_unlock();
    return FALSE;
  }

  PFNWGLCHOOSEPIXELFORMATARBPROC impl = NULL;

  impl = (PFNWGLCHOOSEPIXELFORMATARBPROC)
    wglGetProcAddress( "wglChoosePixelFormatARB" );
  if ( !impl )
  {
    // WGL_EXT_pixel_format uses the same function prototypes as the WGL_ARB_pixel_format extension
    impl = (PFNWGLCHOOSEPIXELFORMATARBPROC)
      wglGetProcAddress( "wglChoosePixelFormatEXT" );
  }

  BOOL ret = FALSE;
  if ( impl )
  {
    ret = impl(
      hdc,
      piAttribIList,
      pfAttribFList,
      nMaxFormats,
      piFormats,
      nNumFormats );
  }

  wglMakeCurrent( hOrigDC, hOrigRC );
  wglarb_intermediary_unlock();
  return ret;
}
#endif



OpenGLShaderRenderClass::OpenGLShaderRenderClass() :
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  m_pMainWindow( NULL ),
#else
  m_RC( NULL ),
  m_DC( NULL ),
#endif
  m_FullScreen( false ),
  m_Width( 0 ),
  m_Height( 0 ),
  m_CurrentShaderType( ST_INVALID ),
  m_AlphaRefValue( 8.0f / 255.0f ),
  m_AlphaFuncValue( GL_GEQUAL ),
  m_SrcBlend( GL_ONE ),
  m_DestBlend( GL_ZERO ),
  m_OldPixelFormat( 0 ),
  m_Depth( 0 ),
  m_Ready( false ),
  m_SupportsVBO( false ),
  m_2dModeActive( false ),
  m_pSetRenderTargetTexture( NULL ),
  m_NumActiveLights( 0 ),
  m_LightsChanged( false ),
  m_LightingEnabled( false ),
  m_UBOGlobalMatrices( 0 ),
  m_UBOLights( 0 ),
  m_UBOMaterial( 0 ),
  m_UBOFog( 0 ),
  m_UBOGlobalMatricesBindingID( 1 ),
  m_UBOLightsBindingID( 2 ),
  m_UBOMaterialBindingID( 3 ),
  m_UBOFogBindingID( 4 ),
  m_QuadCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_QuadCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ ),
  m_LineCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_LineCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ ),
  m_TriangleCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_TriangleCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ )
{
  m_DirectTexelMappingOffset.set( -0.5f, -0.5f );

  for ( int i = 0; i < 8; ++i )
  {
    m_pSetTextures[i] = NULL;
    m_SetLights[i].m_Type = XLight::LT_INVALID;
    m_LightEnabled[i] = false;
  }
#if ( OPERATING_SYSTEM != OS_WEB )
  ZeroMemory( &m_pfd, sizeof( m_pfd ) );
#endif
}



OpenGLShaderRenderClass::~OpenGLShaderRenderClass()
{
  Release();
}



bool OpenGLShaderRenderClass::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment )
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
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

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

  //m_RC = wglCreateContext( m_DC );
  int attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 2,
    0, 0 };

  m_RC = wglarb_CreateContextAttribsARB( m_DC, NULL, attribs );
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

  m_Canvas.set( 0, 0, Width, Height );
  m_VirtualSize.set( Width, Height );

  m_Transform[TT_WORLD].Identity();
  m_Transform[TT_VIEW].Identity();
  m_Transform[TT_PROJECTION].Identity();

  m_Matrices.ScreenCoord2d.OrthoOffCenterLH( 0.0f,
                                             (GR::f32)m_Width,
                                             (GR::f32)m_Height,
                                             0.0f,
                                             0.0f,
                                             1.0f );
  m_Matrices.ScreenCoord2d.Transpose();
  m_Matrices.Model.Identity();
  m_Matrices.TextureTransform.Identity();
  m_StoredTextureTransform.Identity();

  m_LightInfo.Ambient = ColorValue( 0x00000000 );

  glFlush();
  OnResized();

#if OPERATING_SYSTEM != OS_WEB
  glShadeModel( GL_SMOOTH );
  glClearDepth( 1.0f );
#endif
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glBlendFunc( m_SrcBlend, m_DestBlend );

  SetState( XRenderer::RS_CULLMODE, XRenderer::RSV_CULL_CCW );
  CheckError( "x" );

  SetState( XRenderer::RS_MINFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MAGFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MIPFILTER, XRenderer::RSV_FILTER_LINEAR );

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

  glAttachShader      = (PFNGLATTACHSHADERPROC)wglGetProcAddress( "glAttachShader" );
  glCompileShader     = (PFNGLCOMPILESHADERPROC)wglGetProcAddress( "glCompileShader" );
  glCreateProgram     = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress( "glCreateProgram" );
  glCreateShader      = (PFNGLCREATESHADERPROC)wglGetProcAddress( "glCreateShader" );
  glDeleteProgram     = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress( "glDeleteProgram" );
  glDeleteShader      = (PFNGLDELETESHADERPROC)wglGetProcAddress( "glDeleteShader" );
  glDetachShader      = (PFNGLDETACHSHADERPROC)wglGetProcAddress( "glDetachShader" );
  glShaderSource      = (PFNGLSHADERSOURCEPROC)wglGetProcAddress( "glShaderSource" );
  glGetShaderiv       = (PFNGLGETSHADERIVPROC)wglGetProcAddress( "glGetShaderiv" );
  glGetShaderInfoLog  = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress( "glGetShaderInfoLog" );
  glGetProgramiv      = (PFNGLGETPROGRAMIVARBPROC)wglGetProcAddress( "glGetProgramiv" );
  glLinkProgram       = (PFNGLLINKPROGRAMPROC)wglGetProcAddress( "glLinkProgram" );
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress( "glGetProgramInfoLog" );
  glUseProgram        = (PFNGLUSEPROGRAMPROC)wglGetProcAddress( "glUseProgram" );
  glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)wglGetProcAddress( "glGetUniformBlockIndex" );
  glBindBufferBase    = (PFNGLBINDBUFFERBASENVPROC)wglGetProcAddress( "glBindBufferBase" );
  glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)wglGetProcAddress( "glUniformBlockBinding" );
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress( "glEnableVertexAttribArray" );
  glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress( "glVertexAttribPointer" );
  glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress( "glDisableVertexAttribArray" );
  glGenVertexArrays   = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress( "glGenVertexArrays" );
  glBindVertexArray   = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress( "glBindVertexArray" );
  glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress( "glDeleteVertexArrays" );

  if ( ( glAttachShader == NULL )
  ||   ( glCompileShader == NULL )
  ||   ( glCreateProgram == NULL )
  ||   ( glCreateShader == NULL )
  ||   ( glDeleteProgram == NULL )
  ||   ( glDeleteShader == NULL )
  ||   ( glDetachShader == NULL )
  ||   ( glShaderSource == NULL )
  ||   ( glGetProgramiv == NULL )
  ||   ( glLinkProgram == NULL )
  ||   ( glGetProgramInfoLog == NULL )
  ||   ( glUseProgram == NULL )
  ||   ( glGetUniformBlockIndex == NULL )
  ||   ( glBindBufferBase == NULL )
  ||   ( glUniformBlockBinding == NULL )
  ||   ( glEnableVertexAttribArray == NULL )
  ||   ( glVertexAttribPointer == NULL )
  ||   ( glDisableVertexAttribArray == NULL )
  ||   ( glGenVertexArrays == NULL )
  ||   ( glBindVertexArray == NULL )
  ||   ( glDeleteVertexArrays == NULL ) )
  {
    dh::Log( "No shader support" );
  }
#endif

  // init uniform FogConstantBuffer
  // int iFogType = FOG_TYPE_NONE; \n"
  //   "  float fFogStart   = 10.f; \n"
  //   "  float fFogEnd     = 25.f; \n"
  //   "  float fFogDensity = .02f; \n"
  //   "  vec4 vFogColor  = vec4(0.0f, 0.0f, 0.0f, 0.0f); \n"

#if OPERATING_SYSTEM == OS_WEB 
#include "shadercodees.inl" 
#else
#include "shadercode.inl" 
#endif

  /*
  GR::String    vertexShader = "#version 330 core\n"
    "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
    "void main(){\n"
    "gl_Position.xyz = vertexPosition_modelspace;\n"
    "gl_Position.w = 1.0;\n"
    "}\n";

  GR::String    pixelShader = "#version 330 core\n"
    "out vec3 color;\n"
  "void main()\n"
  "{\n"
   " color = vec3( 1, 0, 0 );\n"
  "}\n";

  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR, vertexShader, pixelShader );*/

  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR, VS_positionrhw_texture_color, PS_position_texture_color );
  InitShader( InternalShader::POSITIONRHW_COLOR, VS_positionrhw_color, PS_position_color );
  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT, VS_positionrhw_texture_color_normal_nolight, PS_position_texture_color );
  InitShader( InternalShader::POSITION_COLOR, VS_position_color, PS_position_color );
  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR_ALPHATEST, VS_positionrhw_texture_color, PS_position_texture_color_alphatest );
  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST, VS_positionrhw_texture_color_normal_nolight, PS_position_texture_color_alphatest );

  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST, VS_position_texture_color_normal_nolight, PS_position_texture_color_alphatest );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED_ALPHATEST, VS_position_texture_color_normal_lightdisabled, PS_position_texture_color_alphatest );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT_ALPHATEST, VS_position_texture_color_normal_light1, PS_position_texture_color_alphatest );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS_ALPHATEST, VS_position_texture_color_normal_light2, PS_position_texture_color_alphatest );

  InitShader( InternalShader::POSITIONRHW_COLOR_ALPHATEST, VS_positionrhw_color, PS_position_color_alphatest );
  InitShader( InternalShader::POSITIONRHW_NOTEXTURE_COLOR, VS_positionrhw_notexture_color, PS_position_color );
  InitShader( InternalShader::POSITIONRHW_TEXTURE_COLOR_ALPHATEST_COLOR_FROM_DIFFUSE, VS_positionrhw_texture_color, PS_position_texture_colordiffuse_alphatest );

  InitShader( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_LIGHTSDISABLED, VS_position_notexture_color_normal_lightdisabled, PS_position_color );
  InitShader( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_NOLIGHT, VS_position_notexture_color_normal_nolight, PS_position_color );
  InitShader( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_ONELIGHT, VS_position_notexture_color_normal_light1, PS_position_color );
  InitShader( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_TWOLIGHTS, VS_position_notexture_color_normal_light2, PS_position_color );

  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED, VS_position_texture_color_normal_lightdisabled, PS_position_texture_color );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT, VS_position_texture_color_normal_nolight, PS_position_texture_color );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT, VS_position_texture_color_normal_light1, PS_position_texture_color );
  InitShader( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS, VS_position_texture_color_normal_light2, PS_position_texture_color );

  //InitShader( InternalShader::POSITION_TEXTURE_COLOR, VS_position_texture_color, PS_position_texture_color );

  glGenBuffers( 1, &m_UBOGlobalMatrices );
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOGlobalMatrices );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_UBOGlobalMatricesBindingID, m_UBOGlobalMatrices );

  glGenBuffers( 1, &m_UBOLights );
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOLights );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_UBOLightsBindingID, m_UBOLights );

  glGenBuffers( 1, &m_UBOMaterial );
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOMaterial );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_UBOMaterialBindingID, m_UBOMaterial );

  glGenBuffers( 1, &m_UBOFog );
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOFog );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_UBOFogBindingID, m_UBOFog );

  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
  CheckError( "glGetUniformBlockIndex for UBOs" );

  // set the uniforms for all programs
  std::map<int, GLuint>::iterator   itUB( m_StoredShaderPrograms.begin() );
  while ( itUB != m_StoredShaderPrograms.end() )
  {
    GLuint    programID = itUB->second;

    GLuint UBOGlobalMatricesIndex = glGetUniformBlockIndex( programID, "ModelViewProjectionConstantBuffer" );
    GLuint UBOLightsIndex         = glGetUniformBlockIndex( programID, "LightsConstantBuffer" );
    GLuint UBOMaterialIndex       = glGetUniformBlockIndex( programID, "MaterialConstantBuffer" );
    GLuint UBOFogIndex            = glGetUniformBlockIndex( programID, "FogConstantBuffer" );

    CheckError( "glGetUniformBlockIndex" );

    if ( UBOGlobalMatricesIndex != GL_INVALID_INDEX )
    {
      glUniformBlockBinding( programID, UBOGlobalMatricesIndex, m_UBOGlobalMatricesBindingID );
      CheckError( "glUniformBlockBinding for UBOs 1" );
    }

    if ( UBOLightsIndex != GL_INVALID_INDEX )
    {
      glUniformBlockBinding( programID, UBOLightsIndex, m_UBOLightsBindingID );
      CheckError( "glUniformBlockBinding for UBOs 2" );
    }
    if ( UBOMaterialIndex != GL_INVALID_INDEX )
    {
      glUniformBlockBinding( programID, UBOMaterialIndex, m_UBOMaterialBindingID );
      CheckError( "glUniformBlockBinding for UBOs 3" );
    }
    if ( UBOFogIndex != GL_INVALID_INDEX )
    {
      glUniformBlockBinding( programID, UBOFogIndex, m_UBOFogBindingID );
      CheckError( "glUniformBlockBinding for UBOs 4" );
    }

    CheckError( "glUniformBlockBinding for shader program" );

    ++itUB;
  }


  // states that access the UBOs
  SetState( XRenderer::RS_FOG_ENABLE, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_FOG_TABLEMODE, XRenderer::RSV_FOG_LINEAR );

  SetTransform( XRenderer::TT_WORLD, m_Matrices.Model );

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

              ( (OpenGLShaderTexture*)pTexture )->m_listFileNames.push_back( strPath );

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



bool OpenGLShaderRenderClass::InitShader( InternalShader::Value Program, const GR::String& VertexShaderContent, const GR::String& PixelShaderContent )
{
  VertexShader* pVertexShader = new VertexShader( this );
  if ( !pVertexShader->CreateFromString( VertexShaderContent ) )
  {
    delete pVertexShader;
    dh::Log( "OpenGLShaderRenderClass::InitShader failed at vertex shader for program %d", Program );
    //dh::Log( "OpenGLShaderRenderClass::InitShader failed at vertex shader for program %s", VertexShaderContent.c_str() );
    return false;
  }

  PixelShader* pPixelShader = new PixelShader( this );
  if ( !pPixelShader->CreateFromString( PixelShaderContent ) )
  {
    dh::Log( "OpenGLShaderRenderClass::InitShader failed at pixel shader for program %d", Program );
    //dh::Log( "OpenGLShaderRenderClass::InitShader failed at pixel shader for program %s", PixelShaderContent.c_str() );
    delete pPixelShader;
    delete pVertexShader;
    return false;
  }

  // Link the program
  GLint Result = GL_FALSE;
  int InfoLogLength;

  GLuint programID = glCreateProgram();
  glAttachShader( programID, pVertexShader->m_ShaderID );
  glAttachShader( programID, pPixelShader->m_ShaderID );
  glLinkProgram( programID );

  // Check the program
  glGetProgramiv( programID, GL_LINK_STATUS, &Result );
  glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &InfoLogLength );
  if ( InfoLogLength > 1 )
  {
    std::vector<char> ProgramErrorMessage( InfoLogLength + 1 );
    glGetProgramInfoLog( programID, InfoLogLength, NULL, &ProgramErrorMessage[0] );
    dh::Log( "Link Shader Program Error for program %d: %s", Program, &ProgramErrorMessage[0] );

    //dh::Log( "OpenGLShaderRenderClass::InitShader failed at linking for vertex shader %s", VertexShaderContent.c_str() );
    //dh::Log( "OpenGLShaderRenderClass::InitShader failed at linking for pixel shader %s", PixelShaderContent.c_str() );
  }
  
  glDetachShader( programID, pVertexShader->m_ShaderID );
  glDetachShader( programID, pPixelShader->m_ShaderID );

  CheckError( "InitShader" );
  //glDeleteShader( VertexShaderID );
  //glDeleteShader( FragmentShaderID );

  m_StoredShaderPrograms[Program] = programID;
  return true;
}



void OpenGLShaderRenderClass::SetShader( eShaderType sType )
{
  m_CurrentShaderType = sType;
  if ( sType == ST_FLAT_NO_TEXTURE )
  {
    SetTexture( 0, NULL );
    SetTexture( 1, NULL );
  }

  if ( ( sType == ST_ALPHA_BLEND )
  ||   ( sType == ST_ALPHA_BLEND_AND_TEST ) )
  {
    SetState( RS_ALPHATEST, RSV_ENABLE );
    SetState( RS_ALPHABLENDING, RSV_ENABLE );
  }
  else if ( ( sType == ST_ALPHA_TEST )
  ||        ( sType == ST_ALPHA_TEST_COLOR_FROM_DIFFUSE ) )
  {
    SetState( RS_ALPHATEST, RSV_ENABLE );
    SetState( RS_ALPHABLENDING, RSV_DISABLE );
  }
  else
  {
    SetState( RS_ALPHATEST, RSV_DISABLE );
    SetState( RS_ALPHABLENDING, RSV_DISABLE );
  }
  SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, 0 )], m_RenderStates[std::make_pair( RS_ALPHABLENDING, 0 )] );
}



void OpenGLShaderRenderClass::SetGLShaderProgram( InternalShader::Value Program )
{
  if ( m_StoredShaderPrograms.find( Program ) == m_StoredShaderPrograms.end() )
  {
    dh::Log( "SetGLShaderProgram unsupported program chosen %d", Program );
    return;
  }
  glUseProgram( m_StoredShaderPrograms[Program] );
  CheckError( "SetGLShaderProgram" );
}



void OpenGLShaderRenderClass::ChooseShaders( GR::u32 VertexFormat )
{
  eShaderType   shaderTypeToUse = m_CurrentShaderType;

  int     numLights     = m_NumActiveLights;
  bool    lightDisabled = ( m_RenderStates[std::make_pair( XRenderer::RS_LIGHTING, 0 )] == XRenderer::RSV_DISABLE );

  // fall back to other shaders, blending is handled in the shader
  if ( shaderTypeToUse == ST_ALPHA_BLEND )
  {
    shaderTypeToUse = ST_FLAT;
    SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, 0 )], m_RenderStates[std::make_pair( RS_ALPHABLENDING, 0 )] );
  }
  else if ( shaderTypeToUse == ST_ALPHA_BLEND_AND_TEST )
  {
    shaderTypeToUse = ST_ALPHA_TEST;
    SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, 0 )], m_RenderStates[std::make_pair( RS_ALPHABLENDING, 0 )] );
  }
  else if ( shaderTypeToUse == ST_ADDITIVE )
  {
    shaderTypeToUse = ST_FLAT;
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  }
  else
  {
  }

  // fall back to non-texture mode?
  if ( m_pSetTextures[0] == NULL )
  {
    if ( shaderTypeToUse == ST_FLAT )
    {
      shaderTypeToUse = ST_FLAT_NO_TEXTURE;
    }
  }

  /*
  VFF_XYZ = 0x00000002,
  VFF_XYZRHW = 0x00000004,
  VFF_NORMAL = 0x00000010,
  VFF_DIFFUSE = 0x00000040,
  VFF_SPECULAR = 0x00000080,
  VFF_TEXTURECOORD = 0x00000100,
  */

  switch ( shaderTypeToUse )
  {
    case ST_FLAT:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED );
          return;
        }
        else if ( numLights == 0 )
        {
          SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT );
          return;
        }
        else if ( numLights == 1 )
        {
          SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT );
          return;
        }
        else if ( numLights == 2 )
        {
          SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS );
          return;
        }
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED );
          return;
        }
        else if ( numLights == 0 )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT );
          return;
        }
        else if ( numLights == 1 )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_ONELIGHT );
          return;
        }
        else if ( numLights == 2 )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_TWOLIGHTS );
          return;
        }
      }
      /*
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "position_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "positionrhw_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_texture_color_normal_lightdisabled" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_texture_color_normal_light1" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_texture_color_normal_light2" );
          SetPixelShader( "position_texture_color" );
          return;
        }
      }*/
      break;
    case ST_FLAT_NO_TEXTURE:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetGLShaderProgram( InternalShader::POSITIONRHW_COLOR );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetGLShaderProgram( InternalShader::POSITION_COLOR );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetGLShaderProgram( InternalShader::POSITIONRHW_NOTEXTURE_COLOR );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        if ( lightDisabled )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_LIGHTSDISABLED );
          return;
        }
        else if ( numLights == 0 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_NOLIGHT );
          return;
        }
        else if ( numLights == 1 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_ONELIGHT );
          return;
        }
        else if ( numLights == 2 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_TWOLIGHTS );
          return;
        }
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_LIGHTSDISABLED );
          return;
        }
        else if ( numLights == 0 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_NOLIGHT );
          return;
        }
        else if ( numLights == 1 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_ONELIGHT );
          return;
        }
        else if ( numLights == 2 )
        {
          SetGLShaderProgram( InternalShader::POSITION_NOTEXTURE_COLOR_NORMAL_TWOLIGHTS );
          return;
        }
      }
      /*
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetVertexShader( "position_notexture_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_notexture_color_normal_lightdisabled" );
          //SetPixelShader( "position_texture_color" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_notexture_color_normal_nolight" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_notexture_color_normal_light1" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_notexture_color_normal_light2" );
          SetPixelShader( "position_color" );
          return;
        }
      }
      */
      break;
    case ST_ALPHA_TEST:
      if ( m_pSetTextures[0] == NULL )
      {
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_COLOR_ALPHATEST );
          
          return;
        }
      }
      else
      {
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_ALPHATEST );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED_ALPHATEST );
            //SetVertexShader( "position_texture_color_normal_lightdisabled" );
            //SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST );
            //SetVertexShader( "position_texture_color_normal_nolight" );
            //SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT_ALPHATEST );
            //SetVertexShader( "position_texture_color_normal_light1" );
            //SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetGLShaderProgram( InternalShader::POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS_ALPHATEST );
            //SetVertexShader( "position_texture_color_normal_light2" );
            //SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
        }
      }
      /*
      if ( m_pSetTextures[0] == NULL )
      {
        // no texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no light with XYZRHW!
          SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_notexture_color_normal_lightdisabled" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_notexture_color_normal_nolight" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_notexture_color_normal_light1" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetVertexShader( "position_notexture_color_normal_light2" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        
      }
      else
      {
        // has texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_texture_color" );
          SetPixelShader( "position_texture_color_alphatest" );
          return;
        }
        
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }*/
      break;
      /*
    case ST_ADDITIVE:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        // no lights with xyzrhw
        SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR );
        return;
      }

      if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_texture_color_normal_lightdisabled" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_texture_color_normal_light1" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_texture_color_normal_light2" );
          SetPixelShader( "position_texture_color" );
          return;
        }
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        // no lights with xyzrhw
        SetVertexShader( "positionrhw_texture_color_normal_nolight" );
        SetPixelShader( "position_texture_color" );
        return;
      }*/
      break;
    case ST_ALPHA_TEST_COLOR_FROM_DIFFUSE:
      if ( m_pSetTextures[0] == NULL )
      {
      }
      else
      {
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetGLShaderProgram( InternalShader::POSITIONRHW_TEXTURE_COLOR_ALPHATEST_COLOR_FROM_DIFFUSE );
          return;
        }
      }
      /*
      if ( m_pSetTextures[0] == NULL )
      {
        // no texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_texture_color_normal_lightdisabled" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_texture_color_normal_nolight" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_texture_color_normal_light1" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetVertexShader( "position_texture_color_normal_light2" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }
      else
      {
        // has texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_texture_color" );
          SetPixelShader( "position_texture_colordiffuse_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetVertexShader( "positionrhw_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_colordiffuse_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_texture_color_normal_lightdisabled" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_texture_color_normal_nolight" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_texture_color_normal_light1" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }*/
      break;
  }
  dh::Log( "ChooseShaders unsupported shader for type %d, vertex format 0x%x, really active %d lights (lighting active %d), texture set %x", shaderTypeToUse, VertexFormat, numLights, !lightDisabled, m_pSetTextures[0] );
}



void OpenGLShaderRenderClass::ReleaseTexture( OpenGLShaderTexture* pTexture )
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



bool OpenGLShaderRenderClass::Release()
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



int OpenGLShaderRenderClass::VisibleWidth()
{
  if ( m_pSetRenderTargetTexture != NULL )
  {
    return m_SetRenderTargetTextureSize.x;
  }
  return Width();
}



int OpenGLShaderRenderClass::VisibleHeight()
{
  if ( m_pSetRenderTargetTexture != NULL )
  {
    return m_SetRenderTargetTextureSize.y;
  }
  return Height();
}



bool OpenGLShaderRenderClass::IsExtensionSupported( const GR::String& TargetExtension )
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



void OpenGLShaderRenderClass::RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZRHW;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }

  QuadCache     quads( this, vertexFormat );

  quads.AddEntry( m_pSetTextures[0],
                  GR::tVector( (GR::f32)X, (GR::f32)Y, (GR::f32)Z ), TU1, TV1,
                  GR::tVector( (GR::f32)X + Width, (GR::f32)Y, (GR::f32)Z ), TU2, TV2,
                  GR::tVector( (GR::f32)X, (GR::f32)Y + Height, (GR::f32)Z ), TU3, TV3,
                  GR::tVector( (GR::f32)X + Width, (GR::f32)Y + Height, (GR::f32)Z ), TU4, TV4,
                  Color1, Color2, Color3, Color4,
                  m_CurrentShaderType );

  quads.FlushCache();
  /*
  GLuint vertexbuffer;
  // Generate 1 buffer, put the resulting identifier in vertexbuffer
  glGenBuffers( 1, &vertexbuffer );
  // The following commands will talk about our 'vertexbuffer' buffer
  glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
  // Give our vertices to OpenGL.
  glBufferData( GL_ARRAY_BUFFER, quads.D.V vb.VertexSize( vertexFormat ), vb.Vg_vertex_buffer_data, GL_STATIC_DRAW );
  */
  /*
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
  CheckError( "RenderQuad2d 2" );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU4, TV4 );
  }
  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color4 & 0xff000000 ) >> 24 );
  glVertex3i( X + Width, Y + Height, (int)Z );
  CheckError( "RenderQuad2d 3" );

  if ( m_pSetTextures[0] )
  {
    glTexCoord2f( TU3, TV3 );
  }
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
  glVertex3i( X, Y + Height, (int)Z );
  CheckError( "RenderQuad2d 4" );

  glEnd();*/
  CheckError( "RenderQuad2d x" );
}



void OpenGLShaderRenderClass::RenderQuadDetail2d( GR::f32 X, GR::f32 Y, GR::f32 Width, GR::f32 Height,
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



void OpenGLShaderRenderClass::RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZRHW;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }

  QuadCache     quads( this, vertexFormat );

  quads.AddEntry( m_pSetTextures[0],
                  GR::tVector( (GR::f32)X1, (GR::f32)Y1, (GR::f32)Z ), TU1, TV1,
                  GR::tVector( (GR::f32)X2, (GR::f32)Y2, (GR::f32)Z ), TU2, TV2,
                  GR::tVector( (GR::f32)X3, (GR::f32)Y3, (GR::f32)Z ), TU3, TV3,
                  GR::tVector( (GR::f32)X4, (GR::f32)Y4, (GR::f32)Z ), TU4, TV4,
                  Color1, Color2, Color3, Color4,
                  m_CurrentShaderType );

  quads.FlushCache();
  /*

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

  glEnd();*/
  CheckError( "RenderQuadDetail2d 2" );
}



void OpenGLShaderRenderClass::RenderQuad( const GR::tVector& ptPos1,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_NORMAL;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }
  QuadCache     quads( this, vertexFormat );

  quads.AddEntry( m_pSetTextures[0],
                  ptPos1, TU1, TV1,
                  ptPos2, TU2, TV1,
                  ptPos3, TU1, TV2,
                  ptPos4, TU2, TV2,
                  Color1, Color2, Color3, Color4,
                  m_CurrentShaderType );

  quads.FlushCache();
  CheckError( "RenderQuad" );
}



void OpenGLShaderRenderClass::RenderQuad( const GR::tVector& ptPos1,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_NORMAL;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }
  QuadCache     quads( this, vertexFormat );

  quads.AddEntry( m_pSetTextures[0],
                  ptPos1, TU1, TV1,
                  ptPos2, TU2, TV2,
                  ptPos3, TU3, TV3,
                  ptPos4, TU4, TV4,
                  Color1, Color2, Color3, Color4,
                  m_CurrentShaderType );

  quads.FlushCache();
  CheckError( "RenderQuad 2" );
}



void OpenGLShaderRenderClass::RenderTriangle2d( const GR::tPoint& pt1, const GR::tPoint& pt2, const GR::tPoint& pt3,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZRHW;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }

  TriangleCache     triangles( this, vertexFormat );

  triangles.AddEntry( m_pSetTextures[0],
                  GR::tVector( (GR::f32)pt1.x, (GR::f32)pt1.y, (GR::f32)Z ), TU1, TV1,
                  GR::tVector( (GR::f32)pt2.x, (GR::f32)pt2.y, (GR::f32)Z ), TU2, TV2,
                  GR::tVector( (GR::f32)pt3.x, (GR::f32)pt3.y, (GR::f32)Z ), TU3, TV3,
                  Color1, Color2, Color3,
                  m_CurrentShaderType );

  triangles.FlushCache();
  CheckError( "RenderTriangle2d" );
}



void OpenGLShaderRenderClass::RenderTriangle( const GR::tVector& ptPos1,
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

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZ;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }

  TriangleCache     triangles( this, vertexFormat );

  triangles.AddEntry( m_pSetTextures[0],
                      ptPos1, TU1, TV1,
                      ptPos2, TU2, TV2,
                      ptPos3, TU3, TV3,
                      Color1, Color2, Color3,
                      m_CurrentShaderType );

  triangles.FlushCache();
  CheckError( "RenderTriangle" );
}



void OpenGLShaderRenderClass::CheckError( const GR::String& Function )
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



XTexture* OpenGLShaderRenderClass::LoadTexture( const char* FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
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



XTexture* OpenGLShaderRenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  CheckError( "CreateTexture pre pre 1" );

  OpenGLShaderTexture*   pTexture = new OpenGLShaderTexture( this, AllowUsageAsRenderTarget );

  glGenTextures( 1, &pTexture->m_TextureID );
  if ( pTexture->m_TextureID == 0 )
  {
    return NULL;
  }

  pTexture->m_ImageFormat = ImageData.ImageFormat();
  pTexture->m_SurfaceSize.set( ImageData.Width(), ImageData.Height() );
  pTexture->m_ImageSourceSize.set( ImageData.Width(), ImageData.Height() );

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



XTexture* OpenGLShaderRenderClass::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  OpenGLShaderTexture*   pTexture = new OpenGLShaderTexture( this, AllowUsageAsRenderTarget );

  glGenTextures( 1, &pTexture->m_TextureID );

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

  GR::Graphic::ContextDescriptor    dummyImage;

  dummyImage.CreateData( Width, Height, imgFormat );

  pTexture->m_ImageFormat = imgFormat;
  pTexture->m_SurfaceSize.set( Width, Height );
  pTexture->m_ImageSourceSize.set( Width, Height );

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



void OpenGLShaderRenderClass::SetTexture( GR::u32 Stage, XTexture* pTexture )
{
  if ( m_pSetTextures[Stage] == pTexture )
  {
    return;
  }
  m_pSetTextures[Stage] = pTexture;
  if ( pTexture == NULL )
  {
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
  else
  {
    glBindTexture( GL_TEXTURE_2D, ( (OpenGLShaderTexture*)pTexture )->m_TextureID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  }
  CheckError( "SetTexture" );
}



XVertexBuffer* OpenGLShaderRenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  OpenGLShaderVertexBuffer* pBuffer = new OpenGLShaderVertexBuffer( this );

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



XVertexBuffer* OpenGLShaderRenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  OpenGLShaderVertexBuffer* pBuffer = new OpenGLShaderVertexBuffer( this );
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



XVertexBuffer* OpenGLShaderRenderClass::CreateVertexBuffer( const Mesh::IMesh& Mesh, GR::u32 VertexFormat )
{
  XVertexBuffer* pBuffer = CreateVertexBuffer( Mesh.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( Mesh );

  return pBuffer;
}



void OpenGLShaderRenderClass::DestroyVertexBuffer( XVertexBuffer* pBuffer )
{
  if ( pBuffer == NULL )
  {
    dh::Log( "OpenGLShaderRenderClass::DestroyVertexBuffer Buffer is NULL!" );
    return;
  }
  pBuffer->Release();
  delete pBuffer;
  m_VertexBuffers.remove( pBuffer );
}



bool OpenGLShaderRenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  if ( pBuffer == NULL )
  {
    dh::Log( "OpenGLShaderRenderClass::RenderVertexBuffer Buffer is NULL!" );
    return false;
  }

  OpenGLShaderVertexBuffer* pOpenGLBuffer = (OpenGLShaderVertexBuffer*)pBuffer;

  ChooseShaders( pOpenGLBuffer->VertexFormat() );

  FlushAllCaches();

  return pOpenGLBuffer->Display( Index, Count );
}



bool OpenGLShaderRenderClass::BeginScene()
{
  return m_Ready;
}



void OpenGLShaderRenderClass::EndScene()
{
}



void OpenGLShaderRenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  glFlush();
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  SDL_GL_SwapWindow( m_pMainWindow );
#else
  SwapBuffers( m_DC );
#endif
}



bool OpenGLShaderRenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  return false;
}



bool OpenGLShaderRenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, ( (OpenGLShaderTexture*)pTexture )->m_TextureID );
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



void OpenGLShaderRenderClass::RestoreAllTextures()
{
}



bool OpenGLShaderRenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  if ( ( imgFormat == GR::Graphic::IF_A8R8G8B8 )
  ||   ( imgFormat == GR::Graphic::IF_R8G8B8 ) )
  {
    return true;
  }
  return false;
}



bool OpenGLShaderRenderClass::OnResized()
{
#if OPERATING_SYSTEM == OS_WINDOWS
  RECT    rc;

  GetClientRect( m_hwndViewport, &rc );


  glViewport( 0, 0, rc.right - rc.left, rc.bottom - rc.top );
#endif
  return true;
}



GR::Graphic::eImageFormat OpenGLShaderRenderClass::ImageFormat()
{
  return GR::Graphic::ImageData::ImageFormatFromDepth( m_Depth );
}



GR::u32 OpenGLShaderRenderClass::Width()
{
  return m_Width;
}



GR::u32 OpenGLShaderRenderClass::Height()
{
  return m_Height;
}



bool OpenGLShaderRenderClass::IsReady() const
{
  return m_Ready;
}



void OpenGLShaderRenderClass::Clear( bool bClearColor, bool bClearZ )
{
  int   flags = 0;

  if ( bClearColor )
  {
    flags |= GL_COLOR_BUFFER_BIT;
  }
  if ( bClearZ )
  {
    flags |= GL_DEPTH_BUFFER_BIT;

    // seriously?
    glDepthMask( GL_TRUE );
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



void OpenGLShaderRenderClass::SetAlpha( GR::u32 AlphaTest, GR::u32 AlphaBlend )
{
  if ( ( AlphaBlend == RSV_ENABLE )
  ||   ( AlphaTest == RSV_ENABLE ) )
  {
    CheckError( "b1" );
    glEnable( GL_BLEND );
    CheckError( "b2" );
    //glEnable( GL_ALPHA_TEST );
    CheckError( "b3" );
    //glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
    CheckError( "b4" );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    CheckError( "b5" );
  }
  else
  {
    CheckError( "a1" );
    glDisable( GL_BLEND );
    CheckError( "a2" );
    //glDisable( GL_ALPHA_TEST );
    CheckError( "a3" );
    glBlendFunc( GL_ONE, GL_ZERO );
    CheckError( "a4" );
  }
}



bool OpenGLShaderRenderClass::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
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
      m_Fog.FogColor = ColorValue( rValue );
      if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == RSV_ENABLE )
      {
        UpdateFogUBO();
      }
      break;
    case RS_FOG_DENSITY:
      m_Fog.FogDensity = *(GR::f32*)&rValue;
      if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == RSV_ENABLE )
      {
        UpdateFogUBO();
      }
      break;
    case RS_FOG_START:
      m_Fog.FogStart = *(GR::f32*)&rValue;
      if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == RSV_ENABLE )
      {
        UpdateFogUBO();
      }
      break;
    case RS_FOG_END:
      m_Fog.FogEnd = *(GR::f32*)&rValue;
      if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == RSV_ENABLE )
      {
        UpdateFogUBO();
      }
      break;
    case RS_FOG_ENABLE:
      if ( rValue == RSV_DISABLE )
      {
        m_Fog.FogType = 0;  // FOG_TYPE_NONE
        UpdateFogUBO();
      }
      else if ( rValue == RSV_ENABLE )
      {
        switch ( m_RenderStates[std::make_pair( RS_FOG_TABLEMODE, 0 )] )
        {
          case RSV_FOG_NONE:
            m_Fog.FogType = 0; // FOG_TYPE_NONE
            break;
          case RSV_FOG_LINEAR:
            m_Fog.FogType = 3; // FOG_TYPE_LINEAR
            break;
          case RSV_FOG_EXP:
            m_Fog.FogType = 1; // FOG_TYPE_EXP
            break;
          case RSV_FOG_EXP_SQUARED:
            m_Fog.FogType = 2; // FOG_TYPE_EXP2
            break;
        }
        UpdateFogUBO();
      }
      break;
    case RS_FOG_TABLEMODE:
      switch ( rValue )
      {
        case RSV_FOG_NONE:
          m_Fog.FogType = 0; // FOG_TYPE_NONE
          break;
        case RSV_FOG_LINEAR:
          m_Fog.FogType = 3; // FOG_TYPE_LINEAR
          break;
        case RSV_FOG_EXP:
          m_Fog.FogType = 1; // FOG_TYPE_EXP
          break;
        case RSV_FOG_EXP_SQUARED:
          m_Fog.FogType = 2; // FOG_TYPE_EXP2
          break;
      }
      if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, 0 )] == RSV_ENABLE )
      {
        UpdateFogUBO();
      }
      break;
    case RS_FOG_VERTEXMODE:
      break;
    case RS_TEXTURE_TRANSFORM:
      if ( rValue == RSV_TEXTURE_TRANSFORM_COUNT2 )
      {
      }
      else if ( rValue == RSV_DISABLE )
      {
        //glMatrixMode( GL_TEXTURE );
        //glLoadIdentity();
        m_Matrices.TextureTransform.Identity();
        UpdateMatrixUBO();
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
      if ( Stage < 8 )
      {
        if ( rValue == XRenderer::RSV_ENABLE )
        {
          if ( !m_LightEnabled[Stage] )
          {
            FlushAllCaches();
            m_LightEnabled[Stage] = true;
            ++m_NumActiveLights;
          }
        }
        else if ( rValue == XRenderer::RSV_DISABLE )
        {
          if ( m_LightEnabled[Stage] )
          {
            FlushAllCaches();
            m_LightEnabled[Stage] = false;
            --m_NumActiveLights;
          }
        }
      }
      break;
    case RS_LIGHTING:
      break;
    case RS_AMBIENT:
      m_LightInfo.Ambient = ColorValue( rValue );
      break;
    case RS_COLOR_OP:
      /*
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
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
      }
      {
        bSupported = false;
      }*/
      break;
    case RS_COLOR_ARG_1:
      break;
      /*
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
      break;*/
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
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ONE )
      {
        m_SrcBlend = GL_ONE;
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ZERO )
      {
        m_SrcBlend = GL_ZERO;
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
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ONE )
      {
        m_DestBlend = GL_ONE;
        glBlendFunc( m_SrcBlend, m_DestBlend );
      }
      else if ( rValue == RSV_ZERO )
      {
        m_DestBlend = GL_ZERO;
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
    dh::Log( "OpenGLShaderRenderClass::SetState not supported (%d - %d - Stage %d)", rState, rValue, Stage );
  }

  CheckError( Misc::Format( "OpenGLShaderRenderClass::SetState (%1% - %2% - Stage %3%)" ) << rState << rValue << Stage );
  return bSupported;
}



void OpenGLShaderRenderClass::FlushLightChanges()
{
  if ( m_LightsChanged )
  {
    m_LightsChanged = false;
    UpdateLightsUBO();
  }
}



void OpenGLShaderRenderClass::FlushAllCaches()
{
  FlushLightChanges();
  m_QuadCache3d.FlushCache();
  m_QuadCache.FlushCache();
  m_LineCache.FlushCache();
  m_LineCache3d.FlushCache();
  m_TriangleCache.FlushCache();
  m_TriangleCache3d.FlushCache();
}



void OpenGLShaderRenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{
  FlushAllCaches();

  XBasicRenderer::SetTransform( tType, matTrix );

  switch ( tType )
  {
    case TT_PROJECTION:
      m_Matrices.Projection = matTrix;
      m_Matrices.Projection.Transpose();
      break;
    case TT_WORLD:
      m_Matrices.Model = matTrix;
      m_Matrices.Model.Transpose();
      break;
    case TT_VIEW:
      m_Matrices.View = matTrix;
      m_Matrices.View.Transpose();
      m_Matrices.ViewIT = matTrix;
      m_Matrices.ViewIT.Inverse();
      m_Matrices.ViewIT.Transpose();
      break;
    case TT_TEXTURE_STAGE_0:
      m_StoredTextureTransform = matTrix;
      // my shader hack uses these fields
      m_StoredTextureTransform.ms._14 = matTrix.ms._31;
      m_StoredTextureTransform.ms._24 = matTrix.ms._32;

      if ( m_RenderStates[std::make_pair( XRenderer::RS_TEXTURE_TRANSFORM, 0 )] == XRenderer::RSV_DISABLE )
      {
        // only store, do not set
        return;
      }
      m_Matrices.TextureTransform = m_StoredTextureTransform;
      break;
    default:
      dh::Log( "DX11Renderer::SetTransform unsupported transform type %d", tType );
      break;
  }

  m_QuadCache.TransformChanged( tType );
  m_QuadCache3d.TransformChanged( tType );
  m_LineCache.TransformChanged( tType );
  m_LineCache3d.TransformChanged( tType );
  m_TriangleCache.TransformChanged( tType );
  m_TriangleCache3d.TransformChanged( tType );


  // Prepare the constant buffer to send it to the graphics device.
  UpdateMatrixUBO();

  // camera pos is required for lights
  if ( m_NumActiveLights > 0 )
  {
    if ( tType == TT_WORLD )
    {
      m_LightInfo.EyePos.set( matTrix.ms._41, matTrix.ms._42, matTrix.ms._43 );

      UpdateLightsUBO();
      m_LightsChanged = false;
    }
  }
}



void OpenGLShaderRenderClass::UpdateMaterialUBO()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOMaterial );
  glBufferData( GL_UNIFORM_BUFFER, sizeof( m_Material ), &m_Material, GL_STREAM_DRAW );
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}



void OpenGLShaderRenderClass::UpdateMatrixUBO()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOGlobalMatrices );
  glBufferData( GL_UNIFORM_BUFFER, sizeof( m_Matrices ), &m_Matrices, GL_STREAM_DRAW );
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}



void OpenGLShaderRenderClass::UpdateFogUBO()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOFog );
  glBufferData( GL_UNIFORM_BUFFER, sizeof( m_Fog ), &m_Fog, GL_STREAM_DRAW );
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}



void OpenGLShaderRenderClass::UpdateLightsUBO()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_UBOLights );
  glBufferData( GL_UNIFORM_BUFFER, sizeof( m_LightInfo ), &m_LightInfo, GL_STREAM_DRAW );
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}



bool OpenGLShaderRenderClass::SetViewport( const XViewport& Viewport )
{
  if ( VisibleHeight() == 0 )
  {
    return false;
  }
  if ( !m_2dModeActive )
  {
    glViewport( Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
    CheckError( "SetViewport glViewport 3d" );
    //dh::Log( "Set Viewport to %d,%d  %dx%d", Viewport.X, VisibleHeight() - Viewport.Height - Viewport.Y, Viewport.Width, Viewport.Height );
  }
  else
  {
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

  m_Matrices.ScreenCoord2d.OrthoOffCenterLH( (GR::f32)m_ViewPort.X,
                                             (GR::f32)m_ViewPort.X + m_ViewPort.Width,
                                             (GR::f32)m_ViewPort.Y + m_ViewPort.Height,
                                             (GR::f32)m_ViewPort.Y,
                                             0.0f,
                                             1.0f );
  m_Matrices.ScreenCoord2d.Transpose();
  UpdateMatrixUBO();
  return true;
}



bool OpenGLShaderRenderClass::SetTrueViewport( const XViewport& Viewport )
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



bool OpenGLShaderRenderClass::ToggleFullscreen()
{
  return false;
}



bool OpenGLShaderRenderClass::IsFullscreen()
{
  return m_FullScreen;
}



bool OpenGLShaderRenderClass::SetMode( XRendererDisplayMode& DisplayMode )
{
  return false;
}



void OpenGLShaderRenderClass::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float Z )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

  GR::u32     vertexFormat = XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_XYZRHW;
  if ( m_pSetTextures[0] )
  {
    vertexFormat |= XVertexBuffer::VFF_TEXTURECOORD;
  }

  LineCache     lines( this, vertexFormat );

  lines.AddEntry( m_pSetTextures[0],
                  GR::tVector( m_DisplayOffset.x + m_Canvas.Left + pt1.x * virtualX, 
                               m_DisplayOffset.y + m_Canvas.Top + pt1.y * virtualY, 
                               0.0f ),
                  0.0f, 0.0f,
                  GR::tVector( m_DisplayOffset.x + m_Canvas.Left + pt2.x * virtualX, 
                               m_DisplayOffset.y + m_Canvas.Top + pt2.y * virtualY, 
                               0.0f ),
                  1.0f, 1.0f,
                  Color1, Color2,
                  m_CurrentShaderType );

  lines.FlushCache();
  CheckError( "RenderLine2d" );
}



void OpenGLShaderRenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
{
  dh::Log( "OpenGLShaderRenderClass::RenderLine not yet supported!" );
}




bool OpenGLShaderRenderClass::SetLight( GR::u32 LightIndex, XLight& Light )
{
  CheckError( "SetLight start" );

  bool    hadChange = false;
  if ( m_SetLights[LightIndex].m_Type != XLight::LT_INVALID )
  {
    if ( m_LightEnabled[LightIndex] )
    {
      --m_NumActiveLights;
      hadChange = true;
    }
  }
  m_SetLights[LightIndex] = Light;

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

        m_LightInfo.Light[LightIndex].Direction.set( dir[0], dir[1], dir[2] );
      }
      break;
    case XLight::LT_POINT:
      {
        GR::f32   dir[4];

        memcpy( dir, &Light.m_Position, 3 * sizeof( GR::f32 ) );
        dir[3] = 1.0f;

        m_LightInfo.Light[LightIndex].Direction.set( dir[0], dir[1], dir[2] );
      }
      break;
    case XLight::LT_SPOT:
      {
        GR::f32   dir[4];

        memcpy( dir, &Light.m_Position, 3 * sizeof( GR::f32 ) );
        dir[3] = 1.0f;

        m_LightInfo.Light[LightIndex].Direction.set( dir[0], dir[1], dir[2] );
      }
      break;
  }

  CheckError( "SetLight glLightfv pos" );

  GR::f32   color[4];
  color[0] = ( ( Light.m_Ambient >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Ambient >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Ambient & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Ambient >> 24 ) & 0xff ) / 255.0f;
  m_LightInfo.Light[LightIndex].Ambient = ColorValue( Light.m_Ambient );
  CheckError( "SetLight GL_AMBIENT" );

  color[0] = ( ( Light.m_Diffuse >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Diffuse >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Diffuse & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Diffuse >> 24 ) & 0xff ) / 255.0f;
  m_LightInfo.Light[LightIndex].Diffuse = ColorValue( Light.m_Diffuse );
  CheckError( "SetLight GL_DIFFUSE" );

  color[0] = ( ( Light.m_Specular >> 16 ) & 0xff ) / 255.0f;
  color[1] = ( ( Light.m_Specular >> 8 ) & 0xff ) / 255.0f;
  color[2] = ( Light.m_Specular & 0xff ) / 255.0f;
  color[3] = ( ( Light.m_Specular >> 24 ) & 0xff ) / 255.0f;
  m_LightInfo.Light[LightIndex].Specular = ColorValue( Light.m_Specular );
  CheckError( "SetLight GL_SPECULAR" );

  if ( m_SetLights[LightIndex].m_Type != XLight::LT_INVALID )
  {
    if ( m_LightEnabled[LightIndex] )
    {
      ++m_NumActiveLights;
      hadChange = true;
    }
  }
  if ( hadChange )
  {
    UpdateLightsUBO();
    m_LightsChanged = false;
  }
  else
  {
    m_LightsChanged = true;
  }
  return false;
}



bool OpenGLShaderRenderClass::SetMaterial( const XMaterial& Material )
{
  m_Material.Ambient        = Material.Ambient;
  m_Material.Diffuse        = Material.Diffuse;
  m_Material.Specular       = Material.Specular;
  m_Material.Emissive       = Material.Emissive;
  m_Material.SpecularPower  = Material.Power;

  if ( m_Material.Diffuse == ColorValue( 0 ) )
  {
    m_Material.Diffuse = ColorValue( 0xffffffff );
  }

  UpdateMaterialUBO();
  return false;
}



bool OpenGLShaderRenderClass::SaveScreenShot( const char* szFileName )
{
  return false;
}



bool OpenGLShaderRenderClass::RenderMesh( const Mesh::IMesh& Mesh )
{
  return false;
}



void OpenGLShaderRenderClass::ReleaseAssets()
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




bool OpenGLShaderRenderClass::VSyncEnabled()
{
  //return m_VSyncEnabled;
  return false;
}



void OpenGLShaderRenderClass::EnableVSync( bool Enable )
{
  /*
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
  }
  */
}



void OpenGLShaderRenderClass::SetRenderTarget( XTexture* pTexture )
{
  if ( pTexture == NULL )
  {
    m_pSetRenderTargetTexture = NULL;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    InvertCullMode();
    return;
  }
  if ( !( (OpenGLShaderTexture*)pTexture )->m_AllowUsageAsRenderTarget )
  {
    dh::Log( "SetRenderTarget called with non render target texture!" );
    return;
  }

  m_SetRenderTargetTextureSize = pTexture->m_SurfaceSize;
  glBindFramebuffer( GL_FRAMEBUFFER, ( (OpenGLShaderTexture*)pTexture )->m_RenderTargetFrameBufferID );
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



void OpenGLShaderRenderClass::InvertCullMode()
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