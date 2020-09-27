#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>
#include <Grafik/GFXHelper.h>

#include <Xtreme/Environment/XWindow.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont2D.h>

#include <Misc/Misc.h>

#include <String/XML.h>

#include <IO/FileStream.h>

#include "DDrawRenderclass.h"
#include "DDrawTexture.h"


static CDDrawRenderClass*   g_pDDrawInstance = NULL;



CDDrawRenderClass::CDDrawRenderClass( HINSTANCE hInstance ) :
  m_Ready( false ),
  m_ForceWindowSize( false ),
  m_Windowed( true ),
  m_hInstance( hInstance ),
  m_SaveScreenShotFileName( "" ),
  m_Depth( 0 ),
  m_pDirectXPage( NULL ),
  m_pWinGPage( NULL )
{
  if ( m_hInstance == NULL )
  {
    m_hInstance = GetModuleHandle( NULL );
  }
  g_pDDrawInstance = this;
}



CDDrawRenderClass::~CDDrawRenderClass()
{
  Release();
  g_pDDrawInstance = NULL;
}



bool CDDrawRenderClass::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment )
{
  if ( m_Ready )
  {
    // schon initialisiert
    return true;
  }

  m_pEnvironment = &Environment;
  m_pDebugger = (IDebugService*)m_pEnvironment->Service( "Logger" );

  m_CreationWidth   = Width;
  m_CreationHeight  = Height;
  m_CreationDepth   = Depth;

  m_Windowed        = !FullScreen;

  Xtreme::IAppWindow* pWindowService = (Xtreme::IAppWindow*)Environment.Service( "Window" );
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }


  if ( !m_Windowed )
  {
    // Fenster-Werte für Windows-Mode merken
    AdjustWindowForMode( false );

    m_WindowedModeStyles |= WS_CAPTION | WS_DLGFRAME | WS_SYSMENU;

    RECT    rcWindow;
    GetWindowRect( hWnd, &rcWindow );

    OffsetRect( &rcWindow, -rcWindow.left, -rcWindow.top );
    OffsetRect( &rcWindow, 
                ( GetSystemMetrics( SM_CXSCREEN ) - ( rcWindow.right - rcWindow.left ) ) / 2,
                ( GetSystemMetrics( SM_CYSCREEN ) - ( rcWindow.bottom - rcWindow.top ) ) / 2 );

    m_wPlacement.rcNormalPosition = rcWindow;
  }

  // Unless a substitute hWnd has been specified, create a window to
  // render into
  m_hwndViewport = hWnd;
  if ( m_hwndViewport == NULL)
  {
    dh::Log( "Create: no valid window handle" );
    return false;
  }

  if ( !CreatePage() )
  {
    dh::Log( "Create: Failed to setup page" );
    return false;
  }

  //m_OldWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr( m_hwndViewport, GWL_WNDPROC, (LONG)(LONG_PTR)DDrawWndProc );

  // The app is ready to go
  m_Ready = TRUE;

  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu laden
      GR::up    assetCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE );
      for ( GR::up i = 0; i < assetCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );

        GR::u32     ForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        XTexture* pTexture = LoadTexture( CMisc::AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" ) ).c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey );

        if ( pTexture )
        {
          if ( pAsset )
          {
            dh::Error( "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetImage( pTexture );
          pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pAsset );
        }
        else
        {
          dh::Error( "DX8Renderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) );
        }
      }

      assetCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < assetCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          dh::Error( "DX8Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        }
        else
        {
          XTextureSection   tsSection;

          tsSection.m_XOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "X" ) );
          tsSection.m_YOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Y" ) );
          tsSection.m_Width   = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "W" ) );
          tsSection.m_Height  = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "H" ) );
          tsSection.m_Flags   = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Flags" ) );

          tsSection.m_pTexture = pTexture->Texture();

          if ( pAsset )
          {
            dh::Error( "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetImageSection( tsSection );
          pLoader->SetAsset( Xtreme::Asset::XA_IMAGE_SECTION, i, pAsset );
        }
      }

      assetCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_FONT );
      for ( GR::up i = 0; i < assetCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_FONT, i );

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     LoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        X2dFont*      pFont = NULL;

        if ( ( LoadFlags & X2dFont::FLF_SQUARED )
        ||   ( LoadFlags & X2dFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( CMisc::AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
                                                    LoadFlags, 
                                                    ColorKey );
        }
        else
        {
          pFont = LoadFont( CMisc::AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), ColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            dh::Error( "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont2d( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          dh::Error( "DX8Renderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }

  return true;
}



bool CDDrawRenderClass::Release()
{
  if ( !m_Ready )
  {
    return true;
  }

  ReleaseAssets();

  m_Ready = false;
  DestroyAllFonts();
  DestroyAllTextures();

  //SetWindowLong( m_hwndViewport, GWL_WNDPROC, (LONG)(LONG_PTR)m_OldWndProc );

  ReleasePage();

  return true;
}



void CDDrawRenderClass::RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color )
{
  if ( TexSection.m_pTexture == NULL )
  {
    RenderQuad( X, Y, TexSection.m_Width, TexSection.m_Height, Color );
  }
  else if ( GR::Graphic::ImageData::FormatHasAlpha( TexSection.m_pTexture->m_ImageFormat ) )
  {
    RenderTextureSectionWithAlphaLayer( X, Y, TexSection, Color );
  }
  else
  {
    RenderTextureSectionColorized( X, Y, TexSection, Color );
  }
}



bool CDDrawRenderClass::CreatePage()
{
  m_RenderWidth   = m_CreationWidth;
  m_RenderHeight  = m_CreationHeight;
  m_Depth         = m_CreationDepth;

  // den Viewport aufbewahren (für Pure-Devices)
  // das sind die Default-Werte (laut Dokumentation)
  m_ViewPort.X        = 0;
  m_ViewPort.Y        = 0;
  m_ViewPort.Width    = m_RenderWidth;
  m_ViewPort.Height   = m_RenderHeight;
  m_ViewPort.MinZ     = 0.0f;
  m_ViewPort.MaxZ     = 1.0f;

  if ( m_Windowed )
  {
    m_pWinGPage = new GR::Graphic::GDIPage();
    if ( !m_pWinGPage->Create( m_hwndViewport, m_RenderWidth, m_RenderHeight, (unsigned char)m_Depth ) )
    {
      dh::Log( "create windowed failed" );
      return false;
    }
    return true;
  }
  m_pDirectXPage = new GR::Graphic::DirectXPage();
  if ( m_pDirectXPage->Create( m_hwndViewport, m_RenderWidth, m_RenderHeight, (unsigned char)m_Depth ) )
  {
    return true;
  }
  if ( m_Depth == 24 )
  {
    m_Depth = 32;
    if ( m_pDirectXPage->Create( m_hwndViewport, m_RenderWidth, m_RenderHeight, (unsigned char)m_Depth ) )
    {
      return true;
    }
    m_Depth = 24;
  }
  if ( m_Depth == 32 )
  {
    m_Depth = 24;
    if ( m_pDirectXPage->Create( m_hwndViewport, m_RenderWidth, m_RenderHeight, (unsigned char)m_Depth ) )
    {
      return true;
    }
    m_Depth = 32;
  }
  return false;
}



