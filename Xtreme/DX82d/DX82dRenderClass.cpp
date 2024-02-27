#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>
#include <Grafik/Image.h>

#include <Interface/IAssetLoader.h>

#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont2d.h>

#include <Xtreme/XMultiTexture.h>
#include <Xtreme/Xtreme2d/XMultiTexture2d.h>

#include <Misc/Misc.h>

#include <Xtreme/Environment/XWindow.h>

#include <debug/DebugService.h>

#include <String/XML.h>

#include <IO/FileStream.h>

#include "DX82dRenderClass.h"
#include "DX82dTexture.h"



#undef D3D_SDK_VERSION
#include <d3dx8.h>


#pragma comment ( lib, "d3dx8.lib" )
#pragma comment ( lib, "d3d8.lib" )
#pragma comment ( lib, "dxguid.lib" )

DX82dRenderer* DX82dRenderer::g_pDX8Instance = NULL;



DX82dRenderer::DX82dRenderer( HINSTANCE hInstance ) :
  m_pD3D( NULL ),
  m_pd3dDevice( NULL ),
  m_pOldProc( NULL ),
  m_hWndFocus( NULL ),
  m_Ready( false ),
  m_UseDepthBuffer( true ),
  m_VSyncEnabled( false ),
  m_ForceWindowSize( false ),
  m_MultiThreadSafe( false ),
  m_MinDepthBits( 16 ),
  m_MinStencilBits( 0 ),
  m_Windowed( true ),
  m_Adapter( 0 ),
  m_hInstance( hInstance ),
  m_SaveScreenShotFileName( "" ),
  m_pCurrentModeInfo( NULL )
{
  if ( m_hInstance == NULL )
  {
    m_hInstance = GetModuleHandle( NULL );
  }
  m_DirectTexelMappingOffset.Set( -0.5f, -0.5f );
  g_pDX8Instance = this;
}



DX82dRenderer::~DX82dRenderer()
{
  Release();
  g_pDX8Instance = NULL;
}



bool DX82dRenderer::Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment )
{
  m_pEnvironment      = &Environment;
  m_pDebugger = (IDebugService*)m_pEnvironment->Service( "Logger" );

  for ( int i = 0; i < 8; ++i )
  {
    m_pCurTexture[i] = NULL;
  }

  HRESULT hr;

  // Create the Direct3D object
  m_pD3D = Direct3DCreate8( 220 ); //D3D_SDK_VERSION = 220 für DX8(.1?)
  if ( m_pD3D == NULL )
  {
    dh::Log( "Create: Direct3DCreate8 failed" );
    return false;
  }

  m_CreationWidth   = Width;
  m_CreationHeight  = Height;
  m_CreationDepth   = Depth;

  m_Windowed         = !FullScreen;

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

  m_wPlacement.length = sizeof( m_wPlacement );
  GetWindowPlacement( hWnd, &m_wPlacement );
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

  if ( !FindSuitableMode( m_CreationWidth, m_CreationHeight, m_CreationDepth, m_Windowed ) )
  {
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: failed to find suitable mode (%dx%dx%d Windowed %d)", m_CreationWidth, m_CreationHeight, m_CreationDepth, m_Windowed );
    return false;
  }
  m_VirtualSize.Set( m_CreationWidth, m_CreationHeight );

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
    dh::Log( "Create: no valid window handle" );
    return false;
  }

  m_hWndFocus = m_hwndViewport;

  SortAndCleanDisplayModes();

  // Initialize the 3D environment for the app
  if ( FAILED( hr = Initialize3DEnvironment() ) )
  {
    //DestroyWindow( m_hwndViewport );
    if ( m_pD3D )
    {
      m_pD3D->Release();
      m_pD3D = NULL;
    }
    dh::Log( "Create: Initialize3DEnvironment failed" );
    return false;
  }

  // The app is ready to go
  m_Ready = TRUE;

  m_FullscreenDisplayMode.Width   = m_CreationWidth;
  m_FullscreenDisplayMode.Height  = m_CreationHeight;
  m_FullscreenDisplayMode.ImageFormat = MapFormat( m_Adapters[m_CurrentAdapter].Devices[m_CurrentDevice].Modes[m_CurrentMode].m_PixelFormat );
  m_FullscreenDisplayMode.FullScreen = true;

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

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );

        GR::u32     ForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        XTexture* pTexture = LoadTexture( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" ) ).c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey );

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
        }
        else
        {
          dh::Error( "DX82dRenderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) );
        }
      }

      GR::up    imageSecCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < imageSecCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          dh::Error( "DX82dRenderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
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

        X2dFont*      pFont = NULL;

        if ( ( LoadFlags & X2dFont::FLF_SQUARED )
        ||   ( LoadFlags & X2dFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
                                                    LoadFlags, 
                                                    ColorKey );
        }
        else
        {
          pFont = LoadFont( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), LoadFlags, ColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            dh::Error( "DX82dRenderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont2d( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          dh::Error( "DX82dRenderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }

      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  return true;
}



bool DX82dRenderer::Release()
{
  if ( !m_Ready )
  {
    return true;
  }
  ReleaseAssets();

  Cleanup3DEnvironment();

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



bool DX82dRenderer::FindSuitableMode( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool bWindowed )
{
  int   adapter = 0;

  std::vector<tD3DAdapterInfo>::iterator   itAdapter( m_Adapters.begin() );
  while ( itAdapter != m_Adapters.end() )
  {
    tD3DAdapterInfo&    AdapterInfo( *itAdapter );

    int   device = 0;

    std::vector<tD3DDeviceInfo>::iterator   itDevice( AdapterInfo.Devices.begin() );
    while ( itDevice != AdapterInfo.Devices.end() )
    {
      tD3DDeviceInfo&   DeviceInfo( *itDevice );

      if ( DeviceInfo.DeviceType == D3DDEVTYPE_HAL )
      {
        if ( ( ( bWindowed )
        &&     ( DeviceInfo.CanDoWindowed ) )
        ||   ( !bWindowed ) )
        {
          int               mode = 0;

          std::vector<tD3DModeInfo>::iterator   itMode( DeviceInfo.Modes.begin() );
          while ( itMode != DeviceInfo.Modes.end() )
          {
            tD3DModeInfo&   ModeInfo( *itMode );

            if ( bWindowed )
            {
              // im Fenstermode kann nur die aktive Tiefe benutzt werden!
              Depth = GR::Graphic::ImageData::DepthFromImageFormat( MapFormat( ModeInfo.m_PixelFormat ) );
              //dh::Log( "Depth changed to %d", Depth );
            }

            if ( ( bWindowed )
            ||   ( ( !bWindowed )
            &&     ( ModeInfo.m_Width == Width )
            &&     ( ModeInfo.m_Height == Height ) ) )
            {
              if ( ( ( Depth == 24 )
              ||     ( Depth == 32 ) )
              &&   ( ModeInfo.m_PixelFormat == D3DFMT_R8G8B8 )
              ||   ( ModeInfo.m_PixelFormat == D3DFMT_A8R8G8B8 )
              ||   ( ModeInfo.m_PixelFormat == D3DFMT_X8R8G8B8 ) )
              {
                m_Adapter                     = adapter;
                AdapterInfo.CurrentDevice     = device;
                DeviceInfo.CurrentMode        = mode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_PixelFormat );
                  m_FullscreenDisplayMode.FullScreen = true;
                }
                return true;
              }
              if ( ( Depth == 16 )
              &&   ( ModeInfo.m_PixelFormat == D3DFMT_R5G6B5 )
              ||   ( ModeInfo.m_PixelFormat == D3DFMT_X1R5G5B5 )
              ||   ( ModeInfo.m_PixelFormat == D3DFMT_A1R5G5B5 ) )
              {
                m_Adapter                     = adapter;
                AdapterInfo.CurrentDevice     = device;
                DeviceInfo.CurrentMode        = mode;

                if ( !bWindowed )
                {
                  m_FullscreenDisplayMode.Width     = m_CreationWidth;
                  m_FullscreenDisplayMode.Height    = m_CreationHeight;
                  m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_PixelFormat );
                  m_FullscreenDisplayMode.FullScreen = true;
                }
                return true;
              }
            }

            ++itMode;
            ++mode;
          }
        }
      }

      ++itDevice;
      ++device;
    }


    ++itAdapter;
    ++adapter;
  }

  return false;
}



