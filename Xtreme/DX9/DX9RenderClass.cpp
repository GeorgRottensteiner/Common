#include <debug/debugclient.h>

#include <Grafik/ContextDescriptor.h>
#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <String/XML.h>

#include <Xtreme/Environment/XWindow.h>

#include <Misc/Misc.h>

#include <IO/FileStream.h>

#include ".\DX9renderclass.h"
#include "DX9Texture.h"
#include "DX9VertexBuffer.h"

#include <d3DX9.h>


#pragma comment ( lib, "d3DX9.lib" )
#pragma comment ( lib, "d3d9.lib" )
#pragma comment ( lib, "dxguid.lib" )

CDX9RenderClass* CDX9RenderClass::g_pDX9Instance = NULL;


CDX9RenderClass::CDX9RenderClass( HINSTANCE hInstance ) :
  m_pD3D( NULL ),
  m_pd3dDevice( NULL ),
  m_pOldProc( NULL ),
  m_hWndFocus( NULL ),
  m_bReady( false ),
  m_bUseDepthBuffer( true ),
  m_VSyncEnabled( false ),
  m_bForceWindowSize( false ),
  m_bMultiThreadSafe( false ),
  m_MinDepthBits( 16 ),
  m_MinStencilBits( 0 ),
  m_bWindowed( true ),
  m_Adapter( 0 ),
  m_hInstance( hInstance ),
  m_strSaveScreenShotFileName( "" ),
  m_RenderWidth( 0 ),
  m_RenderHeight( 0 ),
  m_CreationWidth( 0 ),
  m_CreationHeight( 0 )
{
  if ( m_hInstance == NULL )
  {
    m_hInstance = GetModuleHandle( NULL );
  }
  m_DirectTexelMappingOffset.Set( -0.5f, -0.5f );
  g_pDX9Instance = this;
}



CDX9RenderClass::~CDX9RenderClass()
{
  Release();
  g_pDX9Instance = NULL;
}



bool CDX9RenderClass::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment )
{
  m_pEnvironment = &Environment;

  HRESULT hr;

  // Create the Direct3D object
  m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
  if ( m_pD3D == NULL )
  {
    dh::Log( "Create: Direct3DCreate8 failed" );
    return false;
  }

  m_Transform[TT_WORLD].Identity();
  m_Transform[TT_VIEW].Identity();
  m_Transform[TT_PROJECTION].Identity();

  m_CreationWidth   = Width;
  m_CreationHeight  = Height;
  m_CreationDepth   = Depth;

  m_Canvas.Set( 0, 0, Width, Height );
  m_VirtualSize.Set( Width, Height );

  m_bWindowed         = !( Flags & XRenderer::IN_FULLSCREEN );

  // Build a list of Direct3D adapters, modes and devices. The
  // ConfirmDevice() callback is used to confirm that only devices that
  // meet the app's requirements are considered.
  if ( FAILED( hr = BuildDeviceList() ) )
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: failed to build device list" );
    return false;
  }

  if ( !FindSuitableMode( m_CreationWidth, m_CreationHeight, m_CreationDepth, m_bWindowed ) )
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: failed to find suitable mode" );
    return false;
  }

  // Unless a substitute hWnd has been specified, create a window to
  // render into
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
  if ( m_hwndViewport == NULL)
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: no valid window handle" );
    return false;
  }

  RECT    rcParent;

  GetClientRect( m_hwndViewport, &rcParent );

  m_hWndFocus = m_hwndViewport;

  SortAndCleanDisplayModes();

  // Initialize the 3D environment for the app
  if ( FAILED( hr = Initialize3DEnvironment() ) )
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: Initialize3DEnvironment failed" );
    return false;
  }

  // The app is ready to go
  m_bReady = TRUE;

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
          Log( "Renderer.General", "DX9Renderer: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front().c_str(), (GR::Graphic::eImageFormat)ForcedFormat, ColorKey, MipMaps, ColorKeyReplacementColor );

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
              GR::String&  strPath( *it );

              GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)ForcedFormat, ColorKey, ColorKeyReplacementColor );
              if ( pData == NULL )
              {
                Log( "Renderer.General", CMisc::printf( "DX9Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, ColorKey, Level );
                delete pData;
              }

              ( (CDX9Texture*)pTexture )->FileNames.push_back( strPath );

              ++Level;
              ++it;
            }
          }
          else
          {
            Log( "Renderer.General", CMisc::printf( "DX9Renderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
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
          Log( "Renderer.General", CMisc::printf( "DX9Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
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

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     LoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        XFont*      pFont = NULL;

        if ( ( LoadFlags & XFont::FLF_SQUARED )
        ||   ( LoadFlags & XFont::FLF_SQUARED_ONE_FONT ) )
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
            Log( "Renderer.General", "DX9Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          Log( "Renderer.General", "DX9Renderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }

      GR::up    meshCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_MESH );
      for ( GR::up i = 0; i < meshCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_MESH, i );

        XMesh*       pMesh = NULL;

        pMesh = CT3DMeshLoader::Load( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "File" ) ).c_str() );
        if ( pMesh )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX9Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
          }
          delete pAsset;

          Xtreme::Asset::XAssetMesh*    pAssetMesh = new Xtreme::Asset::XAssetMesh( *pMesh );

          pAssetMesh->m_Mesh.CalculateBoundingBox();

          pAssetMesh->m_pVertexBuffer = CreateVertexBuffer( pAssetMesh->m_Mesh );

          pAsset = pAssetMesh;
          pLoader->SetAsset( Xtreme::Asset::XA_MESH, i, pAsset );

          delete pMesh;
        }
        else
        {
          Log( "Renderer.General", "DX9Renderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
        }
      }

      /*
      Xtreme::Asset::XAssetLoader::tMapAssets::iterator   it( pLoader->m_mapAssets.find( Xtreme::Asset::XA_IMAGE ) );
      if ( it != pLoader->m_mapAssets.end() )
      {
        Xtreme::Asset::XAssetLoader::tResourceMap&   resMap( it->second );

        Xtreme::Asset::XAssetLoader::tResourceMap::iterator   it( resMap.begin() );
        while ( it != resMap.end() )
        {
          std::pair<GR::Strings::XMLElement*,Xtreme::Asset::XAsset*>& pairAsset( it->second );

          GR::Strings::XMLElement& xmlAsset( *pairAsset.first );

          GR::u32     ColorKey = GR::Convert::ToU32( xmlAsset.Attribute( "ColorKey" ), 16 );

          GR::u32     ForcedFormat = GR::Convert::ToU32( xmlAsset.Attribute( "ForcedFormat" ) );

          std::list<GR::String>    listImages;

          GR::u32     MipMaps = 0;

          while ( true )
          {
            if ( MipMaps == 0 )
            {
              listImages.push_back( AppPath( xmlAsset.Attribute( "File" ).c_str() ) );
            }
            else
            {
              GR::String   strPath = xmlAsset.Attribute( CMisc::printf( "MipMap%d", MipMaps ) );

              if ( strPath.empty() )
              {
                break;
              }
              listImages.push_back( AppPath( xmlAsset.Attribute( CMisc::printf( "MipMap%d", MipMaps ) ).c_str() ) );
            }
            MipMaps++;
          }

          if ( listImages.empty() )
          {
            dh::Error( "DX9Renderer: Asset Image has no images!" );
          }
          else
          {
            XTexture* pTexture = LoadTexture( listImages.front().c_str(), (GR::Graphic::eImageFormat)ForcedFormat, ColorKey, MipMaps );

            if ( pTexture )
            {
              if ( pairAsset.second )
              {
                dh::Error( "DX9Renderer: Asset already loaded! %s", xmlAsset.Attribute( "Name" ).c_str() );
              }
              delete pairAsset.second;

              pairAsset.second = new Xtreme::Asset::XAssetImage( pTexture );
              pairAsset.second->m_strName = xmlAsset.Attribute( "Name" );

              // MipMaps laden
              std::list<GR::String>::iterator    it( listImages.begin() );
              it++;
              GR::u32   Level = 1;

              while ( it != listImages.end() )
              {
                GR::String&  strPath( *it );

                GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)ForcedFormat, ColorKey );
                if ( pData == NULL )
                {
                  dh::Error( "DX9Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() );
                }
                else
                {
                  CopyDataToTexture( pTexture, *pData, ColorKey, Level );
                  delete pData;
                }

                ( (CDX9Texture*)pTexture )->m_listFileNames.push_back( strPath );

                ++Level;
                ++it;
              }
            }
            else
            {
              dh::Error( "DX9Renderer: Failed to load image %s", xmlAsset.Attribute( "Name" ).c_str() );
            }
          }

          ++it;
        }
      }
      Xtreme::Asset::XAssetLoader::tMapAssets::iterator   it2( pLoader->m_mapAssets.find( Xtreme::Asset::XA_IMAGE_SECTION ) );
      if ( it2 != pLoader->m_mapAssets.end() )
      {
        Xtreme::Asset::XAssetLoader::tResourceMap&   resMap( it2->second );

        Xtreme::Asset::XAssetLoader::tResourceMap::iterator   it( resMap.begin() );
        while ( it != resMap.end() )
        {
          std::pair<GR::Strings::XMLElement*,Xtreme::Asset::XAsset*>& pairAsset( it->second );

          GR::Strings::XMLElement& xmlAsset( *pairAsset.first );

          Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, xmlAsset.Attribute( "Image" ).c_str() );
          if ( pTexture == NULL )
          {
            dh::Error( "DX9Renderer: ImageSection Asset, Image %s not found", xmlAsset.Attribute( "Image" ).c_str() );
          }
          else
          {
            XTextureSection   tsSection;

            tsSection.m_XOffset = GR::Convert::ToI32( xmlAsset.Attribute( "X" ) );
            tsSection.m_YOffset = GR::Convert::ToI32( xmlAsset.Attribute( "Y" ) );
            tsSection.m_Width   = GR::Convert::ToI32( xmlAsset.Attribute( "W" ) );
            tsSection.m_Height  = GR::Convert::ToI32( xmlAsset.Attribute( "H" ) );
            tsSection.m_Flags   = GR::Convert::ToU32( xmlAsset.Attribute( "Flags" ) );

            tsSection.m_pTexture = pTexture->Texture();

            tsSection.CalcTexCoords();

            if ( pairAsset.second )
            {
              dh::Error( "DX9Renderer: Asset already loaded! %s", xmlAsset.Attribute( "Name" ).c_str() );
            }
            delete pairAsset.second;

            pairAsset.second = new Xtreme::Asset::XAssetImageSection( tsSection );
            pairAsset.second->m_strName = xmlAsset.Attribute( "Name" );
          }

          ++it;
        }
      }

      it2 = pLoader->m_mapAssets.find( Xtreme::Asset::XA_FONT );
      if ( it2 != pLoader->m_mapAssets.end() )
      {
        Xtreme::Asset::XAssetLoader::tResourceMap&   resMap( it2->second );

        Xtreme::Asset::XAssetLoader::tResourceMap::iterator   it( resMap.begin() );
        while ( it != resMap.end() )
        {
          std::pair<GR::Strings::XMLElement*,Xtreme::Asset::XAsset*>& pairAsset( it->second );

          GR::Strings::XMLElement& xmlAsset( *pairAsset.first );

          GR::u32     ColorKey = GR::Convert::ToU32( xmlAsset.Attribute( "ColorKey" ), 16 );

          GR::u32     LoadFlags = GR::Convert::ToU32( xmlAsset.Attribute( "LoadFlags" ), 16 );

          XFont*      pFont = NULL;

          if ( ( LoadFlags & XFont::FLF_SQUARED )
          ||   ( LoadFlags & XFont::FLF_SQUARED_ONE_FONT ) )
          {
            pFont = LoadFontSquare( AppPath( xmlAsset.Attribute( "File" ).c_str() ), 
                                             LoadFlags, 
                                             ColorKey );
          }
          else
          {
            pFont = LoadFont( AppPath( xmlAsset.Attribute( "File" ).c_str() ), ColorKey );
          }

          if ( pFont )
          {
            if ( pairAsset.second )
            {
              dh::Error( "DX9Renderer: Asset already loaded! %s", xmlAsset.Attribute( "Name" ).c_str() );
            }
            delete pairAsset.second;

            pairAsset.second = new Xtreme::Asset::XAssetFont( pFont );
            pairAsset.second->m_strName = xmlAsset.Attribute( "Name" );
          }
          else
          {
            dh::Error( "DX9Renderer: Failed to load font %s", xmlAsset.Attribute( "Name" ).c_str() );
          }

          ++it;
        }
      }
      it2 = pLoader->m_mapAssets.find( Xtreme::Asset::XA_MESH );
      if ( it2 != pLoader->m_mapAssets.end() )
      {
        Xtreme::Asset::XAssetLoader::tResourceMap&   resMap( it2->second );

        Xtreme::Asset::XAssetLoader::tResourceMap::iterator   it( resMap.begin() );
        while ( it != resMap.end() )
        {
          std::pair<GR::Strings::XMLElement*,Xtreme::Asset::XAsset*>& pairAsset( it->second );

          GR::Strings::XMLElement& xmlAsset( *pairAsset.first );

          XMesh*       pMesh = NULL;

          pMesh = CT3DMeshLoader::Load( AppPath( xmlAsset.Attribute( "File" ).c_str() ) );
          if ( pMesh )
          {
            if ( pairAsset.second )
            {
              dh::Error( "DX9Renderer: Asset already loaded! %s", xmlAsset.Attribute( "Name" ).c_str() );
            }
            delete pairAsset.second;

            Xtreme::Asset::XAssetMesh*    pAssetMesh = new Xtreme::Asset::XAssetMesh( *pMesh );

            pAssetMesh->m_Mesh.CalculateBoundingBox();

            pAssetMesh->m_pVertexBuffer = CreateVertexBuffer( pAssetMesh->m_Mesh );

            pairAsset.second = pAssetMesh;
            pairAsset.second->m_strName = xmlAsset.Attribute( "Name" );

            delete pMesh;
          }
          else
          {
            dh::Error( "DX9Renderer: Failed to load Mesh %s", xmlAsset.Attribute( "Name" ).c_str() );
          }

          ++it;
        }
      }
      */
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  NotifyService( "Renderer", "Initialised" );

  return true;
}



