#include <Interface/IIOStream.h>

#include <debug/debugclient.h>
#include <debug/DebugService.h>

#include <WinSys/SubclassManager.h>
#include <Xtreme/Environment/XWindow.h>
#include <Misc/Misc.h>

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>


#include "DXInput.h"



#pragma comment ( lib, "dinput8.lib" )
#pragma comment ( lib, "dxguid.lib" )



CDXInput::CDXInput() :
  m_CurrentMouseButtonsEnumerated( 0 )
{
  m_DeleteThisTask        = false;
  m_Initialized           = false;
  m_InputActive           = true;
  m_InputMButtonsSwapped  = false;

  m_hwndInput             = NULL;
  m_hwndRelativeMouseTo   = NULL;

  m_VirtualKeys           = 1;
  m_pVirtualKey           = NULL;
}



CDXInput::~CDXInput()
{
  Release();
}



BOOL CALLBACK CDXInput::EnumObjects( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{
  CDXInput*   pInput = (CDXInput*)pvRef;

  if ( !( lpddoi->dwType & DIDFT_NODATA ) )
  {
    // da kommt was raus
    pInput->m_pDebugger->Log( "Input.Full", "Enum Object %s, API Type %d, API Data Offset %d", lpddoi->tszName, lpddoi->dwType, lpddoi->dwOfs );

    tInputDevice&   Device = pInput->m_Devices[pInput->m_CurrentEnumDevice];

    tInputCtrl      NewCtrl( pInput->m_CurrentEnumDevice, lpddoi->tszName, Xtreme::CT_UNKNOWN, lpddoi->dwOfs );
    NewCtrl.m_APIType = lpddoi->dwType;

    NewCtrl.m_VirtualIndex = pInput->m_VirtualKeys;

    // Zuweisung von Default-Enums
    GR::u32     devType = Xtreme::VKT_UNKNOWN;

    if ( ( pInput->m_CurrentEnumDeviceType & 0xff ) == DI8DEVTYPE_KEYBOARD )
    {
      NewCtrl.m_Type = Xtreme::CT_BUTTON;
      devType = Xtreme::VKT_KEYBOARD;

      UINT vKey = MapVirtualKey( lpddoi->dwOfs, 1 );

      // Special case - cursor keys are dumbly mapped to numpad on a laptop!!
      if ( ( vKey == VK_LEFT )
      ||   ( vKey == VK_RIGHT )
      ||   ( vKey == VK_UP )
      ||   ( vKey == VK_DOWN ) )
      {
        vKey = 0;
      }

      if ( vKey != 0 )
      {
        NewCtrl.m_VirtualKeyCode = vKey;
      }
    }
    else if ( ( pInput->m_CurrentEnumDeviceType & 0xff ) == DI8DEVTYPE_MOUSE )
    {
      devType = Xtreme::VKT_DEFAULT_MOUSE;
    }
    else if ( pInput->m_CurrentEnumJoystickDevice == 0 )
    {
      devType = Xtreme::VKT_JOYSTICK_1;
    }
    else if ( pInput->m_CurrentEnumJoystickDevice == 1 )
    {
      devType = Xtreme::VKT_JOYSTICK_2;
    }
    else
    {
      pInput->m_pDebugger->Log( "Input.General", "Unknown Device Type found" );
    }

    if ( lpddoi->dwType & DIDFT_ABSAXIS )
    {
      if ( lpddoi->dwFlags & DIDOI_FFACTUATOR )
      {
        DeviceInfo* pInfo = (DeviceInfo*)( pInput->m_Devices[pInput->m_CurrentEnumDevice].m_pDevicePointer );
        pInfo->AxisCount++;
        pInfo->AxisOffsets.push_back( lpddoi->dwOfs );
      }

      DIPROPRANGE     diRange;

      diRange.diph.dwSize = sizeof( diRange );
      diRange.diph.dwHeaderSize = sizeof( DIPROPHEADER );
      diRange.diph.dwHow = DIPH_BYID;
      diRange.diph.dwObj = lpddoi->dwType;

      if ( lpddoi->guidType == GUID_XAxis )
      {
        devType |= Xtreme::VKT_X_AXIS;
      }
      if ( lpddoi->guidType == GUID_YAxis )
      {
        devType |= Xtreme::VKT_Y_AXIS;
      }

      HRESULT hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->GetProperty( DIPROP_RANGE, (LPDIPROPHEADER)&diRange );
      if ( SUCCEEDED( hRes ) )
      {
        NewCtrl.m_Min = diRange.lMin;
        NewCtrl.m_Max = diRange.lMax;
        NewCtrl.m_Type = Xtreme::CT_AXIS;

        pInput->m_pDebugger->Log( "Input.Full", "Absolute Axis found, Range %d bis %d", NewCtrl.m_Min, NewCtrl.m_Max );
      }
      else
      {
        pInput->m_pDebugger->Log( "Input.General", "GetProperty failed %x", hRes );
      }
      // 2 Keys pro Achse
      pInput->m_VirtualKeys += 2;

      NewCtrl.m_AnalogIndex = pInput->AddDeviceAnalogControl( pInput->m_CurrentEnumDevice, NewCtrl );
    }
    else if ( lpddoi->dwType & DIDFT_RELAXIS )
    {
      DIPROPRANGE     diRange;

      diRange.diph.dwSize = sizeof( diRange );
      diRange.diph.dwHeaderSize = sizeof( DIPROPHEADER );
      diRange.diph.dwHow = DIPH_BYID;
      diRange.diph.dwObj = lpddoi->dwType;

      if ( lpddoi->guidType == GUID_XAxis )
      {
        devType |= Xtreme::VKT_X_AXIS;
      }
      if ( lpddoi->guidType == GUID_YAxis )
      {
        devType |= Xtreme::VKT_Y_AXIS;
      }

      NewCtrl.m_Type = Xtreme::CT_RELATIVE_AXIS;

      HRESULT hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->GetProperty( DIPROP_RANGE, (LPDIPROPHEADER)&diRange );
      if ( SUCCEEDED( hRes ) )
      {
        NewCtrl.m_Min = diRange.lMin;
        NewCtrl.m_Max = diRange.lMax;
        pInput->m_pDebugger->Log( "Input.Full", "Relative Axis found, Range %d bis %d", NewCtrl.m_Min, NewCtrl.m_Max );
      }
      else
      {
        pInput->m_pDebugger->Log( "Input.General", "GetProperty failed %x", hRes );
      }
      // 2 Keys pro Achse
      pInput->m_VirtualKeys += 2;
    }
    else if ( lpddoi->dwType & DIDFT_PSHBUTTON )
    {
      if ( devType == Xtreme::VKT_DEFAULT_MOUSE )
      {
        // only 3 buttons per mouse, otherwise we overshoot!
        // man staune, das DataOffset ist der tatsächliche Tastatur-Index!!
        if ( pInput->m_CurrentMouseButtonsEnumerated >= 3 )
        {
          return TRUE;
        }
        ++pInput->m_CurrentMouseButtonsEnumerated;
      }
      NewCtrl.m_Type = Xtreme::CT_BUTTON;
      pInput->m_pDebugger->Log( "Input.Full", "Pushbutton found" );
      ++pInput->m_VirtualKeys;
    }
    else if ( lpddoi->dwType & DIDFT_POV )
    {
      // 4 Keys pro POV (NESW)
      NewCtrl.m_Type = Xtreme::CT_HAT_SWITCH;
      pInput->m_pDebugger->Log( "Input.Full", "POV found" );
      pInput->m_VirtualKeys += 4;

      NewCtrl.m_AnalogIndex = pInput->AddDeviceAnalogControl( pInput->m_CurrentEnumDevice, NewCtrl );
    }
    else
    {
      pInput->m_pDebugger->Log( "Input.General", "Unsupported control type found (%x)", lpddoi->dwType );
    }

    NewCtrl.m_DeviceType  = devType;

    int controlIndex = pInput->AddDeviceControl( pInput->m_CurrentEnumDevice, NewCtrl );
    pInput->m_pDebugger->Log( "Input.Full", "Device Control Index (%d)", controlIndex );
  }
  return TRUE;
}



bool CDXInput::Initialize( GR::IEnvironment& Environment )
{
  if ( !XBasicInput::Initialize( Environment ) )
  {
    return false;
  }
  Log( "Input.Full", "DXInput Initialize" );

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

  // BAUSTELLE
  #pragma warning( disable : 4312 )
  HRESULT hResult = DirectInput8Create( (HINSTANCE)(LONG)(LONG_PTR)GetWindowLongPtr( hWnd, GWLP_HINSTANCE ), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_lpDirectInput, NULL); 
  #pragma warning( default : 4312 )
  if ( hResult != DI_OK )
  {
    //MessageBox( hWnd, "Failed to Create DirectInput Instance", "Error", MB_OK | MB_APPLMODAL );
    m_pDebugger->Log( "Input.Full", "DXInput DirectInput8Create failed" );
    m_Initialized = true;
    XBasicInput::Release();
    return FALSE;
  }
  // jetzt die einzelnen Devices erkennen lassen
  // Vector-Speicher reservieren
  m_Devices.reserve( 5 );         // Maus, Tastatur, 3 Gamepads sollte reichen

  m_CurrentEnumDevice             = 0;
  m_CurrentEnumJoystickDevice     = 0;
  m_CurrentMouseButtonsEnumerated = 0;

  m_pDebugger->Log( "Input.Full", "DXInput Enumerating Devices" );

  hResult = m_lpDirectInput->EnumDevices( 0, &EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
  if ( hResult != DI_OK )
  {
    m_pDebugger->Log( "Input.Full", "DXInput Enumerating Devices failed" );

    m_lpDirectInput->Release();
    m_lpDirectInput = NULL;
    m_Initialized = true;
    XBasicInput::Release();
    return FALSE;
  }

  m_pDebugger->Log( "Input.Full", "DXInput Enumerating Devices done" );
  m_pDebugger->Log( "Input.Full", "DXInput Creating Virtual Keys" );

  CreateVirtualKeys();

  m_pDebugger->Log( "Input.Full", "DXInput Creating Virtual Keys done" );

  m_pDebugger->Log( "Input.Full", "DXInput Initialize done" );

  m_Initialized = true;
  return true;
}



bool CDXInput::Release()
{
  if ( !m_Initialized )
  {
    return true;
  }

  m_pDebugger->Log( "Input.Full", "DXInput Release" );

  if ( m_pVirtualKey )
  {
    delete[] m_pVirtualKey;
    m_pVirtualKey = NULL;
  }

  tVectDevices::iterator    itDev( m_Devices.begin() );
  while ( itDev != m_Devices.end() )
  {
    tInputDevice&   Device = *itDev;

    delete[] Device.m_pDeviceData;
    Device.m_pDeviceData = NULL;
    if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice )
    {
      if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect )
      {
        ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect->Stop();
      }
      ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Unacquire();
      ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Release();
      ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice = NULL;
      delete (DeviceInfo*)Device.m_pDevicePointer;
    }

    ++itDev;
  }
  m_Devices.clear();
  m_Controls.clear();

  if ( m_lpDirectInput != NULL )
  {
    m_lpDirectInput->Release();
    m_lpDirectInput = NULL;
  }

  XBasicInput::Release();

  m_pDebugger->Log( "Input.Full", "DXInput Release done" );

  m_Initialized = false;

  return true;
}