HRESULT DX82dRenderer::BuildDeviceList()
{
  const DWORD         NumDeviceTypes = 2;

  const TCHAR*        strDeviceDescs[] = { "HAL", "REF" };



  const D3DDEVTYPE    DeviceTypes[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF };


  BOOL                bHALExists                  = FALSE,
                      bHALIsWindowedCompatible    = FALSE,
                      bHALIsDesktopCompatible     = FALSE,
                      bHALIsSampleCompatible      = FALSE;


  m_WindowedModePossible = false;

  // Loop through all the adapters on the system (usually, there's just one
  // unless more than one graphics card is present).
  Log( "Renderer.Full", "%d Adapters found", m_pD3D->GetAdapterCount() );
  for ( UINT adapter = 0; adapter < m_pD3D->GetAdapterCount(); adapter++ )
  {
    // Fill in adapter info
    tD3DAdapterInfo      Adapter;

    m_pD3D->GetAdapterIdentifier( adapter, 0, &Adapter.d3dAdapterIdentifier );
    m_pD3D->GetAdapterDisplayMode( adapter, &Adapter.d3ddmDesktop );
    Adapter.Devices.clear();
    Adapter.CurrentDevice = 0;

    // Enumerate all display modes on this adapter
    D3DDISPLAYMODE          modes[100];
    D3DFORMAT               formats[20];
    DWORD                   NumFormats      = 0,
                            NumModes        = 0,
                            NumAdapterModes = m_pD3D->GetAdapterModeCount( adapter );


    // Add the adapter's current desktop format to the list of formats
    //formats[NumFormats++] = Adapter.d3ddmDesktop.Format;

    for ( UINT mode = 0; mode < NumAdapterModes; mode++ )
    {
      // Get the display mode attributes
      D3DDISPLAYMODE      DisplayMode;

      m_pD3D->EnumAdapterModes( adapter, mode, &DisplayMode );

      // Check if the mode already exists (to filter out refresh rates)
      DWORD m = 0;
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

        Log( "Renderer.Full", "DX82d - found mode %dx%dx%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.Format );

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
          Log( "Renderer.Full", "DX82d - found new format %d, Mode %d", NumFormats, DisplayMode.Format );
          formats[NumFormats++] = DisplayMode.Format;
        }
      }
    }

    // Sort the list of display modes (by format, then width, then height)
    qsort( modes, NumModes, sizeof( D3DDISPLAYMODE ), SortModesCallback );

    // Add devices to adapter
    for ( UINT device = 0; device < NumDeviceTypes; device++ )
    {
      // Fill in device info
      tD3DDeviceInfo Device;
      //Device                   = &pAdapter->devices[pAdapter->NumDevices];
      Device.DeviceType       = DeviceTypes[device];

      m_pD3D->GetDeviceCaps( adapter, DeviceTypes[device], &Device.d3dCaps );
      Device.Name             = strDeviceDescs[device];
      Device.Modes.clear();
      Device.CurrentMode      = 0;
      Device.CanDoWindowed    = FALSE;
      Device.Windowed         = FALSE;
      Device.MultiSampleType  = D3DMULTISAMPLE_NONE;

      // Examine each format supported by the adapter to see if it will
      // work with this device and meets the needs of the application.
      BOOL        bFormatConfirmed[20];

      DWORD       Behavior[20];

      D3DFORMAT   fmtDepthStencil[20];

      Log( "Renderer.Full", "DX82d - found %d formats in Device %s", NumFormats, Device.Name.c_str() );

      for ( DWORD f = 0; f < NumFormats; f++ )
      {
        bFormatConfirmed[f] = FALSE;
        fmtDepthStencil[f]  = D3DFMT_UNKNOWN;

        // Skip formats that cannot be used as render targets on this device
        HRESULT   hRes = m_pD3D->CheckDeviceType( adapter, Device.DeviceType, formats[f], formats[f], FALSE );

        if ( hRes == D3DERR_INVALIDCALL )
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Fullscreen failed - D3DERR_INVALIDCALL" );
        }
        else if ( hRes == D3DERR_NOTAVAILABLE )
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Fullscreen failed - D3DERR_NOTAVAILABLE" );
        }
        else
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Fullscreen failed - Error %x", hRes );
        }

        hRes = m_pD3D->CheckDeviceType( adapter, Device.DeviceType, formats[f], D3DFMT_UNKNOWN, TRUE );
        if ( hRes == D3DERR_INVALIDCALL )
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Windowed failed - D3DERR_INVALIDCALL" );
        }
        else if ( hRes == D3DERR_NOTAVAILABLE )
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Windowed failed - D3DERR_NOTAVAILABLE" );
        }
        else
        {
          Log( "Renderer.Full", "DX82d - CheckDeviceType Windowed failed - Error %x", hRes );
        }
        // If the device can be used on this adapter, D3D_OK is returned. D3DERR_INVALIDCALL is returned if Adapter equals or exceeds the number of display adapters in the system. 
          // D3DERR_INVALIDCALL is also returned if IDirect3D9::CheckDeviceType specified a device that does not exist. D3DERR_NOTAVAILABLE is returned if the requested back buffer format is not supported, 
        // or if harare acceleration is not available for the specified formats. 


        if ( ( FAILED( m_pD3D->CheckDeviceType( adapter, Device.DeviceType, formats[f], formats[f], FALSE ) ) )
        &&   ( FAILED( m_pD3D->CheckDeviceType( adapter, Device.DeviceType, formats[f], D3DFMT_UNKNOWN, TRUE ) ) ) )
        {
          Log( "Renderer.Full", "DX82d - Reject format %x", formats[f] );
          continue;
        }

        if ( Device.DeviceType == D3DDEVTYPE_HAL )
        {
          // This system has a HAL device
          bHALExists = TRUE;

          if ( Device.d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
          {
            // HAL can run in a window for some mode
            Log( "Renderer.Full", "DX82d - can render windowed" );
            bHALIsWindowedCompatible = TRUE;
            if ( SUCCEEDED( m_pD3D->CheckDeviceType( adapter, Device.DeviceType, Adapter.d3ddmDesktop.Format, formats[f], FALSE ) ) )
            {
              // HAL can run in a window for the current desktop mode
              bHALIsDesktopCompatible = TRUE;
              m_WindowedModePossible  = true;
              Device.CanDoWindowed     = TRUE;
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
        &&   ( m_UseDepthBuffer ) )
        {
          if ( !FindDepthStencilFormat( adapter, Device.DeviceType, formats[f], &fmtDepthStencil[f] ) )
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


              modeInfo.m_Width              = modes[m].Width;
              modeInfo.m_Height             = modes[m].Height;
              modeInfo.m_PixelFormat        = modes[m].Format;
              modeInfo.m_Behaviour          = Behavior[f];
              modeInfo.m_DepthStencilFormat = fmtDepthStencil[f];

              Log( "Renderer.Full", "Possible Mode %dx%dx%d", modeInfo.m_Width, modeInfo.m_Height, modeInfo.m_PixelFormat );

              Device.Modes.push_back( modeInfo );

              m_DisplayModes.push_back( XRendererDisplayMode( modeInfo.m_Width, modeInfo.m_Height, MapFormat( modeInfo.m_PixelFormat ) ) );
              if ( Device.DeviceType == D3DDEVTYPE_HAL )
              {
                bHALIsSampleCompatible = TRUE;
              }
            }
          }
        }
      }

      // Select any 640x480 mode for default (but prefer a 16-bit mode)
      SelectDefaultMode( adapter, &Adapter, &Device );

      // If valid modes were found, keep this device
      if ( Device.Modes.size() )
      {
        Adapter.Devices.push_back( Device );
      }
    }

    // If valid devices were found, keep this adapter
    if ( Adapter.Devices.size() > 0 )
    {
      m_Adapters.push_back( Adapter );
    }
  }

  // Return an error if no compatible devices were found
  if ( m_Adapters.empty() )
  {
    return 1;//D3DAPPERR_NOCOMPATIBLEDEVICES;
  }

  // hier erlauben wir, bei nicht möglichem Im-Fenster auf Fullscreen zu gehen
  if ( ( m_Windowed )
  &&   ( !m_WindowedModePossible ) )
  {
    m_Windowed = false;
  }

  // Pick a default device that can render into a window
  // (This code assumes that the HAL device comes before the REF
  // device in the device array).
  for ( DWORD a = 0; a < m_Adapters.size(); a++ )
  {
    tD3DAdapterInfo&   AdapterInfo = m_Adapters[a];

    for ( DWORD d = 0; d < AdapterInfo.Devices.size(); d++ )
    {
      tD3DDeviceInfo&    DeviceInfo  = AdapterInfo.Devices[d];

      if ( DeviceInfo.Windowed == m_Windowed )
      {
        m_Adapters[a].CurrentDevice = d;
        m_Adapter = a;
        return S_OK;
      }
    }
  }

  // kein Device gefunden, das im Fenster rendern kann
  return 2;
}



