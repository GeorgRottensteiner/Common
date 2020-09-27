/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <windows.h>
#include <windowsx.h>

#include <new>


#include <Debug\debugclient.h>

#include <DX8\DX8Viewer.h>
#include <DX8\TextureSection.h>

#include <math/vector3.h>

#include <IO\FileStream.h>

#include <Misc/Misc.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CD3DViewer*   CD3DViewer::m_pViewer = NULL;



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CD3DViewer::CD3DViewer()
  : m_bSaveSnapShot( FALSE ),
    m_hFont( NULL )
{

  m_pViewer           = this;

  m_dwAdapter         = 0L;
  m_pD3D              = NULL;
  m_pd3dDevice        = NULL;
  m_hWnd              = NULL;
  m_hWndFocus         = NULL;
  m_bActive           = FALSE;
  m_bReady            = FALSE;
  m_bWindowed         = TRUE;
  m_dwCreateFlags     = 0L;
  m_bUseDepthBuffer   = TRUE;

  m_bShowingBackBuffer = FALSE;

  m_dwCreationWidth   = 400;
  m_dwCreationHeight  = 300;
  m_dwCreationDepth   = 16;
  m_dwMinDepthBits    = 16;
  m_dwMinStencilBits  = 0;

  m_pCurrentModeInfo  = NULL;

  m_dwVertexShader    = 0;

  m_pFont             = NULL;

  m_bForceWindowSize        = false;
  m_bAutoMipMapGeneration   = true;
  m_bVSyncEnabled           = true;
  m_bMultiThreadSafe        = false;

  memset( &m_bLightEnabled, 0, sizeof( m_bLightEnabled ) );
  memset( &m_Lights, 0, sizeof( m_Lights ) );
  for ( int i = 0; i < 8; ++i )
  {
    m_Lights[i].Type = D3DLIGHT_DIRECTIONAL;
  }

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CD3DViewer::~CD3DViewer()
{

  Cleanup3DEnvironment();

  m_pViewer = NULL;

}



/*-SaveSnapShot---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::SaveSnapShot( const char *szFile )
{

  m_bSaveSnapShot = TRUE;
  m_strSnapShotFile = szFile;

}



/*-DoSaveSnapShot-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DoSaveSnapShot( const char* szFile )
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

  DWORD   dwPixel,
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
          for ( unsigned int i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &dwPixel, pData, 3 );

            File.WriteU8( unsigned char( ( dwPixel & 0xff0000 ) >> 16 ) );
            File.WriteU8( unsigned char( ( dwPixel & 0x00ff00 ) >> 8  ) );
            File.WriteU8( unsigned char( ( dwPixel & 0x0000ff )       ) );

            pData += 4;
          }
          pData += ( lockRect.Pitch - ddsd.Width ) / 4;
        }
        break;
      case D3DFMT_R8G8B8:
        {
          for ( DWORD i = 0; i < ddsd.Width; i++ )
          {
            memcpy( &dwPixel, pData, 3 );

            File.WriteU8( (BYTE)( ( dwPixel & 0xff0000 ) >> 16 ) );
            File.WriteU8( (BYTE)( ( dwPixel & 0x00ff00 ) >> 8  ) );
            File.WriteU8( (BYTE)( ( dwPixel & 0x0000ff )       ) );

            pData += 3;
          }
          pData += ( lockRect.Pitch - ddsd.Width ) / 3;
        }
        break;
    }
  }
  File.Close();

  pDestSurface->UnlockRect();
  pDestSurface->Release();

}



BOOL CD3DViewer::Create( HINSTANCE hInstance, HWND hwnd )
{

  HRESULT hr;

  // Create the Direct3D object
  m_pD3D = Direct3DCreate8( D3D_SDK_VERSION );
  if ( m_pD3D == NULL )
  {
    dh::Log( "Create: Direct3DCreate8 failed" );
    return FALSE;
  }
  // Build a list of Direct3D adapters, modes and devices. The
  // ConfirmDevice() callback is used to confirm that only devices that
  // meet the app's requirements are considered.
  if ( FAILED( hr = BuildDeviceList() ) )
  {
    SAFE_RELEASE( m_pD3D );
    dh::Log( "Create: failed to build device list" );
    return FALSE;
  }

  // Unless a substitute hWnd has been specified, create a window to
  // render into
  if ( hwnd != NULL )
  {
    m_hWnd = hwnd;
  }
  if ( m_hWnd == NULL)
  {
    SAFE_RELEASE( m_pD3D );
    dh::Log( "Create: no valid window handle" );
    return FALSE;
  }

  // The focus window can be a specified to be a different window than the
  // device window.  If not, use the device window as the focus window.
  if ( m_hWndFocus == NULL )
  {
    m_hWndFocus = m_hWnd;
  }

#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )
  // Häßlich, W64-Warning disablen, läßt sich hier sonst nicht unterdrücken
  m_pOldProc = (WNDPROC)SetWindowLongPtr( m_hWnd, GWL_WNDPROC, (LONG_PTR)My3dAppProc );
#pragma warning( pop )

  // Save window properties
  m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
  m_dwWindowStyleEx = GetWindowLong( m_hWnd, GWL_EXSTYLE );
  m_WindowPlacement.length = sizeof( WINDOWPLACEMENT );
  GetWindowPlacement( m_hWnd, &m_WindowPlacement );

  // Initialize the 3D environment for the app
  if ( FAILED( hr = Initialize3DEnvironment() ) )
  {
    SAFE_RELEASE( m_pD3D );
    //LogText( "Create: Initialize3DEnvironment failed" );
    //return FALSE;
  }

  // The app is ready to go
  m_bReady = TRUE;

  return TRUE;

}



/*-SetTransform---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetTransform( D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix )
{

  if ( m_pd3dDevice == NULL )
  {
    return E_FAIL;
  }

  switch ( State )
  {
    case D3DTS_WORLD:
      m_matWorld = *pMatrix;
      break;
    case D3DTS_VIEW:
      m_matView = *pMatrix;
      break;
    case D3DTS_PROJECTION:
      m_matProjection = *pMatrix;
      break;
  }

  return m_pd3dDevice->SetTransform( State, pMatrix );

}



/*-SetTransform---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetTransform( D3DTRANSFORMSTATETYPE State, CONST math::matrix4& matTransform )
{

  if ( m_pd3dDevice == NULL )
  {
    return E_FAIL;
  }

  D3DXMATRIX    matTemp;

  memcpy( &matTemp, &matTransform, sizeof( matTransform ) );

  switch ( State )
  {
    case D3DTS_WORLD:
      m_matWorld = matTemp;
      break;
    case D3DTS_VIEW:
      m_matView = matTemp;
      break;
    case D3DTS_PROJECTION:
      m_matProjection = matTemp;
      break;
  }

  return m_pd3dDevice->SetTransform( State, &matTemp );

}



/*-SetViewport----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetViewport( CONST D3DVIEWPORT8* pViewport )
{

  if ( m_pd3dDevice == NULL )
  {
    return E_FAIL;
  }
  if ( pViewport == NULL )
  {
    return E_FAIL;
  }
  m_viewPort = *pViewport;

  HRESULT   hRes = m_pd3dDevice->SetViewport( pViewport );

  if ( FAILED( hRes ) )
  {
    dh::Log( "SetViewport failed (%x)\n", hRes );
  }

  // PFUI PFUI ATI PFUI PFUI
  // ATI hat gepfuscht, und manchmal funktioniert die letzte gesetzte Textur vor SetViewport
  // nicht mehr, und es muß vorher eine andere gesetzt werden???
  SetTexture( 0, NULL );

  return hRes;

}



/*-SortModesCallback----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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



/*-BuildDeviceList------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::BuildDeviceList()
{

  const DWORD         dwNumDeviceTypes = 2;

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
    Adapter.dwCurrentDevice = 0;

    // Enumerate all display modes on this adapter
    D3DDISPLAYMODE          modes[100];
    D3DFORMAT               formats[20];
    DWORD                   dwNumFormats      = 0,
                            dwNumModes        = 0,
                            dwNumAdapterModes = m_pD3D->GetAdapterModeCount( iAdapter );


    // Add the adapter's current desktop format to the list of formats
    //formats[dwNumFormats++] = Adapter.d3ddmDesktop.Format;

    for ( UINT iMode = 0; iMode < dwNumAdapterModes; iMode++ )
    {
      // Get the display mode attributes
      D3DDISPLAYMODE      DisplayMode;

      m_pD3D->EnumAdapterModes( iAdapter, iMode, &DisplayMode );

      // Check if the mode already exists (to filter out refresh rates)
      DWORD m = 0L;
      for ( m = 0L; m < dwNumModes; m++ )
      {
        if ( ( modes[m].Width  == DisplayMode.Width )
        &&   ( modes[m].Height == DisplayMode.Height )
        &&   ( modes[m].Format == DisplayMode.Format ) )
        {
          break;
        }
      }

      // If we found a new mode, add it to the list of modes
      if ( m == dwNumModes )
      {
        modes[dwNumModes].Width       = DisplayMode.Width;
        modes[dwNumModes].Height      = DisplayMode.Height;
        modes[dwNumModes].Format      = DisplayMode.Format;
        modes[dwNumModes].RefreshRate = 0;
        dwNumModes++;

        // Check if the mode's format already exists
        DWORD f = 0;
        for ( f = 0; f < dwNumFormats; f++ )
        {
          if ( DisplayMode.Format == formats[f] )
          {
            break;
          }
        }

        // If the format is new, add it to the list
        if ( f == dwNumFormats )
        {
          formats[dwNumFormats++] = DisplayMode.Format;
        }
      }
    }

    // Sort the list of display modes (by format, then width, then height)
    qsort( modes, dwNumModes, sizeof( D3DDISPLAYMODE ), SortModesCallback );

    // Add devices to adapter
    for ( UINT iDevice = 0; iDevice < dwNumDeviceTypes; iDevice++ )
    {
      // Fill in device info
      tD3DDeviceInfo Device;
      //Device                   = &pAdapter->devices[pAdapter->dwNumDevices];
      Device.deviceType       = DeviceTypes[iDevice];

      m_pD3D->GetDeviceCaps( iAdapter, DeviceTypes[iDevice], &Device.d3dCaps );
      Device.strName          = strDeviceDescs[iDevice];
      Device.m_vectMode.clear();
      Device.dwCurrentMode    = 0;
      Device.bCanDoWindowed   = FALSE;
      Device.bWindowed        = FALSE;
      Device.MultiSampleType  = D3DMULTISAMPLE_NONE;

      // Examine each format supported by the adapter to see if it will
      // work with this device and meets the needs of the application.
      BOOL        bFormatConfirmed[20];

      DWORD       dwBehavior[20];

      D3DFORMAT   fmtDepthStencil[20];


      for ( DWORD f = 0; f < dwNumFormats; f++ )
      {
        bFormatConfirmed[f] = FALSE;
        fmtDepthStencil[f]  = D3DFMT_UNKNOWN;

        // Skip formats that cannot be used as render targets on this device
        if ( FAILED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, formats[f], formats[f], FALSE ) ) )
        {
          continue;
        }

        if ( Device.deviceType == D3DDEVTYPE_HAL )
        {
          // This system has a HAL device
          bHALExists = TRUE;

          if ( Device.d3dCaps.Caps2 & D3DCAPS2_CANRENDERWINDOWED )
          {
            // HAL can run in a window for some mode
            bHALIsWindowedCompatible = TRUE;
            if ( SUCCEEDED( m_pD3D->CheckDeviceType( iAdapter, Device.deviceType, Adapter.d3ddmDesktop.Format, formats[f], FALSE ) ) )
            {
              // HAL can run in a window for the current desktop mode
              bHALIsDesktopCompatible = TRUE;
            }
          }
        }

        // Confirm the device/format for HW vertex processing
        if ( Device.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
        {
          if ( Device.d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE )
          {
            dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;

            if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, dwBehavior[f], formats[f] ) ) )
            {
              bFormatConfirmed[f] = TRUE;
            }
          }

          if ( !bFormatConfirmed[f] )
          {
            dwBehavior[f] = D3DCREATE_HARDWARE_VERTEXPROCESSING;

            if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, dwBehavior[f], formats[f] ) ) )
            {
              bFormatConfirmed[f] = TRUE;
            }
          }

          if ( !bFormatConfirmed[f] )
          {
            dwBehavior[f] = D3DCREATE_MIXED_VERTEXPROCESSING;

            if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, dwBehavior[f], formats[f] ) ) )
            {
              bFormatConfirmed[f] = TRUE;
            }
          }
        }

        // Confirm the device/format for SW vertex processing
        if ( !bFormatConfirmed[f] )
        {
          dwBehavior[f] = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

          if ( SUCCEEDED( ConfirmDevice( &Device.d3dCaps, dwBehavior[f], formats[f] ) ) )
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
      for ( DWORD m = 0L; m < dwNumModes; m++ )
      {
        for ( DWORD f = 0; f < dwNumFormats; f++ )
        {
          if ( modes[m].Format == formats[f] )
          {
            if ( bFormatConfirmed[f] )
            {
              // Add this mode to the device's list of valid modes
              tD3DModeInfo   modeInfo;


              modeInfo.m_dwWidth    = modes[m].Width;
              modeInfo.m_dwHeight   = modes[m].Height;
              modeInfo.m_d3dfPixel   = modes[m].Format;
              modeInfo.m_dwBehaviour   = dwBehavior[f];
              modeInfo.m_d3dfDepthStencil   = fmtDepthStencil[f];
              Device.m_vectMode.push_back( modeInfo );
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
        m_vectAdapters[a].dwCurrentDevice = d;
        m_dwAdapter = a;
        return S_OK;
      }
    }
  }

  // kein Device gefunden, das im Fenster rendern kann
  return 2;

}


/*-SelectDefaultMode----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice )
{

  if ( !pDevice )
  {
    dh::Log( "no device\n" );
    return;
  }

  pDevice->dwCurrentMode = 0;
  for ( unsigned int m = 0; m < pDevice->m_vectMode.size(); m++ )
  {
    tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[m];

    if ( ( ModeInfo.m_dwWidth == m_dwCreationWidth )
    &&   ( ModeInfo.m_dwHeight == m_dwCreationHeight ) )
    {

      if ( ( m_dwCreationDepth == 24 )
      ||   ( m_dwCreationDepth == 32 ) )
      {
        if ( ( ModeInfo.m_d3dfPixel == D3DFMT_R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A8R8G8B8 )
        ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X8R8G8B8 ) )
        {
          pDevice->dwCurrentMode = m;
          break;
        }
      }
      if ( m_dwCreationDepth == 16 )
      {
      if ( ( ModeInfo.m_d3dfPixel == D3DFMT_R5G6B5 )
      ||   ( ModeInfo.m_d3dfPixel == D3DFMT_X1R5G5B5 )
      ||   ( ModeInfo.m_d3dfPixel == D3DFMT_A1R5G5B5 ) )
      {
          pDevice->dwCurrentMode = m;
        break;
      }
    }
  }
  }

  // kann im aktuellen Desktop-Modus im Fenster gerendert werden?
  if ( m_bWindowed )
  {
    tD3DModeInfo&    ModeInfo = pDevice->m_vectMode[pDevice->dwCurrentMode];

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



/*-FindDepthStencilFormat-----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CD3DViewer::FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat )
{

  if ( ( m_dwMinDepthBits <= 16 )
  &&   ( m_dwMinStencilBits == 0 ) )
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

  if ( ( m_dwMinDepthBits <= 15 )
  &&   ( m_dwMinStencilBits <= 1 ) )
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

  if ( ( m_dwMinDepthBits <= 24 )
  &&   ( m_dwMinStencilBits == 0 ) )
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

  if ( ( m_dwMinDepthBits <= 24 )
  &&   ( m_dwMinStencilBits <= 8 ) )
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

  if ( ( m_dwMinDepthBits <= 24 )
  &&   ( m_dwMinStencilBits <= 4 ) )
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

  if ( ( m_dwMinDepthBits <= 32 )
  &&   ( m_dwMinStencilBits == 0 ) )
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



/*-ResizeSurface--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CD3DViewer::ResizeSurface()
{

  static BOOL   bInside = FALSE;

  if ( bInside )
  {
    return TRUE;
  }
  bInside = TRUE;

  if ( !IsWindow( m_hWnd ) )
  {
    return FALSE;
  }

  if ( ( m_bActive )
  &&   ( m_bWindowed ) )
  {
    RECT rcWindowClient;

    GetClientRect( m_hWnd, &rcWindowClient );
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



/*-MsgProc--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

LRESULT CALLBACK My3dAppProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch( uMsg )
  {
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT:
      // Handle paint messages when the app is not ready
      if ( ( CD3DViewer::m_pViewer->m_pd3dDevice )
      &&   ( !CD3DViewer::m_pViewer->m_bReady ) )
      {
        if ( CD3DViewer::m_pViewer->m_bWindowed )
        {
          CD3DViewer::m_pViewer->m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
          CD3DViewer::m_pViewer->m_bShowingBackBuffer = !CD3DViewer::m_pViewer->m_bShowingBackBuffer;
          return 0;
        }
      }
      break;
    case WM_SIZE:
      // Check to see if we are losing our window...
      if ( ( SIZE_MAXHIDE == wParam )
      ||   ( SIZE_MINIMIZED == wParam ) )
      {
        CD3DViewer::m_pViewer->m_bActive = FALSE;
      }
      else
      {
        CD3DViewer::m_pViewer->m_bActive = TRUE;
      }
      if ( ( wParam == SIZE_MAXIMIZED )
      ||   ( wParam == SIZE_RESTORED ) )
      {
        CD3DViewer::m_pViewer->ResizeSurface();
        break;
      }
      break;
    case WM_EXITSIZEMOVE:
      if ( !CD3DViewer::m_pViewer->ResizeSurface() )
      {
        break;
      }
      break;
    case WM_DESTROY:
      CD3DViewer::m_pViewer->Cleanup3DEnvironment();
      break;
  }

  return CallWindowProc( CD3DViewer::m_pViewer->m_pOldProc, hWnd, uMsg, wParam, lParam );

}



/*-RestoreAppDeviceObjects----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::RestoreAppDeviceObjects()
{

  // Renderstates wieder setzen
  {
    tMapRenderStates::iterator  it( m_mapRenderStates.begin() );
    while ( it != m_mapRenderStates.end() )
    {
      m_pd3dDevice->SetRenderState( it->first, it->second );
      it++;
    }
  }

  // Texture-Stage-States wieder setzen
  {
    for ( int i = 0; i < 8; i++ )
    {
      tMapTextureStageStates::iterator  it( m_mapTextureStageStates[i].begin() );
      while ( it != m_mapTextureStageStates[i].end() )
      {
        m_pd3dDevice->SetTextureStageState( i, it->first, it->second );
        it++;
      }
    }
  }

  if ( m_dwVertexShader != 0 )
  {
    m_pd3dDevice->SetVertexShader( m_dwVertexShader );
  }

  // produziert ein Leck?
  // Font anlegen
  if ( m_hFont )
  {
    DeleteObject( m_hFont );
    m_hFont = NULL;
  }
  m_hFont = CreateFont( 16, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, "Arial" );
  if ( m_pFont != NULL )
  {
    dh::Log( "der font war schon da!" );
  }
  if ( FAILED( D3DXCreateFont( m_pd3dDevice, m_hFont, &m_pFont ) ) )
  {
    m_pFont = NULL;
  }

  std::set<CVertexBuffer*>::iterator   it( m_setVBs.begin() );

  while ( it != m_setVBs.end() )
  {
    CVertexBuffer *pVB = *it;

    pVB->Restore( m_pd3dDevice );
    it++;
  }

  // Lights wieder setzen
  for ( size_t i = 0; i < 8; ++i )
  {
    if ( m_bLightEnabled[i] )
    {
      SetLight( i, m_Lights[i] );
    }
    EnableLight( i, m_bLightEnabled[i] );
  }

}



/*-InvalidateAppDeviceObjects-------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::InvalidateAppDeviceObjects()
{

  std::set<CVertexBuffer*>::iterator   it( m_setVBs.begin() );

  while ( it != m_setVBs.end() )
  {
    CVertexBuffer *pVB = *it;

    pVB->Invalidate();
    it++;
  }

  if ( m_pFont != NULL )
  {
    if ( m_pFont->Release() != 0 )
    {
      dh::Log( "font still referenced!" );
    }
    m_pFont = NULL;
  }
  if ( m_hFont )
  {
    DeleteObject( m_hFont );
    m_hFont = NULL;
  }

}



/*-Initialize3DEnvironment----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::Initialize3DEnvironment()
{

  if ( m_pD3D == NULL )
  {
    return E_FAIL;
  }

  HRESULT hr;


  tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_dwAdapter];
  tD3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->vectDevices[pAdapterInfo->dwCurrentDevice];


  m_dwCurrentAdapter  = m_dwAdapter;
  m_pCurrentModeInfo  = &pDeviceInfo->m_vectMode[pDeviceInfo->dwCurrentMode];

  m_dwCurrentDevice   = pAdapterInfo->dwCurrentDevice;
  m_dwCurrentMode     = pDeviceInfo->dwCurrentMode;

  pDeviceInfo->bWindowed  = m_bWindowed;

  // Prepare window for possible windowed/fullscreen change
  AdjustWindowForChange();

  // Set up the presentation parameters
  ZeroMemory( &m_d3dpp, sizeof( m_d3dpp ) );
  m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
  m_d3dpp.BackBufferCount        = 1;
  m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
  m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
  m_d3dpp.EnableAutoDepthStencil = m_bUseDepthBuffer;
  m_d3dpp.AutoDepthStencilFormat = m_pCurrentModeInfo->m_d3dfDepthStencil;
  m_d3dpp.hDeviceWindow          = m_hWnd;
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

      GetClientRect( m_hWnd, &rcWindowClient );
      m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
      m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;
    }

    // im Fenster muß immer das Format vom Desktop genommen werden!!
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
  }
  else
  {
    m_d3dpp.BackBufferWidth  = m_pCurrentModeInfo->m_dwWidth;
    m_d3dpp.BackBufferHeight = m_pCurrentModeInfo->m_dwHeight;
    m_d3dpp.BackBufferFormat = m_pCurrentModeInfo->m_d3dfPixel;
    if ( !m_bVSyncEnabled )
    {
      m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
  }

  if ( ( m_d3dpp.BackBufferHeight == 0 )
  ||   ( m_d3dpp.BackBufferWidth == 0 ) )
  {
    return E_FAIL;
  }

  // Create the device
  DWORD   dwBehaviour = m_pCurrentModeInfo->m_dwBehaviour;
  //dwBehaviour = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  if ( m_bMultiThreadSafe )
  {
    dwBehaviour |= D3DCREATE_MULTITHREADED;
  }
  hr = m_pD3D->CreateDevice( m_dwAdapter, pDeviceInfo->deviceType,
                             m_hWndFocus, dwBehaviour, &m_d3dpp,
                             &m_pd3dDevice );
  if ( SUCCEEDED( hr ) )
  {
    m_dwRenderWidth = m_d3dpp.BackBufferWidth;
    m_dwRenderHeight = m_d3dpp.BackBufferHeight;

    // den Viewport aufbewahren (für Pure-Devices)
    // das sind die Default-Werte (laut Dokumentation)
    m_viewPort.X        = 0;
    m_viewPort.Y        = 0;
    m_viewPort.Width    = m_dwRenderWidth;
    m_viewPort.Height   = m_dwRenderHeight;
    m_viewPort.MinZ     = 0.0f;
    m_viewPort.MaxZ     = 1.0f;

    // When moving from fullscreen to windowed mode, it is important to
    // adjust the window size after recreating the device rather than
    // beforehand to ensure that you get the window size you want.  For
    // example, when switching from 640x480 fullscreen to windowed with
    // a 1000x600 window on a 1024x768 desktop, it is impossible to set
    // the window size to 1000x600 until after the display mode has
    // changed to 1024x768, because windows cannot be larger than the
    // desktop.


    if ( m_bWindowed )
    {
      SetWindowPlacement( m_hWnd, &m_WindowPlacement );
    }

    // Store device Caps
    m_pd3dDevice->GetDeviceCaps( &m_d3dCaps );
    m_dwCreateFlags = m_pCurrentModeInfo->m_dwBehaviour;

    // Store render target surface desc
    LPDIRECT3DSURFACE8 pBackBuffer;
    m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
    pBackBuffer->Release();

    // Initialize the app's device-dependent objects
    hr = InitDeviceObjects();
    if ( SUCCEEDED( hr ) )
    {
      // Restore the textures
      if ( !RecreateTextures() )
      {
        dh::Log( "Initialize3DEnvironment: RecreateTextures failed" );
        return E_FAIL;
      }
      if ( !RecreateFonts() )
      {
        dh::Log( "Initialize3DEnvironment: RecreateFonts failed" );
        return E_FAIL;
      }
      hr = RestoreDeviceObjects();
      if ( SUCCEEDED( hr ) )
      {
        RestoreAppDeviceObjects();
        m_bActive = TRUE;
        m_bShowingBackBuffer = FALSE;
        return S_OK;
      }
    }
    else
    {
      dh::Log( "Initialize3DEnvironment: InitDeviceObjects failed" );
    }

    // Cleanup before we try again
    InvalidateDeviceObjects();
    InvalidateAppDeviceObjects();
    //DeleteAppDeviceObjects();
    DeleteDeviceObjects();
    SAFE_RELEASE( m_pd3dDevice );
  }
  else
  {
    dh::Log( "Initialize3DEnvironment: CreateDevice failed" );
  }

  // If that failed, fall back to the reference rasterizer
  if ( pDeviceInfo->deviceType == D3DDEVTYPE_HAL )
  {
    // Select the default adapter
    m_dwAdapter = 0L;
    pAdapterInfo = &m_vectAdapters[m_dwAdapter];

    // Look for a software device
    for ( UINT i = 0L; i < pAdapterInfo->vectDevices.size(); i++ )
    {
      if ( pAdapterInfo->vectDevices[i].deviceType == D3DDEVTYPE_REF )
      {
        pAdapterInfo->dwCurrentDevice = i;
        pDeviceInfo = &pAdapterInfo->vectDevices[i];
        m_bWindowed = pDeviceInfo->bWindowed;
        break;
      }
    }

    // Try again, this time with the reference rasterizer
    if ( pAdapterInfo->vectDevices[pAdapterInfo->dwCurrentDevice].deviceType == D3DDEVTYPE_REF )
    {
      hr = Initialize3DEnvironment();
      if ( FAILED( hr ) )
      {
        dh::Log( "Initialize3DEnvironment: 2nd time failed" );
      }
    }
  }

  m_bShowingBackBuffer = FALSE;

  return hr;

}



/*-Resize3DEnvironment--------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::Resize3DEnvironment()
{

  if ( m_pd3dDevice == NULL )
  {
    return E_FAIL;
  }

  HRESULT hr;

  // Release all vidmem objects
  if ( FAILED( hr = InvalidateDeviceObjects() ) )
  {
    dh::Log( "Resize3DEnvironment: InvalidateDeviceObjects failed" );
    return hr;
  }

  InvalidateAppDeviceObjects();
  ReleaseTextures();
  ReleaseFonts();


  // Reset the device
  if ( ( !m_d3dpp.BackBufferWidth )
  ||   ( !m_d3dpp.BackBufferHeight ) )
  {
    return E_FAIL;
  }
  if ( FAILED( hr = m_pd3dDevice->Reset( &m_d3dpp ) ) )
  {
    dh::Log( "Resize3DEnvironment: m_pd3dDevice->Reset failed" );
    return hr;
  }

  // Store render target surface desc
  LPDIRECT3DSURFACE8 pBackBuffer;
  m_pd3dDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
  pBackBuffer->GetDesc( &m_d3dsdBackBuffer );
  pBackBuffer->Release();

  m_dwRenderWidth = m_d3dsdBackBuffer.Width;
  m_dwRenderHeight = m_d3dsdBackBuffer.Height;

  // den Viewport aufbewahren (für Pure-Devices)
  // das sind die Default-Werte (laut Dokumentation)
  m_viewPort.X        = 0;
  m_viewPort.Y        = 0;
  m_viewPort.Width    = m_dwRenderWidth;
  m_viewPort.Height   = m_dwRenderHeight;
  m_viewPort.MinZ     = 0.0f;
  m_viewPort.MaxZ     = 1.0f;

  // Restore the textures
  if ( !RecreateTextures() )
  {
    dh::Log( "Resize3DEnvironment: RecreateTextures failed" );
    return E_FAIL;
  }
  if ( !RecreateFonts() )
  {
    dh::Log( "Resize3DEnvironment: RecreateFonts failed" );
    return E_FAIL;
  }

  // Initialize the app's device-dependent objects
  hr = RestoreDeviceObjects();
  if ( FAILED( hr ) )
  {
    dh::Log( "Resize3DEnvironment: RestoreDeviceObjects failed" );
    return hr;
  }
  RestoreAppDeviceObjects();
  m_bShowingBackBuffer = FALSE;

  return S_OK;

}



/*-ToggleFullScreen-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::ToggleFullscreen()
{

  // Get access to current adapter, device, and mode
  tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_dwAdapter];
  tD3DDeviceInfo*  pDeviceInfo  = &pAdapterInfo->vectDevices[pAdapterInfo->dwCurrentDevice];
  tD3DModeInfo*    pModeInfo    = &pDeviceInfo->m_vectMode[pDeviceInfo->dwCurrentMode];

  // Need device change if going windowed and the current device
  // can only be fullscreen
  m_bReady = FALSE;

  // Toggle the windowed state
  m_bWindowed = !m_bWindowed;
  pDeviceInfo->bWindowed = m_bWindowed;

  // Prepare window for windowed/fullscreen change
  AdjustWindowForChange();

  // Set up the presentation parameters
  m_d3dpp.Windowed               = pDeviceInfo->bWindowed;
  m_d3dpp.MultiSampleType        = pDeviceInfo->MultiSampleType;
  m_d3dpp.AutoDepthStencilFormat = pModeInfo->m_d3dfDepthStencil;
  m_d3dpp.hDeviceWindow          = m_hWnd;
  if ( m_bWindowed )
  {
    RECT    rcWindowClient;

    GetClientRect( m_hWnd, &rcWindowClient );

    m_d3dpp.BackBufferWidth  = rcWindowClient.right - rcWindowClient.left;
    m_d3dpp.BackBufferHeight = rcWindowClient.bottom - rcWindowClient.top;
    m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
  }
  else
  {
    m_d3dpp.BackBufferWidth  = pModeInfo->m_dwWidth;
    m_d3dpp.BackBufferHeight = pModeInfo->m_dwHeight;
    m_d3dpp.BackBufferFormat = pModeInfo->m_d3dfPixel;
    if ( !m_bVSyncEnabled )
    {
      m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }
  }

  // Resize the 3D device
  if ( FAILED( Resize3DEnvironment() ) )
  {
    if ( m_bWindowed )
    {
      return ForceWindowed();
    }
    else
    {
      return E_FAIL;
    }
  }

  // When moving from fullscreen to windowed mode, it is important to
  // adjust the window size after resetting the device rather than
  // beforehand to ensure that you get the window size you want.  For
  // example, when switching from 640x480 fullscreen to windowed with
  // a 1000x600 window on a 1024x768 desktop, it is impossible to set
  // the window size to 1000x600 until after the display mode has
  // changed to 1024x768, because windows cannot be larger than the
  // desktop.
  if ( m_bWindowed )
  {
    SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );// | WS_POPUP );
    SetWindowLong( m_hWnd, GWL_EXSTYLE, m_dwWindowStyleEx );
    SetWindowPlacement( m_hWnd, &m_WindowPlacement );
  }

  m_bReady = TRUE;

  return S_OK;

}



/*-ToggleVSync----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::ToggleVSync()
{

  m_bVSyncEnabled = !m_bVSyncEnabled;
  if ( m_bWindowed )
  {
    return;
  }

  if ( !m_bVSyncEnabled )
  {
    m_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
  }
  else
  {
    m_d3dpp.FullScreen_PresentationInterval = 0;
  }

  // und neu setzen
  if ( FAILED( Resize3DEnvironment() ) )
  {
    if ( m_bWindowed )
    {
      ForceWindowed();
    }
  }

}



/*-ForceWindowed--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::ForceWindowed()
{

  HRESULT         hr;

  tD3DAdapterInfo*  pAdapterInfoCur = &m_vectAdapters[m_dwAdapter];

  tD3DDeviceInfo*   pDeviceInfoCur  = &pAdapterInfoCur->vectDevices[pAdapterInfoCur->dwCurrentDevice];

  BOOL            bFoundDevice = FALSE;


  if ( pDeviceInfoCur->bCanDoWindowed )
  {
    bFoundDevice = TRUE;
  }
  else
  {
    // Look for a windowable device on any adapter
    tD3DAdapterInfo*  pAdapterInfo;

    DWORD             dwAdapter;

    tD3DDeviceInfo*   pDeviceInfo;

    DWORD             dwDevice;


    for ( dwAdapter = 0; dwAdapter < m_vectAdapters.size(); dwAdapter++ )
    {
      pAdapterInfo = &m_vectAdapters[dwAdapter];
      for ( dwDevice = 0; dwDevice < pAdapterInfo->vectDevices.size(); dwDevice++ )
      {
        pDeviceInfo = &pAdapterInfo->vectDevices[dwDevice];
        if ( pDeviceInfo->bCanDoWindowed )
        {
          m_dwAdapter = dwAdapter;
          pDeviceInfoCur = pDeviceInfo;
          pAdapterInfo->dwCurrentDevice = dwDevice;
          bFoundDevice = TRUE;
          break;
        }
      }
      if ( bFoundDevice )
      {
        break;
      }
    }
  }

  if ( !bFoundDevice )
  {
    return E_FAIL;
  }

  pDeviceInfoCur->bWindowed = TRUE;
  m_bWindowed = TRUE;

  // Now destroy the current 3D device objects, then reinitialize

  m_bReady = FALSE;

  // Release all scene objects that will be re-created for the new device
  InvalidateAppDeviceObjects();
  ReleaseTextures();
  ReleaseFonts();

  InvalidateDeviceObjects();
  DeleteDeviceObjects();

  // Release display objects, so a new device can be created
  if ( m_pd3dDevice->Release() > 0L )
  {
    dh::Log( "ForceWindowed: No Zero RefCount" );
    return 3;//D3DAPPERR_NONZEROREFCOUNT;
  }

  // Create the new device
  if ( FAILED( hr = Initialize3DEnvironment() ) )
  {
    dh::Log( "ForceWindowed: Failed to Initialize3DEnvironment" );
    return hr;
  }
  m_bReady = TRUE;

  return S_OK;
}




/*-AdjustWindowForChange------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::AdjustWindowForChange()
{


  if ( m_bWindowed )
  {
    // Set windowed-mode style
    SetWindowLong( m_hWnd, GWL_STYLE, m_dwWindowStyle );
  }
  else
  {
    // Set fullscreen-mode style
    GetWindowPlacement( m_hWnd, &m_WindowPlacement );
    SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE );
  }

  return S_OK;

}



/*-RenderBegin----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CD3DViewer::RenderBegin()
{

  HRESULT hr = S_OK;

  if ( !m_pd3dDevice )
  {
    return FALSE;
  }
  if ( !m_bReady )
  {
    return FALSE;
  }

  // Test the cooperative level to see if it's okay to render
  if ( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
  {

    // If the device was lost, do not render until we get it back
    if ( D3DERR_DEVICELOST == hr )
    {
      return TRUE;
    }

    // Check if the device needs to be resized.
    if ( D3DERR_DEVICENOTRESET == hr )
    {
      // If we are windowed, read the desktop mode and use the same format for
      // the back buffer
      if ( m_bWindowed )
      {
        tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_dwAdapter];
        m_pD3D->GetAdapterDisplayMode( m_dwAdapter, &pAdapterInfo->d3ddmDesktop );
        m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
      }

      if ( FAILED( hr = Resize3DEnvironment() ) )
      {
        return FALSE;
      }
    }
  }

  return SUCCEEDED( m_pd3dDevice->BeginScene() );

}



/*-RenderDone-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::RenderDone( RECT* rectSrc, RECT* rectDest )
{

  m_pd3dDevice->EndScene();

  if ( m_bSaveSnapShot )
  {
    m_bSaveSnapShot = FALSE;
    DoSaveSnapShot( m_strSnapShotFile.c_str() );
  }

  // Show the frame on the primary surface.
  m_pd3dDevice->Present( rectSrc, rectDest, NULL, NULL );
  m_bShowingBackBuffer = !m_bShowingBackBuffer;

}



/*-ShowFrontBuffer------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::ShowFrontBuffer()
{

  if ( m_pd3dDevice == NULL )
  {
    return;
  }
  if ( m_bShowingBackBuffer )
  {
    m_pd3dDevice->Present( NULL, NULL, NULL, NULL );
    m_bShowingBackBuffer = FALSE;
  }

}



/*-Cleanup3DEnvironment-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::Cleanup3DEnvironment()
{

  m_bActive = FALSE;
  m_bReady  = FALSE;

  if ( m_pd3dDevice )
  {
    InvalidateAppDeviceObjects();
    DeleteAppDeviceObjects();
    ReleaseTextures();
    DestroyTextures();
    ReleaseFonts();
    DestroyAllFonts();
    InvalidateDeviceObjects();
    DeleteDeviceObjects();

    m_pd3dDevice->Release();
    m_pd3dDevice = NULL;
    m_pD3D->Release();

    m_pd3dDevice = NULL;
    m_pD3D       = NULL;
  }

  /*
  if ( ( m_pOldProc )
  &&   ( IsWindow( m_hWnd ) ) )
  {
    SetWindowLongPtr( m_hWnd, GWL_WNDPROC, (LONG_PTR)m_pOldProc );
  }
  */

}



