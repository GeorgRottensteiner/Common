#include "AndroidInput.h"

#include <debug/debugclient.h>

#include <Xtreme/Environment/XWindow.h>
#include <MasterFrame/XFrameApp.h>

#include <Xtreme/Environment/AndroidMain.h>



char AndroidInput::m_KeyName[240][25] =
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



AndroidInput::AndroidInput() :
  m_pFrameApp( NULL ),
  m_GamepadsChanged( false ),
  m_AnalogJoystickThreshold( 0.25 ),
  m_pSensorManager( NULL ),
  m_pAccelerometerSensor( NULL ),
  m_pSensorEventQueue( NULL ),
  m_pLooper( NULL )
{
  m_GamepadsChanged = false;
}


AndroidInput::~AndroidInput()
{ 
}



bool AndroidInput::Initialize( GR::IEnvironment& Environment )
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

  m_pFrameApp = (XFrameApp*)Environment.Service( "Application" );

  m_pMediator = (AndroidMediator*)m_pFrameApp->GetVarUP( "AndroidMediator" );

  Environment.SetService( "Input", this );

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

  // Prepare to monitor accelerometer
  m_pSensorManager        = ASensorManager_getInstance();
  m_pAccelerometerSensor  = ASensorManager_getDefaultSensor( m_pSensorManager, ASENSOR_TYPE_ACCELEROMETER );
  m_pSensorEventQueue     = ASensorManager_createEventQueue( m_pSensorManager, m_pMediator->looper, LOOPER_ID_USER, NULL, NULL );

  m_Initialized = true;
  return true;
}



bool AndroidInput::Release()
{
  if ( m_pEnvironment != NULL )
  {
    m_pEnvironment->RemoveService( "Input" );
  }

  return XBasicInput::Release();
}



bool AndroidInput::IsTouchAvailable()
{
  return false;
}



void AndroidInput::Init()
{
}



void AndroidInput::Exit()
{
}



bool AndroidInput::HasDeviceForceFeedback( GR::u32 Device ) const
{
  return true;
}



GR::u32 AndroidInput::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  return 0;
}



bool AndroidInput::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  return true;
}



// und die elementare Poll-Funktion
void AndroidInput::Poll()
{
  if ( m_pAccelerometerSensor != NULL )
  {
    ASensorEvent event;
    while ( ASensorEventQueue_getEvents( m_pSensorEventQueue, &event, 1 ) > 0 )
    {
      dh::Log( "accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z );
    }
  }
}



void AndroidInput::Update( const float ElapsedTime )
{
  FrameCompleted();
}



void AndroidInput::UpdateMouse( int X, int Y, int Buttons )
{
  m_InputWMMouseX = X;
  m_InputWMMouseY = Y;

  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_1 ), !!( Buttons & 1 ) );
  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_2 ), !!( Buttons & 2 ) );
  SetButtonPressed( MapKeyToVKey( Xtreme::MOUSE_BUTTON_3 ), !!( Buttons & 4 ) );



  Xtreme::tInputEvent keyEvent( Xtreme::tInputEvent::IE_MOUSE_UPDATE,
    ( X << 16 ) + Y,
    Buttons );

  SendEvent( keyEvent );
}



void AndroidInput::SetActive( bool Active )
{
  m_InputActive = Active;

  if ( m_InputActive )
  {
    if ( m_pAccelerometerSensor != NULL )
    {
      ASensorEventQueue_enableSensor( m_pSensorEventQueue, m_pAccelerometerSensor );

      // We'd like to get 60 events per second (in us).
      ASensorEventQueue_setEventRate( m_pSensorEventQueue, m_pAccelerometerSensor, ( 1000L / 60 ) * 1000 );
    }
  }
  else
  {
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if ( m_pAccelerometerSensor != NULL )
    {
      ASensorEventQueue_disableSensor( m_pSensorEventQueue, m_pAccelerometerSensor );
    }
  }
}



