#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include <Interface/IAssetLoader.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <Misc/Misc.h>

#include <Xtreme/Environment/XWindow.h>

#include <String/XML.h>

#include <IO/FileStream.h>

#include "DX8RenderClass.h"
#include "DX8Texture.h"
#include "DX8VertexBuffer.h"

#undef D3D_SDK_VERSION
#include <d3dx8.h>


#pragma comment ( lib, "d3dx8.lib" )
#pragma comment ( lib, "d3d8.lib" )
#pragma comment ( lib, "dxguid.lib" )

CDX8RenderClass* CDX8RenderClass::g_pDX8Instance = NULL;


CDX8RenderClass::CDX8RenderClass( HINSTANCE hInstance ) :
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
  m_pBackBufferTargetSurface( NULL ),
  m_pBackBufferDepthStencilSurface( NULL ),
  m_pCurrentlySetTargetSurface( NULL ),
  m_pCurrentlySetTargetTexture( NULL )
{
  if ( m_hInstance == NULL )
  {
    m_hInstance = GetModuleHandle( NULL );
  }
  m_DirectTexelMappingOffset.Set( -0.5f, -0.5f );
  g_pDX8Instance = this;
}



CDX8RenderClass::~CDX8RenderClass()
{
  Release();
  g_pDX8Instance = NULL;
}



bool CDX8RenderClass::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment )
{
  m_pEnvironment = &Environment;

  m_pDebugger = (IDebugService*)m_pEnvironment->Service( "Logger" );

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

  HRESULT hr;

  // Create the Direct3D object
  m_pD3D = Direct3DCreate8( 220 ); //D3D_SDK_VERSION = 220 für DX8(.1?)
  if ( m_pD3D == NULL )
  {
    Log( "Renderer.General", "Create: Direct3DCreate8 failed" );
    return false;
  }

  m_Transform[TT_WORLD].Identity();
  m_Transform[TT_VIEW].Identity();
  m_Transform[TT_PROJECTION].Identity();

  // best guess
  if ( Depth == 0 )
  {
    Depth = 32;
  }

  m_CreationWidth   = Width;
  m_CreationHeight  = Height;
  m_CreationDepth   = Depth;

  m_Canvas.Set( 0, 0, Width, Height );
  m_VirtualSize.Set( Width, Height );

  m_bWindowed         = !( Flags & XRenderer::IN_FULLSCREEN );
  m_VSyncEnabled      = !!( Flags & XRenderer::IN_VSYNC );

  // find ideal aspect ratio of monitor
  HMONITOR mon = MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
  MONITORINFO   mInfo;

  mInfo.cbSize = sizeof( mInfo );

  if ( GetMonitorInfo( mon, &mInfo ) )
  {
    m_NativeMonitorSize.Set( mInfo.rcMonitor.right - mInfo.rcMonitor.left, mInfo.rcMonitor.bottom - mInfo.rcMonitor.top );
  }
  else
  {
    // will that ever happen?
    m_NativeMonitorSize.Set( m_CreationWidth, m_CreationHeight );
  }

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
    Log( "Renderer.General", "Create: failed to build device list" );
    return false;
  }

  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Width %d", m_FullscreenDisplayMode.Width ) );
  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Height %d", m_FullscreenDisplayMode.Height ) );
  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Format %d", m_FullscreenDisplayMode.ImageFormat ) );

  if ( m_vectAdapters.empty() )
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    Log( "Renderer.General", "DX8RenderClass: No adapters available" );
    return false;
  }

  if ( !FindSuitableMode( m_CreationWidth, m_CreationHeight, m_CreationDepth, m_bWindowed ) )
  {
    // fallback - simply try to use given mode
    Log( "Renderer.General", "Create: failed to find suitable mode" );
    /*
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    return false;
    */
    m_FullscreenDisplayMode.Width       = m_CreationWidth;
    m_FullscreenDisplayMode.Height      = m_CreationHeight;
    m_FullscreenDisplayMode.ImageFormat = GR::Graphic::ImageData::ImageFormatFromDepth( Depth );
  }

  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Width %d", m_FullscreenDisplayMode.Width ) );
  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Height %d", m_FullscreenDisplayMode.Height ) );
  Log( "Renderer.Full", CMisc::printf( "FullscreenMode.Format %d", m_FullscreenDisplayMode.ImageFormat ) );

  m_Canvas.Set( 0, 0, Width, Height );

  // Unless a substitute hWnd has been specified, create a window to
  // render into
  m_hwndViewport = hWnd;
  if ( m_hwndViewport == NULL)
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    Log( "Renderer.General", "Create: no valid window handle" );
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
    Log( "Renderer.General", "Create: Initialize3DEnvironment failed" );
    return false;
  }

  // apply common settings
  SetState( XRenderer::RS_NORMALIZE_NORMALS, XRenderer::RSV_ENABLE ); // seems to be default on DX9/DX11
  SetState( XRenderer::RS_MINFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MAGFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MIPFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_FOG_ENABLE, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_FOG_TABLEMODE, XRenderer::RSV_FOG_LINEAR );
  SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_AMBIENT, 0 );


  // The app is ready to go
  m_bReady = TRUE;

  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
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
          Log( "Renderer.General", "DX8Renderer: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front().c_str(), (GR::Graphic::eImageFormat)ForcedFormat, ColorKey, MipMaps );

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
                Log( "Renderer.General", CMisc::printf( "DX8Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, ColorKey, Level );
                delete pData;
              }

              ( (CDX8Texture*)pTexture )->m_listFileNames.push_back( strPath );

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
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          Log( "Renderer.General", CMisc::printf( "DX8Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
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
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_MESH, i );

        XMesh*       pMesh = NULL;

        pMesh = CT3DMeshLoader::Load( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "File" ) ).c_str() );
        if ( pMesh )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX8Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
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
          Log( "Renderer.General", "DX8Renderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
        }
      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  return true;
}