/*-DrawLine-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawLine( const math::vector3& posAnfang, const math::vector3& posEnde, DWORD dwColor )
{

  struct CUSTOMVERTEX
  {
    math::vector3   position; // The position
    D3DCOLOR        color;    // The color
  };

  CUSTOMVERTEX          vertData[2];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

  SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

  math::vector3 vectDelta( -0.5f, -0.5f, 0.0f );

  vertData[0].position = posAnfang - vectDelta;
  vertData[0].color = dwColor;

  vertData[1].position = posEnde - vectDelta;
  vertData[1].color = dwColor;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawLine( D3DXVECTOR3 posAnfang, D3DXVECTOR3 posEnde, DWORD dwColor )
{

  struct CUSTOMVERTEX
  {
    D3DXVECTOR3   position; // The position
    float         fRHW;
    D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[2];


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

  D3DXVECTOR3   vectDelta( -0.5f, -0.5f, 0.0f );

  vertData[0].position = posAnfang - vectDelta;
  vertData[0].color = dwColor;

  vertData[1].position = posEnde - vectDelta;
  vertData[1].color = dwColor;

  vertData[0].fRHW = vertData[1].fRHW = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawLine( int iX1, int iY1, int iX2, int iY2, DWORD dwColor )
{

  DrawLine( D3DXVECTOR3( (float)iX1, (float)iY1, 0 ), D3DXVECTOR3( (float)iX2, (float)iY2, 0 ), dwColor );

}



void CD3DViewer::DrawFreeLine( const D3DXVECTOR3& posAnfang, const D3DXVECTOR3& posEnde, DWORD dwColor1, DWORD dwColor2 )
{

  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
  }

  struct CUSTOMVERTEX
  {
    D3DXVECTOR3   position; // The position
    D3DCOLOR    color;    // The color
  };

  CUSTOMVERTEX          vertData[2];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

  vertData[0].position = posAnfang;
  vertData[0].color = dwColor1;

  vertData[1].position = posEnde;
  vertData[1].color = dwColor2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINELIST,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DisplayBoundingBox---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DisplayBoundingBox( const CBoundingBox& BBox, DWORD dwColor )
{

  if ( !BBox.m_bTransformed )
  {
    DisplayBoundingBox( D3DXVECTOR3( BBox.UpperLeftCorner().x,
                                     BBox.UpperLeftCorner().y,
                                     BBox.UpperLeftCorner().z ),
                        D3DXVECTOR3( BBox.LowerRightCorner().x,
                                     BBox.LowerRightCorner().y,
                                     BBox.LowerRightCorner().z ),
                        dwColor );
    return;
  }

  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 0 ).x,
                             BBox.TransformedCorner( 0 ).y,
                             BBox.TransformedCorner( 0 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 1 ).x,
                             BBox.TransformedCorner( 1 ).y,
                             BBox.TransformedCorner( 1 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 1 ).x,
                             BBox.TransformedCorner( 1 ).y,
                             BBox.TransformedCorner( 1 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 2 ).x,
                             BBox.TransformedCorner( 2 ).y,
                             BBox.TransformedCorner( 2 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 2 ).x,
                             BBox.TransformedCorner( 2 ).y,
                             BBox.TransformedCorner( 2 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 3 ).x,
                             BBox.TransformedCorner( 3 ).y,
                             BBox.TransformedCorner( 3 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 3 ).x,
                             BBox.TransformedCorner( 3 ).y,
                             BBox.TransformedCorner( 3 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 0 ).x,
                             BBox.TransformedCorner( 0 ).y,
                             BBox.TransformedCorner( 0 ).z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 4 ).x,
                             BBox.TransformedCorner( 4 ).y,
                             BBox.TransformedCorner( 4 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 5 ).x,
                             BBox.TransformedCorner( 5 ).y,
                             BBox.TransformedCorner( 5 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 5 ).x,
                             BBox.TransformedCorner( 5 ).y,
                             BBox.TransformedCorner( 5 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 6 ).x,
                             BBox.TransformedCorner( 6 ).y,
                             BBox.TransformedCorner( 6 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 6 ).x,
                             BBox.TransformedCorner( 6 ).y,
                             BBox.TransformedCorner( 6 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 7 ).x,
                             BBox.TransformedCorner( 7 ).y,
                             BBox.TransformedCorner( 7 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 7 ).x,
                             BBox.TransformedCorner( 7 ).y,
                             BBox.TransformedCorner( 7 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 4 ).x,
                             BBox.TransformedCorner( 4 ).y,
                             BBox.TransformedCorner( 4 ).z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 0 ).x,
                             BBox.TransformedCorner( 0 ).y,
                             BBox.TransformedCorner( 0 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 4 ).x,
                             BBox.TransformedCorner( 4 ).y,
                             BBox.TransformedCorner( 4 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 1 ).x,
                             BBox.TransformedCorner( 1 ).y,
                             BBox.TransformedCorner( 1 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 5 ).x,
                             BBox.TransformedCorner( 5 ).y,
                             BBox.TransformedCorner( 5 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 2 ).x,
                             BBox.TransformedCorner( 2 ).y,
                             BBox.TransformedCorner( 2 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 6 ).x,
                             BBox.TransformedCorner( 6 ).y,
                             BBox.TransformedCorner( 6 ).z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( BBox.TransformedCorner( 3 ).x,
                             BBox.TransformedCorner( 3 ).y,
                             BBox.TransformedCorner( 3 ).z ),
                D3DXVECTOR3( BBox.TransformedCorner( 7 ).x,
                             BBox.TransformedCorner( 7 ).y,
                             BBox.TransformedCorner( 7 ).z ),
                dwColor );

}



void CD3DViewer::DisplayBoundingBox( const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, DWORD dwColor )
{

  // Bounding Box
  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v1.y,
                             v1.z ),
                D3DXVECTOR3( v1.x,
                             v2.y,
                             v1.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v2.y,
                             v1.z ),
                D3DXVECTOR3( v2.x,
                             v2.y,
                             v1.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v2.y,
                             v1.z ),
                D3DXVECTOR3( v2.x,
                             v1.y,
                             v1.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v1.y,
                             v1.z ),
                D3DXVECTOR3( v1.x,
                             v1.y,
                             v1.z ),
                dwColor );

  // Seite 2
  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v1.y,
                             v2.z ),
                D3DXVECTOR3( v1.x,
                             v2.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v2.y,
                             v2.z ),
                D3DXVECTOR3( v2.x,
                             v2.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v2.y,
                             v2.z ),
                D3DXVECTOR3( v2.x,
                             v1.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v1.y,
                             v2.z ),
                D3DXVECTOR3( v1.x,
                             v1.y,
                             v2.z ),
                dwColor );

  // Seite 3
  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v1.y,
                             v1.z ),
                D3DXVECTOR3( v1.x,
                             v1.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v1.x,
                             v2.y,
                             v1.z ),
                D3DXVECTOR3( v1.x,
                             v2.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v1.y,
                             v1.z ),
                D3DXVECTOR3( v2.x,
                             v1.y,
                             v2.z ),
                dwColor );

  DrawFreeLine( D3DXVECTOR3( v2.x,
                             v2.y,
                             v1.z ),
                D3DXVECTOR3( v2.x,
                             v2.y,
                             v2.z ),
                dwColor );

}



/*-SetTexture-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::SetTexture( DWORD dwStage, CDX8Texture* pTexture )
{

  if ( !m_pd3dDevice )
  {
    return;
  }

  if ( !pTexture )
  {
    m_pd3dDevice->SetTexture( dwStage, NULL );
    return;
  }
  pTexture->Touch();
  /*
  if ( pTexture->m_Surface == NULL )
  {
    dh::Log( "SetTexture - Surface = NULL - trying to load %s\n", pTexture->m_strFileName.c_str() );
    if ( !LoadSurface( pTexture, pTexture->m_dwCreationFlags, pTexture->m_dwTransparentColor ) )
    {
      dh::Log( "-failed\n" );
    }
  }
  */

  if ( FAILED( m_pd3dDevice->SetTexture( dwStage, pTexture->m_Surface ) ) )
  {
    dh::Log( "SetTexture failed\n" );
  }

}



