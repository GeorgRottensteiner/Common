#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include <Xtreme/Environment/XWindow.h>

#include "OpenGLRenderClass.h"
#include "OpenGLTexture.h"


#pragma comment( lib, "opengl32.lib" )
#pragma comment( lib, "glu32.lib" )
//#pragma comment( lib, "glaux.lib" )


OpenGLRenderClass::OpenGLRenderClass( HINSTANCE hInstance ) :
  m_RC( NULL ),
  m_DC( NULL ),
  m_FullScreen( false ),
  m_Width( 0 ),
  m_Height( 0 ),
  m_AlphaRefValue( 8.0f / 255.0f ),
  m_AlphaFuncValue( GL_GEQUAL ),
  m_SrcBlend( GL_ONE ),
  m_DestBlend( GL_ZERO ),
  m_OldPixelFormat( 0 ),
  m_Depth( 0 ),
  m_Ready( false )
{
  m_DirectTexelMappingOffset.set( -0.5f, -0.5f );

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

  GLuint		    pixelFormat;

  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );
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

  m_FullScreen = ( Flags & XRenderer::IN_FULLSCREEN );

  if ( m_FullScreen )
  {
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
  }

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

  pixelFormat = ChoosePixelFormat( m_DC, &pfd );
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

  GR::String  version = (char*)glGetString( GL_VERSION );
  //dh::Log( "Open GL version %s", version.c_str() );

  m_Width   = Width;
  m_Height  = Height;
  m_Depth   = Depth;

  m_Canvas.set( 0, 0, Width, Height );
  m_VirtualSize.set( Width, Height );

  OnResized();

  glShadeModel( GL_SMOOTH );
  glClearDepth( 1.0f );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
  glAlphaFunc( m_AlphaFuncValue, m_AlphaRefValue );
  glBlendFunc( m_SrcBlend, m_DestBlend );
  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );

  GR::String     openGLVersion = (char*)glGetString( GL_VERSION );


  SortAndCleanDisplayModes();

  NotifyService( "Renderer", "Initialised" );

  m_Ready = true;
  return true;
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

  m_Ready = false;
  NotifyService( "Renderer", "Released" );
  return true;
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

  glDisable( GL_LIGHTING );

  glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, this->Width(), this->Height(), 0, -1.0f, 1.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
  glTranslated( X, Y, (int)Z );

  glBegin( GL_QUADS );
  glTexCoord2d( TU1, TV1 );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
	glVertex2f( 0, 0 );

  glTexCoord2d( TU2, TV2 );
  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
	glVertex2f( Width - 1.0f, 0 );

  glTexCoord2d( TU4, TV4 );
  glColor4ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ), ( Color4 & 0xff000000 ) >> 24 );
	glVertex2f( Width - 1.0f, Height - 1.0f );

  glTexCoord2d( TU3, TV3 );
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
	glVertex2f( 0, Height - 1.0f );

  glEnd();
  CheckError();
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

  glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, this->Width(), this->Height(), 0, -1.0f, 1.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

  glDisable( GL_LIGHTING );

  glTranslatef( X, Y, Z );

  glBegin( GL_QUADS );
  glColor3ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU1, TV1 );
	glVertex2f( 0, 0 );

  glColor3ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU2, TV2 );
	glVertex2f( Width - 1.0f, 0 );

  glColor3ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU4, TV4 );
	glVertex2f( Width - 1.0f, Height - 1.0f );

  glColor3ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU3, TV3 );
	glVertex2f( 0, Height - 1.0f );

  glEnd();
  CheckError();
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

  glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, Width(), Height(), 0, -1.0f, 1.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

  glDisable( GL_LIGHTING );

  glBegin( GL_QUADS );
  glColor3ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU1, TV1 );
	glVertex3f( X1, Y1, Z );

  glColor3ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU2, TV2 );
	glVertex3f( X2, Y2, Z );

  glColor3ub( (GLubyte)( ( Color4 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color4 & 0x00ff00 ) >> 8 ), (GLubyte)( Color4 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU4, TV4 );
	glVertex3f( X4, Y4, Z );

  glColor3ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ) );//, ( Color1 & 0xff000000 ) >> 24 );
	glTexCoord2d( TU3, TV3 );
	glVertex3f( X3, Y3, Z );

  glEnd();
  CheckError();
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

  glDisable( GL_LIGHTING );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, this->Width(), this->Height(), 0, -1.0f, 1.0f );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslated( (GLdouble)m_DisplayOffset.x, (GLdouble)m_DisplayOffset.y, (GLdouble)Z );

  glBegin( GL_TRIANGLES );
  glTexCoord2d( TU1, TV1 );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
  glVertex2f( (GLfloat)pt1.x, (GLfloat)pt1.y );

  glTexCoord2d( TU2, TV2 );
  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
  glVertex2f( (GLfloat)pt2.x, (GLfloat)pt2.y );

  glTexCoord2d( TU3, TV3 );
  glColor4ub( (GLubyte)( ( Color3 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color3 & 0x00ff00 ) >> 8 ), (GLubyte)( Color3 & 0xff ), ( Color3 & 0xff000000 ) >> 24 );
  glVertex2f( (GLfloat)pt3.x, (GLfloat)pt3.y );

  glEnd();
  CheckError();
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
}