bool CDDrawRenderClass::ReleasePage()
{
  if ( m_pWinGPage )
  {
    delete m_pWinGPage;
    m_pWinGPage = NULL;
  }
  if ( m_pDirectXPage )
  {
    delete m_pDirectXPage;
    m_pDirectXPage = NULL;
  }
  return true;
}



bool CDDrawRenderClass::IsReady() const
{
  return m_Ready;
}



bool CDDrawRenderClass::ResizeSurface()
{
  static BOOL   bInside = FALSE;

  if ( bInside )
  {
    return TRUE;
  }
  bInside = TRUE;

  if ( !IsWindow( m_hwndViewport ) )
  {
    return FALSE;
  }

  if ( m_Windowed )
  {
    RECT rcWindowClient;

    GetClientRect( m_hwndViewport, &rcWindowClient );

    m_Ready = FALSE;

    m_RenderWidth   = rcWindowClient.right - rcWindowClient.left;
    m_RenderHeight  = rcWindowClient.bottom - rcWindowClient.top;

    // Resize the 3D environment
    ReleasePage();
    if ( !CreatePage() )
    {
      dh::Log( "MsgProc: Failed to Resize3DEnvironment" );
      bInside = FALSE;
      m_Ready = FALSE;
      return FALSE;
    }
    m_Ready = TRUE;
  }

  bInside = FALSE;

  return TRUE;
}



bool CDDrawRenderClass::BeginScene()
{
  if ( !m_Ready )
  {
    return false;
  }

  if ( m_pDirectXPage )
  {
    if ( !m_pDirectXPage->GFXBegin() )
    {
      return false;
    }
  }
  return true;
}



void CDDrawRenderClass::EndScene()
{
  if ( m_pDirectXPage )
  {
    m_pDirectXPage->GFXDone();
  }
}



void CDDrawRenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  if ( !m_SaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_SaveScreenShotFileName.c_str() );

    m_SaveScreenShotFileName.clear();
  }

  // Show the frame on the primary surface.
  if ( m_pDirectXPage )
  {
    m_pDirectXPage->GFXUpdate();
  }
  else if ( m_pWinGPage )
  {
    m_pWinGPage->GFXUpdate( FALSE, FALSE, NULL, NULL );
    /*
    if ( IsWindow( m_hwndViewport ) )
    {
      InvalidateRect( m_hwndViewport, NULL, FALSE );
    }*/
  }
}



GR::Graphic::GFXPage* CDDrawRenderClass::CurrentPage()
{
  if ( m_pDirectXPage )
  {
    return m_pDirectXPage;
  }
  if ( m_pWinGPage )
  {
    return m_pWinGPage;
  }
  return NULL;
}



XMultiTexture* CDDrawRenderClass::LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey )
{
  XMultiTexture2d*    pTex2d = new XMultiTexture2d( this );

  if ( !pTex2d->LoadImage( *m_pEnvironment, FileName, 16384, 16384 ) )
  {
    delete pTex2d;
    return NULL;
  }

  m_MultiTextures.push_back( pTex2d );
  return pTex2d;
}



XTexture* CDDrawRenderClass::LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  GR::String   path = FileName;

  ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }
  GR::Graphic::ImageData*   pData = pManager->LoadData( path.c_str() );
  if ( pData == NULL )
  {
    return NULL;
  }

  // convert format
  if ( FormatToConvert != GR::Graphic::IF_UNKNOWN )
  {
    pData->ConvertSelfTo( FormatToConvert, 0, ColorKey != 0, ColorKey );
  }

  if ( ColorKey )
  {
    // bei Color-Key soll Alpha mit rein
    if ( pData->BitsProPixel() <= 16 )
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A1R5G5B5 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A1R5G5B5, 0, ColorKey != 0, ColorKey );
      }
    }
    else
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A8R8G8B8, 0, ColorKey != 0, ColorKey );
      }
    }
  }

  XTexture*   pTexture = CreateTexture( *pData );
  if ( pTexture == NULL )
  {
    delete pData;
    return NULL;
  }

  pTexture->m_ColorKey        = ColorKey;
  pTexture->m_ColorKeyReplacementColor = 0;
  pTexture->m_LoadedFromFile = path;

  CopyDataToTexture( pTexture, *pData, ColorKey );

  delete pData;
  return pTexture;
}



XTexture* CDDrawRenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat() );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );

  return pTexture;
}



XTexture* CDDrawRenderClass::CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat Format )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    return NULL;
  }


  if ( !IsTextureFormatOK( Format ) )
  {
    // unterstützte Wechsel
    if ( Format == GR::Graphic::IF_R8G8B8 )
    {
      Format = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( Format ) )
      {
        return NULL;
      }
    }
    else if ( Format == GR::Graphic::IF_X1R5G5B5 )
    {
      Format = GR::Graphic::IF_R5G6B5;
      if ( !IsTextureFormatOK( Format ) )
      {
        return NULL;
      }
    }
    else
    {
      return NULL;
    }
  }

  CDDrawTexture*          pTexture = NULL;

  pTexture = new( std::nothrow )CDDrawTexture( this );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  pTexture->m_cdImage.CreateData( Width, Height, Format );

  pTexture->m_ImageFormat         = Format;
  pTexture->m_SurfaceSize.set( Width, Height );
  pTexture->m_ImageSourceSize.set( Width, Height );

  AddTexture( pTexture );

  return pTexture;
}



bool CDDrawRenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat Format )
{
  switch ( Format )
  {
    case GR::Graphic::IF_INDEX8:
    case GR::Graphic::IF_A8R8G8B8:
    case GR::Graphic::IF_A8:
    case GR::Graphic::IF_X8R8G8B8:
    case GR::Graphic::IF_R5G6B5:
    case GR::Graphic::IF_X1R5G5B5:
    case GR::Graphic::IF_R8G8B8:
    case GR::Graphic::IF_A1R5G5B5:
      return true;
  }
  return false;
}



bool CDDrawRenderClass::CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData )
{
  if ( pTexture == NULL )
  {
    return false;
  }

  CDDrawTexture*    pDDrawTexture = (CDDrawTexture*)pTexture;

  ImageData = pDDrawTexture->m_cdImage;

  return true;
}



bool CDDrawRenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  if ( pTexture == NULL )
  {
    return false;
  }
  if ( pTexture->m_ImageFormat != ImageData.ImageFormat() )
  {
    dh::Log( "CDDrawRenderClass::CopyDataToTexture mismatching format %d != %d!", pTexture->m_ImageFormat, ImageData.ImageFormat() );
    return false;
  }

  CDDrawTexture*    pDDrawTexture = (CDDrawTexture*)pTexture;

  // neu erzeugen
  pDDrawTexture->m_cdImage.CreateData( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), ImageData.LineOffsetInBytes() );

  /*
  if ( ColorKey )
  {
    if ( !ImageData.ConvertInto( &pDDrawTexture->m_cdImage, true, ColorKey ) )
    {
      dh::Log( "CDDrawRenderClass::CopyDataToTexture failed!" );
    }
  }
  else
  {
    if ( !ImageData.ConvertInto( &pDDrawTexture->m_cdImage ) )
    {
      dh::Log( "CDDrawRenderClass::CopyDataToTexture without CK failed!" );
    }
  }
  */
  if ( !ImageData.ConvertInto( &pDDrawTexture->m_cdImage ) )
  {
    dh::Log( "CDDrawRenderClass::CopyDataToTexture without CK failed!" );
  }

  pDDrawTexture->m_ImageFormat = ImageData.ImageFormat();
  pDDrawTexture->m_TransparentColor = ColorKey;
  pDDrawTexture->m_rleList.Release();
  if ( ColorKey )
  {
    pDDrawTexture->m_rleList.Compress( pDDrawTexture->m_cdImage, 0, 0, ImageData.Width(), ImageData.Height(), ColorKey, true );
  }


  return true;
}