bool CDX9RenderClass::Release()
{
  ReleaseAssets();

  Cleanup3DEnvironment();

  NotifyService( "Renderer", "Released" );
  return true;
}



bool CDX9RenderClass::FindSuitableMode( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool bWindowed )
{

  int   iAdapter = 0;

  std::vector<tD3DAdapterInfo>::iterator   itAdapter( m_vectAdapters.begin() );
  while ( itAdapter != m_vectAdapters.end() )
  {
    tD3DAdapterInfo&    AdapterInfo( *itAdapter );

    int   iDevice = 0;

    std::vector<tD3DDeviceInfo>::iterator   itDevice( AdapterInfo.vectDevices.begin() );
    while ( itDevice != AdapterInfo.vectDevices.end() )
    {
      tD3DDeviceInfo&   DeviceInfo( *itDevice );

      if ( DeviceInfo.deviceType == D3DDEVTYPE_HAL )
      {
        if ( ( ( bWindowed )
        &&     ( DeviceInfo.bCanDoWindowed ) )
        ||   ( !bWindowed ) )
        {
          int               iMode = 0;

          std::vector<tD3D9ModeInfo>::iterator   itMode( DeviceInfo.m_vectMode.begin() );
          while ( itMode != DeviceInfo.m_vectMode.end() )
          {
            tD3D9ModeInfo&   ModeInfo( *itMode );

            if ( bWindowed )
            {
              // im Fenstermode kann nur die aktive Tiefe benutzt werden!
              Depth = GR::Graphic::ImageData::DepthFromImageFormat( MapFormat( ModeInfo.m_d3dfPixel ) );
              //dh::Log( "Depth changed to %d", Depth );
            }

            if ( ( bWindowed )
            ||   ( ( !bWindowed )
            &&     ( ModeInfo.m_Width == Width )
            &&     ( ModeInfo.m_Height == Height ) ) )
            {
              if ( ( ( Depth == 24 )
              ||     ( Depth == 32 ) )
              &&   ( ModeInfo.m_d3dfPixel == D3DFMT_R8G8B8 )
              ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A8R8G8B8 )
              ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X8R8G8B8 ) )
              {
                m_Adapter                     = iAdapter;
                AdapterInfo.CurrentDevice     = iDevice;
                DeviceInfo.CurrentMode        = iMode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_d3dfPixel );
                  m_FullscreenDisplayMode.FullScreen = true;
                }
                return true;
              }
              if ( ( Depth == 16 )
              &&   ( ModeInfo.m_d3dfPixel == D3DFMT_R5G6B5 )
              ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X1R5G5B5 )
              ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A1R5G5B5 ) )
              {
                m_Adapter                     = iAdapter;
                AdapterInfo.CurrentDevice     = iDevice;
                DeviceInfo.CurrentMode        = iMode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_d3dfPixel );
                  m_FullscreenDisplayMode.FullScreen = true;
                }
                return true;
              }
            }

            ++itMode;
            ++iMode;
          }
        }
      }

      ++itDevice;
      ++iDevice;
    }


    ++itAdapter;
    ++iAdapter;
  }

  return false;

}



// TODO - weg damit!!
int SortModesCallback( const VOID* arg1, const VOID* arg2 )
{

  D3DDISPLAYMODE* p1 = (D3DDISPLAYMODE*)arg1;
  D3DDISPLAYMODE* p2 = (D3DDISPLAYMODE*)arg2;

  if ( p1->Format > p2->Format )
  {
    return -1;
  }
  if ( p1->Format < p2->Format )
  {
    return +1;
  }
  if ( p1->Width  < p2->Width )
  {
    return -1;
  }
  if ( p1->Width  > p2->Width )
  {
    return +1;
  }
  if ( p1->Height < p2->Height )
  {
    return -1;
  }
  if ( p1->Height > p2->Height )
  {
    return +1;
  }

  return 0;

}




HRESULT CDX9RenderClass::BuildDeviceList()
{

  m_bWindowedModePossible = false;

  // Loop through all the adapters on the system (usually, there's just one
  // unless more than one graphics card is present).
  for ( UINT iAdapter = 0; iAdapter < m_pD3D->GetAdapterCount(); iAdapter++ )
  {
    // Fill in adapter info
    tD3DAdapterInfo      Adapter;

    m_pD3D->GetAdapterIdentifier( iAdapter, 0, &Adapter.d3dAdapterIdentifier );
    m_pD3D->GetAdapterDisplayMode( iAdapter, &Adapter.d3ddmDesktop );
    Adapter.vectDevices.clear();
    Adapter.CurrentDevice = 0;

    CheckAdapterMode( Adapter, iAdapter, D3DFMT_A1R5G5B5 );
    CheckAdapterMode( Adapter, iAdapter, D3DFMT_A2R10G10B10 );
    CheckAdapterMode( Adapter, iAdapter, D3DFMT_A8R8G8B8 );
    CheckAdapterMode( Adapter, iAdapter, D3DFMT_R5G6B5 );
    CheckAdapterMode( Adapter, iAdapter, D3DFMT_X1R5G5B5 );
    CheckAdapterMode( Adapter, iAdapter, D3DFMT_X8R8G8B8 );


    // If valid devices were found, keep this adapter
    if ( Adapter.vectDevices.size() > 0 )
    {
      m_vectAdapters.push_back( Adapter );
    }
  }

  // Return an error if no compatible devices were found
  if ( m_vectAdapters.empty() )
  {
    return 1;//D3DAPPERR_NOCOMPATIBLEDEVICES;
  }

  // hier erlauben wir, bei nicht möglichem Im-Fenster auf Fullscreen zu gehen
  if ( ( m_bWindowed )
  &&   ( !m_bWindowedModePossible ) )
  {
    m_bWindowed = false;
  }

  // Pick a default device that can render into a window
  // (This code assumes that the HAL device comes before the REF
  // device in the device array).
  for ( DWORD a = 0; a < m_vectAdapters.size(); a++ )
  {
    tD3DAdapterInfo&   AdapterInfo = m_vectAdapters[a];

    for ( DWORD d = 0; d < AdapterInfo.vectDevices.size(); d++ )
    {
      tD3DDeviceInfo&    DeviceInfo  = AdapterInfo.vectDevices[d];

      if ( DeviceInfo.bWindowed == m_bWindowed )
      {
        m_vectAdapters[a].CurrentDevice = d;
        m_Adapter = a;
        return S_OK;
      }
    }
  }

  // kein Device gefunden, das im Fenster rendern kann
  return 2;
}



