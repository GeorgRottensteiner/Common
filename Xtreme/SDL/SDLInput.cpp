#include "SDLInput.h"

#include <SDL.h>

#include <debug/debugclient.h>

#include <Xtreme/Environment/XWindow.h>
#include <MasterFrame/XFrameApp.h>



char SDLInput::m_KeyName[240][25] =
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
"\xDC", // Ü
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
"\xD6", // Ö
"\xC4", // Ä
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



SDLInput::SDLInput()
{
}


SDLInput::~SDLInput()
{ 
}



bool SDLInput::Initialize( GR::IEnvironment& Environment )
{
  if ( !XBasicInput::Initialize( Environment ) )
  {
    return false;
  }

  auto pWindowService = (Xtreme::IAppWindow*)Environment.Service( "Window" );
  if ( pWindowService == NULL )
  {
    dh::Log( "No Window service found" );
    return false;
  }

  // Pseudo-Devices und Virtual-Keys erzeugen
  m_VirtualKeys = Xtreme::DEFKEY_LAST_ENTRY;
  m_pVirtualKey = new tVirtualKey[m_VirtualKeys];

  // das 0. gibt es nicht
  m_pVirtualKey[0].m_Name = "No Key";
  m_pVirtualKey[0].m_GlobalIndex = 0;
  m_pVirtualKey[0].m_EnumIndex = Xtreme::KEY_INVALID;

  m_Controls.reserve( Xtreme::DEFKEY_LAST_ENTRY );
  m_Controls.push_back( tInputCtrl() );

  m_Devices.push_back( tInputDevice( "Keyboard" ) );

  for ( int i = Xtreme::FIRST_KEYBOARD_KEY; i <= Xtreme::LAST_KEYBOARD_KEY; ++i )
  {
    tInputCtrl      NewCtrl( 0, m_KeyName[i] );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    m_pVirtualKey[i].m_Name = m_KeyName[i];
    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex = ( Xtreme::eInputDefaultButtons )i;
    m_pVirtualKey[i].m_Released = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( ( Xtreme::eInputDefaultButtons )i, NewCtrl.m_VirtualIndex ) );
  }

  // Maus
  m_Devices.push_back( tInputDevice( "Mouse" ) );

  for ( int i = Xtreme::MOUSE_LEFT; i <= Xtreme::MOUSE_BUTTON_3; ++i )
  {
    tInputCtrl      NewCtrl( 1, m_KeyName[i] );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );
    m_pVirtualKey[i].m_Name = m_KeyName[i];
    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex = ( Xtreme::eInputDefaultButtons )i;
    m_pVirtualKey[i].m_Released = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( ( Xtreme::eInputDefaultButtons )i, NewCtrl.m_VirtualIndex ) );
  }

  // Joystick
  m_Devices.push_back( tInputDevice( "Joystick" ) );
  GR::String     strDummy = "";
  for ( int i = Xtreme::JOYSTICK_1_LEFT; i <= Xtreme::JOYSTICK_2_BUTTON_32; ++i )
  {
    strDummy = "Joystick ";

    if ( i >= Xtreme::JOYSTICK_2_LEFT )
    {
      strDummy += "2 ";
    }
    else
    {
      strDummy += "1 ";
    }
    switch ( ( i - Xtreme::JOYSTICK_1_LEFT ) % 36 )
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
        strDummy += CMisc::printf( "Button %d", ( i - Xtreme::JOYSTICK_1_LEFT ) % 36 - 4 );
    }

    tInputCtrl      NewCtrl( 2, strDummy );

    NewCtrl.m_GlobalIndex = (int)m_Controls.size();
    NewCtrl.m_VirtualIndex = NewCtrl.m_GlobalIndex;

    m_Controls.push_back( NewCtrl );

    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex = ( Xtreme::eInputDefaultButtons )i;
    m_pVirtualKey[i].m_Name = strDummy;
    m_pVirtualKey[i].m_Released = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( ( Xtreme::eInputDefaultButtons )i, NewCtrl.m_VirtualIndex ) );
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
    m_pVirtualKey[i].m_Name = strKeyName;
    m_pVirtualKey[i].m_GlobalIndex = NewCtrl.m_GlobalIndex;
    m_pVirtualKey[i].m_EnumIndex = ( Xtreme::eInputDefaultButtons )i;
    m_pVirtualKey[i].m_Released = true;
    m_pVirtualKey[i].m_DeviceControlIndex = (int)m_Controls.size() - 1;

    m_DefaultKeys.insert( std::make_pair( ( Xtreme::eInputDefaultButtons )i, NewCtrl.m_VirtualIndex ) );
  }

  m_Initialized = true;
  return true;
}