void CDDrawRenderClass::RestoreAllTextures()
{
}



bool CDDrawRenderClass::OnResized()
{
  return ResizeSurface();
}



bool CDDrawRenderClass::SetViewport( XViewport& Viewport )
{
  m_ViewPort = Viewport;

  GR::Graphic::GFXPage*   pPage = CurrentPage();

  if ( pPage )
  {
    pPage->SetRange( m_ViewPort.X, 
                     m_ViewPort.Y, 
                     m_ViewPort.X + m_ViewPort.Width - 1,
                     m_ViewPort.Y + m_ViewPort.Height - 1 );
  }
  return true;
}



GR::u32 CDDrawRenderClass::Width()
{
  return m_RenderWidth;
}



GR::u32 CDDrawRenderClass::Height()
{
  return m_RenderHeight;
}



GR::u32 CDDrawRenderClass::Depth()
{
  return m_Depth;
}



bool CDDrawRenderClass::IsFullscreen()
{
  return !m_Windowed;
}



bool CDDrawRenderClass::ToggleFullscreen()
{
  m_Windowed = !m_Windowed;

  ReleasePage();
  AdjustWindowForMode( m_Windowed );
  return CreatePage();
}



bool CDDrawRenderClass::SetMode( XRendererDisplayMode& DisplayMode )
{
  return false;
  /*
  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&   DispMode = *it;
    if ( DispMode == DisplayMode )
    {
      // der Mode existiert
      m_bWindowed = false;
      m_FullscreenDisplayMode = DisplayMode;
      
      ReleasePage();
      return CreatePage();
    }

    ++it;
  }
  dh::Log( "CDDrawRenderClass::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.m_Width, DisplayMode.m_Height, DisplayMode.m_ImageFormat );
  return false;
  */
}



void CDDrawRenderClass::DoSaveSnapShot( const GR::String& FileName )
{
  GR::Graphic::Image        imgSnapshot;

  imgSnapshot.Set( (WORD)m_RenderWidth, (WORD)m_RenderHeight, (unsigned char)m_Depth, 0, 0 );

  imgSnapshot.GetImage( CurrentPage(), 0, 0 );

  bool            rle = false;

  unsigned char   header[18];

  GR::u16         width,
                  height;

  GR::IO::FileStream       outFile;

  if ( !outFile.Open( FileName, IIOStream::OT_WRITE_ONLY ) )
  {
    return;
  }

  // Header zusammenbauen
  ZeroMemory( header, 18 );

  header[0] = 0;
  header[1] = 0;
  header[2] = 2;

  header[10] = m_RenderHeight % 256;
  header[11] = (unsigned char)( m_RenderHeight / 256 );

  header[12] = m_RenderWidth % 256;
  header[13] = (unsigned char)( m_RenderWidth / 256 );
  header[14] = m_RenderHeight % 256;
  header[15] = (unsigned char)( m_RenderHeight / 256 );
  header[16] = 24; 

  outFile.WriteBlock( &header, 18 );


  // ID-Feld überlesen
  outFile.SetPosition( header[0], IIOStream::PT_CURRENT );

  // ein umkomprimiertes Bild
  height = (GR::u16)m_RenderHeight;
  width  = (GR::u16)m_RenderWidth;

  GR::u32*    pData = (GR::u32*)imgSnapshot.GetData();

  for ( int j = height - 1; j >= 0; j-- )
  {
    switch ( GR::Graphic::ImageData::ImageFormatFromDepth( imgSnapshot.GetDepth() ) )
    {
      case GR::Graphic::IF_X1R5G5B5:
        {
          GR::u16     pixel;

          for ( GR::i32 i = 0; i < m_RenderWidth; i++ )
          {
            pixel = ( (WORD*)imgSnapshot.GetData() )[i + j * imgSnapshot.GetLineSize() / 2];

            outFile.WriteU8( ( ( pixel & 0x001f )       ) * 255 / 31 );
            outFile.WriteU8( ( ( pixel & 0x03e0 ) >> 5  ) * 255 / 31 );
            outFile.WriteU8( ( ( pixel & 0x7c00 ) >> 10 ) * 255 / 31 );
          }
        }
        break;
      case GR::Graphic::IF_R5G6B5:
        {
          GR::u16     pixel;

          for ( GR::i32 i = 0; i < m_RenderWidth; i++ )
          {
            pixel = ( (WORD*)imgSnapshot.GetData() )[i + j * imgSnapshot.GetLineSize() / 2];

            outFile.WriteU8( ( ( pixel & 0x001f )       ) * 255 / 31 );
            outFile.WriteU8( ( ( pixel & 0x07e0 ) >> 5  ) * 255 / 63 );
            outFile.WriteU8( ( ( pixel & 0xf800 ) >> 11 ) * 255 / 31 );
          }
        }
        break;
      case GR::Graphic::IF_X8R8G8B8:
        {
          GR::u32     pixel = 0;

          for ( GR::i32 i = 0; i < m_RenderWidth; i++ )
          {
            memcpy( &pixel, pData, 3 );

            outFile.WriteU8( unsigned char( ( pixel & 0xff0000 ) >> 16 ) );
            outFile.WriteU8( unsigned char( ( pixel & 0x00ff00 ) >> 8  ) );
            outFile.WriteU8( unsigned char( ( pixel & 0x0000ff )       ) );

            pData += 4;
          }
          pData += ( imgSnapshot.GetLineSize() - m_RenderWidth ) / 4;
        }
        break;
      case GR::Graphic::IF_R8G8B8:
        {
          GR::u32     pixel = 0;

          for ( GR::i32 i = 0; i < m_RenderWidth; i++ )
          {
            memcpy( &pixel, pData, 3 );

            outFile.WriteU8( (BYTE)( ( pixel & 0xff0000 ) >> 16 ) );
            outFile.WriteU8( (BYTE)( ( pixel & 0x00ff00 ) >> 8  ) );
            outFile.WriteU8( (BYTE)( ( pixel & 0x0000ff )       ) );

            pData += 3;
          }
          pData += ( imgSnapshot.GetLineSize() - m_RenderWidth ) / 3;
        }
        break;
    }
  }
  outFile.Close();
}



bool CDDrawRenderClass::SaveScreenShot( const GR::String& FileName )
{
  if ( FileName )
  {
    m_SaveScreenShotFileName = FileName;
    return true;
  }
  return false;
}




void CDDrawRenderClass::RenderLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color )
{
  GR::Graphic::GFXPage*   pCurrentPage = CurrentPage();

  pCurrentPage->Line( Pos1.x, Pos1.y, Pos2.x, Pos2.y, pCurrentPage->GetRGB256( Color ) );
}