HRESULT DX82dRenderer::Initialize3DEnvironment()
{
  if ( m_pD3D == NULL )
  {
    return E_FAIL;
  }

  if ( m_Adapter >= m_Adapters.size() )
  {
    dh::Warning( "[DX8][Initialize3DEnvironment] Adapter out of bounds (%d > %d)",
                 m_Adapter,
                 m_Adapters.size() );
    return E_FAIL;
  }

  HRESULT hr;


  tD3DAdapterInfo* pAdapterInfo = &m_Adapters[m_Adapter];

  if ( pAdapterInfo->CurrentDevice >= pAdapterInfo->Devices.size() )
  {
    dh::Warning( "[DX8][Initialize3DEnvironment] Device out of bounds (%d > %d)",
                 pAdapterInfo->CurrentDevice,
                 pAdapterInfo->Devices.size() );
    return E_FAIL;
  }

  tD3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->Devices[pAdapterInfo->CurrentDevice];


  m_CurrentAdapter  = m_Adapter;
  m_pCurrentModeInfo  = &pDeviceInfo->Modes[pDeviceInfo->CurrentMode];

  m_CurrentDevice   = pAdapterInfo->CurrentDevice;
  m_CurrentMode     = pDeviceInfo->CurrentMode;
                                  
  pDeviceInfo->Windowed  = m_Windowed;

  // Set up the presentation parameters
  ZeroMemory( &m_d3dpp, sizeof( m_d3dpp ) );
  m_d3dpp.Windowed               = pDeviceInfo->Windowed;
  m_d3dpp.BackBufferCount        = 1;
  m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
  m_d3dpp.EnableAutoDepthStencil = m_UseDepthBuffer;
  m_d3dpp.AutoDepthStencilFormat = m_pCurrentModeInfo->m_DepthStencilFormat;
  m_d3dpp.hDeviceWindow          = m_hwndViewport;
  if ( m_Windowed )
  {
    //AdjustWindowForMode( m_hwndViewport, true );

    RECT    rcWindowClient;

    GetClientRect( m_hwndViewport, &rcWindowClient );
    m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
    m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
  }
  else
  {
    m_d3dpp.BackBufferWidth  = m_CreationWidth;
    m_d3dpp.BackBufferHeight = m_CreationHeight;

    /*
    m_d3dpp.BackBufferWidth  = m_pCurrentModeInfo->m_Width;
    m_d3dpp.BackBufferHeight = m_pCurrentModeInfo->m_Height;
    */
    m_d3dpp.BackBufferFormat = m_pCurrentModeInfo->m_PixelFormat;
    if ( !m_VSyncEnabled )
    {
      m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    // Fenster anpassen
    //AdjustWindowForMode( m_hwndViewport, false );

    //SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    m_hwndWindowedModeParent = GetParent( m_hwndViewport );
    SetParent( m_hwndViewport, NULL );
  }

  if ( ( m_d3dpp.BackBufferHeight == 0 )
  ||   ( m_d3dpp.BackBufferWidth == 0 ) )
  {
    return E_FAIL;
  }

  // Create the device
  DWORD   Behaviour = m_pCurrentModeInfo->m_Behaviour;
  //Behaviour = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  if ( m_MultiThreadSafe )
  {
    Behaviour |= D3DCREATE_MULTITHREADED;
  }

  hr = m_pD3D->CreateDevice( m_Adapter, pDeviceInfo->DeviceType,
                             m_hWndFocus, Behaviour, &m_d3dpp,
                             &m_pd3dDevice );
  if ( SUCCEEDED( hr ) )
  {
    m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 0.0f, 0 );
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );

    m_RenderWidth = m_d3dpp.BackBufferWidth;
    m_RenderHeight = m_d3dpp.BackBufferHeight;

    // den Viewport aufbewahren (für Pure-Devices)
    // das sind die Default-Werte (laut Dokumentation)
    m_ViewPort.X         = 0;
    m_ViewPort.Y         = 0;
    m_ViewPort.Width     = m_RenderWidth;
    m_ViewPort.Height    = m_RenderHeight;
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
    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
    pBackBuffer->Release();

    // Initialize the app's device-dependent objects
    hr = InitDeviceObjects();
    if ( SUCCEEDED( hr ) )
    {
      m_Active = TRUE;
      return S_OK;
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
  if ( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
  {
    // Select the default adapter
    m_Adapter = 0L;
    pAdapterInfo = &m_Adapters[m_Adapter];

    // Look for a software device
    for ( UINT i = 0L; i < pAdapterInfo->Devices.size(); i++ )
    {
      if ( pAdapterInfo->Devices[i].DeviceType == D3DDEVTYPE_REF )
      {
        pAdapterInfo->CurrentDevice = i;
        pDeviceInfo = &pAdapterInfo->Devices[i];
        m_Windowed = pDeviceInfo->Windowed;
        break;
      }
    }

    // Try again, this time with the reference rasterizer
    if ( pAdapterInfo->Devices[pAdapterInfo->CurrentDevice].DeviceType == D3DDEVTYPE_REF )
    {
      hr = Initialize3DEnvironment();
      if ( FAILED( hr ) )
      {
        dh::Log( "Initialize3DEnvironment: 2nd time failed" );
      }
    }
  }

  return hr;
}



void DX82dRenderer::Cleanup3DEnvironment()
{
  m_Active = FALSE;
  m_Ready  = FALSE;

  if ( m_pd3dDevice )
  {
    for ( GR::u32 i = 0; i < 8; ++i )
    {
      SetTexture( i, NULL );
    }
    m_ColorKeyedTextures.clear();
    DestroyAllTextures();
    DestroyAllFonts();
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

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
  UnregisterClass( "XtremeDX8Holder", m_hInstance );
}



bool DX82dRenderer::IsReady() const
{
  return m_Ready;
}



HRESULT DX82dRenderer::ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT )   
{ 
  return S_OK; 
}



BOOL DX82dRenderer::FindDepthStencilFormat( int iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
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



void DX82dRenderer::SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice )
{
  if ( !pDevice ) 
  {
    dh::Log( "no device\n" );
    return;
  }

  pDevice->CurrentMode = 0;
  for ( unsigned int m = 0; m < pDevice->Modes.size(); m++ )
  {
    tD3DModeInfo&    ModeInfo = pDevice->Modes[m];

    if ( ( ModeInfo.m_Width == 640 )
    &&   ( ModeInfo.m_Height == 480 ) )
    {
      
      if ( ( m_CreationDepth == 24 )
      ||   ( m_CreationDepth == 32 ) )
      {
        if ( ( ModeInfo.m_PixelFormat == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_X8R8G8B8 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = 640;
          m_FullscreenDisplayMode.Height    = 480;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_PixelFormat );
          m_FullscreenDisplayMode.FullScreen = true;
          break;
        }
      }
      if ( m_CreationDepth == 16 )
      {
        if ( ( ModeInfo.m_PixelFormat == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = 640;
          m_FullscreenDisplayMode.Height    = 480;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_PixelFormat );
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
    for ( unsigned int m = 0; m < pDevice->Modes.size(); m++ )
    {
      tD3DModeInfo&    ModeInfo = pDevice->Modes[m];

      Log( "Renderer.Full", CMisc::printf( "SelectDefaultMode  mode %dx%dx%d", ModeInfo.m_Width, ModeInfo.m_Height, ModeInfo.m_PixelFormat ) );

      if ( ( ModeInfo.m_Width == m_CreationWidth )
      &&   ( ModeInfo.m_Height == m_CreationHeight ) )
      {
        if ( ( ModeInfo.m_PixelFormat == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_X8R8G8B8 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_R5G6B5 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_X1R5G5B5 )
        ||   ( ModeInfo.m_PixelFormat == D3DFMT_A1R5G5B5 ) )
        {
          pDevice->CurrentMode = m;

          m_FullscreenDisplayMode.Width     = m_CreationWidth;
          m_FullscreenDisplayMode.Height    = m_CreationHeight;
          m_FullscreenDisplayMode.ImageFormat = MapFormat( ModeInfo.m_PixelFormat );
          m_FullscreenDisplayMode.FullScreen = true;
          Log( "Renderer.Full", "Keep this mode 16/24/32" );
          break;
        }
      }
    }
  }

  // kann im aktuellen Desktop-Modus im Fenster gerendert werden?
  if ( m_Windowed )
  {
    if ( pDevice->CurrentMode >= pDevice->Modes.size() )
    {
      dh::Log( "CurrentMode out of bounds (%d > %d)", pDevice->CurrentMode, pDevice->Modes.size() );
    }
    else
    {
      tD3DModeInfo&    ModeInfo = pDevice->Modes[pDevice->CurrentMode];

      if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapterNr, 
                                              pDevice->DeviceType, 
                                              pAdapter->d3ddmDesktop.Format, 
                                              ModeInfo.m_PixelFormat,
                                              FALSE ) ) )
      {
        m_WindowedModePossible = true;
      }
    }
  }
}



bool DX82dRenderer::ResizeSurface()
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

  if ( ( m_Active )
  &&   ( m_Windowed ) )
  {
    RECT rcWindowClient;

    GetClientRect( m_hwndViewport, &rcWindowClient );

    //MoveWindow( m_hwndViewport, 0, 0, rcWindowClient.right - rcWindowClient.left, rcWindowClient.bottom - rcWindowClient.top, TRUE );
    // Update window properties
    // A new window size will require a new backbuffer
    // size, so the 3D structures must be changed accordingly.
    HRESULT     hr;

    m_Ready = FALSE;

    m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
    m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;

    GetWindowPlacement( m_hwndViewport, &m_wPlacement );

    // Resize the 3D environment
    if ( FAILED( hr = Resize3DEnvironment() ) )
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



HRESULT DX82dRenderer::Resize3DEnvironment()
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

  for ( int i = 0; i < 8; ++i )
  {
    SetTexture( i, NULL );
  }
  ReleaseAllTextures();

  // Reset the device
  if ( ( !m_d3dpp.BackBufferWidth )
  ||   ( !m_d3dpp.BackBufferHeight ) )
  {
    dh::Log( "Resize3DEnvironment: Invalid BackBuffer Size" );
    return E_FAIL;
  }

  if ( ( !m_Windowed )
  &&   ( m_d3dpp.Windowed ) )
  {
    // jetzt nach Fullscreen
    m_d3dpp.BackBufferWidth   = m_FullscreenDisplayMode.Width;
    m_d3dpp.BackBufferHeight  = m_FullscreenDisplayMode.Height;
    m_d3dpp.BackBufferFormat  = MapFormat( m_FullscreenDisplayMode.ImageFormat );

    AdjustWindowForMode( m_Windowed );
  }
  else   if ( ( m_Windowed )
  &&          ( !m_d3dpp.Windowed ) )
  {
    AdjustWindowForMode( m_Windowed );

    RECT    rc;
    GetClientRect( m_hwndViewport, &rc );

    m_d3dpp.BackBufferWidth  = rc.right - rc.left;
    m_d3dpp.BackBufferHeight = rc.bottom - rc.top;

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = m_Adapters[m_CurrentAdapter].d3ddmDesktop.Format;
  }

  m_d3dpp.Windowed = m_Windowed;

  if ( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
  {
    dh::Log( "Resize3DEnvironment: m_pd3dDevice->Reset failed (%x)", hr );

    dh::Log( "D3DERR_INVALIDCALL %x", D3DERR_INVALIDCALL );
    dh::Log( "D3DERR_OUTOFVIDEOMEMORY %x", D3DERR_OUTOFVIDEOMEMORY  );
    dh::Log( "E_OUTOFMEMORY %x", E_OUTOFMEMORY );

    return hr;
  }

  // Store render target surface desc
  LPDIRECT3DSURFACE8 pBackBuffer;
  m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
  pBackBuffer->GetDesc( &m_SurfaceDescBackBuffer );
  pBackBuffer->Release();

  m_RenderWidth   = m_SurfaceDescBackBuffer.Width;
  m_RenderHeight  = m_SurfaceDescBackBuffer.Height;

  // den Viewport aufbewahren (für Pure-Devices)
  // das sind die Default-Werte (laut Dokumentation)
  m_ViewPort.X         = 0;
  m_ViewPort.Y         = 0;
  m_ViewPort.Width     = m_RenderWidth;
  m_ViewPort.Height    = m_RenderHeight;
  m_ViewPort.MinZ      = 0.0f;
  m_ViewPort.MaxZ      = 1.0f;

  // Initialize the app's device-dependent objects
  hr = RestoreDeviceObjects();
  if ( FAILED( hr ) )
  {
    dh::Log( "Resize3DEnvironment: RestoreDeviceObjects failed" );
    return hr;
  }

  return S_OK;

}



bool DX82dRenderer::InitDeviceObjects()
{

  RestoreDeviceObjects();
  return true;

}



bool DX82dRenderer::RestoreDeviceObjects()
{

  RestoreAllTextures();
  RestoreAllFonts();

  SetTextureStageState( D3DTSS_MINFILTER, D3DTEXF_POINT );
  SetTextureStageState( D3DTSS_MAGFILTER, D3DTEXF_POINT );
  SetTextureStageState( D3DTSS_MIPFILTER, D3DTEXF_POINT );

  SetRenderState( D3DRS_LIGHTING, FALSE );
  SetRenderState( D3DRS_ZENABLE, FALSE );

  return true;

}



bool DX82dRenderer::InvalidateDeviceObjects()
{

  return true;

}



bool DX82dRenderer::DeleteDeviceObjects()
{

  return true;

}



bool DX82dRenderer::BeginScene()
{

  HRESULT hr = S_OK;

  if ( !m_pd3dDevice )
  {
    return false;
  }
  if ( !m_Ready )
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
      if ( m_Windowed )
      {
        tD3DAdapterInfo* pAdapterInfo = &m_Adapters[m_Adapter];
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



void DX82dRenderer::EndScene()
{

  if ( m_pd3dDevice )
  {
    m_pd3dDevice->EndScene();
  }

}



void DX82dRenderer::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  if ( !m_SaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_SaveScreenShotFileName.c_str() );

    m_SaveScreenShotFileName.clear();
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



XMultiTexture* DX82dRenderer::LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey )
{
  XMultiTexture2d*    pTexture = new XMultiTexture2d( this );

  if ( !pTexture->LoadImage( *m_pEnvironment, FileName ) )
  {
    delete pTexture;
    return NULL;
  }

  m_MultiTextures.push_back( pTexture );

  return pTexture;
}



XTexture* DX82dRenderer::LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  GR::String     path = FileName;

  IImageFormatManager*    pManager = (IImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }
  GR::Graphic::ImageData*   pData = pManager->LoadData( path.c_str() );
  if ( pData == NULL )
  {
    dh::Error( "DX82dRenderer: failed to load image data from file (%s)", path.c_str() );
    return NULL;
  }

  if ( pData->Data() == NULL )
  {
    dh::Error( "DX82dRenderer: image data contains no data after loading from file (%s)", path.c_str() );
  }

  // convert format
  if ( imgFormatToConvert != GR::Graphic::IF_UNKNOWN )
  {
    pData->ConvertSelfTo( imgFormatToConvert, 0, ColorKey != 0, ColorKey );
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
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;
}



XTexture* DX82dRenderer::CreateTexture( const GR::Graphic::ImageData& ImageData )
{

  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat() );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );

  pTexture->m_ColorKey = ImageData.TransparentColor();
  pTexture->m_ColorKeyReplacementColor = 0;

  return pTexture;

}



XTexture* DX82dRenderer::CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat imgFormat )
{

  if ( ( Width == 0 )
  ||   ( Height == 0 ) )
  {
    return NULL;
  }


  if ( !IsTextureFormatOK( imgFormat ) )
  {
    // unterstützte Wechsel
    if ( imgFormat == GR::Graphic::IF_R8G8B8 )
    {
      imgFormat = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat ) )
      {
        return NULL;
      }
    }
    else if ( imgFormat == GR::Graphic::IF_X1R5G5B5 )
    {
      imgFormat = GR::Graphic::IF_R5G6B5;
      if ( !IsTextureFormatOK( imgFormat ) )
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


  CDX8Texture*          pTexture = NULL;

  pTexture = new( std::nothrow )CDX8Texture( this );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  if ( FAILED( m_pd3dDevice->CreateTexture( 
                 iTWidth,
                 iTHeight,
                 1,//MipMapLevels,
                 0,
                 MapFormat( imgFormat ),
                 D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                 &pTexture->m_Surface ) ) )
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
  pTexture->m_PixelFormat   = ddsd.Format;
  pTexture->m_ImageFormat   = imgFormat;

  AddTexture( pTexture );

  return pTexture;
}



bool DX82dRenderer::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat )
{
  if ( imgFormat == GR::Graphic::IF_A8 )
  {
    // Wegen Font-Problemen in D3D nicht unterstützt!
    return false;
  }

  HRESULT hr = m_pD3D->CheckDeviceFormat( 
          m_CurrentAdapter,
          D3DDEVTYPE_HAL,
          m_pCurrentModeInfo->m_PixelFormat,
          0,
          D3DRTYPE_TEXTURE,
          MapFormat( imgFormat ) );

  return !!SUCCEEDED( hr );
}



