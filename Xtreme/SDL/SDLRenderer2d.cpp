#include <debug/debugclient.h>

#include <Grafik/ContextDescriptor.h>
#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont2d.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/Environment/XWindowSDL.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <String/XML.h>

#include <Misc/Misc.h>

#include <IO/FileStream.h>

#include "SDLRenderer2d.h"
#include "SDLTexture.h"
#include "SDLVertexBuffer.h"



SDLRenderer2d::SDLRenderer2d() :
  m_IsReady( false ),
  m_Windowed( true ),
  m_VSyncEnabled( false ),
  m_pRenderer( NULL ),
  m_Width( 0 ),
  m_Height( 0 )
{
}



SDLRenderer2d::~SDLRenderer2d()
{
  Release();
}



bool SDLRenderer2d::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment )
{
  m_pEnvironment  = &Environment;
  m_pDebugger     = (IDebugService*)m_pEnvironment->Service( "Logger" );
  m_Width         = Width;
  m_Height        = Height;


  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );

  SDL_Window* pWindow = NULL;
  if ( pWindowService != NULL )
  {
    pWindow = (SDL_Window*)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
    return false;
  }

  m_pRenderer = SDL_CreateRenderer( pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
  if ( m_pRenderer == NULL )
  {
    m_pRenderer = SDL_CreateRenderer( pWindow, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC );
    if ( m_pRenderer == NULL )
    {
      dh::Log( "SDL_CreateRenderer failed" );
      return false;
    }
  }
  m_IsReady = true;

  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu laden
      GR::up    imageCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE );
      for ( GR::up i = 0; i < imageCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );

        GR::u32         dwColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );
        GR::u32         colorKeyReplacementColor = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKeyReplacementColor" ), 16 );

        GR::u32         dwForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        GR::String      fileName = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" );

        std::list<GR::String>    listImages;

        GR::u32     dwMipMaps = 0;

        while ( true )
        {
          if ( dwMipMaps == 0 )
          {
            listImages.push_back( AppPath( fileName ) );
          }
          else
          {
            GR::String   strPath = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", dwMipMaps ) );

            if ( strPath.empty() )
            {
              break;
            }
            listImages.push_back( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", dwMipMaps ) ) ) );
          }
          dwMipMaps++;
        }

        if ( listImages.empty() )
        {
          Log( "Renderer.General", "SDLRenderer2d: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front(), (GR::Graphic::eImageFormat)dwForcedFormat, dwColorKey, colorKeyReplacementColor );

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
            GR::u32   dwLevel = 1;

            while ( it != listImages.end() )
            {
              GR::String&  strPath( *it );

              GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)dwForcedFormat, dwColorKey, colorKeyReplacementColor );
              if ( pData == NULL )
              {
                Log( "Renderer.General", CMisc::printf( "SDLRenderer2d:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, dwColorKey );
                delete pData;
              }

              ( (SDLTexture*)pTexture )->m_FileNames.push_back( strPath );

              ++dwLevel;
              ++it;
            }
          }
          else
          {
            Log( "Renderer.General", CMisc::printf( "SDLRenderer2d: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
          }
        }
      }

      GR::up    imageSectionCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < imageSectionCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          Log( "Renderer.General", CMisc::printf( "SDLRenderer2d: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
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
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_FONT, i );

        GR::u32     dwColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     dwLoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        X2dFont*      pFont = NULL;

        if ( ( dwLoadFlags & X2dFont::FLF_SQUARED )
        ||   ( dwLoadFlags & X2dFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
                                  dwLoadFlags,
                                  dwColorKey );
        }
        else
        {
          pFont = LoadFont( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), dwColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "SDLRenderer2d: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont2d( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          Log( "Renderer.General", "SDLRenderer2d: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }

      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  NotifyService( "Renderer", "Initialised" );
  return true;
}



bool SDLRenderer2d::Release()
{
  ReleaseAssets();

  m_IsReady = false;
  if ( m_pRenderer != NULL )
  {
    SDL_DestroyRenderer( m_pRenderer );
    m_pRenderer = NULL;
  }

  NotifyService( "Renderer", "Released" );
  return true;
}



bool SDLRenderer2d::IsReady() const
{
  return m_IsReady;
}



bool SDLRenderer2d::BeginScene()
{
  return true;
}



void SDLRenderer2d::EndScene()
{
}



void SDLRenderer2d::VirtualSize( const GR::tPoint& NewSize )
{
  XBasic2dRenderer::VirtualSize( NewSize );

  SDL_RenderSetLogicalSize( m_pRenderer, NewSize.x, NewSize.y );
}



void SDLRenderer2d::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  SDL_RenderPresent( m_pRenderer );

  /*
  if ( !m_strSaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_strSaveScreenShotFileName.c_str() );
    m_strSaveScreenShotFileName.clear();
  }*/
}



XTexture* SDLRenderer2d::LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 dwColorKey, GR::u32 ColorKeyReplacementColor )
{
  GR::Graphic::ImageData*   pData = LoadAndConvert( FileName, imgFormatToConvert, dwColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    dh::Log( "-fail" );
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData );
  if ( pTexture == NULL )
  {
    dh::Log( "-fail CreateTexture" );
    delete pData;
    return NULL;
  }

  pTexture->m_ColorKey        = dwColorKey;
  pTexture->m_LoadedFromFile  = FileName;

  delete pData;
  return pTexture;
}



