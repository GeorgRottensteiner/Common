#include "XBasicInput.h"

#include <Interface/IIOStream.h>

#if ( OPERATING_SYSTEM == OS_WINDOWS )
#include <WinSys/SubclassManager.h>
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif
#endif

#include <Xtreme/Environment/XWindow.h>

#include <debug/debugclient.h>
#include <debug/DebugService.h>

#include <Misc/Misc.h>





XBasicInput* XBasicInput::m_pGlobalInputInstance = NULL;



XBasicInput::XBasicInput() :
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  m_lpfnInputOldWndProc( NULL ),
#endif
#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE ) && ( OPERATING_SUB_SYSTEM != OS_SUB_SDL ) && ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP )
  m_hwndInput( NULL ),
#endif
  m_Initialized( false ),
  m_pVirtualKey( NULL ),
  m_VirtualKeys( 0 ),
  m_InputMouseSensitivity( 5 ),
  m_InputMouseTreshold( 5 ),
  m_InputJoyTreshold( 16000 ),
  m_PollTicks( 0 ),
  m_KeyboardDelayTicks( 0 ),
  m_KeyboardRepeatDelayTicks( 0 ),
  m_InputActive( false ),
  m_InputMButtonsSwapped( false ),
  m_InputWMMouseX( 0 ),
  m_InputWMMouseY( 0 ),
  m_InputMouseWheelAccumulatedDelta( 0 ),
  m_pDebugger( NULL )
{
  m_pGlobalInputInstance = this;

  memset( m_VirtualCode, Xtreme::KEY_INVALID, sizeof( m_VirtualCode ) );
  m_VirtualCode[  8] = Xtreme::KEY_BACKSPACE;
  m_VirtualCode[  9] = Xtreme::KEY_TAB;
  m_VirtualCode[ 13] = Xtreme::KEY_ENTER;
  m_VirtualCode[ 16] = Xtreme::KEY_LSHIFT;
  m_VirtualCode[ 17] = Xtreme::KEY_LCONTROL;
  m_VirtualCode[ 18] = Xtreme::KEY_LALT;
  m_VirtualCode[ 27] = Xtreme::KEY_ESCAPE;
  m_VirtualCode[ 32] = Xtreme::KEY_SPACE;
  m_VirtualCode[ 33] = Xtreme::KEY_PAGEUP;
  m_VirtualCode[ 34] = Xtreme::KEY_PAGEDOWN;
  m_VirtualCode[ 35] = Xtreme::KEY_END;
  m_VirtualCode[ 36] = Xtreme::KEY_HOME;
  m_VirtualCode[ 37] = Xtreme::KEY_LEFT;
  m_VirtualCode[ 38] = Xtreme::KEY_UP;
  m_VirtualCode[ 39] = Xtreme::KEY_RIGHT;
  m_VirtualCode[ 40] = Xtreme::KEY_DOWN;
  m_VirtualCode[ 45] = Xtreme::KEY_INSERT;
  m_VirtualCode[ 46] = Xtreme::KEY_DELETE;
  m_VirtualCode[ 48] = Xtreme::KEY_0;
  m_VirtualCode[ 49] = Xtreme::KEY_1;
  m_VirtualCode[ 50] = Xtreme::KEY_2;
  m_VirtualCode[ 51] = Xtreme::KEY_3;
  m_VirtualCode[ 52] = Xtreme::KEY_4;
  m_VirtualCode[ 53] = Xtreme::KEY_5;
  m_VirtualCode[ 54] = Xtreme::KEY_6;
  m_VirtualCode[ 55] = Xtreme::KEY_7;
  m_VirtualCode[ 56] = Xtreme::KEY_8;
  m_VirtualCode[ 57] = Xtreme::KEY_9;
  m_VirtualCode[ 65] = Xtreme::KEY_A;
  m_VirtualCode[ 66] = Xtreme::KEY_B;
  m_VirtualCode[ 67] = Xtreme::KEY_C;
  m_VirtualCode[ 68] = Xtreme::KEY_D;
  m_VirtualCode[ 69] = Xtreme::KEY_E;
  m_VirtualCode[ 70] = Xtreme::KEY_F;
  m_VirtualCode[ 71] = Xtreme::KEY_G;
  m_VirtualCode[ 72] = Xtreme::KEY_H;
  m_VirtualCode[ 73] = Xtreme::KEY_I;
  m_VirtualCode[ 74] = Xtreme::KEY_J;
  m_VirtualCode[ 75] = Xtreme::KEY_K;
  m_VirtualCode[ 76] = Xtreme::KEY_L;
  m_VirtualCode[ 77] = Xtreme::KEY_M;
  m_VirtualCode[ 78] = Xtreme::KEY_N;
  m_VirtualCode[ 79] = Xtreme::KEY_O;
  m_VirtualCode[ 80] = Xtreme::KEY_P;
  m_VirtualCode[ 81] = Xtreme::KEY_Q;
  m_VirtualCode[ 82] = Xtreme::KEY_R;
  m_VirtualCode[ 83] = Xtreme::KEY_S;
  m_VirtualCode[ 84] = Xtreme::KEY_T;
  m_VirtualCode[ 85] = Xtreme::KEY_U;
  m_VirtualCode[ 86] = Xtreme::KEY_V;
  m_VirtualCode[ 87] = Xtreme::KEY_W;
  m_VirtualCode[ 88] = Xtreme::KEY_X;
  m_VirtualCode[ 89] = Xtreme::KEY_Y;
  m_VirtualCode[ 90] = Xtreme::KEY_Z;
  m_VirtualCode[ 96] = Xtreme::KEY_NUMPAD_0;
  m_VirtualCode[ 97] = Xtreme::KEY_NUMPAD_1;
  m_VirtualCode[ 98] = Xtreme::KEY_NUMPAD_2;
  m_VirtualCode[ 99] = Xtreme::KEY_NUMPAD_3;
  m_VirtualCode[100] = Xtreme::KEY_NUMPAD_4;
  m_VirtualCode[101] = Xtreme::KEY_NUMPAD_5;
  m_VirtualCode[102] = Xtreme::KEY_NUMPAD_6;
  m_VirtualCode[103] = Xtreme::KEY_NUMPAD_7;
  m_VirtualCode[104] = Xtreme::KEY_NUMPAD_8;
  m_VirtualCode[105] = Xtreme::KEY_NUMPAD_9;
  m_VirtualCode[106] = Xtreme::KEY_NUMPAD_MULTIPLY;
  m_VirtualCode[107] = Xtreme::KEY_NUMPAD_PLUS;
  m_VirtualCode[109] = Xtreme::KEY_NUMPAD_MINUS;
  m_VirtualCode[110] = Xtreme::KEY_PUNKT;
  m_VirtualCode[111] = Xtreme::KEY_NUMPAD_DIVIDE;
  m_VirtualCode[112] = Xtreme::KEY_F1;
  m_VirtualCode[113] = Xtreme::KEY_F2;
  m_VirtualCode[114] = Xtreme::KEY_F3;
  m_VirtualCode[115] = Xtreme::KEY_F4;
  m_VirtualCode[116] = Xtreme::KEY_F5;
  m_VirtualCode[117] = Xtreme::KEY_F6;
  m_VirtualCode[118] = Xtreme::KEY_F7;
  m_VirtualCode[119] = Xtreme::KEY_F8;
  m_VirtualCode[120] = Xtreme::KEY_F9;
  m_VirtualCode[121] = Xtreme::KEY_F10;
  m_VirtualCode[122] = Xtreme::KEY_F11;
  m_VirtualCode[123] = Xtreme::KEY_F12;
  m_VirtualCode[144] = Xtreme::KEY_NUMLOCK;
  m_VirtualCode[145] = Xtreme::KEY_SCROLLLOCK;
  m_VirtualCode[186] = Xtreme::KEY_UE;
  m_VirtualCode[187] = Xtreme::KEY_PLUS;
  m_VirtualCode[188] = Xtreme::KEY_KOMMA;
  //m_VirtualCode[189] = Xtreme::KEY_SLASH;
  m_VirtualCode[190] = Xtreme::KEY_PUNKT;
  m_VirtualCode[191] = Xtreme::KEY_BACKSLASH;
  m_VirtualCode[192] = Xtreme::KEY_OE;
  m_VirtualCode[189] = Xtreme::KEY_MINUS;
  m_VirtualCode[220] = Xtreme::KEY_GRAVE;
  m_VirtualCode[221] = Xtreme::KEY_EQUALS;
  m_VirtualCode[222] = Xtreme::KEY_AE;
  m_VirtualCode[226] = Xtreme::KEY_GREATER;
}