D3DFORMAT DX82dRenderer::MapFormat( GR::Graphic::eImageFormat imgFormat )
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
      dh::Log( "DX82dRenderer::MapFormat Unsupported format %d", imgFormat );
      break;
  }

  return d3dFmt;
}



GR::Graphic::eImageFormat DX82dRenderer::MapFormat( D3DFORMAT d3dFormat )
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
      dh::Log( "DX82dRenderer::MapFormat Unsupported D3D format %d", d3dFormat );
      break;
  }

  return imgFmt;
}



LPDIRECT3DDEVICE8 DX82dRenderer::Device()
{
  return m_pd3dDevice;
}



void DX82dRenderer::SetTexture( GR::u32 Stage, XTexture* pTexture )
{

  if ( m_pCurTexture[Stage] == pTexture )
  {
    return;
  }
  m_pd3dDevice->SetTexture( Stage, pTexture ? ( (CDX8Texture*)pTexture )->m_Surface : NULL );

}



bool DX82dRenderer::CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData )
{

  if ( pTexture == NULL )
  {
    return false;
  }

  D3DLOCKED_RECT      lockRect;

  if ( FAILED( ( (CDX8Texture*)pTexture )->m_Surface->LockRect( 0, &lockRect, NULL, 0 ) ) )
  {
    return false;
  }

  GR::Graphic::ContextDescriptor      cdTexture;
  
  cdTexture.Attach( pTexture->m_SurfaceSize.x, pTexture->m_SurfaceSize.y, lockRect.Pitch, pTexture->m_ImageFormat, lockRect.pBits );

  if ( !cdTexture.ConvertInto( &ImageData ) )
  {
    ( (CDX8Texture*)pTexture )->m_Surface->UnlockRect( 0 );
    return false;
  }

  ( (CDX8Texture*)pTexture )->m_Surface->UnlockRect( 0 );
  return true;

}



bool DX82dRenderer::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  if ( ( pTexture == NULL )
  ||   ( ( (CDX8Texture*)pTexture )->m_Surface == NULL ) )
  {
    return false;
  }

  D3DLOCKED_RECT      lockRect;

  if ( FAILED( ( (CDX8Texture*)pTexture )->m_Surface->LockRect( 0, &lockRect, NULL, 0 ) ) )
  {
    return false;
  }

  GR::Graphic::ContextDescriptor      cdTexture;
  
  cdTexture.Attach( pTexture->m_SurfaceSize.x, pTexture->m_SurfaceSize.y, lockRect.Pitch, pTexture->m_ImageFormat, lockRect.pBits );

  if ( !ImageData.ConvertInto( &cdTexture, ColorKey != 0, ColorKey ) )
  {
    ( (CDX8Texture*)pTexture )->m_Surface->UnlockRect( 0 );
    return false;
  }

  pTexture->m_ColorKey = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  ( (CDX8Texture*)pTexture )->m_Surface->UnlockRect( 0 );
  ( (CDX8Texture*)pTexture )->m_ImageData = ImageData;
  return true;

}



void DX82dRenderer::RestoreAllTextures()
{
  tTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    CDX8Texture*    pDX8Texture = (CDX8Texture*)pTexture;

    // sicher gehen
    pDX8Texture->Release();
      
    if ( SUCCEEDED( m_pd3dDevice->CreateTexture( 
                    pTexture->m_SurfaceSize.x,
                    pTexture->m_SurfaceSize.y,
                    1,//MipMapLevels,
                    0,
                    pDX8Texture->m_PixelFormat,
                    D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                    &pDX8Texture->m_Surface ) ) )
    {
      CopyDataToTexture( pTexture, pDX8Texture->m_ImageData );
    }
    else
    {
      dh::Log( "CreateTexture failed" );
    }

    ++it;
  }
}



X2dFont* DX82dRenderer::LoadFontSquare( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{

  X2dFont*   pFont = new X2dFont( this, m_pEnvironment );

  pFont->LoadFontSquare( FileName, Flags, TransparentColor );

  AddFont( pFont );

  return pFont;

}



bool DX82dRenderer::OnResized()
{
  return ResizeSurface();
}



bool DX82dRenderer::SetViewport( XViewport& Viewport )
{
  D3DVIEWPORT8    vPort;

  vPort.Width    = Viewport.Width;
  vPort.Height   = Viewport.Height;
  vPort.X        = Viewport.X;
  vPort.Y        = Viewport.Y;
  vPort.MinZ     = Viewport.MinZ;
  vPort.MaxZ     = Viewport.MaxZ;

  if ( vPort.X >= Width() )
  {
    vPort.X = Width();
    vPort.Width = 0;
  }
  if ( vPort.Y >= Height() )
  {
    vPort.Y = Height();
    vPort.Height = 0;
  }
  if ( vPort.X + vPort.Width > Width() )
  {
    vPort.Width = Width() - vPort.X;
  }
  if ( vPort.Y + vPort.Height> Height() )
  {
    vPort.Height = Height() - vPort.Y;
  }

  m_ViewPort = Viewport;

  return SUCCEEDED( m_pd3dDevice->SetViewport( &vPort ) );

}



GR::u32 DX82dRenderer::Width()
{

  return m_RenderWidth;

}



GR::u32 DX82dRenderer::Height()
{

  return m_RenderHeight;

}



GR::u32 DX82dRenderer::Depth()
{

  // TODO - stimmt ja nicht immer
  return m_CreationDepth;

}



bool DX82dRenderer::IsFullscreen()
{

  return !m_Windowed;

}



bool DX82dRenderer::ToggleFullscreen()
{

  m_Windowed = !m_Windowed;

  return SUCCEEDED( Resize3DEnvironment() );

}



bool DX82dRenderer::SetMode( XRendererDisplayMode& DisplayMode )
{
  tDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&   DispMode = *it;
    if ( DispMode.FormatIsEqualTo( DisplayMode ) )
    {
      // der Mode existiert
      m_Windowed = false;
      m_FullscreenDisplayMode = DisplayMode;

      return SUCCEEDED( Resize3DEnvironment() );
    }

    ++it;
  }
  dh::Log( "DX82dRenderer::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.ImageFormat );
  return false;
}



LRESULT CALLBACK DX82dRenderer::DX8Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return g_pDX8Instance->WindowProc( hWnd, uMsg, wParam, lParam );
}



LRESULT CALLBACK DX82dRenderer::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch( uMsg )
  {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT:
      // Handle paint messages when the app is not ready
      if ( ( m_pd3dDevice )
      &&   ( !m_Ready ) )
      {
        if ( m_Windowed )
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



void DX82dRenderer::DoSaveSnapShot( const GR::String& szFile )
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
          pData = ( (DWORD*)lockRect.pBits ) + j * lockRect.Pitch / 4;
          //  lockRect.Pitch / 4 - ddsd.Width;
          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &Pixel, pData, 3 );

            File.WriteU8( unsigned char( ( Pixel & 0x0000ff )       ) );
            File.WriteU8( unsigned char( ( Pixel & 0x00ff00 ) >> 8  ) );
            File.WriteU8( unsigned char( ( Pixel & 0xff0000 ) >> 16 ) );

            ++pData;
          }
        }
        break;
      case D3DFMT_R8G8B8:
        {
          GR::u8*   pData8 = (GR::u8*)pData;

          pData8 += lockRect.Pitch + j * ddsd.Width * 3;

          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &Pixel, pData8, 3 );

            File.WriteU8( pData8[2] );
            File.WriteU8( pData8[1] );
            File.WriteU8( pData8[0] );

            pData8 += 3;
          }
        }
        break;
    }
  }
  File.Close();

  pDestSurface->UnlockRect();
  pDestSurface->Release();

}



bool DX82dRenderer::SaveScreenShot( const GR::String& FileName )
{
  if ( FileName )
  {
    m_SaveScreenShotFileName = FileName;
    return true;
  }
  return false;
}