void OpenGLRenderClass::CheckError()
{
  int error = glGetError();
  if ( error != 0 )
  {
    dh::Log( "CreateTexture error %d", error );
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
  OpenGLTexture*   pTexture = new OpenGLTexture();

  glGenTextures( 1, &pTexture->m_TextureID );
  if ( pTexture->m_TextureID == 0 )
  {
    return NULL;
  }

  pTexture->m_ImageFormat = ImageData.ImageFormat();
  pTexture->m_SurfaceSize.set( ImageData.Width(), ImageData.Height() );
  pTexture->m_ImageSourceSize.set( ImageData.Width(), ImageData.Height() );

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

  // Give the image to OpenGL
  switch ( ImageData.ImageFormat() )
  {
    case GR::Graphic::IF_R8G8B8:
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ImageData.Width(), ImageData.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, ImageData.Data() );
      break;
    case GR::Graphic::IF_A8R8G8B8:
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ImageData.Width(), ImageData.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, ImageData.Data() );
      break;
    default:
      {
        GR::Graphic::ImageData      convertedImage( ImageData );

        if ( GR::Graphic::ImageData::FormatHasAlpha( ImageData.ImageFormat() ) )
        {
          convertedImage.ConvertSelfTo( GR::Graphic::IF_A8R8G8B8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, convertedImage.Width(), convertedImage.Height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, convertedImage.Data() );
        }
        else
        {
          // TODO - differ if it has alpha
          convertedImage.ConvertSelfTo( GR::Graphic::IF_R8G8B8 );
          glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_BGR, GL_UNSIGNED_BYTE, convertedImage.Data() );
        }
      }
      break;
  }
  glBindTexture( GL_TEXTURE_2D, 0 );
  CheckError();
  return pTexture;
}



XTexture* OpenGLRenderClass::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  OpenGLTexture*   pTexture = new OpenGLTexture();

  glGenTextures( 1, &pTexture->m_TextureID );

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture( GL_TEXTURE_2D, pTexture->m_TextureID );

  unsigned char texDat[64];
  for ( int i = 0; i < 64; ++i )
  {
    texDat[i] = ( ( i + ( i / 8 ) ) % 2 ) * 128 + 127;
  }

  GR::Graphic::ImageData      dummyImage;

  dummyImage.CreateData( Width, Height, imgFormat );

  // Give the image to OpenGL
  switch ( imgFormat )
  {
    case GR::Graphic::IF_R8G8B8:
      //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, dummyImage.Width(), dummyImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, dummyImage.Data() );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, texDat );
      break;
    case GR::Graphic::IF_A8R8G8B8:
      //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, dummyImage.Width(), dummyImage.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, dummyImage.Data() );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, texDat );
      break;
    default:
      {
        // TODO - differ if it has alpha
        GR::Graphic::ImageData      convertedImage( dummyImage );

        convertedImage.ConvertSelfTo( GR::Graphic::IF_R8G8B8 );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, convertedImage.Width(), convertedImage.Height(), 0, GL_RGB, GL_UNSIGNED_BYTE, convertedImage.Data() );
      }
      break;
  }
  int error = glGetError();
  if ( error != 0 )
  {
    dh::Log( "CreateTexture error %d", error );
  }

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
  CheckError();
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  return NULL;
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  return NULL;
}



XVertexBuffer* OpenGLRenderClass::CreateVertexBuffer( const Mesh::IMesh& Mesh, GR::u32 VertexFormat )
{
  return NULL;
}



void OpenGLRenderClass::DestroyVertexBuffer( XVertexBuffer* )
{
}



bool OpenGLRenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  return false;
}



bool OpenGLRenderClass::BeginScene()
{
  glLoadIdentity();
  return true;
}



void OpenGLRenderClass::EndScene()
{
}



void OpenGLRenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  SwapBuffers( m_DC );
}



bool OpenGLRenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  return false;
}



bool OpenGLRenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{
  return false;
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
  RECT    rc;

  GetClientRect( m_hwndViewport, &rc );


  glViewport( 0, 0, rc.right - rc.left, rc.bottom - rc.top );
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
    glClear( flags );
  }

}