XBasicInput::~XBasicInput()
{
  Release();
}



bool XBasicInput::Initialize( GR::IEnvironment& Environment )
{
  if ( m_Initialized )
  {
    return true;
  }

  m_pEnvironment = &Environment;

  m_pDebugger = (IDebugService*)Environment.Service( "Logger" );
  m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize" );

  Xtreme::IAppWindow* pWindowService = (Xtreme::IAppWindow*)Environment.Service( "Window" );

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE )
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }
#endif
  m_Controls.clear();
  m_Devices.clear();
  m_Binding2VKey.clear();

  m_InputMButtonsSwapped = false;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  m_InputMButtonsSwapped = !!GetSystemMetrics( SM_SWAPBUTTON );
#endif

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE ) && ( OPERATING_SUB_SYSTEM != OS_SUB_SDL ) && ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP )
  m_hwndInput             = hWnd;
  m_hwndRelativeMouseTo   = hWnd;
#endif

  m_VirtualKeys          = 1;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  SystemParametersInfo( SPI_GETKEYBOARDDELAY, 0, &m_KeyboardDelayTicks, 0 );
  SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &m_KeyboardRepeatDelayTicks, 0 );
#endif
  m_KeyboardDelayTicks = ( m_KeyboardDelayTicks + 1 ) * 250;
  m_KeyboardRepeatDelayTicks = ( 400 - 33 ) * m_KeyboardRepeatDelayTicks / 400;

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
  if ( GetActiveWindow() == m_hwndInput )
  {
    m_InputActive = true;
    m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize active window is input window, set active" );
  }
  else
  {
    m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize active window is not input window, set inactive" );
  }
#endif

  m_pDebugger->Log( "Input.Full", "Subclassing Window" );

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  // Window subclassen (zum Messages abfangen)
  SubclassInputWindow();
#endif

  m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize done" );

  return true;
}



bool XBasicInput::Release()
{
  if ( !m_Initialized )
  {
    return true;
  }

  if ( m_pDebugger )
  {
    m_pDebugger->Log( "Input.Full", "XBasicInput::Release" );
  }
#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
  if ( IsWindow( m_hwndInput ) )
  {
    UnSubclassInputWindow();
  }
#endif
  if ( m_pDebugger )
  {
    m_pDebugger->Log( "Input.Full", "XBasicInput::Release done" );
  }
  m_pEnvironment = NULL;
  m_Initialized = false;
  return true;
}



#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
LRESULT WINAPI XBasicInput::InputSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return m_pGlobalInputInstance->WindowProc( hWnd, uMsg, wParam, lParam );
}