void CDX9RenderClass::CheckAdapterMode( tD3DAdapterInfo& Adapter, int iAdapter, D3DFORMAT d3dFmt )
{
  const DWORD         NumDeviceTypes = 2;

  const TCHAR*        strDeviceDescs[] = { "HAL", "REF" };



  const D3DDEVTYPE    DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };


  BOOL                bHALExists                  = FALSE,
                      bHALIsWindowedCompatible    = FALSE,
                      bHALIsDesktopCompatible     = FALSE,
                      bHALIsSampleCompatible      = FALSE;

  // Enumerate all display modes on this adapter
  D3DDISPLAYMODE          modes[100];
  D3DFORMAT               formats[20];
  DWORD                   NumFormats      = 0,
                          NumModes        = 0,
                          NumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter, d3dFmt );


  // Add the adapter's current desktop format to the list of formats
  //formats[NumFormats++] = Adapter.d3ddmDesktop.Format;

  for ( UINT iMode = 0; iMode < NumAdapterModes; iMode++ )
  {
    // Get the display mode attributes
    D3DDISPLAYMODE      DisplayMode;

    m_pD3D->EnumAdapterModes( iAdapter, d3dFmt, iMode, &DisplayMode );

    // Check if the mode already exists (to filter out refresh rates)
    DWORD   m = 0;
    for ( m = 0L; m < NumModes; m++ )
    {
      if ( ( modes[m].Width  == DisplayMode.Width )
      &&   ( modes[m].Height == DisplayMode.Height )
      &&   ( modes[m].Format == DisplayMode.Format ) )
      {
        break;
      }
    }

    // If we found a new mode, add it to the list of modes
    if ( m == NumModes )
    {
      modes[NumModes].Width       = DisplayMode.Width;
      modes[NumModes].Height      = DisplayMode.Height;
      modes[NumModes].Format      = DisplayMode.Format;
      modes[NumModes].RefreshRate = 0;
      NumModes++;

      // Check if the mode's format already exists
      DWORD   f = 0;
      for ( f = 0; f < NumFormats; f++ )
      {
        if ( DisplayMode.Format == formats[f] )
        {
          break;
        }
      }

      // If the format is new, add it to the list
      if ( f == NumFormats )
      {
        formats[NumFormats++] = DisplayMode.Format;
      }
    }
  }

  // Sort the list of display modes (by format, then width, then height)
  qsort( modes, NumModes, sizeof( D3DDISPLAYMODE ), SortModesCallback );

  // Add devices to adapter
  for ( UINT iDevice = 0; iDevice < NumDeviceTypes; iDevice++ )
  {
    // Fill in device info
    tD3DDeviceInfo Device;
    //Device                   = &pAdapter->devices[pAdapter->NumDevices];
    Device.deviceType       = DeviceTypes[iDevice];

    m_pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &Device.d3dCaps );
    Device.strName          = strDeviceDescs[iDevice];
    Device.m_vectMode.clear();
    Device.CurrentMode    = 0;
    Device.bCanDoWindowed   = FALSE;
    Device.bWindowed        = FALSE;
    Device.MultiSampleType  = D3DMULTISAMPLE_NONE;

    // Examine each format supported by the adapter to see if it will
    // work with this device and meets the needs of the application.
    BOOL        bFormatConfirmed[50];

    DWORD       Behavior[50];

    D3DFORMAT   fmtDepthStencil[50];


    for ( DWORD f = 0; f < NumFormats; f++ )
    {
      bFormatConfirmed[f] = FALSE;
      fmtDepthStencil[f]  = D3DFMT_UNKNOWN;

      // Skip formats that cannot be used as render targets on this device
      if ( FAILED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, formats[f], formats[f], FALSE ) ) )
      {
        continue;
      }
      if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, formats[f], formats[f], TRUE ) ) )
      {
        Device.bCanDoWindowed = true;
        Device.bWindowed      = true;
      }


      if ( Device.deviceType == D3DDEVTYPE_HAL )
      {
        // This system has a HAL device
        bHALExists = TRUE;

        if ( Device.bWindowed )
        {
          // HAL can run in a window for some mode
          bHALIsWindowedCompatible = TRUE;
          if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, Adapter.d3ddmDesktop.Format, formats[f], FALSE ) ) )
          {
            // HAL can run in a window for the current desktop mode
            bHALIsDesktopCompatible = TRUE;
            m_bWindowedModePossible = true;
          }
        }
      }

      // Confirm the device/format for HW vertex processing
      if ( Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
      {
        if ( Device.d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE )
        {
          Behavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;

          if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, Behavior[f], formats[f] ) ) )
          {
            bFormatConfirmed[f] = TRUE;
          }
        }

        if ( !bFormatConfirmed[f] )
        {
          Behavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;

          if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, Behavior[f], formats[f] ) ) )
          {
            bFormatConfirmed[f] = TRUE;
          }
        }

        if ( !bFormatConfirmed[f] )
        {
          Behavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

          if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, Behavior[f], formats[f] ) ) )
          {
            bFormatConfirmed[f] = TRUE;
          }
        }
      }

      // Confirm the device/format for SW vertex processing
      if ( !bFormatConfirmed[f] )
      {
        Behavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

        if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, Behavior[f], formats[f] ) ) )
        {
          bFormatConfirmed[f] = TRUE;
        }
      }

      // Find a suitable depth/stencil buffer format for this device/format
      if ( ( bFormatConfirmed[f] )
      &&   ( m_bUseDepthBuffer ) )
      {
        if ( !FindDepthStencilFormat( iAdapter, Device.deviceType, formats[f], &fmtDepthStencil[f] ) )
        {
          bFormatConfirmed[f] = FALSE;
        }
      }
    }

    // Add all enumerated display modes with confirmed formats to the
    // device's list of valid modes
    for ( DWORD m = 0L; m < NumModes; m++ )
    {
      for ( DWORD f = 0; f < NumFormats; f++ )
      {
        if ( modes[m].Format == formats[f] )
        {
          if ( bFormatConfirmed[f] )
          {
            // Add this mode to the device's list of valid modes
            tD3D9ModeInfo   modeInfo;


            modeInfo.m_Width    = modes[m].Width;
            modeInfo.m_Height   = modes[m].Height;
            modeInfo.m_d3dfPixel   = modes[m].Format;
            modeInfo.m_Behaviour   = Behavior[f];
            modeInfo.m_d3dfDepthStencil   = fmtDepthStencil[f];
            Device.m_vectMode.push_back( modeInfo );

            m_DisplayModes.push_back( XRendererDisplayMode( modeInfo.m_Width, modeInfo.m_Height, MapFormat( modeInfo.m_d3dfPixel ) ) );

            if ( Device.deviceType == D3DDEVTYPE_HAL )
            {
              bHALIsSampleCompatible = TRUE;
            }
          }
        }
      }
    }

    // Select any 640x480 mode for default (but prefer a 16-bit mode)
    SelectDefaultMode( iAdapter, &Adapter, &Device );

    // If valid modes were found, keep this device
    if ( Device.m_vectMode.size() )
    {
      Adapter.vectDevices.push_back( Device );
    }
  }
}



HRESULT CDX9RenderClass::Initialize3DEnvironment()
{
  if ( m_pD3D == NULL )
  {
    return E_FAIL;
  }

  if ( m_Adapter >= m_vectAdapters.size() )
  {
    dh::Warning( "[DX9][Initialize3DEnvironment] Adapter out of bounds (%d > %d)",
                 m_Adapter,
                 m_vectAdapters.size() );
    return E_FAIL;
  }

  HRESULT hr;


  tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_Adapter];

  if ( pAdapterInfo->CurrentDevice >= pAdapterInfo->vectDevices.size() )
  {
    dh::Warning( "[DX9][Initialize3DEnvironment] Device out of bounds (%d > %d)",
                 pAdapterInfo->CurrentDevice,
                 pAdapterInfo->vectDevices.size() );
    return E_FAIL;
  }

  tD3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->vectDevices[pAdapterInfo->CurrentDevice];


  m_CurrentAdapter  = m_Adapter;
  m_pCurrentModeInfo  = &pDeviceInfo->m_vectMode[pDeviceInfo->CurrentMode];

  m_CurrentDevice   = pAdapterInfo->CurrentDevice;
  m_CurrentMode     = pDeviceInfo->CurrentMode;
                                  
  pDeviceInfo->bWindowed  = m_bWindowed;

  // Set up the presentation parameters
  ZeroMemory( &m_d3dpp, sizeof( m_d3dpp ) );
  m_d3dpp.Windowed                = pDeviceInfo->bWindowed;
  m_d3dpp.BackBufferCount         = 1;
  m_d3dpp.MultiSampleType         = pDeviceInfo->MultiSampleType;
  m_d3dpp.SwapEffect              = D3DSWAPEFFECT_FLIP;
  m_d3dpp.EnableAutoDepthStencil  = m_bUseDepthBuffer;
  m_d3dpp.AutoDepthStencilFormat  = m_pCurrentModeInfo->m_d3dfDepthStencil;
  m_d3dpp.hDeviceWindow           = m_hwndViewport;
  m_d3dpp.PresentationInterval    = D3DPRESENT_INTERVAL_IMMEDIATE;
  
  if ( m_bWindowed )
  {
    if ( m_bForceWindowSize )
    {
      m_d3dpp.BackBufferWidth  = m_iForcedWindowWidth;
      m_d3dpp.BackBufferHeight = m_iForcedWindowHeight;
    }
    else
    {
      RECT    rcWindowClient;

      GetClientRect( m_hwndViewport, &rcWindowClient );
      m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
      m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;
    }

    // im Fenster sollte immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;

    if ( !m_VSyncEnabled )
    {
      m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.PresentationInterval = 0;
    }
  }
  else
  {
    m_d3dpp.BackBufferWidth  = m_pCurrentModeInfo->m_Width;
    m_d3dpp.BackBufferHeight = m_pCurrentModeInfo->m_Height;
    m_d3dpp.BackBufferFormat = m_pCurrentModeInfo->m_d3dfPixel;
    if ( !m_VSyncEnabled )
    {
      m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.PresentationInterval = 0;
    }

    m_d3dpp.hDeviceWindow           = m_hwndViewport;

    // Fenster anpassen
    AdjustWindowForMode( false );

    //SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  }

  if ( ( m_d3dpp.BackBufferHeight == 0 )
  ||   ( m_d3dpp.BackBufferWidth == 0 ) )
  {
    return E_FAIL;
  }

  m_Canvas.Set( 0, 0, m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight );

  // Create the device
  DWORD   Behaviour = m_pCurrentModeInfo->m_Behaviour;
  //Behaviour = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  if ( m_bMultiThreadSafe )
  {
    Behaviour |= D3DCREATE_MULTITHREADED;
  }
  hr = m_pD3D->CreateDevice( m_Adapter, pDeviceInfo->deviceType,
                             m_hWndFocus, Behaviour, &m_d3dpp,
                             &m_pd3dDevice );
  if ( SUCCEEDED( hr ) )
  {
    m_RenderWidth = m_d3dpp.BackBufferWidth;
    m_RenderHeight = m_d3dpp.BackBufferHeight;

    // den Viewport aufbewahren (für Pure-Devices)
    // das sind die Default-Werte (laut Dokumentation)
    m_ViewPort.X        = 0;
    m_ViewPort.Y        = 0;
    m_ViewPort.Width    = m_RenderWidth;
    m_ViewPort.Height   = m_RenderHeight;
    m_ViewPort.MinZ      = 0.0f;
    m_ViewPort.MaxZ      = 1.0f;

    // When moving from fullscreen to windowed mode, it is important to
    // adjust the window size after recreating the device rather than
    // beforehand to ensure that you get the window size you want.  For
    // example, when switching from 640x480 fullscreen to windowed with
    // a 1000x600 window on a 1024x768 desktop, it is impossible to set
    // the window size to 1000x600 until after the display mode has
    // changed to 1024x768, because windows cannot be larger than the
    // desktop.


    // Store device Caps
    m_pd3dDevice->GetDeviceCaps( &m_DeviceCaps );
    m_CreateFlags = m_pCurrentModeInfo->m_Behaviour;

    // Store render target surface desc
    LPDIRECT3DSURFACE9 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
    pBackBuffer->Release();

    // Initialize the app's device-dependent objects
    hr = InitDeviceObjects();
    if ( SUCCEEDED( hr ) )
    {
      hr = RestoreDeviceObjects();
      if ( SUCCEEDED( hr ) )
      {
        m_bActive = TRUE;
        return S_OK;
      }
    }
    else
    {
      dh::Log( "Initialize3DEnvironment: InitDeviceObjects failed" );
    }

    // Cleanup before we try again
    InvalidateDeviceObjects();
    DeleteDeviceObjects();
    if ( m_pd3dDevice )
    {
      m_pd3dDevice->Release();
      m_pd3dDevice = NULL;
    }
  }
  else
  {
    dh::Log( "Initialize3DEnvironment: CreateDevice failed" );
  }

  // If that failed, fall back to the reference rasterizer
  if ( pDeviceInfo->deviceType == D3DDEVTYPE_HAL )
  {
    // Select the default adapter
    m_Adapter = 0L;
    pAdapterInfo = &m_vectAdapters[m_Adapter];

    // Look for a software device
    for ( UINT i = 0L; i < pAdapterInfo->vectDevices.size(); i++ )
    {
      if ( pAdapterInfo->vectDevices[i].deviceType == D3DDEVTYPE_REF )
      {
        pAdapterInfo->CurrentDevice = i;
        pDeviceInfo = &pAdapterInfo->vectDevices[i];
        m_bWindowed = pDeviceInfo->bWindowed;
        break;
      }
    }

    // Try again, this time with the reference rasterizer
    if ( pAdapterInfo->vectDevices[pAdapterInfo->CurrentDevice].deviceType == D3DDEVTYPE_REF )
    {
      hr = Initialize3DEnvironment();
      if ( FAILED( hr ) )
      {
        dh::Log( "Initialize3DEnvironment: 2nd time failed" );
      }
    }
  }
  NotifyService( "Renderer", "Initialised" );
  return hr;
}



void CDX9RenderClass::Cleanup3DEnvironment()
{
  m_bActive = FALSE;
  m_bReady  = FALSE;

  if ( m_pd3dDevice )
  {
    DestroyAllTextures();
    DestroyAllVertexBuffers();
    DestroyAllFonts();
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

    m_pd3dDevice->Release();
    m_pd3dDevice = NULL;
    m_pD3D->Release();

    m_pd3dDevice = NULL;
    m_pD3D       = NULL;
  }
}



bool CDX9RenderClass::IsReady() const
{
  return m_bReady;
}



HRESULT CDX9RenderClass::ConfirmDevice( D3DCAPS9* pCaps, DWORD Behaviour, D3DFORMAT Format )   
{ 
  return S_OK; 
}



