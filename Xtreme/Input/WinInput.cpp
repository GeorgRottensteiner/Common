#include <Misc/Misc.h>

#include "WinInput.h"

#include <MMSystem.h>



static MMRESULT ( WINAPI *MyJoyGetPos )( UINT uJoyID,    LPJOYINFO pji );

static UINT ( WINAPI *MyJoyGetNumDevs )( VOID ); 



char CWinInput::m_KeyName[240][25] =
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
  "Mouse Middle Button" 
};



CWinInput::CWinInput() :
  m_InputMouseX( 0 ),
  m_InputMouseY( 0 ),
  m_InputMouseRelX( 0 ),
  m_InputMouseRelY( 0 ),
  m_hWinMMHandle( NULL ),
  m_wInputJoystickID( -1 )
{
}



CWinInput::~CWinInput()
{
  Release();
}



bool CWinInput::Initialize( GR::IEnvironment& Environment )
{
  if ( !XBasicInput::Initialize( Environment ) )
  {
    return false;
  }

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
  m_VirtualKeys = Xtreme::DEFKEY_LAST_ENTRY;
  m_pVirtualKey = new tVirtualKey[m_VirtualKeys];

  // das 0. gibt es nicht
  m_pVirtualKey[0].m_Name         = "No Key";
  m_pVirtualKey[0].m_GlobalIndex  = 0;
  m_pVirtualKey[0].m_EnumIndex    = Xtreme::KEY_INVALID;

  m_Controls.reserve( Xtreme::DEFKEY_LAST_ENTRY );
  m_Controls.push_back( tInputCtrl() );

  m_Devices.push_back( tInputDevice( "Keyboard" ) );

  for ( int i = Xtreme::FIRST_KEYBOARD_KEY; i <= Xtreme::LAST_KEYBOARD_KEY; ++i )
  {
    tInputCtrl      NewCtrl( 0, m_KeyName[i] );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    m_pVirtualKey[i].m_Name         = m_KeyName[i];
    m_pVirtualKey[i].m_GlobalIndex  = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex    = (Xtreme::eInputDefaultButtons)i;
    m_pVirtualKey[i].m_Released     = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)i, NewCtrl.m_VirtualIndex ) );
  }

  // Maus
  m_Devices.push_back( tInputDevice( "Mouse" ) );

  for ( int i = Xtreme::MOUSE_LEFT; i <= Xtreme::MOUSE_BUTTON_3; ++i )
  {
    tInputCtrl      NewCtrl( 1, m_KeyName[i] );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    m_pVirtualKey[i].m_Name      = m_KeyName[i];
    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex   = (Xtreme::eInputDefaultButtons)i;
    m_pVirtualKey[i].m_Released    = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)i, NewCtrl.m_VirtualIndex ) );
  }

  for ( int i = Xtreme::MOUSE_WHEEL_UP; i <= Xtreme::MOUSE_WHEEL_DOWN; ++i )
  {
    GR::String   strKeyName;
    if ( i == Xtreme::MOUSE_WHEEL_UP )
    {
      strKeyName = "Mousewheel up";
    }
    else if ( i == Xtreme::MOUSE_WHEEL_DOWN )
    {
      strKeyName = "Mousewheel down";
    }
    tInputCtrl      NewCtrl( 1, strKeyName );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    m_pVirtualKey[i].m_Name      = strKeyName;
    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex   = (Xtreme::eInputDefaultButtons)i;
    m_pVirtualKey[i].m_Released    = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)i, NewCtrl.m_VirtualIndex ) );
  }

  // Joystick
  m_Devices.push_back( tInputDevice( "Joystick" ) );

  GR::String     dummy = "";
  for ( int i = Xtreme::JOYSTICK_1_LEFT; i <= Xtreme::JOYSTICK_2_BUTTON_32; ++i )
  {
    dummy = "Joystick ";

    if ( i >= Xtreme::JOYSTICK_2_LEFT )
    {
      dummy += "2 ";
    }
    else
    {
      dummy += "1 ";
    }
    switch ( ( i - Xtreme::JOYSTICK_1_LEFT ) % 36 )
    {
      case 0:
        dummy += "left";
        break;
      case 1:
        dummy += "right";
        break;
      case 2:
        dummy += "up";
        break;
      case 3:
        dummy += "down";
        break;
      default:
        dummy += CMisc::printf( "Button %d", ( i - Xtreme::JOYSTICK_1_LEFT ) % 36 - 4 );
    }
    
    tInputCtrl      NewCtrl( 2, dummy );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    
    m_pVirtualKey[i].m_GlobalIndex  = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex    = (Xtreme::eInputDefaultButtons)i;
    m_pVirtualKey[i].m_Name         = dummy;
    m_pVirtualKey[i].m_Released     = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( (Xtreme::eInputDefaultButtons)i, NewCtrl.m_VirtualIndex ) );
  }

  int k = MyJoyGetNumDevs();
  if ( k > 0 )
  {
    JOYINFO         stJoyInfo;

    if ( MyJoyGetPos( JOYSTICKID1, &stJoyInfo ) != JOYERR_UNPLUGGED )
    {
      m_Devices.push_back( tInputDevice( "Joystick 1" ) );
    }
    if ( MyJoyGetPos( JOYSTICKID2, &stJoyInfo ) != JOYERR_UNPLUGGED )
    {
      m_Devices.push_back( tInputDevice( "Joystick 2" ) );
    }
  }

  m_Initialized = true;
  return true;
}



