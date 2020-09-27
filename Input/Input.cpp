#include <Input/Input.h>

#include <Interface/IIOStream.h>

#include <debug/debugclient.h>

#include <Misc/Misc.h>

#ifndef __NO_DIRECT_X__
#include <dinput.h>

#pragma comment ( lib, "dinput8.lib" )
#pragma comment ( lib, "dxguid.lib" )

#endif // __NO_DIRECT_X__

#ifdef __NO_DIRECT_X__
#pragma message( "Compiling Input without DirectX.\n" )
#else
#pragma message( "Compiling Input with DirectX.\n" )
#endif


CInputSystem*       g_pGlobalInput = NULL;

char                  szKeyName[240][25] =
{ "na",
  "Escape",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "0",
  "Minus",
  "=",
  "Back-Space",
  "Tab",
  "Q",
  "W",
  "E",
  "R",
  "T",
  "Y",
  "U",
  "I",
  "O",
  "P",
  "Ü",
  "Plus",
  "Enter",
  "LControl",
  "A",
  "S",
  "D",
  "F",
  "G",
  "H",
  "J",
  "K",
  "L",
  "Ö",
  "Ä",
  "^",
  "LShift",
  "Backslash",
  "Z",
  "X",
  "C",
  "V",
  "B",
  "N",
  "M",
  "Komma",
  "Punkt",
  "Slash",
  "RShift",
  "Numpad Multiply",
  "LAlt",
  "Space",
  "Caps-Lock",
  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
  "F7",
  "F8",
  "F9",
  "F10",
  "Num-Lock",
  "Scroll-Lock",
  "Numpad 7",
  "Numpad 8",
  "Numpad 9",
  "Numpad Minus",
  "Numpad 4",
  "Numpad 5",
  "Numpad 6",
  "Numpad Plus",
  "Numpad 1",
  "Numpad 2",
  "Numpad 3",
  "Numpad 0",
  "Numpad Komma",
  "na",
  "na",
  "Greater",
  "F11",
  "F12",      // 88
  "na",
  "na",       // 90
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 100
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 110
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 120
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 130
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 140
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 150
  "na",
  "na",
  "na",
  "na",
  "na",
  "Numpad Enter", // 156
  "RControl",     // 157
  "nax",
  "na",
  "naz",           // 160
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 170
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",       // 180
  "Numpad Divide",  // 181
  "na",
  "na",
  "RAlt",           // 184
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",             // 190
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "na",
  "Home",           // 199
  "Up",
  "PageUp",
  "na",
  "Left",
  "na",
  "Right",
  "na",
  "End",
  "Down",
  "PageDown",
  "Insert",
  "Delete",
  "Mouse Left",
  "Mouse Right",
  "Mouse Up",
  "Mouse Down",
  "Mouse Left Button",
  "Mouse Right Button",
  "Mouse Middle Button" };


static MMRESULT ( WINAPI *MyJoyGetPos )( UINT uJoyID,    LPJOYINFO pji );

static UINT ( WINAPI *MyJoyGetNumDevs )( VOID ); 



CInputSystem::CInputSystem()
{
  m_DeleteThisTask       = false;

  m_bInitialized          = FALSE;
  g_pGlobalInput          = this;

  m_bInputDX              = FALSE;
  m_bInputSubClassed      = FALSE;
  m_bInputActive          = TRUE;

  m_bInputMButtonsSwapped = FALSE;

  m_iInputJoyX            = 32768;
  m_iInputJoyY            = 32768;
                                  
  m_lpfnInputOldWndProc   = NULL;
  m_hwndInput             = NULL;
  m_hwndRelativeMouseTo   = NULL;

  m_iVirtualKeys          = 1;

  m_pVirtualKey           = NULL;

  memset( m_ucVirtualCode, KEY_INVALID, sizeof( m_ucVirtualCode ) );
  m_ucVirtualCode[  8] = KEY_BACKSPACE;
  m_ucVirtualCode[  9] = KEY_TAB;
  m_ucVirtualCode[ 13] = KEY_ENTER;
  m_ucVirtualCode[ 16] = KEY_LSHIFT;
  m_ucVirtualCode[ 17] = KEY_LCONTROL;
  m_ucVirtualCode[ 18] = KEY_LALT;
  m_ucVirtualCode[ 27] = KEY_ESCAPE;
  m_ucVirtualCode[ 32] = KEY_SPACE;
  m_ucVirtualCode[ 33] = KEY_PAGEUP;
  m_ucVirtualCode[ 34] = KEY_PAGEDOWN;
  m_ucVirtualCode[ 35] = KEY_END;
  m_ucVirtualCode[ 36] = KEY_HOME;
  m_ucVirtualCode[ 37] = KEY_LEFT;
  m_ucVirtualCode[ 38] = KEY_UP;
  m_ucVirtualCode[ 39] = KEY_RIGHT;
  m_ucVirtualCode[ 40] = KEY_DOWN;
  m_ucVirtualCode[ 45] = KEY_INSERT;
  m_ucVirtualCode[ 46] = KEY_DELETE;
  m_ucVirtualCode[ 48] = KEY_0;
  m_ucVirtualCode[ 49] = KEY_1;
  m_ucVirtualCode[ 50] = KEY_2;
  m_ucVirtualCode[ 51] = KEY_3;
  m_ucVirtualCode[ 52] = KEY_4;
  m_ucVirtualCode[ 53] = KEY_5;
  m_ucVirtualCode[ 54] = KEY_6;
  m_ucVirtualCode[ 55] = KEY_7;
  m_ucVirtualCode[ 56] = KEY_8;
  m_ucVirtualCode[ 57] = KEY_9;
  m_ucVirtualCode[ 65] = KEY_A;
  m_ucVirtualCode[ 66] = KEY_B;
  m_ucVirtualCode[ 67] = KEY_C;
  m_ucVirtualCode[ 68] = KEY_D;
  m_ucVirtualCode[ 69] = KEY_E;
  m_ucVirtualCode[ 70] = KEY_F;
  m_ucVirtualCode[ 71] = KEY_G;
  m_ucVirtualCode[ 72] = KEY_H;
  m_ucVirtualCode[ 73] = KEY_I;
  m_ucVirtualCode[ 74] = KEY_J;
  m_ucVirtualCode[ 75] = KEY_K;
  m_ucVirtualCode[ 76] = KEY_L;
  m_ucVirtualCode[ 77] = KEY_M;
  m_ucVirtualCode[ 78] = KEY_N;
  m_ucVirtualCode[ 79] = KEY_O;
  m_ucVirtualCode[ 80] = KEY_P;
  m_ucVirtualCode[ 81] = KEY_Q;
  m_ucVirtualCode[ 82] = KEY_R;
  m_ucVirtualCode[ 83] = KEY_S;
  m_ucVirtualCode[ 84] = KEY_T;
  m_ucVirtualCode[ 85] = KEY_U;
  m_ucVirtualCode[ 86] = KEY_V;
  m_ucVirtualCode[ 87] = KEY_W;
  m_ucVirtualCode[ 88] = KEY_X;
  m_ucVirtualCode[ 89] = KEY_Z;
  m_ucVirtualCode[ 90] = KEY_Y;
  m_ucVirtualCode[ 96] = KEY_NUMPAD_0;
  m_ucVirtualCode[ 97] = KEY_NUMPAD_1;
  m_ucVirtualCode[ 98] = KEY_NUMPAD_2;
  m_ucVirtualCode[ 99] = KEY_NUMPAD_3;
  m_ucVirtualCode[100] = KEY_NUMPAD_4;
  m_ucVirtualCode[101] = KEY_NUMPAD_5;
  m_ucVirtualCode[102] = KEY_NUMPAD_6;
  m_ucVirtualCode[103] = KEY_NUMPAD_7;
  m_ucVirtualCode[104] = KEY_NUMPAD_8;
  m_ucVirtualCode[105] = KEY_NUMPAD_9;
  m_ucVirtualCode[106] = KEY_NUMPAD_MULTIPLY;
  m_ucVirtualCode[107] = KEY_NUMPAD_PLUS;
  m_ucVirtualCode[109] = KEY_NUMPAD_MINUS;
  m_ucVirtualCode[110] = KEY_PUNKT;
  m_ucVirtualCode[111] = KEY_NUMPAD_DIVIDE;
  m_ucVirtualCode[112] = KEY_F1;
  m_ucVirtualCode[113] = KEY_F2;
  m_ucVirtualCode[114] = KEY_F3;
  m_ucVirtualCode[115] = KEY_F4;
  m_ucVirtualCode[116] = KEY_F5;
  m_ucVirtualCode[117] = KEY_F6;
  m_ucVirtualCode[118] = KEY_F7;
  m_ucVirtualCode[119] = KEY_F8;
  m_ucVirtualCode[120] = KEY_F9;
  m_ucVirtualCode[121] = KEY_F10;
  m_ucVirtualCode[122] = KEY_F11;
  m_ucVirtualCode[123] = KEY_F12;
  m_ucVirtualCode[144] = KEY_NUMLOCK;
  m_ucVirtualCode[145] = KEY_SCROLLLOCK;
  m_ucVirtualCode[186] = KEY_UE;
  m_ucVirtualCode[187] = KEY_PLUS;
  m_ucVirtualCode[188] = KEY_KOMMA;
  m_ucVirtualCode[189] = KEY_SLASH;
  m_ucVirtualCode[190] = KEY_PUNKT;
  m_ucVirtualCode[191] = KEY_BACKSLASH;
  m_ucVirtualCode[192] = KEY_OE;
  m_ucVirtualCode[219] = KEY_MINUS;
  m_ucVirtualCode[220] = KEY_GRAVE;
  m_ucVirtualCode[221] = KEY_EQUALS;
  m_ucVirtualCode[222] = KEY_AE;
  m_ucVirtualCode[226] = KEY_GREATER;
}