BOOL CALLBACK CDXInput::EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
  GR::u32* pdwNumForceFeedbackAxis = (GR::u32*)pContext;

  if ( ( pdidoi->dwFlags & DIDOI_FFACTUATOR ) != 0 )
  {
    (*pdwNumForceFeedbackAxis)++;
  }
  return DIENUM_CONTINUE;
}



bool CDXInput::CreateFFEffect( DeviceInfo* pInfo )
{
  // This application needs only one effect: Applying raw forces.
  DIPROPDWORD dipdw;    dipdw;

  dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
  dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  dipdw.diph.dwObj        = 0;
  dipdw.diph.dwHow        = DIPH_DEVICE;
  dipdw.dwData            = FALSE;

  if ( FAILED( pInfo->pDevice->SetProperty( DIPROP_AUTOCENTER, &dipdw.diph ) ) )
  {
    m_pDebugger->Log( "Input.General", "CreateFFEffect, setproperty failed" );
    return false;
  }

  // clamp the max number since i can't test more than two axes
  if ( pInfo->AxisCount > 2 )
  {
    pInfo->AxisCount = 2;
  }
  if ( pInfo->AxisCount == 0 )
  {
    m_pDebugger->Log( "Input.General", "CreateFFEffect, no axis detected" );
    return false;
  }
  for ( GR::u32 i = 0; i < pInfo->AxisCount; ++i )
  {
    pInfo->AxisValue[i] = 0;
  }
  DICONSTANTFORCE cf              = { 0 };

  DIEFFECT eff;
  ZeroMemory( &eff, sizeof( eff ) );
  eff.dwSize                  = sizeof( DIEFFECT );
  eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
  eff.dwDuration              = INFINITE;
  eff.dwSamplePeriod          = 0;
  eff.dwGain                  = DI_FFNOMINALMAX;
  eff.dwTriggerButton         = DIEB_NOTRIGGER;
  eff.dwTriggerRepeatInterval = 0;
  eff.cAxes                   = pInfo->AxisCount;
  eff.rgdwAxes                = (LPDWORD)&pInfo->AxisOffsets[0];
  eff.rglDirection            = pInfo->AxisValue;
  eff.lpEnvelope              = 0;
  eff.cbTypeSpecificParams    = sizeof( DICONSTANTFORCE );
  eff.lpvTypeSpecificParams   = &cf;
  eff.dwStartDelay            = 0;

  //if ( FAILED( g_pDevice->SetCooperativeLevel( hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
  if ( FAILED( pInfo->pDevice->SetCooperativeLevel( m_hwndInput, DISCL_EXCLUSIVE | DISCL_BACKGROUND ) ) )
  {
    m_pDebugger->Log( "Input.General", "CreateEffect cannot set coop level" );
    return false;
  }

  pInfo->pDevice->Acquire();

  // Create the prepared effect
  if ( FAILED( pInfo->pDevice->CreateEffect( GUID_ConstantForce, &eff, &pInfo->pEffect, NULL ) ) )
  {
    m_pDebugger->Log( "Input.General", "CreateFFEffect, CreateEffect failed" );
    return false;
  }

  return ( pInfo->pEffect != NULL );
}