XTexture* SDLRenderer2d::CreateTexture( const GR::Graphic::ImageData& ImageData )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat() );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



int SDLRenderer2d::MapTextureFormat( GR::Graphic::eImageFormat Format )
{
  switch ( Format )
  {
    case GR::Graphic::IF_A1R5G5B5:
      return SDL_PIXELFORMAT_ARGB1555;
    case GR::Graphic::IF_A2B10G10R10:
      break;
    case GR::Graphic::IF_A2R10G10B10:
      return SDL_PIXELFORMAT_ARGB2101010;
    case GR::Graphic::IF_A4R4G4B4:
      return SDL_PIXELFORMAT_ARGB4444;
    case GR::Graphic::IF_A8:
      return SDL_PIXELFORMAT_ARGB8888;
    case GR::Graphic::IF_A8R8G8B8:
      return SDL_PIXELFORMAT_ARGB8888;
    case GR::Graphic::IF_INDEX1:
      return SDL_PIXELFORMAT_INDEX1MSB;
    case GR::Graphic::IF_INDEX2:
      break;
    case GR::Graphic::IF_INDEX4:
      return SDL_PIXELFORMAT_INDEX4MSB;
    case GR::Graphic::IF_INDEX8:
      return SDL_PIXELFORMAT_INDEX8;
    case GR::Graphic::IF_R5G6B5:
      return SDL_PIXELFORMAT_RGB565;
    case GR::Graphic::IF_X1R5G5B5:
      return SDL_PIXELFORMAT_RGB555;
    case GR::Graphic::IF_X8R8G8B8:
      {
        // why is there no SDL_PIXELFORMAT_XRGB8888 ???
        return SDL_DEFINE_PIXELFORMAT( SDL_PIXELTYPE_PACKED32, SDL_PACKEDORDER_XRGB,
                                       SDL_PACKEDLAYOUT_8888, 32, 4 );
      }
    case GR::Graphic::IF_R8G8B8:
      return SDL_PIXELFORMAT_BGR24;
    default:
      // shut up stupid warnings about unused switch values
      break;
  }

  dh::Log( "MapTextureFormat: Format %d not supported", Format );
  return SDL_PIXELFORMAT_RGBA8888;
}



XTexture* SDLRenderer2d::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat Format )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    return NULL;
  }

  if ( !IsTextureFormatOK( Format ) )
  {
    return NULL;
  }


  SDLTexture*   pTexture = new SDLTexture();

  // TODO - Create
  pTexture->m_pSDLTexture = SDL_CreateTexture( m_pRenderer, MapTextureFormat( Format ), SDL_TEXTUREACCESS_STATIC, Width, Height );
  if ( pTexture->m_pSDLTexture != NULL )
  {
    pTexture->m_ImageFormat = Format;
    pTexture->m_ImageSourceSize.set( Width, Height );
  }

  AddTexture( pTexture );

  return pTexture;

}