CInputSystem::~CInputSystem()
{

  Release();

}



CInputSystem& CInputSystem::Instance()
{

  static CInputSystem      g_Instance;

  return g_Instance;

}



#ifndef __NO_DIRECT_X__
BOOL CALLBACK CInputSystem::EnumObjects( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef )
{

  CInputSystem*   pInput = (CInputSystem*)pvRef;

  if ( !( lpddoi->dwType & DIDFT_NODATA ) )
  {
    // da kommt was raus
    tInputDevice&   Device = pInput->m_vectDevices[pInput->m_iCurrentEnumDevice];

    tInputCtrl      NewCtrl( pInput->m_iCurrentEnumDevice, lpddoi->tszName, lpddoi->dwType, lpddoi->dwOfs );

    NewCtrl.m_iGlobalIndex = (int)pInput->m_vectControls.size();
    NewCtrl.m_iVirtualIndex = pInput->m_iVirtualKeys;
    NewCtrl.m_iDeviceControlIndex = (GR::u32)Device.m_vectControls.size();

    // Zuweisung von Default-Enums
    GR::u32     devType = VKT_UNKNOWN;

    if ( ( pInput->m_dwCurrentEnumDeviceType & 0xff ) == DI8DEVTYPE_KEYBOARD )
    {
      devType = VKT_KEYBOARD;
    }
    else if ( ( pInput->m_dwCurrentEnumDeviceType & 0xff ) == DI8DEVTYPE_MOUSE )
    {
      devType = VKT_DEFAULT_MOUSE;
    }
    else if ( pInput->m_iCurrentEnumJoystickDevice == 0 )
    {
      devType = VKT_JOYSTICK_1;
    }
    else if ( pInput->m_iCurrentEnumJoystickDevice == 1 )
    {
      devType = VKT_JOYSTICK_2;
    }

    if ( lpddoi->dwType & DIDFT_ABSAXIS )
    {
      DIPROPRANGE     diRange;

      diRange.diph.dwSize = sizeof( diRange );
      diRange.diph.dwHeaderSize = sizeof( DIPROPHEADER );
      diRange.diph.dwHow = DIPH_BYID;
      diRange.diph.dwObj = lpddoi->dwType;

      if ( lpddoi->guidType == GUID_XAxis )
      {
        devType |= VKT_X_AXIS;
      }
      if ( lpddoi->guidType == GUID_YAxis )
      {
        devType |= VKT_Y_AXIS;
      }

      HRESULT hRes = Device.m_pDevice->GetProperty( DIPROP_RANGE, (LPDIPROPHEADER)&diRange );
      if ( SUCCEEDED( hRes ) )
      {
        NewCtrl.m_iMin = diRange.lMin;
        NewCtrl.m_iMax = diRange.lMax;
        //dh::Log( "Range %d bis %d", NewCtrl.m_iMin, NewCtrl.m_iMax );
      }
      else
      {
        dh::Log( "GetProperty failed %x", hRes );
      }
      // 2 Keys pro Achse
      pInput->m_iVirtualKeys += 2;
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
        devType |= VKT_X_AXIS;
      }
      if ( lpddoi->guidType == GUID_YAxis )
      {
        devType |= VKT_Y_AXIS;
      }

      HRESULT hRes = Device.m_pDevice->GetProperty( DIPROP_RANGE, (LPDIPROPHEADER)&diRange );
      if ( SUCCEEDED( hRes ) )
      {
        NewCtrl.m_iMin = diRange.lMin;
        NewCtrl.m_iMax = diRange.lMax;
        //dh::Log( "Range %d bis %d", NewCtrl.m_iMin, NewCtrl.m_iMax );
      }
      else
      {
        dh::Log( "GetProperty failed %x", hRes );
      }
      // 2 Keys pro Achse
      pInput->m_iVirtualKeys += 2;
    }
    else if ( lpddoi->dwType & DIDFT_PSHBUTTON )
    {
      ++pInput->m_iVirtualKeys;
    }
    else if ( lpddoi->dwType & DIDFT_POV )
    {
      // 4 Keys pro POV (NESW)
      pInput->m_iVirtualKeys += 4;
    }

    NewCtrl.m_DeviceType = devType;

    pInput->m_vectControls.push_back( NewCtrl );
    Device.m_vectControls.push_back( NewCtrl );
  }
   
  return TRUE;

}

#endif // __NO_DIRECT_X__