void DX82dRenderer::RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                      GR::i32 X2, GR::i32 Y2,
                                      GR::i32 X3, GR::i32 Y3,
                                      GR::u32 Color )
{
  X1 += m_DisplayOffset.x;
  Y1 += m_DisplayOffset.y;
  X2 += m_DisplayOffset.x;
  Y2 += m_DisplayOffset.y;
  X3 += m_DisplayOffset.x;
  Y3 += m_DisplayOffset.y;

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  struct CUSTOMVERTEX
  {
    D3DXVECTOR3   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[3];

  float   fRHW = 1.0f;

  GR::tVector   ptPos1( (float)X1, (float)Y1, 0.0f );
  GR::tVector   ptPos2( (float)X2, (float)Y2, 0.0f );
  GR::tVector   ptPos3( (float)X3, (float)Y3, 0.0f );

  ptPos1.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos1.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos2.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos2.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos3.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos3.y *= m_CreationHeight / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  vertData[0].position.x  = ptPos1.x + m_DirectTexelMappingOffset.x;
  vertData[0].position.y  = ptPos1.y + m_DirectTexelMappingOffset.y;
  vertData[0].position.z  = (float)ptPos1.z;
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color;

  vertData[1].position.x  = ptPos2.x + m_DirectTexelMappingOffset.x;
  vertData[1].position.y  = ptPos2.y + m_DirectTexelMappingOffset.y;
  vertData[1].position.z  = (float)ptPos2.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color;

  vertData[2].position.x  = ptPos3.x + m_DirectTexelMappingOffset.x;
  vertData[2].position.y  = ptPos3.y + m_DirectTexelMappingOffset.y;
  vertData[2].position.z  = (float)ptPos3.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height, GR::u32 Color )
{
  X += m_DisplayOffset.x;
  Y += m_DisplayOffset.y;

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  if ( ( Color & 0xff000000 ) != 0xff000000 )
  {
    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  }
  else
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[4];

  float   fDelta = -0.5f;

  float   fRHW = 1.0f;

  GR::tVector   ptPos( (float)X, (float)Y, 0.0f );
  GR::tVector   ptSize( (float)Width, (float)Height, 0.0f );

  ptPos.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos.y *= m_CreationHeight / m_VirtualSize.y;
  ptSize.x *= m_CreationWidth / m_VirtualSize.x;
  ptSize.y *= m_CreationHeight / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  vertData[0].position.x  = ptPos.x + fDelta;
  vertData[0].position.y  = ptPos.y + fDelta;
  vertData[0].position.z  = (float)ptPos.z;
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color;

  vertData[1].position.x  = ptPos.x + ptSize.x + fDelta;
  vertData[1].position.y  = ptPos.y + fDelta;
  vertData[1].position.z  = (float)ptPos.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color;

  vertData[2].position.x  = ptPos.x + fDelta;
  vertData[2].position.y  = ptPos.y + ptSize.y + fDelta;
  vertData[2].position.z  = (float)ptPos.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color;

  vertData[3].position.x  = ptPos.x + ptSize.x + fDelta;
  vertData[3].position.y  = ptPos.y + ptSize.y + fDelta;
  vertData[3].position.z  = (float)ptPos.z;
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = Color;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void DX82dRenderer::RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                        GR::i32 MaskX, GR::i32 MaskY, 
                                        const XTextureSection& TexSecMask, 
                                        const GR::u32 Color, 
                                        const GR::u32 MaskColorKey )
{
  XTextureSection     TexSectionMask( TexSecMask );

  // Textur extra laden für anderen Colorkey?
  if ( TexSecMask.m_pTexture )
  {
    if ( TexSecMask.m_pTexture->m_ColorKey != MaskColorKey )
    {
      GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSecMask.m_pTexture << MaskColorKey;
      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

      if ( itCK != m_ColorKeyedTextures.end() )
      {
        TexSectionMask.m_pTexture = itCK->second;
      }
      else
      {
        // neue Textur mit Colorkey erzeugen
        CDX8Texture*          pTex = (CDX8Texture*)TexSectionMask.m_pTexture;
        GR::Graphic::ContextDescriptor    cdNewData( pTex->m_ImageData );

        GR::Graphic::eImageFormat    fmtToUse = cdNewData.ImageFormat();

        if ( !GR::Graphic::ImageData::FormatHasAlpha( fmtToUse ) )
        {
          switch ( fmtToUse )
          {
            case GR::Graphic::IF_X1R5G5B5:
            case GR::Graphic::IF_R5G6B5:
              fmtToUse = GR::Graphic::IF_A1R5G5B5;
              break;
            default:
              fmtToUse = GR::Graphic::IF_A8R8G8B8;
              break;
          }
        }

        cdNewData.ConvertSelfTo( fmtToUse, cdNewData.LineOffsetInBytes(), true, MaskColorKey );

        /*
        CFileStream   ioOut;

        ioOut.Open( CMisc::AppPath( "masked.bin" ), IIOStream::OT_WRITE_ONLY );
        ioOut.WriteBlock( cdNewData.Data(), cdNewData.DataSize() );
        ioOut.Close();
        */

        XTexture*   pNewTex = CreateTexture( cdNewData );

        m_ColorKeyedTextures[strEntry] = pNewTex;

        TexSectionMask.m_pTexture = pNewTex;
      }
    }
  }

  // überlappendes Rect finden
  GR::tRect     rcSection( X, Y, Width, Height );
  GR::tRect     rcMask( MaskX, MaskY, TexSectionMask.m_Width, TexSectionMask.m_Height );

  GR::tRect     rcOverlap( rcSection.Intersection( rcMask ) );

  if ( rcOverlap.Empty() )
  {
    return;
  }

  XTextureSection     TS2( TexSectionMask );

  TS2.m_XOffset += rcOverlap.Left - MaskX;
  TS2.m_YOffset += rcOverlap.Top - MaskY;

  Width = rcOverlap.Width();
  Height = rcOverlap.Height();
  TS2.m_Width = rcOverlap.Width();
  TS2.m_Height = rcOverlap.Height();

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  // Alpha-Map
  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1, 1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE, 1 );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1, 1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE, 1 );

  SetTextureStageState( D3DTSS_COLOROP,  D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP,  D3DTOP_DISABLE, 1 );

  SetTexture( 0, TS2.m_pTexture );

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TS2.GetTrueUV( UV1, UV2, UV3, UV4, TS2.m_Flags );

  struct CUSTOMVERTEX
  {
    GR::tVector   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
    float         fTU,
                  fTV;
  };

  CUSTOMVERTEX          vertData[4];

  GR::tVector   ptPos( (float)rcOverlap.Left, (float)rcOverlap.Top, 0.0f );
  GR::tVector   ptSize( (float)rcOverlap.Width(), (float)rcOverlap.Height(), 0.0f );

  ptPos.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos.y *= m_CreationHeight / m_VirtualSize.y;
  ptSize.x *= m_CreationWidth / m_VirtualSize.x;
  ptSize.y *= m_CreationHeight / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[0].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[0].position.z  = (float)ptPos.z;
  vertData[0].color       = Color;
  vertData[0].fRHW        = 1.0f;
  vertData[0].fTU         = UV1.x;
  vertData[0].fTV         = UV1.y;

  vertData[1].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[1].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[1].position.z  = (float)ptPos.z;
  vertData[1].color       = Color;
  vertData[1].fRHW        = 1.0f;
  vertData[1].fTU         = UV2.x;
  vertData[1].fTV         = UV2.y;

  vertData[2].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[2].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[2].position.z  = (float)ptPos.z;
  vertData[2].color       = Color;
  vertData[2].fRHW        = 1.0f;
  vertData[2].fTU         = UV3.x;
  vertData[2].fTV         = UV3.y;

  vertData[3].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[3].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[3].position.z  = (float)ptPos.z;
  vertData[3].color       = Color;
  vertData[3].fRHW        = 1.0f;
  vertData[3].fTU         = UV4.x;
  vertData[3].fTV         = UV4.y;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::RenderQuad2d( const GR::tVector& Pos1,
                                    const GR::tVector& Pos2,
                                    const GR::tVector& Pos3,
                                    const GR::tVector& Pos4,
                                    GR::f32 TU1, GR::f32 TV1,
                                    GR::f32 TU2, GR::f32 TV2,
                                    GR::f32 TU3, GR::f32 TV3,
                                    GR::f32 TU4, GR::f32 TV4,
                                    GR::u32 Color1, GR::u32 Color2, 
                                    GR::u32 Color3, GR::u32 Color4 )
{
  GR::tVector   Offset( (float)m_DisplayOffset.x, (float)m_DisplayOffset.y, 0 );

  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  struct CUSTOMVERTEX
  {
    GR::tVector   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
    float         fTU,
                  fTV;
  };

  CUSTOMVERTEX          vertData[4];


  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position    = Pos1 + Offset;
  vertData[0].color       = Color1;
  vertData[0].fRHW        = 1.0f;
  vertData[0].fTU         = TU1;
  vertData[0].fTV         = TV1;

  vertData[1].position    = Pos2 + Offset;
  vertData[1].color       = Color2;
  vertData[1].fRHW        = 1.0f;
  vertData[1].fTU         = TU2;
  vertData[1].fTV         = TV2;

  vertData[2].position    = Pos3 + Offset;
  vertData[2].color       = Color3;
  vertData[2].fRHW        = 1.0f;
  vertData[2].fTU         = TU3;
  vertData[2].fTV         = TV3;

  vertData[3].position    = Pos4 + Offset;
  vertData[3].color       = Color4;
  vertData[3].fRHW        = 1.0f;
  vertData[3].fTU         = TU4;
  vertData[3].fTV         = TV4;

  for ( int i = 0; i < 4; ++i )
  {
    vertData[0].position.x *= m_CreationWidth / m_VirtualSize.x;
    vertData[0].position.y *= m_CreationHeight / m_VirtualSize.y;
  }

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                    GR::f32 TU1, GR::f32 TV1,
                                    GR::f32 TU2, GR::f32 TV2,
                                    GR::f32 TU3, GR::f32 TV3,
                                    GR::f32 TU4, GR::f32 TV4,
                                    GR::u32 Color1, GR::u32 Color2, 
                                    GR::u32 Color3, GR::u32 Color4 )
{
  X += m_DisplayOffset.x;
  Y += m_DisplayOffset.y;

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

  GR::tVector   ptPos( (float)X, (float)Y, 0.0f );
  GR::tVector   ptSize( (float)Width, (float)Height, 0.0f );

  ptPos.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos.y *= m_CreationHeight / m_VirtualSize.y;
  ptSize.x *= m_CreationWidth / m_VirtualSize.x;
  ptSize.y *= m_CreationHeight / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[0].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[0].position.z  = (float)ptPos.z;
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = Color1;
  vertData[0].fTU         = TU1;
  vertData[0].fTV         = TV1;

  vertData[1].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[1].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[1].position.z  = (float)ptPos.z;
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = Color2;
  vertData[1].fTU         = TU2;
  vertData[1].fTV         = TV2;

  vertData[2].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[2].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[2].position.z  = (float)ptPos.z;
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = Color3;
  vertData[2].fTU         = TU3;
  vertData[2].fTV         = TV3;

  vertData[3].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[3].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[3].position.z  = (float)ptPos.z;
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = Color4;
  vertData[3].fTU         = TU4;
  vertData[3].fTV         = TV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color )
{
  if ( TexSection.m_pTexture == NULL )
  {
    RenderTextureSectionColorized( X, Y, TexSection, Color );
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



void DX82dRenderer::RenderTextureSection( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );

  ( (const XMultiTexture2d*)pTexture )->Render2d( *this, X, Y );
}



void DX82dRenderer::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture, GR::u32 Color )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  ( (const XMultiTexture2d*)pTexture )->Render2d( *this, X, Y, Color );
}



void DX82dRenderer::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                const XMultiTexture* pTexture,
                                                                GR::i32 AlphaValue )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_TEXTURE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  ( (const XMultiTexture2d*)pTexture )->Render2d( *this, X, Y, 0x00ffffff + ( AlphaValue << 24 ) );
}



void DX82dRenderer::RenderTextureSection( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );

  SetTexture( 0, TexSection.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;
  /*
  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };

  if ( AlternativeFlags & XTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( AlternativeFlags & XTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }*/


  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  /*
  if ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[0][1];
    fTV[0][0] = fTV[0][1];
    fTU[0][1] = fTU[1][1];
    fTV[0][1] = fTV[1][1];
    fTU[1][1] = fTU[1][0];
    fTV[1][1] = fTV[1][0];
    fTU[1][0] = fTempTU;
    fTV[1][0] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_90 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[1][0];
    fTV[0][0] = fTV[1][0];
    fTU[1][0] = fTU[1][1];
    fTV[1][0] = fTV[1][1];
    fTU[1][1] = fTU[0][1];
    fTV[1][1] = fTV[0][1];
    fTU[0][1] = fTempTU;
    fTV[0][1] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }
  */

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, AlternativeFlags );

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                0xffffffff );
}