bool CWinInput::Release()
{
  if ( !m_Initialized )
  {
    return true;
  }

  if ( m_pVirtualKey )
  {
    delete[] m_pVirtualKey;
    m_pVirtualKey = NULL;
  }

  if ( m_hWinMMHandle )
  {
    FreeLibrary( m_hWinMMHandle );
    m_hWinMMHandle = NULL;
  }

  bool    Result = XBasicInput::Release();

  m_Initialized = false;
  return Result;
}



void CWinInput::Update( const float fElapsedTime )
{
  Poll();
}



void CWinInput::Init()
{
}



void CWinInput::Exit()
{
  RemoveAllListeners();
  Release();
}



void CWinInput::Poll()
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

  JOYINFO         stJoyInfo;

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

  // Poll, wenn kein Direct-Input

  // Joystick
  if ( MyJoyGetPos( JOYSTICKID1, &stJoyInfo ) == JOYERR_NOERROR )
  {
    SetButtonPressed( Xtreme::JOYSTICK_1_LEFT, (BOOL)( stJoyInfo.wXpos < 32768 - (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_1_RIGHT, (BOOL)( stJoyInfo.wXpos > 32768 + (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_1_UP, (BOOL)( stJoyInfo.wYpos < 32768 - (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_1_DOWN, (BOOL)( stJoyInfo.wYpos > 32768 + (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_1_BUTTON_1, (BOOL)( stJoyInfo.wButtons & JOY_BUTTON1 ) );
    SetButtonPressed( Xtreme::JOYSTICK_1_BUTTON_2, !!( stJoyInfo.wButtons & JOY_BUTTON2 ) );
  }
  if ( MyJoyGetPos( JOYSTICKID2, &stJoyInfo ) == JOYERR_NOERROR )
  {
    SetButtonPressed( Xtreme::JOYSTICK_2_LEFT, (BOOL)( stJoyInfo.wXpos < 32768 - (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_2_RIGHT, (BOOL)( stJoyInfo.wXpos > 32768 + (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_2_UP, (BOOL)( stJoyInfo.wYpos < 32768 - (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_2_DOWN, (BOOL)( stJoyInfo.wYpos > 32768 + (UINT)m_InputJoyTreshold ) );
    SetButtonPressed( Xtreme::JOYSTICK_2_BUTTON_1, (BOOL)( stJoyInfo.wButtons & JOY_BUTTON1 ) );
    SetButtonPressed( Xtreme::JOYSTICK_2_BUTTON_2, !!( stJoyInfo.wButtons & JOY_BUTTON2 ) );
  }

  // Maus
  GetCursorPos( &pt );
  ScreenToClient( m_hwndInput, &pt );

  SetButtonPressed( m_InputMButtonsSwapped ? Xtreme::MOUSE_BUTTON_2 : Xtreme::MOUSE_BUTTON_1, ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 );
  SetButtonPressed( m_InputMButtonsSwapped ? Xtreme::MOUSE_BUTTON_1 : Xtreme::MOUSE_BUTTON_2, ( GetAsyncKeyState( VK_RBUTTON ) & 0x8000 ) != 0 );
  SetButtonPressed( Xtreme::MOUSE_BUTTON_3, ( GetAsyncKeyState( VK_MBUTTON ) & 0x8000 ) != 0 );

  m_InputMouseRelX = pt.x - m_InputMouseX;
  m_InputMouseRelY = pt.y - m_InputMouseY;

  m_InputMouseX = pt.x;
  m_InputMouseY = pt.y;

  SetButtonPressed( Xtreme::MOUSE_LEFT, (BOOL)( m_InputMouseRelX < -m_InputMouseTreshold ) );
  SetButtonPressed( Xtreme::MOUSE_RIGHT, (BOOL)( m_InputMouseRelX > m_InputMouseTreshold ) );
  SetButtonPressed( Xtreme::MOUSE_UP, (BOOL)( m_InputMouseRelY < -m_InputMouseTreshold ) );
  SetButtonPressed( Xtreme::MOUSE_DOWN, (BOOL)( m_InputMouseRelY > m_InputMouseTreshold ) );
}



BOOL CWinInput::WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam )
{
  static BOOL               lastKeyDownControl = FALSE,
                            controlWasOn = FALSE;


  switch ( wMessage )
  {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if ( ( wParam >= 0 )
      &&   ( wParam <= 239 ) )
      {
        if ( m_VirtualCode[wParam] != Xtreme::KEY_INVALID )
        {
          GR::u8 ucDummy = m_VirtualCode[wParam];
          if ( ucDummy == Xtreme::KEY_LSHIFT )
          {
            if ( ( ( lParam & 0xff0000 ) >> 16 ) == 54 )
            {
              ucDummy = Xtreme::KEY_RSHIFT;
            }
          }
          if ( lParam & 0x1000000 )
          {
            // ein enhanced Keycode!
            if ( ucDummy == Xtreme::KEY_ENTER )
            {
              ucDummy = Xtreme::KEY_NUMPAD_ENTER;
            }
            else if ( ucDummy == Xtreme::KEY_LCONTROL )
            {
              ucDummy = Xtreme::KEY_RCONTROL;
            }
            else if ( ucDummy == Xtreme::KEY_LALT )
            {
              ucDummy = Xtreme::KEY_RALT;
              if ( ( lastKeyDownControl )
              &&   ( !controlWasOn ) )
              {
                SetButtonPressed( Xtreme::KEY_LCONTROL, FALSE );
              }
            }
          }
          else
          {
            if ( ucDummy == Xtreme::KEY_LEFT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_4;
            }
            else if ( ucDummy == Xtreme::KEY_UP )
            {
              ucDummy = Xtreme::KEY_NUMPAD_8;
            }
            else if ( ucDummy == Xtreme::KEY_RIGHT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_6;
            }
            else if ( ucDummy == Xtreme::KEY_DOWN )
            {
              ucDummy = Xtreme::KEY_NUMPAD_2;
            }
            else if ( ucDummy == Xtreme::KEY_PAGEUP )
            {
              ucDummy = Xtreme::KEY_NUMPAD_9;
            }
            else if ( ucDummy == Xtreme::KEY_PAGEDOWN )
            {
              ucDummy = Xtreme::KEY_NUMPAD_3;
            }
            else if ( ucDummy == Xtreme::KEY_HOME )
            {
              ucDummy = Xtreme::KEY_NUMPAD_7;
            }
            else if ( ucDummy == Xtreme::KEY_END )
            {
              ucDummy = Xtreme::KEY_NUMPAD_1;
            }
            else if ( ucDummy == Xtreme::KEY_INSERT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_0;
            }
            else if ( ucDummy == Xtreme::KEY_DELETE )
            {
              ucDummy = Xtreme::KEY_NUMPAD_KOMMA;
            }
          }
          if ( ucDummy == Xtreme::KEY_LCONTROL )
          {
            if ( m_pVirtualKey[Xtreme::KEY_LCONTROL].m_Pressed )
            {
              controlWasOn = TRUE;
            }
            else
            {
              controlWasOn = FALSE;
            }
            lastKeyDownControl = TRUE;
          }
          else
          {
            lastKeyDownControl = FALSE;
          }
          SetButtonPressed( ucDummy, TRUE );
        }
      }
      break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      lastKeyDownControl = FALSE;
      if ( ( wParam >= 0 )
      &&   ( wParam <= 239 ) )
      {
        if ( m_VirtualCode[wParam] != Xtreme::KEY_INVALID )
        {
          GR::u8 ucDummy = m_VirtualCode[wParam];
          if ( ucDummy == Xtreme::KEY_LSHIFT )
          {
            if ( ( ( lParam & 0xff0000 ) >> 16 ) == 54 )
            {
              ucDummy = Xtreme::KEY_RSHIFT;
            }
          }
          if ( lParam & 0x1000000 )
          {
            // ein enhanced Keycode!
            if ( ucDummy == Xtreme::KEY_ENTER )
            {
              ucDummy = Xtreme::KEY_NUMPAD_ENTER;
            }
            else if ( ucDummy == Xtreme::KEY_LCONTROL )
            {
              ucDummy = Xtreme::KEY_RCONTROL;
            }
            else if ( ucDummy == Xtreme::KEY_LALT )
            {
              ucDummy = Xtreme::KEY_RALT;
            }
          }
          else
          {
            if ( ucDummy == Xtreme::KEY_LEFT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_4;
            }
            else if ( ucDummy == Xtreme::KEY_UP )
            {
              ucDummy = Xtreme::KEY_NUMPAD_8;
            }
            else if ( ucDummy == Xtreme::KEY_RIGHT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_6;
            }
            else if ( ucDummy == Xtreme::KEY_DOWN )
            {
              ucDummy = Xtreme::KEY_NUMPAD_2;
            }
            else if ( ucDummy == Xtreme::KEY_PAGEUP )
            {
              ucDummy = Xtreme::KEY_NUMPAD_9;
            }
            else if ( ucDummy == Xtreme::KEY_PAGEDOWN )
            {
              ucDummy = Xtreme::KEY_NUMPAD_3;
            }
            else if ( ucDummy == Xtreme::KEY_HOME )
            {
              ucDummy = Xtreme::KEY_NUMPAD_7;
            }
            else if ( ucDummy == Xtreme::KEY_END )
            {
              ucDummy = Xtreme::KEY_NUMPAD_1;
            }
            else if ( ucDummy == Xtreme::KEY_INSERT )
            {
              ucDummy = Xtreme::KEY_NUMPAD_0;
            }
            else if ( ucDummy == Xtreme::KEY_DELETE )
            {
              ucDummy = Xtreme::KEY_NUMPAD_KOMMA;
            }
          }
          SetButtonPressed( ucDummy, FALSE );
        }
      }
      break;
  }
  return XBasicInput::WindowProc( hWnd, wMessage, wParam, lParam );
}



bool CWinInput::HasDeviceForceFeedback( GR::u32 Device ) const
{
  return false;
}



GR::u32 CWinInput::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  return 0;
}



bool CWinInput::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  return false;
}