BOOL CInputSystem::Create( HINSTANCE hInst, HWND hWnd, BOOL bAllowDirectInput, bool bExclusive )
{

  #ifndef __NO_DIRECT_X__
  HRESULT               hResult;
  #endif // __NO_DIRECT_X__

  JOYINFO               stJoyInfo;


  if ( m_bInitialized )
  {
    return TRUE;
  }

  m_bExclusive          = bExclusive;

  m_vectControls.clear();
  m_vectDevices.clear();
  m_mapBinding2VKey.clear();

  m_bInputMButtonsSwapped = FALSE;
  m_bInputMButtonsSwapped = SwapMouseButton( m_bInputMButtonsSwapped );
  SwapMouseButton( m_bInputMButtonsSwapped );

  m_hwndInput             = hWnd;
  m_hwndRelativeMouseTo   = hWnd;

  m_iVirtualKeys          = 1;

  m_wInputJoystickID      = 255;

  SystemParametersInfo( SPI_GETKEYBOARDDELAY, 0, &m_dwKeyboardDelayTicks, 0 );
  SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &m_dwKeyboardRepeatDelayTicks, 0 );

  m_dwKeyboardDelayTicks = ( m_dwKeyboardDelayTicks + 1 ) * 250;
  m_dwKeyboardRepeatDelayTicks = ( 400 - 33 ) * m_dwKeyboardRepeatDelayTicks / 400;

  #ifdef __NO_DIRECT_X__
  bAllowDirectInput = FALSE;
  #endif // __NO_DIRECT_X__
  if ( !bAllowDirectInput )
  {
    // kein DirectInput -> Window subclassen und WM_KEY-Messages benutzen
    // und Joysticks über WinMM.DLL lesen
    m_bInputDX = FALSE;

    m_hWinMMHandle = LoadLibrary( "winmm.dll" );
    if ( m_hWinMMHandle == NULL )
    {
      // kein WINMM.DLL ???
      return FALSE;
    }
    MyJoyGetPos = ( MMRESULT (__stdcall *)( UINT, LPJOYINFO ) )(FARPROC)GetProcAddress( m_hWinMMHandle, "joyGetPos" );
    if ( MyJoyGetPos == NULL )
    {
      FreeLibrary( m_hWinMMHandle );
      return FALSE;
    }

    MyJoyGetNumDevs = ( MMRESULT (__stdcall *)( VOID ) )(FARPROC)GetProcAddress( m_hWinMMHandle, "joyGetNumDevs" );
    if ( MyJoyGetNumDevs == NULL )
    {
      FreeLibrary( m_hWinMMHandle );
      return FALSE;
    }

    // Pseudo-Devices und Virtual-Keys erzeugen
    m_iVirtualKeys = DEFKEY_LAST_ENTRY;
    m_pVirtualKey = new tVirtualKey[m_iVirtualKeys];

    // das 0. gibt es nicht
    m_pVirtualKey[0].m_strName = "No Key";
    m_pVirtualKey[0].m_iGlobalIndex = 0;
    m_pVirtualKey[0].m_iEnumIndex = KEY_INVALID;

    m_vectControls.reserve( DEFKEY_LAST_ENTRY );
    m_vectControls.push_back( tInputCtrl() );

    m_vectDevices.push_back( tInputDevice( "Keyboard" ) );

    for ( int i = FIRST_KEYBOARD_KEY; i <= LAST_KEYBOARD_KEY; ++i )
    {
      tInputCtrl      NewCtrl( 0, szKeyName[i] );

      NewCtrl.m_iGlobalIndex = (int)m_vectControls.size();
      NewCtrl.m_iVirtualIndex = NewCtrl.m_iGlobalIndex;

      m_vectControls.push_back( NewCtrl );
      m_pVirtualKey[i].m_strName      = szKeyName[i];
      m_pVirtualKey[i].m_iGlobalIndex = NewCtrl.m_iGlobalIndex;
      m_pVirtualKey[i].m_iEnumIndex   = (eInputDefaultButtons)i;
      m_pVirtualKey[i].m_bReleased    = true;
      m_pVirtualKey[i].m_iDeviceControlIndex = (int)m_vectControls.size() - 1;

      m_mapDefaultKeys[(eInputDefaultButtons)i] = NewCtrl.m_iVirtualIndex;
    }

    // Maus
    m_vectDevices.push_back( tInputDevice( "Mouse" ) );

    for ( int i = MOUSE_LEFT; i <= MOUSE_BUTTON_3; ++i )
    {
      tInputCtrl      NewCtrl( 1, szKeyName[i] );

      NewCtrl.m_iGlobalIndex = (int)m_vectControls.size();
      NewCtrl.m_iVirtualIndex = NewCtrl.m_iGlobalIndex;

      m_vectControls.push_back( NewCtrl );
      m_pVirtualKey[i].m_strName      = szKeyName[i];
      m_pVirtualKey[i].m_iGlobalIndex = NewCtrl.m_iGlobalIndex;
      m_pVirtualKey[i].m_iEnumIndex   = (eInputDefaultButtons)i;
      m_pVirtualKey[i].m_bReleased    = true;
      m_pVirtualKey[i].m_iDeviceControlIndex = (int)m_vectControls.size() - 1;

      m_mapDefaultKeys[(eInputDefaultButtons)i] = NewCtrl.m_iVirtualIndex;
    }

    // Joystick
    m_vectDevices.push_back( tInputDevice( "Joystick" ) );

    GR::String     strDummy = "";
    for ( int i = JOYSTICK_1_LEFT; i <= JOYSTICK_2_BUTTON_32; ++i )
    {
      strDummy = "Joystick ";

      if ( i >= JOYSTICK_2_LEFT )
      {
        strDummy += "2 ";
      }
      else
      {
        strDummy += "1 ";
      }
      switch ( ( i - JOYSTICK_1_LEFT ) % 36 )
      {
        case 0:
          strDummy += "left";
          break;
        case 1:
          strDummy += "right";
          break;
        case 2:
          strDummy += "up";
          break;
        case 3:
          strDummy += "down";
          break;
        default:
          strDummy += CMisc::printf( "Button %d", ( i - JOYSTICK_1_LEFT ) % 36 - 4 );
      }
      
      tInputCtrl      NewCtrl( 2, strDummy );

      NewCtrl.m_iGlobalIndex = (int)m_vectControls.size();
      NewCtrl.m_iVirtualIndex = NewCtrl.m_iGlobalIndex;

      m_vectControls.push_back( NewCtrl );
      
      m_pVirtualKey[i].m_iGlobalIndex = NewCtrl.m_iGlobalIndex;
      m_pVirtualKey[i].m_iEnumIndex   = (eInputDefaultButtons)i;
      m_pVirtualKey[i].m_strName      = strDummy;
      m_pVirtualKey[i].m_bReleased    = true;
      m_pVirtualKey[i].m_iDeviceControlIndex = (int)m_vectControls.size() - 1;

      m_mapDefaultKeys[(eInputDefaultButtons)i] = NewCtrl.m_iVirtualIndex;
    }

    int k = MyJoyGetNumDevs();
    if ( k > 0 )
    {
      if ( MyJoyGetPos( JOYSTICKID1, &stJoyInfo ) != JOYERR_UNPLUGGED )
      {
        m_vectDevices.push_back( tInputDevice( "Joystick 1" ) );
      }
      if ( MyJoyGetPos( JOYSTICKID2, &stJoyInfo ) != JOYERR_UNPLUGGED )
      {
        m_vectDevices.push_back( tInputDevice( "Joystick 2" ) );
      }
    }
  }
  else
  {
    m_bInputDX = TRUE;
  }

  //dh::Log( "Input vor SetWindowLong %x", GetCurrentProcessId() );

  //dh::Log( "this %x, Old %x, New %x", this, m_lpfnInputOldWndProc, InputSubClassProc );

  // Window subclassen (zum Messages abfangen)
#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )
  // Häßlich, W64-Warning disablen, läßt sich hier sonst nicht unterdrücken
  m_lpfnInputOldWndProc = (WNDPROC)SetWindowLongPtr( hWnd, GWL_WNDPROC, (LONG_PTR)(WNDPROC)InputSubClassProc );
  //m_lpfnInputOldWndProc = (WNDPROC)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)InputSubClassProc );
#pragma warning( pop )
  m_bInputSubClassed = TRUE;

  //dh::Log( "Input nach SetWindowLong" );

  m_iInputMouseSensitivity    = 5;
  m_iInputMouseTreshold       = 5;
  m_iInputJoyTreshold         = 16000;
  m_iInputMouseRelX           = 0;
  m_iInputMouseRelY           = 0;

  #ifndef __NO_DIRECT_X__
  if ( m_bInputDX )
  {
    // BAUSTELLE
    hResult = DirectInput8Create( hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_lpDirectInput, NULL); 
    if ( hResult != DI_OK )
    {
      MessageBox( hWnd, "Failed to Create DirectInput Instance", "Error", MB_OK | MB_APPLMODAL );
      return FALSE;
    }
    // jetzt die einzelnen Devices erkennen lassen

    // Vector-Speicher reservieren
    m_vectDevices.reserve( 5 );         // Maus, Tastatur, 3 Gamepads sollte reichen

    m_iCurrentEnumDevice = 0;
    m_iCurrentEnumJoystickDevice = 0;

    hResult = m_lpDirectInput->EnumDevices( 0, &EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
    if ( hResult != DI_OK )
    {
      m_lpDirectInput->Release();
      m_lpDirectInput = NULL;
      return FALSE;
    }

    CreateVirtualKeys();
  }
  #endif // __NO_DIRECT_X__

  m_bInitialized = TRUE;

  return TRUE;

}



BOOL CInputSystem::Release()
{

  if ( !m_bInitialized )
  {
    return TRUE;
  }

  delete[] m_pVirtualKey;
  m_pVirtualKey = NULL;


  #ifndef __NO_DIRECT_X__
  if ( m_bInputDX )
  {
    tVectDevices::iterator    itDev( m_vectDevices.begin() );
    while ( itDev != m_vectDevices.end() )
    {
      tInputDevice&   Device = *itDev;

      delete[] Device.m_pDeviceData;
      Device.m_pDeviceData = NULL;
      if ( Device.m_pDevice )
      {
        Device.m_pDevice->Unacquire();
        Device.m_pDevice->Release();
        Device.m_pDevice = NULL;
      }

      ++itDev;
    }
    m_vectDevices.clear();
    m_vectControls.clear();

    if ( m_lpDirectInput != NULL )
    {
      m_lpDirectInput->Release();
      m_lpDirectInput = NULL;
    }
  }
  #endif // __NO_DIRECT_X__
  if ( m_hWinMMHandle != NULL )
  {
    FreeLibrary( m_hWinMMHandle );
    m_hWinMMHandle = NULL;
  }

  if ( m_bInputSubClassed )
  {
    if ( IsWindow( m_hwndInput ) )
    {
      #pragma warning( push )
      #pragma warning( disable : 4244 )
      #pragma warning( disable : 4312 )
      // Häßlich, W64-Warning disablen, läßt sich hier sonst nicht unterdrücken
      if ( (WNDPROC)GetWindowLongPtr( m_hwndInput, GWL_WNDPROC ) == InputSubClassProc )
      {
        SetWindowLongPtr( m_hwndInput, GWL_WNDPROC, (DWORD_PTR)m_lpfnInputOldWndProc );
      }
      /*
      if ( (WNDPROC)GetWindowLong( m_hwndInput, GWL_WNDPROC ) == InputSubClassProc )
      {
        SetWindowLong( m_hwndInput, GWL_WNDPROC, (DWORD)m_lpfnInputOldWndProc );
      }
      */
      #pragma warning( pop )
    }
    m_bInputSubClassed = FALSE;
  }

  m_bInitialized = FALSE;

  return TRUE;

}