BOOL XBasicInput::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    case WM_MOUSEWHEEL:
      if ( m_InputActive )
      {
        int mouseZ = GET_WHEEL_DELTA_WPARAM( wParam );

        m_InputMouseWheelAccumulatedDelta += mouseZ;

        while ( m_InputMouseWheelAccumulatedDelta <= -WHEEL_DELTA )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_MOUSEWHEEL_DOWN, ( m_InputWMMouseX << 16 ) + m_InputWMMouseY, MouseButton() ) );

          std::pair<GR::u32,GR::u32>    pairKey( MapKeyToVKey( Xtreme::MOUSE_WHEEL_DOWN ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );
          tMapKeyHandler::iterator      itKeyHandler( m_KeyHandler.find( pairKey ) );
          if ( itKeyHandler != m_KeyHandler.end() )
          {
            itKeyHandler->second();
          }

          m_InputMouseWheelAccumulatedDelta += WHEEL_DELTA;
        }
        while ( m_InputMouseWheelAccumulatedDelta >= WHEEL_DELTA )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_MOUSEWHEEL_UP, ( m_InputWMMouseX << 16 ) + m_InputWMMouseY, MouseButton() ) );

          std::pair<GR::u32,GR::u32>    pairKey( MapKeyToVKey( Xtreme::MOUSE_WHEEL_UP ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );
          tMapKeyHandler::iterator      itKeyHandler( m_KeyHandler.find( pairKey ) );
          if ( itKeyHandler != m_KeyHandler.end() )
          {
            itKeyHandler->second();
          }

          m_InputMouseWheelAccumulatedDelta -= WHEEL_DELTA;
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
        SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_1 ), !!( wParam & MK_LBUTTON ) );
        SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_2 ), !!( wParam & MK_RBUTTON ) );
        SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_3 ), !!( wParam & MK_MBUTTON ) );

        // GUI
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
      }
      break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      if ( m_InputActive )
      {
        GR::u32     specialKeyFlags = 0;
        if ( ShiftPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
        }
        if ( CtrlPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
        }
        if ( AltPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
        }
        SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_WIN_KEY_DOWN, wParam, specialKeyFlags ) );
      }
      break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      if ( m_InputActive )
      {
        GR::u32     specialKeyFlags = 0;
        if ( ShiftPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
        }
        if ( CtrlPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
        }
        if ( AltPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
        }
        SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_WIN_KEY_UP, wParam, specialKeyFlags ) );
      }
      break;
    case WM_ACTIVATEAPP:
      if ( wParam )
      {
        m_InputActive = TRUE;
        m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize app window active, set active" );
      }
      else
      {
        m_InputActive = FALSE;
        m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize app window inactive, set inactive" );
      }
      break;
    case WM_ACTIVATE:
      {
        if ( wParam == WA_INACTIVE  )
        {
          m_InputActive = FALSE;
          m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize main window inactive, set inactive" );
        }
        else
        {
          m_InputActive = TRUE;
          m_pDebugger->Log( "Input.Full", "XBasicInput::Initialize main window active, set active" );
        }
      }
      break;
    case WM_CHAR:
      if ( m_InputActive )
      {
        SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_CHAR_ENTERED, m_VirtualCode[wParam], (DWORD_PTR)wParam ) );
      }
      break;
  }
  ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );

  if ( pManager )
  {
    return pManager->CallNext( hWnd, uMsg, wParam, lParam );
  }

  return (BOOL)CallWindowProc( m_lpfnInputOldWndProc, hWnd, uMsg, wParam, lParam );
}



bool XBasicInput::SubclassInputWindow()
{
  m_pDebugger->Log( "Input.Full", "SubclassingInputWindow" );
  ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );

  if ( pManager )
  {
    return pManager->AddHandler( "XInput", fastdelegate::MakeDelegate( this, &XBasicInput::WindowProc ) );
  }
#pragma warning( push )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4312 )
  // Häßlich, W64-Warning disablen, läßt sich hier sonst nicht unterdrücken
  //m_lpfnInputOldWndProc = (WNDPROC)SetWindowLongPtr( m_hwndInput, GWL_WNDPROC, (LONG_PTR)(WNDPROC)InputSubClassProc );
  //m_lpfnInputOldWndProc = (WNDPROC)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)InputSubClassProc );
#pragma warning( pop )

  m_pDebugger->Log( "Input.Full", "SubclassingInputWindow done" );

  return true;
}



bool XBasicInput::UnSubclassInputWindow()
{
  m_pDebugger->Log( "Input.Full", "UnSubclassingInputWindow" );
  if ( m_pEnvironment )
  {
    ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );

    if ( pManager )
    {
      m_pDebugger->Log( "Input.Full", "UnSubclassingInputWindow done" );
      return pManager->RemoveHandler( "XInput" );
    }
  }

  #pragma warning( push )
  #pragma warning( disable : 4244 )
  #pragma warning( disable : 4312 )
  // Häßlich, W64-Warning disablen, läßt sich hier sonst nicht unterdrücken
  /*
  if ( (WNDPROC)GetWindowLongPtr( m_hwndInput, GWL_WNDPROC ) == InputSubClassProc )
  {

    SetWindowLongPtr( m_hwndInput, GWL_WNDPROC, (ORD_PTR)m_lpfnInputOldWndProc );
  }
  */
  /*
  if ( (WNDPROC)GetWindowLong( m_hwndInput, GWL_WNDPROC ) == InputSubClassProc )
  {
    SetWindowLong( m_hwndInput, GWL_WNDPROC, (ORD)m_lpfnInputOldWndProc );
  }
  */
  #pragma warning( pop )

  m_pDebugger->Log( "Input.Full", "UnSubclassingInputWindow done" );

  return true;
}
#endif



#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
void XBasicInput::SetAlternateMousePosWindow( HWND hwndRelative )
{
  m_hwndRelativeMouseTo = hwndRelative;
}
#endif



void XBasicInput::AddCommand( const char* Command, GR::u32 Key, GR::u32 Flags )
{
  Xtreme::tInputCommand    tiCmd;

  tiCmd.m_Flags = Flags;
  tiCmd.m_Key   = Key;
  tiCmd.m_Desc  = Command;

  m_Commands.insert( std::pair<GR::string,Xtreme::tInputCommand>( tiCmd.m_Desc.c_str(), tiCmd ) );
}



void XBasicInput::RemoveCommand( const char* Command )
{
  tMapCommands::iterator   it;

  while ( ( it = m_Commands.find( Command ) ) != m_Commands.end() )
  {
    m_Commands.erase( it );
  }
}