void DX82dRenderer::RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  SetTexture( 0, TexSection.m_pTexture );

  int width  = TexSection.m_Width;
  int height = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  /*
  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( AlternativeFlags & XTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( AlternativeFlags & XTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }*/


  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( width, height );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= width / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= height / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= height - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= width - 1;
  }

  /*
  if ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[0][1];
    fTV[0][0] = fTV[0][1];
    fTU[0][1] = fTU[1][1];
    fTV[0][1] = fTV[1][1];
    fTU[1][1] = fTU[1][0];
    fTV[1][1] = fTV[1][0];
    fTU[1][0] = fTempTU;
    fTV[1][0] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_90 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[1][0];
    fTV[0][0] = fTV[1][0];
    fTU[1][0] = fTU[1][1];
    fTV[1][0] = fTV[1][1];
    fTU[1][1] = fTU[0][1];
    fTV[1][1] = fTV[0][1];
    fTU[0][1] = fTempTU;
    fTV[0][1] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }
  
  RenderQuad2d( iX, iY,
                iWidth, iHeight,
                fTU[0][0], fTV[0][0],
                fTU[1][0], fTV[1][0],
                fTU[0][1], fTV[0][1],
                fTU[1][1], fTV[1][1],
                Color );*/
  RenderQuad2d( X, Y,
                width, height,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color ); 
}



void DX82dRenderer::RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                  const XTextureSection& TexSection, GR::u32 Color,
                                  float Angle, float ZoomX, float ZoomY )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  SetTexture( 0, TexSection.m_pTexture );

  int width  = TexSection.m_Width;
  int height = TexSection.m_Height;

  GR::u32  alternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );
  /*
  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( AlternativeFlags & XTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( AlternativeFlags & XTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }*/


  if ( ( ( alternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( alternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( width, height );
  }

  if ( alternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= width / 2;
  }
  if ( alternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= height / 2;
  }
  if ( alternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= height - 1;
  }
  if ( alternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= width - 1;
  }
  /*
  if ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[0][1];
    fTV[0][0] = fTV[0][1];
    fTU[0][1] = fTU[1][1];
    fTV[0][1] = fTV[1][1];
    fTU[1][1] = fTU[1][0];
    fTV[1][1] = fTV[1][0];
    fTU[1][0] = fTempTU;
    fTV[1][0] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_90 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[1][0];
    fTV[0][0] = fTV[1][0];
    fTU[1][0] = fTU[1][1];
    fTV[1][0] = fTV[1][1];
    fTU[1][1] = fTU[0][1];
    fTV[1][1] = fTV[0][1];
    fTU[0][1] = fTempTU;
    fTV[0][1] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }*/

  GR::tVector   vDelta( -width * 0.5f * ZoomX, 
                        -height * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta2( width * 0.5f * ZoomX,
                        -height * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta3( -width * 0.5f * ZoomX,
                        height * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta4( width * 0.5f * ZoomX,
                        height * 0.5f * ZoomY,
                        0.0f );

  vDelta.RotateZ( -Angle );
  vDelta2.RotateZ( -Angle );
  vDelta3.RotateZ( -Angle );
  vDelta4.RotateZ( -Angle );

  GR::tVector   ptPos1 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta;
  GR::tVector   ptPos2 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta2;
  GR::tVector   ptPos3 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta3;
  GR::tVector   ptPos4 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta4;

  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  RenderQuad2d( ptPos1, ptPos2, ptPos3, ptPos4,
                UV1.x, UV1.y, 
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}



void DX82dRenderer::RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTexture( 0, TexSection.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );
  /*
  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( AlternativeFlags & XTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( AlternativeFlags & XTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }*/


  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  /*
  if ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[0][1];
    fTV[0][0] = fTV[0][1];
    fTU[0][1] = fTU[1][1];
    fTV[0][1] = fTV[1][1];
    fTU[1][1] = fTU[1][0];
    fTV[1][1] = fTV[1][0];
    fTU[1][0] = fTempTU;
    fTV[1][0] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_90 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[1][0];
    fTV[0][0] = fTV[1][0];
    fTU[1][0] = fTU[1][1];
    fTV[1][0] = fTV[1][1];
    fTU[1][1] = fTU[0][1];
    fTV[1][1] = fTV[0][1];
    fTU[0][1] = fTempTU;
    fTV[0][1] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }*/

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}



void DX82dRenderer::RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::i32 Alpha )
{
  SetTexture( 0, TexSection.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  /*
  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( AlternativeFlags & XTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( AlternativeFlags & XTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }*/


  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  /*
  if ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[0][1];
    fTV[0][0] = fTV[0][1];
    fTU[0][1] = fTU[1][1];
    fTV[0][1] = fTV[1][1];
    fTU[1][1] = fTU[1][0];
    fTV[1][1] = fTV[1][0];
    fTU[1][0] = fTempTU;
    fTV[1][0] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_90 )
  {
    float   fTempTU = fTU[0][0];
    float   fTempTV = fTV[0][0];

    fTU[0][0] = fTU[1][0];
    fTV[0][0] = fTV[1][0];
    fTU[1][0] = fTU[1][1];
    fTV[1][0] = fTV[1][1];
    fTU[1][1] = fTU[0][1];
    fTV[1][1] = fTV[0][1];
    fTU[0][1] = fTempTU;
    fTV[0][1] = fTempTV;
  }
  else if ( AlternativeFlags & XTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }*/

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_TEXTURE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                ( Alpha << 24 ) + 0xffffff );
}




void DX82dRenderer::RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::i32 Alpha, GR::u32 ColorKey )
{
  XTextureSection     TexSec( TexSection );

  // Textur extra laden für anderen Colorkey?
  if ( TexSection.m_pTexture )
  {
    if ( TexSection.m_pTexture->m_ColorKey != ColorKey )
    {
      GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

      if ( itCK != m_ColorKeyedTextures.end() )
      {
        TexSec.m_pTexture = itCK->second;
      }
      else
      {
        // neue Textur mit Colorkey erzeugen
        XTexture*   pNewTex = LoadTexture( TexSec.m_pTexture->m_LoadedFromFile.c_str(), GR::Graphic::IF_UNKNOWN, ColorKey );

        m_ColorKeyedTextures[strEntry] = pNewTex;

        TexSec.m_pTexture = pNewTex;
      }
    }
  }

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSec.m_Width;
  int iHeight = TexSec.m_Height;

  GR::u32  AlternativeFlags = TexSec.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_TEXTURE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y, 
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                ( Alpha << 24 ) + 0xffffff );
}



void DX82dRenderer::RenderRect( const GR::tPoint& pt1, const GR::tPoint& ptSize, GR::u32 Color1 )
{
  SetTexture( 0, NULL );
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );

  RenderQuad2d( pt1.x, pt1.y, ptSize.x, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Color1 );
  RenderQuad2d( pt1.x + ptSize.x - 1, pt1.y, 1,  ptSize.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Color1 );
  RenderQuad2d( pt1.x, pt1.y + ptSize.y - 1, ptSize.x, 1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Color1 ); 
  RenderQuad2d( pt1.x, pt1.y, 1, ptSize.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, Color1 );
}




void DX82dRenderer::RenderLine( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  SetRenderState( D3DRS_LASTPIXEL, TRUE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
  SetTextureStageState( D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

  struct CUSTOMVERTEX
  {
    D3DXVECTOR3   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[3];


  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );


  D3DXVECTOR3   Delta( (float)m_DisplayOffset.x + m_DirectTexelMappingOffset.x, (float)m_DisplayOffset.y + m_DirectTexelMappingOffset.y, 0.0f );

  vertData[0].position = D3DXVECTOR3( (float)pt1.x, (float)pt1.y, 0.0f ) + Delta;
  vertData[0].color = Color;

  vertData[1].position = D3DXVECTOR3( (float)pt2.x, (float)pt2.y, 0.0f ) + Delta;
  vertData[1].color = Color;

  vertData[2].position = D3DXVECTOR3( (float)pt2.x + 1, (float)pt2.y + 1, 0.0f ) + Delta;
  vertData[2].color = 0xff00ff00;

  vertData[2].fRHW = vertData[0].fRHW = vertData[1].fRHW = 1.0f;

  for ( int i = 0; i < 3; ++i )
  {
    vertData[i].position.x *= m_CreationWidth / m_VirtualSize.x;
    vertData[i].position.y *= m_CreationHeight / m_VirtualSize.y;
  }
  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 ColorKey,
                                                      bool bDoNotCreateNewIfNeeded )
{
  XTextureSection     TexSec( TexSection );

  // TODO - Textur extra laden für anderen Colorkey?
  if ( TexSection.m_pTexture )
  {
    if ( TexSection.m_pTexture->m_ColorKey != ColorKey )
    {
      GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

      if ( itCK != m_ColorKeyedTextures.end() )
      {
        TexSec.m_pTexture = itCK->second;
      }
      else
      {
        // neue Textur mit Colorkey erzeugen
        GR::Graphic::ImageData    ImageData;

        GR::Graphic::eImageFormat  fmtToUse = TexSec.m_pTexture->m_ImageFormat;

        if ( !GR::Graphic::ImageData::FormatHasAlpha( TexSec.m_pTexture->m_ImageFormat ) )
        {
          switch ( TexSec.m_pTexture->m_ImageFormat )
          {
            case GR::Graphic::IF_X1R5G5B5:
            case GR::Graphic::IF_R5G6B5:
              fmtToUse = GR::Graphic::IF_A1R5G5B5;
              break;
            default:
              fmtToUse = GR::Graphic::IF_A8R8G8B8;
              break;
          }
        }
        
        ImageData.CreateData( TexSec.m_pTexture->m_SurfaceSize.x, 
                              TexSec.m_pTexture->m_SurfaceSize.y,
                              fmtToUse );

        if ( !( (CDX8Texture*)TexSec.m_pTexture )->m_ImageData.ConvertInto( &ImageData, true, ColorKey ) )
        {
          dh::Log( "DX82dRenderer::RenderTextureSectionColorKeyed Colorkeying failed" );
        }

        XTexture*   pNewTex = CreateTexture( ImageData );

        m_ColorKeyedTextures[strEntry] = pNewTex;

        TexSec.m_pTexture = pNewTex;
      }
    }
  }

  if ( ( TexSec.m_pTexture )
  &&   ( ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A8 )
  ||     ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 ) 
  ||     ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 ) ) )
  {
    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  }
  else
  {
    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  }

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSec.m_Width;
  int iHeight = TexSec.m_Height;

  GR::u32  AlternativeFlags = TexSec.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, AlternativeFlags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                0xffffffff );
}