#ifndef __NO_DIRECT_X__
BOOL CALLBACK CInputSystem::EnumDevicesProc( const DIDEVICEINSTANCE *lpddi, LPVOID pvRef )
{

  CInputSystem*   pInput = (CInputSystem*)pvRef;

  LPDIRECTINPUTDEVICE8    dummyDevice = NULL;

  HRESULT hResult =  pInput->m_lpDirectInput->CreateDevice( lpddi->guidInstance, &dummyDevice, NULL );
  if ( hResult == DI_OK )
  {
    pInput->m_vectDevices.push_back( tInputDevice( lpddi->tszInstanceName, dummyDevice ) );
    pInput->m_iCurrentEnumDevice = (int)pInput->m_vectDevices.size() - 1;
    pInput->m_dwCurrentEnumDeviceType = lpddi->dwDevType;

    dummyDevice->EnumObjects( EnumObjects, pInput, DIDFT_ALL );

    if ( ( ( lpddi->dwDevType & 0xff ) != DI8DEVTYPE_MOUSE )
    &&   ( ( lpddi->dwDevType & 0xff ) != DI8DEVTYPE_KEYBOARD ) )
    {
      // ein Joystick/Gamepad/wasauchimmer
      ++pInput->m_iCurrentEnumJoystickDevice;
    }

    pInput->SetDataFormat( pInput->m_vectDevices[pInput->m_iCurrentEnumDevice] );
  }

  return DIENUM_CONTINUE;  

}
#endif // __NO_DIRECT_X__



LRESULT CALLBACK CInputSystem::InputSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  //dh::Log( "CInputSystem::InputSubClassProc %x", uMsg );
  return g_pGlobalInput->WindowProc( hWnd, uMsg, wParam, lParam );

}



LRESULT CInputSystem::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  unsigned char             ucDummy;

  static BOOL               bLastKeyDownControl = FALSE,
                            bControlWasOn = FALSE;


  switch ( uMsg )
  {
    case WM_MOUSEWHEEL:
      if ( m_bInputActive )
      {
        int   iMouseZ = (short)HIWORD( wParam );

        iMouseZ /= 120;
        while ( iMouseZ < 0 )
        {
          SendEvent( tInputEvent( tInputEvent::IE_MOUSEWHEEL_DOWN, 0, 0 ) );
          iMouseZ++;
        }
        while ( iMouseZ > 0 )
        {
          SendEvent( tInputEvent( tInputEvent::IE_MOUSEWHEEL_UP, 0, 0 ) );
          iMouseZ--;
        }
      }
      break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONUP:
      {
        // Mauswerte setzen
        SetButtonPressed( MOUSE_BUTTON_1, !!( wParam & MK_LBUTTON ) );
        SetButtonPressed( MOUSE_BUTTON_2, !!( wParam & MK_RBUTTON ) );
        SetButtonPressed( MOUSE_BUTTON_3, !!( wParam & MK_MBUTTON ) );

        // GUI
        POINT           pt;


        GetCursorPos( &pt );
        if ( IsWindow( m_hwndRelativeMouseTo ) )
        {
          ScreenToClient( m_hwndRelativeMouseTo, &pt );
        }

        m_iInputWMMouseX = pt.x;
        m_iInputWMMouseY = pt.y;

        if ( m_bInputActive )
        {
          SendEvent( tInputEvent( tInputEvent::IE_MOUSE_UPDATE, 0, 0 ) );
        }
      }
      break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      if ( m_bInputActive )
      {
        SendEvent( tInputEvent( tInputEvent::IE_WIN_KEY_DOWN, wParam ) );

        if ( ( wParam >= 0 )
        &&   ( wParam <= 239 )
        &&   ( !m_bInputDX ) )
        {
          if ( m_ucVirtualCode[wParam] != KEY_INVALID )
          {
            ucDummy = m_ucVirtualCode[wParam];
            if ( ucDummy == KEY_LSHIFT )
            {
              if ( ( ( lParam & 0xff0000 ) >> 16 ) == 54 )
              {
                ucDummy = KEY_RSHIFT;
              }
            }
            if ( lParam & 0x1000000 )
            {
              // ein enhanced Keycode!
              if ( ucDummy == KEY_ENTER )
              {
                ucDummy = KEY_NUMPAD_ENTER;
              }
              else if ( ucDummy == KEY_LCONTROL )
              {
                ucDummy = KEY_RCONTROL;
              }
              else if ( ucDummy == KEY_LALT )
              {
                ucDummy = KEY_RALT;
                if ( ( bLastKeyDownControl )
                &&   ( !bControlWasOn ) )
                {
                  SetButtonPressed( KEY_LCONTROL, FALSE );
                }
              }
            }
            else
            {
              if ( ucDummy == KEY_LEFT )
              {
                ucDummy = KEY_NUMPAD_4;
              }
              else if ( ucDummy == KEY_UP )
              {
                ucDummy = KEY_NUMPAD_8;
              }
              else if ( ucDummy == KEY_RIGHT )
              {
                ucDummy = KEY_NUMPAD_6;
              }
              else if ( ucDummy == KEY_DOWN )
              {
                ucDummy = KEY_NUMPAD_2;
              }
              else if ( ucDummy == KEY_PAGEUP )
              {
                ucDummy = KEY_NUMPAD_9;
              }
              else if ( ucDummy == KEY_PAGEDOWN )
              {
                ucDummy = KEY_NUMPAD_3;
              }
              else if ( ucDummy == KEY_HOME )
              {
                ucDummy = KEY_NUMPAD_7;
              }
              else if ( ucDummy == KEY_END )
              {
                ucDummy = KEY_NUMPAD_1;
              }
              else if ( ucDummy == KEY_INSERT )
              {
                ucDummy = KEY_NUMPAD_0;
              }
              else if ( ucDummy == KEY_DELETE )
              {
                ucDummy = KEY_NUMPAD_KOMMA;
              }
            }
            if ( ucDummy == KEY_LCONTROL )
            {
              if ( m_pVirtualKey[KEY_LCONTROL].m_bPressed )
              {
                bControlWasOn = TRUE;
              }
              else
              {
                bControlWasOn = FALSE;
              }
              bLastKeyDownControl = TRUE;
            }
            else
            {
              bLastKeyDownControl = FALSE;
            }
            SetButtonPressed( ucDummy, TRUE );
          }
        }
      }
      break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      if ( m_bInputActive )
      {
        SendEvent( tInputEvent( tInputEvent::IE_WIN_KEY_UP, wParam ) );

        bLastKeyDownControl = FALSE;
        if ( ( wParam >= 0 )
        &&   ( wParam <= 239 )
        &&   ( !m_bInputDX ) )
        {
          if ( m_ucVirtualCode[wParam] != KEY_INVALID )
          {
            ucDummy = m_ucVirtualCode[wParam];
            if ( ucDummy == KEY_LSHIFT )
            {
              if ( ( ( lParam & 0xff0000 ) >> 16 ) == 54 )
              {
                ucDummy = KEY_RSHIFT;
              }
            }
            if ( lParam & 0x1000000 )
            {
              // ein enhanced Keycode!
              if ( ucDummy == KEY_ENTER )
              {
                ucDummy = KEY_NUMPAD_ENTER;
              }
              else if ( ucDummy == KEY_LCONTROL )
              {
                ucDummy = KEY_RCONTROL;
              }
              else if ( ucDummy == KEY_LALT )
              {
                ucDummy = KEY_RALT;
              }
            }
            else
            {
              if ( ucDummy == KEY_LEFT )
              {
                ucDummy = KEY_NUMPAD_4;
              }
              else if ( ucDummy == KEY_UP )
              {
                ucDummy = KEY_NUMPAD_8;
              }
              else if ( ucDummy == KEY_RIGHT )
              {
                ucDummy = KEY_NUMPAD_6;
              }
              else if ( ucDummy == KEY_DOWN )
              {
                ucDummy = KEY_NUMPAD_2;
              }
              else if ( ucDummy == KEY_PAGEUP )
              {
                ucDummy = KEY_NUMPAD_9;
              }
              else if ( ucDummy == KEY_PAGEDOWN )
              {
                ucDummy = KEY_NUMPAD_3;
              }
              else if ( ucDummy == KEY_HOME )
              {
                ucDummy = KEY_NUMPAD_7;
              }
              else if ( ucDummy == KEY_END )
              {
                ucDummy = KEY_NUMPAD_1;
              }
              else if ( ucDummy == KEY_INSERT )
              {
                ucDummy = KEY_NUMPAD_0;
              }
              else if ( ucDummy == KEY_DELETE )
              {
                ucDummy = KEY_NUMPAD_KOMMA;
              }
            }
            SetButtonPressed( ucDummy, FALSE );
          }
        }
      }
      break;
    case WM_ACTIVATE:
      {
        if ( wParam == WA_INACTIVE  )
        {
          m_bInputActive = FALSE;
        }
        else
        {
          m_bInputActive = TRUE;
        }
        #ifndef __NO_DIRECT_X__
        if ( m_bInputDX )
        {
          tVectDevices::iterator    itDev( m_vectDevices.begin() );
          while ( itDev != m_vectDevices.end() )
          {
            tInputDevice&   Device = *itDev;

            if ( Device.m_pDevice )
            {
              if ( m_bInputActive )
              {
                Device.m_pDevice->Acquire();
              }
              else
              {
                Device.m_pDevice->Unacquire();
              }
            }

            ++itDev;
          }
        }
        #endif // __NO_DIRECT_X__
      }
      break;
    case WM_CHAR:
      if ( m_bInputActive )
      {
        SendEvent( tInputEvent( tInputEvent::IE_CHAR_ENTERED, m_ucVirtualCode[wParam], (DWORD_PTR)wParam ) );
      }
      break;
  }

  //dh::Log( "CInputSystem::InputSubClassProc CallWindowProc %x", m_lpfnInputOldWndProc );

  return CallWindowProc( m_lpfnInputOldWndProc, hWnd, uMsg, wParam, lParam );

}


 
void CInputSystem::AddCommand( const char *szCommand, GR::u32 dwKey, GR::u32 dwFlags )
{

  tInputCommand    tiCmd = tInputCommand();


  tiCmd.m_dwFlags = dwFlags;
  tiCmd.m_dwKey   = dwKey;
  tiCmd.m_strDesc = szCommand;


  m_mapCommands.insert( std::pair<GR::String,tInputCommand>( tiCmd.m_strDesc, tiCmd ) );

}