void CDDrawRenderClass::RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                        GR::i32 X2, GR::i32 Y2,
                                        GR::i32 X3, GR::i32 Y3,
                                        GR::u32 Color )
{
  GR::Graphic::GFXPage*   pCurrentPage = CurrentPage();
  
  GR::u32 color = pCurrentPage->GetRGB256( Color );

  pCurrentPage->Line( X1, Y1, X2, Y2, color );
  pCurrentPage->Line( X2, Y2, X3, Y3, color );
  pCurrentPage->Line( X3, Y3, X1, Y1, color );
}



void CDDrawRenderClass::RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height, GR::u32 Color )
{
  GR::Graphic::GFXPage*   pCurrentPage = CurrentPage();

  if ( ( Color & 0xff000000 ) != 0xff000000 )
  {
    // Alpha!
    GR::Graphic::ContextDescriptor      cdPage( pCurrentPage );

    cdPage.AlphaBox( X, Y, X + Width - 1, Y + Height - 1, Color | 0xff000000, ( Color & 0xff000000 ) >> 24 );
  }
  else
  {
    pCurrentPage->Box( X, Y, X + Width - 1, Y + Height - 1, pCurrentPage->GetRGB256( Color ) );
  }
}




void CDDrawRenderClass::RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                          GR::i32 MaskX, GR::i32 MaskY, 
                                          const XTextureSection& TexSectionMask, 
                                          const GR::u32 Color,
                                          const GR::u32 MaskColorKey )
{
  // überlappendes Rect finden
  GR::tRect     rcSection( X, Y, Width, Height );
  GR::tRect     rcMask( MaskX, MaskY, TexSectionMask.m_Width, TexSectionMask.m_Height );

  GR::tRect     rcOverlap( rcSection.intersection( rcMask ) );

  if ( ( rcOverlap.width() == 0 )
  ||   ( rcOverlap.height() == 0 ) )
  {
    return;
  }

  CDDrawTexture* pTextureMask = (CDDrawTexture*)TexSectionMask.m_pTexture;
  if ( pTextureMask == NULL )
  {
    return;
  }


  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImageMask( pTextureMask->m_cdImage );

  if ( cdImageMask.TransparentColor() != MaskColorKey )
  {
    cdImageMask.ConvertSelfTo( cdImageMask.BitsProPixel(), cdImageMask.LineOffsetInBytes(), true, MaskColorKey );
  }

  XTextureSection     TS2( TexSectionMask );

  TS2.m_XOffset += rcOverlap.Left - MaskX;
  TS2.m_YOffset += rcOverlap.Top - MaskY;

  Width         = rcOverlap.width();
  Height        = rcOverlap.height();
  TS2.m_Width   = rcOverlap.width();
  TS2.m_Height  = rcOverlap.height();

  for ( int y = 0; y < Height; ++y )
  {
    for ( int x = 0; x < Width; ++x )
    {
      if ( ( cdImageMask.GetPixel( TS2.m_XOffset + x, TS2.m_YOffset + y ) | 0xff000000 ) != MaskColorKey )
      {
        cdPage.PutPixelAlpha( rcOverlap.Left + x, rcOverlap.Top + y, Color, Color >> 24 );
      }
    }
  }
}



void CDDrawRenderClass::RenderTextureSection( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture )
{
  if ( pTexture == NULL )
  {
    return;
  }

  XTextureSection& TexSection = ( ( (XMultiTexture2d*)pTexture )->m_Textures[0] );
  CDDrawTexture* pDDTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pDDTexture == NULL )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pDDTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pDDTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pDDTexture->m_cdImage;
  }

  cdImage.CopyArea( 0, 0, TexSection.m_Width, TexSection.m_Height, X, Y, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                          const XMultiTexture* pTextureA,
                                                          GR::i32 AlphaValue )
{
  XTextureSection& TexSection = ( ( (XMultiTexture2d*)pTextureA )->m_Textures[0] );
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  pTexture->m_cdImage.CopyAreaAlpha( 0, 0, TexSection.m_Width, TexSection.m_Height, X, Y, AlphaValue, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                       const XMultiTexture* pTextureA,
                                                       GR::u32 Color )
{
  XTextureSection& TexSection = ( ( (XMultiTexture2d*)pTextureA )->m_Textures[0] );
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  cdImage.CopyAreaColorized( 0, 0, TexSection.m_Width, TexSection.m_Height, X, Y, Color, &cdPage );
}



void CDDrawRenderClass::RenderTextureSection( GR::i32 X, GR::i32 Y, 
                                              const XTextureSection& TexSection )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }


  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  cdImage.CopyArea( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y, 
                                                            const XTextureSection& TexSection, const GR::u32 Color )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  if ( pTexture->m_cdAlphaLayer.ImageFormat() == GR::Graphic::IF_UNKNOWN )
  {
    // TODO - doppelte RLE-Liste?
    // kein Alpha dran wie wir es benötigen
    pTexture->m_cdAlphaLayer.CreateData( pTexture->m_cdImage.Width(), pTexture->m_cdImage.Height(), GR::Graphic::IF_A8 );
    pTexture->m_cdAlphaLayer.SetPalette( GR::Graphic::Palette::AlphaPalette() );

    // Alpha kopieren
    if ( pTexture->m_cdImage.ImageFormat() == GR::Graphic::IF_A8 )
    {
      for ( int i = 0; i < pTexture->m_cdImage.Width(); ++i )
      {
        for ( int j = 0; j < pTexture->m_cdImage.Height(); ++j )
        {
          GR::u32 Pixel = pTexture->m_cdImage.GetDirectPixel( i, j );

          pTexture->m_cdAlphaLayer.PutPixelFast( i, j, Pixel );
        }
      }
    }
    else
    {
      for ( int i = 0; i < pTexture->m_cdImage.Width(); ++i )
      {
        for ( int j = 0; j < pTexture->m_cdImage.Height(); ++j )
        {
          GR::u32 Pixel = pTexture->m_cdImage.GetPixel( i, j );

          pTexture->m_cdAlphaLayer.PutPixelFast( i, j, Pixel );
        }
      }
    }
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  cdImage.CopyAreaAlphaAndMaskColorized( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, 
                                         X, Y, 255, Color, &pTexture->m_cdAlphaLayer, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                         const XTextureSection& TexSection, GR::u32 Color,
                                                                         float Angle, float ZoomX, float ZoomY )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  if ( pTexture->m_cdAlphaLayer.ImageFormat() == GR::Graphic::IF_UNKNOWN )
  {
    // TODO - doppelte RLE-Liste?
    // kein Alpha dran wie wir es benötigen
    pTexture->m_cdAlphaLayer.CreateData( pTexture->m_cdImage.Width(), pTexture->m_cdImage.Height(), GR::Graphic::IF_A8 );
    pTexture->m_cdAlphaLayer.SetPalette( GR::Graphic::Palette::AlphaPalette() );

    // Alpha kopieren
    if ( pTexture->m_cdImage.ImageFormat() == GR::Graphic::IF_A8 )
    {
      for ( int i = 0; i < pTexture->m_cdImage.Width(); ++i )
      {
        for ( int j = 0; j < pTexture->m_cdImage.Height(); ++j )
        {
          GR::u32 Pixel = pTexture->m_cdImage.GetDirectPixel( i, j );

          pTexture->m_cdAlphaLayer.PutPixelFast( i, j, Pixel );
        }
      }
    }
    else
    {
      for ( int i = 0; i < pTexture->m_cdImage.Width(); ++i )
      {
        for ( int j = 0; j < pTexture->m_cdImage.Height(); ++j )
        {
          GR::u32 Pixel = pTexture->m_cdImage.GetPixel( i, j );

          pTexture->m_cdAlphaLayer.PutPixelFast( i, j, Pixel );
        }
      }
    }
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdTemp;

  cdTemp.Attach( TexSection.m_Width, 
                 TexSection.m_Height, 
                 pTexture->m_cdImage.LineOffsetInBytes(),
                 pTexture->m_cdImage.ImageFormat(),
                 pTexture->m_cdImage.GetRowColumnData( TexSection.m_XOffset, TexSection.m_YOffset ) );

  cdTemp.CopyAreaAlphaAndMaskRotatedColorized( X, Y, Angle, ZoomX, ZoomY, 255, Color, &pTexture->m_cdAlphaLayer, &cdPage );
}



void CDDrawRenderClass::AdjustPosBySectionFlags( int& X, int& Y, const XTextureSection& Section )
{
  if ( Section.m_Flags & XTextureSection::TSF_HCENTER )
  {
    X -= Section.m_Width / 2;
  }
  if ( Section.m_Flags & XTextureSection::TSF_VCENTER )
  {
    Y -= Section.m_Height / 2;
  }
  if ( Section.m_Flags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= Section.m_Height - 1;
  }
  if ( Section.m_Flags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= Section.m_Width - 1;
  }
}



void CDDrawRenderClass::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                       const XTextureSection& TexSection,
                                                       GR::u32 Color )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  cdImage.CopyAreaColorized( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, Color, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y, 
                                                        const XTextureSection& TexSection,
                                                        GR::u32 ColorKey,
                                                        bool bDoNotCreateNewIfNeeded )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  GR::Graphic::eImageFormat  ifCurFormat = cdImage.ImageFormat();
  GR::Graphic::eImageFormat  ifPageFormat = cdPage.ImageFormat();

  if ( !IsBitCompatible( ifCurFormat, ifPageFormat ) )
  {
    /*
    dh::Log( "CDDrawRenderClass::RenderTextureSectionColorKeyed ImageFormat Mismatch %d != %d",
             cdImage.ImageFormat(), cdPage.ImageFormat() );
             */

    ConvertTextureFormat( pTexture, cdPage.ImageFormat() );

    cdImage = pTexture->m_cdImage;
  }

  tColorKeyEntry    Entry;

  Entry.ColorKey  = ColorKey;
  Entry.Section     = TexSection;

  tRLELists::iterator    it( m_RLELists.find( Entry ) );
  if ( it != m_RLELists.end() )
  {
    cdImage.CopyAreaCompressed( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, it->second, NULL, &cdPage );
  }
  else
  {
    GR::Graphic::RLEList      newList;

    newList.Compress( cdImage, 
                      TexSection.m_XOffset, 
                      TexSection.m_YOffset,
                      TexSection.m_Width, 
                      TexSection.m_Height,
                      ColorKey, 
                      true );

    //dh::Log( "CDDrawRenderClass::RenderTextureSectionColorKeyed ImageFormat Created new RLE list CK %x", ColorKey );

    m_RLELists[Entry] = newList;

    cdImage.CopyAreaCompressed( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, newList, NULL, &cdPage );
  }
}