bool SDLInput::Release()
{
  m_pEnvironment->RemoveService( "Input" );

  return XBasicInput::Release();
}



void SDLInput::Init()
{
}



void SDLInput::Exit()
{
}



bool SDLInput::HasDeviceForceFeedback( GR::u32 Device ) const
{
  return true;
}



GR::u32 SDLInput::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  return 0;
}



bool SDLInput::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  return true;
}



// und die elementare Poll-Funktion
void SDLInput::Poll()
{
}



void SDLInput::Update( const float ElapsedTime )
{

}



void SDLInput::UpdateMouse( int X, int Y, int Buttons )
{
  m_InputWMMouseX = X;
  m_InputWMMouseY = Y;

  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_1 ), !!( Buttons & SDL_BUTTON_LMASK ) );
  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_2 ), !!( Buttons & SDL_BUTTON_RMASK ) );
  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_3 ), !!( Buttons & SDL_BUTTON_MMASK ) );



  Xtreme::tInputEvent keyEvent( Xtreme::tInputEvent::IE_MOUSE_UPDATE,
    ( X << 16 ) + Y,
    Buttons );

  SendEvent( keyEvent );
}



void SDLInput::SetActive( bool Active )
{
  m_InputActive = Active;
}



Xtreme::eInputDefaultButtons SDLInput::MapToKey( SDL_Scancode SDLKey )
{
  switch ( SDLKey )
  {
    case SDL_SCANCODE_0:
      return Xtreme::KEY_0;
    case SDL_SCANCODE_1:
      return Xtreme::KEY_1;
    case SDL_SCANCODE_2:
      return Xtreme::KEY_2;
    case SDL_SCANCODE_3:
      return Xtreme::KEY_3;
    case SDL_SCANCODE_4:
      return Xtreme::KEY_4;
    case SDL_SCANCODE_5:
      return Xtreme::KEY_5;
    case SDL_SCANCODE_6:
      return Xtreme::KEY_6;
    case SDL_SCANCODE_7:
      return Xtreme::KEY_7;
    case SDL_SCANCODE_8:
      return Xtreme::KEY_8;
    case SDL_SCANCODE_9:
      return Xtreme::KEY_9;
    case SDL_SCANCODE_A:
      return Xtreme::KEY_A;
    case SDL_SCANCODE_B:
      return Xtreme::KEY_B;
    case SDL_SCANCODE_C:
      return Xtreme::KEY_C;
    case SDL_SCANCODE_D:
      return Xtreme::KEY_D;
    case SDL_SCANCODE_E:
      return Xtreme::KEY_E;
    case SDL_SCANCODE_F:
      return Xtreme::KEY_F;
    case SDL_SCANCODE_G:
      return Xtreme::KEY_G;
    case SDL_SCANCODE_H:
      return Xtreme::KEY_H;
    case SDL_SCANCODE_I:
      return Xtreme::KEY_I;
    case SDL_SCANCODE_J:
      return Xtreme::KEY_J;
    case SDL_SCANCODE_K:
      return Xtreme::KEY_K;
    case SDL_SCANCODE_L:
      return Xtreme::KEY_L;
    case SDL_SCANCODE_M:
      return Xtreme::KEY_M;
    case SDL_SCANCODE_N:
      return Xtreme::KEY_N;
    case SDL_SCANCODE_O:
      return Xtreme::KEY_O;
    case SDL_SCANCODE_P:
      return Xtreme::KEY_P;
    case SDL_SCANCODE_Q:
      return Xtreme::KEY_Q;
    case SDL_SCANCODE_R:
      return Xtreme::KEY_R;
    case SDL_SCANCODE_S:
      return Xtreme::KEY_S;
    case SDL_SCANCODE_T:
      return Xtreme::KEY_T;
    case SDL_SCANCODE_U:
      return Xtreme::KEY_U;
    case SDL_SCANCODE_V:
      return Xtreme::KEY_V;
    case SDL_SCANCODE_W:
      return Xtreme::KEY_W;
    case SDL_SCANCODE_X:
      return Xtreme::KEY_X;
    case SDL_SCANCODE_Y:
      return Xtreme::KEY_Y;
    case SDL_SCANCODE_Z:
      return Xtreme::KEY_Z;
    case SDL_SCANCODE_SPACE:
      return Xtreme::KEY_SPACE;
    case SDL_SCANCODE_ESCAPE:
      return Xtreme::KEY_ESCAPE;
    case SDL_SCANCODE_KP_PLUS:
      return Xtreme::KEY_NUMPAD_PLUS;
    case SDL_SCANCODE_KP_MINUS:
      return Xtreme::KEY_NUMPAD_MINUS;
    case SDL_SCANCODE_KP_DIVIDE:
      return Xtreme::KEY_NUMPAD_DIVIDE;
    case SDL_SCANCODE_KP_MULTIPLY:
      return Xtreme::KEY_NUMPAD_MULTIPLY;
    case SDL_SCANCODE_KP_0:
      return Xtreme::KEY_NUMPAD_5;
    case SDL_SCANCODE_KP_1:
      return Xtreme::KEY_NUMPAD_1;
    case SDL_SCANCODE_KP_2:
      return Xtreme::KEY_NUMPAD_2;
    case SDL_SCANCODE_KP_3:
      return Xtreme::KEY_NUMPAD_3;
    case SDL_SCANCODE_KP_4:
      return Xtreme::KEY_NUMPAD_4;
    case SDL_SCANCODE_KP_5:
      return Xtreme::KEY_NUMPAD_5;
    case SDL_SCANCODE_KP_6:
      return Xtreme::KEY_NUMPAD_6;
    case SDL_SCANCODE_KP_7:
      return Xtreme::KEY_NUMPAD_7;
    case SDL_SCANCODE_KP_8:
      return Xtreme::KEY_NUMPAD_8;
    case SDL_SCANCODE_KP_9:
      return Xtreme::KEY_NUMPAD_9;
    case SDL_SCANCODE_KP_ENTER:
      return Xtreme::KEY_NUMPAD_ENTER;
    case SDL_SCANCODE_KP_COMMA:
      return Xtreme::KEY_NUMPAD_KOMMA;
    case SDL_SCANCODE_LCTRL:
      return Xtreme::KEY_LCONTROL;
    case SDL_SCANCODE_UP:
      return Xtreme::KEY_UP;
    case SDL_SCANCODE_DOWN:
      return Xtreme::KEY_DOWN;
    case SDL_SCANCODE_LEFT:
      return Xtreme::KEY_LEFT;
    case SDL_SCANCODE_RIGHT:
      return Xtreme::KEY_RIGHT;
    case SDL_SCANCODE_F1:
      return Xtreme::KEY_F1;
    case SDL_SCANCODE_F2:
      return Xtreme::KEY_F2;
    case SDL_SCANCODE_F3:
      return Xtreme::KEY_F3;
    case SDL_SCANCODE_F4:
      return Xtreme::KEY_F4;
    case SDL_SCANCODE_F5:
      return Xtreme::KEY_F5;
    case SDL_SCANCODE_F6:
      return Xtreme::KEY_F6;
    case SDL_SCANCODE_F7:
      return Xtreme::KEY_F7;
    case SDL_SCANCODE_F8:
      return Xtreme::KEY_F8;
    case SDL_SCANCODE_F9:
      return Xtreme::KEY_F9;
    case SDL_SCANCODE_F10:
      return Xtreme::KEY_F10;
    case SDL_SCANCODE_F11:
      return Xtreme::KEY_F11;
    case SDL_SCANCODE_F12:
      return Xtreme::KEY_F12;
    case SDL_SCANCODE_RETURN:
      return Xtreme::KEY_ENTER;
    case SDL_SCANCODE_NONUSBACKSLASH:
      return Xtreme::KEY_GREATER;
    case SDL_SCANCODE_CLEAR:
      return Xtreme::KEY_NUMPAD_5;
    case SDL_SCANCODE_LSHIFT:
      return Xtreme::KEY_LSHIFT;
    case SDL_SCANCODE_RSHIFT:
      return Xtreme::KEY_RSHIFT;
    case SDL_SCANCODE_LALT:
      return Xtreme::KEY_LALT;
    case SDL_SCANCODE_RALT:
      return Xtreme::KEY_RALT;
    case SDL_SCANCODE_RCTRL:
      return Xtreme::KEY_RCONTROL;
    default:
      break;
  }
  dh::Log( "SDLInput::MapToKey, unsupported SDL_SCANCODE %d", SDLKey );
  return Xtreme::KEY_INVALID;
}



void SDLInput::OnSDLEvent( const SDL_Event& Event )
{
  switch ( Event.type )
  {
    case SDL_KEYDOWN:
      SetButtonPressed( MapKeyToVKey( MapToKey( Event.key.keysym.scancode ) ), true );
      break;
    case SDL_KEYUP:
      SetButtonPressed( MapKeyToVKey( MapToKey( Event.key.keysym.scancode ) ), false );
      break;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
      UpdateMouse( Event.button.x, Event.button.y, SDL_GetMouseState( NULL, NULL ) );
      break;
    case SDL_MOUSEMOTION:
      UpdateMouse( Event.motion.x, Event.motion.y, Event.motion.state );
      break;
  }
}