void CInputSystem::RemoveCommand( const char *szCommand )
{

  tMapCommands::iterator   it;


  while ( ( it = m_mapCommands.find( GR::String( szCommand ) ) ) != m_mapCommands.end() )
  {
    m_mapCommands.erase( it );
  }

}



bool CInputSystem::IsCommandByKey( GR::u32 dwKey, GR::String& strCommand ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }

  tMapCommands::const_iterator   it( m_mapCommands.begin() );
  while ( it != m_mapCommands.end() )
  {
    const tInputCommand&    Command = it->second;

    if ( Command.m_dwKey != dwKey )
    {
      ++it;
      continue;
    }

    switch ( Command.m_dwFlags )
    {
      case tInputCommand::CMD_DOWN:
        if ( VKeyPressed( dwKey ) )
        {
          strCommand = it->first;
          return true;
        }
        break;
      case tInputCommand::CMD_RELEASED_DOWN:
        if ( ( m_pVirtualKey[dwKey].m_bPressed )
        &&   ( m_pVirtualKey[dwKey].m_bReleased ) )
        {
          strCommand = it->first;
          return true;
        }
        break;
      case tInputCommand::CMD_UP:
        if ( !VKeyPressed( it->second.m_dwKey ) )
        {
          strCommand = it->first;
          return true;
        }
        break;
    }
    ++it;
  }
  return false;

}



bool CInputSystem::IsCommand( const GR::String& strCommand, bool bDoNotModifyFlags ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }

  tMapCommands::const_iterator   it( m_mapCommands.find( strCommand ) );

  if ( it == m_mapCommands.end() )
  {
    return false;
  }

  do
  {
    switch ( it->second.m_dwFlags )
    {
      case tInputCommand::CMD_DOWN:
        if ( VKeyPressed( it->second.m_dwKey ) )
        {
          return true;
        }
        break;
      case tInputCommand::CMD_RELEASED_DOWN:
        if ( bDoNotModifyFlags )
        {
          if ( ( VKeyPressed( it->second.m_dwKey ) )
          &&   ( VKeyReleased( it->second.m_dwKey ) ) )
          {
            return true;
          }
        }
        else if ( ReleasedVKeyPressed( it->second.m_dwKey ) )
        {
          return true;
        }
        break;
      case tInputCommand::CMD_UP:
        if ( !VKeyPressed( it->second.m_dwKey ) )
        {
          return true;
        }
        break;
    }
    it++;
    if ( it == m_mapCommands.end() )
    {
      break;
    }
    if ( it->first != strCommand )
    {
      break;
    }
  }
  while ( true );
  return false;

}



GR::u32 CInputSystem::PressedVKey() const
{


  if ( !m_bInitialized )
  {
    return KEY_INVALID;
  }

  for ( GR::u32 dwI = 1; dwI < m_iVirtualKeys; dwI++ )
  {
    if ( VKeyPressed( dwI ) )
    {
      return dwI;
    }
  }
  return KEY_INVALID;

}



BOOL CInputSystem::SetAbilities( int iSensitivity, int iTreshold, int iJTreshold )
{

  if ( !m_bInitialized )
  {
    return TRUE;
  }
  if ( iSensitivity != -1 )
  {
    m_iInputMouseSensitivity = iSensitivity;
  }
  if ( iTreshold != -1 )
  {
    m_iInputMouseTreshold = iTreshold;
  }
  if ( iJTreshold != -1 )
  {
    m_iInputJoyTreshold = iJTreshold;
  }
  return TRUE;

}



bool CInputSystem::VKeyDown( GR::u32 dwKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( dwKey >= m_iVirtualKeys )
  {
    return false;
  }
  return m_pVirtualKey[dwKey].m_bButtonDown;

}



GR::u32 CInputSystem::MapKeyToVKey( eInputDefaultButtons edbKey ) const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  if ( edbKey >= DEFKEY_LAST_ENTRY )
  {
    return 0;
  }
  tMapDefaultKeys::const_iterator   it( m_mapDefaultKeys.find( edbKey ) );
  if ( it == m_mapDefaultKeys.end() )
  {
    return 0;
  }
  return it->second;

}



bool CInputSystem::KeyPressed( eInputDefaultButtons edbKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( edbKey >= DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapDefaultKeys::const_iterator   it( m_mapDefaultKeys.find( edbKey ) );
  if ( it == m_mapDefaultKeys.end() )
  {
    return false;
  }
  return VKeyPressed( it->second );

}



bool CInputSystem::VKeyPressed( GR::u32 dwKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( dwKey >= m_iVirtualKeys )
  {
    dh::Warning( "CInputSystem::VKeyPressed VKey Index out of bounds %d >= %d!", dwKey, m_iVirtualKeys );
    return false;
  }
  return m_pVirtualKey[dwKey].m_bPressed;

}



bool CInputSystem::ReleasedKeyPressed( eInputDefaultButtons edbKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( edbKey >= DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapDefaultKeys::const_iterator   it( m_mapDefaultKeys.find( edbKey ) );
  if ( it == m_mapDefaultKeys.end() )
  {
    return false;
  }
  return ReleasedVKeyPressed( it->second );

}



bool CInputSystem::ReleasedVKeyPressed( GR::u32 dwKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( dwKey >= m_iVirtualKeys )
  {
    dh::Warning( "CInputSystem::ReleasedVKeyPressed VKey Index out of bounds %d >= %d!", dwKey, m_iVirtualKeys );
    return false;
  }

  if ( ( m_pVirtualKey[dwKey].m_bPressed )
  &&   ( m_pVirtualKey[dwKey].m_bReleased ) )
  {
    m_pVirtualKey[dwKey].m_bReleased = false;
    return true;
  }
  return false;

}



bool CInputSystem::KeyReleased( eInputDefaultButtons edbKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( edbKey >= DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapDefaultKeys::const_iterator   it( m_mapDefaultKeys.find( edbKey ) );
  if ( it == m_mapDefaultKeys.end() )
  {
    return false;
  }
  return VKeyReleased( it->second );

}



bool CInputSystem::VKeyReleased( GR::u32 dwKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( dwKey >= m_iVirtualKeys )
  {
    dh::Warning( "CInputSystem::VKeyReleased VKey Index out of bounds %d >= %d!", dwKey, m_iVirtualKeys );
    return false;
  }
  return m_pVirtualKey[dwKey].m_bReleased;

}



bool CInputSystem::VKeySetReleaseFlag( GR::u32 dwKey ) const
{

  if ( !m_bInitialized )
  {
    return false;
  }
  if ( dwKey >= m_iVirtualKeys )
  {
    dh::Warning( "CInputSystem::VKeySetReleaseFlag VKey Index out of bounds %d >= %d!", dwKey, m_iVirtualKeys );
    return false;
  }
  if ( m_pVirtualKey[dwKey].m_bPressed )
  {
    m_pVirtualKey[dwKey].m_bReleased = false;
  }
  return true;

}



int CInputSystem::MouseX() const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  return m_iInputWMMouseX;

}



int CInputSystem::MouseY() const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  return m_iInputWMMouseY;

}



int CInputSystem::MouseZ() const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  return m_iInputMouseZ;

}



GR::tPoint CInputSystem::MousePos() const
{

  if ( !m_bInitialized )
  {
    return GR::tPoint();
  }
  return GR::tPoint( m_iInputWMMouseX, m_iInputWMMouseY );

}



int CInputSystem::MouseDeltaX() const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  return m_iInputMouseRelX;

}



int CInputSystem::MouseDeltaY() const
{

  if ( !m_bInitialized )
  {
    return 0;
  }
  return m_iInputMouseRelY;

}



GR::u32 CInputSystem::MouseButton() const
{

  GR::u32         dwDummy = 0;


  if ( KeyPressed( MOUSE_BUTTON_1 ) )
  {
    dwDummy |= 1;
  }
  if ( KeyPressed( MOUSE_BUTTON_2 ) )
  {
    dwDummy |= 2;
  }
  if ( KeyPressed( MOUSE_BUTTON_3 ) )
  {
    dwDummy |= 4;
  }
  return dwDummy;

}