bool XBasicInput::IsCommandByKey( GR::u32 Key, const char* Command ) const
{
  if ( !m_Initialized )
  {
    return false;
  }

  tMapCommands::const_iterator   it( m_Commands.begin() );
  while ( it != m_Commands.end() )
  {
    const Xtreme::tInputCommand&    command = it->second;

    if ( command.m_Key != Key )
    {
      ++it;
      continue;
    }

    switch ( command.m_Flags )
    {
      case Xtreme::tInputCommand::CMD_DOWN:
        if ( VKeyPressed( Key ) )
        {
          Command = it->first.c_str();
          return true;
        }
        break;
      case Xtreme::tInputCommand::CMD_RELEASED_DOWN:
        if ( ( m_pVirtualKey[Key].m_Pressed )
        &&   ( m_pVirtualKey[Key].m_Released ) )
        {
          Command = it->first.c_str();
          return true;
        }
        break;
      case Xtreme::tInputCommand::CMD_UP:
        if ( !VKeyPressed( it->second.m_Key ) )
        {
          Command = it->first.c_str();
          return true;
        }
        break;
    }
    ++it;
  }
  return false;
}



bool XBasicInput::IsCommand( const char* Command, bool DoNotModifyFlags ) const
{
  if ( !m_Initialized )
  {
    return false;
  }

  tMapCommands::const_iterator   it( m_Commands.find( Command ) );
  if ( it == m_Commands.end() )
  {
    return false;
  }

  do
  {
    switch ( it->second.m_Flags )
    {
      case Xtreme::tInputCommand::CMD_DOWN:
        if ( VKeyPressed( it->second.m_Key ) )
        {
          return true;
        }
        break;
      case Xtreme::tInputCommand::CMD_RELEASED_DOWN:
        if ( DoNotModifyFlags )
        {
          if ( ( VKeyPressed( it->second.m_Key ) )
          &&   ( VKeyReleased( it->second.m_Key ) ) )
          {
            return true;
          }
        }
        else if ( ReleasedVKeyPressed( it->second.m_Key ) )
        {
          return true;
        }
        break;
      case Xtreme::tInputCommand::CMD_UP:
        if ( !VKeyPressed( it->second.m_Key ) )
        {
          return true;
        }
        break;
    }
    it++;
    if ( it == m_Commands.end() )
    {
      break;
    }
    if ( it->first != Command )
    {
      break;
    }
  }
  while ( true );
  return false;
}



bool XBasicInput::KeyPressed( Xtreme::eInputDefaultButtons Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= Xtreme::DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapVirtualKeys::const_iterator   it( m_VirtualKeyToXtremeKey.find( Key ) );
  while ( it != m_VirtualKeyToXtremeKey.end() )
  {
    if ( it->first != Key )
    {
      break;
    }
    if ( VKeyPressed( it->second ) )
    {
      return true;
    }

    ++it;
  }
  tMapDefaultKeys::const_iterator   it2( m_DefaultKeys.find( Key ) );
  if ( it2 == m_DefaultKeys.end() )
  {
    return false;
  }
  return VKeyPressed( it2->second );
}



bool XBasicInput::VKeyPressed( GR::u32 Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= m_VirtualKeys )
  {
    dh::Log( "CDXInput::VKeyPressed VKey Index out of bounds %d >= %d!", Key, m_VirtualKeys );
    return false;
  }
  return m_pVirtualKey[Key].m_Pressed;
}



bool XBasicInput::ReleasedKeyPressed( Xtreme::eInputDefaultButtons Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= Xtreme::DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapVirtualKeys::const_iterator   it( m_VirtualKeyToXtremeKey.find( Key ) );
  while ( it != m_VirtualKeyToXtremeKey.end() )
  {
    if ( it->first != Key )
    {
      break;
    }
    if ( ReleasedVKeyPressed( it->second ) )
    {
      return true;
    }

    ++it;
  }
  tMapDefaultKeys::const_iterator   it2( m_DefaultKeys.find( Key ) );
  if ( it2 == m_DefaultKeys.end() )
  {
    return false;
  }
  return ReleasedVKeyPressed( it2->second );
}



bool XBasicInput::ReleasedVKeyPressed( GR::u32 Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= m_VirtualKeys )
  {
    dh::Log( "CDXInput::ReleasedVKeyPressed VKey Index out of bounds %d >= %d!", Key, m_VirtualKeys );
    return false;
  }

  if ( ( m_pVirtualKey[Key].m_Pressed )
  &&   ( m_pVirtualKey[Key].m_Released ) )
  {
    m_pVirtualKey[Key].m_Released = false;
    return true;
  }
  if ( ( m_pVirtualKey[Key].m_WasPressed )
  &&   ( m_pVirtualKey[Key].m_WasReleasedWhilePressed ) )
  {
    m_pVirtualKey[Key].m_WasReleasedWhilePressed = false;
    m_pVirtualKey[Key].m_WasPressed = false;
    return true;
  }
  return false;
}



bool XBasicInput::KeyReleased( Xtreme::eInputDefaultButtons Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= Xtreme::DEFKEY_LAST_ENTRY )
  {
    return false;
  }
  tMapVirtualKeys::const_iterator   it( m_VirtualKeyToXtremeKey.find( Key ) );
  while ( it != m_VirtualKeyToXtremeKey.end() )
  {
    if ( it->first != Key )
    {
      break;
    }
    if ( VKeyReleased( it->second ) )
    {
      return true;
    }

    ++it;
  }
  tMapDefaultKeys::const_iterator   it2( m_DefaultKeys.find( Key ) );
  if ( it2 == m_DefaultKeys.end() )
  {
    return false;
  }
  return VKeyReleased( it2->second );
}



bool XBasicInput::VKeyReleased( GR::u32 Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= m_VirtualKeys )
  {
    dh::Log( "CDXInput::VKeyReleased VKey Index out of bounds %d >= %d!", Key, m_VirtualKeys );
    return false;
  }
  return m_pVirtualKey[Key].m_Released;
}



