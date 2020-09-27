#include "XBasic2dRenderer.h"

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Grafik/GDIPage.h>
#endif

#include <Misc/Format.h>

#include <Interface/IValueStorage.h>
#include <Interface/IFileLocator.h>
#include <Interface/IDebugService.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>



XBasic2dRenderer::XBasic2dRenderer() :
#if OPERATING_SYSTEM == OS_WINDOWS
  m_WindowedModeStyles( WS_VISIBLE | WS_POPUP | WS_DLGFRAME | WS_SYSMENU | WS_CAPTION ),
  m_WindowedModeExStyles( 0 ),
#endif
  m_pEnvironment( NULL ),
  m_pDebugger( NULL ),
  m_DirectTexelMappingOffset( 0.0f, 0.0f )
{

}



XBasic2dRenderer::~XBasic2dRenderer()
{
}



GR::String XBasic2dRenderer::AppPath( const GR::String& Path )
{
  if ( m_pEnvironment == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  GR::Gamebase::IValueStorage* pStorage = ( GR::Gamebase::IValueStorage* )m_pEnvironment->Service( "ValueStorage" );
  if ( pStorage == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  return pStorage->AppPath( Path.c_str() );

  /*
  IFileLocator*   pLocator = (IFileLocator*)m_pEnvironment->Service( "FileLocator" );
  if ( pLocator == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  return pLocator->LocateFile( Path.c_str() );*/
}



void XBasic2dRenderer::AddTexture( XTexture* pTexture )
{
  m_Textures.push_back( pTexture );
}



void XBasic2dRenderer::AddFont( X2dFont* pFont )
{
  m_Fonts.push_back( pFont );
}



void XBasic2dRenderer::ReleaseAllTextures()
{
  tTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    pTexture->Release();

    ++it;
  }
}



void XBasic2dRenderer::DestroyTexture( XTexture* pTexture )
{
  tTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pThisTexture = *it;
    
    if ( pThisTexture == pTexture )
    {
      pTexture->Release();
      delete pTexture;

      m_Textures.erase( it );

      return;
    }

    ++it;
  }

}



void XBasic2dRenderer::DestroyAllTextures()
{
  tTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    pTexture->Release();
    delete pTexture;

    ++it;
  }
  m_Textures.clear();
}



void XBasic2dRenderer::RestoreAllTextures()
{
}



void XBasic2dRenderer::RestoreAllFonts()
{
  tFonts::iterator   it( m_Fonts.begin() );
  while ( it != m_Fonts.end() )
  {
    X2dFont*   pFont = *it;

    pFont->Recreate();

    ++it;
  }
}



void XBasic2dRenderer::DestroyAllFonts()
{
  tFonts::iterator   it( m_Fonts.begin() );
  while ( it != m_Fonts.end() )
  {
    X2dFont*   pFont = *it;

    delete pFont;

    ++it;
  }
  m_Fonts.clear();
}



void XBasic2dRenderer::AdjustWindowForMode( bool Windowed )
{
#if OPERATING_SYSTEM == OS_WINDOWS
  if ( Windowed )
  {
    //SetParent( m_hwndViewport, m_hwndWindowedModeParent );

    SetWindowPlacement( m_hwndViewport, &m_wPlacement );
    SetWindowLong( m_hwndViewport, GWL_STYLE, m_WindowedModeStyles );
    SetWindowLong( m_hwndViewport, GWL_EXSTYLE, m_WindowedModeExStyles );

    SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED );

    SetMenu( m_hwndViewport, m_hMenu );
  }
  else
  {
    m_hMenu = GetMenu( m_hwndViewport );
    SetMenu( m_hwndViewport, NULL );
    m_wPlacement.length = sizeof( m_wPlacement );
    GetWindowPlacement( m_hwndViewport, &m_wPlacement );
    m_WindowedModeStyles    = GetWindowLong( m_hwndViewport, GWL_STYLE );
    m_WindowedModeExStyles  = GetWindowLong( m_hwndViewport, GWL_EXSTYLE );

    GR::u32   styles = m_WindowedModeStyles;

    //dwStyles &= ~( WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_OVERLAPPED | WS_CHILD | WS_DLGFRAME | WS_THICKFRAME );
    //dwStyles |= WS_POPUP;
    //m_hwndWindowedModeParent = SetParent( m_hwndViewport, NULL );
    styles = WS_POPUP | WS_VISIBLE;
    SetWindowLong( m_hwndViewport, GWL_STYLE, styles );

    SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED );
  }
#endif
}



XViewport XBasic2dRenderer::Viewport()
{
  return m_ViewPort;
}



void XBasic2dRenderer::RenderText( X2dFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, Color );
  }
}



void XBasic2dRenderer::RenderTextCentered( X2dFont* pFont, int CenterX, int Y, const GR::String& Text, GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawText( CenterX - pFont->TextLength( Text ) / 2, Y, Text, Color );
  }
}



void XBasic2dRenderer::RenderTextRightAligned( X2dFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawText( X - pFont->TextLength( Text ), Y, Text, Color );
  }
}



void XBasic2dRenderer::RenderText( X2dFont* pFont, int X, int Y, const GR::String& Text, float ScaleX, float ScaleY, GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, ScaleX, ScaleY, Color );
  }
}