BOOL CALLBACK CDXInput::EnumDevicesProc( const DIDEVICEINSTANCE *lpddi, LPVOID pvRef )
{
  CDXInput*   pInput = (CDXInput*)pvRef;

  LPDIRECTINPUTDEVICE8    dummyDevice = NULL;

  HRESULT hResult = pInput->m_lpDirectInput->CreateDevice( lpddi->guidInstance, &dummyDevice, NULL );
  if ( hResult == DI_OK )
  {
    pInput->m_pDebugger->Log( "Input.Full", "Enum Device %s", lpddi->tszInstanceName );

    DeviceInfo*   pInfo = new DeviceInfo();
    pInfo->pDevice = dummyDevice;

    tInputDevice    Device( lpddi->tszInstanceName, (GR::up*)pInfo );

    pInput->m_CurrentEnumDevice = pInput->AddDevice( Device );
    pInput->m_CurrentEnumDeviceType = lpddi->dwDevType;

    dummyDevice->EnumObjects( EnumObjects, pInput, DIDFT_ALL );

    if ( ( ( lpddi->dwDevType & 0xff ) != DI8DEVTYPE_MOUSE )
    &&   ( ( lpddi->dwDevType & 0xff ) != DI8DEVTYPE_KEYBOARD ) )
    {
      // ein Joystick/Gamepad/wasauchimmer
      ++pInput->m_CurrentEnumJoystickDevice;
    }

    pInput->SetDataFormat( pInput->m_Devices[pInput->m_CurrentEnumDevice] );

    if ( ( lpddi->guidFFDriver.Data1 != 0 )
    ||   ( lpddi->guidFFDriver.Data2 != 0 )
    ||   ( lpddi->guidFFDriver.Data3 != 0 ) )
    {
      // assume since there's a GUID set that the device supports FF
      pInput->CreateFFEffect( pInfo );
    }
  }

  return DIENUM_CONTINUE;  
}



void CDXInput::Poll()
{
  if ( !m_Initialized )
  {
    return;
  }

  m_PollTicks = GetTickCount();

  for ( GR::u32 i = 0; i < m_VirtualKeys; ++i )
  {
    m_pVirtualKey[i].m_ButtonDown = false;
  }

  POINT           pt;

  GetCursorPos( &pt );
  if ( IsWindow( m_hwndRelativeMouseTo ) )
  {
    ScreenToClient( m_hwndRelativeMouseTo, &pt );
  }

  m_InputWMMouseX = pt.x;
  m_InputWMMouseY = pt.y;

  if ( m_InputActive )
  {
    SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_MOUSE_UPDATE, ( pt.x << 16 ) + pt.y, MouseButton() ) );
  }

  tVectDevices::iterator    itDev( m_Devices.begin() );
  while ( itDev != m_Devices.end() )
  {
    tInputDevice&   Device = *itDev;

    if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice )
    {
      PollDevice( Device );
    }

    ++itDev;
  }
}



void CDXInput::Init()
{
}



void CDXInput::Exit()
{
  RemoveAllListeners();
  Release();
}



void CDXInput::Update( const float fElapsedTime )
{
  FrameCompleted();
  Poll();
}



bool CDXInput::PollDevice( tInputDevice& Device )
{
  HRESULT   hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Poll();
  if ( SUCCEEDED( hRes ) )
  {
    hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->GetDeviceState( (GR::u32)( Device.m_Controls.size() * sizeof( GR::u32 ) ), Device.m_pDeviceData );
    if ( SUCCEEDED( hRes ) )
    {
      ParseDeviceData( Device );
    }
    else
    {
      m_pDebugger->Log( "Input.General", "GetDeviceState failed %x", hRes );
    }
    return true;
  }
  if ( hRes == DIERR_NOTACQUIRED )
  {
    if ( m_InputActive )
    {
      // nur automatisch acquiren, wenn wir aktiv sind
      hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Acquire();
      if ( FAILED( hRes ) )
      {
        m_pDebugger->Log( "Input.General", "Acquire failed %x", hRes );
      }
      else
      {
        if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect )
        {
          ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect->Start( 1, 0 );
        }

        return true;
      }
    }
  }
  else
  {
    m_pDebugger->Log( "Input.General", "Poll failed %x", hRes );
  }
  return false;
}