BOOL CDX9RenderClass::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{

  if ( ( m_MinDepthBits <= 16 )
  &&   ( m_MinStencilBits == 0 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D16 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D16;
        return TRUE;
      }
    }
  }

  if ( ( m_MinDepthBits <= 15 )
  &&   ( m_MinStencilBits <= 1 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D15S1 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D15S1 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D15S1;
        return TRUE;
      }
    }
  }

  if ( ( m_MinDepthBits <= 24 )
  &&   ( m_MinStencilBits == 0 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24X8 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24X8;
        return TRUE;
      }
    }
  }

  if ( ( m_MinDepthBits <= 24 )
  &&   ( m_MinStencilBits <= 8 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24S8 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24S8;
        return TRUE;
      }
    }
  }

  if ( ( m_MinDepthBits <= 24 )
  &&   ( m_MinStencilBits <= 4 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D24X4S4 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D24X4S4;
        return TRUE;
      }
    }
  }

  if ( ( m_MinDepthBits <= 32 )
  &&   ( m_MinStencilBits == 0 ) )
  {
    if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( iAdapter, DeviceType, TargetFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 ) ) )
    {
      if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( iAdapter, DeviceType, TargetFormat, TargetFormat, D3DFMT_D32 ) ) )
      {
        *pDepthStencilFormat = D3DFMT_D32;
        return TRUE;
      }
    }
  }

  return FALSE;

}



void CDX9RenderClass::SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice )
{

  if ( !pDevice ) 
  {
    dh::Log( "no device\n" );
    return;
  }

  pDevice->CurrentMode = 0;
  for ( unsigned int m = 0; m < pDevice->m_vectMode.size(); m++ )
  {
    tD3D9ModeInfo&    ModeInfo = pDevice->m_vectMode[m];

    if ( ( ModeInfo.m_Width == 640 )
    &&   ( ModeInfo.m_Height == 480 ) )
    {
      if ( ( m_CreationDepth == 24 )
      ||   ( m_CreationDepth == 32 ) )
      {
        if ( ( ModeInfo.m_d3dfPixel == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X8R8G8B8 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = 640;
          m_FullscreenDisplayMode.Height    = 480;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_d3dfPixel );
          m_FullscreenDisplayMode.FullScreen = true;
          break;
        }
      }
      if ( m_CreationDepth == 16 )
      {
        if ( ( ModeInfo.m_d3dfPixel == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = 640;
          m_FullscreenDisplayMode.Height    = 480;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_d3dfPixel );
          m_FullscreenDisplayMode.FullScreen = true;
          break;
        }
      }
    }
  }

  if ( m_FullscreenDisplayMode.Width == 0 )
  {
    // found no mode (depth not available?)
    // try again, but this time take any matching sized mode
    for ( unsigned int m = 0; m < pDevice->m_vectMode.size(); m++ )
    {
      tD3D9ModeInfo&    ModeInfo = pDevice->m_vectMode[m];

      Log( "Renderer.Full", CMisc::printf( "SelectDefaultMode  mode %dx%dx%d", ModeInfo.m_Width, ModeInfo.m_Height, ModeInfo.m_d3dfPixel ) );

      if ( ( ModeInfo.m_Width == m_CreationWidth )
      &&   ( ModeInfo.m_Height == m_CreationHeight ) )
      {
        if ( ( ModeInfo.m_d3dfPixel == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X8R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = m_CreationWidth;
          m_FullscreenDisplayMode.Height    = m_CreationHeight;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_d3dfPixel );
          m_FullscreenDisplayMode.FullScreen = true;
          Log( "Renderer.Full", "Keep this mode 16/24/32" );
          break;
        }
      }
    }
  }

  // kann im aktuellen Desktop-Modus im Fenster gerendert werden?
  if ( m_bWindowed )
  {
    if ( pDevice->CurrentMode >= pDevice->m_vectMode.size() )
    {
      if ( !pDevice->m_vectMode.empty() )
      {
        dh::Log( "CurrentMode out of bounds (%d > %d)", pDevice->CurrentMode, pDevice->m_vectMode.size() );
      }
    }
    else
    {
      tD3D9ModeInfo&    ModeInfo = pDevice->m_vectMode[pDevice->CurrentMode];

      if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapterNr, 
                                              pDevice->deviceType, 
                                              pAdapter->d3ddmDesktop.Format, 
                                              ModeInfo.m_d3dfPixel,
                                              FALSE ) ) )
      {
        m_bWindowedModePossible = true;
      }
    }
  }
}



bool CDX9RenderClass::ResizeSurface()
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

  if ( ( m_bActive )
  &&   ( m_bWindowed ) )
  {
    RECT rcWindowClient;

    GetClientRect( m_hwndViewport, &rcWindowClient );

    //MoveWindow( m_hwndViewport, 0, 0, rcWindowClient.right - rcWindowClient.left, rcWindowClient.bottom - rcWindowClient.top, TRUE );
    // Update window properties
    // A new window size will require a new backbuffer
    // size, so the 3D structures must be changed accordingly.
    HRESULT     hr;

    m_bReady = FALSE;

    m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
    m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;

    // Resize the 3D environment
    if ( FAILED( hr = Resize3DEnvironment() ) )
    {
      dh::Log( "MsgProc: Failed to Resize3DEnvironment" );
      bInside = FALSE;
      m_bReady = FALSE;
      return FALSE;
    }
    m_bReady = TRUE;
  }

  bInside = FALSE;

  return TRUE;

}



HRESULT CDX9RenderClass::Resize3DEnvironment()
{

  if ( m_pd3dDevice == NULL )
  {
    dh::Log( "Resize3DEnvironment: no device" );
    return E_FAIL;
  }

  HRESULT hr;

  // Release all vidmem objects
  if ( FAILED( hr = InvalidateDeviceObjects() ) )
  {
    dh::Log( "Resize3DEnvironment: InvalidateDeviceObjects failed" );
    return hr;
  }

  ReleaseAllTextures();
  ReleaseAllVertexBuffers();
  /*
  ReleaseFonts();
  */

  // Reset the device
  if ( ( !m_d3dpp.BackBufferWidth )
  ||   ( !m_d3dpp.BackBufferHeight ) )
  {
    dh::Log( "Resize3DEnvironment: invalid backbuffer size" );
    return E_FAIL;
  }

  if ( ( m_d3dpp.Windowed )
  &&   ( !m_bWindowed ) )
  {
    // alte Fenster-Werte merken -> soll nach Fullscreen
    AdjustWindowForMode( false );

    //SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    //m_hwnindowedModeParent = GetParent( m_hwndViewport );
    //SetWindowLong( m_hwndViewport, GWL_STYLE, GetWindowLong( m_hwndViewport, GWL_STYLE ) & ~WS_CHILD );
    //SetParent( m_hwndViewport, NULL );
  }

  bool    bOldModeWindowed = !!m_d3dpp.Windowed;

  m_d3dpp.Windowed = m_bWindowed;

  if ( !m_bWindowed )
  {
    // jetzt nach Fullscreen
    m_d3dpp.BackBufferWidth   = m_FullscreenDisplayMode.Width;
    m_d3dpp.BackBufferHeight  = m_FullscreenDisplayMode.Height;
    m_d3dpp.BackBufferFormat  = MapFormat( m_FullscreenDisplayMode.ImageFormat );
    if ( !m_VSyncEnabled )
    {
      m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.PresentationInterval = 0;
    }
    m_d3dpp.hDeviceWindow     = m_hwndViewport;
  }
  else
  {
    RECT    rc;
    GetClientRect( m_hwndViewport, &rc );

    m_d3dpp.BackBufferWidth  = rc.right - rc.left;
    m_d3dpp.BackBufferHeight = rc.bottom - rc.top;

    m_d3dpp.hDeviceWindow     = m_hwndViewport;

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = m_vectAdapters[m_CurrentAdapter].d3ddmDesktop.Format;

    if ( !m_VSyncEnabled )
    {
      m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }
  }

  if ( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
  {
    dh::Log( "Resize3DEnvironment: m_pd3dDevice->Reset failed" );
    return hr;
  }

  // Store render target surface desc
  LPDIRECT3DSURFACE9 pBackBuffer;
  m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
  pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
  pBackBuffer->Release();

  m_RenderWidth   = m_SurfaceDescBackBuffer.Width;
  m_RenderHeight  = m_SurfaceDescBackBuffer.Height;

  // den Viewport aufbewahren (für Pure-Devices)
  // das sind die Default-Werte (laut Dokumentation)
  m_ViewPort.X        = 0;
  m_ViewPort.Y        = 0;
  m_ViewPort.Width    = m_RenderWidth;
  m_ViewPort.Height   = m_RenderHeight;
  m_ViewPort.MinZ      = 0.0f;
  m_ViewPort.MaxZ      = 1.0f;

  if ( ( !bOldModeWindowed )
  &&   ( m_bWindowed ) )
  {
    // alte Fensterwerte wieder herstellen
    AdjustWindowForMode( true );
  }

  // Initialize the app's device-dependent objects
  hr = RestoreDeviceObjects();
  if ( FAILED( hr ) )
  {
    dh::Log( "Resize3DEnvironment: RestoreDeviceObjects failed" );
    return hr;
  }

  return S_OK;

}



bool CDX9RenderClass::InitDeviceObjects()
{

  RestoreDeviceObjects();
  return true;

}



bool CDX9RenderClass::RestoreDeviceObjects()
{

  RestoreStates();

  RestoreAllVertexBuffers();
  RestoreAllTextures();
  RestoreAllFonts();

  return true;

}



bool CDX9RenderClass::InvalidateDeviceObjects()
{

  return true;

}



bool CDX9RenderClass::DeleteDeviceObjects()
{

  return true;

}



bool CDX9RenderClass::BeginScene()
{

  HRESULT hr = S_OK;

  if ( !m_pd3dDevice )
  {
    return false;
  }
  if ( !m_bReady )
  {
    return false;
  }

  // Test the cooperative level to see if it's okay to render
  if ( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
  {

    // If the device was lost, do not render until we get it back
    if ( D3DERR_DEVICELOST == hr )
    {
      return false;
    }

    // Check if the device needs to be resized.
    if ( D3DERR_DEVICENOTRESET == hr )
    {
      // If we are windowed, read the desktop mode and use the same format for
      // the back buffer
      if ( m_bWindowed )
      {
        tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_Adapter];
        m_pD3D->GetAdapterDisplayMode( m_Adapter, &pAdapterInfo->d3ddmDesktop );
        m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
      }

      if ( FAILED( hr = Resize3DEnvironment() ) )
      {
        return false;
      }
    }
  }

  return !!SUCCEEDED( m_pd3dDevice->BeginScene() );

}



void CDX9RenderClass::EndScene()
{

  m_pd3dDevice->EndScene();

}



void CDX9RenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  if ( !m_strSaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_strSaveScreenShotFileName.c_str() );
    m_strSaveScreenShotFileName.clear();
  }

  // Show the frame on the primary surface.
  RECT    rcSrc;
  RECT*   pSrc = NULL;
  RECT    rcDest;
  RECT*   pDest = NULL;

  if ( rectSrc )
  {
    ::SetRect( &rcSrc, rectSrc->Left, rectSrc->Top, rectSrc->Right, rectSrc->Bottom );
    pSrc = &rcSrc;
  }
  if ( rectDest )
  {
    ::SetRect( &rcDest, rectDest->Left, rectDest->Top, rectDest->Right, rectDest->Bottom );
    pDest = &rcDest;
  }
  m_pd3dDevice->Present( pSrc, pDest, NULL, NULL );

}



void CDX9RenderClass::Clear( bool bClearColor, bool bClearZ )
{
  if ( m_pd3dDevice )
  {
    DWORD   ClearFlags = 0;
    
    if ( bClearColor )
    {
      ClearFlags |= D3DCLEAR_TARGET;
    }

    if ( ( m_bUseDepthBuffer )
    &&   ( bClearZ ) )
    {
      ClearFlags |= D3DCLEAR_ZBUFFER;
    }
    if ( m_MinStencilBits > 0 )
    {
      ClearFlags |= D3DCLEAR_STENCIL;
    }

    if ( ClearFlags == 0 )
    {
      return;
    }

    m_pd3dDevice->Clear( 0, NULL, ClearFlags, m_ClearColor, 1.0f, 0 );
  }
}



XTexture* CDX9RenderClass::LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  GR::String path = szFileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), imgFormatToConvert, ColorKey, ColorKeyReplacementColor );
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

  pTexture->m_ColorKey        = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;

}