bool CDX8RenderClass::Release()
{
  ReleaseAssets();
  if ( !m_bReady )
  {
    return true;
  }
  Cleanup3DEnvironment();
  m_pEnvironment = NULL;
  // crashed in managed environment (why??)
  //NotifyService( "Renderer", "Released" );
  return true;
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



HRESULT CDX8RenderClass::BuildDeviceList()
{
  const DWORD         NumDeviceTypes = 2;

  const TCHAR*        strDeviceDescs[] = { "HAL", "REF" };



  const D3DDEVTYPE    DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };


  BOOL                bHALExists                  = FALSE,
                      bHALIsWindowedCompatible    = FALSE,
                      bHALIsDesktopCompatible     = FALSE,
                      bHALIsSampleCompatible      = FALSE;


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

    // Enumerate all display modes on this adapter
    D3DDISPLAYMODE          modes[100];
    D3DFORMAT               formats[20];
    DWORD                   NumFormats      = 0,
                            NumModes        = 0,
                            NumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter );


    // Add the adapter's current desktop format to the list of formats
    //formats[NumFormats++] = Adapter.d3ddmDesktop.Format;

    for ( UINT iMode = 0; iMode < NumAdapterModes; iMode++ )
    {
      // Get the display mode attributes
      D3DDISPLAYMODE      DisplayMode;

      m_pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );

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

        Log( "Renderer.Full", "DX8 - found mode %dx%dx%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.Format );

        NumModes++;

        // Check if the mode's format already exists
        DWORD f = 0;
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
    // für Referenz-Device mit 1 beginnen!
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
      BOOL        bFormatConfirmed[20];

      DWORD       Behavior[20];

      D3DFORMAT   fmtDepthStencil[20];


      Log( "Renderer.Full", "DX8 - found %d formats in Device %s", NumFormats, Device.strName.c_str() );

      for ( DWORD f = 0; f < NumFormats; f++ )
      {
        bFormatConfirmed[f] = FALSE;
        fmtDepthStencil[f]  = D3DFMT_UNKNOWN;

        // Skip formats that cannot be used as render targets on this device
        if ( ( FAILED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, formats[f], formats[f], FALSE ) ) )
        &&   ( FAILED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, formats[f], D3DFMT_UNKNOWN, TRUE ) ) ) )
        {
          Log( "Renderer.Full", "DX8 - Reject format %x", formats[f] );
          continue;
        }

        if ( Device.deviceType == D3DDEVTYPE_HAL )
        {
          // This system has a HAL device
          bHALExists = TRUE;

          if ( Device.d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
          {
            // HAL can run in a window for some mode
            Log( "Renderer.Full", "DX8 - can render windowed" );
            bHALIsWindowedCompatible = TRUE;
            if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, Adapter.d3ddmDesktop.Format, formats[f], FALSE ) ) )
            {
              // HAL can run in a window for the current desktop mode
              bHALIsDesktopCompatible = TRUE;
              m_bWindowedModePossible = true;
              Device.bCanDoWindowed   = TRUE;
            }
          }
        }

        // Confirm the device/format for HW vertex processing
        if ( Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
        {
          Log( "Renderer.Full", "DX8 - can hw TL" );
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
              Log( "Renderer.Full", "DX8 - can do hw vertexprocessing" );
              bFormatConfirmed[f] = TRUE;
            }
          }

          if ( !bFormatConfirmed[f] )
          {
            Behavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

            if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, Behavior[f], formats[f] ) ) )
            {
              Log( "Renderer.Full", "DX8 - can do mixed vertexprocessing" );
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
            Log( "Renderer.Full", "DX8 - can do sw vertexprocessing" );
            bFormatConfirmed[f] = TRUE;
          }
        }

        // Find a suitable depth/stencil buffer format for this device/format
        if ( ( bFormatConfirmed[f] )
        &&   ( m_bUseDepthBuffer ) )
        {
          if ( !FindDepthStencilFormat( iAdapter, Device.deviceType, formats[f], &fmtDepthStencil[f] ) )
          {
            Log( "Renderer.Full", "DX8 - can't do depth/stencil format" );
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
              tD3DModeInfo   modeInfo;


              modeInfo.Width              = modes[m].Width;
              modeInfo.Height             = modes[m].Height;
              modeInfo.PixelFormat        = modes[m].Format;
              modeInfo.Behaviour          = Behavior[f];
              modeInfo.DepthStencilFormat = fmtDepthStencil[f];

              Log( "Renderer.Full", "Possible Mode %dx%dx%d", modeInfo.Width, modeInfo.Height, modeInfo.PixelFormat );

              Device.m_vectMode.push_back( modeInfo );

              m_DisplayModes.push_back( XRendererDisplayMode( modeInfo.Width, modeInfo.Height, MapFormat( modeInfo.PixelFormat ) ) );
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



HRESULT CDX8RenderClass::Initialize3DEnvironment()
{
  if ( m_pD3D == NULL )
  {
    return E_FAIL;
  }

  if ( m_Adapter >= m_vectAdapters.size() )
  {
    Log( "Renderer.General", 
         "[DX8][Initialize3DEnvironment] Adapter out of bounds (%d > %d)",
         m_Adapter,
         m_vectAdapters.size() );
    return E_FAIL;
  }

  HRESULT hr;


  tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_Adapter];

  if ( pAdapterInfo->CurrentDevice >= pAdapterInfo->vectDevices.size() )
  {
    Log( "Renderer.General", 
         "[DX8][Initialize3DEnvironment] Device out of bounds (%d > %d)",
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
  m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
  m_d3dpp.BackBufferCount        = 1;
  m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
  m_d3dpp.EnableAutoDepthStencil = m_bUseDepthBuffer;
  m_d3dpp.AutoDepthStencilFormat = m_pCurrentModeInfo->DepthStencilFormat;
  m_d3dpp.hDeviceWindow          = m_hwndViewport;
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

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;

    if ( m_VSyncEnabled )
    {
      m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
    }
    else
    {
      m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    }
  }
  else
  {
    m_d3dpp.BackBufferWidth  = m_pCurrentModeInfo->Width;
    m_d3dpp.BackBufferHeight = m_pCurrentModeInfo->Height;
    m_d3dpp.BackBufferFormat = m_pCurrentModeInfo->PixelFormat;
    if ( !m_VSyncEnabled )
    {
      m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.FullScreen_PresentationInterval = 0;
    }

    // Fenster anpassen
    AdjustWindowForMode( false );

    //SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    //m_hwnindowedModeParent = GetParent( m_hwndViewport );
    //SetParent( m_hwndViewport, NULL );
  }

  Log( "Renderer.Full", "PresentParameters" );
  Log( "Renderer.Full", "Windowed %d", m_d3dpp.Windowed );
  Log( "Renderer.Full", "BackBufferCount %d", m_d3dpp.BackBufferCount );
  Log( "Renderer.Full", "MultiSampleType %d", m_d3dpp.MultiSampleType );
  Log( "Renderer.Full", "SwapEffect %d", m_d3dpp.SwapEffect );
  Log( "Renderer.Full", "EnableAutoDepthStencil %d", m_d3dpp.EnableAutoDepthStencil );
  Log( "Renderer.Full", "AutoDepthStencilFormat %d", m_d3dpp.AutoDepthStencilFormat );
  Log( "Renderer.Full", "hDeviceWindow %x", m_d3dpp.hDeviceWindow );
  Log( "Renderer.Full", "BackBufferWidth %d", m_d3dpp.BackBufferWidth );
  Log( "Renderer.Full", "BackBufferHeight %d", m_d3dpp.BackBufferHeight );
  Log( "Renderer.Full", "BackBufferFormat %d", m_d3dpp.BackBufferFormat );

  if ( ( m_d3dpp.BackBufferHeight == 0 )
  ||   ( m_d3dpp.BackBufferWidth == 0 ) )
  {
    return E_FAIL;
  }

  m_Canvas.Set( 0, 0, m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight );

  // Create the device
  DWORD   Behaviour = m_pCurrentModeInfo->Behaviour;
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
    m_CreateFlags = m_pCurrentModeInfo->Behaviour;

    // Store render target surface desc
    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
    pBackBuffer->Release();

    hr = m_pd3dDevice->GetDepthStencilSurface( &m_pBackBufferDepthStencilSurface );
    if ( FAILED( hr ) )
    {
      dh::Log( "CDX8RenderClass::SetRenderTarget failed to get back depth buffer/stencil surface (%x)", hr );
      return E_FAIL;
    }

    // Initialize the app's device-dependent objects
    hr = InitDeviceObjects();
    if ( SUCCEEDED( hr ) )
    {
      m_bActive = TRUE;
      return S_OK;
    }
    else
    {
      Log( "Renderer.General", "Initialize3DEnvironment: InitDeviceObjects failed" );
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
    Log( "Renderer.General", "Initialize3DEnvironment: CreateDevice failed" );
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
        Log( "Renderer.General", "Initialize3DEnvironment: 2nd time failed" );
      }
    }
  }
  return hr;
}