bool CDXInput::SetDataFormat( tInputDevice& Device )
{
  Device.m_pDeviceData = new GR::u32[Device.m_Controls.size()];

  DIOBJECTDATAFORMAT*   pDataFormat = new DIOBJECTDATAFORMAT[Device.m_Controls.size()];

  for ( size_t iObject = 0; iObject < Device.m_Controls.size(); ++iObject )
  {
    tInputCtrl&   Ctrl = Device.m_Controls[iObject];

    pDataFormat[iObject].dwOfs    = Ctrl.m_APIDataOffset;
    pDataFormat[iObject].dwType   = Ctrl.m_APIType;
    pDataFormat[iObject].pguid    = NULL;
    pDataFormat[iObject].dwFlags  = 0;
  }


  DIDATAFORMAT      diDataFormat;

  diDataFormat.dwSize       = sizeof( diDataFormat );
  diDataFormat.dwObjSize    = sizeof( DIOBJECTDATAFORMAT );
  diDataFormat.dwNumObjs    = (GR::u32)Device.m_Controls.size();
  diDataFormat.dwFlags      = 0;
  diDataFormat.dwDataSize   = (GR::u32)Device.m_Controls.size() * sizeof( GR::u32 );
  diDataFormat.rgodf        = pDataFormat;

  HRESULT hRes = ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->SetDataFormat( &diDataFormat );

  delete[] pDataFormat;

  if ( SUCCEEDED( hRes ) )
  {
    m_pDebugger->Log( "Input.Full", "SetDataFormat succeeded (%d Bytes allocated)",
                      Device.m_Controls.size() * sizeof( GR::u32 ) );
  }
  else
  {
    m_pDebugger->Log( "Input.General", "SetDataFormat failed %x", hRes );
  }
  return SUCCEEDED( hRes );
}




void CDXInput::ParseDeviceData( tInputDevice& Device )
{
  for ( size_t i = 0; i < Device.m_Controls.size(); ++i )
  {
    tInputCtrl&   Ctrl = Device.m_Controls[i];

    GR::u32   value = *(GR::u32*)( (BYTE*)Device.m_pDeviceData + Ctrl.m_APIDataOffset );


    if ( Ctrl.m_APIType & DIDFT_BUTTON )
    {
      if ( ( value & 0x080 ) != 0 )
      {
        m_pDebugger->Log( "Input.Full", "Button pushed (VKey %d, APIDataOffset %d, Name %s)", Ctrl.m_VirtualIndex, Ctrl.m_APIDataOffset, Ctrl.m_Name.c_str() );
      }
      SetButtonPressed( Ctrl.m_VirtualIndex, ( value & 0x080 ) != 0 );
    }
    else if ( Ctrl.m_APIType & DIDFT_ABSAXIS )
    {
      if ( Ctrl.m_AnalogIndex != (GR::u32)-1 )
      {
        // set analog values
        m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_Value = (int)value;
        m_AnalogControls[m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_GlobalIndex].m_Value = (int)value;
      }
      // minus
      SetButtonPressed( Ctrl.m_VirtualIndex, ( (int)value < ( Ctrl.m_Max - Ctrl.m_Min ) / 2 - m_InputJoyTreshold ) );
      // plus
      SetButtonPressed( Ctrl.m_VirtualIndex + 1, ( (int)value > ( Ctrl.m_Max - Ctrl.m_Min ) / 2 + m_InputJoyTreshold ) );
    }
    else if ( Ctrl.m_APIType & DIDFT_RELAXIS )
    {
      int   iDelta = (int)value - Ctrl.m_Min;

      if ( abs( iDelta ) > m_InputMouseTreshold )
      {
        if ( iDelta < 0 )
        {
          SetButtonPressed( Ctrl.m_VirtualIndex, true );
          SetButtonPressed( Ctrl.m_VirtualIndex + 1, false );
        }
        else
        {
          SetButtonPressed( Ctrl.m_VirtualIndex, false );
          SetButtonPressed( Ctrl.m_VirtualIndex + 1, true );
        }
      }
      else
      {
        SetButtonPressed( Ctrl.m_VirtualIndex, false );
        SetButtonPressed( Ctrl.m_VirtualIndex + 1, false );
      }
      Ctrl.m_Min = (int)value;
    }
    else if ( Ctrl.m_APIType & DIDFT_POV )
    {
      if ( LOWORD( value ) != 0xFFFF )
      {
        int     degrees = value / DI_DEGREES;

        if ( Ctrl.m_AnalogIndex != (GR::u32)-1 )
        {
          // set analog values
          m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_Value = (int)degrees;
          m_AnalogControls[m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_GlobalIndex].m_Value = (int)degrees;
        }

        SetButtonPressed( Ctrl.m_VirtualIndex, ( ( degrees >= 225 ) && ( degrees <= 315 ) ) );
        SetButtonPressed( Ctrl.m_VirtualIndex + 1, ( ( degrees >= 45 ) && ( degrees <= 135 ) ) );
        SetButtonPressed( Ctrl.m_VirtualIndex + 2, ( ( degrees <= 45 ) || ( degrees >= 315 ) ) );
        SetButtonPressed( Ctrl.m_VirtualIndex + 3, ( ( degrees >= 135 ) && ( degrees <= 225 ) ) );
      }
      else
      {
        // Mittelstellung
        if ( Ctrl.m_AnalogIndex != (GR::u32)-1 )
        {
          // set analog values
          m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_Value = -1;
          m_AnalogControls[m_Devices[Ctrl.m_Device].m_AnalogControls[Ctrl.m_AnalogIndex].m_GlobalIndex].m_Value = -1;
        }

        SetButtonPressed( Ctrl.m_VirtualIndex + 0, false );
        SetButtonPressed( Ctrl.m_VirtualIndex + 1, false );
        SetButtonPressed( Ctrl.m_VirtualIndex + 2, false );
        SetButtonPressed( Ctrl.m_VirtualIndex + 3, false );
      }
    }
  }
}