XTexture* CDX9RenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), MipMapLevels );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



XTexture* CDX9RenderClass::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormatArg, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    return NULL;
  }

  GR::Graphic::eImageFormat    imgFormat( imgFormatArg );


  if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
  {
    // unterstützte Wechsel
    if ( imgFormat == GR::Graphic::IF_R8G8B8 )
    {
      imgFormat = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        return NULL;
      }
    }
    else if ( imgFormat == GR::Graphic::IF_X1R5G5B5 )
    {
      imgFormat = GR::Graphic::IF_R5G6B5;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        return NULL;
      }
    }
    else
    {
      return NULL;
    }
  }

  int   iTWidth = Width,
        iTHeight = Height;

  int iDummy = 1;
  for ( int i = 0; i < 32; i++ )
  {
    if ( iTWidth <= iDummy )
    {
      iTWidth = iDummy;
      break;
    }
    iDummy <<= 1;
  }
  iDummy = 1;
  for ( int i = 0; i < 32; i++ )
  {
    if ( iTHeight <= iDummy )
    {
      iTHeight = iDummy;
      break;
    }
    iDummy <<= 1;
  }


  CDX9Texture*          pTexture = NULL;

  pTexture = new( std::nothrow )CDX9Texture();
  if ( pTexture == NULL )
  {
    return NULL;
  }

  pTexture->m_MipMapLevels = MipMapLevels;
  if ( pTexture->m_MipMapLevels == 0 )
  {
    pTexture->m_MipMapLevels = 1;
  }
  pTexture->AllowUsageAsRenderTarget = AllowUsageAsRenderTarget;

  if ( FAILED( m_pd3dDevice->CreateTexture( 
                 iTWidth,
                 iTHeight,
                 pTexture->m_MipMapLevels,
                 AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
                 MapFormat( imgFormat ),
                 AllowUsageAsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
                 &pTexture->m_Surface,
                 NULL ) ) )
  {
    // failed to load (suupa)
    dh::Log( "CreateTexture: CreateTexture failed" );
    delete pTexture;

    return NULL;
  }

  // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
  D3DSURFACE_DESC ddsd;
  pTexture->m_Surface->GetLevelDesc( 0, &ddsd );

  pTexture->m_SurfaceSize.Set( ddsd.Width, ddsd.Height );
  pTexture->m_ImageSourceSize.Set( Width, Height );
  pTexture->m_d3dfPixelFormat     = ddsd.Format;
  pTexture->m_ImageFormat         = imgFormat;

  AddTexture( pTexture );

  return pTexture;

}



bool CDX9RenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  HRESULT hr = m_pD3D->CheckDeviceFormat( 
          m_CurrentAdapter,
          D3DDEVTYPE_HAL,
          m_pCurrentModeInfo->m_d3dfPixel,
          AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
          D3DRTYPE_TEXTURE,
          MapFormat( imgFormat ) );

  return !!SUCCEEDED( hr );
}



D3DFORMAT CDX9RenderClass::MapFormat( GR::Graphic::eImageFormat imgFormat )
{

  D3DFORMAT     d3dFmt = D3DFMT_UNKNOWN;

  switch ( imgFormat )
  {
    case GR::Graphic::IF_A1R5G5B5:
      d3dFmt = D3DFMT_A1R5G5B5;
      break;
    case GR::Graphic::IF_A4R4G4B4:
      d3dFmt = D3DFMT_A4R4G4B4;
      break;
    case GR::Graphic::IF_A8:
      d3dFmt = D3DFMT_A8;
      break;
    case GR::Graphic::IF_A8R8G8B8:
      d3dFmt = D3DFMT_A8R8G8B8;
      break;
    case GR::Graphic::IF_INDEX8:
      d3dFmt = D3DFMT_P8;
      break;
    case GR::Graphic::IF_R5G6B5:
      d3dFmt = D3DFMT_R5G6B5;
      break;
    case GR::Graphic::IF_R8G8B8:
      d3dFmt = D3DFMT_R8G8B8;
      break;
    case GR::Graphic::IF_X1R5G5B5:
      d3dFmt = D3DFMT_X1R5G5B5;
      break;
    case GR::Graphic::IF_X8R8G8B8:
      d3dFmt = D3DFMT_X8R8G8B8;
      break;
    default:
      dh::Log( "CDX9RenderClass::MapFormat Unsupported format %d", imgFormat );
      break;
  }

  return d3dFmt;

}



GR::Graphic::eImageFormat CDX9RenderClass::MapFormat( D3DFORMAT d3dFormat )
{
  GR::Graphic::eImageFormat  imgFmt = GR::Graphic::IF_UNKNOWN;

  switch ( d3dFormat )
  {
    case D3DFMT_A1R5G5B5:
      imgFmt = GR::Graphic::IF_A1R5G5B5;
      break;
    case D3DFMT_A4R4G4B4:
      imgFmt = GR::Graphic::IF_A4R4G4B4;
      break;
    case D3DFMT_A8:
      imgFmt = GR::Graphic::IF_A8;
      break;
    case D3DFMT_A8R8G8B8:
      imgFmt = GR::Graphic::IF_A8R8G8B8;
      break;
    case D3DFMT_P8:
      imgFmt = GR::Graphic::IF_INDEX8;
      break;
    case D3DFMT_R5G6B5:
      imgFmt = GR::Graphic::IF_R5G6B5;
      break;
    case D3DFMT_R8G8B8:
      imgFmt = GR::Graphic::IF_R8G8B8;
      break;
    case D3DFMT_X1R5G5B5:
      imgFmt = GR::Graphic::IF_X1R5G5B5;
      break;
    case D3DFMT_X8R8G8B8:
      imgFmt = GR::Graphic::IF_X8R8G8B8;
      break;
    case D3DFMT_A2B10G10R10:
      imgFmt = GR::Graphic::IF_A2B10G10R10;
      break;
    case D3DFMT_A2R10G10B10:
      imgFmt = GR::Graphic::IF_A2R10G10B10;
      break;
    default:
      dh::Log( "CDX9RenderClass::MapFormat Unsupported D3D format %d", d3dFormat );
      break;
  }

  return imgFmt;

}



void CDX9RenderClass::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                    GR::f32 fTU1, GR::f32 fTV1,
                                    GR::f32 fTU2, GR::f32 fTV2,
                                    GR::f32 fTU3, GR::f32 fTV3,
                                    GR::f32 fTU4, GR::f32 fTV4,
                                    GR::u32 Color1, GR::u32 Color2, 
                                    GR::u32 Color3, GR::u32 Color4, float fZ )
{
  iX += m_DisplayOffset.x;
  iY += m_DisplayOffset.y;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  CUSTOMVERTEX          vertData[4];

  float   fRHW = 0.9f;

  GR::tVector   ptPos( (float)iX, (float)iY, fZ );
  GR::tVector   ptSize( (float)iWidth, (float)iHeight, 0.1f );


  m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = m_Canvas.Left + ptPos.x * virtualX + m_DirectTexelMappingOffset.x;
  vertData[0].position.y  = m_Canvas.Top + ptPos.y * virtualY + m_DirectTexelMappingOffset.y;
  vertData[0].position.z  = (float)ptPos.z;
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = m_Canvas.Left + ( ptPos.x + ptSize.x ) * virtualX + m_DirectTexelMappingOffset.x;
  vertData[1].position.y  = m_Canvas.Top + ptPos.y * virtualY + m_DirectTexelMappingOffset.y;
  vertData[1].position.z  = (float)ptPos.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position.x  = m_Canvas.Left + ptPos.x * virtualX + m_DirectTexelMappingOffset.x;
  vertData[2].position.y  = m_Canvas.Top + ( ptPos.y + ptSize.y ) * virtualY + m_DirectTexelMappingOffset.y;
  vertData[2].position.z  = (float)ptPos.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position.x  = m_Canvas.Left + ( ptPos.x + ptSize.x ) * virtualX + m_DirectTexelMappingOffset.x;
  vertData[3].position.y  = m_Canvas.Top + ( ptPos.y + ptSize.y ) * virtualY + m_DirectTexelMappingOffset.y;
  vertData[3].position.z  = (float)ptPos.z;
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = Color4;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void CDX9RenderClass::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::f32 fTU3, GR::f32 fTV3,
                                          GR::f32 fTU4, GR::f32 fTV4,
                                          GR::u32 Color1, GR::u32 Color2, 
                                          GR::u32 Color3, GR::u32 Color4, float fZ )
{
  fX += m_DisplayOffset.x;
  fY += m_DisplayOffset.y;

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  CUSTOMVERTEX          vertData[4];

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  float   fRHW = 1.0f;

  GR::tVector   ptPos( fX, fY, fZ );
  GR::tVector   ptSize( fWidth, fHeight, 0.0f );

  m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position    = D3DXVECTOR3( m_Canvas.Left + ptPos.x * virtualX, m_Canvas.Top + ptPos.y * virtualY, ptPos.z );
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = m_Canvas.Left + ( ptPos.x + ptSize.x ) * virtualX;
  vertData[1].position.y  = m_Canvas.Top + ptPos.y * virtualY;
  vertData[1].position.z  = ptPos.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position.x  = m_Canvas.Left + ptPos.x * virtualX;
  vertData[2].position.y  = m_Canvas.Top + ( ptPos.y + ptSize.y ) * virtualY;
  vertData[2].position.z  = ptPos.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position.x  = m_Canvas.Left + ( ptPos.x + ptSize.x ) * virtualX;
  vertData[3].position.y  = m_Canvas.Top + ( ptPos.y + ptSize.y ) * virtualY;
  vertData[3].position.z  = ptPos.z;
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = Color4;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CDX9RenderClass::RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
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
  fX1 += m_DisplayOffset.x;
  fY1 += m_DisplayOffset.y;
  fX2 += m_DisplayOffset.x;
  fY2 += m_DisplayOffset.y;
  fX3 += m_DisplayOffset.x;
  fY3 += m_DisplayOffset.y;
  fX4 += m_DisplayOffset.x;
  fY4 += m_DisplayOffset.y;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  CUSTOMVERTEX          vertData[4];

  float   fRHW = 1.0f;

  GR::tVector   ptPos( fX1, fY1, fZ );

  m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position    = D3DXVECTOR3( m_Canvas.Left + ptPos.x * virtualX, m_Canvas.Top + ptPos.y * virtualY, ptPos.z );
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = m_Canvas.Left + fX2 * virtualX;
  vertData[1].position.y  = m_Canvas.Top + fY2 * virtualY;
  vertData[1].position.z  = ptPos.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position.x  = m_Canvas.Left + fX3 * virtualX;
  vertData[2].position.y  = m_Canvas.Top + fY3 * virtualY;
  vertData[2].position.z  = ptPos.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position.x  = m_Canvas.Left + fX4 * virtualX;
  vertData[3].position.y  = m_Canvas.Top + fY4 * virtualY;
  vertData[3].position.z  = ptPos.z;
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = Color4;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CDX9RenderClass::RenderTriangle2d( const GR::tPoint& pt1, const GR::tPoint& pt2, const GR::tPoint& pt3, 
                                        GR::f32 fTU1, GR::f32 fTV1,
                                        GR::f32 fTU2, GR::f32 fTV2,
                                        GR::f32 fTU3, GR::f32 fTV3,
                                        GR::u32 Color1, GR::u32 Color2, 
                                        GR::u32 Color3, float fZ )
{

  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  CUSTOMVERTEX          vertData[3];

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  float   fRHW = 1.0f;

  m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position    = D3DXVECTOR3( m_Canvas.Left + (float)( m_DisplayOffset.x + pt1.x ) * virtualX + m_DirectTexelMappingOffset.x, m_Canvas.Top + (float)( m_DisplayOffset.y + pt1.y ) * virtualY + m_DirectTexelMappingOffset.y, fZ );
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = D3DXVECTOR3( m_Canvas.Left + (float)( m_DisplayOffset.x + pt2.x ) * virtualX + m_DirectTexelMappingOffset.x, m_Canvas.Top + (float)( m_DisplayOffset.y + pt2.y ) * virtualY + m_DirectTexelMappingOffset.y, fZ );
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position    = D3DXVECTOR3( m_Canvas.Left + (float)( m_DisplayOffset.x + pt3.x ) * virtualX + m_DirectTexelMappingOffset.x, m_Canvas.Top + (float)( m_DisplayOffset.y + pt3.y ) * virtualY + m_DirectTexelMappingOffset.y, fZ );
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void CDX9RenderClass::RenderTriangle( const GR::tVector& ptPos1,
                                      const GR::tVector& ptPos2,
                                      const GR::tVector& ptPos3,
                                      GR::f32 fTU1, GR::f32 fTV1,
                                      GR::f32 fTU2, GR::f32 fTV2,
                                      GR::f32 fTU3, GR::f32 fTV3,
                                      GR::u32 Color1, GR::u32 Color2, 
                                      GR::u32 Color3 )
{

  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  struct CUSTOMVERTEX
  {
    GR::tVector   position; // The position
    GR::tVector   normal; // The position
    D3DCOLOR      color;    // The color
    float         fTU,
                  fTV;
  };

  CUSTOMVERTEX          vertData[3];

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );

  normal.normalize();

  m_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

  vertData[0].position    = ptPos1;
  vertData[0].color       = Color1;
  vertData[0].normal      = normal;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = ptPos2;
  vertData[1].color       = Color2;
  vertData[1].normal      = normal;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position    = ptPos3;
  vertData[2].color       = Color3;
  vertData[2].normal      = normal;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CDX9RenderClass::RenderQuad( const GR::tVector& ptPos1,
                                  const GR::tVector& ptPos2,
                                  const GR::tVector& ptPos3,
                                  const GR::tVector& ptPos4,
                                  GR::f32 fTU1, GR::f32 fTV1,
                                  GR::f32 fTU2, GR::f32 fTV2,
                                  GR::u32 Color1, GR::u32 Color2, 
                                  GR::u32 Color3, GR::u32 Color4 )
{

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
    GR::tVector   position; // The position
    GR::tVector   normal; // The position
    D3DCOLOR      color;    // The color
    float         fTU,
                  fTV;
  };

  CUSTOMVERTEX          vertData[4];

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );

  normal.normalize();

  m_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

  vertData[0].position    = ptPos1;
  vertData[0].color       = Color1;
  vertData[0].normal      = normal;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = ptPos2;
  vertData[1].color       = Color2;
  vertData[1].normal      = normal;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV1;

  vertData[2].position    = ptPos3;
  vertData[2].color       = Color3;
  vertData[2].normal      = normal;
  vertData[2].fTU         = fTU1;
  vertData[2].fTV         = fTV2;

  vertData[3].position    = ptPos4;
  vertData[3].color       = Color4;
  vertData[3].normal      = normal;
  vertData[3].fTU         = fTU2;
  vertData[3].fTV         = fTV2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CDX9RenderClass::RenderQuad( const GR::tVector& ptPos1,
                                  const GR::tVector& ptPos2,
                                  const GR::tVector& ptPos3,
                                  const GR::tVector& ptPos4,
                                  GR::f32 fTU1, GR::f32 fTV1,
                                  GR::f32 fTU2, GR::f32 fTV2,
                                  GR::f32 fTU3, GR::f32 fTV3,
                                  GR::f32 fTU4, GR::f32 fTV4,
                                  GR::u32 Color1, GR::u32 Color2, 
                                  GR::u32 Color3, GR::u32 Color4 )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
    GR::tVector   position; // The position
    GR::tVector   normal; // The position
    D3DCOLOR      color;    // The color
    float         fTU,
                  fTV;
  };

  CUSTOMVERTEX          vertData[4];

  GR::tVector       normal( ( ptPos2 - ptPos1 ).cross( ptPos3 - ptPos1 ) );


  m_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

  vertData[0].position    = ptPos1;
  vertData[0].color       = Color1;
  vertData[0].normal      = normal;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = ptPos2;
  vertData[1].color       = Color2;
  vertData[1].normal      = normal;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position    = ptPos3;
  vertData[2].color       = Color3;
  vertData[2].normal      = normal;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position    = ptPos4;
  vertData[3].color       = Color4;
  vertData[3].normal      = normal;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