void CDX8RenderClass::Cleanup3DEnvironment()
{
  m_bActive = FALSE;
  m_bReady  = FALSE;

  if ( m_pd3dDevice )
  {
    for ( GR::u32 i = 0; i < 8; ++i )
    {
      SetTexture( i, NULL );
    }
    SetRenderTarget( NULL );

    DestroyAllTextures();
    DestroyAllVertexBuffers();
    DestroyAllFonts();
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

    if ( m_pBackBufferDepthStencilSurface != NULL )
    {
      m_pBackBufferDepthStencilSurface->Release();
      m_pBackBufferDepthStencilSurface = NULL;
    }

    m_pd3dDevice->Release();
    m_pd3dDevice = NULL;
    m_pD3D->Release();

    m_pd3dDevice = NULL;
    m_pD3D       = NULL;
  }

  if ( m_hwndViewport )
  {
    //DestroyWindow( m_hwndViewport );
    m_hwndViewport = NULL;
  }

}



bool CDX8RenderClass::IsReady() const
{

  return m_bReady;

}



HRESULT CDX8RenderClass::ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT )   
{ 
  return S_OK; 
}



BOOL CDX8RenderClass::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
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



void CDX8RenderClass::SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice )
{
  if ( !pDevice ) 
  {
    Log( "Renderer.General", "no device" );
    return;
  }

  Log( "Renderer.Full", CMisc::printf( "SelectDefaultMode  Creation %dx%dx%d", m_CreationWidth, m_CreationHeight, m_CreationDepth ) );

  pDevice->CurrentMode = 0;
  for ( unsigned int m = 0; m < pDevice->m_vectMode.size(); m++ )
  {
    tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[m];

    Log( "Renderer.Full", CMisc::printf( "SelectDefaultMode  mode %dx%dx%d", ModeInfo.Width, ModeInfo.Height, ModeInfo.PixelFormat ) );

    if ( ( ModeInfo.Width == m_CreationWidth )
    &&   ( ModeInfo.Height == m_CreationHeight ) )
    {
      
      if ( ( m_CreationDepth == 24 )
      ||   ( m_CreationDepth == 32 ) )
      {
        if ( ( ModeInfo.PixelFormat == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_X8R8G8B8 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = m_CreationWidth;
          m_FullscreenDisplayMode.Height    = m_CreationHeight;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.PixelFormat );
          m_FullscreenDisplayMode.FullScreen = true;
          Log( "Renderer.Full", "Keep this mode 24/32" );
          break;
        }
      }
      if ( m_CreationDepth == 16 )
      {
        if ( ( ModeInfo.PixelFormat == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = m_CreationWidth;
          m_FullscreenDisplayMode.Height    = m_CreationHeight;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.PixelFormat );
          m_FullscreenDisplayMode.FullScreen = true;
          Log( "Renderer.Full", "Keep this mode 16" );
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
      tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[m];

      Log( "Renderer.Full", CMisc::printf( "SelectDefaultMode  mode %dx%dx%d", ModeInfo.Width, ModeInfo.Height, ModeInfo.PixelFormat ) );

      if ( ( ModeInfo.Width == m_CreationWidth )
      &&   ( ModeInfo.Height == m_CreationHeight ) )
      {
        if ( ( ModeInfo.PixelFormat == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_X8R8G8B8 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.PixelFormat == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = m_CreationWidth;
          m_FullscreenDisplayMode.Height    = m_CreationHeight;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.PixelFormat );
          m_FullscreenDisplayMode.FullScreen = true;
          Log( "Renderer.Full", "Keep this mode 16/24/32" );
          break;
        }
      }
    }
  }

  // find suitable fullscreen mode for aspect ratio
  /*
  if ( m_FullscreenDisplayMode.m_Width != ( m_NativeMonitorSize.x * m_FullscreenDisplayMode.m_Height ) / m_NativeMonitorSize.y )
  {
    dh::Log( "Fullscreen mode is not in native aspect ratio" );

    for ( unsigned int m = 0; m < pDevice->m_vectMode.size(); m++ )
    {
      tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[m];

      if ( ( ModeInfo.m_Width >= m_CreationWidth )
      &&   ( ModeInfo.m_Height >= m_CreationHeight ) )
      {
        dh::Log( "-check for mode %dx%d", ModeInfo.m_Width, ModeInfo.m_Height );
        dh::Log( " Ratio wanted = %d, actual = %d", ModeInfo.m_Width, ( m_NativeMonitorSize.x * ModeInfo.m_Height ) / m_NativeMonitorSize.y );
        if ( ( m_CreationDepth == 24 )
        ||   ( m_CreationDepth == 32 ) )
        {
          if ( ( ModeInfo.PixelFormat == D3DFMT_R8G8B8 )
          ||   ( ModeInfo.PixelFormat == D3DFMT_A8R8G8B8 )
          ||   ( ModeInfo.PixelFormat == D3DFMT_X8R8G8B8 ) )
          {
            //pDevice->CurrentMode = m;

            if ( ModeInfo.m_Width == ( m_NativeMonitorSize.x * ModeInfo.m_Height ) / m_NativeMonitorSize.y )
            {
              dh::Log( "Found best fs mode %dx%d", ModeInfo.m_Width, ModeInfo.m_Height );
              m_FullscreenDisplayMode.m_Width     = ModeInfo.m_Width;
              m_FullscreenDisplayMode.m_Height    = ModeInfo.m_Height;
              m_FullscreenDisplayMode.m_ImageFormat = MapFormat( ModeInfo.PixelFormat );
              break;
            }
          }
        }
        if ( m_CreationDepth == 16 )
        {
          if ( ( ModeInfo.PixelFormat == D3DFMT_R5G6B5 )
          ||   ( ModeInfo.PixelFormat == D3DFMT_X1R5G5B5 )
          ||   ( ModeInfo.PixelFormat == D3DFMT_A1R5G5B5 ) )
          {
            //pDevice->CurrentMode = m;

            if ( ModeInfo.m_Width == ( m_NativeMonitorSize.x * ModeInfo.m_Height ) / m_NativeMonitorSize.y )
            {
              dh::Log( "Found best fs mode %dx%d", ModeInfo.m_Width, ModeInfo.m_Height );
              m_FullscreenDisplayMode.m_Width     = ModeInfo.m_Width;
              m_FullscreenDisplayMode.m_Height    = ModeInfo.m_Height;
              m_FullscreenDisplayMode.m_ImageFormat = MapFormat( ModeInfo.PixelFormat );
              break;
            }
          }
        }
      }
    }
  }*/

  // kann im aktuellen Desktop-Modus im Fenster gerendert werden?
  if ( m_bWindowed )
  {
    if ( pDevice->CurrentMode < pDevice->m_vectMode.size() )
    {
      tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[pDevice->CurrentMode];

      if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapterNr, 
                                              pDevice->deviceType, 
                                              pAdapter->d3ddmDesktop.Format, 
                                              ModeInfo.PixelFormat,
                                              FALSE ) ) )
      {
        m_bWindowedModePossible = true;
      }
    }
  }
}