bool CDXInput::SaveBindings( IIOStream& Stream )
{
  if ( !Stream.IsGood() )
  {
    return false;
  }

  Stream.WriteSize( (size_t)0xcdee );
  Stream.WriteSize( m_Binding2VKey.size() );

  tMapBinding2VKey::iterator    it( m_Binding2VKey.begin() );
  while ( it != m_Binding2VKey.end() )
  {
    GR::u32   dwVKey( it->second.first );

    if ( ( dwVKey >= 0 )
    &&   ( dwVKey < m_VirtualKeys )
    &&   ( m_pVirtualKey[dwVKey].m_GlobalIndex < m_Controls.size() )
    &&   ( m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device < (int)m_Devices.size() ) )
    {
      Stream.WriteU32( it->first );

      GR::String     deviceName = m_Devices[m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device].m_Device;
      Stream.WriteString( deviceName );
      Stream.WriteString( m_pVirtualKey[dwVKey].m_Name );

      m_pDebugger->Log( "Input.Full", "Saving Binding for Key %d, Device %s, Control Name (%s)", it->first, deviceName.c_str(), m_pVirtualKey[dwVKey].m_Name.c_str() );
    }

    dwVKey = it->second.second;

    if ( ( dwVKey >= 0 )
    &&   ( dwVKey < m_VirtualKeys )
    &&   ( m_pVirtualKey[dwVKey].m_GlobalIndex < m_Controls.size() )
    &&   ( m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device < (int)m_Devices.size() ) )
    {
      Stream.WriteU32( it->first );

      GR::String     deviceName = m_Devices[m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device].m_Device;
      Stream.WriteString( deviceName );
      Stream.WriteString( m_pVirtualKey[dwVKey].m_Name );

      m_pDebugger->Log( "Input.Full", "Saving Binding for secondary key %d, Device %s, Control Name (%s)", it->first, deviceName.c_str(), m_pVirtualKey[dwVKey].m_Name.c_str() );
    }

    ++it;
  }

  // old version
  /*
  Stream.WriteSize( m_Binding2VKey.size() );

  tMapBinding2VKey::iterator    it( m_Binding2VKey.begin() );
  while ( it != m_Binding2VKey.end() )
  {
    GR::u32   dwVKey( it->second );

    if ( ( dwVKey >= 0 )
    &&   ( dwVKey < m_VirtualKeys )
    &&   ( m_pVirtualKey[dwVKey].m_GlobalIndex < m_Controls.size() )
    &&   ( m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device < (int)m_Devices.size() ) )
    {
      Stream.WriteU32( it->first );

      GR::String     deviceName = m_Devices[m_Controls[m_pVirtualKey[dwVKey].m_GlobalIndex].m_Device].m_Device;
      Stream.WriteU32( (GR::u32)deviceName.length() );
      Stream.WriteBlock( deviceName.c_str(), deviceName.length() );
      Stream.WriteU32( m_pVirtualKey[dwVKey].m_DeviceControlIndex );

      m_pDebugger->Log( "Input.Full", "Saving Binding for Key %d, Device %s, Device Control Index %d", it->first, deviceName.c_str(), m_pVirtualKey[dwVKey].m_DeviceControlIndex );
    }

    ++it;
  }*/
  return true;
}