void CDDrawRenderClass::RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                        const XTextureSection& TexSection,
                                                        GR::u32 ColorKey, GR::u32 Color,
                                                        bool DoNotCreateNewIfNeeded )
{
  if ( Color == 0xffffffff )
  {
    RenderTextureSectionColorKeyed( X, Y, TexSection, ColorKey, DoNotCreateNewIfNeeded );
    return;
  }

  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    ConvertTextureFormat( pTexture, cdPage.ImageFormat() );

    cdImage = pTexture->m_cdImage;
  }

  tColorKeyEntry    Entry;

  Entry.ColorKey  = ColorKey;
  Entry.Section     = TexSection;

  tRLELists::iterator    it( m_RLELists.find( Entry ) );
  if ( it != m_RLELists.end() )
  {
    // TODO - Colorize
    cdImage.CopyAreaCompressedColorize( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, it->second, Color, &cdPage );
  }
  else
  {
    GR::Graphic::RLEList      newList;

    newList.Compress( cdImage, 
                      TexSection.m_XOffset, TexSection.m_YOffset,
                      TexSection.m_Width, 
                      TexSection.m_Height,
                      ColorKey, 
                      true );

    m_RLELists[Entry] = newList;

    // TODO - Colorize
    cdImage.CopyAreaCompressedColorize( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, newList, Color, &cdPage );
  }
}



void CDDrawRenderClass::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                    const XTextureSection& TexSection,
                                                    GR::i32 AlphaValue )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  pTexture->m_cdImage.CopyAreaAlpha( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, AlphaValue, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                    const XTextureSection& TexSection,
                                                                    GR::i32 AlphaValue,
                                                                    GR::u32 ColorKey )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  pTexture->m_cdImage.CopyAreaAlphaTransparent( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, AlphaValue, ColorKey, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  pTexture->m_cdImage.MirrorH( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  pTexture->m_cdImage.MirrorV( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                      const XTextureSection& TexSection,
                                                      float Angle, float ZoomX, float ZoomY )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  GR::Graphic::ContextDescriptor    cdTemp;

  cdTemp.Attach( TexSection.m_Width, 
                 TexSection.m_Height, 
                 pTexture->m_cdImage.LineOffsetInBytes(),
                 pTexture->m_cdImage.ImageFormat(),
                 pTexture->m_cdImage.GetRowColumnData( TexSection.m_XOffset, TexSection.m_YOffset ) );

  cdTemp.CopyRotated( X, Y, Angle, ZoomX, ZoomY, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                      const XTextureSection& TexSection,
                                                      float Angle, float ZoomX, float ZoomY,
                                                      GR::u32 ColorKey )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  GR::Graphic::ContextDescriptor    cdTemp;

  cdTemp.Attach( TexSection.m_Width, 
                 TexSection.m_Height, 
                 pTexture->m_cdImage.LineOffsetInBytes(),
                 pTexture->m_cdImage.ImageFormat(),
                 pTexture->m_cdImage.GetRowColumnData( TexSection.m_XOffset, TexSection.m_YOffset ) );

  cdTemp.CopyRotatedTransparent( X, Y, Angle, ZoomX, ZoomY, ColorKey, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                      const XTextureSection& TexSection,
                                                      float Angle, float ZoomX, float ZoomY,
                                                      GR::u32 ColorKey,
                                                      GR::u32 Color )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  dh::Log( "RenderTextureSectionRotatedZoomedColorKeyedColorized not fully implemented yet!" );
  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  GR::Graphic::ContextDescriptor    cdTemp;

  cdTemp.Attach( TexSection.m_Width, 
                 TexSection.m_Height, 
                 pTexture->m_cdImage.LineOffsetInBytes(),
                 pTexture->m_cdImage.ImageFormat(),
                 pTexture->m_cdImage.GetRowColumnData( TexSection.m_XOffset, TexSection.m_YOffset ) );

  cdTemp.CopyRotatedTransparent( X, Y, Angle, ZoomX, ZoomY, ColorKey, &cdPage );
}



LRESULT CALLBACK CDDrawRenderClass::DDrawWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  return g_pDDrawInstance->WndProc( hwnd, message, wParam, lParam );
}