void DX82dRenderer::RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 ColorKey,
                                                               GR::u32 Color,
                                                               bool bDoNotCreateNewIfNeeded )
{
  XTextureSection     TexSec( TexSection );

  // TODO - Textur extra laden für anderen Colorkey?
  if ( TexSection.m_pTexture )
  {
    if ( TexSection.m_pTexture->m_ColorKey != ColorKey )
    {
      GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

      if ( itCK != m_ColorKeyedTextures.end() )
      {
        TexSec.m_pTexture = itCK->second;
      }
      else
      {
        // neue Textur mit Colorkey erzeugen
        GR::Graphic::ContextDescriptor    ImageData;

        GR::Graphic::eImageFormat  fmtToUse = TexSec.m_pTexture->m_ImageFormat;

        if ( !GR::Graphic::ImageData::FormatHasAlpha( TexSec.m_pTexture->m_ImageFormat ) )
        {
          switch ( TexSec.m_pTexture->m_ImageFormat )
          {
            case GR::Graphic::IF_X1R5G5B5:
            case GR::Graphic::IF_R5G6B5:
              fmtToUse = GR::Graphic::IF_A1R5G5B5;
              break;
            default:
              fmtToUse = GR::Graphic::IF_A8R8G8B8;
              break;
          }
        }
        
        ImageData.CreateData( TexSec.m_pTexture->m_SurfaceSize.x, 
                              TexSec.m_pTexture->m_SurfaceSize.y,
                              fmtToUse );

        if ( !( (CDX8Texture*)TexSec.m_pTexture )->m_ImageData.ConvertInto( &ImageData, true, ColorKey ) )
        {
          dh::Log( "DX82dRenderer::RenderTextureSectionColorKeyed Colorkeying failed" );
        }

        XTexture*   pNewTex = CreateTexture( ImageData );

        m_ColorKeyedTextures[strEntry] = pNewTex;

        TexSec.m_pTexture = pNewTex;
      }
    }
  }

  if ( ( TexSec.m_pTexture )
  &&   ( ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A8 )
  ||     ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 ) 
  ||     ( TexSec.m_pTexture->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 )
  ||     ( ( Color & 0xff000000 ) != 0xff000000 ) ) )
  {
    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
  }
  else
  {
    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  }

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSec.m_Width;
  int iHeight = TexSec.m_Height;

  GR::u32  AlternativeFlags = TexSec.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}



void DX82dRenderer::RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  XTextureSection   TempSec( TexSection );

  TempSec.m_Flags |= XTextureSection::TSF_H_MIRROR;
  RenderTextureSection( X, Y, TempSec );
}



void DX82dRenderer::RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection )
{
  XTextureSection   TempSec( TexSection );

  TempSec.m_Flags |= XTextureSection::TSF_V_MIRROR;
  RenderTextureSection( X, Y, TempSec );
}



void DX82dRenderer::RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, float fAngle, float ZoomX, float ZoomY )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

  SetTexture( 0, TexSection.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  GR::tVector   vDelta( -iWidth * 0.5f * ZoomX,
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta2( iWidth * 0.5f * ZoomX,
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta3( -iWidth * 0.5f * ZoomX,
                        iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta4( iWidth * 0.5f * ZoomX,
                        iHeight * 0.5f * ZoomY,
                        0.0f );

  vDelta.RotateZ( -fAngle );
  vDelta2.RotateZ( -fAngle );
  vDelta3.RotateZ( -fAngle );
  vDelta4.RotateZ( -fAngle );

  GR::tVector   ptPos1 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta;
  GR::tVector   ptPos2 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta2;
  GR::tVector   ptPos3 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta3;
  GR::tVector   ptPos4 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta4;

  ptPos1.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos1.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos2.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos2.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos3.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos3.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos4.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos4.y *= m_CreationHeight / m_VirtualSize.y;

  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  RenderQuad2d( ptPos1, ptPos2, ptPos3, ptPos4,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                0xffffffff );
}



void DX82dRenderer::RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, float Angle, float ZoomX, float ZoomY, GR::u32 ColorKey )
{
  XTextureSection     TexSec( TexSection );

  if ( TexSection.m_pTexture )
  {
    GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
    tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

    if ( itCK != m_ColorKeyedTextures.end() )
    {
      TexSec.m_pTexture = itCK->second;
    }
    else
    {
      // neue Textur mit Colorkey erzeugen
      XTexture*   pNewTex = LoadTexture( TexSec.m_pTexture->m_LoadedFromFile.c_str(), GR::Graphic::IF_UNKNOWN, ColorKey );

      m_ColorKeyedTextures[strEntry] = pNewTex;

      TexSec.m_pTexture = pNewTex;
    }
  }

  SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  GR::tVector   vDelta( -iWidth * 0.5f * ZoomX,
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta2( iWidth * 0.5f * ZoomX,
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta3( -iWidth * 0.5f * ZoomX,
                        iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta4( iWidth * 0.5f * ZoomX,
                        iHeight * 0.5f * ZoomY,
                        0.0f );

  vDelta.RotateZ( -Angle );
  vDelta2.RotateZ( -Angle );
  vDelta3.RotateZ( -Angle );
  vDelta4.RotateZ( -Angle );

  GR::tVector   ptPos1 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta;
  GR::tVector   ptPos2 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta2;
  GR::tVector   ptPos3 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta3;
  GR::tVector   ptPos4 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta4;

  ptPos1.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos1.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos2.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos2.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos3.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos3.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos4.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos4.y *= m_CreationHeight / m_VirtualSize.y;


  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  RenderQuad2d( ptPos1, ptPos2, ptPos3, ptPos4,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                0xffffffff );
}



void DX82dRenderer::RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, float Angle, float ZoomX, float ZoomY, GR::u32 ColorKey, GR::u32 Color )
{
  XTextureSection     TexSec( TexSection );

  if ( TexSection.m_pTexture )
  {
    GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
    tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

    if ( itCK != m_ColorKeyedTextures.end() )
    {
      TexSec.m_pTexture = itCK->second;
    }
    else
    {
      // neue Textur mit Colorkey erzeugen
      XTexture*   pNewTex = LoadTexture( TexSec.m_pTexture->m_LoadedFromFile.c_str(), GR::Graphic::IF_UNKNOWN, ColorKey );

      m_ColorKeyedTextures[strEntry] = pNewTex;

      TexSec.m_pTexture = pNewTex;
    }
  }

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  GR::tVector   vDelta( -iWidth * 0.5f * ZoomX, 
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta2( iWidth * 0.5f * ZoomX, 
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta3( -iWidth * 0.5f * ZoomX, 
                        iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta4( iWidth * 0.5f * ZoomX, 
                        iHeight * 0.5f * ZoomY,
                        0.0f );

  vDelta.RotateZ( -Angle );
  vDelta2.RotateZ( -Angle );
  vDelta3.RotateZ( -Angle );
  vDelta4.RotateZ( -Angle );

  GR::tVector   ptPos1 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta;
  GR::tVector   ptPos2 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta2;
  GR::tVector   ptPos3 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta3;
  GR::tVector   ptPos4 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta4;

  ptPos1.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos1.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos2.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos2.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos3.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos3.y *= m_CreationHeight / m_VirtualSize.y;
  ptPos4.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos4.y *= m_CreationHeight / m_VirtualSize.y;

  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  RenderQuad2d( ptPos1, ptPos2, ptPos3, ptPos4,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}


void DX82dRenderer::RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, 
                                                    const XTextureSection& TexSection )
{
  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetTexture( 0, TexSection.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                0xffffffff );
}



void DX82dRenderer::RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                              const XTextureSection& TexSection, GR::u32 Color )
{
  XTextureSection     TexSec( TexSection );

  // TODO - Textur extra laden für anderen Colorkey?
  if ( TexSection.m_pTexture )
  {
    if ( TexSection.m_pTexture->m_ColorKey != ColorKey )
    {
      GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

      if ( itCK != m_ColorKeyedTextures.end() )
      {
        TexSec.m_pTexture = itCK->second;
      }
      else
      {
        // neue Textur mit Colorkey erzeugen
        GR::Graphic::ImageData    ImageData;

        GR::Graphic::eImageFormat  fmtToUse = TexSec.m_pTexture->m_ImageFormat;

        if ( !GR::Graphic::ImageData::FormatHasAlpha( TexSec.m_pTexture->m_ImageFormat ) )
        {
          switch ( TexSec.m_pTexture->m_ImageFormat )
          {
            case GR::Graphic::IF_X1R5G5B5:
            case GR::Graphic::IF_R5G6B5:
              fmtToUse = GR::Graphic::IF_A1R5G5B5;
              break;
            default:
              fmtToUse = GR::Graphic::IF_A8R8G8B8;
              break;
          }
        }
        
        ImageData.CreateData( TexSec.m_pTexture->m_SurfaceSize.x, 
                              TexSec.m_pTexture->m_SurfaceSize.y,
                              fmtToUse );

        if ( !( (CDX8Texture*)TexSec.m_pTexture )->m_ImageData.ConvertInto( &ImageData, true, ColorKey ) )
        {
          dh::Log( "DX82dRenderer::RenderTextureSectionColorKeyed Colorkeying failed" );
        }

        XTexture*   pNewTex = CreateTexture( ImageData );

        m_ColorKeyedTextures[strEntry] = pNewTex;

        TexSec.m_pTexture = pNewTex;
      }
    }
  }

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
  SetRenderState( D3DRS_ALPHAREF, 8 );
  SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSec.m_Width;
  int iHeight = TexSec.m_Height;

  GR::u32  AlternativeFlags = TexSec.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  RenderQuad2d( X, Y,
                iWidth, iHeight,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}



void DX82dRenderer::RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                          float Angle, float ZoomX, float ZoomY,
                                                                          const GR::u32 ColorKey,
                                                                          const XTextureSection& TexSection, GR::u32 Color )
{
  XTextureSection     TexSec( TexSection );

  if ( TexSection.m_pTexture )
  {
    GR::String   strEntry = Misc::Format( "%01x:8%%2%" ) << (GR::up)TexSection.m_pTexture << ColorKey;
    tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.find( strEntry ) );

    if ( itCK != m_ColorKeyedTextures.end() )
    {
      TexSec.m_pTexture = itCK->second;
    }
    else
    {
      // neue Textur mit Colorkey erzeugen
      XTexture*   pNewTex = LoadTexture( TexSec.m_pTexture->m_LoadedFromFile.c_str(), GR::Graphic::IF_UNKNOWN, ColorKey );

      m_ColorKeyedTextures[strEntry] = pNewTex;

      TexSec.m_pTexture = pNewTex;
    }
  }

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

  SetTexture( 0, TexSec.m_pTexture );

  int iWidth  = TexSection.m_Width;
  int iHeight = TexSection.m_Height;

  GR::u32  AlternativeFlags = TexSection.m_Flags;

  static GR::tFPoint     UV1;
  static GR::tFPoint     UV2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  TexSection.GetTrueUV( UV1, UV2, UV3, UV4, TexSection.m_Flags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( iWidth, iHeight );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= iWidth / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= iHeight / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= iHeight - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= iWidth - 1;
  }

  GR::tVector   vDelta( -iWidth * 0.5f * ZoomX, 
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta2( iWidth * 0.5f * ZoomX, 
                        -iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta3( -iWidth * 0.5f * ZoomX, 
                        iHeight * 0.5f * ZoomY,
                        0.0f );
  GR::tVector   vDelta4( iWidth * 0.5f * ZoomX, 
                        iHeight * 0.5f * ZoomY,
                        0.0f );

  vDelta.RotateZ( -Angle );
  vDelta2.RotateZ( -Angle );
  vDelta3.RotateZ( -Angle );
  vDelta4.RotateZ( -Angle );

  GR::tVector   ptPos1 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta;
  GR::tVector   ptPos2 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta2;
  GR::tVector   ptPos3 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta3;
  GR::tVector   ptPos4 = GR::tVector( (float)X, (float)Y, 0 ) + vDelta4;

  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  RenderQuad2d( ptPos1, ptPos2, ptPos3, ptPos4,
                UV1.x, UV1.y,
                UV2.x, UV2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color );
}



bool DX82dRenderer::SetRenderState( GR::u32 State, GR::u32 Value )
{
  tRenderStates::iterator    it( m_RenderStates.find( State ) );
  if ( it == m_RenderStates.end() )
  {
    return SUCCEEDED( m_pd3dDevice->SetRenderState( (D3DRENDERSTATETYPE)State, Value ) );
  }
  if ( it->second == Value )
  {
    return true;
  }
  return SUCCEEDED( m_pd3dDevice->SetRenderState( (D3DRENDERSTATETYPE)State, Value ) );
}



bool DX82dRenderer::SetTextureStageState( GR::u32 State, GR::u32 Value, GR::u32 Stage )
{
  if ( Stage >= 4 )
  {
    return false;
  }

  tTextureStageStates::iterator    it( m_TextureStageStates[Stage].find( State ) );
  if ( it == m_TextureStageStates[Stage].end() )
  {
    return SUCCEEDED( m_pd3dDevice->SetTextureStageState( Stage, (D3DTEXTURESTAGESTATETYPE)State, Value ) );
  }
  if ( it->second == Value )
  {
    return true;
  }
  m_TextureStageStates[Stage][State] = Value;
  return SUCCEEDED( m_pd3dDevice->SetTextureStageState( Stage, (D3DTEXTURESTAGESTATETYPE)State, Value ) );
}



void DX82dRenderer::UnloadAssets( Xtreme::Asset::eXAssetType Type )
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



void DX82dRenderer::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu entladen
      UnloadAssets( Xtreme::Asset::XA_IMAGE_SECTION );
      UnloadAssets( Xtreme::Asset::XA_IMAGE );
      UnloadAssets( Xtreme::Asset::XA_FONT );
      pLoader->NotifyService( "GUI", "AssetsUnloaded" );
    }
  }
}



void DX82dRenderer::RenderTextureSectionMasked( GR::i32 X, GR::i32 Y, 
                                                  const XTextureSection& TexSection,
                                                  GR::i32 MaskX, GR::i32 MaskY, 
                                                  const XTextureSection& TexSectionMask, 
                                                  const GR::u32 Color, 
                                                  const GR::u32 MaskColorKey )
{
  // überlappendes Rect finden
  GR::tRect     rcSection( X, Y, TexSection.m_Width, TexSection.m_Height );
  GR::tRect     rcMask( MaskX, MaskY, TexSectionMask.m_Width, TexSectionMask.m_Height );

  GR::tRect     rcOverlap( rcSection.Intersection( rcMask ) );

  if ( rcOverlap.Empty() )
  {
    return;
  }

  XTextureSection     TS1( TexSection );
  XTextureSection     TS2( TexSectionMask );

  TS1.m_XOffset += rcOverlap.Left - X;
  TS1.m_YOffset += rcOverlap.Top - Y;

  TS2.m_XOffset += rcOverlap.Left - MaskX;
  TS2.m_YOffset += rcOverlap.Top - MaskY;

  TS1.m_Width = rcOverlap.Width();
  TS1.m_Height = rcOverlap.Height();
  TS2.m_Width = rcOverlap.Width();
  TS2.m_Height = rcOverlap.Height();

  static GR::tFPoint     UV1a;
  static GR::tFPoint     UV2a;
  static GR::tFPoint     UV3a;
  static GR::tFPoint     UV4a;
  static GR::tFPoint     UV1b;
  static GR::tFPoint     UV2b;
  static GR::tFPoint     UV3b;
  static GR::tFPoint     UV4b;

  TS1.GetTrueUV( UV1a, UV2a, UV3a, UV4a, TS1.m_Flags );
  TS2.GetTrueUV( UV1b, UV2b, UV3b, UV4b, TS2.m_Flags );

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );

  SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  // Alpha-Map
  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1, 1 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_CURRENT, 1 );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE, 1 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE, 1 );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_CURRENT, 1 );

  // Stage 2
  SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE, 0 );
  SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE, 0 );
  SetTextureStageState( D3DTSS_COLORARG2, D3DTA_CURRENT, 0 );

  SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE, 0 );
  SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE, 0 );
  SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_CURRENT, 0 );

  SetTextureStageState( D3DTSS_COLOROP,  D3DTOP_DISABLE, 2 );
  SetTextureStageState( D3DTSS_ALPHAOP,  D3DTOP_DISABLE, 2 );


  SetTexture( 0, TS1.m_pTexture );
  SetTexture( 1, TS2.m_pTexture );

  struct CUSTOMVERTEX
  {
      GR::tVector   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
      float         fTU2,
                    fTV2;
  };

  CUSTOMVERTEX          vertData[4];

  GR::tVector   ptPos( (float)rcOverlap.Left, (float)rcOverlap.Top, 0.0f );
  GR::tVector   ptSize( (float)rcOverlap.Width(), (float)rcOverlap.Height(), 0.0f );

  ptPos.x *= m_CreationWidth / m_VirtualSize.x;
  ptPos.y *= m_CreationHeight / m_VirtualSize.y;
  ptSize.x *= m_CreationWidth / m_VirtualSize.x;
  ptSize.y *= m_CreationHeight / m_VirtualSize.y;

  m_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2 );

  vertData[0].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[0].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[0].position.z  = (float)ptPos.z;
  vertData[0].color       = Color;
  vertData[0].fRHW        = 1.0f;
  vertData[0].fTU         = UV1a.x;
  vertData[0].fTV         = UV1a.y;
  vertData[0].fTU2        = UV1b.x;
  vertData[0].fTV2        = UV1b.y;

  vertData[1].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[1].position.y  = ptPos.y + m_DirectTexelMappingOffset.y;
  vertData[1].position.z  = (float)ptPos.z;
  vertData[1].color       = Color;
  vertData[1].fRHW        = 1.0f;
  vertData[1].fTU         = UV2a.x;
  vertData[1].fTV         = UV2a.y;
  vertData[1].fTU2        = UV2b.x;
  vertData[1].fTV2        = UV2b.y;

  vertData[2].position.x  = ptPos.x + m_DirectTexelMappingOffset.x;
  vertData[2].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[2].position.z  = (float)ptPos.z;
  vertData[2].color       = Color;
  vertData[2].fRHW        = 1.0f;
  vertData[2].fTU         = UV3a.x;
  vertData[2].fTV         = UV3a.y;
  vertData[2].fTU2        = UV3b.x;
  vertData[2].fTV2        = UV3b.y;

  vertData[3].position.x  = ptPos.x + ptSize.x + m_DirectTexelMappingOffset.x;
  vertData[3].position.y  = ptPos.y + ptSize.y + m_DirectTexelMappingOffset.y;
  vertData[3].position.z  = (float)ptPos.z;
  vertData[3].color       = Color;
  vertData[3].fRHW        = 1.0f;
  vertData[3].fTU         = UV4a.x;
  vertData[3].fTV         = UV4a.y;
  vertData[3].fTU2        = UV4b.x;
  vertData[3].fTV2        = UV4b.y;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );
}