bool CDX8RenderClass::ResizeSurface()
{
  static BOOL   bInside = FALSE;

  if ( bInside )
  {
    return TRUE;
  }
  bInside = TRUE;

  if ( !IsWindow( m_hwndViewport ) )
  {
    bInside = FALSE;
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
      Log( "Renderer.General", "MsgProc: Failed to Resize3DEnvironment" );
      bInside = FALSE;
      m_bReady = FALSE;
      return FALSE;
    }
    m_bReady = TRUE;
  }
  else
  {
    //Log( "Renderer.General", "ResizeSurface d %d,%d", m_bActive, m_bWindowed );
  }

  bInside = FALSE;
  return TRUE;
}



bool CDX8RenderClass::FindSuitableMode( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool bWindowed )
{
  GR::u32     origDepth = Depth;
  int         iAdapter = 0;

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

          std::vector<tD3DModeInfo>::iterator   itMode( DeviceInfo.m_vectMode.begin() );
          while ( itMode != DeviceInfo.m_vectMode.end() )
          {
            tD3DModeInfo&   ModeInfo( *itMode );

            if ( ( bWindowed )
            ||   ( ( !bWindowed )
            &&     ( ModeInfo.Width == Width )
            &&     ( ModeInfo.Height == Height ) ) )
            {
              if ( bWindowed )
              {
                Depth = GR::Graphic::ImageData::DepthFromImageFormat( MapFormat( ModeInfo.PixelFormat ) );
              }
              else
              {
                Depth = origDepth;
              }
              if ( ( ( Depth == 24 )
              ||     ( Depth == 32 ) )
              &&   ( ModeInfo.PixelFormat == D3DFMT_R8G8B8 )
              ||   ( ModeInfo.PixelFormat == D3DFMT_A8R8G8B8 )
              ||   ( ModeInfo.PixelFormat == D3DFMT_X8R8G8B8 ) )
              {
                m_Adapter                     = iAdapter;
                AdapterInfo.CurrentDevice     = iDevice;
                DeviceInfo.CurrentMode        = iMode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.PixelFormat );
                  m_FullscreenDisplayMode.FullScreen = true;
                }
                return true;
              }
              if ( ( Depth == 16 )
              &&   ( ModeInfo.PixelFormat == D3DFMT_R5G6B5 )
              ||   ( ModeInfo.PixelFormat == D3DFMT_X1R5G5B5 )
              ||   ( ModeInfo.PixelFormat == D3DFMT_A1R5G5B5 ) )
              {
                m_Adapter                     = iAdapter;
                AdapterInfo.CurrentDevice     = iDevice;
                DeviceInfo.CurrentMode        = iMode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.PixelFormat );
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



HRESULT CDX8RenderClass::Resize3DEnvironment()
{
  if ( m_pd3dDevice == NULL )
  {
    Log( "Renderer.General", "Resize3DEnvironment: no device" );
    return E_FAIL;
  }

  HRESULT hr;

  // Release all vidmem objects
  if ( FAILED( hr = InvalidateDeviceObjects() ) )
  {
    Log( "Renderer.General", "Resize3DEnvironment: InvalidateDeviceObjects failed" );
    return hr;
  }

  SetRenderTarget( NULL );
  for ( int i = 0; i < 8; ++i )
  {
    SetTexture( i, NULL );
  }
  ReleaseAllTextures();
  ReleaseAllVertexBuffers();

  if ( m_pBackBufferDepthStencilSurface != NULL )
  {
    m_pBackBufferDepthStencilSurface->Release();
    m_pBackBufferDepthStencilSurface = NULL;
  }

  // Reset the device
  if ( ( !m_d3dpp.BackBufferWidth )
  ||   ( !m_d3dpp.BackBufferHeight ) )
  {
    //Log( "Renderer.General", "Resize3DEnvironment: Invalid BackBuffer Size (%dx%d)", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight );
    //return E_FAIL;
    return S_OK;
  }

  if ( ( m_d3dpp.Windowed )
  &&   ( !m_bWindowed ) )
  {
    // alte Fenster-Werte merken -> soll nach Fullscreen
    AdjustWindowForMode( false );

    //SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    //m_hwnindowedModeParent = GetParent( m_hwndViewport );
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
      m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
    else
    {
      m_d3dpp.FullScreen_PresentationInterval = 0;
    }
  }
  else
  {
    RECT    rc;
    GetClientRect( m_hwndViewport, &rc );

    m_d3dpp.BackBufferWidth  = rc.right - rc.left;
    m_d3dpp.BackBufferHeight = rc.bottom - rc.top;

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = m_vectAdapters[m_CurrentAdapter].d3ddmDesktop.Format;

    if ( m_VSyncEnabled )
    {
      m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
    }
    else
    {
      m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    }
  }

  Log( "Renderer.Full", "Reset with %dx%dx%d (IF %d Depth %d) (Windowed: %d)", m_d3dpp.BackBufferWidth, m_d3dpp.BackBufferHeight, m_d3dpp.BackBufferFormat,
    MapFormat( m_d3dpp.BackBufferFormat ), GR::Graphic::ImageData::DepthFromImageFormat( MapFormat( m_d3dpp.BackBufferFormat ) ), m_bWindowed );
  Log( "Renderer.Full", "Reset PresentParameters" );
  Log( "Renderer.Full", "Windowed %d", m_d3dpp.Windowed );
  Log( "Renderer.Full", "BackBufferCount %d", m_d3dpp.BackBufferCount );
  Log( "Renderer.Full", "MultiSampleType %d", m_d3dpp.MultiSampleType );
  Log( "Renderer.Full", "SwapEffect %d", m_d3dpp.SwapEffect );
  Log( "Renderer.Full", "EnableAutoDepthStencil %d", m_d3dpp.EnableAutoDepthStencil );
  Log( "Renderer.Full", "AutoDepthStencilFormat %d", m_d3dpp.AutoDepthStencilFormat );
  Log( "Renderer.Full", "hDeviceWindow %x", m_d3dpp.hDeviceWindow );
  Log( "Renderer.Full", "BackBufferWidth %d", m_d3dpp.BackBufferWidth );
  Log( "Renderer.Full", "BackBufferHeight %d", m_d3dpp.BackBufferHeight );
  Log( "Renderer.Full", "BackBufferFormat %d", m_d3dpp.BackBufferFormat );
  Log( "Renderer.Full", "Actual Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
  Log( "Renderer.Full", "Actual Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );
  Log( "Renderer.Full", "Stored Window Styles %x", m_WindowedModeStyles );
  Log( "Renderer.Full", "Stored Window ExStyles %x", m_WindowedModeExStyles );

  if ( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
  {
    if ( hr != D3DERR_DEVICENOTRESET )
    {
      Log( "Renderer.General", "TestCooperativeLevel failed" );
      return hr;
    }
    //Log( "Renderer.General", "TestCooperativeLevel -> Device not reset" );
  }
  /*
  else
  {
    dh::Log( "TestCooperativeLevel ok" );
  }
  */

  if ( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
  {
    if ( hr == D3DERR_DEVICELOST )
    {
      Log( "Renderer.General", "Device is lost, retry later" );
      return hr;
    }
    Log( "Renderer.General", "Resize3DEnvironment: m_pd3dDevice->Reset failed (%x)", hr );

    dh::Log( "D3DERR_INVALIDCALL %x", D3DERR_INVALIDCALL );
    dh::Log( "D3DERR_OUTOFVIDEOMEMORY %x", D3DERR_OUTOFVIDEOMEMORY  );
    dh::Log( "D3DERR_DEVICELOST %x", D3DERR_DEVICELOST );
    dh::Log( "D3DERR_DEVICENOTRESET %x", D3DERR_DEVICENOTRESET );
    dh::Log( "E_OUTOFMEMORY %x", E_OUTOFMEMORY );

    return hr;
  }

  // Store render target surface desc
  LPDIRECT3DSURFACE8 pBackBuffer;
  m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
  pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
  pBackBuffer->Release();

  hr = m_pd3dDevice->GetDepthStencilSurface( &m_pBackBufferDepthStencilSurface );
  if ( FAILED( hr ) )
  {
    dh::Log( "CDX8RenderClass::SetRenderTarget failed to get back depth buffer/stencil surface (%x)", hr );
    return E_FAIL;
  }


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
    //SetParent( m_hwndViewport, m_hwnindowedModeParent );
    AdjustWindowForMode( true );
  }

  /*
  dh::LogFlags( dh::LLF_GENERIC, "Nach Reset" );
  dh::LogFlags( dh::LLF_GENERIC, "Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
  dh::LogFlags( dh::LLF_GENERIC, "Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );
  */
  Log( "Renderer.Full", "After Reset" );
  Log( "Renderer.Full", "Actual Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
  Log( "Renderer.Full", "Actual Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );
  Log( "Renderer.Full", "Stored Window Styles %x", m_WindowedModeStyles );
  Log( "Renderer.Full", "Stored Window ExStyles %x", m_WindowedModeExStyles );

  // Initialize the app's device-dependent objects
  hr = RestoreDeviceObjects();
  if ( FAILED( hr ) )
  {
    Log( "Renderer.General", "Resize3DEnvironment: RestoreDeviceObjects failed" );
    return hr;
  }
  //Log( "Renderer.General", "Resize3DEnvironment: Done" );

  return S_OK;
}



bool CDX8RenderClass::InitDeviceObjects()
{
  RestoreDeviceObjects();
  return true;
}



bool CDX8RenderClass::RestoreDeviceObjects()
{
  RestoreStates();

  RestoreAllVertexBuffers();
  RestoreAllTextures();
  RestoreAllFonts();

  return true;
}



bool CDX8RenderClass::InvalidateDeviceObjects()
{
  return true;
}



bool CDX8RenderClass::DeleteDeviceObjects()
{
  return true;
}



bool CDX8RenderClass::BeginScene()
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
      //return true;
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



void CDX8RenderClass::EndScene()
{

  if ( m_pd3dDevice )
  {
    m_pd3dDevice->EndScene();
  }

}



void CDX8RenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{

  if ( !m_strSaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_strSaveScreenShotFileName.c_str() );

    m_strSaveScreenShotFileName.clear();
  }

  // Show the frame on the primary surface.
  if ( m_pd3dDevice )
  {
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
    if ( FAILED( m_pd3dDevice->Present( pSrc, pDest, NULL, NULL ) ) )
    {
      //dh::Log( "Present failed" );
    }
  }

}



void CDX8RenderClass::Clear( bool bClearColor, bool bClearZ )
{
  if ( m_pd3dDevice )
  {
    DWORD   ClearFlags = bClearColor ? D3DCLEAR_TARGET : 0;

    if ( ( m_bUseDepthBuffer )
    &&   ( bClearZ ) )
    {
      ClearFlags |= D3DCLEAR_ZBUFFER;
    }
    if ( m_MinStencilBits > 0 )
    {
      ClearFlags |= D3DCLEAR_STENCIL;
    }

    if ( ClearFlags )
    {
      HRESULT hr = m_pd3dDevice->Clear( 0, NULL, ClearFlags, m_ClearColor, 1.0f, 0 );
      if ( FAILED( hr ) )
      {
        dh::Log( "Clear failed, %x", hr );
      }
    }
  }
}



GR::Graphic::ImageData* CDX8RenderClass::LoadAndConvert( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  if ( m_pEnvironment == NULL )
  {
    return NULL;
  }
  IImageFormatManager*    pManager = (IImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }
  GR::Graphic::ImageData*   pData = pManager->LoadData( szFileName );
  if ( pData == NULL )
  {
    return NULL;
  }

  // convert format
  if ( imgFormatToConvert != GR::Graphic::IF_UNKNOWN )
  {
    pData->ConvertSelfTo( imgFormatToConvert, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
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



XTexture* CDX8RenderClass::LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, const GR::u32 MipMaps, GR::u32 ColorKeyReplacementColor )
{
  GR::String     path = szFileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), imgFormatToConvert, ColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData, MipMaps );
  if ( pTexture == NULL )
  {
    delete pData;
    return NULL;
  }

  Log( "Renderer.Full", "Texture (%s) loaded, loaded format %d, created format %d", 
                        path.c_str(), pData->ImageFormat(), pTexture->m_ImageFormat );

  pTexture->m_ColorKey        = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;
}



XTexture* CDX8RenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), MipMapLevels, AllowUsageAsRenderTarget );
  if ( pTexture == NULL )
  {
    return NULL;
  }
  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



XTexture* CDX8RenderClass::CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    Log( "Renderer.General", "CDX8RenderClass::CreateTexture Invalid size" );
    return NULL;
  }

  if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
  {
    // unterstützte Wechsel
    if ( imgFormat == GR::Graphic::IF_R8G8B8 )
    {
      imgFormat = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        Log( "Renderer.General", "CDX8RenderClass::CreateTexture Format IF_R8G8B8 or IF_X8R8G8B8 not supported" );
        return NULL;
      }
    }
    else if ( imgFormat == GR::Graphic::IF_X1R5G5B5 )
    {
      imgFormat = GR::Graphic::IF_R5G6B5;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        Log( "Renderer.General", "CDX8RenderClass::CreateTexture Format IF_X1R5G5B5 or IF_R5G6B5 not supported" );
        return NULL;
      }
    }
    else if ( imgFormat == GR::Graphic::IF_PALETTED )
    {
      // let it convert
      imgFormat = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        Log( "Renderer.General", "CDX8RenderClass::CreateTexture Format IF_X1R5G5B5 or IF_R5G6B5 not supported" );
        return NULL;
      }
    }
    else
    {
      Log( "Renderer.General", CMisc::printf( "CDX8RenderClass::CreateTexture Format %d not supported3", imgFormat ) );
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

  Log( "Renderer.Full", "Create Texture from format %d to d3d format %d", imgFormat, MapFormat( imgFormat ) );


  CDX8Texture*          pTexture = NULL;

  pTexture = new( std::nothrow )CDX8Texture( this );
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
  if ( pTexture->AllowUsageAsRenderTarget )
  {
    pTexture->RequiresRebuilding = true;
  }

  HRESULT hRes = m_pd3dDevice->CreateTexture( iTWidth,
                                              iTHeight,
                                              pTexture->m_MipMapLevels,
                                              pTexture->AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
                                              MapFormat( imgFormat ),
                                              pTexture->AllowUsageAsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
                                              &pTexture->m_Surface );
  if ( FAILED( hRes ) )
  {
    // failed to create
    Log( "Renderer.General", "CreateTexture: CreateTexture failed" );
    delete pTexture;

    return NULL;
  }

  // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
  D3DSURFACE_DESC ddsd;
  pTexture->m_Surface->GetLevelDesc( 0, &ddsd );

  pTexture->m_SurfaceSize.Set( ddsd.Width, ddsd.Height );
  pTexture->m_ImageSourceSize.Set( Width, Height );
  pTexture->m_PixelFormat   = ddsd.Format;
  pTexture->m_ImageFormat   = imgFormat;

  AddTexture( pTexture );

  GR::Graphic::ImageData    data;
  data.CreateData( Width, Height, imgFormat );

  pTexture->m_StoredImageData.push_back( data );
  return pTexture;
}