LRESULT CALLBACK CDDrawRenderClass::WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
  {
    case WM_PAINT:
      if ( m_Windowed )
      {
        /*
        if ( m_pWinGPage != NULL )
        {
          m_pWinGPage->GFXUpdate( FALSE, FALSE, NULL, (HDC)wParam, FALSE );
        }
        return 0;*/
      }
      break;
    case WM_DESTROY:
      SetWindowLong( m_hwndViewport, GWL_WNDPROC, (LONG)(LONG_PTR)m_OldWndProc );
      break;
  }

  return CallWindowProc( m_OldWndProc, hwnd, message, wParam, lParam );
}



void CDDrawRenderClass::RenderText( X2dFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color )
{
  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  if ( pFont->FontImageFormat() != GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) )
  {
    ConvertFontFormat( pFont, GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) );
  }

  for ( size_t i = 0; i < Text.length(); i++ )
  {
    char  cLetter = Text[i];

    std::map<GR::u32,X2dFont::XLetter*>::iterator   it( pFont->m_mapLetters.find( cLetter ) );
    if ( it == pFont->m_mapLetters.end() )
    {
      continue;
    }

    X2dFont::XLetter* pLetter = it->second;

    tFontRLEKey    ckEntry;

    ckEntry.pFont      = pFont;
    ckEntry.Letter   = cLetter;

    tFontRLELists::iterator    itRLE( m_FontRLELists.find( ckEntry ) );
    if ( itRLE == m_FontRLELists.end() )
    {
      continue;
    }

    GR::Graphic::ContextDescriptor&   cdImage = ( (CDDrawTexture*)( pLetter->m_TextureSection.m_pTexture ) )->m_cdImage;

    if ( Color == 0xffffffff )
    {
      cdImage.CopyAreaCompressed( pLetter->m_TextureSection.m_XOffset, pLetter->m_TextureSection.m_YOffset, 
                                  pLetter->m_TextureSection.m_Width, pLetter->m_TextureSection.m_Height, 
                                  X, Y, 
                                  itRLE->second.rleList, 
                                  &itRLE->second.cdAlphaMask,
                                  &cdPage );
      //RenderTextureSectionColorKeyed( iX, iY, it->second->m_TextureSection, Font.m_TransparentColor );
    }
    else
    {
      cdImage.CopyAreaCompressedColorize( pLetter->m_TextureSection.m_XOffset, pLetter->m_TextureSection.m_YOffset, 
                                  pLetter->m_TextureSection.m_Width, pLetter->m_TextureSection.m_Height, 
                                  X, Y, 
                                  itRLE->second.rleList, 
                                  Color,
                                  &itRLE->second.cdAlphaMask,
                                  &cdPage );
      //RenderTextureSectionColorKeyedColorized( iX, iY, it->second->m_TextureSection, Font.m_TransparentColor, Color );
    }
    X += pLetter->m_TextureSection.m_Width + pFont->FontSpacing();
  }

}



void CDDrawRenderClass::RenderText( X2dFont* pFont, int X, int Y, const GR::String& Text, 
                                   float ScaleX, float ScaleY, GR::u32 Color )
{
  if ( pFont->FontImageFormat() != GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) )
  {
    ConvertFontFormat( pFont, GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) );
  }
  pFont->DrawText( X, Y, Text, ScaleX, ScaleY, Color );
}



void CDDrawRenderClass::ConvertTextureFormat( XTexture* pTexture, GR::Graphic::eImageFormat Format )
{
  if ( pTexture->m_ImageFormat == Format )
  {
    return;
  }
  ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return;
  }

  if ( GR::Graphic::ImageData::FormatHasAlpha( pTexture->m_ImageFormat ) )
  {
    if ( Format == GR::Graphic::IF_X1R5G5B5 )
    {
      Format = GR::Graphic::IF_A1R5G5B5;
    }
    else if ( Format == GR::Graphic::IF_X8R8G8B8 )
    {
      Format = GR::Graphic::IF_A8R8G8B8;
    }
  }

  GR::Graphic::ImageData*   pImageData = pManager->LoadData( pTexture->m_LoadedFromFile.c_str() );
  if ( pImageData )
  {
    GR::Graphic::ContextDescriptor      cdImage;

    cdImage.Attach( pImageData->Width(), pImageData->Height(),
                    pImageData->LineOffsetInBytes(),
                    pImageData->ImageFormat(),
                    pImageData->Data() );

    cdImage.ConvertSelfTo( Format );

    //dh::Log( "Texture Converted from %d to %d", CImageData::ImageFormatFromDepth( ( (CDDrawTexture*)pTexture )->m_Image.GetDepth() ), cdImage.ImageFormat() );

    ( (CDDrawTexture*)pTexture )->m_cdImage = cdImage;
    delete pImageData;
  }
  else
  {
    // manuelles konvertieren
    GR::Graphic::ContextDescriptor&      cdImage = ( (CDDrawTexture*)pTexture )->m_cdImage;

    cdImage.ConvertSelfTo( Format );

    pTexture->m_ImageFormat = Format;
    //dh::Log( "Texture manually Converted from %d to %d", CImageData::ImageFormatFromDepth( ( (CDDrawTexture*)pTexture )->m_Image.GetDepth() ), cdImage.ImageFormat() );
  }
}



void CDDrawRenderClass::Offset( const GR::tPoint& NewOffset )
{
  XBasic2dRenderer::Offset( NewOffset );

  CurrentPage()->SetOffset( NewOffset.x, NewOffset.y );
}



X2dFont* CDDrawRenderClass::LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  X2dFont*    pFont = XBasic2dRenderer::LoadFont( FileName, Flags, TransparentColor );

  PreCacheFont( pFont, TransparentColor );

  ConvertFontFormat( pFont, GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) );

  return pFont;
}



X2dFont* CDDrawRenderClass::LoadFontSquare( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  X2dFont*   pFont = new X2dFont( this, m_pEnvironment );

  pFont->LoadFontSquare( FileName, Flags, TransparentColor );

  AddFont( pFont );

  PreCacheFont( pFont, TransparentColor );

  ConvertFontFormat( pFont, GR::Graphic::ImageData::ImageFormatFromDepth( CurrentPage()->GetDepth() ) );

  return pFont;
}