void XBasic2dRenderer::RenderRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color1 )
{
  RenderLine( Pos, 
              GR::tPoint( Pos.x + Size.x - 1, Pos.y ), Color1 );
  RenderLine( GR::tPoint( Pos.x + Size.x - 1, Pos.y ),
              GR::tPoint( Pos.x + Size.x - 1, Pos.y + Size.y - 1 ), Color1 );
  RenderLine( GR::tPoint( Pos.x + Size.x - 1, Pos.y + Size.y - 1 ),
              GR::tPoint( Pos.x, Pos.y + Size.y - 1 ), Color1 );
  RenderLine( Pos, 
              GR::tPoint( Pos.x, Pos.y + Size.y - 1 ), Color1 );
}



X2dFont* XBasic2dRenderer::CreateFontSquare()
{
  X2dFont*   pFont = new X2dFont( this, m_pEnvironment );

#if OPERATING_SYSTEM == OS_WINDOWS
  GR::Graphic::GDIPage     pageTemp;

  pageTemp.Create( m_hwndViewport, 256, 256, 16 );

  HDC       hdcTemp = pageTemp.GetDC();

  SelectObject( hdcTemp, GetStockObject( DEFAULT_GUI_FONT ) );
  SetTextColor( hdcTemp, RGB( 255, 255, 255 ) );//0xffffffff );
  SetBkMode( hdcTemp, TRANSPARENT );

  for ( int i = 0; i < 256; ++i )
  {
    RECT    rc;
    char    cTemp[2];
    
    cTemp[0] = (char)i;
    cTemp[1] = 0;

    SetRect( &rc, ( i % 16 ) * 16, ( i / 16 ) * 16,
                  ( i % 16 ) * 16 + 16, ( i / 16 ) * 16 + 16 );

    TextOut( hdcTemp, rc.left, rc.top, cTemp, 1 );
  }

  GR::Graphic::ImageData    imageData;

  imageData.Attach( 256, 256, 512, GR::Graphic::IF_X1R5G5B5, pageTemp.GetData() );
  imageData.ConvertSelfTo( GR::Graphic::IF_A1R5G5B5, 0, true, 0xff000000 );

  pFont->m_TransparentColor = 0xff000000;

  XTexture*     pTeX2dFont = CreateTexture( imageData );

  pFont->FontFromTexture( X2dFont::FLF_SQUARED_ONE_FONT | X2dFont::FLF_ALPHA_BIT, pTeX2dFont, &imageData );

  pageTemp.ReleaseDC();

#endif
  AddFont( pFont );

  return pFont;
}



void XBasic2dRenderer::Offset( const GR::tPoint& NewOffset )
{
  m_DisplayOffset = NewOffset;
}



GR::tPoint XBasic2dRenderer::Offset() const
{
  return m_DisplayOffset;
}



void XBasic2dRenderer::VirtualSize( const GR::tPoint& NewSize )
{
  m_VirtualSize = NewSize;
}



GR::tPoint XBasic2dRenderer::VirtualSize() const
{
  return m_VirtualSize;
}



void XBasic2dRenderer::Log( const GR::String& System, const char* Format, ... )
{
  if ( m_pDebugger )
  {
    static char    miscBuffer[5000];
#if OPERATING_SYSTEM == OS_WINDOWS
    vsprintf_s( miscBuffer, 5000, Format, (char*)( &Format + 1 ) );
#elif ( OPERATING_SYSTEM == OS_ANDROID )
    va_list args;
    va_start( args, Format );
    vsprintf( miscBuffer, Format, args );
    va_end( args );
#endif

    m_pDebugger->LogDirect( System.c_str(), miscBuffer );
  }
}



X2dFont* XBasic2dRenderer::LoadFontSquare( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  X2dFont* pFont = new X2dFont( this, m_pEnvironment );

  pFont->LoadFontSquare( FileName, Flags, TransparentColor );

  AddFont( pFont );

  return pFont;
}



X2dFont* XBasic2dRenderer::LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  X2dFont*    pFont = new X2dFont( this, m_pEnvironment );

  if ( !pFont->LoadFont( FileName, Flags, TransparentColor ) )
  {
    delete pFont;
    return NULL;
  }
  AddFont( pFont );
  return pFont;
}



void XBasic2dRenderer::DestroyFont( X2dFont* pFont )
{
  m_Fonts.remove( pFont );
}



void XBasic2dRenderer::SortAndCleanDisplayModes()
{
  std::map<GR::String,XRendererDisplayMode>    sortedModes;


  tDisplayModes     clean;


  tDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&     mode( *it );

    GR::String  key = Misc::Format( "%01:8%_%02:8%_%03:8%" ) << mode.Width << mode.Height << mode.ImageFormat;

    sortedModes[key] = mode;

    ++it;
  }

  m_DisplayModes.clear();

  std::map<GR::String,XRendererDisplayMode>::iterator    itM( sortedModes.begin() );
  while ( itM != sortedModes.end() )
  {
    m_DisplayModes.push_back( itM->second );

    ++itM;
  }
}



GR::Graphic::ImageData* XBasic2dRenderer::LoadAndConvert( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  if ( m_pEnvironment == NULL )
  {
    return NULL;
  }
  ImageFormatManager* pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }
  GR::Graphic::ImageData* pData = pManager->LoadData( FileName );
  if ( pData == NULL )
  {
    return NULL;
  }

  // convert format
  if ( FormatToConvert != GR::Graphic::IF_UNKNOWN )
  {
    pData->ConvertSelfTo( FormatToConvert, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
  }

  if ( ColorKey )
  {
    // bei Color-Key soll Alpha mit rein
    if ( pData->BitsProPixel() <= 16 )
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A1R5G5B5 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A1R5G5B5, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
      else if ( IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A8R8G8B8, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
    }
    else
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A8R8G8B8, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
    }
  }
  return pData;
}