bool CDX8RenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  HRESULT hr = m_pD3D->CheckDeviceFormat( 
          m_CurrentAdapter,
          m_vectAdapters[m_CurrentAdapter].vectDevices[m_CurrentDevice].deviceType, //D3DDEVTYPE_HAL,
          m_pCurrentModeInfo->PixelFormat,
          AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
          D3DRTYPE_TEXTURE,
          MapFormat( imgFormat ) );

  return !!SUCCEEDED( hr );
}



D3DFORMAT CDX8RenderClass::MapFormat( GR::Graphic::eImageFormat imgFormat )
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
      Log( "Renderer.General", "CDX8RenderClass::MapFormat Unsupported format %d", imgFormat );
      break;
  }

  return d3dFmt;

}



GR::Graphic::eImageFormat CDX8RenderClass::MapFormat( D3DFORMAT d3dFormat )
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
    default:
      Log( "Renderer.General", "CDX8RenderClass::MapFormat Unsupported D3D format %d", d3dFormat );
      break;
  }

  return imgFmt;

}



void CDX8RenderClass::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                    GR::f32 fTU1, GR::f32 fTV1,
                                    GR::f32 fTU2, GR::f32 fTV2,
                                    GR::f32 fTU3, GR::f32 fTV3,
                                    GR::f32 fTU4, GR::f32 fTV4,
                                    GR::u32 Color1, GR::u32 Color2, 
                                    GR::u32 Color3, GR::u32 Color4, float fZ )
{

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  iX += m_DisplayOffset.x;
  iY += m_DisplayOffset.y;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

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

  GR::tVector   ptPos( (float)iX, (float)iY, fZ );
  GR::tVector   ptSize( (float)iWidth, (float)iHeight, 0.0f );

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

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



void CDX8RenderClass::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::f32 fTU3, GR::f32 fTV3,
                                          GR::f32 fTU4, GR::f32 fTV4,
                                          GR::u32 Color1, GR::u32 Color2, 
                                          GR::u32 Color3, GR::u32 Color4, float fZ )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  fX += m_DisplayOffset.x;
  fY += m_DisplayOffset.y;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

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

  GR::tVector   ptPos( fX, fY, fZ );
  GR::tVector   ptSize( fWidth, fHeight, 0.0f );

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

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