void CDDrawRenderClass::PreCacheFont( X2dFont* pFont, GR::u32 TransparentColor )
{
  std::map<GR::u32,X2dFont::XLetter*>::iterator    it( pFont->m_mapLetters.begin() );
  while ( it != pFont->m_mapLetters.end() )
  {

    X2dFont::XLetter*   pLetter = it->second;

    CDDrawTexture*    pTex = (CDDrawTexture*)pLetter->m_TextureSection.m_pTexture;

    GR::u32   ColorKey = TransparentColor;
    if ( GR::Graphic::ImageData::FormatHasAlpha( pTex->m_cdImage.ImageFormat() ) )
    {
      // es ist davon auszugehen, dass der Font beim Einladen schon veralphatisiert worden ist, Alpha-Bits stimmen nicht mehr!
      ColorKey &= 0x00ffffff;
    }

    GR::Graphic::RLEList    rleList;

    tFontRLEKey    ColorKeyEntry;

    ColorKeyEntry.pFont      = pFont;
    ColorKeyEntry.Letter   = it->first;

    tRLEEntry&    Entry( m_FontRLELists[ColorKeyEntry] );

    if ( pTex->m_cdImage.ImageFormat() == GR::Graphic::IF_A8 )
    {
      Entry.rleList.CompressMask( pTex->m_cdImage, 
                            pLetter->m_TextureSection.m_XOffset,
                            pLetter->m_TextureSection.m_YOffset,
                            pLetter->m_TextureSection.m_Width,
                            pLetter->m_TextureSection.m_Height,
                            true );

      Entry.cdAlphaMask.CreateData( pLetter->m_TextureSection.m_Width, pLetter->m_TextureSection.m_Height, GR::Graphic::IF_A8 );

      for ( int j = 0; j < Entry.cdAlphaMask.Height(); ++j )
      {
        for ( int i = 0; i < Entry.cdAlphaMask.Width(); ++i )
        {
          Entry.cdAlphaMask.PutDirectPixel( i, j, pTex->m_cdImage.GetDirectPixel( pLetter->m_TextureSection.m_XOffset + i, 
                                                                                  pLetter->m_TextureSection.m_YOffset + j ) );
        }
      }
    }
    else if ( pTex->m_cdImage.ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
    {
      // TODO - Alphamaske aufbewahren!!!!
      Entry.rleList.CompressMaskA1( pTex->m_cdImage, 
                              pLetter->m_TextureSection.m_XOffset,
                              pLetter->m_TextureSection.m_YOffset,
                              pLetter->m_TextureSection.m_Width,
                              pLetter->m_TextureSection.m_Height,
                              true );

      Entry.cdAlphaMask.CreateData( pLetter->m_TextureSection.m_Width, pLetter->m_TextureSection.m_Height, GR::Graphic::IF_A8 );

      for ( int j = 0; j < Entry.cdAlphaMask.Height(); ++j )
      {
        for ( int i = 0; i < Entry.cdAlphaMask.Width(); ++i )
        {
          if ( pTex->m_cdImage.GetDirectPixel( pLetter->m_TextureSection.m_XOffset + i, 
                                               pLetter->m_TextureSection.m_YOffset + j ) & 0x8000 )
          {
            Entry.cdAlphaMask.PutDirectPixel( i, j, 255 );
          }
        }
      }
    }
    else
    {
      Entry.rleList.Compress( pTex->m_cdImage, 
                        pLetter->m_TextureSection.m_XOffset,
                        pLetter->m_TextureSection.m_YOffset,
                        pLetter->m_TextureSection.m_Width,
                        pLetter->m_TextureSection.m_Height,
                        TransparentColor, true );
    }


    ++it;
  }
}



void CDDrawRenderClass::DestroyFont( X2dFont* pFont )
{
  tFontRLELists::iterator    it( m_FontRLELists.begin() );
  while ( it != m_FontRLELists.end() )
  {
    const tFontRLEKey&    ColorKeyEntry = it->first;

    if ( ColorKeyEntry.pFont == pFont )
    {
      it = m_FontRLELists.erase( it );
      continue;
    }

    ++it;
  }

  XBasic2dRenderer::DestroyFont( pFont );
}



void CDDrawRenderClass::DestroyAllFonts()
{
  XBasic2dRenderer::DestroyAllFonts();

  m_FontRLELists.clear();
}



void CDDrawRenderClass::RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  cdImage.CopyAreaAdditive( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, &cdPage );
}



void CDDrawRenderClass::RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                                const XTextureSection& TexSection, GR::u32 Color )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }


  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  cdImage.CopyAreaAdditiveTransparentColorized( TexSection.m_XOffset, TexSection.m_YOffset, TexSection.m_Width, TexSection.m_Height, X, Y, ColorKey, &cdPage, Color );
}



void CDDrawRenderClass::RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                              float Angle, float ZoomX, float ZoomY,
                                                              const GR::u32 ColorKey,
                                                              const XTextureSection& TexSection, GR::u32 Color )
{
  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }

  AdjustPosBySectionFlags( X, Y, TexSection );

  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );

  GR::Graphic::ContextDescriptor    cdTemp;

  cdTemp.Attach( TexSection.m_Width, 
                 TexSection.m_Height, 
                 pTexture->m_cdImage.LineOffsetInBytes(),
                 pTexture->m_cdImage.ImageFormat(),
                 pTexture->m_cdImage.GetRowColumnData( TexSection.m_XOffset, TexSection.m_YOffset ) );

  if ( Color == 0xffffffff )
  {
    cdTemp.CopyRotatedAdditive( X, Y, Angle, ZoomX, ZoomY, ColorKey, &cdPage );
  }
  else
  {
    cdTemp.CopyRotatedAdditiveColorized( X, Y, Angle, ZoomX, ZoomY, ColorKey, Color, &cdPage );
  }
}