bool XBasicInput::VKeySetReleaseFlag( GR::u32 Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= m_VirtualKeys )
  {
    dh::Log( "CDXInput::VKeySetReleaseFlag VKey Index out of bounds %d >= %d!", Key, m_VirtualKeys );
    return false;
  }
  if ( m_pVirtualKey[Key].m_Pressed )
  {
    m_pVirtualKey[Key].m_Released = false;
  }
  return true;
}



bool XBasicInput::VKeyDown( GR::u32 Key ) const
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Key >= m_VirtualKeys )
  {
    return false;
  }
  return m_pVirtualKey[Key].m_ButtonDown;
}



GR::u32 XBasicInput::MapKeyToVKey( Xtreme::eInputDefaultButtons Key ) const
{
  if ( !m_Initialized )
  {
    dh::Log( "XBasicInput::MapKeyToVKey not initialized" );
    return 0;
  }
  if ( Key >= Xtreme::DEFKEY_LAST_ENTRY )
  {
    dh::Log( "XBasicInput::MapKeyToVKey out of bounds" );
    return 0;
  }

  tMapVirtualKeys::const_iterator   it( m_VirtualKeyToXtremeKey.find( Key ) );
  while ( it != m_VirtualKeyToXtremeKey.end() )
  {
    if ( it->first != Key )
    {
      break;
    }
    return it->second;
  }

  tMapDefaultKeys::const_iterator   it2( m_DefaultKeys.find( Key ) );
  if ( it2 == m_DefaultKeys.end() )
  {
    //dh::Log( "XBasicInput::MapKeyToVKey no mapping found" );
    return 0;
  }
  return it2->second;
}



void XBasicInput::AddBinding( const GR::u32 Handle, const GR::u32 Key, const GR::u32 SecondaryKey )
{
  m_Binding2VKey[Handle].first  = Key;
  m_Binding2VKey[Handle].second = SecondaryKey;

  m_pDebugger->Log( "Input.Full", "AddBinding for control %d, vkey %d, vkey2 %d, device %d, device control index %d, Enum Index %d, %s", Handle, Key, SecondaryKey,
                    GetControlDevice( Key ), m_pVirtualKey[Key].m_DeviceControlIndex, m_pVirtualKey[Key].m_EnumIndex, m_pVirtualKey[Key].m_Name.c_str() );
}



void XBasicInput::RemoveBinding( const GR::u32 Handle )
{
  tMapBinding2VKey::iterator    it( m_Binding2VKey.find( Handle ) );
  if ( it != m_Binding2VKey.end() )
  {
    m_Binding2VKey.erase( it );
  }
}



bool XBasicInput::SaveBindings( IIOStream& Stream )
{
  if ( !Stream.IsGood() )
  {
    return false;
  }

  Stream.WriteSize( m_Binding2VKey.size() );

  // primary bindings
  tMapBinding2VKey::iterator    it( m_Binding2VKey.begin() );
  while ( it != m_Binding2VKey.end() )
  {
    GR::u32   key( it->second.first );

    if ( ( key >= 0 )
    &&   ( key < m_VirtualKeys )
    &&   ( m_pVirtualKey[key].m_GlobalIndex < m_Controls.size() )
    &&   ( m_Controls[m_pVirtualKey[key].m_GlobalIndex].m_Device < (int)m_Devices.size() ) )
    {
      Stream.WriteU32( it->first );
      Stream.WriteString( m_Devices[m_Controls[m_pVirtualKey[key].m_GlobalIndex].m_Device].m_Device );
      Stream.WriteU32( m_pVirtualKey[key].m_DeviceControlIndex );
    }

    ++it;
  }

  // secondary bindings
  it = m_Binding2VKey.begin();
  while ( it != m_Binding2VKey.end() )
  {
    GR::u32   key( it->second.second );

    if ( ( key >= 0 )
    &&   ( key < m_VirtualKeys )
    &&   ( m_pVirtualKey[key].m_GlobalIndex < m_Controls.size() )
    &&   ( m_Controls[m_pVirtualKey[key].m_GlobalIndex].m_Device < (int)m_Devices.size() ) )
    {
      Stream.WriteU32( it->first );
      Stream.WriteString( m_Devices[m_Controls[m_pVirtualKey[key].m_GlobalIndex].m_Device].m_Device );
      Stream.WriteU32( m_pVirtualKey[key].m_DeviceControlIndex );
    }

    ++it;
  }

  return true;
}



bool XBasicInput::LoadBindings( IIOStream& Stream )
{
  ClearAllBindings();

  if ( !Stream.IsGood() )
  {
    return false;
  }

  bool              restoreBindingFailed = false;
  size_t            numEntries = Stream.ReadSize();

  for ( size_t i = 0; i < numEntries; ++i )
  {
    GR::u32     bindHandle      = Stream.ReadU32();
    GR::String  device          = Stream.ReadString();
    GR::u32     controlIndex    = Stream.ReadU32();
    bool        bindingPossible = false;


    tVectDevices::iterator    it( m_Devices.begin() );
    while ( it != m_Devices.end() )
    {
      tInputDevice&  Device( *it );

      if ( ( Device.m_Device == device )
      &&   ( controlIndex < Device.m_Controls.size() ) )
      {
        bindingPossible = true;

        m_Binding2VKey[bindHandle].first = Device.m_Controls[controlIndex].m_VirtualIndex;
        break;
      }

      ++it;
    }
    if ( !bindingPossible )
    {
      restoreBindingFailed = true;
    }
  }

  // secondary binding
  for ( size_t i = 0; i < numEntries; ++i )
  {
    GR::u32         bindHandle      = Stream.ReadU32();
    GR::String      device          = Stream.ReadString();
    GR::u32         controlIndex    = Stream.ReadU32();
    bool            bindingPossible = false;


    tVectDevices::iterator    it( m_Devices.begin() );
    while ( it != m_Devices.end() )
    {
      tInputDevice& Device( *it );

      if ( ( Device.m_Device == device )
      &&   ( controlIndex < Device.m_Controls.size() ) )
      {
        bindingPossible = true;

        m_Binding2VKey[bindHandle].second = Device.m_Controls[controlIndex].m_VirtualIndex;
        break;
      }

      ++it;
    }
    if ( !bindingPossible )
    {
      restoreBindingFailed = true;
    }
  }
  return !restoreBindingFailed;
}