void CInputSystem::ClearMouseFlags()
{

  m_pVirtualKey[MOUSE_BUTTON_1].m_bPressed = false;
  m_pVirtualKey[MOUSE_BUTTON_2].m_bPressed = false;
  m_pVirtualKey[MOUSE_BUTTON_3].m_bPressed = false;

}



GR::u32 CInputSystem::GetVirtualCode( GR::u32 dwNr ) const
{

  if ( dwNr >= 240 )
  {
    return 0;
  }
  return m_ucVirtualCode[dwNr];

}



void CInputSystem::SetButtonPressed( GR::u32 dwButton, bool bPressed )
{

  if ( dwButton >= m_iVirtualKeys )
  {
    return;
  }

  if ( bPressed )
  {
    if ( !m_pVirtualKey[dwButton].m_bPressed )
    {
      // erstmalig gedrückt
      /*
      dh::Log( "Key Pressed %s (DefKey %d, Ofs %d)", m_pVirtualKey[dwButton].m_strName.c_str(), 
               m_pVirtualKey[dwButton].m_iEnumIndex,
               m_vectControls[m_pVirtualKey[dwButton].m_iGlobalIndex].m_dwDataOffset );
               */

      m_pVirtualKey[dwButton].m_bPressed = true;
      m_pVirtualKey[dwButton].m_dwButtonDownTicks = m_dwPollTicks;
      m_pVirtualKey[dwButton].m_bButtonFirstTimeDelay = true;
      m_pVirtualKey[dwButton].m_bButtonDown = true;


      if ( m_bInputActive )
      {
        if ( m_pVirtualKey[dwButton].m_iEnumIndex )
        {
          SendEvent( tInputEvent( tInputEvent::IE_KEY_DOWN, m_pVirtualKey[dwButton].m_iEnumIndex ) );
        }
        SendEvent( tInputEvent( tInputEvent::IE_VKEY_DOWN, dwButton ) );

        GR::String strCommand;

        if ( IsCommandByKey( m_pVirtualKey[dwButton].m_iEnumIndex, strCommand ) )
        {
          SendEvent( tInputEvent( tInputEvent::IE_COMMAND, 0, 0, strCommand ) );
        }
      }
    }
    else
    {
      if ( m_pVirtualKey[dwButton].m_bButtonFirstTimeDelay )
      {
        if ( m_dwPollTicks - m_pVirtualKey[dwButton].m_dwButtonDownTicks >= m_dwKeyboardDelayTicks )
        {
          m_pVirtualKey[dwButton].m_bButtonFirstTimeDelay = false;
          m_pVirtualKey[dwButton].m_bButtonDown = true;
          m_pVirtualKey[dwButton].m_dwButtonDownTicks = m_dwPollTicks;

          if ( m_bInputActive )
          {
            if ( m_pVirtualKey[dwButton].m_iEnumIndex )
            {
              SendEvent( tInputEvent( tInputEvent::IE_KEY_DOWN, m_pVirtualKey[dwButton].m_iEnumIndex ) );
            }
            SendEvent( tInputEvent( tInputEvent::IE_VKEY_DOWN, dwButton ) );
          }
        }
      }
      else if ( m_dwPollTicks - m_pVirtualKey[dwButton].m_dwButtonDownTicks >= m_dwKeyboardRepeatDelayTicks )
      {
        m_pVirtualKey[dwButton].m_bButtonDown = true;
        m_pVirtualKey[dwButton].m_dwButtonDownTicks = m_dwPollTicks;

        if ( m_bInputActive )
        {
          if ( m_pVirtualKey[dwButton].m_iEnumIndex )
          {
            SendEvent( tInputEvent( tInputEvent::IE_KEY_DOWN, m_pVirtualKey[dwButton].m_iEnumIndex ) );
          }
          SendEvent( tInputEvent( tInputEvent::IE_VKEY_DOWN, dwButton ) );
        }
      }
    }
  }
  else
  {
    if ( m_pVirtualKey[dwButton].m_bPressed )
    {
      if ( m_bInputActive )
      {
        if ( m_pVirtualKey[dwButton].m_iEnumIndex )
        {
          SendEvent( tInputEvent( tInputEvent::IE_KEY_UP, m_pVirtualKey[dwButton].m_iEnumIndex ) );
        }
        SendEvent( tInputEvent( tInputEvent::IE_VKEY_UP, dwButton ) );
      }

      m_pVirtualKey[dwButton].m_bPressed = false;

      if ( m_bInputActive )
      {
        GR::String strCommand;
        
        if ( IsCommandByKey( m_pVirtualKey[dwButton].m_iEnumIndex, strCommand ) )
        {
          SendEvent( tInputEvent( tInputEvent::IE_COMMAND, 0, 0, strCommand ) );
        }
      }
    }
    m_pVirtualKey[dwButton].m_bReleased = true;
  }
  
}