bool SDLRenderer2d::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat )
{
  if ( imgFormat == GR::Graphic::IF_A8 )
  {
    // we could, but seriously
    return false;
  }
  return true;
}



bool SDLRenderer2d::OnResized()
{
  //return ResizeSurface();
  return true;
}



GR::u32 SDLRenderer2d::Width()
{
  if ( m_pRenderer == NULL )
  {
    return 0;
  }
  return m_Width;
}



GR::u32 SDLRenderer2d::Height()
{
  if ( m_pRenderer == NULL )
  {
    return 0;
  }
  return m_Height;
}



GR::Graphic::eImageFormat SDLRenderer2d::ImageFormat()
{
  //return MapFormat( m_d3dpp.BackBufferFormat );
  return GR::Graphic::IF_A8R8G8B8;
}



bool SDLRenderer2d::IsFullscreen()
{
  return !m_Windowed;
}



bool SDLRenderer2d::ToggleFullscreen()
{
  m_Windowed = !m_Windowed;

  //return SUCCEEDED( Resize3DEnvironment() );
  return true;
}



bool SDLRenderer2d::SetMode( XRendererDisplayMode& DisplayMode )
{
  /*
  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&   DispMode = *it;
    if ( DispMode.FormatIsEqualTo( DisplayMode ) )
    {
      // der Mode existiert
      m_Windowed = !DisplayMode.FullScreen;
      if ( !m_Windowed )
      {
        //m_FullscreenDisplayMode = DisplayMode;
      }

      return false;
      //return SUCCEEDED( Resize3DEnvironment() );
    }

    ++it;
  }
  dh::Log( "SDLRenderer2d::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.ImageFormat );*/
  return false;
}



bool SDLRenderer2d::SaveScreenShot( const GR::String& FileName )
{
  if ( !FileName.empty() )
  {
    //m_strSaveScreenShotFileName = szFileName;
    return false;
  }
  return false;
}



void SDLRenderer2d::UnloadAssets( Xtreme::Asset::eXAssetType Type )
{
  Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader )
  {
    GR::up    assetCount = pLoader->AssetTypeCount( Type );
    for ( GR::up i = 0; i < assetCount; ++i )
    {
      Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Type, i );
      delete pAsset;
      pLoader->SetAsset( Type, i, NULL );
    }
  }
}



void SDLRenderer2d::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
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



bool SDLRenderer2d::VSyncEnabled()
{
  return m_VSyncEnabled;
}



void SDLRenderer2d::EnableVSync( bool Enable )
{
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
    //Resize3DEnvironment();
  }
}



void SDLRenderer2d::AdjustWindowForMode( bool Windowed )
{
}



bool SDLRenderer2d::SetViewport( XViewport& Viewport )
{
  m_ViewPort = Viewport;

  SDL_Rect    rc;

  rc.x = m_ViewPort.X;
  rc.y = m_ViewPort.Y;
  rc.w = m_ViewPort.Width;
  rc.h = m_ViewPort.Height;

  SDL_RenderSetViewport( m_pRenderer, &rc );
  return true;
}



GR::u32 SDLRenderer2d::Depth()
{
  // TODO
  return 32;
}



bool SDLRenderer2d::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  SDLTexture* pTex = (SDLTexture*)pTexture;

  SDL_Rect    rc;

  rc.x = 0;
  rc.y = 0;
  rc.w = ImageData.Width();
  rc.h = ImageData.Height();

  return ( SDL_UpdateTexture( pTex->m_pSDLTexture, &rc, ImageData.Data(), ImageData.BytesPerLine() ) == 0 );
}



bool SDLRenderer2d::CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData )
{
  dh::Log( "CopyTextureToData not supported" );
  return false;
}