void CDX8RenderClass::RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
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

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

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



void CDX8RenderClass::RenderTriangle2d( const GR::tPoint& pt1, const GR::tPoint& pt2, const GR::tPoint& pt3, 
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

  float   fRHW = 1.0f;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

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



void CDX8RenderClass::RenderQuad( const GR::tVector& ptPos1,
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

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

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



void CDX8RenderClass::RenderQuad( const GR::tVector& ptPos1,
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

  normal.normalize();

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

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



void CDX8RenderClass::RenderTriangle( const GR::tVector& ptPos1,
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

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_NORMAL );

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



XVertexBuffer* CDX8RenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  if ( !m_pd3dDevice )
  {
    Log( "Renderer.General", "CDX8RenderClass::CreateVertexBuffer: no Device" );
    return NULL;
  }

  CDX8VertexBuffer*   pBuffer = new CDX8VertexBuffer( this );

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* CDX8RenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{

  if ( !m_pd3dDevice )
  {
    Log( "Renderer.General", "CDX8RenderClass::CreateVertexBuffer: no Device" );
    return NULL;
  }

  CDX8VertexBuffer*   pBuffer = new CDX8VertexBuffer( this );
  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* CDX8RenderClass::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat )
{
  XVertexBuffer*    pBuffer = CreateVertexBuffer( MeshObject.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( MeshObject );

  return pBuffer;
}



void CDX8RenderClass::DestroyVertexBuffer( XVertexBuffer* pVBBuffer )
{
  if ( pVBBuffer == NULL )
  {
    return;
  }

  pVBBuffer->Release();

  m_VertexBuffers.remove( pVBBuffer );
  delete pVBBuffer;
}



bool CDX8RenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
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

  return ( (CDX8VertexBuffer*)pBuffer )->Display( Index, Count );

}



LPDIRECT3DDEVICE8 CDX8RenderClass::Device()
{

  return m_pd3dDevice;

}



void CDX8RenderClass::SetTexture( GR::u32 Stage, XTexture* pTexture )
{

  m_pd3dDevice->SetTexture( Stage, pTexture ? ( (CDX8Texture*)pTexture )->m_Surface : NULL );

}



bool CDX8RenderClass::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
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
    case RS_SWAP_MODE:
      m_d3dpp.SwapEffect = (D3DSWAPEFFECT)rValue;
      break;
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
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
      }
      break;
    case RS_ADDRESSV:
      if ( rValue == RSV_ADDRESS_WRAP )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
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
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MINFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MAGFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MAGFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MIPFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_MIPFILTER, D3DTEXF_POINT );
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
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
      }
      else if ( rValue == RSV_BLENDFACTORALPHA )
      {
        hRes = m_pd3dDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA );
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
    Log( "Renderer.General", "CDX8RenderClass::SetState not supported (%d - %d - Stage %d)", rState, rValue, Stage );
  }

  if ( ( rState == RS_SWAP_MODE )
  &&   ( SUCCEEDED( hRes ) ) )
  {
    // reinit
    hRes = Resize3DEnvironment();
  }
  return SUCCEEDED( hRes );
}