void CInputSystem::Poll()
{

  if ( !m_bInitialized )
  {
    return;
  }

  m_dwPollTicks = GetTickCount();

  for ( GR::u32 i = 0; i < m_iVirtualKeys; ++i )
  {
    m_pVirtualKey[i].m_bButtonDown = false;
  }

  JOYINFO         stJoyInfo;

  POINT           pt;


  GetCursorPos( &pt );
  if ( IsWindow( m_hwndRelativeMouseTo ) )
  {
    ScreenToClient( m_hwndRelativeMouseTo, &pt );
  }

  m_iInputWMMouseX = pt.x;
  m_iInputWMMouseY = pt.y;

  if ( m_bInputActive )
  {
    SendEvent( tInputEvent( tInputEvent::IE_MOUSE_UPDATE, 0, 0 ) );
  }

  if ( !m_bInputDX )
  {
    // Poll, wenn kein Direct-Input

    // Joystick
    if ( MyJoyGetPos( JOYSTICKID1, &stJoyInfo ) == JOYERR_NOERROR )
    {
      m_iInputJoyX = stJoyInfo.wXpos;
      m_iInputJoyY = stJoyInfo.wYpos;

      SetButtonPressed( JOYSTICK_1_LEFT, (BOOL)( m_iInputJoyX < 32768 - m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_1_RIGHT, (BOOL)( m_iInputJoyX > 32768 + m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_1_UP, (BOOL)( m_iInputJoyY < 32768 - m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_1_DOWN, (BOOL)( m_iInputJoyY > 32768 + m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_1_BUTTON_1, (BOOL)( stJoyInfo.wButtons & JOY_BUTTON1 ) );
      SetButtonPressed( JOYSTICK_1_BUTTON_2, !!( stJoyInfo.wButtons & JOY_BUTTON2 ) );
    }
    if ( MyJoyGetPos( JOYSTICKID2, &stJoyInfo ) == JOYERR_NOERROR )
    {
      m_iInputJoyX = stJoyInfo.wXpos;
      m_iInputJoyY = stJoyInfo.wYpos;

      SetButtonPressed( JOYSTICK_2_LEFT, (BOOL)( m_iInputJoyX < 32768 - m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_2_RIGHT, (BOOL)( m_iInputJoyX > 32768 + m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_2_UP, (BOOL)( m_iInputJoyY < 32768 - m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_2_DOWN, (BOOL)( m_iInputJoyY > 32768 + m_iInputJoyTreshold ) );
      SetButtonPressed( JOYSTICK_2_BUTTON_1, (BOOL)( stJoyInfo.wButtons & JOY_BUTTON1 ) );
      SetButtonPressed( JOYSTICK_2_BUTTON_2, !!( stJoyInfo.wButtons & JOY_BUTTON2 ) );
    }

    // Maus
    GetCursorPos( &pt );
    ScreenToClient( m_hwndInput, &pt );

    SetButtonPressed( m_bInputMButtonsSwapped ? MOUSE_BUTTON_2 : MOUSE_BUTTON_1, ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 );
    SetButtonPressed( m_bInputMButtonsSwapped ? MOUSE_BUTTON_1 : MOUSE_BUTTON_2, ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) != 0 );
    SetButtonPressed( MOUSE_BUTTON_3, ( GetAsyncKeyState( VK_MBUTTON ) & 0x8000 ) != 0 );

    m_iInputMouseRelX = pt.x - m_iInputMouseX;
    m_iInputMouseRelY = pt.y - m_iInputMouseY;

    m_iInputMouseX = pt.x;
    m_iInputMouseY = pt.y;

    SetButtonPressed( MOUSE_LEFT, (BOOL)( m_iInputMouseRelX < -m_iInputMouseTreshold ) );
    SetButtonPressed( MOUSE_RIGHT, (BOOL)( m_iInputMouseRelX > m_iInputMouseTreshold ) );
    SetButtonPressed( MOUSE_UP, (BOOL)( m_iInputMouseRelY < -m_iInputMouseTreshold ) );
    SetButtonPressed( MOUSE_DOWN, (BOOL)( m_iInputMouseRelY > m_iInputMouseTreshold ) );
  }
  #ifndef __NO_DIRECT_X__
  else
  {
    tVectDevices::iterator    itDev( m_vectDevices.begin() );
    while ( itDev != m_vectDevices.end() )
    {
      tInputDevice&   Device = *itDev;

      if ( Device.m_pDevice )
      {
        PollDevice( Device );
      }

      ++itDev;
    }
  }
  #endif // __NO_DIRECT_X__

}



bool CInputSystem::IsJoystickAvailable()
{

  return ( m_iCurrentEnumJoystickDevice != 0 );

}



void CInputSystem::SetAlternateMousePosWindow( HWND hwndRelative )
{

  m_hwndRelativeMouseTo = hwndRelative;

}



void CInputSystem::Init()
{
}



void CInputSystem::Exit()
{

  RemoveAllListeners();
  Release();

}



void CInputSystem::Update( const float fElapsedTime )
{
  Poll();
}



bool CInputSystem::ShiftPressed() const
{

  return ( KeyPressed( KEY_LSHIFT ) | KeyPressed( KEY_RSHIFT ) ) != 0;

}


bool CInputSystem::CtrlPressed() const
{

  return ( KeyPressed( KEY_LCONTROL ) | KeyPressed( KEY_RCONTROL ) ) != 0;

}


bool CInputSystem::AltPressed() const
{

  return ( KeyPressed( KEY_LALT ) | KeyPressed( KEY_RALT ) ) != 0;

}





const char* CInputSystem::GetControlName( GR::u32 dwKey ) const
{

  if ( dwKey >= m_vectControls.size() )
  {
    return "";
  }
  return m_vectControls[dwKey].m_strName.c_str();

}



const char* CInputSystem::GetKeyName( GR::u32 dwKey ) const
{

  if ( dwKey >= m_iVirtualKeys )
  {
    return "";
  }
  return m_pVirtualKey[dwKey].m_strName.c_str();

}



GR::u32 CInputSystem::GetControlType( GR::u32 dwKey ) const
{

  if ( dwKey >= m_vectControls.size() )
  {
    return 0;
  }
  return m_vectControls[dwKey].m_dwType;

}



GR::u32 CInputSystem::GetControlCount() const
{

  return m_iVirtualKeys;

}



int CInputSystem::GetControlDevice( GR::u32 dwKey ) const
{

  if ( dwKey >= m_iVirtualKeys )
  {
    return 0;
  }
  return m_vectControls[m_pVirtualKey[dwKey].m_iGlobalIndex].m_iDevice;

}



const char* CInputSystem::GetDeviceName( GR::u32 iDevice ) const
{

  if ( iDevice >= m_vectDevices.size() )
  {
    return "";
  }
  return m_vectDevices[iDevice].m_strDevice.c_str();

}



#ifndef __NO_DIRECT_X__
bool CInputSystem::PollDevice( tInputDevice& Device )
{

  HRESULT   hRes = Device.m_pDevice->Poll();
  if ( SUCCEEDED( hRes ) )
  {
    hRes = Device.m_pDevice->GetDeviceState( (GR::u32)( Device.m_vectControls.size() * sizeof( GR::u32 ) ), Device.m_pDeviceData );
    if ( SUCCEEDED( hRes ) )
    {
      ParseDeviceData( Device );
    }
    else
    {
      dh::Log( "GetDeviceState failed %x", hRes );
    }
    return true;
  }
  if ( hRes == DIERR_NOTACQUIRED )
  {
    if ( m_bInputActive )
    {
      // nur automatisch acquiren, wenn wir aktiv sind
      hRes = Device.m_pDevice->Acquire();
      if ( FAILED( hRes ) )
      {
        dh::Log( "Acquire failed %x", hRes );
      }
      else
      {
        return true;
      }
    }
  }
  else
  {
    dh::Log( "Poll failed %x", hRes );
  }

  return false;

}




bool CInputSystem::SetDataFormat( tInputDevice& Device )
{

  Device.m_pDeviceData = new GR::u32[Device.m_vectControls.size()];

  DIOBJECTDATAFORMAT*   pDataFormat = new DIOBJECTDATAFORMAT[Device.m_vectControls.size()];

  for ( size_t iObject = 0; iObject < Device.m_vectControls.size(); ++iObject )
  {
    tInputCtrl&   Ctrl = Device.m_vectControls[iObject];

    pDataFormat[iObject].dwOfs    = Ctrl.m_dwDataOffset;
    pDataFormat[iObject].dwType   = Ctrl.m_dwType;
    pDataFormat[iObject].pguid    = NULL;
    pDataFormat[iObject].dwFlags  = 0;
  }


  DIDATAFORMAT      diDataFormat;

  diDataFormat.dwSize       = sizeof( diDataFormat );
  diDataFormat.dwObjSize    = sizeof( DIOBJECTDATAFORMAT );
  diDataFormat.dwNumObjs    = (GR::u32)Device.m_vectControls.size();
  diDataFormat.dwFlags      = 0;
  diDataFormat.dwDataSize   = (GR::u32)Device.m_vectControls.size() * sizeof( GR::u32 );
  diDataFormat.rgodf        = pDataFormat;

  HRESULT hRes = Device.m_pDevice->SetDataFormat( &diDataFormat );

  delete[] pDataFormat;

  if ( SUCCEEDED( hRes ) )
  {
    /*
    dh::Log( "SetDataFormat succeeded (%d Bytes allocated)",
      Device.m_vectControls.size() * sizeof( GR::u32 ) );
      */
  }
  else
  {
    dh::Log( "SetDataFormat failed %x", hRes );
  }

  return SUCCEEDED( hRes );

}




void CInputSystem::ParseDeviceData( tInputDevice& Device )
{

  for ( size_t i = 0; i < Device.m_vectControls.size(); ++i )
  {
    tInputCtrl&   Ctrl = Device.m_vectControls[i];

    GR::u32   dwValue = *(GR::u32*)( (BYTE*)Device.m_pDeviceData + Ctrl.m_dwDataOffset );


    if ( Ctrl.m_dwType & DIDFT_BUTTON )
    {
      SetButtonPressed( Ctrl.m_iVirtualIndex, ( dwValue & 0x080 ) != 0 );
    }
    else if ( Ctrl.m_dwType & DIDFT_ABSAXIS )
    {
      // minus
      SetButtonPressed( Ctrl.m_iVirtualIndex, ( (int)dwValue < ( Ctrl.m_iMax - Ctrl.m_iMin ) / 2 - m_iInputJoyTreshold ) );
      // plus
      SetButtonPressed( Ctrl.m_iVirtualIndex + 1, ( (int)dwValue > ( Ctrl.m_iMax - Ctrl.m_iMin ) / 2 + m_iInputJoyTreshold ) );
    }
    else if ( Ctrl.m_dwType & DIDFT_RELAXIS )
    {
      int   iDelta = (int)dwValue - Ctrl.m_iMin;

      if ( abs( iDelta ) > m_iInputMouseTreshold )
      {
        if ( iDelta < 0 )
        {
          SetButtonPressed( Ctrl.m_iVirtualIndex, true );
          SetButtonPressed( Ctrl.m_iVirtualIndex + 1, false );
        }
        else
        {
          SetButtonPressed( Ctrl.m_iVirtualIndex, false );
          SetButtonPressed( Ctrl.m_iVirtualIndex + 1, true );
        }
      }
      else
      {
        SetButtonPressed( Ctrl.m_iVirtualIndex, false );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 1, false );
      }
      Ctrl.m_iMin = (int)dwValue;
    }
    else if ( Ctrl.m_dwType & DIDFT_POV )
    {
      if ( LOWORD( dwValue ) != 0xFFFF )
      {
        int     iDegrees = dwValue / DI_DEGREES;

        SetButtonPressed( Ctrl.m_iVirtualIndex, ( ( iDegrees >= 225 ) && ( iDegrees <= 315 ) ) );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 1, ( ( iDegrees >= 45 ) && ( iDegrees <= 135 ) ) );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 2, ( ( iDegrees <= 45 ) || ( iDegrees >= 315 ) ) );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 3, ( ( iDegrees >= 135 ) && ( iDegrees <= 225 ) ) );
      }
      else
      {
        // Mittelstellung
        SetButtonPressed( Ctrl.m_iVirtualIndex + 0, false );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 1, false );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 2, false );
        SetButtonPressed( Ctrl.m_iVirtualIndex + 3, false );
      }
    }
  }


}