XMultiTexture* SDLRenderer2d::LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey )
{
  dh::Log( "LoadMultiTexture not supported" );
  return NULL;
}



void SDLRenderer2d::RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height, GR::u32 Color )
{
  SDL_SetRenderDrawColor( m_pRenderer, 
                          ( Color & 0x00ff0000 ) >> 16,
                          ( Color & 0x0000ff00 ) >>  8,
                          ( Color & 0x000000ff ) >>  0,
                          ( Color & 0xff000000 ) >> 24 );
  SDL_Rect  rc;
  rc.x = X;
  rc.y = Y;
  rc.w = Width;
  rc.h = Height;
  SDL_RenderFillRect( m_pRenderer, &rc );
}



void SDLRenderer2d::RenderLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color )
{
  SDL_SetRenderDrawColor( m_pRenderer, ( Color & 0x00ff0000 ) >> 16,
                                       ( Color & 0x0000ff00 ) >> 8,
                                       ( Color & 0x000000ff ) >> 0,
                                       ( Color & 0xff000000 ) >> 24 );

  SDL_RenderDrawLine( m_pRenderer, Pos1.x, Pos1.y, Pos2.x, Pos2.y );
}



void SDLRenderer2d::RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                    GR::i32 X2, GR::i32 Y2,
                                    GR::i32 X3, GR::i32 Y3,
                                    GR::u32 Color )
{
  dh::Log( "RenderTriangle not supported" );
}



void SDLRenderer2d::RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color )
{
  SDL_Rect    rcSource;
  SDL_Rect    rcTarget;

  rcSource.x = TexSection.m_XOffset;
  rcSource.y = TexSection.m_YOffset;
  rcSource.w = TexSection.m_Width;
  rcSource.h = TexSection.m_Height;

  rcTarget.x = X;
  rcTarget.y = Y;
  rcTarget.w = TexSection.m_Width;
  rcTarget.h = TexSection.m_Height;

  GR::u32 color = Color;
  SDL_SetTextureColorMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
    ( color & 0xff0000 ) >> 16,
                          ( color & 0x00ff00 ) >> 8,
                          ( color & 0x0000ff ) >> 0 );
  SDL_SetTextureAlphaMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
    ( color & 0xff000000 ) >> 24 );

  SDL_SetTextureBlendMode( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, SDL_BLENDMODE_BLEND );

  RenderSDLSection( TexSection, rcSource, rcTarget );
}



void SDLRenderer2d::RenderTextureSection( GR::i32 X, GR::i32 Y,
                                          const XTextureSection& TexSection )
{
  SDL_Rect    rcSource;
  SDL_Rect    rcTarget;

  rcSource.x = TexSection.m_XOffset;
  rcSource.y = TexSection.m_YOffset;
  rcSource.w = TexSection.m_Width;
  rcSource.h = TexSection.m_Height;

  rcTarget.x = X;
  rcTarget.y = Y;
  rcTarget.w = TexSection.m_Width;
  rcTarget.h = TexSection.m_Height;

  SDL_SetTextureColorMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, 255, 255, 255 );
  SDL_SetTextureAlphaMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, 255 );
  SDL_SetTextureBlendMode( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, SDL_BLENDMODE_NONE );
  SDL_SetRenderDrawBlendMode( m_pRenderer, SDL_BLENDMODE_NONE );

  RenderSDLSection( TexSection, rcSource, rcTarget );
}



void SDLRenderer2d::RenderTextureSection( GR::i32 X, GR::i32 Y,
                                          const XMultiTexture* pTexture )
{
  dh::Log( "RenderTextureSection not supported" );
}