bool CDXInput::LoadBindings( IIOStream& Stream )
{
  ClearAllBindings();

  if ( !Stream.IsGood() )
  {
    return false;
  }

  bool      restoreBindingFailed = false;

  size_t    entryCount = Stream.ReadSize();
  if ( entryCount == 0xcdef )
  {
    // new version
    entryCount = Stream.ReadSize();

    for ( size_t i = 0; i < entryCount; ++i )
    {
      GR::u32         bindHandle = Stream.ReadU32();

      GR::String      deviceName;
      GR::String      controlName;

      // ugly, isn't it somehow possible to pass a string from .exe FileStream into the .dll heap?
      GR::u32         deviceNameLength = Stream.ReadU32();
      char* pDeviceName = new char[deviceNameLength];
      Stream.ReadBlock( pDeviceName, deviceNameLength );
      deviceName.append( pDeviceName, deviceNameLength );
      delete[] pDeviceName;

      GR::u32         controlNameLength = Stream.ReadU32();
      char* pControlName = new char[controlNameLength];
      Stream.ReadBlock( pControlName, controlNameLength );
      controlName.append( pControlName, controlNameLength );
      delete[] pControlName;

      bool            bindingPossible = false;

      m_pDebugger->Log( "Input.Full", "Loading Binding for Key %d, Device %s, Controld %s", bindHandle, deviceName.c_str(), controlName.c_str() );

      tVectDevices::iterator    it( m_Devices.begin() );
      while ( it != m_Devices.end() )
      {
        tInputDevice&  Device( *it );

        if ( Device.m_Device == deviceName )
        {
          auto itC( Device.m_Controls.begin() );
          while ( itC != Device.m_Controls.end() )
          {
            const auto& control( *itC );

            if ( control.m_Name == controlName )
            {
              m_pDebugger->Log( "Input.Full", "Re-bound key to %s (%d)", control.m_Name.c_str(), control.m_VirtualIndex );
              bindingPossible = true;
              m_Binding2VKey[bindHandle].first = control.m_VirtualIndex;
              break;
            }
            ++itC;
          }
        }
        if ( bindingPossible )
        {
          break;
        }

        ++it;
      }
      if ( !bindingPossible )
      {
        m_pDebugger->Log( "Input.Full", "Re-bind key failed" );
        restoreBindingFailed = true;
      }
    }
  }
  else if ( entryCount == 0xcdee )
  {
    // new version
    entryCount = Stream.ReadSize();

    for ( size_t i = 0; i < entryCount; ++i )
    {
      // primary binding
      GR::u32         bindHandle = Stream.ReadU32();

      GR::String      deviceName;
      GR::String      controlName;

      // ugly, isn't it somehow possible to pass a string from .exe FileStream into the .dll heap?
      GR::u32         deviceNameLength = Stream.ReadU32();
      char* pDeviceName = new char[deviceNameLength];
      Stream.ReadBlock( pDeviceName, deviceNameLength );
      deviceName.append( pDeviceName, deviceNameLength );
      delete[] pDeviceName;

      GR::u32         controlNameLength = Stream.ReadU32();
      char* pControlName = new char[controlNameLength];
      Stream.ReadBlock( pControlName, controlNameLength );
      controlName.append( pControlName, controlNameLength );
      delete[] pControlName;

      bool            bindingPossible = false;

      m_pDebugger->Log( "Input.Full", "Loading Binding for Key %d, Device %s, Controld %s", bindHandle, deviceName.c_str(), controlName.c_str() );

      for ( int i = 0; i < (int)m_VirtualKeys; ++i )
      {
        auto& virtualKey = m_pVirtualKey[i];

        if ( ( virtualKey.m_Name == controlName )
        &&   ( m_Devices[m_Controls[virtualKey.m_GlobalIndex].m_Device].m_Device == deviceName ) )
        {
          m_pDebugger->Log( "Input.Full", "Re-bound key to %s (%d)", controlName.c_str(), i );
          bindingPossible = true;
          m_Binding2VKey[bindHandle].first = i;
          break;
        }
      }
      if ( !bindingPossible )
      {
        m_pDebugger->Log( "Input.Full", "Re-bind key failed" );
        restoreBindingFailed = true;
      }

      // secondary binding
      bindHandle = Stream.ReadU32();

      deviceName.clear();
      controlName.clear();

      // ugly, isn't it somehow possible to pass a string from .exe FileStream into the .dll heap?
      deviceNameLength = Stream.ReadU32();
      pDeviceName = new char[deviceNameLength];
      Stream.ReadBlock( pDeviceName, deviceNameLength );
      deviceName.append( pDeviceName, deviceNameLength );
      delete[] pDeviceName;

      controlNameLength = Stream.ReadU32();
      pControlName = new char[controlNameLength];
      Stream.ReadBlock( pControlName, controlNameLength );
      controlName.append( pControlName, controlNameLength );
      delete[] pControlName;

      bindingPossible = false;

      m_pDebugger->Log( "Input.Full", "Loading Binding for secondary Key %d, Device %s, Controld %s", bindHandle, deviceName.c_str(), controlName.c_str() );

     for ( int i = 0; i < (int)m_VirtualKeys; ++i )
      {
        auto& virtualKey = m_pVirtualKey[i];

        if ( ( virtualKey.m_Name == controlName )
        &&   ( m_Devices[m_Controls[virtualKey.m_GlobalIndex].m_Device].m_Device == deviceName ) )
        {
          m_pDebugger->Log( "Input.Full", "Re-bound secondary key to %s (%d)", controlName.c_str(), i );
          bindingPossible = true;
          m_Binding2VKey[bindHandle].second = i;
          break;
        }
      }
      if ( !bindingPossible )
      {
        m_pDebugger->Log( "Input.Full", "Re-bind secondary key failed" );
        restoreBindingFailed = true;
      }
    }
  }
  else
  {
    for ( size_t i = 0; i < entryCount; ++i )
    {
      GR::u32         bindHandle = Stream.ReadU32();
      GR::u32         deviceNameLength = Stream.ReadU32();

      char*           pDeviceName = new char[deviceNameLength];

      Stream.ReadBlock( pDeviceName, deviceNameLength );

      GR::String     deviceName;
      deviceName.append( pDeviceName, deviceNameLength );

      delete[] pDeviceName;

      GR::u32         controlIndex = Stream.ReadU32();
      bool            bindingPossible = false;

      m_pDebugger->Log( "Input.Full", "Loading Binding for Key %d, Device %s, Device Control Index %d", bindHandle, deviceName.c_str(), controlIndex );

      tVectDevices::iterator    it( m_Devices.begin() );
      while ( it != m_Devices.end() )
      {
        tInputDevice&  Device( *it );

        if ( ( Device.m_Device == deviceName )
        &&   ( controlIndex < Device.m_Controls.size() ) )
        {
          m_pDebugger->Log( "Input.Full", "Re-bound key to %s (%d)", Device.m_Controls[controlIndex].m_Name.c_str(), Device.m_Controls[controlIndex].m_VirtualIndex );

          bindingPossible = true;

          m_Binding2VKey[bindHandle].first = Device.m_Controls[controlIndex].m_VirtualIndex;
          break;
        }

        ++it;
      }
      if ( !bindingPossible )
      {
        m_pDebugger->Log( "Input.Full", "Re-bind key failed" );
        restoreBindingFailed = true;
      }
    }
  }
  return restoreBindingFailed;
}