void CD3DViewer::SetTexture( DWORD dwStage, CDX8MultiTexture *pTexture, DWORD dwSection )
{

  if ( !m_pd3dDevice )
  {
    return;
  }
  if ( !pTexture )
  {
    m_pd3dDevice->SetTexture( dwStage, NULL );
    return;
  }
  if ( pTexture->m_vectSections.size() <= dwSection )
  {
    return;
  }
  if ( pTexture->m_vectSections[0] == NULL )
  {
    pTexture->RecreateSections();
    if ( pTexture->m_vectSections[0] == NULL )
    {
      return;
    }
  }

  SetTexture( dwStage, pTexture->m_vectSections[dwSection]->m_pTexture );

}



/*-SetTextureStageState-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetTextureStageState( GR::u32 dwStage, D3DTEXTURESTAGESTATETYPE dwState, GR::u32 dwValue )
{

  if ( !m_pd3dDevice )
  {
    return E_FAIL;
  }
  if ( dwStage >= 8 )
  {
    return E_FAIL;
  }

  if ( CDX8RenderStateBlock::SetTextureStageState( dwStage, dwState, dwValue ) == S_OK )
  {
    return S_OK;
  }

  HRESULT   hRes = m_pd3dDevice->SetTextureStageState( dwStage, dwState, dwValue );
  if ( FAILED( hRes ) )
  {
    dh::Log( "SetTextureStageState %0x %0x to %0x failed (Error %0x)", dwStage, dwState, dwValue, hRes );
  }
  return hRes;

}



/*-SetRenderState-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetRenderState( D3DRENDERSTATETYPE dwState, GR::u32 dwValue )
{

  if ( !m_pd3dDevice )
  {
    return E_FAIL;
  }

  if ( CDX8RenderStateBlock::SetRenderState( dwState, dwValue ) == S_OK )
  {
    // war schon gesetzt
    return S_OK;
  }

  HRESULT   hRes = m_pd3dDevice->SetRenderState( dwState, dwValue );
  if ( SUCCEEDED( hRes ) )
  {
    return hRes;
  }
  dh::Log( "SetRenderState %0x to %0x failed (Error %0x)", dwState, dwValue, hRes );
  return hRes;

}



/*-SetVertexShader------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::SetVertexShader( GR::u32 Handle )
{

  if ( CDX8RenderStateBlock::SetVertexShader( Handle ) == S_OK )
  {
    return S_OK;
  }
  return m_pd3dDevice->SetVertexShader( m_dwVertexShader );

}



/*-DrawRect-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawRect( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor )
{

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[5];


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

  SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

  vertData[0].position = posAnfang;
  vertData[0].fRHW      = 1.0f;
  vertData[0].color = dwColor;

  vertData[1].position = posAnfang;
  vertData[1].fRHW      = 1.0f;
  vertData[1].position.x += vectSize.x - 1.0f;
  vertData[1].color = dwColor;

  vertData[2].position = posAnfang;
  vertData[2].fRHW      = 1.0f;
  vertData[2].position.x += vectSize.x - 1.0f;
  vertData[2].position.y += vectSize.y - 1.0f;
  vertData[2].color = dwColor;

  vertData[3].position = posAnfang;
  vertData[3].fRHW      = 1.0f;
  vertData[3].position.y += vectSize.y - 1.0f;
  vertData[3].color = dwColor;

  vertData[4].position = posAnfang;
  vertData[4].fRHW      = 1.0f;
  vertData[4].color = dwColor;


  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINESTRIP,
                  4,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawFreeRect( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor )
{

  DrawFreeLine( posAnfang, D3DXVECTOR3( posAnfang.x + vectSize.x, posAnfang.y, posAnfang.z ), dwColor );
  DrawFreeLine( D3DXVECTOR3( posAnfang.x + vectSize.x, posAnfang.y, posAnfang.z ),
                D3DXVECTOR3( posAnfang.x + vectSize.x, posAnfang.y + vectSize.y, posAnfang.z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( posAnfang.x + vectSize.x, posAnfang.y + vectSize.y, posAnfang.z ),
                D3DXVECTOR3( posAnfang.x, posAnfang.y + vectSize.y, posAnfang.z ),
                dwColor );
  DrawFreeLine( D3DXVECTOR3( posAnfang.x, posAnfang.y + vectSize.y, posAnfang.z ),
                posAnfang,
                dwColor );

}



void CD3DViewer::DrawRect( const int iX, const int iY, const int iWidth, const int iHeight, DWORD dwColor )
{

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[5];


  D3DXVECTOR3   posAnfang( (float)iX, (float)iY, 0.0f );


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

  SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

  SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
  SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

  vertData[0].position  = posAnfang;
  vertData[0].fRHW      = 1.0f;
  vertData[0].color     = dwColor;

  vertData[1].position  = posAnfang;
  vertData[1].fRHW      = 1.0f;
  vertData[1].position.x += iWidth - 1.0f;
  vertData[1].color     = dwColor;

  vertData[2].position  = posAnfang;
  vertData[2].fRHW      = 1.0f;
  vertData[2].position.x += iWidth - 1.0f;
  vertData[2].position.y += iHeight - 1.0f;
  vertData[2].color     = dwColor;

  vertData[3].position  = posAnfang;
  vertData[3].fRHW      = 1.0f;
  vertData[3].position.y += iHeight - 1.0f;
  vertData[3].color     = dwColor;

  vertData[4].position  = posAnfang;
  vertData[4].fRHW      = 1.0f;
  vertData[4].color     = dwColor;


  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_LINESTRIP,
                  4,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawBox--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawBox( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor1,
                          DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[4];


  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
    dwColor3 = dwColor1;
    dwColor4 = dwColor1;
  }
  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  SetTexture( 0, NULL );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

  vertData[0].position    = posAnfang;
  vertData[0].fRHW        = 1.0f;
  vertData[0].color       = dwColor1;

  vertData[1].position    = posAnfang;
  vertData[1].position.x  += vectSize.x;
  vertData[1].fRHW        = 1.0f;
  vertData[1].color       = dwColor2;

  vertData[2].position    = posAnfang;
  vertData[2].position.y  += vectSize.y;
  vertData[2].fRHW        = 1.0f;
  vertData[2].color       = dwColor3;

  vertData[3].position    = posAnfang;
  vertData[3].position.x  += vectSize.x;
  vertData[3].position.y  += vectSize.y;
  vertData[3].fRHW        = 1.0f;
  vertData[3].color       = dwColor4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawBox( int iX, int iY, int iWidth, int iHeight, DWORD dwColor1,
                          DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
  };

  CUSTOMVERTEX          vertData[4];

  D3DXVECTOR3           posAnfang( (float)iX, (float)iY, 0 );


  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
    dwColor3 = dwColor1;
    dwColor4 = dwColor1;
  }
  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
  SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

  SetTexture( 0, NULL );

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );

  vertData[0].position    = posAnfang;
  vertData[0].fRHW        = 1.0f;
  vertData[0].color       = dwColor1;

  vertData[1].position    = posAnfang;
  vertData[1].position.x  += iWidth;
  vertData[1].fRHW        = 1.0f;
  vertData[1].color       = dwColor2;

  vertData[2].position    = posAnfang;
  vertData[2].position.y  += iHeight;
  vertData[2].fRHW        = 1.0f;
  vertData[2].color       = dwColor3;

  vertData[3].position    = posAnfang;
  vertData[3].position.x  += iWidth;
  vertData[3].position.y  += iHeight;
  vertData[3].fRHW        = 1.0f;
  vertData[3].color       = dwColor4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawQuad-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawQuad( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize,
                           DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
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

  float   fDelta = -0.5f;

  float   fRHW = 1.0f;


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = floorf( posAnfang.x ) + fDelta;
  vertData[0].position.y  = floorf( posAnfang.y ) + fDelta;
  vertData[0].position.z  = floorf( posAnfang.z );
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = 0.0f;
  vertData[0].fTV         = 0.0f;

  vertData[1].position.x  = floorf( posAnfang.x ) + (int)vectSize.x + fDelta;
  vertData[1].position.y  = floorf( posAnfang.y ) + fDelta;
  vertData[1].position.z  = floorf( posAnfang.z );
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = 1.0f;
  vertData[1].fTV         = 0.0f;

  vertData[2].position.x  = floorf( posAnfang.x ) + fDelta;
  vertData[2].position.y  = floorf( posAnfang.y ) + (int)vectSize.y + fDelta;
  vertData[2].position.z  = floorf( posAnfang.z );
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = 0.0f;
  vertData[2].fTV         = 1.0f;

  vertData[3].position.x  = floorf( posAnfang.x ) + (int)vectSize.x + fDelta;
  vertData[3].position.y  = floorf( posAnfang.y ) + (int)vectSize.y + fDelta;
  vertData[3].position.z  = floorf( posAnfang.z );
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = 1.0f;
  vertData[3].fTV         = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawQuad( int fX, int fY, int iWidth, int iHeight,
                           DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }

  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      float         fRHW;
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  CUSTOMVERTEX      vertData[4];

  float   fDelta = -0.5f;

  float   fRHW = 1.0f;


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = floorf( fX + 0.1f ) + fDelta;
  vertData[0].position.y  = floorf( fY + 0.1f ) + fDelta;
  vertData[0].position.z  = floorf( 0.1f );
  vertData[0].fRHW        = fRHW;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = 0.0f;
  vertData[0].fTV         = 0.0f;

  vertData[1].position.x  = floorf( fX + 0.1f ) + iWidth + fDelta;
  vertData[1].position.y  = floorf( fY + 0.1f ) + fDelta;
  vertData[1].position.z  = floorf( 0.1f );
  vertData[1].fRHW        = fRHW;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = 1.0f;
  vertData[1].fTV         = 0.0f;

  vertData[2].position.x  = floorf( fX + 0.1f ) + fDelta;
  vertData[2].position.y  = floorf( fY + 0.1f ) + iHeight + fDelta;
  vertData[2].position.z  = floorf( 0.1f );
  vertData[2].fRHW        = fRHW;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = 0.0f;
  vertData[2].fTV         = 1.0f;

  vertData[3].position.x  = floorf( fX + 0.1f ) + iWidth + fDelta;
  vertData[3].position.y  = floorf( fY + 0.1f ) + iHeight + fDelta;
  vertData[3].position.z  = floorf( 0.1f );
  vertData[3].fRHW        = fRHW;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = 1.0f;
  vertData[3].fTV         = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawQuadDetail-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawQuadDetail( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize,
                                 float fTU1, float fTV1, float fTU2, float fTV2,
                                 DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
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

  float   fDelta = -0.5f;


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = floorf( posAnfang.x + 0.1f ) + fDelta;
  vertData[0].position.y  = floorf( posAnfang.y + 0.1f ) + fDelta;
  vertData[0].position.z  = floorf( posAnfang.z + 0.1f );
  vertData[0].fRHW        = 1.0f;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = floorf( posAnfang.x + 0.1f ) + (int)vectSize.x + fDelta;
  vertData[1].position.y  = floorf( posAnfang.y + 0.1f ) + fDelta;
  vertData[1].position.z  = floorf( posAnfang.z + 0.1f );
  vertData[1].fRHW        = 1.0f;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV1;

  vertData[2].position.x  = floorf( posAnfang.x + 0.1f ) + fDelta;
  vertData[2].position.y  = floorf( posAnfang.y + 0.1f ) + (int)vectSize.y + fDelta;
  vertData[2].position.z  = floorf( posAnfang.z + 0.1f );
  vertData[2].fRHW        = 1.0f;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = fTU1;
  vertData[2].fTV         = fTV2;

  vertData[3].position.x  = floorf( posAnfang.x + 0.1f ) + (int)vectSize.x + fDelta;
  vertData[3].position.y  = floorf( posAnfang.y + 0.1f ) + (int)vectSize.y + fDelta;
  vertData[3].position.z  = floorf( posAnfang.z + 0.1f );
  vertData[3].fRHW        = 1.0f;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = fTU2;
  vertData[3].fTV         = fTV2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawQuadDetail( const int iX, const int iY, const int iWidth, const int iHeight,
                                 float fTU1, float fTV1, float fTU2, float fTV2,
                                 DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
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

  float   fDelta = -0.5f;


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = floorf( iX + 0.1f ) + fDelta;
  vertData[0].position.y  = floorf( iY + 0.1f ) + fDelta;
  vertData[0].position.z  = floorf( 0.1f );
  vertData[0].fRHW        = 1.0f;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = floorf( iX + 0.1f ) + iWidth + fDelta;
  vertData[1].position.y  = floorf( iY + 0.1f ) + fDelta;
  vertData[1].position.z  = floorf( 0.1f );
  vertData[1].fRHW        = 1.0f;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV1;

  vertData[2].position.x  = floorf( iX + 0.1f ) + fDelta;
  vertData[2].position.y  = floorf( iY + 0.1f ) + iHeight + fDelta;
  vertData[2].position.z  = floorf( 0.1f );
  vertData[2].fRHW        = 1.0f;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = fTU1;
  vertData[2].fTV         = fTV2;

  vertData[3].position.x  = floorf( iX + 0.1f ) + iWidth + fDelta;
  vertData[3].position.y  = floorf( iY + 0.1f ) + iHeight + fDelta;
  vertData[3].position.z  = floorf( 0.1f );
  vertData[3].fRHW        = 1.0f;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = fTU2;
  vertData[3].fTV         = fTV2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawQuadDetail( const int iX, const int iY, const int iWidth, const int iHeight,
                                 float fTU1, float fTV1, float fTU2, float fTV2,
                                 float fTU3, float fTV3, float fTU4, float fTV4,
                                 DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
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

  float   fDelta = -0.5f;


  SetVertexShader( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );

  vertData[0].position.x  = floorf( iX + 0.1f ) + fDelta;
  vertData[0].position.y  = floorf( iY + 0.1f ) + fDelta;
  vertData[0].position.z  = floorf( 0.1f );
  vertData[0].fRHW        = 1.0f;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position.x  = floorf( iX + 0.1f ) + iWidth + fDelta;
  vertData[1].position.y  = floorf( iY + 0.1f ) + fDelta;
  vertData[1].position.z  = floorf( 0.1f );
  vertData[1].fRHW        = 1.0f;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position.x  = floorf( iX + 0.1f ) + fDelta;
  vertData[2].position.y  = floorf( iY + 0.1f ) + iHeight + fDelta;
  vertData[2].position.z  = floorf( 0.1f );
  vertData[2].fRHW        = 1.0f;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position.x  = floorf( iX + 0.1f ) + iWidth + fDelta;
  vertData[3].position.y  = floorf( iY + 0.1f ) + iHeight + fDelta;
  vertData[3].position.z  = floorf( 0.1f );
  vertData[3].fRHW        = 1.0f;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawFreeTriangle-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeTriangle( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                   D3DXVECTOR3& v3,
                                   DWORD dwColor1, DWORD dwColor2, DWORD dwColor3 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor1;
  }
  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      D3DXVECTOR3   normal; // The position
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  D3DXVECTOR3           normal;

  D3DXVec3Cross( &normal, &v1, &v2 );
  D3DXVec3Normalize( &normal, &normal );

  CUSTOMVERTEX          vertData[3];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 );

  vertData[0].position    = v1;
  vertData[0].normal      = normal;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = 0.0f;
  vertData[0].fTV         = 0.0f;

  vertData[1].position    = v2;
  vertData[1].normal      = normal;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = 1.0f;
  vertData[1].fTV         = 0.0f;

  vertData[2].position    = v3;
  vertData[2].normal      = normal;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = 0.0f;
  vertData[2].fTV         = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  1,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawFreeBox----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeBox( D3DXVECTOR3& vPos, D3DXVECTOR3& vSize, DWORD dwColor )
{

  DrawFreeQuad( D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z ),
                D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z ),
                dwColor );
  DrawFreeQuad( D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z + vSize.z ),
                dwColor );
  DrawFreeQuad( D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z + vSize.z ),
                dwColor );
  DrawFreeQuad( D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z ),
                D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z ),
                dwColor );

  DrawFreeQuad( D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x,            vPos.y + vSize.y,         vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y + vSize.y,         vPos.z ),
                dwColor );

  DrawFreeQuad( D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z ),
                D3DXVECTOR3( vPos.x,            vPos.y,                   vPos.z + vSize.z ),
                D3DXVECTOR3( vPos.x + vSize.x,  vPos.y,                   vPos.z + vSize.z ),
                dwColor );

}



/*-DrawFreeQuad---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeQuad( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                               D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                               DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      D3DXVECTOR3   normal; // The position
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  D3DXVECTOR3           normal;

  D3DXVec3Cross( &normal, &( v2 - v1 ), &( v3 - v1 ) );
  D3DXVec3Normalize( &normal, &normal );

  CUSTOMVERTEX          vertData[4];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 );

  vertData[0].position    = v1;
  vertData[0].normal      = normal;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = 0.0f;
  vertData[0].fTV         = 0.0f;

  vertData[1].position    = v2;
  vertData[1].normal      = normal;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = 1.0f;
  vertData[1].fTV         = 0.0f;

  vertData[2].position    = v3;
  vertData[2].normal      = normal;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = 0.0f;
  vertData[2].fTV         = 1.0f;

  vertData[3].position    = v4;
  vertData[3].normal      = normal;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = 1.0f;
  vertData[3].fTV         = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawFreeQuad( D3DXVECTOR3& vPos, D3DXVECTOR3& vSize,
                               DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      D3DXVECTOR3   normal; // The position
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  D3DXVECTOR3           normal;

  D3DXVec3Cross( &normal, &D3DXVECTOR3( vSize.x, 0, 0 ), &D3DXVECTOR3( 0, vSize.y, 0 ) );
  D3DXVec3Normalize( &normal, &normal );

  CUSTOMVERTEX          vertData[4];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 );

  vertData[0].position    = vPos;
  vertData[0].normal      = normal;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = 0.0f;
  vertData[0].fTV         = 0.0f;

  vertData[1].position    = vPos;
  vertData[1].position.x  += vSize.x;
  vertData[1].normal      = normal;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = 1.0f;
  vertData[1].fTV         = 0.0f;

  vertData[2].position    = vPos;
  vertData[2].position.y  += vSize.y;
  vertData[2].normal      = normal;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = 0.0f;
  vertData[2].fTV         = 1.0f;

  vertData[3].position    = vPos + vSize;
  vertData[3].normal      = normal;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = 1.0f;
  vertData[3].fTV         = 1.0f;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-DrawFreeQuad---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeQuadDetail( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                     D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                     float fTU1, float fTV1, float fTU2, float fTV2,
                                     DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      D3DXVECTOR3   normal; // The position
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  D3DXVECTOR3           normal;

  D3DXVec3Cross( &normal, &v1, &v2 );
  D3DXVec3Normalize( &normal, &normal );

  CUSTOMVERTEX          vertData[4];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 );

  vertData[0].position    = v1;
  vertData[0].normal      = normal;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = v2;
  vertData[1].normal      = normal;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV1;

  vertData[2].position    = v3;
  vertData[2].normal      = normal;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = fTU1;
  vertData[2].fTV         = fTV2;

  vertData[3].position    = v4;
  vertData[3].normal      = normal;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = fTU2;
  vertData[3].fTV         = fTV2;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



void CD3DViewer::DrawFreeQuadDetail( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                     D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                     float fTU1, float fTV1, float fTU2, float fTV2,
                                     float fTU3, float fTV3, float fTU4, float fTV4,
                                     DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 )
{

  if ( ( dwColor2 == 0 )
  &&   ( dwColor3 == 0 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
  struct CUSTOMVERTEX
  {
      D3DXVECTOR3   position; // The position
      D3DXVECTOR3   normal; // The position
      D3DCOLOR      color;    // The color
      float         fTU,
                    fTV;
  };

  D3DXVECTOR3           normal;

  D3DXVec3Cross( &normal, &( v2 - v1 ), &( v3 - v2 ) );
  D3DXVec3Normalize( &normal, &normal );

  CUSTOMVERTEX          vertData[4];


  SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX1 );

  vertData[0].position    = v1;
  vertData[0].normal      = normal;
  vertData[0].color       = dwColor1;
  vertData[0].fTU         = fTU1;
  vertData[0].fTV         = fTV1;

  vertData[1].position    = v2;
  vertData[1].normal      = normal;
  vertData[1].color       = dwColor2;
  vertData[1].fTU         = fTU2;
  vertData[1].fTV         = fTV2;

  vertData[2].position    = v3;
  vertData[2].normal      = normal;
  vertData[2].color       = dwColor3;
  vertData[2].fTU         = fTU3;
  vertData[2].fTV         = fTV3;

  vertData[3].position    = v4;
  vertData[3].normal      = normal;
  vertData[3].color       = dwColor4;
  vertData[3].fTU         = fTU4;
  vertData[3].fTV         = fTV4;

  m_pd3dDevice->DrawPrimitiveUP(
                  D3DPT_TRIANGLESTRIP,
                  2,
                  &vertData,
                  sizeof( vertData[0] ) );

}



/*-Print----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::Print( int iX, int iY, const char *szText, DWORD dwColor )
{


  if ( m_pFont == NULL )
  {
    return;
  }
  if ( !m_bReady )
  {
    return;
  }
  RECT        rc;
  rc.left = iX;
  rc.top = iY;
  rc.right = iX + 20;
  rc.bottom = iY + 20;
  rc.bottom = rc.top + m_pFont->DrawText( szText, -1, &rc, DT_CALCRECT, dwColor );

  m_pFont->DrawText( szText, -1, &rc, 0, dwColor );

}




/*-CastRayFromScreen----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::CastRayFromScreen( int iX, int iY, D3DXVECTOR3& v1, D3DXVECTOR3& v2, float fZ1Value )
{

  if ( m_pd3dDevice == NULL )
  {
    return;
  }

  D3DXVECTOR3 vectOrig1 = D3DXVECTOR3( float( iX ), float( iY ), fZ1Value ),
              vectOrig2 = D3DXVECTOR3( float( iX ), float( iY ), 1.0f );

  D3DXVec3Unproject( &v1,
                     &vectOrig1,
                     &m_viewPort,
                     &m_matProjection,
                     &m_matView,
                     &m_matWorld );
  D3DXVec3Unproject( &v2,
                     &vectOrig2,
                     &m_viewPort,
                     &m_matProjection,
                     &m_matView,
                     &m_matWorld );

}



/*-TransformToScreen----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::TransformToScreen( D3DXVECTOR3& vect )
{

  if ( m_pd3dDevice == NULL )
  {
    return;
  }

  D3DXVec3Project( &vect, &vect, &m_viewPort, &m_matProjection, &m_matView, &m_matWorld );

}



/*-InitDeviceObjects----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::InitDeviceObjects()
{

  RestoreDeviceObjects();
  return S_OK;

}



/*-RestoreDeviceObjects-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::RestoreDeviceObjects()
{

  SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
  SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
  SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
  SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

  SetRenderState( D3DRS_DITHERENABLE, TRUE );
  SetRenderState( D3DRS_SPECULARENABLE, TRUE );
  SetRenderState( D3DRS_AMBIENT,  0xffffffff );

  // Set the transform matrices
  D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
  D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f,  0.0f );
  D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f,  0.0f );
  D3DXMATRIX  matWorld, matView, matProj;

  D3DXMatrixIdentity( &matWorld );
  D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
  FLOAT fAspect = m_d3dsdBackBuffer.Width / (FLOAT)m_d3dsdBackBuffer.Height;
  //D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );

  D3DXMatrixOrthoOffCenterLH( &matProj,
                0, (float)m_d3dsdBackBuffer.Width,
                (float)m_d3dsdBackBuffer.Height, 0,
                1.0f, 100.0f );


  SetTransform( D3DTS_WORLD,      &matWorld );
  SetTransform( D3DTS_VIEW,       &matView );
  SetTransform( D3DTS_PROJECTION, &matProj );

  return S_OK;

}



/*-InvalidateDeviceObjects----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::InvalidateDeviceObjects()
{

  return S_OK;

}



/*-DeleteDeviceObjects--------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::DeleteDeviceObjects()
{

  return S_OK;

}



/*-DeleteAppDeviceObjects-----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DeleteAppDeviceObjects()
{

  std::set<CVertexBuffer*>::iterator   it( m_setVBs.begin() );
  while ( it != m_setVBs.end() )
  {
    //- setinhalt sind definitionsgemäss const, deshalb funzt safedelete hier nicht
    CVertexBuffer*    pVB = *it;

    *it = NULL;
    ++it;
    delete pVB;
  }
  m_setVBs.clear();

  ReleaseTextures();
  ReleaseFonts();

}



/*-SetWorldMatrixForBillBoardingAt--------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::SetWorldMatrixForBillBoardingAt( const D3DXVECTOR3& vecPos )
{

  D3DXMATRIX  matTransposed;

  D3DXMatrixTranspose( &matTransposed, &m_matView );

  matTransposed._14 = matTransposed._24 = matTransposed._34 = 0.0f;

  //position the billboard
  matTransposed._41 = vecPos.x;
  matTransposed._42 = vecPos.y;
  matTransposed._43 = vecPos.z;

  // Set as world transformation matrix
  SetTransform( D3DTS_WORLD, &matTransposed );

}



/*-CreateVertexBuffer---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CVertexBuffer* CD3DViewer::CreateVertexBuffer( DWORD dwPrimitiveCount, DWORD dwPrimitiveSize, DWORD dwBufferSize, D3DPRIMITIVETYPE dwPrimitiveType, DWORD dwUsage, DWORD dwFVF, D3DPOOL d3dPool )
{

  if ( !m_pd3dDevice )
  {
    dh::Log( "CD3DViewer::CreateVertexBuffer: no Device" );
    return NULL;
  }

  CVertexBuffer*   pDX8VB = NULL;

  pDX8VB = new( std::nothrow ) CVertexBuffer( this );
  if ( pDX8VB == NULL )
  {
    return NULL;
  }

  pDX8VB->m_dwPrimitiveSize = dwPrimitiveSize;
  pDX8VB->m_dwPrimitiveCount = dwPrimitiveCount;
  pDX8VB->m_dwCompleteBufferSize  = dwBufferSize;
  pDX8VB->m_d3dPrimType = dwPrimitiveType;
  pDX8VB->m_d3dPool               = d3dPool;
  pDX8VB->m_dwFVF                 = dwFVF;
  pDX8VB->m_dwUsage               = dwUsage;

  pDX8VB->CreateBuffers( m_pd3dDevice );

  pDX8VB->m_pData = new( std::nothrow ) BYTE[dwBufferSize];
  if ( pDX8VB->m_pData == NULL )
  {
    delete pDX8VB;
    return NULL;
  }

  m_setVBs.insert( pDX8VB );

  return pDX8VB;

}



/*-RemoveVertexBuffer---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::RemoveVertexBuffer( CVertexBuffer *pVB )
{

  if ( !pVB )
  {
    return;
  }

  std::set<CVertexBuffer*>::iterator   it( m_setVBs.find( pVB ) );
  if ( it != m_setVBs.end() )
  {
    m_setVBs.erase( it );
  }

}



/*-DumpModes------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/


void CD3DViewer::DumpModes()
{

  for ( size_t iAdapter = 0; iAdapter < m_vectAdapters.size(); iAdapter++ )
  {
    tD3DAdapterInfo&     ai = m_vectAdapters[iAdapter];

    dh::Log( "Adapter: %s", ai.d3dAdapterIdentifier.Description );

    for ( size_t iDevices = 0; iDevices < ai.vectDevices.size(); iDevices++ )
    {
      tD3DDeviceInfo&    di = ai.vectDevices[iDevices];

      dh::Log( "-Device: %s", di.strName.c_str() );

      for ( size_t iMode = 0; iMode < di.m_vectMode.size(); iMode++ )
      {
        tD3DModeInfo&  mi = di.m_vectMode[iMode];

        dh::Log( " -Mode: %dx%dx%d", mi.m_dwWidth, mi.m_dwHeight, mi.m_d3dfPixel );
      }
    }
  }

}



void CD3DViewer::DrawTextureSection( int iX, int iY, const tTextureSection& TexSection, DWORD dwColor, int iWidth, int iHeight, DWORD dwAlternativeFlags )
{
  SetTexture( 0, TexSection.m_pTexture );

  if ( iWidth == -1 )
  {
    iWidth  = TexSection.m_Width;
  }
  if ( iHeight == -1 )
  {
    iHeight = TexSection.m_Height;
  }

  if ( dwAlternativeFlags == 0 )
  {
    dwAlternativeFlags = TexSection.m_Flags;
  }

  if ( dwAlternativeFlags & tTextureSection::TSF_HCENTER )
  {
    iX -= iWidth / 2;
  }
  if ( dwAlternativeFlags & tTextureSection::TSF_VCENTER )
  {
    iY -= iHeight / 2;
  }

  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( dwAlternativeFlags & tTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( dwAlternativeFlags & tTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }

  if ( ( dwAlternativeFlags & tTextureSection::TSF_ROTATE_270 ) == tTextureSection::TSF_ROTATE_270 )
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
  else if ( dwAlternativeFlags & tTextureSection::TSF_ROTATE_90 )
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
  else if ( dwAlternativeFlags & tTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }

  if ( TexSection.m_pTexture )
  {
    DrawQuadDetail( iX, iY, iWidth, iHeight,
                    fTU[0][0], fTV[0][0],
                    fTU[1][0], fTV[1][0],
                    fTU[0][1], fTV[0][1],
                    fTU[1][1], fTV[1][1],
                    dwColor );
  }
  else
  {
    DrawQuad( iX, iY, iWidth, iHeight, dwColor );
  }

}



/*-DrawTextureSection---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DrawFreeTextureSection( D3DXVECTOR3& v1, D3DXVECTOR3& v2, D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                         const tTextureSection& TexSection, DWORD dwColor1, DWORD dwColor2,
                                         DWORD dwColor3, DWORD dwColor4, DWORD dwAlternativeFlags )
{

  SetTexture( 0, TexSection.m_pTexture );

  DWORD   dwFlags = TexSection.m_Flags;

  if ( dwAlternativeFlags )
  {
    dwFlags = dwAlternativeFlags;
  }

  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
    dwColor3 = dwColor1;
    dwColor4 = dwColor1;
  }

  float   fTU[2][2] = { { TexSection.m_TU[0], TexSection.m_TU[0] }, { TexSection.m_TU[1], TexSection.m_TU[1] } },
          fTV[2][2] = { { TexSection.m_TV[0], TexSection.m_TV[1] }, { TexSection.m_TV[0], TexSection.m_TV[1] } };


  if ( dwFlags & tTextureSection::TSF_H_MIRROR )
  {
    std::swap( fTU[0][0], fTU[1][0] );
    std::swap( fTU[0][1], fTU[1][1] );
  }
  if ( dwFlags & tTextureSection::TSF_V_MIRROR )
  {
    std::swap( fTV[0][0], fTV[0][1] );
    std::swap( fTV[1][0], fTV[1][1] );
  }

  if ( ( dwFlags & tTextureSection::TSF_ROTATE_270 ) == tTextureSection::TSF_ROTATE_270 )
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
  else if ( dwFlags & tTextureSection::TSF_ROTATE_90 )
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
  else if ( dwFlags & tTextureSection::TSF_ROTATE_180 )
  {
    std::swap( fTU[0][0], fTU[1][1] );
    std::swap( fTV[0][0], fTV[1][1] );
    std::swap( fTU[1][0], fTU[0][1] );
    std::swap( fTV[1][0], fTV[0][1] );
  }

  DrawFreeQuadDetail( v1, v2, v3, v4,
                      fTU[0][0], fTV[0][0],
                      fTU[1][0], fTV[1][0],
                      fTU[0][1], fTV[0][1],
                      fTU[1][1], fTV[1][1],
                      dwColor1, dwColor2, dwColor3, dwColor4 );

}



HWND CD3DViewer::GetSafeHwnd() const
{

  return m_hWnd;

}



/*-IsTextureFormatOk----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CD3DViewer::IsTextureFormatOk( D3DFORMAT TextureFormat )
{

  HRESULT hr = m_pD3D->CheckDeviceFormat(
          m_dwCurrentAdapter,
          D3DDEVTYPE_HAL,
          m_pCurrentModeInfo->m_d3dfPixel,
          0,
          D3DRTYPE_TEXTURE,
          TextureFormat);

  return SUCCEEDED( hr );

}



/*-CreateTexture--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Texture* CD3DViewer::CreateTexture( DWORD dwWidth, DWORD dwHeight, D3DFORMAT formatSurface, DWORD dwMipMapLevels )
{

  if ( ( dwWidth == 0 )
  ||   ( dwHeight == 0 ) )
  {
    return NULL;
  }

  if ( !IsTextureFormatOk( formatSurface ) )
  {
    if ( formatSurface == D3DFMT_R8G8B8 )
    {
      formatSurface = D3DFMT_X8R8G8B8;
      if ( !IsTextureFormatOk( formatSurface ) )
      {
        return NULL;
      }
    }
    else
    {
      return NULL;
    }
  }

  int   iTWidth = dwWidth,
        iTHeight = dwHeight;

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

  pTexture = new( std::nothrow ) CDX8Texture();
  if ( pTexture == NULL )
  {
    return NULL;
  }

  if ( FAILED( m_pd3dDevice->CreateTexture(
                 iTWidth,
                 iTHeight,
                 dwMipMapLevels,
                 0,
                 formatSurface,
                 D3DPOOL_MANAGED,
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

  pTexture->m_vSize.x             = ddsd.Width;
  pTexture->m_vSize.y             = ddsd.Height;
  pTexture->m_d3dfPixelFormat     = ddsd.Format;
  pTexture->m_dwCreationFlags     = DX8::TF_DONT_LOAD;
  pTexture->m_dwReferenzen        = 1;
  pTexture->m_dwTransparentColor  = 0;

  pTexture->m_pViewer             = this;

  return pTexture;

}



/*-CopyImageToSingleSurface---------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::CopyImageToSingleSurface(
                                  GR::Graphic::ImageData* pImageData,
                                  LPDIRECT3DTEXTURE8 resultingSurface,
                                  int iWidth, int iHeight,
                                  DWORD dwTransparentColor,
                                  DWORD dwFlags,
                                  int iCopyX1, int iCopyY1, int iCopyX2, int iCopyY2,
                                  int iTargetMipMapLevel )
{

  D3DLOCKED_RECT        lockedRect;

  DWORD                 dwTargetMipMapLevel = 0;

  if ( iTargetMipMapLevel > -1 )
  {
    dwTargetMipMapLevel = iTargetMipMapLevel;
  }

  if ( FAILED( resultingSurface->LockRect(
               dwTargetMipMapLevel,
               &lockedRect,
               NULL,
               0 ) ) )
  {
    return E_FAIL;
  }

  // Zielformat feststellen
  D3DSURFACE_DESC ddsd;
  resultingSurface->GetLevelDesc( dwTargetMipMapLevel, &ddsd );


  bool    bConvertedImageData = false;

  GR::Graphic::ImageData*   pNewData = NULL;

  GR::Graphic::eImageFormat    newFormat = GR::Graphic::IF_UNKNOWN;

  switch ( ddsd.Format )
  {
    case D3DFMT_A1R5G5B5:
      newFormat = GR::Graphic::IF_A1R5G5B5;
      break;
    case D3DFMT_A8R8G8B8:
      newFormat = GR::Graphic::IF_A8R8G8B8;
      break;
    case D3DFMT_X8R8G8B8:
      newFormat = GR::Graphic::IF_X8R8G8B8;
      break;
    case D3DFMT_R5G6B5:
      newFormat = GR::Graphic::IF_R5G6B5;
      break;
    case D3DFMT_A8:
      newFormat = GR::Graphic::IF_A8;
      break;
    case D3DFMT_A4R4G4B4:
      newFormat = GR::Graphic::IF_A4R4G4B4;
      break;
    default:
      dh::Log( "CD3DViewer::CopyImageToSingleSurface - Conversion not supported (%d to %d)\n",
                ddsd.Format, newFormat );
      break;
  }

  if ( newFormat != GR::Graphic::IF_UNKNOWN )
  {
    if ( pNewData = pImageData->ConvertTo( newFormat, lockedRect.Pitch, true, dwTransparentColor,
                                           iCopyX1, iCopyY1, iCopyX2 - iCopyX1 + 1, iCopyY2 - iCopyY1 + 1 ) )
    {
      bConvertedImageData = true;
    }
  }

  if ( bConvertedImageData )
  {
    BYTE*   pTarget;
    BYTE*   pSource;

    for ( int i = 0; i < pNewData->Height(); i++ )
    {
      pSource = (BYTE*)pNewData->Data() + i * pNewData->LineOffsetInBytes();
      pTarget = (BYTE*)lockedRect.pBits + i * lockedRect.Pitch;

      memcpy( pTarget, pSource, pNewData->BytesProPixel() * pNewData->Width() );
    }
    resultingSurface->UnlockRect( dwTargetMipMapLevel );

    if ( ( m_bAutoMipMapGeneration )
    &&   ( iTargetMipMapLevel == -1 ) )
    {
      // Auto-Mipmap generieren
      DWORD           dwMipMaps = resultingSurface->GetLevelCount();

      D3DSURFACE_DESC ddsdMipMap;

      for ( DWORD i = 1; i < dwMipMaps; i++ )
      {

        resultingSurface->GetLevelDesc( i, &ddsdMipMap );


        if ( SUCCEEDED( resultingSurface->LockRect(
                        i,
                        &lockedRect,
                        NULL,
                        0 ) ) )
        {
          int   iXStep = ddsd.Width / ddsdMipMap.Width;
          int   iYStep = ddsd.Height / ddsdMipMap.Height;
          int   iBytes = pNewData->BytesProPixel();

          int   iFaktor = 1 << i;



          BYTE*   pSource;
          for ( int y = 0; y < pNewData->Height() / iFaktor; y++ )
          {
            pSource = (BYTE*)pNewData->Data() + iYStep * y * pNewData->LineOffsetInBytes();
            pTarget = (BYTE*)lockedRect.pBits + y * lockedRect.Pitch;
            for ( int x = 0; x < pNewData->Width() / iFaktor; x++ )
            {
              memcpy( pTarget, pSource, iBytes );
              pSource += iXStep * iBytes;
              pTarget += iBytes;
            }
          }

          resultingSurface->UnlockRect( i );
        }
      }

    }

    delete pNewData;
  }
  else
  {
    dh::Log( "\n\n-----------ImageData NOT converted %d nach %d\n", pImageData->ImageFormat(), ddsd.Format );
    ( (BYTE*)lockedRect.pBits )[0] = 255;
    resultingSurface->UnlockRect( dwTargetMipMapLevel );
  }


  return S_OK;

}



/*-CopyImageToSplitSurfaces---------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

HRESULT CD3DViewer::CopyImageToSplitSurfaces( CDX8MultiTexture *pNewTexture, GR::Graphic::ImageData* pImageData, DWORD dwFlags, DWORD dwTransparentColor )
{

  int     iCopyX1,
          iCopyY1,
          iCopyX2,
          iCopyY2,
          iCopyWidth = pImageData->Width(),
          iCopyHeight = pImageData->Height();

  if ( iCopyWidth > 256 )
  {
    iCopyWidth = 256;
  }
  if ( iCopyHeight > 256 )
  {
    iCopyHeight = 256;
  }

  pNewTexture->m_iTilesX = ( pImageData->Width() - 1 ) / iCopyWidth + 1;
  pNewTexture->m_iTilesY = ( pImageData->Height() - 1 ) / iCopyHeight + 1;

  pNewTexture->m_iTileWidth   = iCopyWidth;
  pNewTexture->m_iTileHeight  = iCopyHeight;

  //LogText( "TileSize %dx%d - Loader %dx%d", iCopyWidth, iCopyHeight, pLoader->m_dwWidth, pLoader->m_dwHeight );

  pNewTexture->m_vSize.x = pImageData->Width();
  pNewTexture->m_vSize.y = pImageData->Height();

  for ( int iZeile = 0; iZeile < pNewTexture->m_iTilesY; iZeile++ )
  {
    for ( int iSpalte = 0; iSpalte < pNewTexture->m_iTilesX; iSpalte++ )
    {
      D3DFORMAT           formatToUse = m_pCurrentModeInfo->m_d3dfPixel;

      tTextureSection*  pTextureInfo      = NULL;

      pTextureInfo = new( std::nothrow ) tTextureSection();
      if ( pTextureInfo == NULL )
      {
        std::vector<tTextureSection*>::iterator   it( pNewTexture->m_vectSections.begin() );
        while ( it != pNewTexture->m_vectSections.end() )
        {
          tTextureSection*  pTexInfo = *it;

          if ( pTexInfo )
          {
            delete pTexInfo;
          }

          ++it;
        }

        pNewTexture->m_vectSections.clear();
        pNewTexture->m_iTilesX = 0;
        pNewTexture->m_iTilesY = 0;
        return E_FAIL;
      }

      iCopyWidth = pImageData->Width(),
      iCopyHeight = pImageData->Height();

      if ( iCopyWidth > 256 )
      {
        iCopyWidth = 256;
      }
      if ( iCopyHeight > 256 )
      {
        iCopyHeight = 256;
      }

      iCopyX1 = iSpalte * iCopyWidth;
      iCopyY1 = iZeile * iCopyHeight;
      iCopyX2 = iCopyX1 + iCopyWidth - 1;
      iCopyY2 = iCopyY1 + iCopyHeight - 1;
      if ( iCopyX2 >= pImageData->Width() )
      {
        iCopyWidth -= ( iCopyX2 - pImageData->Width() + 1 );
        iCopyX2 = pImageData->Width() - 1;
      }
      if ( iCopyY2 >= pImageData->Height() )
      {
        iCopyHeight -= ( iCopyY2 - pImageData->Height() + 1 );
        iCopyY2 = pImageData->Height() - 1;
      }

      if ( ( pImageData->ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
      ||   ( pImageData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
      {
        dwFlags |= DX8::TF_ALPHA_MASK;
      }

      if ( dwFlags & DX8::TF_HOLES )
      {
        if ( IsTextureFormatOk( D3DFMT_A1R5G5B5 ) )
        {
          formatToUse = D3DFMT_A1R5G5B5;
        }
      }
      if ( dwFlags & DX8::TF_CONVERT_TO_ALPHA )
      {
        // 8bit Alpha ist hier wichtiger als Farbtreue

        if ( IsTextureFormatOk( D3DFMT_A8R3G3B2 ) )
        {
          formatToUse = D3DFMT_A8R3G3B2;
        }
        else if ( IsTextureFormatOk( D3DFMT_A8R8G8B8 ) )
        {
          formatToUse = D3DFMT_A8R8G8B8;
        }
        else if ( IsTextureFormatOk( D3DFMT_A4R4G4B4 ) )
        {
          formatToUse = D3DFMT_A4R4G4B4;
        }
      }
      if ( dwFlags & DX8::TF_ALPHA_MASK )
      {
        if ( IsTextureFormatOk( D3DFMT_A8R8G8B8 ) )
        {
          formatToUse = D3DFMT_A8R8G8B8;
        }
        else if ( IsTextureFormatOk( D3DFMT_A4R4G4B4 ) )
        {
          formatToUse = D3DFMT_A4R4G4B4;
        }
        else if ( IsTextureFormatOk( D3DFMT_A8R3G3B2 ) )
        {
          formatToUse = D3DFMT_A8R3G3B2;
        }
      }

      // passende Textur-Größe errechnen
      int   iTWidth = iCopyWidth,
            iTHeight = iCopyHeight;

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

      pTextureInfo->m_pTexture = CreateTexture( iTWidth, iTHeight, formatToUse );
      pTextureInfo->m_pTexture->m_dwCreationFlags |= DX8::TF_DONT_LOAD;

      // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
      pTextureInfo->m_TU[1] = (float)iCopyWidth / (float)pTextureInfo->m_pTexture->m_vSize.x;
      pTextureInfo->m_TV[1] = (float)iCopyHeight / (float)pTextureInfo->m_pTexture->m_vSize.y;

      dwTransparentColor &= 0x00ffffff;

      pNewTexture->m_dwCreationFlags = dwFlags;
      pNewTexture->m_dwTransparentColor = dwTransparentColor;

      if ( FAILED( CopyImageToSingleSurface( pImageData,
                                             pTextureInfo->m_pTexture->m_Surface,
                                             pNewTexture->m_iTileWidth,
                                             pNewTexture->m_iTileHeight,
                                             dwTransparentColor,
                                             dwFlags,
                                             iCopyX1,
                                             iCopyY1,
                                             iCopyX2,
                                             iCopyY2 ) ) )
      {
        dh::Log( "CopyImageToSingleSurface failed" );
        if ( pTextureInfo )
        {
          delete pTextureInfo;
        }

        std::vector<tTextureSection*>::iterator   it( pNewTexture->m_vectSections.begin() );
        while ( it != pNewTexture->m_vectSections.end() )
        {
          tTextureSection*  pTexInfo = *it;

          if ( pTexInfo )
          {
            delete pTexInfo;
          }

          ++it;
        }

        pNewTexture->m_vectSections.clear();
        pNewTexture->m_iTilesX = 0;
        pNewTexture->m_iTilesY = 0;
        return E_FAIL;
      }

      pNewTexture->m_vectSections.push_back( pTextureInfo );
    }
  }

  return S_OK;

}



BOOL CD3DViewer::LoadSurfaces( CDX8MultiTexture *pNewTexture, DWORD dwFlags, DWORD dwTransparentColor )
{

  GR::Graphic::ImageData*   pData = ImageFormatManager::Instance().LoadData( pNewTexture->m_strFileName.c_str() );
  if ( pData == NULL )
  {
    dh::Log( "CD3DViewer::LoadSurfaces %s failed\n", pNewTexture->m_strFileName.c_str() );
    return FALSE;
  }
  CopyImageToSplitSurfaces( pNewTexture, pData, dwFlags, dwTransparentColor );

  delete pData;

  return TRUE;

}



CDX8MultiTexture* CD3DViewer::LoadMultiTexture( const char *szFileName, DWORD dwFlags, DWORD dwTransparentColor, DWORD dwMipmapLevels )
{

  //- Hier prüfen, ob eine Datei schon als Textur geladen wurde
  char      szEintrag[2 * MAX_PATH];

  wsprintf( szEintrag, "%s%x%x%x", szFileName, dwFlags, dwTransparentColor, dwMipmapLevels );
  tMapMultiTextures::iterator   itPos( m_mapMultiTextures.find( GR::String( szEintrag ) ) );

  if ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture* pTex = itPos->second;
    // es ist eindeutig die selbe Textur
    pTex->m_dwReferenzen++;
    return itPos->second;
  }

  CDX8MultiTexture*       pNewTexture = NULL;

  pNewTexture = new( std::nothrow ) CDX8MultiTexture();
  if ( pNewTexture == NULL )
  {
    return NULL;
  }

  pNewTexture->m_strFileName = szFileName;
  pNewTexture->m_vectSections.clear();
  pNewTexture->m_pViewer = this;
  pNewTexture->m_dwMipmapLevels = dwMipmapLevels;

  if ( !LoadSurfaces( pNewTexture, dwFlags, dwTransparentColor ) )
  {
    delete pNewTexture;
    return NULL;
  }

  // LogText( "Add Texture (%s) to map", szEintrag );
  m_mapMultiTextures[GR::String( szEintrag )] = pNewTexture;

  return pNewTexture;

}



BOOL CD3DViewer::LoadSurface( CDX8Texture* pNewTexture, DWORD dwFlags, DWORD dwTransparentColor )
{

  GR::Graphic::ImageData*   pImageData = ImageFormatManager::Instance().LoadData( pNewTexture->m_strFileName.c_str() );
  if ( pImageData == NULL )
  {
    dh::Log( "LoadSurface (%s) failed!\n", pNewTexture->m_strFileName.c_str() );
    return FALSE;
  }

  D3DFORMAT             formatToUse = m_pCurrentModeInfo->m_d3dfPixel;

  if ( dwFlags & DX8::TF_CONVERT_TO_ALPHA )
  {
    dwFlags |= DX8::TF_ALPHA_MASK;
    if ( pImageData->ImageFormat() == GR::Graphic::IF_PALETTED )
    {
      GR::Graphic::ImageData*   pNewData = new GR::Graphic::ImageData();

      pNewData->CreateData( pImageData->Width(), pImageData->Height(), GR::Graphic::IF_A8, pImageData->LineOffsetInBytes() );

      memcpy( pNewData->Data(), pImageData->Data(), pImageData->DataSize() );
      delete pImageData;
      pImageData = pNewData;
    }
  }

  pNewTexture->m_Surface = NULL;
  if ( ( pImageData->ImageFormat() == GR::Graphic::IF_A1R5G5B5 )
  ||   ( pImageData->ImageFormat() == GR::Graphic::IF_A8R8G8B8 ) )
  {
    dwFlags |= DX8::TF_ALPHA_MASK;
  }

  if ( dwFlags & DX8::TF_HOLES )
  {
    if ( IsTextureFormatOk( D3DFMT_A1R5G5B5 ) )
    {
      formatToUse = D3DFMT_A1R5G5B5;
    }
  }
  if ( dwFlags & DX8::TF_ALPHA_MASK )
  {
    if ( IsTextureFormatOk( D3DFMT_A8R8G8B8 ) )
    {
      formatToUse = D3DFMT_A8R8G8B8;
    }
    else if ( IsTextureFormatOk( D3DFMT_A4R4G4B4 ) )
    {
      formatToUse = D3DFMT_A4R4G4B4;
    }
    else if ( IsTextureFormatOk( D3DFMT_A8R3G3B2 ) )
    {
      formatToUse = D3DFMT_A8R3G3B2;
    }
  }

  // Texture erzeugen
  int   iTWidth = pImageData->Width(),
        iTHeight = pImageData->Height();

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


  if ( FAILED( m_pd3dDevice->CreateTexture(
                 iTWidth,
                 iTHeight,
                 pNewTexture->m_dwMipmapLevels,
                 0,
                 formatToUse,
                 D3DPOOL_MANAGED,
                 &pNewTexture->m_Surface ) ) )
  {
    // failed to load (suupa)
    pNewTexture->m_Surface = NULL;

    delete pImageData;

    return FALSE;
  }

  // eventuell (sehr wahrscheinlich sogar) angepaßte Größen der Surfaces holen
  D3DSURFACE_DESC ddsd;
  pNewTexture->m_Surface->GetLevelDesc( 0, &ddsd );

  dwTransparentColor &= 0x00ffffff;

  if ( FAILED( CopyImageToSingleSurface( pImageData,
                                         pNewTexture->m_Surface,
                                         pImageData->Width(),
                                         pImageData->Height(),
                                         dwTransparentColor,
                                         dwFlags,
                                         0,
                                         0,
                                         pImageData->Width() - 1,
                                         pImageData->Height() - 1 ) ) )
  {
    delete pImageData;
    return FALSE;
  }

  pNewTexture->m_vSize.x             = ddsd.Width;
  pNewTexture->m_vSize.y             = ddsd.Height;
  pNewTexture->m_d3dfPixelFormat     = ddsd.Format;
  pNewTexture->m_dwCreationFlags     = dwFlags;
  pNewTexture->m_dwReferenzen        = 1;
  pNewTexture->m_dwTransparentColor  = dwTransparentColor;

  delete pImageData;

  return TRUE;

}



BOOL CD3DViewer::LoadMipMapTexture( CDX8Texture *pNewTexture, const char* szFileName, DWORD dwMipmapLevel, DWORD dwFlags, DWORD dwTransparentColor )
{

  if ( pNewTexture == NULL )
  {
    return FALSE;
  }
  if ( pNewTexture->m_dwMipmapLevels <= dwMipmapLevel )
  {
    // ungültiger Mipmap-Level-Wert
    return FALSE;
  }

  GR::Graphic::ImageData*   pImageData = ImageFormatManager::Instance().LoadData( szFileName );
  if ( pImageData == NULL )
  {
    dh::Log( "LoadMipMapTexture (%s) failed!\n", szFileName );
    return FALSE;
  }

  D3DSURFACE_DESC ddsd;
  if ( !SUCCEEDED( pNewTexture->m_Surface->GetLevelDesc( dwMipmapLevel, &ddsd ) ) )
  {
    delete pImageData;
    return FALSE;
  }

  if ( ( ddsd.Width != pImageData->Width() )
  ||   ( ddsd.Height != pImageData->Height() ) )
  {
    dh::Log( "LoadMipMapTexture (%s) - invalid width/height (is %dx%d, should be %dx%d)!\n",
              szFileName,
              pImageData->Width(),
              pImageData->Height(),
              ddsd.Width,
              ddsd.Height );
    delete pImageData;
    return FALSE;
  }

  dwTransparentColor &= 0x00ffffff;

  if ( FAILED( CopyImageToSingleSurface( pImageData,
                                         pNewTexture->m_Surface,
                                         pImageData->Width(),
                                         pImageData->Height(),
                                         dwTransparentColor,
                                         dwFlags,
                                         0,
                                         0,
                                         pImageData->Width() - 1,
                                         pImageData->Height() - 1,
                                         dwMipmapLevel ) ) )
  {
    delete pImageData;
    return FALSE;
  }

  delete pImageData;

  return TRUE;

}



/*-LoadTexture----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Texture* CD3DViewer::LoadTexture( const char* szFileName, DWORD dwFlags, DWORD dwTransparentColor, DWORD dwMipmapLevels )
{

  //- Hier prüfen, ob eine Datei schon als Textur geladen wurde
  CDX8Texture*    pOldTexture = CTextureManager::Instance().Request( szFileName );

  if ( pOldTexture )
  {
    pOldTexture->m_dwReferenzen++;
    return pOldTexture;
  }

  CDX8Texture*          pTexture = NULL;

  pTexture = new( std::nothrow ) CDX8Texture();
  if ( pTexture == NULL )
  {
    return NULL;
  }

  pTexture->m_Surface             = NULL;
  pTexture->m_strFileName         = szFileName;
  pTexture->m_dwResourceID        = 0;
  pTexture->m_hResourceInstance   = NULL;
  pTexture->m_strResourceType     = "";
  pTexture->m_pViewer = this;
  pTexture->m_dwCreationFlags     = dwFlags;
  pTexture->m_dwTransparentColor  = dwTransparentColor;
  pTexture->m_dwMipmapLevels = dwMipmapLevels;

  if ( ( pTexture->m_dwCreationFlags & DX8::TF_DONT_LOAD ) == 0 )
  {
    if ( !LoadSurface( pTexture, dwFlags, dwTransparentColor ) )
    {
      delete pTexture;
      return NULL;
    }
  }
  pTexture->m_bIsLoaded = true;

  CTextureManager::Instance().Insert( szFileName, pTexture );

  return pTexture;

}



/*-FindMultiTexture-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8MultiTexture* CD3DViewer::FindMultiTexture( const char *szFileName )
{

  tMapMultiTextures::iterator itPos( m_mapMultiTextures.find( szFileName ) );

  if ( itPos == m_mapMultiTextures.end() )
  {
    return NULL;
  }
  return itPos->second;

}



/*-LoadTextureFromResource----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8MultiTexture* CD3DViewer::LoadMultiTextureFromResource( HINSTANCE hInstance,
                                                            DWORD dwResourceID,
                                                            const char *szResourceType,
                                                            DWORD dwFlags,
                                                            DWORD dwTransparentColor,
                                                            DWORD dwMipmapLevels )
{

  //- Hier prüfen, ob eine Datei schon als Textur geladen wurde
  char      szEintrag[2 * MAX_PATH];

  dwFlags |= DX8::TF_FROM_RESOURCE;
  wsprintf( szEintrag, "%d%x%x%x", dwResourceID, dwFlags, dwTransparentColor, dwMipmapLevels );

  tMapMultiTextures::iterator   itPos( m_mapMultiTextures.find( GR::String( szEintrag ) ) );

  if ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture* pTex = itPos->second;

    // es ist eindeutig die selbe Textur
    pTex->m_dwReferenzen++;
    return itPos->second;
  }

  CDX8MultiTexture*       pNewTexture = NULL;

  pNewTexture = new( std::nothrow ) CDX8MultiTexture();
  if ( pNewTexture == NULL )
  {
    return NULL;
  }


  pNewTexture->m_pViewer = this;
  pNewTexture->m_vectSections.clear();
  pNewTexture->m_strFileName = szEintrag;
  pNewTexture->m_hResourceInstance = hInstance;
  pNewTexture->m_dwResourceID     = dwResourceID;
  pNewTexture->m_strResourceType = szResourceType;
  pNewTexture->m_dwMipmapLevels = dwMipmapLevels;

  if ( !LoadSurfaces( pNewTexture, dwFlags | DX8::TF_FROM_RESOURCE, dwTransparentColor ) )
  {
    delete pNewTexture;
    return NULL;
  }

  m_mapMultiTextures[GR::String( szEintrag )] = pNewTexture;

  return pNewTexture;

}



/*-RecreateTextures-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CD3DViewer::RecreateTextures()
{

  // Standard-Textures
  CTextureManager::Instance().RecreateTextures();
  // Multi-Textures
  tMapMultiTextures::iterator  itPos( m_mapMultiTextures.begin() );
  while ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture   *pTextureEntry = itPos->second;

    if ( pTextureEntry == NULL )
    {
      dh::Log( "RecreateTextures: Schwerer Fehler! pTextureEntry = NULL!" );
      continue;
    }

    pTextureEntry->ReleaseSections();
    pTextureEntry->m_vectSections.clear();
    pTextureEntry->RecreateSections();
    itPos++;
  }

  return TRUE;

}



/*-DumpTextures---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DumpTextures()
{

  dh::Log( "dump textures>" );
  tMapMultiTextures::iterator  itPos( m_mapMultiTextures.begin() );
  while ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture   *pTextureEntry = itPos->second;

    dh::Log( ">texture %d (%s)", pTextureEntry->m_dwCreationFlags, itPos->first.c_str() );

    itPos++;
  }

}



/*-ReleaseTextures------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::ReleaseTextures()
{

  CTextureManager::Instance().ReleaseTextures();

  tMapMultiTextures::iterator  itPos( m_mapMultiTextures.begin() );
  while ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture   *pTextureEntry = itPos->second;
    pTextureEntry->ReleaseSections();

    itPos++;
  }

}



/*-DestroyTexture-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DestroyTexture( CDX8Texture* pTextureEntry )
{

  if ( pTextureEntry == NULL )
  {
    return;
  }

  if ( pTextureEntry->m_dwReferenzen > 1 )
  {
    // von dieser Textur gab es mehrere Referenzen, nur den Zähler runter
    pTextureEntry->m_dwReferenzen--;
    return;
  }

  CTextureManager::Instance().Remove( pTextureEntry );

  if ( pTextureEntry->m_Surface )
  {
    pTextureEntry->m_Surface->Release();
    pTextureEntry->m_Surface = NULL;
  }
  delete pTextureEntry;

}



/*-DestroyTexture-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DestroyTexture( CDX8MultiTexture *pTextureEntry )
{

  if ( pTextureEntry == NULL )
  {
    return;
  }

 tMapMultiTextures::iterator  itPos( m_mapMultiTextures.begin() );
  while ( itPos != m_mapMultiTextures.end() )
  {
    if ( pTextureEntry == itPos->second )
    {
      if ( pTextureEntry->m_dwReferenzen > 1 )
      {
        // von dieser Textur gab es mehrere Referenzen, nur den Zähler runter
        pTextureEntry->m_dwReferenzen--;
        return;
      }
      pTextureEntry->ReleaseSections();

      delete pTextureEntry;
      pTextureEntry = NULL;

      //- die iterator-returnende erase-func  gibt es nur in der verhunzten VC++6-STL
      // itPos = m_mapMultiTextures.erase( itPos );
      m_mapMultiTextures.erase( itPos );
      return;
    }
    itPos++;
  }

}



/*-DestroyTextures------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CD3DViewer::DestroyTextures()
{

  //CTextureManager::Instance().DeleteAllEntries();

  tMapMultiTextures::iterator  itPos( m_mapMultiTextures.begin() );
  while ( itPos != m_mapMultiTextures.end() )
  {
    CDX8MultiTexture   *pTextureEntry = itPos->second;

    pTextureEntry->ReleaseSections();

    // LogText( "delete all Texture (%s) to map", itPos->first.c_str() );
    delete pTextureEntry;

    itPos++;
  }
  m_mapMultiTextures.clear();

}



/*-ReleaseSections------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8MultiTexture::ReleaseSections()
{

  for ( unsigned int j = 0; j < m_vectSections.size(); j++ )
  {
    if ( m_vectSections[j] != NULL )
    {
      delete m_vectSections[j];
      m_vectSections[j] = NULL;
    }
  }

}



/*-RecreateSections-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CDX8MultiTexture::RecreateSections()
{

  if ( !m_pViewer )
  {
    return FALSE;
  }

  ReleaseSections();
  m_vectSections.clear();

  if ( !m_pViewer->LoadSurfaces( this, m_dwCreationFlags, m_dwTransparentColor ) )
  {
    return FALSE;
  }
  return TRUE;

}




CD3DViewer& CD3DViewer::Instance()
{

  return *m_pViewer;

}



int CD3DViewer::Width() const
{

  return m_dwRenderWidth;

}



int CD3DViewer::Height() const
{

  return m_dwRenderHeight;

}



void CD3DViewer::SetLight( size_t iIndex, D3DLIGHT8& Light )
{

  if ( ( iIndex < 8 )
  &&   ( m_pd3dDevice ) )
  {
    m_Lights[iIndex] = Light;
    m_pd3dDevice->SetLight( (DWORD)iIndex, &Light );
  }

}



void CD3DViewer::EnableLight( size_t iIndex, bool bEnable )
{

  if ( ( iIndex < 8 )
  &&   ( m_pd3dDevice ) )
  {
    m_pd3dDevice->LightEnable( (DWORD)iIndex, bEnable ? TRUE : FALSE );

    m_bLightEnabled[iIndex] = bEnable;
  }

}