void SDLRenderer2d::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y,
                                                     const XTextureSection& TexSection,
                                                     GR::u32 Color )
{
  SDL_Rect    rcSource;
  SDL_Rect    rcTarget;

  rcSource.x = TexSection.m_XOffset;
  rcSource.y = TexSection.m_YOffset;
  rcSource.w = TexSection.m_Width;
  rcSource.h = TexSection.m_Height;

  rcTarget.x = X;
  rcTarget.y = Y;
  rcTarget.w = TexSection.m_Width;
  rcTarget.h = TexSection.m_Height;

  SDL_SetTextureColorMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, 
                            ( Color & 0xff0000 ) >> 16, 
                            ( Color & 0x00ff00 ) >>  8, 
                            ( Color & 0x0000ff ) >>  0 );
  SDL_SetTextureAlphaMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
                          ( Color & 0xff000000 ) >> 24 );
  SDL_SetTextureBlendMode( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, SDL_BLENDMODE_NONE );

  RenderSDLSection( TexSection, rcSource, rcTarget );
}




void SDLRenderer2d::RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y,
                                                          const XTextureSection& TexSection,
                                                          GR::u32 Color )
{
  dh::Log( "RenderTextureSectionWithAlphaLayer not supported" );
}




void SDLRenderer2d::RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                                       const XTextureSection& TexSection,
                                                                       GR::u32 Color,
                                                                       float Angle, float ZoomX, float ZoomY )
{
  dh::Log( "RenderTextureSectionWithAlphaLayerRotatedZoomed not supported" );
}




void SDLRenderer2d::RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y,
                                                      const XTextureSection& TexSection,
                                                      GR::u32 ColorKey,
                                                      bool DoNotCreateNewIfNeeded )
{
  SDL_Rect    rcSource;
  SDL_Rect    rcTarget;

  rcSource.x = TexSection.m_XOffset;
  rcSource.y = TexSection.m_YOffset;
  rcSource.w = TexSection.m_Width;
  rcSource.h = TexSection.m_Height;

  rcTarget.x = X;
  rcTarget.y = Y;
  rcTarget.w = TexSection.m_Width;
  rcTarget.h = TexSection.m_Height;

  GR::u32 color = 0xffffffff;
  SDL_SetTextureColorMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
                          ( color & 0xff0000 ) >> 16,
                          ( color & 0x00ff00 ) >> 8,
                          ( color & 0x0000ff ) >> 0 );
  SDL_SetTextureAlphaMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
    ( color & 0xff000000 ) >> 24 );

  SDL_SetTextureBlendMode( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, SDL_BLENDMODE_BLEND );

  RenderSDLSection( TexSection, rcSource, rcTarget );
}




void SDLRenderer2d::RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y,
                                                             const XTextureSection& TexSection,
                                                             GR::u32 ColorKey, GR::u32 Color,
                                                             bool DoNotCreateNewIfNeeded )
{
  SDL_Rect    rcSource;
  SDL_Rect    rcTarget;

  rcSource.x = TexSection.m_XOffset;
  rcSource.y = TexSection.m_YOffset;
  rcSource.w = TexSection.m_Width;
  rcSource.h = TexSection.m_Height;

  rcTarget.x = X;
  rcTarget.y = Y;
  rcTarget.w = TexSection.m_Width;
  rcTarget.h = TexSection.m_Height;

  SDL_SetTextureColorMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
                          ( Color & 0xff0000 ) >> 16,
                          ( Color & 0x00ff00 ) >> 8,
                          ( Color & 0x0000ff ) >> 0 );
  SDL_SetTextureAlphaMod( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture,
                          ( Color & 0xff000000 ) >> 24 );
  SDL_SetTextureBlendMode( ( (SDLTexture*)TexSection.m_pTexture )->m_pSDLTexture, SDL_BLENDMODE_BLEND );

  RenderSDLSection( TexSection, rcSource, rcTarget );
}