void CDXInput::CreateVirtualKeys()
{
  // plus Fake-Mauswheel
  m_VirtualKeys += 2;

  tInputCtrl    CtrlMWUp;

  CtrlMWUp.m_DeviceType   = Xtreme::VKT_DEFAULT_MOUSE;
  CtrlMWUp.m_APIDataOffset = 0;
  CtrlMWUp.m_Name      = "Mousewheel up";
  CtrlMWUp.m_Type      = Xtreme::CT_UNKNOWN;

  m_Controls.push_back( CtrlMWUp );

  tInputCtrl    CtrlMWDown;

  CtrlMWDown.m_DeviceType   = Xtreme::VKT_DEFAULT_MOUSE;
  CtrlMWDown.m_APIDataOffset = 1;
  CtrlMWDown.m_Name      = "Mousewheel down";
  CtrlMWDown.m_Type      = Xtreme::CT_UNKNOWN;

  m_Controls.push_back( CtrlMWDown );

  m_pVirtualKey = new tVirtualKey[m_VirtualKeys];

  m_pDebugger->Log( "Input.Full", "Allocating %d Virtual Keys", m_VirtualKeys );

  int   curVKey = 1;
  
  int   curMButton = 0;

  int   curJ1Button = 0,
        curJ2Button = 0;


  // der 0. ist ungültig
  m_pVirtualKey[0].m_Name = "No Key";
  m_pVirtualKey[0].m_GlobalIndex = 0;
  m_pVirtualKey[0].m_EnumIndex = Xtreme::KEY_INVALID;

  for ( size_t i = 0; i < m_Controls.size(); ++i )
  {
    tInputCtrl&   Ctrl = m_Controls[i];

    if ( Ctrl.m_Device != -1 )
    {
      m_pDebugger->Log( "Input.Full", "Virtual Key %d/%s for device (%d/%s), device control index %d, API Data offset %d, VK %d", 
                        curVKey, Ctrl.m_Name.c_str(), Ctrl.m_Device, m_Devices[Ctrl.m_Device].m_Device.c_str(), 
                        Ctrl.m_DeviceControlIndex, Ctrl.m_APIDataOffset, Ctrl.m_VirtualKeyCode );
    }
    else
    {
      m_pDebugger->Log( "Input.Full", "Virtual Key %d/%s for non device, VK %d", curVKey, Ctrl.m_Name.c_str(), Ctrl.m_VirtualKeyCode );
    }
    m_pDebugger->Log( "Input.Full", "Create Virtual Key Type %d, Device Type %d", Ctrl.m_Type, Ctrl.m_DeviceType );
    if ( curVKey >= (int)m_VirtualKeys )
    {
      m_pDebugger->Log( "Input.General", "Error, Virtual Key out of bounds" );
      continue;
    }


    if ( Ctrl.m_VirtualIndex != curVKey )
    {
      m_pDebugger->Log( "Input.Full", "VKey was mismatching! (VKey set was %d, real value was %d)", Ctrl.m_VirtualIndex, curVKey );
    }

    Ctrl.m_VirtualIndex         = curVKey;

    if ( ( Ctrl.m_Device != -1 )
    &&   ( Ctrl.m_DeviceControlIndex < m_Devices[Ctrl.m_Device].m_Controls.size() ) )
    {
      tInputCtrl& CtrlInDevice = m_Devices[Ctrl.m_Device].m_Controls[Ctrl.m_DeviceControlIndex];
      CtrlInDevice.m_VirtualIndex = curVKey;
    }

    if ( Ctrl.m_APIType & DIDFT_BUTTON )
    {
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name;

      if ( Ctrl.m_DeviceType & Xtreme::VKT_KEYBOARD )
      {
        // man staune, das DataOffset ist der tatsächliche Tastatur-Index!!
        if ( Ctrl.m_APIDataOffset )
        {
          if ( Ctrl.m_VirtualKeyCode != 0 )
          {
            //dh::Log( "XInputVK %d VK %d (map to %d) = %s", Ctrl.m_VirtualIndex, Ctrl.m_VirtualKeyCode, m_ucVirtualCode[Ctrl.m_VirtualKeyCode], Ctrl.m_Name.c_str() );

            m_VirtualKeyToXtremeKey.insert( std::make_pair( (Xtreme::eInputDefaultButtons)m_VirtualCode[Ctrl.m_VirtualKeyCode], Ctrl.m_VirtualIndex ) );
          }
          if ( Ctrl.m_APIDataOffset < Xtreme::LAST_KEYBOARD_KEY )
          {
            m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)Ctrl.m_APIDataOffset, Ctrl.m_VirtualIndex ) );
          }

          m_pVirtualKey[curVKey].m_EnumIndex = (Xtreme::eInputDefaultButtons)Ctrl.m_APIDataOffset;
          m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
        }
      }
      else if ( Ctrl.m_DeviceType & Xtreme::VKT_DEFAULT_MOUSE )
      {
        // man staune, das DataOffset ist der tatsächliche Tastatur-Index!!
        if ( curMButton < 3 )
        {
          m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)( Xtreme::MOUSE_BUTTON_1 + curMButton ), Ctrl.m_VirtualIndex ) );
          m_pVirtualKey[curVKey].m_EnumIndex = (Xtreme::eInputDefaultButtons)( Xtreme::MOUSE_BUTTON_1 + curMButton );
          m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
        }

        ++curMButton;
      }
      else if ( Ctrl.m_DeviceType & Xtreme::VKT_JOYSTICK_1 )
      {
        if ( curJ1Button < 16 )
        {
          m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)( Xtreme::JOYSTICK_1_BUTTON_1 + curJ1Button ), Ctrl.m_VirtualIndex ) );
          m_pVirtualKey[curVKey].m_EnumIndex = (Xtreme::eInputDefaultButtons)( Xtreme::JOYSTICK_1_BUTTON_1 + curJ1Button );
          m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
        }
        ++curJ1Button;
      }
      else if ( Ctrl.m_DeviceType & Xtreme::VKT_JOYSTICK_2 )
      {
        if ( curJ2Button < 16 )
        {
          m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)( Xtreme::JOYSTICK_2_BUTTON_1 + curJ2Button ), Ctrl.m_VirtualIndex ) );
          m_pVirtualKey[curVKey].m_EnumIndex = (Xtreme::eInputDefaultButtons)( Xtreme::JOYSTICK_2_BUTTON_1 + curJ2Button );
          m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
        }
        ++curJ2Button;
      }


      ++curVKey;
    }
    else if ( Ctrl.m_APIType & DIDFT_ABSAXIS )
    {
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " down";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      if ( Ctrl.m_DeviceType & Xtreme::VKT_JOYSTICK_1 )
      {
        if ( Ctrl.m_DeviceType & Xtreme::VKT_X_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_1_LEFT, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::JOYSTICK_1_LEFT;
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_1_RIGHT, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::JOYSTICK_1_RIGHT;
        }
        if ( Ctrl.m_DeviceType & Xtreme::VKT_Y_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_1_UP, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::JOYSTICK_1_UP;
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_1_DOWN, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::JOYSTICK_1_DOWN;
        }
      }
      if ( Ctrl.m_DeviceType & Xtreme::VKT_JOYSTICK_2 )
      {
        if ( Ctrl.m_DeviceType & Xtreme::VKT_X_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_2_LEFT, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::JOYSTICK_2_LEFT;
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_2_RIGHT, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::JOYSTICK_2_RIGHT;
        }
        if ( Ctrl.m_DeviceType & Xtreme::VKT_Y_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_2_UP, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::JOYSTICK_2_UP;
          m_DefaultKeys.insert( std::make_pair( Xtreme::JOYSTICK_2_DOWN, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::JOYSTICK_2_DOWN;
        }
      }
    }
    else if ( Ctrl.m_APIType & DIDFT_RELAXIS )
    {
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " down";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      if ( Ctrl.m_DeviceType & Xtreme::VKT_DEFAULT_MOUSE )
      {
        if ( Ctrl.m_DeviceType & Xtreme::VKT_X_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_LEFT, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::MOUSE_LEFT;
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_RIGHT, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_RIGHT;
        }
        if ( Ctrl.m_DeviceType & Xtreme::VKT_Y_AXIS )
        {
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_UP, curVKey - 2 ) );
          m_pVirtualKey[curVKey - 2].m_EnumIndex = Xtreme::MOUSE_UP;
          m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_DOWN, curVKey - 1 ) );
          m_pVirtualKey[curVKey - 1].m_EnumIndex = Xtreme::MOUSE_DOWN;
        }
      }
    }
    else if ( Ctrl.m_APIType & DIDFT_POV )
    {
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " left";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " right";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " up";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;

      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name + " down";
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      ++curVKey;
    }
    else if ( Ctrl.m_APIType == 0 )
    {
      // manuell gefakte Keys (Mousewheel)
      // Mousewheel
      m_pVirtualKey[curVKey].m_GlobalIndex = (GR::u32)i;
      m_pVirtualKey[curVKey].m_Name = Ctrl.m_Name;
      m_pVirtualKey[curVKey].m_DeviceControlIndex = Ctrl.m_DeviceControlIndex;
      if ( Ctrl.m_APIDataOffset == 0 )
      {
        m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_WHEEL_UP, curVKey ) );
        m_pVirtualKey[curVKey].m_EnumIndex = Xtreme::MOUSE_WHEEL_UP;
      }
      if ( Ctrl.m_APIDataOffset == 1 )
      {
        m_DefaultKeys.insert( std::make_pair( Xtreme::MOUSE_WHEEL_DOWN, curVKey ) );
        m_pVirtualKey[curVKey].m_EnumIndex = Xtreme::MOUSE_WHEEL_DOWN;
      }

      ++curVKey;
    }
  }
}