void CDX8RenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
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



bool CDX8RenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{

  if ( pTexture == NULL )
  {
    Log( "Renderer.General", "ImageDataFromTexture: Texture was NULL" );
    return false;
  }

  CDX8Texture*    pDX8Texture = (CDX8Texture*)pTexture;

  if ( MipMapLevel >= pDX8Texture->m_StoredImageData.size() )
  {
    Log( "Renderer.General", "ImageDataFromTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, pDX8Texture->m_StoredImageData.size() );
    return false;
  }

  std::list<GR::Graphic::ImageData>::iterator   it( pDX8Texture->m_StoredImageData.begin() );
  std::advance( it, MipMapLevel );

  ImageData = *it;
  return true;

}



bool CDX8RenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{

  if ( pTexture == NULL )
  {
    Log( "Renderer.General", "CopyDataToTexture: Texture was NULL" );
    return false;
  }

  CDX8Texture*    pDX8Texture = (CDX8Texture*)pTexture;

  if ( MipMapLevel >= pDX8Texture->m_MipMapLevels )
  {
    Log( "Renderer.General", "CopyDataToTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, pDX8Texture->m_MipMapLevels );
    return false;
  }

  D3DLOCKED_RECT      lockRect;

  if ( FAILED( pDX8Texture->m_Surface->LockRect( MipMapLevel, &lockRect, NULL, 0 ) ) )
  {
    Log( "Renderer.General", "CopyDataToTexture LockRect failed" );
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

  /*
  if ( ( ImageData.Width() != Width )
  ||   ( ImageData.Height() != Height ) )
  */
  if ( ( ImageData.Width() > (int)Width )
  ||   ( ImageData.Height() > (int)Height ) )
  {
    pDX8Texture->m_Surface->UnlockRect( MipMapLevel );
    Log( "Renderer.General", CMisc::printf( "CopyDataToTexture: Sizes mismatching %dx%d != %dx%d (Level %d)", ImageData.Width(), ImageData.Height(), Width, Height, MipMapLevel ) );
    dh::Log( "CopyDataToTexture: Sizes mismatching %dx%d != %dx%d (Level %d)", ImageData.Width(), ImageData.Height(), Width, Height, MipMapLevel );
    return false;
  }

  GR::Graphic::ContextDescriptor      cdTexture;
  
  cdTexture.Attach( Width, Height, lockRect.Pitch, pTexture->m_ImageFormat, lockRect.pBits );

  if ( !ImageData.ConvertInto( &cdTexture, ColorKey != 0, ColorKey ) )
  {
    pDX8Texture->m_Surface->UnlockRect( MipMapLevel );
    Log( "Renderer.General", "CopyDataToTexture ConvertInto failed" );
    dh::Log( "CopyDataToTexture ConvertInto failed" );
    return false;
  }

  pDX8Texture->m_Surface->UnlockRect( MipMapLevel );

  // store copied data
  while ( pDX8Texture->m_StoredImageData.size() <= MipMapLevel )
  {
    pDX8Texture->m_StoredImageData.push_back( GR::Graphic::ImageData() );
  }
  pDX8Texture->m_StoredImageData.back() = ImageData;
  return true;

}



void CDX8RenderClass::RestoreAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    if ( pTexture->m_LoadedFromFile.empty() )
    {
      // eine bloss erzeugte Textur
      CDX8Texture*    pDX8Texture = (CDX8Texture*)pTexture;

      // sicher gehen
      pDX8Texture->Release();
        
      if ( SUCCEEDED( m_pd3dDevice->CreateTexture( 
                      pTexture->m_SurfaceSize.x,
                      pTexture->m_SurfaceSize.y,
                      pDX8Texture->m_MipMapLevels,
                      pDX8Texture->AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
                      pDX8Texture->m_PixelFormat,
                      pDX8Texture->AllowUsageAsRenderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
                      &pDX8Texture->m_Surface ) ) )
      {
      }
      else
      {
        Log( "Renderer.General", "CreateTexture failed" );
      }
      GR::u32     MipMapLevel = 0;
      std::list<GR::Graphic::ImageData>::iterator   itID( pDX8Texture->m_StoredImageData.begin() );
      while ( itID != pDX8Texture->m_StoredImageData.end() )
      {
        CopyDataToTexture( pDX8Texture, *itID, 0, MipMapLevel );

        ++itID;
        ++MipMapLevel;
      }
      if ( pDX8Texture->AllowUsageAsRenderTarget )
      {
        pDX8Texture->RequiresRebuilding = true;
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

        CDX8Texture*    pDX8Texture = (CDX8Texture*)pTexture;

        // sicher gehen
        pDX8Texture->Release();
          
        if ( SUCCEEDED( m_pd3dDevice->CreateTexture( 
                        pTexture->m_SurfaceSize.x,
                        pTexture->m_SurfaceSize.y,
                        pDX8Texture->m_MipMapLevels,
                        pDX8Texture->AllowUsageAsRenderTarget ? D3DUSAGE_RENDERTARGET : 0,
                        pDX8Texture->m_PixelFormat,
                        D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                        &pDX8Texture->m_Surface ) ) )
        {
          CopyDataToTexture( pTexture, *pData );

          // Mipmap-Levels einlesen
          std::list<GR::String>::iterator    it( pDX8Texture->m_listFileNames.begin() );

          GR::u32     Level = 1;

          while ( it != pDX8Texture->m_listFileNames.end() )
          {
            GR::String&  strPath( *it );

            GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)pDX8Texture->m_ImageFormat, pDX8Texture->m_ColorKey, pDX8Texture->m_ColorKeyReplacementColor );
            if ( pData == NULL )
            {
              Log( "Renderer.General", CMisc::printf( "DX8Renderer:: RestoreAllTextures failed to load MipMap (%s)", strPath.c_str() ) );
            }
            else
            {
              CopyDataToTexture( pTexture, *pData, pDX8Texture->m_ColorKey, Level );
              delete pData;
            }
            ++it;
            ++Level;
          }
        }
        else
        {
          Log( "Renderer.General", "Create Texture Failed" );
        }
        delete pData;
      }
    }

    ++it;
  }

}



bool CDX8RenderClass::OnResized()
{
  return ResizeSurface();

}



bool CDX8RenderClass::SetViewport( const XViewport& Viewport )
{
  if ( !m_bReady )
  {
    Log( "Renderer.General", "SetViewport called while not initialized" );
    return false;
  }

  D3DVIEWPORT8    viewPort;

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
  m_ViewPort.X       = viewPort.X;
  m_ViewPort.Y       = viewPort.Y;
  m_ViewPort.Width   = viewPort.Width;
  m_ViewPort.Height  = viewPort.Height;
  m_ViewPort.MinZ    = viewPort.MinZ;
  m_ViewPort.MaxZ    = viewPort.MaxZ;
  return true;
}