XVertexBuffer* CDX9RenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  if ( !m_pd3dDevice )
  {
    dh::Log( "CD3DViewer::CreateVertexBuffer: no Device" );
    return NULL;
  }

  CDX9VertexBuffer*   pBuffer = new CDX9VertexBuffer( this );

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* CDX9RenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  if ( !m_pd3dDevice )
  {
    dh::Log( "CD3DViewer::CreateVertexBuffer: no Device" );
    return NULL;
  }

  CDX9VertexBuffer*   pBuffer = new CDX9VertexBuffer( this );
  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* CDX9RenderClass::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat )
{
  XVertexBuffer*    pBuffer = CreateVertexBuffer( MeshObject.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( MeshObject );

  return pBuffer;
}



void CDX9RenderClass::DestroyVertexBuffer( XVertexBuffer* pVBBuffer )
{

  if ( pVBBuffer == NULL )
  {
    return;
  }

  pVBBuffer->Release();

  m_VertexBuffers.remove( pVBBuffer );
  delete pVBBuffer;

}



bool CDX9RenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{

  if ( ( !m_pd3dDevice )
  ||   ( !m_bReady ) )
  {
    return false;
  }

  if ( pBuffer == NULL )
  {
    return false;
  }

  return ( (CDX9VertexBuffer*)pBuffer )->Display( Index, Count );

}



LPDIRECT3DDEVICE9 CDX9RenderClass::Device()
{

  return m_pd3dDevice;

}



void CDX9RenderClass::SetTexture( GR::u32 Stage, XTexture* pTexture )
{

  m_pd3dDevice->SetTexture( Stage, pTexture ? ( (CDX9Texture*)pTexture )->m_Surface : NULL );

}



bool CDX9RenderClass::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
{

  tMapRenderStates::iterator    it( m_RenderStates.find( std::make_pair( rState, Stage ) ) );
  if ( it != m_RenderStates.end() )
  {
    if ( it->second == rValue )
    {
      return true;
    }
  }

  HRESULT   hRes = E_FAIL;

  switch ( rState )
  {
    case RS_FOG_COLOR:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, rValue );
      break;
    case RS_FOG_DENSITY:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGDENSITY, rValue );
      break;
    case RS_FOG_START:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGSTART, rValue );
      break;
    case RS_FOG_END:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGEND, rValue );
      break;
    case RS_FOG_ENABLE:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
      }
      break;
    case RS_FOG_TABLEMODE:
      if ( rValue == RSV_FOG_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
      }
      else if ( rValue == RSV_FOG_EXP )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_EXP );
      }
      else if ( rValue == RSV_FOG_EXP_SQUARED )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_EXP2 );
      }
      else if ( rValue == RSV_FOG_LINEAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
      }
      break;
    case RS_FOG_VERTEXMODE:
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
      }
      break;
    case RS_COLORMASK:
      {
        DWORD   Flags = 0;

        if ( rValue & RSV_COLORMASK_RED )
        {
          Flags |= D3DCOLORWRITEENABLE_RED;
        }
        if ( rValue & RSV_COLORMASK_GREEN )
        {
          Flags |= D3DCOLORWRITEENABLE_GREEN;
        }
        if ( rValue & RSV_COLORMASK_BLUE )
        {
          Flags |= D3DCOLORWRITEENABLE_BLUE;
        }
        if ( rValue & RSV_COLORMASK_ALPHA )
        {
          Flags |= D3DCOLORWRITEENABLE_ALPHA;
        }
        hRes = m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, Flags );
      }
      break;
    case RS_FILL_MODE:
      if ( rValue == RSV_FILL_SOLID )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
      }
      else if ( rValue == RSV_FILL_POINTS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_POINT );
      }
      else if ( rValue == RSV_FILL_WIREFRAME )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
      }
      break;
    case RS_SHADE_MODE:
      if ( rValue == RSV_SHADE_FLAT )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
      }
      else if ( rValue == RSV_SHADE_GOURAUD )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
      }
      break;
    case RS_TEXTURE_FACTOR:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, rValue );
      break;
    case RS_NORMALIZE_NORMALS:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE );
      }
      break;
    case RS_DIFFUSE_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_EMISSIVE_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_AMBIENT_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_SPECULAR_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_TEXTURE_TRANSFORM:
      if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
      }
      else if ( rValue == RSV_TEXTURE_TRANSFORM_COUNT2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
      }
      break;
    case RS_ADDRESSU:
      if ( rValue == RSV_ADDRESS_WRAP )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
      }
      break;
    case RS_ADDRESSV:
      if ( rValue == RSV_ADDRESS_WRAP )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
      }
      break;
    case RS_ZWRITE:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
      }
      break;
    case RS_SPECULAR:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
      }
      break;
    case RS_DITHERING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );
      }
      break;
    case RS_CLEAR_COLOR:
      m_ClearColor = rValue;
      hRes = S_OK;
      break;
    case RS_LIGHT:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->LightEnable( Stage, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->LightEnable( Stage, FALSE );
      }
      break;
    case RS_MINFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MINFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MAGFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MIPFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( Stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_ZBUFFER:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
      }
      break;
    case RS_LIGHTING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
      }
      break;
    case RS_AMBIENT:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, rValue );
      break;
    case RS_COLOR_OP:
      if ( rValue == RSV_MODULATE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_BLENDFACTORALPHA )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
      }
      break;
    case RS_COLOR_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG1, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG1, D3DTA_TFACTOR );
      }
      break;
    case RS_COLOR_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG2, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG2, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_COLORARG2, D3DTA_TFACTOR );
      }
      break;
    case RS_ALPHA_OP:
      if ( rValue == RSV_MODULATE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_BLENDFACTORALPHA )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
      }
      break;
    case RS_ALPHA_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
      }
      break;
    case RS_ALPHA_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
      }
      break;
    case RS_SRC_BLEND:
      if ( rValue == RSV_SRC_ALPHA )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
      }
      else if ( rValue == RSV_ONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
      }
      else if ( rValue == RSV_ZERO )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
      }
      break;
    case RS_DEST_BLEND:
      if ( rValue == RSV_INV_SRC_ALPHA)
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
      }
      else if ( rValue == RSV_ONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
      }
      else if ( rValue == RSV_ZERO )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
      }
      break;
    case RS_CULLMODE:
      if ( rValue == RSV_CULL_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
      }
      else if ( rValue == RSV_CULL_CCW )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
      }
      else if ( rValue == RSV_CULL_CW )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
      }
      break;
    case RS_ALPHATEST:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
      }
      break;
    case RS_ALPHABLENDING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
      }
      break;
    case RS_ALPHAREF:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, rValue );
      break;
    case RS_ALPHAFUNC:
      if ( rValue == RSV_COMPARE_GREATEREQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
      }
      else if ( rValue == RSV_COMPARE_GREATER )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
      }
      else if ( rValue == RSV_COMPARE_EQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_EQUAL );
      }
      else if ( rValue == RSV_COMPARE_ALWAYS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_ALWAYS );
      }
      else if ( rValue == RSV_COMPARE_LESS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESS );
      }
      else if ( rValue == RSV_COMPARE_LESSEQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );
      }
      else if ( rValue == RSV_COMPARE_NEVER )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_NEVER );
      }
      else if ( rValue == RSV_COMPARE_NOTEQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
      }
      break;
  }

  m_RenderStates[std::make_pair( rState, Stage )] = rValue;
  if ( hRes == E_FAIL )
  {
    dh::Log( "CDX9RenderClass::SetState not supported (%d - %d - Stage %d)", rState, rValue, Stage );
  }
  return SUCCEEDED( hRes );

}