void SDLRenderer2d::RenderSDLSection( const XTextureSection& TS, SDL_Rect& Source, SDL_Rect& Target )
{
  SDL_RendererFlip    flip = SDL_FLIP_NONE;

  if ( TS.m_Flags & XTextureSection::TSF_H_MIRROR )
  {
    flip = (SDL_RendererFlip)( flip | SDL_FLIP_HORIZONTAL );
  }
  if ( TS.m_Flags & XTextureSection::TSF_V_MIRROR )
  {
    flip = (SDL_RendererFlip)( flip | SDL_FLIP_VERTICAL );
  }

  if ( TS.m_Flags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Target.y -= TS.m_Height - 1;
  }
  if ( TS.m_Flags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    Target.x -= TS.m_Width - 1;
  }
  if ( TS.m_Flags & XTextureSection::TSF_VCENTER )
  {
    Target.y -= TS.m_Height / 2;
  }
  if ( TS.m_Flags & XTextureSection::TSF_HCENTER )
  {
    Target.x -= TS.m_Width / 2;
  }
  SDL_RenderCopyEx( m_pRenderer, ( (SDLTexture*)TS.m_pTexture )->m_pSDLTexture, &Source, &Target, 0.0f, NULL, flip );
}




void SDLRenderer2d::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y,
                                                        const XTextureSection& TexSection,
                                                        GR::i32 AlphaValue )
{
  RenderTextureSection( X, Y, TexSection );
}




void SDLRenderer2d::RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y,
                                                                  const XTextureSection& TexSection,
                                                                  GR::i32 AlphaValue,
                                                                  GR::u32 ColorKey )
{
  RenderTextureSection( X, Y, TexSection );
}



void SDLRenderer2d::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y,
                                                   const XMultiTexture* pTexture,
                                                   GR::u32 Color )
{
  dh::Log( "RenderTextureSectionColorized not supported" );
}



void SDLRenderer2d::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y,
                                                      const XMultiTexture* pTexture,
                                                      GR::i32 AlphaValue )
{
  dh::Log( "RenderTextureSectionAlphaBlended not supported" );
}



void SDLRenderer2d::RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  dh::Log( "RenderTextureSectionHMirrored not supported" );
}




void SDLRenderer2d::RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  dh::Log( "RenderTextureSectionVMirrored not supported" );
}




void SDLRenderer2d::RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                     const XTextureSection& TexSection,
                                                     float Angle, float ZoomX, float ZoomY )
{
  dh::Log( "RenderTextureSectionRotatedZoomed not supported" );
}




void SDLRenderer2d::RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y,
                                                               const XTextureSection& TexSection,
                                                               float Angle, float ZoomX, float ZoomY,
                                                               GR::u32 ColorKey )
{
  dh::Log( "RenderTextureSectionRotatedZoomedColorKeyed not supported" );
}




void SDLRenderer2d::RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y,
                                                                        const XTextureSection& TexSection,
                                                                        float Angle, float ZoomX, float ZoomY,
                                                                        GR::u32 ColorKey,
                                                                        GR::u32 Color )
{
  dh::Log( "RenderTextureSectionRotatedZoomedColorKeyedColorized not supported" );
}




void SDLRenderer2d::RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  dh::Log( "RenderTextureSectionAdditive not supported" );
}




void SDLRenderer2d::RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                          const XTextureSection& TexSection, GR::u32 Color )
{
  dh::Log( "RenderTextureSectionAdditiveColorKeyed not supported" );
}




void SDLRenderer2d::RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                                       float Angle, float ZoomX, float ZoomY,
                                                                       const GR::u32 ColorKey,
                                                                       const XTextureSection& TexSection, GR::u32 Color )
{
  dh::Log( "RenderTextureSectionAdditiveColorKeyedRotatedZoomed not supported" );
}




void SDLRenderer2d::RenderTextureSectionMasked( GR::i32 X, GR::i32 Y,
                                              const XTextureSection& TexSection,
                                              GR::i32 MaskX, GR::i32 MaskY,
                                              const XTextureSection& TexSectionMask,
                                              const GR::u32 Color,
                                              const GR::u32 MaskColorKey )
{
  dh::Log( "RenderTextureSectionMasked not supported" );
}




void SDLRenderer2d::RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                    GR::i32 MaskX, GR::i32 MaskY,
                                    const XTextureSection& TexSectionMask,
                                    const GR::u32 Color,
                                    const GR::u32 MaskColorKey )
{
  dh::Log( "RenderQuadMasked not supported" );
}