bool CDDrawRenderClass::ConvertFontFormat( X2dFont* pFont, GR::Graphic::eImageFormat Format )
{
  if ( pFont->m_mapLetters.empty() )
  {
    return false;
  }

  std::map<XTexture*,XTexture*>   convertedTextures;


  std::map<GR::u32,X2dFont::XLetter*>::iterator   itLetters( pFont->m_mapLetters.begin() );
  while ( itLetters != pFont->m_mapLetters.end() )
  {
    X2dFont::XLetter*    pLetter = itLetters->second;

    std::map<XTexture*,XTexture*>::iterator   itTexture( convertedTextures.find( pLetter->m_TextureSection.m_pTexture ) );
    if ( itTexture == convertedTextures.end() )
    {
      XTexture*   pOldTexture = pLetter->m_TextureSection.m_pTexture;

      if ( pOldTexture->m_LoadedFromFile.empty() )
      {
        // Manuell erzeugte Textur, direkt konvertieren

        GR::Graphic::ImageData      imgData;

        imgData.CreateData( pOldTexture->m_SurfaceSize.x, pOldTexture->m_SurfaceSize.y, pOldTexture->m_ImageFormat );

        CopyTextureToData( pOldTexture, imgData );

        imgData.ConvertSelfTo( Format );

        XTexture*   pNewTexture = CreateTexture( pOldTexture->m_SurfaceSize.x, pOldTexture->m_SurfaceSize.y, Format );
        if ( pNewTexture )
        {
          CopyDataToTexture( pNewTexture, imgData, pFont->m_TransparentColor );
          
          pLetter->m_TextureSection.m_pTexture = pNewTexture;
          convertedTextures[pOldTexture] = pNewTexture;
        }
      }
      else
      {
        ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
        if ( pManager == NULL )
        {
          return false;
        }
        GR::Graphic::ImageData*   pImageData = pManager->LoadData( pOldTexture->m_LoadedFromFile.c_str() );
        if ( pImageData )
        {
          //dh::Log( "Font::Convertformat loaded format %d", pImageData->ImageFormat() );
          GR::Graphic::ContextDescriptor      cdImage;

          GR::Graphic::ContextDescriptor*     pCDMask = NULL;

          cdImage.Attach( pImageData->Width(), pImageData->Height(),
                          pImageData->LineOffsetInBytes(),
                          pImageData->ImageFormat(),
                          pImageData->Data() );

          /*
          if ( cdImage.ImageFormat() == IF_X8R8G8B8 )
          {
            // Spezialwurst - Alpha drin
            pCDMask = new GR::Graphic::ContextDescriptor();

            pCDMask->CreateData( pImageData->Width(), pImageData->Height(), IF_A8 );

            for ( int j = 0; j < pImageData->Height(); ++j )
            {
              for ( int i = 0; i < pImageData->Width(); ++i )
              {
                pCDMask->PutDirectPixel( i, j, pImageData->GetPixel( i, j ) >> 24 );
              }
            }

            tFontRLEKey   Key;

            Key.Letter = itLetters->first;
            Key.pFont    = pFont;

            m_mapFontRLELists[Key].rleList.CompressMask( *pCDMask, 0, 0, pCDMask->Width(), pCDMask->Height(), true );
          }
          */

          cdImage.ConvertSelfTo( Format );

          //dh::Log( "Font::Convertformat converted to format %d", Format );

          // neue Textur erzeugen
          XTexture*   pNewTexture = CreateTexture( pOldTexture->m_SurfaceSize.x, pOldTexture->m_SurfaceSize.y, Format );
          if ( pNewTexture )
          {
            pNewTexture->m_LoadedFromFile = pOldTexture->m_LoadedFromFile;

            CopyDataToTexture( pNewTexture, cdImage );
            
            pLetter->m_TextureSection.m_pTexture = pNewTexture;
            convertedTextures[pOldTexture] = pNewTexture;
          }
          delete pCDMask;
          delete pImageData;
        }
        else
        {
          dh::Log( "Convert: ImageData loading %s failed", pOldTexture->m_LoadedFromFile.c_str() );
        }
      }
    }
    else
    {
      pLetter->m_TextureSection.m_pTexture = itTexture->second;
    }

    ++itLetters;
  }

  // TODO - nicht sauber, Referenzzähler!!!
  std::map<XTexture*,XTexture*>::iterator   it( convertedTextures.begin() );
  while ( it != convertedTextures.end() )
  {
    DestroyTexture( it->first );

    ++it;
  }

  return true;
}



void CDDrawRenderClass::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu entladen
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION ); ++i )
      {
        delete pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );
        pLoader->SetAsset( Xtreme::Asset::XA_IMAGE_SECTION, i, NULL );
      }
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE ); ++i )
      {
        delete pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );
        pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, NULL );
      }
      for ( GR::up i = 0; i < pLoader->AssetTypeCount( Xtreme::Asset::XA_FONT ); ++i )
      {
        delete pLoader->Asset( Xtreme::Asset::XA_FONT, i );
        pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, NULL );
      }
      pLoader->NotifyService( "GUI", "AssetsUnloaded" );
    }
  }
}



bool CDDrawRenderClass::IsBitCompatible( const GR::Graphic::eImageFormat Format1, const GR::Graphic::eImageFormat Format2 )
{
  if ( Format1 == Format2 )
  {
    return true;
  }
  if ( ( ( Format1 == GR::Graphic::IF_X1R5G5B5 )
  &&     ( Format2 == GR::Graphic::IF_A1R5G5B5 ) )
  ||   ( ( Format1 == GR::Graphic::IF_X8R8G8B8 )
  &&     ( Format2 == GR::Graphic::IF_A8R8G8B8 ) )
  ||   ( ( Format1 == GR::Graphic::IF_A8R8G8B8 )
  &&     ( Format2 == GR::Graphic::IF_X8R8G8B8 ) )
  ||   ( ( Format1 == GR::Graphic::IF_A1R5G5B5 )
  &&     ( Format2 == GR::Graphic::IF_X1R5G5B5 ) ) )
  {
    return true;
  }
  return false;
}



void CDDrawRenderClass::RenderTextureSectionMasked( GR::i32 X, GR::i32 Y, 
                                                    const XTextureSection& TexSection,
                                                    GR::i32 MaskX, GR::i32 MaskY, 
                                                    const XTextureSection& TexSectionMask, 
                                                    const GR::u32 Color,
                                                    const GR::u32 MaskColorKey )
{

  // überlappendes Rect finden
  GR::tRect     rcSection( X, Y, TexSection.m_Width, TexSection.m_Height );
  GR::tRect     rcMask( MaskX, MaskY, TexSectionMask.m_Width, TexSectionMask.m_Height );

  GR::tRect     rcOverlap( rcSection.intersection( rcMask ) );

  if ( ( rcOverlap.width() == 0 )
  ||   ( rcOverlap.height() == 0 ) )
  {
    return;
  }

  CDDrawTexture* pTexture = (CDDrawTexture*)TexSection.m_pTexture;
  if ( pTexture == NULL )
  {
    return;
  }
  CDDrawTexture* pTextureMask = (CDDrawTexture*)TexSectionMask.m_pTexture;
  if ( pTextureMask == NULL )
  {
    return;
  }


  GR::Graphic::ContextDescriptor    cdPage( CurrentPage() );
  GR::Graphic::ContextDescriptor    cdImage( pTexture->m_cdImage );
  GR::Graphic::ContextDescriptor    cdImageMask( pTextureMask->m_cdImage );

  if ( cdImage.ImageFormat() != cdPage.ImageFormat() )
  {
    pTexture->m_cdImage.ConvertSelfTo( cdPage.ImageFormat() );
    
    cdImage = pTexture->m_cdImage;
  }

  XTextureSection     TS1( TexSection );
  XTextureSection     TS2( TexSectionMask );

  TS1.m_XOffset += rcOverlap.Left - X;
  TS1.m_YOffset += rcOverlap.Top - Y;

  TS2.m_XOffset += rcOverlap.Left - MaskX;
  TS2.m_YOffset += rcOverlap.Top - MaskY;

  TS1.m_Width = rcOverlap.width();
  TS1.m_Height = rcOverlap.height();
  TS2.m_Width = rcOverlap.width();
  TS2.m_Height = rcOverlap.height();

  for ( int iY = 0; iY < TS1.m_Height; ++iY )
  {
    for ( int iX = 0; iX < TS1.m_Width; ++iX )
    {
      GR::u32       Pixel = GFX::Modulate( 0xff000000 | cdImage.GetPixel( TS1.m_XOffset + iX, TS1.m_YOffset + iY ), Color );
      GR::u32       PixelMask = GFX::Modulate( 0xff000000 | cdImageMask.GetPixel( TS2.m_XOffset + iX, TS2.m_YOffset + iY ), Color );

      GR::u32       ResPixel = GFX::Modulate( Pixel, PixelMask );

      cdPage.PutPixelAlpha( rcOverlap.Left + iX, rcOverlap.Top + iY, ResPixel, ResPixel >> 24 );
    }
  }
}