void CDX9RenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{

  XBasicRenderer::SetTransform( tType, matTrix );

  switch ( tType )
  {
    case TT_WORLD:
      m_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&matTrix );
      break;
    case TT_VIEW:
      m_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&matTrix );
      break;
    case TT_PROJECTION:
      m_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&matTrix );
      break;
    case TT_TEXTURE_STAGE_0:
      m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, (D3DMATRIX*)&matTrix );
      break;
    case TT_TEXTURE_STAGE_1:
      m_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DMATRIX*)&matTrix );
      break;
  }

}



bool CDX9RenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{

  if ( pTexture == NULL )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: Texture was NULL" );
    return false;
  }

  CDX9Texture*    pDX9Texture = (CDX9Texture*)pTexture;

  if ( MipMapLevel >= pDX9Texture->m_StoredImageData.size() )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, pDX9Texture->m_MipMapLevels );
    return false;
  }

  std::list<GR::Graphic::ImageData>::iterator   it( pDX9Texture->m_StoredImageData.begin() );
  std::advance( it, MipMapLevel );

  ImageData = *it;
  return true;

}



bool CDX9RenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{

  if ( pTexture == NULL )
  {
    dh::Error( "CopyDataToTexture: Texture was NULL" );
    return false;
  }

  CDX9Texture*    pDX9Texture = (CDX9Texture*)pTexture;

  if ( MipMapLevel >= pDX9Texture->m_MipMapLevels )
  {
    dh::Error( "CopyDataToTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, ( (CDX9Texture*)pTexture )->m_MipMapLevels );
    return false;
  }

  D3DLOCKED_RECT      lockRect;

  if ( FAILED( pDX9Texture->m_Surface->LockRect( MipMapLevel, &lockRect, NULL, 0 ) ) )
  {
    dh::Log( "CopyDataToTexture LockRect failed" );
    return false;
  }

  GR::u32       Width   = pTexture->m_SurfaceSize.x;
  GR::u32       Height  = pTexture->m_SurfaceSize.y;

  GR::u32       CurLevel = MipMapLevel;

  while ( CurLevel >= 1 )
  {
    Width /= 2;
    if ( Width == 0 )
    {
      Width = 1;
    }
    Height /= 2;
    if ( Height == 0 )
    {
      Height = 1;
    }
    --CurLevel;
  }

  if ( ( ImageData.Width() > (int)Width )
  ||   ( ImageData.Height() > (int)Height ) )
  {
    pDX9Texture->m_Surface->UnlockRect( MipMapLevel );
    dh::Error( "CopyDataToTexture: Sizes mismatching %dx%d != %dx%d", ImageData.Width(), ImageData.Height(), Width, Height );
    return false;
  }

  GR::Graphic::ContextDescriptor      cdTexture;
  
  cdTexture.Attach( Width, Height, lockRect.Pitch, pTexture->m_ImageFormat, lockRect.pBits );

  if ( !ImageData.ConvertInto( &cdTexture, ColorKey != 0, ColorKey ) )
  {
    pDX9Texture->m_Surface->UnlockRect( MipMapLevel );
    dh::Error( "CopyDataToTexture ConvertInto failed" );
    return false;
  }

  pDX9Texture->m_Surface->UnlockRect( MipMapLevel );

  // store copied data
  while ( pDX9Texture->m_StoredImageData.size() <= MipMapLevel )
  {
    pDX9Texture->m_StoredImageData.push_back( GR::Graphic::ImageData() );
  }
  pDX9Texture->m_StoredImageData.back() = ImageData;
  return true;

}



void CDX9RenderClass::RestoreAllTextures()
{

  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    if ( pTexture->m_LoadedFromFile.empty() )
    {
      // eine bloss erzeugte Textur
      CDX9Texture*    pDX9Texture = (CDX9Texture*)pTexture;

      // sicher gehen
      pDX9Texture->Release();
        
      if ( SUCCEEDED( m_pd3dDevice->CreateTexture( 
                      pTexture->m_SurfaceSize.x,
                      pTexture->m_SurfaceSize.y,
                      pDX9Texture->m_MipMapLevels,
                      0,
                      ( (CDX9Texture*)pTexture )->m_d3dfPixelFormat,
                      D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                      &( (CDX9Texture*)pTexture )->m_Surface,
                      NULL ) ) )
      {
      }
      else
      {
        dh::Log( "CreateTexture failed" );
      }
      GR::u32     MipMapLevel = 0;
      std::list<GR::Graphic::ImageData>::iterator   itID( pDX9Texture->m_StoredImageData.begin() );
      while ( itID != pDX9Texture->m_StoredImageData.end() )
      {
        CopyDataToTexture( pDX9Texture, *itID, 0, MipMapLevel );

        ++itID;
        ++MipMapLevel;
      }
    }
    else
    {
      if ( m_pEnvironment == NULL )
      {
        return;
      }
      ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
      if ( pManager == NULL )
      {
        return;
      }
      GR::Graphic::ImageData*   pData = pManager->LoadData( pTexture->m_LoadedFromFile.c_str() );
      if ( pData )
      {
        pData->ConvertSelfTo( pTexture->m_ImageFormat, 0, pTexture->m_ColorKey != 0, pTexture->m_ColorKey, 0, 0, 0, 0, pTexture->m_ColorKeyReplacementColor );

        pTexture->Release();

        if ( SUCCEEDED( m_pd3dDevice->CreateTexture( 
                        pTexture->m_SurfaceSize.x,
                        pTexture->m_SurfaceSize.y,
                        ( (CDX9Texture*)pTexture )->m_MipMapLevels,
                        0,
                        ( (CDX9Texture*)pTexture )->m_d3dfPixelFormat,
                        D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                        &( (CDX9Texture*)pTexture )->m_Surface,
                        NULL ) ) )
        {
          CopyDataToTexture( pTexture, *pData );

          // Mipmap-Levels einlesen
          std::list<GR::String>::iterator    it( ( (CDX9Texture*)pTexture )->FileNames.begin() );

          GR::u32     Level = 1;

          while ( it != ( (CDX9Texture*)pTexture )->FileNames.end() )
          {
            GR::String&  strPath( *it );

            GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)( (CDX9Texture*)pTexture )->m_ImageFormat, ( (CDX9Texture*)pTexture )->m_ColorKey, pTexture->m_ColorKeyReplacementColor );
            if ( pData == NULL )
            {
              dh::Error( "DX9Renderer:: RestoreAllTextures failed to load MipMap (%s)", strPath.c_str() );
            }
            else
            {
              CopyDataToTexture( pTexture, *pData, ( (CDX9Texture*)pTexture )->m_ColorKey, Level );
              delete pData;
            }
            ++it;
            ++Level;
          }
        }
        delete pData;
      }
    }

    ++it;
  }

}



bool CDX9RenderClass::OnResized()
{
  return ResizeSurface();
}



bool CDX9RenderClass::SetViewport( const XViewport& Viewport )
{
  D3DVIEWPORT9    viewPort;

  viewPort.Width    = Viewport.Width;
  viewPort.Height   = Viewport.Height;
  viewPort.X        = Viewport.X;
  viewPort.Y        = Viewport.Y;
  viewPort.MinZ     = Viewport.MinZ;
  viewPort.MaxZ     = Viewport.MaxZ;

  if ( viewPort.X >= Width() )
  {
    viewPort.X = Width();
    viewPort.Width = 0;
  }
  if ( viewPort.Y >= Height() )
  {
    viewPort.Y = Height();
    viewPort.Height = 0;
  }
  if ( viewPort.X + viewPort.Width > Width() )
  {
    viewPort.Width = Width() - viewPort.X;
  }
  if ( viewPort.Y + viewPort.Height> Height() )
  {
    viewPort.Height = Height() - viewPort.Y;
  }

  if ( FAILED( m_pd3dDevice->SetViewport( &viewPort ) ) )
  {
    return false;
  }
  m_ViewPort.X        = viewPort.X;
  m_ViewPort.Y        = viewPort.Y;
  m_ViewPort.Width    = viewPort.Width;
  m_ViewPort.Height   = viewPort.Height;
  m_ViewPort.MinZ     = viewPort.MinZ;
  m_ViewPort.MaxZ     = viewPort.MaxZ;
  return true;
}



bool CDX9RenderClass::SetTrueViewport( const XViewport& Viewport )
{
  D3DVIEWPORT9    viewPort;

  viewPort.Width    = Viewport.Width;
  viewPort.Height   = Viewport.Height;
  viewPort.X        = Viewport.X;
  viewPort.Y        = Viewport.Y;
  viewPort.MinZ     = Viewport.MinZ;
  viewPort.MaxZ     = Viewport.MaxZ;

  if ( viewPort.X >= Width() )
  {
    viewPort.X = Width();
    viewPort.Width = 0;
  }
  if ( viewPort.Y >= Height() )
  {
    viewPort.Y = Height();
    viewPort.Height = 0;
  }
  if ( viewPort.X + viewPort.Width > Width() )
  {
    viewPort.Width = Width() - viewPort.X;
  }
  if ( viewPort.Y + viewPort.Height> Height() )
  {
    viewPort.Height = Height() - viewPort.Y;
  }

  if ( FAILED( m_pd3dDevice->SetViewport( &viewPort ) ) )
  {
    return false;
  }
  m_ViewPort.X        = viewPort.X;
  m_ViewPort.Y        = viewPort.Y;
  m_ViewPort.Width    = viewPort.Width;
  m_ViewPort.Height   = viewPort.Height;
  m_ViewPort.MinZ     = viewPort.MinZ;
  m_ViewPort.MaxZ     = viewPort.MaxZ;
  return true;
}



GR::u32 CDX9RenderClass::Width()
{
  return m_RenderWidth;
}



GR::u32 CDX9RenderClass::Height()
{
  return m_RenderHeight;
}



GR::Graphic::eImageFormat CDX9RenderClass::ImageFormat()
{
  return MapFormat( m_d3dpp.BackBufferFormat );
}



bool CDX9RenderClass::IsFullscreen()
{
  return !m_bWindowed;
}



bool CDX9RenderClass::ToggleFullscreen()
{
  m_bWindowed = !m_bWindowed;

  return SUCCEEDED( Resize3DEnvironment() );
}



bool CDX9RenderClass::SetMode( XRendererDisplayMode& DisplayMode )
{
  // setting windowed mode directly
  if ( !DisplayMode.FullScreen )
  {
    m_bWindowed = !DisplayMode.FullScreen;

    return SUCCEEDED( Resize3DEnvironment() );
  }

  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&   DispMode = *it;
    if ( DispMode.FormatIsEqualTo( DisplayMode ) )
    {
      // der Mode existiert
      m_bWindowed = !DisplayMode.FullScreen;
      if ( !m_bWindowed )
      {
        m_FullscreenDisplayMode = DisplayMode;
      }

      return SUCCEEDED( Resize3DEnvironment() );
    }

    ++it;
  }
  dh::Log( "CDX9RenderClass::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.ImageFormat );
  return false;
}