void DX82dRenderer::Log( const GR::String& strSystem, const char* szFormat, ... )
{
  if ( m_pDebugger )
  {
    static char    szMiscBuffer[5000];
    vsprintf( szMiscBuffer, szFormat, (char *)( &szFormat + 1 ) );

    m_pDebugger->LogDirect( strSystem.c_str(), szMiscBuffer );
  }
}



void DX82dRenderer::RenderText( X2dFont* Font, int iX, int iY, const GR::String& Text, GR::u32 Color )
{
  if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_FULL )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

    /*
    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    SetRenderState( D3DRS_LASTPIXEL, TRUE );

    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_DISABLE, 1 );
    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_DISABLE, 1 );

    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    SetTextureStageState( D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    */
  }
  else if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_BIT )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  }

  XBasic2dRenderer::RenderText( Font, iX, iY, Text, Color );
}



void DX82dRenderer::RenderTextCentered( X2dFont* Font, int iCenterX, int iY, const GR::String& Text, GR::u32 Color )
{
  if ( Font == NULL )
  {
    dh::Log( "DX82dRenderer::RenderTextCentered: Font = NULL!" );
    return;
  }
  if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_FULL )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_BIT )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  }

  XBasic2dRenderer::RenderTextCentered( Font, iCenterX, iY, Text, Color );
}



void DX82dRenderer::RenderTextRightAligned( X2dFont* Font, int iX, int iY, const GR::String&  Text, GR::u32 Color )
{
  if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_FULL )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_BIT )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  }

  XBasic2dRenderer::RenderTextRightAligned( Font, iX, iY, Text, Color );
}



void DX82dRenderer::RenderText( X2dFont* Font, int iX, int iY, const GR::String& Text,
                                      float fScaleX, float fScaleY, GR::u32 Color )
{
  if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_FULL )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetTextureStageState( D3DTSS_ALPHAOP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else if ( Font->m_CreationFlags & X2dFont::FLF_ALPHA_BIT )
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
    SetRenderState( D3DRS_ALPHAREF, 8 );
    SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
  }
  else
  {
    SetTextureStageState( D3DTSS_COLOROP, D3DTOP_MODULATE );
    SetTextureStageState( D3DTSS_COLORARG1, D3DTA_TEXTURE );
    SetTextureStageState( D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
  }

  XBasic2dRenderer::RenderText( Font, iX, iY, Text, fScaleX, fScaleY, Color );
}



void DX82dRenderer::DestroyTexture( XTexture* pTexture )
{
  tTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pThisTexture = *it;
    
    if ( pThisTexture == pTexture )
    {
      GR::String     Entry = Misc::Format( "%01x:8%" ) << (GR::up)pTexture;

      std::set<XTexture*>   ToDeleteTextures;

      tColorKeyedTextures::iterator    itCK( m_ColorKeyedTextures.begin() );
      while ( itCK != m_ColorKeyedTextures.end() )
      {
        if ( itCK->first.find( Entry ) == 0 )
        {
          ToDeleteTextures.insert( itCK->second );
          itCK = m_ColorKeyedTextures.erase( itCK );
        }
        else
        {
          ++itCK;
        }
      }
      std::set<XTexture*>::iterator   itTDT( ToDeleteTextures.begin() );
      while ( itTDT != ToDeleteTextures.end() )
      {
        DestroyTexture( *itTDT );

        ++itTDT;
      }

      pTexture->Release();
      delete pTexture;

      m_Textures.erase( it );

      return;
    }

    ++it;
  }
}