void XBasicInput::ClearAllBindings()
{
  m_Binding2VKey.clear();
}



bool XBasicInput::BoundActionPressed( GR::u32 BindHandle ) const
{
  GR::u32     key1 = BoundKey( BindHandle );
  GR::u32     key2 = BoundSecondaryKey( BindHandle );

  if ( ( key1 != 0 )
  &&   ( VKeyPressed( key1 ) ) )
  {
    return true;
  }
  if ( ( key2 != 0 )
  &&   ( VKeyPressed( key2 ) ) )
  {
    return true;
  }
  return false;
}



bool XBasicInput::ReleasedBoundActionPressed( GR::u32 BindHandle ) const
{
  GR::u32     key1 = BoundKey( BindHandle );
  GR::u32     key2 = BoundSecondaryKey( BindHandle );

  if ( ( key1 != 0 )
  &&   ( ReleasedVKeyPressed( key1 ) ) )
  {
    return true;
  }
  if ( ( key2 != 0 )
  &&   ( ReleasedVKeyPressed( key2 ) ) )
  {
    return true;
  }
  return false;
}



GR::u32 XBasicInput::BoundKey( const GR::u32 BindHandle ) const
{
  tMapBinding2VKey::const_iterator    it( m_Binding2VKey.find( BindHandle ) );
  if ( it == m_Binding2VKey.end() )
  {
    return 0;
  }
  return it->second.first;
}



GR::u32 XBasicInput::BoundSecondaryKey( const GR::u32 BindHandle ) const
{
  tMapBinding2VKey::const_iterator    it( m_Binding2VKey.find( BindHandle ) );
  if ( it == m_Binding2VKey.end() )
  {
    return 0;
  }
  return it->second.second;
}



bool XBasicInput::SetAbilities( int Sensitivity, int Treshold, int JoystickTreshold )
{
  if ( !m_Initialized )
  {
    return false;
  }
  if ( Sensitivity != -1 )
  {
    m_InputMouseSensitivity = Sensitivity;
  }
  if ( Treshold != -1 )
  {
    m_InputMouseTreshold = Treshold;
  }
  if ( JoystickTreshold != -1 )
  {
    m_InputJoyTreshold = JoystickTreshold;
  }
  return true;
}



bool XBasicInput::IsJoystickAvailable()
{
  return false;
}



bool XBasicInput::IsTouchAvailable()
{
  return false;
}



void XBasicInput::SetButtonPressed( GR::u32 Button, bool Pressed )
{
  if ( ( Button >= m_VirtualKeys )
  ||   ( m_pVirtualKey == NULL ) )
  {
    return;
  }

  if ( Pressed )
  {
    if ( !m_pVirtualKey[Button].m_Pressed )
    {
      // erstmalig gedrückt
      /*
      dh::Log( "Key Pressed %s (DefKey %d, Ofs %d)", m_pVirtualKey[Button].m_strName.c_str(),
               m_pVirtualKey[Button].m_EnumIndex,
               m_Controls[m_pVirtualKey[Button].m_GlobalIndex].m_DataOffset );
               */

      m_pVirtualKey[Button].m_Pressed                 = true;
      m_pVirtualKey[Button].m_ButtonDownTicks         = m_PollTicks;
      m_pVirtualKey[Button].m_ButtonFirstTimeDelay    = true;
      m_pVirtualKey[Button].m_ButtonDown              = true;

      if ( m_InputActive )
      {
        std::pair<GR::u32,GR::u32>    pairKey( Button, Xtreme::tInputCommand::CMD_RELEASED_DOWN );

        tMapKeyHandler::iterator      itKeyHandler( m_KeyHandler.find( pairKey ) );
        if ( itKeyHandler != m_KeyHandler.end() )
        {
          itKeyHandler->second();
        }

        GR::u32     specialKeyFlags = 0;
        if ( ShiftPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
        }
        if ( CtrlPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
        }
        if ( AltPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
        }

        if ( m_pVirtualKey[Button].m_EnumIndex )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_KEY_DOWN, m_pVirtualKey[Button].m_EnumIndex, specialKeyFlags ) );
        }
        SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_VKEY_DOWN, Button, specialKeyFlags ) );

        GR::String strCommand;

        if ( IsCommandByKey( m_pVirtualKey[Button].m_EnumIndex, strCommand.c_str() ) )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_COMMAND, 0, 0, strCommand ) );
        }
      }
    }
    else
    {
      if ( m_InputActive )
      {
        std::pair<GR::u32,GR::u32>    pairKey( Button, Xtreme::tInputCommand::CMD_DOWN );

        tMapKeyHandler::iterator      itKeyHandler( m_KeyHandler.find( pairKey ) );
        if ( itKeyHandler != m_KeyHandler.end() )
        {
          itKeyHandler->second();
        }
      }

      if ( m_pVirtualKey[Button].m_ButtonFirstTimeDelay )
      {
        if ( m_PollTicks - m_pVirtualKey[Button].m_ButtonDownTicks >= m_KeyboardDelayTicks )
        {
          m_pVirtualKey[Button].m_ButtonFirstTimeDelay = false;
          m_pVirtualKey[Button].m_ButtonDown = true;
          m_pVirtualKey[Button].m_ButtonDownTicks = m_PollTicks;

          if ( m_InputActive )
          {
            GR::u32     SpecialKeyFlags = 0;
            if ( ShiftPressed() )
            {
              SpecialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
            }
            if ( CtrlPressed() )
            {
              SpecialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
            }
            if ( AltPressed() )
            {
              SpecialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
            }

            if ( m_pVirtualKey[Button].m_EnumIndex )
            {
              SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_KEY_DOWN, m_pVirtualKey[Button].m_EnumIndex, SpecialKeyFlags ) );
            }
            SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_VKEY_DOWN, Button, SpecialKeyFlags ) );
          }
        }
      }
      else if ( m_PollTicks - m_pVirtualKey[Button].m_ButtonDownTicks >= m_KeyboardRepeatDelayTicks )
      {
        m_pVirtualKey[Button].m_ButtonDown = true;
        m_pVirtualKey[Button].m_ButtonDownTicks = m_PollTicks;

        if ( m_InputActive )
        {
          GR::u32     SpecialKeyFlags = 0;
          if ( ShiftPressed() )
          {
            SpecialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
          }
          if ( CtrlPressed() )
          {
            SpecialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
          }
          if ( AltPressed() )
          {
            SpecialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
          }
          if ( m_pVirtualKey[Button].m_EnumIndex )
          {
            SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_KEY_DOWN, m_pVirtualKey[Button].m_EnumIndex, SpecialKeyFlags ) );
          }
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_VKEY_DOWN, Button, SpecialKeyFlags ) );
        }
      }
    }
  }
  else
  {
    if ( m_pVirtualKey[Button].m_Pressed )
    {
      m_pVirtualKey[Button].m_WasPressed              = true;
      m_pVirtualKey[Button].m_WasReleasedWhilePressed = m_pVirtualKey[Button].m_Released;

      if ( m_InputActive )
      {
        std::pair<GR::u32,GR::u32>    pairKey( Button, Xtreme::tInputCommand::CMD_UP );

        tMapKeyHandler::iterator      itKeyHandler( m_KeyHandler.find( pairKey ) );
        if ( itKeyHandler != m_KeyHandler.end() )
        {
          itKeyHandler->second();
        }

        GR::u32     SpecialKeyFlags = 0;
        if ( ShiftPressed() )
        {
          SpecialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
        }
        if ( CtrlPressed() )
        {
          SpecialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
        }
        if ( AltPressed() )
        {
          SpecialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
        }
        if ( m_pVirtualKey[Button].m_EnumIndex )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_KEY_UP, m_pVirtualKey[Button].m_EnumIndex, SpecialKeyFlags ) );
        }
        SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_VKEY_UP, Button, SpecialKeyFlags ) );
      }

      m_pVirtualKey[Button].m_Pressed = false;

      if ( m_InputActive )
      {
        GR::String strCommand;
        
        if ( IsCommandByKey( m_pVirtualKey[Button].m_EnumIndex, strCommand.c_str() ) )
        {
          SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_COMMAND, 0, 0, strCommand ) );
        }
      }
    }
    m_pVirtualKey[Button].m_Released = true;
  }
}