void OpenGLRenderClass::SetAlpha( GR::u32 AlphaTest, GR::u32 AlphaBlend )
{
  if ( ( AlphaBlend == RSV_ENABLE )
  ||   ( AlphaTest == RSV_ENABLE ) )
  {
    glEnable( GL_BLEND );
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.f );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  else
  {
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glBlendFunc( GL_ONE, GL_ZERO );
  }
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  /*
  if ( AlphaBlend == RSV_ENABLE )
  {
    // if alpha and alpha texture just modulate, otherwise use only the alpha channel
    if ( AlphaTest == RSV_ENABLE )
    {
      glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    }
    else
    {
      if ( m_pSetTextures[0] == NULL )
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
        // rgb always modulates
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT );
      }
      else
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE );
        // rgb always modulates
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT );
      }
    }
  }
  else
  {
    if ( AlphaTest == RSV_ENABLE )
    {
      if ( m_pSetTextures[0] == NULL )
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
        // rgb always modulates
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT );
      }
      else
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
        // rgb always modulates
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PRIMARY_COLOR_EXT );
      }
    }
    else
    {
      if ( m_pSetTextures[0] == NULL )
      {
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT );
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_PRIMARY_COLOR_EXT );
        // rgb always modulates
        glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE );
        glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_PRIMARY_COLOR_EXT );
      }
      else
      {
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
      }
    }
  }*/
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
    case RS_COLOR_OP:
      if ( rValue == RSV_MODULATE )
      {
        //glActiveTexture( Stage );
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
        glEnable( GL_CULL_FACE );
        glFrontFace( GL_CW );
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

        //glEnable( GL_ALPHA_TEST );
        //glEnable( GL_BLEND );
      }
      else if ( rValue == RSV_DISABLE )
      {
        SetAlpha( rValue, m_RenderStates[std::make_pair( RS_ALPHABLENDING, Stage )] );
        //glDisable( GL_ALPHA_TEST );
        //glDisable( GL_BLEND );
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
        //glDisable( GL_ALPHA_TEST );
        //glEnable( GL_BLEND );
      }
      else if ( rValue == RSV_DISABLE )
      {
        SetAlpha( m_RenderStates[std::make_pair( RS_ALPHATEST, Stage )], rValue );
        //glDisable( GL_ALPHA_TEST );
        //glDisable( GL_BLEND );
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
  return bSupported;
}



void OpenGLRenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{
}



bool OpenGLRenderClass::SetViewport( const XViewport& Viewport )
{
  glViewport( Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );

  m_ViewPort.X      = Viewport.X;
  m_ViewPort.Y      = Viewport.Y;
  m_ViewPort.Width  = Viewport.Width;
  m_ViewPort.Height = Viewport.Height;
  return true;
}



bool OpenGLRenderClass::SetTrueViewport( const XViewport& Viewport )
{
  glViewport( Viewport.X, Viewport.Y, Viewport.Width, Viewport.Height );
  m_ViewPort.X      = Viewport.X;
  m_ViewPort.Y      = Viewport.Y;
  m_ViewPort.Width  = Viewport.Width;
  m_ViewPort.Height = Viewport.Height;
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
  if ( ( Color2 == 0 ) )
  {
    Color2 = Color1;
  }

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

  glDisable( GL_LIGHTING );

  glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, this->Width(), this->Height(), 0, -1.0f, 1.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
  glTranslated( m_DisplayOffset.x, m_DisplayOffset.y, (int)Z );

  glBegin( GL_LINES );
  glColor4ub( (GLubyte)( ( Color1 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color1 & 0x00ff00 ) >> 8 ), (GLubyte)( Color1 & 0xff ), ( Color1 & 0xff000000 ) >> 24 );
	glVertex2f( m_Canvas.Left + pt1.x * virtualX, m_Canvas.Top + pt1.y * virtualY );

  glColor4ub( (GLubyte)( ( Color2 & 0xff0000 ) >> 16 ), (GLubyte)( ( Color2 & 0x00ff00 ) >> 8 ), (GLubyte)( Color2 & 0xff ), ( Color2 & 0xff000000 ) >> 24 );
	glVertex2f( m_Canvas.Left + pt2.x * virtualX, m_Canvas.Top + pt2.y * virtualY );

  glEnd();
  CheckError();
}



void OpenGLRenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
{
}




bool OpenGLRenderClass::SetLight( GR::u32 LightIndex, XLight& Light )
{
  return false;
}



bool OpenGLRenderClass::SetMaterial( const XMaterial& Material )
{
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
}