void CInputSystem::CreateVirtualKeys()
{

  m_pVirtualKey = new tVirtualKey[m_iVirtualKeys];

  int   iCurVKey = 1;
  
  int   iCurMButton = 0;

  int   iCurJ1Button = 0,
        iCurJ2Button = 0;


  // der 0. ist ungültig
  m_pVirtualKey[0].m_strName = "No Key";
  m_pVirtualKey[0].m_iGlobalIndex = 0;
  m_pVirtualKey[0].m_iEnumIndex = KEY_INVALID;

  for ( size_t i = 0; i < m_vectControls.size(); ++i )
  {
    tInputCtrl&   Ctrl = m_vectControls[i];

    if ( Ctrl.m_dwType & DIDFT_BUTTON )
    {
      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName;

      if ( Ctrl.m_DeviceType & VKT_KEYBOARD )
      {
        // man staune, das DataOffset ist der tatsächliche Tastatur-Index!!
        if ( Ctrl.m_dwDataOffset )
        {
          m_mapDefaultKeys[(eInputDefaultButtons)Ctrl.m_dwDataOffset] = Ctrl.m_iVirtualIndex;
          m_pVirtualKey[iCurVKey].m_iEnumIndex = (eInputDefaultButtons)Ctrl.m_dwDataOffset;
          m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
        }
      }
      else if ( Ctrl.m_DeviceType & VKT_DEFAULT_MOUSE )
      {
        // man staune, das DataOffset ist der tatsächliche Tastatur-Index!!
        if ( iCurMButton < 3 )
        {
          m_mapDefaultKeys[(eInputDefaultButtons)( MOUSE_BUTTON_1 + iCurMButton )] = Ctrl.m_iVirtualIndex;
          m_pVirtualKey[iCurVKey].m_iEnumIndex = (eInputDefaultButtons)( MOUSE_BUTTON_1 + iCurMButton );
          m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
        }

        ++iCurMButton;
      }
      else if ( Ctrl.m_DeviceType & VKT_JOYSTICK_1 )
      {
        if ( iCurJ1Button < 16 )
        {
          m_mapDefaultKeys[(eInputDefaultButtons)( JOYSTICK_1_BUTTON_1 + iCurJ1Button )] = Ctrl.m_iVirtualIndex;
          m_pVirtualKey[iCurVKey].m_iEnumIndex = (eInputDefaultButtons)( JOYSTICK_1_BUTTON_1 + iCurJ1Button );
          m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
        }
        ++iCurJ1Button;
      }
      else if ( Ctrl.m_DeviceType & VKT_JOYSTICK_2 )
      {
        if ( iCurJ2Button < 16 )
        {
          m_mapDefaultKeys[(eInputDefaultButtons)( JOYSTICK_2_BUTTON_1 + iCurJ2Button )] = Ctrl.m_iVirtualIndex;
          m_pVirtualKey[iCurVKey].m_iEnumIndex = (eInputDefaultButtons)( JOYSTICK_2_BUTTON_1 + iCurJ2Button );
          m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
        }
        ++iCurJ2Button;
      }


      ++iCurVKey;
    }
    else if ( Ctrl.m_dwType & DIDFT_ABSAXIS )
    {
      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " down";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " up";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      if ( Ctrl.m_DeviceType & VKT_JOYSTICK_1 )
      {
        if ( Ctrl.m_DeviceType & VKT_X_AXIS )
        {
          m_mapDefaultKeys[JOYSTICK_1_LEFT] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = JOYSTICK_1_LEFT;
          m_mapDefaultKeys[JOYSTICK_1_RIGHT] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = JOYSTICK_1_RIGHT;
        }
        if ( Ctrl.m_DeviceType & VKT_Y_AXIS )
        {
          m_mapDefaultKeys[JOYSTICK_1_UP] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = JOYSTICK_1_UP;
          m_mapDefaultKeys[JOYSTICK_1_DOWN] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = JOYSTICK_1_DOWN;
        }
      }
      if ( Ctrl.m_DeviceType & VKT_JOYSTICK_2 )
      {
        if ( Ctrl.m_DeviceType & VKT_X_AXIS )
        {
          m_mapDefaultKeys[JOYSTICK_2_LEFT] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = JOYSTICK_2_LEFT;
          m_mapDefaultKeys[JOYSTICK_2_RIGHT] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = JOYSTICK_2_RIGHT;
        }
        if ( Ctrl.m_DeviceType & VKT_Y_AXIS )
        {
          m_mapDefaultKeys[JOYSTICK_2_UP] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = JOYSTICK_2_UP;
          m_mapDefaultKeys[JOYSTICK_2_DOWN] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = JOYSTICK_2_DOWN;
        }
      }
    }
    else if ( Ctrl.m_dwType & DIDFT_RELAXIS )
    {
      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " down";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " up";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      if ( Ctrl.m_DeviceType & VKT_DEFAULT_MOUSE )
      {
        if ( Ctrl.m_DeviceType & VKT_X_AXIS )
        {
          m_mapDefaultKeys[MOUSE_LEFT] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = MOUSE_LEFT;
          m_mapDefaultKeys[MOUSE_RIGHT] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = MOUSE_RIGHT;
        }
        if ( Ctrl.m_DeviceType & VKT_Y_AXIS )
        {
          m_mapDefaultKeys[MOUSE_UP] = iCurVKey - 2;
          m_pVirtualKey[iCurVKey - 2].m_iEnumIndex = MOUSE_UP;
          m_mapDefaultKeys[MOUSE_DOWN] = iCurVKey - 1;
          m_pVirtualKey[iCurVKey - 1].m_iEnumIndex = MOUSE_DOWN;
        }
      }
    }
    else if ( Ctrl.m_dwType & DIDFT_POV )
    {
      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " left";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " right";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " up";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;

      m_pVirtualKey[iCurVKey].m_iGlobalIndex = (GR::u32)i;
      m_pVirtualKey[iCurVKey].m_strName = Ctrl.m_strName + " down";
      m_pVirtualKey[iCurVKey].m_iDeviceControlIndex = Ctrl.m_iDeviceControlIndex;
      ++iCurVKey;
    }
  }

}
#endif // __NO_DIRECT_X__



bool CInputSystem::IsInitialized()
{

  return m_bInitialized;

}



void CInputSystem::AddBinding( const GR::u32 dwHandle, const GR::u32 dwKey )
{

  m_mapBinding2VKey[dwHandle] = dwKey;

}



void CInputSystem::RemoveBinding( const GR::u32 dwHandle )
{

  tMapBinding2VKey::iterator    it( m_mapBinding2VKey.find( dwHandle ) );
  if ( it != m_mapBinding2VKey.end() )
  {
    m_mapBinding2VKey.erase( it );
  }

}



bool CInputSystem::SaveBindings( IIOStream& Stream )
{

  if ( !Stream.IsGood() )
  {
    return false;
  }

  Stream.WriteSize( m_mapBinding2VKey.size() );

  tMapBinding2VKey::iterator    it( m_mapBinding2VKey.begin() );
  while ( it != m_mapBinding2VKey.end() )
  {
    GR::u32   dwVKey( it->second );

    if ( ( dwVKey >= 0 )
    &&   ( dwVKey < m_iVirtualKeys )
    &&   ( m_pVirtualKey[dwVKey].m_iGlobalIndex < m_vectControls.size() )
    &&   ( m_vectControls[m_pVirtualKey[dwVKey].m_iGlobalIndex].m_iDevice < (int)m_vectDevices.size() ) )
    {
      Stream.WriteU32( it->first );
      Stream.WriteString( m_vectDevices[m_vectControls[m_pVirtualKey[dwVKey].m_iGlobalIndex].m_iDevice].m_strDevice );
      Stream.WriteU32( m_pVirtualKey[dwVKey].m_iDeviceControlIndex );
    }

    ++it;
  }
  return true;
}



bool CInputSystem::LoadBindings( IIOStream& Stream )
{

  ClearAllBindings();

  if ( !Stream.IsGood() )
  {
    return false;
  }

  bool      bRestoreBindingFailed = false;

  size_t    iEntries = Stream.ReadSize();

  for ( size_t i = 0; i < iEntries; ++i )
  {
    GR::u32         dwBindHandle = Stream.ReadU32();

    GR::String      strDevice = Stream.ReadString();

    GR::u32         dwControlIndex = Stream.ReadU32();

    bool            bBindingPossible = false;


    tVectDevices::iterator    it( m_vectDevices.begin() );
    while ( it != m_vectDevices.end() )
    {
      tInputDevice&  Device( *it );

      if ( ( Device.m_strDevice == strDevice )
      &&   ( dwControlIndex < Device.m_vectControls.size() ) )
      {
        bBindingPossible = true;

        m_mapBinding2VKey[dwBindHandle] = Device.m_vectControls[dwControlIndex].m_iVirtualIndex;
        break;
      }

      ++it;
    }
    if ( !bBindingPossible )
    {
      bRestoreBindingFailed = true;
    }
  }

  return bRestoreBindingFailed;

}



void CInputSystem::ClearAllBindings()
{

  m_mapBinding2VKey.clear();

}



GR::u32 CInputSystem::BoundKey( const GR::u32 dwBindHandle ) const
{

  tMapBinding2VKey::const_iterator    it( m_mapBinding2VKey.find( dwBindHandle ) );
  if ( it == m_mapBinding2VKey.end() )
  {
    return 0;
  }
  return it->second;

}