bool CDX8RenderClass::SetTrueViewport( const XViewport& Viewport )
{
  D3DVIEWPORT8    viewPort;

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
  m_ViewPort.X       = viewPort.X;
  m_ViewPort.Y       = viewPort.Y;
  m_ViewPort.Width   = viewPort.Width;
  m_ViewPort.Height  = viewPort.Height;
  m_ViewPort.MinZ    = viewPort.MinZ;
  m_ViewPort.MaxZ    = viewPort.MaxZ;
  return true;
}



GR::u32 CDX8RenderClass::Width()
{

  return m_RenderWidth;

}



GR::u32 CDX8RenderClass::Height()
{

  return m_RenderHeight;

}



GR::Graphic::eImageFormat CDX8RenderClass::ImageFormat()
{
  return MapFormat( m_d3dpp.BackBufferFormat );
}



bool CDX8RenderClass::IsFullscreen()
{
  return !m_bWindowed;
}



bool CDX8RenderClass::ToggleFullscreen()
{
  m_bWindowed = !m_bWindowed;

  return SUCCEEDED( Resize3DEnvironment() );
}



bool CDX8RenderClass::SetMode( XRendererDisplayMode& DisplayMode )
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

  Log( "Renderer.General", CMisc::printf( "CDX8RenderClass::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.ImageFormat ) );
  return false;
}



void CDX8RenderClass::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float fZ )
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


  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  D3DXVECTOR3   vectDelta( m_DirectTexelMappingOffset.x, m_DirectTexelMappingOffset.y, 0.0f );

  vertData[0].position = D3DXVECTOR3( m_Canvas.Left + (float)m_DisplayOffset.x + pt1.x * virtualX, m_Canvas.Top + (float)m_DisplayOffset.y + pt1.y * virtualY, fZ ) + vectDelta;
  vertData[0].color = Color1;

  vertData[1].position = D3DXVECTOR3( m_Canvas.Left + (float)m_DisplayOffset.x + pt2.x * virtualX, m_Canvas.Top + (float)m_DisplayOffset.y + pt2.y * virtualY, fZ ) + vectDelta;
  vertData[1].color = Color2;

  vertData[0].fRHW = vertData[1].fRHW = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void CDX8RenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 )
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


  m_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );

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



bool CDX8RenderClass::SetLight( GR::u32 LightIndex, XLight& Light )
{

  if ( LightIndex >= 8 )
  {
    return false;
  }

  D3DLIGHT8     d3dLight;

  memset( &d3dLight, 0, sizeof( d3dLight ) );

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

  m_Light[LightIndex] = Light;

  return SUCCEEDED( m_pd3dDevice->SetLight( LightIndex, &d3dLight ) );

}



bool CDX8RenderClass::SetMaterial( const XMaterial& Material )
{

  D3DMATERIAL8      mat;

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



void CDX8RenderClass::DoSaveSnapShot( const char* szFile )
{

  IDirect3DSurface8   *pSourceSurface,
                      *pDestSurface;

  HRESULT   hRes;

  hRes = m_pd3dDevice->GetBackBuffer( 0,
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

  hRes = m_pd3dDevice->CreateImageSurface( ddsd.Width,
                                           ddsd.Height,
                                           ddsd.Format,
                                           &pDestSurface );

  if ( !SUCCEEDED( hRes ) )
  {
    pSourceSurface->Release();
    return;
  }

  hRes = m_pd3dDevice->CopyRects( pSourceSurface,
                                  NULL,
                                  0,
                                  pDestSurface,
                                  NULL );

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



bool CDX8RenderClass::SaveScreenShot( const char* szFileName )
{

  if ( szFileName )
  {
    m_strSaveScreenShotFileName = szFileName;
    return true;
  }
  return false;

}



bool CDX8RenderClass::RenderMesh( const Mesh::IMesh& Mesh )
{
  CDX8VertexBuffer   vbTemp( this );

  vbTemp.Create( Mesh.FaceCount() * 3, XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD, XVertexBuffer::PT_TRIANGLE );

  vbTemp.FillFromMesh( Mesh );

  // TODO - auf max primitive count prüfen!
  m_pd3dDevice->SetVertexShader( vbTemp.VertexFormat() );
  m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, (UINT)( vbTemp.VertexCount() / 3 ), vbTemp.Data(), (UINT)vbTemp.VertexSize() );

  return true;//RenderVertexBuffer( &vbTemp );
}



bool CDX8RenderClass::VSyncEnabled()
{

  return m_VSyncEnabled;

}



void CDX8RenderClass::EnableVSync( bool Enable )
{

  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
    Resize3DEnvironment();
  }

}



void CDX8RenderClass::SetRenderTarget( XTexture* pTexture )
{
  if ( pTexture == NULL )
  {
    if ( m_pCurrentlySetTargetSurface != NULL )
    {
      m_pCurrentlySetTargetSurface->Release();
      m_pCurrentlySetTargetSurface = NULL;
    }
    if ( m_pBackBufferTargetSurface != NULL )
    {
      HRESULT hRes = m_pd3dDevice->SetRenderTarget( m_pBackBufferTargetSurface, m_pBackBufferDepthStencilSurface );
      m_pBackBufferTargetSurface->Release();
      m_pBackBufferTargetSurface = NULL;
      if ( FAILED( hRes ) )
      {
        dh::Log( "CDX8RenderClass::SetRenderTarget failed to reset to back buffer (%x)", hRes );
      }
    }
    m_pCurrentlySetTargetTexture = NULL;
  }
  else
  {
    if ( m_pBackBufferTargetSurface == NULL )
    {
      HRESULT hRes = m_pd3dDevice->GetRenderTarget( &m_pBackBufferTargetSurface );
      if ( FAILED( hRes ) )
      {
        dh::Log( "CDX8RenderClass::SetRenderTarget failed to get back buffer surface (%x)", hRes );
        return;
      }
    }
    if ( ( (CDX8Texture*)pTexture )->m_Surface == NULL )
    {
      dh::Log( "CDX8RenderClass::SetRenderTarget trying to set texture without surface" );
      return;
    }
    m_pCurrentlySetTargetTexture = pTexture;

    IDirect3DSurface8*  pSurface = NULL;
    HRESULT hRes = ( (CDX8Texture*)pTexture )->m_Surface->GetSurfaceLevel( 0, &pSurface );
    if ( FAILED( hRes ) )
    {
      dh::Log( "CDX8RenderClass::SetRenderTarget GetSurfaceLevel failed (%x)", hRes );
      return;
    }
    if ( m_pCurrentlySetTargetSurface != NULL )
    {
      m_pCurrentlySetTargetSurface->Release();
      m_pCurrentlySetTargetSurface = NULL;
    }
    m_pCurrentlySetTargetSurface = pSurface;

    hRes = m_pd3dDevice->SetRenderTarget( m_pCurrentlySetTargetSurface, m_pBackBufferDepthStencilSurface );
    if ( FAILED( hRes ) )
    {
      dh::Log( "CDX8RenderClass::SetRenderTarget failed to set (%x)", hRes );
    }
    else
    {
      ( (CDX8Texture*)pTexture )->RequiresRebuilding = false;
    }
  }
}