BOOL CDXInput::WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam )
{
  LRESULT   lRes = XBasicInput::WindowProc( hWnd, wMessage, wParam, lParam );

  if ( wMessage == WM_ACTIVATE )
  {
    tVectDevices::iterator    itDev( m_Devices.begin() );
    while ( itDev != m_Devices.end() )
    {
      tInputDevice&   Device = *itDev;

      if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice )
      {
        if ( m_InputActive )
        {
          ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Acquire();
          if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect )
          {
            ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect->Start( 1, 0 );
          }
        }
        else
        {
          if ( ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect )
          {
            ( (DeviceInfo*)Device.m_pDevicePointer )->pEffect->Stop();
          }
          ( (DeviceInfo*)Device.m_pDevicePointer )->pDevice->Unacquire();
        }
      }

      ++itDev;
    }
  }
  return (BOOL)lRes;
}



bool CDXInput::HasDeviceForceFeedback( GR::u32 Device ) const
{
  if ( Device >= (GR::u32)m_Devices.size() )
  {
    m_pDebugger->Log( "Input.General", "Device index out of bounds" );
    return false;
  }
  DeviceInfo*   pInfo = (DeviceInfo*)m_Devices[Device].m_pDevicePointer;

  return ( pInfo->pEffect != NULL );
}



bool CDXInput::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  if ( Device >= (GR::u32)m_Devices.size() )
  {
    m_pDebugger->Log( "Input.General", "Device index out of bounds" );
    return false;
  }
  DeviceInfo*   pInfo = (DeviceInfo*)m_Devices[Device].m_pDevicePointer;

  if ( pInfo->pEffect == NULL )
  {
    m_pDebugger->Log( "Input.General", "SetDeviceForce: Accessing Device with no effect available" );
    return false;
  }
  if ( Axis >= pInfo->AxisCount )
  {
    m_pDebugger->Log( "Input.General", "SetDeviceForce: Axis index out of bounds" );
    return false;
  }

  if ( Value < -100 )
  {
    Value = -100;
  }
  if ( Value > 100 )
  {
    Value = 100;
  }

  DICONSTANTFORCE     cf;

  Value *= DI_FFNOMINALMAX / 100;

  pInfo->AxisValue[Axis] = Value;

  if ( pInfo->AxisCount == 1 )
  {
    cf.lMagnitude = pInfo->AxisValue[0];
  }
  else if ( pInfo->AxisCount == 2 )
  {
    cf.lMagnitude = (DWORD)sqrt( (double)pInfo->AxisValue[0] * (double)pInfo->AxisValue[0] 
                                 + (double)pInfo->AxisValue[1] * (double)pInfo->AxisValue[1] );
  }

  DIEFFECT eff;
  ZeroMemory( &eff, sizeof(eff) );
  eff.dwSize                = sizeof( DIEFFECT );
  eff.dwFlags               = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
  eff.cAxes                 = pInfo->AxisCount;
  eff.rglDirection          = pInfo->AxisValue;
  eff.lpEnvelope            = 0;
  eff.cbTypeSpecificParams  = sizeof( DICONSTANTFORCE );
  eff.lpvTypeSpecificParams = &cf;
  eff.dwStartDelay            = 0;

  // Now set the new parameters and start the effect immediately.
  if ( FAILED( pInfo->pEffect->SetParameters( &eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START ) ) )
  {
    return false;
  }
  pInfo->pEffect->Start( 1, 0 );
  return true;
}



GR::u32 CDXInput::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  if ( Device >= (GR::u32)m_Devices.size() )
  {
    m_pDebugger->Log( "Input.General", "Device index out of bounds" );
    return 0;
  }
  DeviceInfo*   pInfo = (DeviceInfo*)m_Devices[Device].m_pDevicePointer;

  if ( pInfo->pEffect == NULL )
  {
    m_pDebugger->Log( "Input.General", "Accessing device with no effect available" );
    return 0;
  }
  return pInfo->AxisCount;
}