GR::u32 XBasicInput::AddDevice( const tInputDevice& Device )
{
  m_Devices.push_back( Device );

  return (GR::u32)( m_Devices.size() - 1 );
}



GR::u32 XBasicInput::AddDeviceControl( const GR::u32 DeviceIndex, const tInputCtrl& Control )
{
  //Log( "Input.Full", "AddDeviceControl %d", DeviceIndex );
  if ( DeviceIndex >= (GR::u32)m_Devices.size() )
  {
    Log( "Input.General", "AddDeviceControl invalid device index %d", DeviceIndex );
    return -1;
  }

  m_Controls.push_back( Control );

  m_Controls.back().m_GlobalIndex = (GR::u32)( m_Controls.size() - 1 );

  m_Devices[DeviceIndex].m_Controls.push_back( Control );

  m_Devices[DeviceIndex].m_Controls.back().m_GlobalIndex         = (GR::u32)( m_Controls.size() - 1 );
  m_Devices[DeviceIndex].m_Controls.back().m_DeviceControlIndex  = (GR::u32)( m_Devices[DeviceIndex].m_Controls.size() - 1 );

  m_Controls.back().m_DeviceControlIndex = (GR::u32)( m_Devices[DeviceIndex].m_Controls.size() - 1 );

  //Log( "Input.Full", "AddDeviceControl done" );

  return m_Controls.back().m_DeviceControlIndex;
}



GR::u32 XBasicInput::AddDeviceAnalogControl( const GR::u32 DeviceIndex, const tInputCtrl& Control )
{
  Log( "Input.Full", "AddDeviceAnalogControl %d", DeviceIndex );
  if ( DeviceIndex >= (GR::u32)m_Devices.size() )
  {
    Log( "Input.General", "AddDeviceAnalogControl invalid device index %d", DeviceIndex );
    return -1;
  }

  m_AnalogControls.push_back( Control );
  m_AnalogControls.back().m_GlobalIndex = (GR::u32)( m_AnalogControls.size() - 1 );

  m_Devices[DeviceIndex].m_AnalogControls.push_back( Control );

  m_Devices[DeviceIndex].m_AnalogControls.back().m_GlobalIndex         = (GR::u32)( m_AnalogControls.size() - 1 );
  m_Devices[DeviceIndex].m_AnalogControls.back().m_DeviceControlIndex  = (GR::u32)( m_Devices[DeviceIndex].m_AnalogControls.size() - 1 );

  m_AnalogControls.back().m_DeviceControlIndex = (GR::u32)( m_Devices[DeviceIndex].m_AnalogControls.size() - 1 );

  Log( "Input.Full", "AddDeviceAnalogControl done" );

  return m_AnalogControls.back().m_DeviceControlIndex;
}



bool XBasicInput::IsInitialized() const
{
  return m_Initialized;
}



bool XBasicInput::ShiftPressed() const
{
  return ( KeyPressed( Xtreme::KEY_LSHIFT ) | KeyPressed( Xtreme::KEY_RSHIFT ) ) != 0;
}



bool XBasicInput::CtrlPressed() const
{
  return ( KeyPressed( Xtreme::KEY_LCONTROL ) | KeyPressed( Xtreme::KEY_RCONTROL ) ) != 0;
}



bool XBasicInput::AltPressed() const
{
  return ( KeyPressed( Xtreme::KEY_LALT ) | KeyPressed( Xtreme::KEY_RALT ) ) != 0;
}



const GR::Char* XBasicInput::GetControlName( GR::u32 Key ) const
{
  if ( Key >= m_Controls.size() )
  {
    return "";
  }
  return m_Controls[Key].m_Name.c_str();
}