void CDX9RenderClass::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float fZ )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }
  struct CUSTOMVERTEX
  {
    D3DXVECTOR3   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[2];

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  m_pd3dDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  D3DXVECTOR3   vectDelta( m_DirectTexelMappingOffset.x, m_DirectTexelMappingOffset.y, 0.0f );

  vertData[0].position = D3DXVECTOR3( m_Canvas.Left + (float)m_DisplayOffset.x + pt1.x * virtualX, m_Canvas.Top + (float)m_DisplayOffset.y + pt1.y * virtualY, fZ ) - vectDelta;
  vertData[0].color = Color1;

  vertData[1].position = D3DXVECTOR3( m_Canvas.Left + (float)m_DisplayOffset.x + pt2.x * virtualX, m_Canvas.Top + (float)m_DisplayOffset.y + pt2.y * virtualY, fZ ) - vectDelta;
  vertData[1].color = Color2;

  vertData[0].fRHW = vertData[1].fRHW = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void CDX9RenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
{

  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }
  struct CUSTOMVERTEX
  {
    GR::tVector       position; // The position
    D3DCOLOR          color;    // The color
  };

  CUSTOMVERTEX          vertData[2];


  m_pd3dDevice->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

  vertData[0].position = vect1;
  vertData[0].color = Color1;

  vertData[1].position = vect2;
  vertData[1].color = Color2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



bool CDX9RenderClass::SetLight( GR::u32 LightIndex, XLight& Light )
{

  if ( LightIndex >= 8 )
  {
    return false;
  }

  m_Light[LightIndex] = Light;

  D3DLIGHT9     d3dLight;

  d3dLight.Ambient.r    = ( ( Light.m_Ambient & 0x00ff0000 ) >> 16 ) / 255.0f;
  d3dLight.Ambient.g    = ( ( Light.m_Ambient & 0x0000ff00 ) >>  8 ) / 255.0f;
  d3dLight.Ambient.b    = ( ( Light.m_Ambient & 0x000000ff )       ) / 255.0f;
  d3dLight.Ambient.a    = ( ( Light.m_Ambient & 0xff000000 ) >> 24 ) / 255.0f;

  d3dLight.Diffuse.r    = ( ( Light.m_Diffuse & 0x00ff0000 ) >> 16 ) / 255.0f;
  d3dLight.Diffuse.g    = ( ( Light.m_Diffuse & 0x0000ff00 ) >>  8 ) / 255.0f;
  d3dLight.Diffuse.b    = ( ( Light.m_Diffuse & 0x000000ff )       ) / 255.0f;
  d3dLight.Diffuse.a    = ( ( Light.m_Diffuse & 0xff000000 ) >> 24 ) / 255.0f;

  d3dLight.Specular.r   = ( ( Light.m_Specular & 0x00ff0000 ) >> 16 ) / 255.0f;
  d3dLight.Specular.g   = ( ( Light.m_Specular & 0x0000ff00 ) >>  8 ) / 255.0f;
  d3dLight.Specular.b   = ( ( Light.m_Specular & 0x000000ff )       ) / 255.0f;
  d3dLight.Specular.a   = ( ( Light.m_Specular & 0xff000000 ) >> 24 ) / 255.0f;

  d3dLight.Attenuation0 = Light.m_Attenuation0;
  d3dLight.Attenuation1 = Light.m_Attenuation1;
  d3dLight.Attenuation2 = Light.m_Attenuation2;
  d3dLight.Direction    = D3DXVECTOR3( Light.m_Direction.x, Light.m_Direction.y, Light.m_Direction.z );
  d3dLight.Falloff      = Light.m_Falloff;
  d3dLight.Phi          = Light.m_Phi;
  d3dLight.Position     = D3DXVECTOR3( Light.m_Position.x, Light.m_Position.y, Light.m_Position.z );
  d3dLight.Range        = Light.m_Range;
  d3dLight.Theta        = Light.m_Theta;

  switch ( Light.m_Type )
  {
    case XLight::LT_DIRECTIONAL:
      d3dLight.Type = D3DLIGHT_DIRECTIONAL;
      break;
    case XLight::LT_POINT:
      d3dLight.Type = D3DLIGHT_POINT;
      break;
    case XLight::LT_SPOT:
      d3dLight.Type = D3DLIGHT_SPOT;
      break;
  }

  return SUCCEEDED( m_pd3dDevice->SetLight( LightIndex, &d3dLight ) );

}



bool CDX9RenderClass::SetMaterial( const XMaterial& Material )
{
  D3DMATERIAL9      mat;

  mat.Power     = Material.Power;

  mat.Ambient.r    = ( ( Material.Ambient & 0x00ff0000 ) >> 16 ) / 255.0f;
  mat.Ambient.g    = ( ( Material.Ambient & 0x0000ff00 ) >>  8 ) / 255.0f;
  mat.Ambient.b    = ( ( Material.Ambient & 0x000000ff )       ) / 255.0f;
  mat.Ambient.a    = ( ( Material.Ambient & 0xff000000 ) >> 24 ) / 255.0f;
  mat.Diffuse.r    = ( ( Material.Diffuse & 0x00ff0000 ) >> 16 ) / 255.0f;
  mat.Diffuse.g    = ( ( Material.Diffuse & 0x0000ff00 ) >>  8 ) / 255.0f;
  mat.Diffuse.b    = ( ( Material.Diffuse & 0x000000ff )       ) / 255.0f;
  mat.Diffuse.a    = ( ( Material.Diffuse & 0xff000000 ) >> 24 ) / 255.0f;
  mat.Specular.r   = ( ( Material.Specular & 0x00ff0000 ) >> 16 ) / 255.0f;
  mat.Specular.g   = ( ( Material.Specular & 0x0000ff00 ) >>  8 ) / 255.0f;
  mat.Specular.b   = ( ( Material.Specular & 0x000000ff )       ) / 255.0f;
  mat.Specular.a   = ( ( Material.Specular & 0xff000000 ) >> 24 ) / 255.0f;
  mat.Emissive.r   = ( ( Material.Emissive & 0x00ff0000 ) >> 16 ) / 255.0f;
  mat.Emissive.g   = ( ( Material.Emissive & 0x0000ff00 ) >>  8 ) / 255.0f;
  mat.Emissive.b   = ( ( Material.Emissive & 0x000000ff )       ) / 255.0f;
  mat.Emissive.a   = ( ( Material.Emissive & 0xff000000 ) >> 24 ) / 255.0f;

  m_Material = Material;

  return SUCCEEDED( m_pd3dDevice->SetMaterial( &mat ) );
}



LRESULT CALLBACK CDX9RenderClass::DX9Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return g_pDX9Instance->WindowProc( hWnd, uMsg, wParam, lParam );
}



LRESULT CALLBACK CDX9RenderClass::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT:
      // Handle paint messages when the app is not ready
      if ( ( m_pd3dDevice )
      &&   ( !m_bReady ) )
      {
        if ( m_bWindowed )
        {
          m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
          return 0;
        }
      }
      break;
    case WM_DESTROY:
      Cleanup3DEnvironment();
      break;
  }

  return ::DefWindowProc( hWnd, uMsg, wParam, lParam );

}



void CDX9RenderClass::DoSaveSnapShot( const char* szFile )
{

  IDirect3DSurface9   *pSourceSurface,
                      *pDestSurface;

  HRESULT   hRes;

  hRes = m_pd3dDevice->GetBackBuffer( 0,
                                      0,
                                      D3DBACKBUFFER_TYPE_MONO,
                                      &pSourceSurface );
  if ( !SUCCEEDED( hRes ) )
  {
    return;
  }

  D3DSURFACE_DESC   ddsd;

  hRes = pSourceSurface->GetDesc( &ddsd );

  if ( !SUCCEEDED( hRes ) )
  {
    pSourceSurface->Release();
    return;
  }

  hRes = m_pd3dDevice->CreateOffscreenPlainSurface( ddsd.Width,
                                                    ddsd.Height,
                                                    ddsd.Format,
                                                    D3DPOOL_SCRATCH,
                                                    &pDestSurface,
                                                    NULL );

  if ( !SUCCEEDED( hRes ) )
  {
    pSourceSurface->Release();
    return;
  }

  hRes = m_pd3dDevice->StretchRect( pSourceSurface,
                                    NULL,
                                    pDestSurface,
                                    NULL,
                                    D3DTEXF_NONE );

  pSourceSurface->Release();

  if ( !SUCCEEDED( hRes ) )
  {
    pDestSurface->Release();
    return;
  }

  D3DLOCKED_RECT    lockRect;

  hRes = pDestSurface->LockRect( &lockRect,
                                 NULL,
                                 D3DLOCK_READONLY );

  int             j;

  BOOL            bRLE;

  unsigned char   ucHeader[18];

  WORD            wWidth,
                  wHeight;

  GR::IO::FileStream       File;


  bRLE = FALSE;
  
  if ( !File.Open( szFile, IIOStream::OT_WRITE_ONLY ) )
  {
    pDestSurface->UnlockRect();
    pDestSurface->Release();

    return;
  }

  // Header zusammenbauen
  ZeroMemory( ucHeader, 18 );

  ucHeader[0] = 0;
  ucHeader[1] = 0;
  ucHeader[2] = 2;

  ucHeader[10] = ddsd.Height % 256;
  ucHeader[11] = ddsd.Height / 256;

  ucHeader[12] = ddsd.Width % 256;
  ucHeader[13] = ddsd.Width / 256;
  ucHeader[14] = ddsd.Height % 256;
  ucHeader[15] = ddsd.Height / 256;
  ucHeader[16] = 24; 

  File.WriteBlock( &ucHeader, 18 );


  // ID-Feld überlesen
  File.SetPosition( ucHeader[0], IIOStream::PT_CURRENT );

  // ein umkomprimiertes Bild
  wHeight = ddsd.Height;
  wWidth = ddsd.Width;

  DWORD   Pixel,
          *pData = (DWORD*)lockRect.pBits;
  WORD    wPixel;
  for ( j = wHeight - 1; j >= 0; j-- )
  {
    switch ( ddsd.Format )
    {
      case D3DFMT_X1R5G5B5:
        {
          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            wPixel = ( (WORD*)lockRect.pBits )[i + j * lockRect.Pitch / 2];

            File.WriteU8( ( ( wPixel & 0x001f )       ) * 255 / 31 );
            File.WriteU8( ( ( wPixel & 0x03e0 ) >> 5  ) * 255 / 31 );
            File.WriteU8( ( ( wPixel & 0x7c00 ) >> 10 ) * 255 / 31 );
          }
        }
        break;
      case D3DFMT_R5G6B5:
        {
          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            wPixel = ( (WORD*)lockRect.pBits )[i + j * lockRect.Pitch / 2];

            File.WriteU8( ( ( wPixel & 0x001f )       ) * 255 / 31 );
            File.WriteU8( ( ( wPixel & 0x07e0 ) >> 5  ) * 255 / 63 );
            File.WriteU8( ( ( wPixel & 0xf800 ) >> 11 ) * 255 / 31 );
          }
        }
        break;
      case D3DFMT_X8R8G8B8:
        {
          pData = (DWORD*)( (GR::u8*)lockRect.pBits + lockRect.Pitch * j );
          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &Pixel, pData, 3 );

            File.WriteU8( unsigned char( ( Pixel & 0x0000ff )       ) );
            File.WriteU8( unsigned char( ( Pixel & 0x00ff00 ) >> 8  ) );
            File.WriteU8( unsigned char( ( Pixel & 0xff0000 ) >> 16 ) );

            ++pData;
          }
          //pData += lockRect.Pitch / 4 - ddsd.Width;
        }
        break;
      case D3DFMT_R8G8B8:
        {
          for ( DWORD i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &Pixel, pData, 3 );

            File.WriteU8( (BYTE)( ( Pixel & 0xff0000 ) >> 16 ) );
            File.WriteU8( (BYTE)( ( Pixel & 0x00ff00 ) >> 8  ) );
            File.WriteU8( (BYTE)( ( Pixel & 0x0000ff )       ) );

            pData = (DWORD*)( ( (GR::u8*)pData ) + 3 );
          }
          pData = (DWORD*)( ( (GR::u8*)pData ) + lockRect.Pitch - ddsd.Width / 3 );
        }
        break;
    }
  }
  File.Close();

  pDestSurface->UnlockRect();
  pDestSurface->Release();

}



bool CDX9RenderClass::SaveScreenShot( const char* szFileName )
{

  if ( szFileName )
  {
    m_strSaveScreenShotFileName = szFileName;
    return true;
  }

  return false;

}



bool CDX9RenderClass::RenderMesh( const Mesh::IMesh& Mesh )
{
  CDX9VertexBuffer   vbTemp( this );

  vbTemp.Create( Mesh.FaceCount() * 3, XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD, XVertexBuffer::PT_TRIANGLE );

  vbTemp.FillFromMesh( Mesh );

  return RenderVertexBuffer( &vbTemp );
}



bool CDX9RenderClass::VSyncEnabled()
{
  return m_VSyncEnabled;
}



void CDX9RenderClass::EnableVSync( bool Enable )
{
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
    Resize3DEnvironment();
  }
}



void CDX9RenderClass::SetRenderTarget( XTexture* pTexture )
{
}