const GR::Char* XBasicInput::GetKeyName( GR::u32 Key ) const
{
  if ( Key >= m_VirtualKeys )
  {
    return "";
  }
  return m_pVirtualKey[Key].m_Name.c_str();
}



GR::u32 XBasicInput::GetControlType( GR::u32 Key ) const
{
  if ( Key >= m_Controls.size() )
  {
    return 0;
  }
  return m_Controls[Key].m_Type;
}



GR::u32 XBasicInput::GetControlCount() const
{
  return m_VirtualKeys;
}



int XBasicInput::GetControlDevice( GR::u32 Key ) const
{
  if ( Key >= m_VirtualKeys )
  {
    return 0;
  }
  return m_Controls[m_pVirtualKey[Key].m_GlobalIndex].m_Device;
}



const GR::Char* XBasicInput::GetDeviceName( GR::u32 Device ) const
{
  if ( Device >= m_Devices.size() )
  {
    return "";
  }
  return m_Devices[Device].m_Device.c_str();
}



GR::u32 XBasicInput::PressedVKey() const
{
  if ( !m_Initialized )
  {
    return Xtreme::KEY_INVALID;
  }

  for ( GR::u32 i = 1; i < m_VirtualKeys; i++ )
  {
    if ( VKeyPressed( i ) )
    {
      return i;
    }
  }
  return Xtreme::KEY_INVALID;
}



int XBasicInput::MouseX() const
{
  if ( !m_Initialized )
  {
    return 0;
  }
  return m_InputWMMouseX;
}



int XBasicInput::MouseY() const
{
  if ( !m_Initialized )
  {
    return 0;
  }
  return m_InputWMMouseY;
}



GR::tPoint XBasicInput::MousePos() const
{
  if ( !m_Initialized )
  {
    return GR::tPoint();
  }
  return GR::tPoint( m_InputWMMouseX, m_InputWMMouseY );
}



GR::u32 XBasicInput::MouseButton() const
{
  GR::u32         dummy = 0;

  if ( KeyPressed( Xtreme::MOUSE_BUTTON_1 ) )
  {
    dummy |= 1;
  }
  if ( KeyPressed( Xtreme::MOUSE_BUTTON_2 ) )
  {
    dummy |= 2;
  }
  if ( KeyPressed( Xtreme::MOUSE_BUTTON_3 ) )
  {
    dummy |= 4;
  }
  return dummy;
}



void XBasicInput::ClearMouseFlags()
{
  m_pVirtualKey[Xtreme::MOUSE_BUTTON_1].m_Pressed = false;
  m_pVirtualKey[Xtreme::MOUSE_BUTTON_2].m_Pressed = false;
  m_pVirtualKey[Xtreme::MOUSE_BUTTON_3].m_Pressed = false;
}



GR::u32 XBasicInput::GetVirtualCode( GR::u32 Nr ) const
{
  if ( Nr >= 240 )
  {
    return 0;
  }
  return m_VirtualCode[Nr];
}




bool XBasicInput::AddHandler( Xtreme::tKeyHandlerFunction Function, const GR::u32 Key, GR::u32 Flags )
{
  m_KeyHandler[std::make_pair( Key, Flags )] = Function;

  return true;
}



bool XBasicInput::RemoveHandler( Xtreme::tKeyHandlerFunction Function )
{
  tMapKeyHandler::iterator    it( m_KeyHandler.begin() );
  while ( it != m_KeyHandler.end() )
  {
    if ( it->second == Function )
    {
      m_KeyHandler.erase( it );
      return true;
    }

    ++it;
  }

  return true;
}



void XBasicInput::LogDirect( const GR::String& System, const GR::String& Text )
{
  if ( m_pDebugger )
  {
    m_pDebugger->LogDirect( System.c_str(), Text.c_str() );
  }
}



void XBasicInput::Log( const GR::String& System, const char* Format, ... )
{
  if ( m_pDebugger )
  {
    static char    miscBuffer[5000];

#if ( OPERATING_SYSTEM == OS_WINDOWS )
    vsprintf_s( miscBuffer, 5000, Format, (char *)( &Format + 1 ) );

    m_pDebugger->LogDirect( System.c_str(), miscBuffer );
#elif ( OPERATING_SYSTEM == OS_WEB )
    // TODO
#else
    va_list args;
    va_start( args, Format );

    vsprintf( miscBuffer, Format, args );

    va_end( args );

    __android_log_print( ANDROID_LOG_INFO, "GRGames", miscBuffer );
#endif
  }
}



const GR::Char* XBasicInput::GetAnalogControlName( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return "Invalid Index";
  }
  return m_AnalogControls[Key].m_Name.c_str();
}



Xtreme::ControlType XBasicInput::GetAnalogControlType( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return Xtreme::CT_UNKNOWN;
  }
  return m_AnalogControls[Key].m_Type;
}



int XBasicInput::GetAnalogControlDevice( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return -1;
  }
  return m_AnalogControls[Key].m_Device;
}



int XBasicInput::GetAnalogControlValue( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return 0;
  }
  return m_AnalogControls[Key].m_Value;
}



int XBasicInput::GetAnalogControlMinValue( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return 0;
  }
  return m_AnalogControls[Key].m_Min;
}



int XBasicInput::GetAnalogControlMaxValue( GR::u32 Key ) const
{
  if ( Key >= (GR::u32)m_AnalogControls.size() )
  {
    return 0;
  }
  return m_AnalogControls[Key].m_Max;
}



GR::u32 XBasicInput::GetAnalogControlCount() const
{
  return (GR::u32)m_AnalogControls.size();
}



void XBasicInput::FrameCompleted()
{
  if ( !m_Initialized )
  {
    return;
  }
  for ( GR::u32 i = 0; i < m_VirtualKeys; ++i )
  {
    m_pVirtualKey[i].m_WasPressed               = false;
    m_pVirtualKey[i].m_WasReleasedWhilePressed  = false;
  }
}