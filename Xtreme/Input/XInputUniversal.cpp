#include "XInputUniversal.h"

#include <debug/debugclient.h>

#include <Xtreme/Environment/XWindow.h>
#include <MasterFrame/XFrameApp.h>

#include <Xtreme/Environment/XWindowUniversalApp.h>



XInputUniversal*      s_pInstance = NULL;


#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
using namespace Windows::Gaming::Input;
#endif

GR::String    s_InputLogString;



ref class XInputCLRMediator sealed
{
  private:

  public:




    XInputCLRMediator()
    {

    }

    void XInputCLRMediator::OnKeyDown( Windows::UI::Core::CoreWindow^ Sender, Windows::UI::Core::KeyEventArgs^ Args )
    {
      //dh::Log( "ScanKey %d  Virtual Key %d  Ext %d", (int)Args->KeyStatus.ScanCode, (int)Args->VirtualKey, Args->KeyStatus.IsExtendedKey  );
      GR::u32     specialKeyFlags = 0;
      /*
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
      }*/
      //s_pInstance->SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_WIN_KEY_DOWN, (int)Args->VirtualKey, specialKeyFlags ) );

      // TODO - not VKey, but its xtreme key counter part
      //Xtreme::tInputEvent keyEvent( Xtreme::tInputEvent::IE_KEY_DOWN, (int)Args->KeyStatus.ScanCode );

      //s_pInstance->SetButtonPressed( (Xtreme::eInputDefaultButtons)s_pInstance->m_VirtualCode[(int)Args->VirtualKey], true );
      if ( s_pInstance )
      {
        s_pInstance->SetButtonPressed( ( Xtreme::eInputDefaultButtons )s_pInstance->m_VirtualCode[(int)Args->VirtualKey], true );
      }
      //s_pInstance->SendEvent( keyEvent );
    }



    void XInputCLRMediator::OnKeyUp( Windows::UI::Core::CoreWindow^ Sender, Windows::UI::Core::KeyEventArgs^ Args )
    {
      GR::u32     specialKeyFlags = 0;
      /*
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
      }*/
      //s_pInstance->SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_WIN_KEY_UP, (int)Args->VirtualKey, specialKeyFlags ) );

      // TODO - not VKey, but its xtreme key counter part
      Xtreme::tInputEvent keyEvent( Xtreme::tInputEvent::IE_KEY_UP, (int)Args->VirtualKey );

      if ( s_pInstance )
      {
        s_pInstance->SetButtonPressed( ( Xtreme::eInputDefaultButtons )s_pInstance->m_VirtualCode[(int)Args->VirtualKey], false );
        s_pInstance->SendEvent( keyEvent );
      }
    }



    void XInputCLRMediator::OnCharacterReceived( Windows::UI::Core::CoreWindow^ Sender, Windows::UI::Core::CharacterReceivedEventArgs^ Args )
    {
      //dh::Log( "Key %d", (int)Args->KeyCode );
      //GR::String   info = Misc::Format( "Key %1%" ) << Args->KeyCode;

      //OutputDebugStringA( info.c_str() );

      GR::u32     specialKeyFlags = 0;

      if ( ( s_pInstance )
      &&   ( Args != nullptr ) )
      {
        if ( s_pInstance->ShiftPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_SHIFT_PUSHED;
        }
        if ( s_pInstance->CtrlPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_CTRL_PUSHED;
        }
        if ( s_pInstance->AltPressed() )
        {
          specialKeyFlags |= Xtreme::tInputEvent::KF_ALT_PUSHED;
        }
        s_pInstance->SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_CHAR_ENTERED,
                                                     s_pInstance->m_VirtualCode[(int)Args->KeyCode],
                                                     (int)Args->KeyCode,
                                                     specialKeyFlags ) );
      }
    }



    void XInputCLRMediator::OnPointerMoved( Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs ^args )
    {
      auto coreWindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
      if ( coreWindow == nullptr )
      {
        return;
      }

      auto bounds = coreWindow->Bounds;

      if ( s_pInstance )
      {
        int   trueX = (int)( args->CurrentPoint->Position.X * s_pInstance->m_pFrameApp->m_RenderFrame.Width() / bounds.Width );
        int   trueY = (int)( args->CurrentPoint->Position.Y * s_pInstance->m_pFrameApp->m_RenderFrame.Height() / bounds.Height );
        //int   mouseButtons = args->CurrentPoint->IsInContact ? 1 : 0;

        int mouseButtons = 0;
        if ( args->CurrentPoint->Properties->IsLeftButtonPressed )
        {
          mouseButtons |= 1;
        }
        if ( args->CurrentPoint->Properties->IsRightButtonPressed )
        {
          mouseButtons |= 2;
        }
        if ( args->CurrentPoint->Properties->IsMiddleButtonPressed )
        {
          mouseButtons |= 4;
        }

        int wheelDelta = args->CurrentPoint->Properties->MouseWheelDelta;
        if ( wheelDelta != 0 )
        {
          wheelDelta /= 120;
          while ( wheelDelta < 0 )
          {
            s_pInstance->SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_MOUSEWHEEL_DOWN, ( trueX << 16 ) + trueY, mouseButtons ) );

            std::pair<GR::u32, GR::u32>    pairKey( s_pInstance->MapKeyToVKey( Xtreme::MOUSE_WHEEL_DOWN ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );
            XBasicInput::tMapKeyHandler::iterator      itKeyHandler( s_pInstance->m_KeyHandler.find( pairKey ) );
            if ( itKeyHandler != s_pInstance->m_KeyHandler.end() )
            {
              itKeyHandler->second();
            }

            wheelDelta++;
          }
          while ( wheelDelta > 0 )
          {
            s_pInstance->SendEvent( Xtreme::tInputEvent( Xtreme::tInputEvent::IE_MOUSEWHEEL_UP, ( trueX << 16 ) + trueY, mouseButtons ) );

            std::pair<GR::u32, GR::u32>    pairKey( s_pInstance->MapKeyToVKey( Xtreme::MOUSE_WHEEL_UP ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );
            XBasicInput::tMapKeyHandler::iterator      itKeyHandler( s_pInstance->m_KeyHandler.find( pairKey ) );
            if ( itKeyHandler != s_pInstance->m_KeyHandler.end() )
            {
              itKeyHandler->second();
            }

            wheelDelta--;
          }
        }

        s_pInstance->UpdateMouse( trueX, trueY, mouseButtons );
      }
    }



#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
    void XInputCLRMediator::OnGamepadChanged( _In_ Platform::Object^ sender, _In_ Gamepad^ gamepad )
    {
      if ( s_pInstance )
      {
        s_pInstance->OnGamepadChanged();
      }
    }
#endif



	  void XInputCLRMediator::OnReadingChanged( Windows::Devices::Sensors::Gyrometer^ sender, Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^ e )
	  {
      if ( s_pInstance )
      {
        s_pInstance->OnGyrometerReadingChanged( e->Reading->AngularVelocityX, e->Reading->AngularVelocityY, e->Reading->AngularVelocityZ );
      }
	  }

    void XInputCLRMediator::OnOrientationReadingChanged( Windows::Devices::Sensors::OrientationSensor^ sender, Windows::Devices::Sensors::OrientationSensorReadingChangedEventArgs^ e )
    {
      if ( s_pInstance )
      {
        s_pInstance->OnOrientationReadingChanged( e->Reading->Quaternion, e->Reading->RotationMatrix );
      }
    }



#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
    void XInputCLRMediator::OnBackRequested( Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ args )
    {
      args->Handled = true;

      if ( s_pInstance )
      {
        std::pair<GR::u32, GR::u32>    pairKey( s_pInstance->MapKeyToVKey( Xtreme::KEY_ESCAPE ), Xtreme::tInputCommand::CMD_RELEASED_DOWN );

        Xtreme::tInputEvent keyEvent( Xtreme::tInputEvent::IE_KEY_DOWN, s_pInstance->MapKeyToVKey( Xtreme::KEY_ESCAPE ) );

        s_pInstance->SetButtonPressed( Xtreme::KEY_ESCAPE, true );
        s_pInstance->SendEvent( keyEvent );

        Xtreme::tInputEvent keyEventUp( Xtreme::tInputEvent::IE_KEY_UP, s_pInstance->MapKeyToVKey( Xtreme::KEY_ESCAPE ) );

        s_pInstance->SetButtonPressed( Xtreme::KEY_ESCAPE, false );
        s_pInstance->SendEvent( keyEventUp );
      }
    }
#endif

};

XInputCLRMediator^        s_Mediator;



char XInputUniversal::m_KeyName[240][25] =
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



XInputUniversal::XInputUniversal() :
  m_pFrameApp( NULL ),
  m_GamepadsChanged( false ),
  m_AnalogJoystickThreshold( 0.25 ),
  m_Gyrometer( nullptr )
{
  s_pInstance = this;
  m_GamepadsChanged = false;
}


XInputUniversal::~XInputUniversal()
{
  s_pInstance = NULL;
}



bool XInputUniversal::Initialize( GR::IEnvironment& Environment )
{
  if ( !XBasicInput::Initialize( Environment ) )
  {
    return false;
  }

  RotationInfo = "No Rotation";
  auto pWindowService = (Xtreme::IAppWindow*)Environment.Service( "Window" );
  if ( pWindowService == NULL )
  {
    dh::Log( "No Window service found" );
    return false;
  }

  m_pFrameApp = (XFrameApp*)Environment.Service( "Application" );

  Environment.SetService( "Input", this );

  s_Mediator = ref new XInputCLRMediator();

  Xtreme::UniversalAppWindow*  pWnd = (Xtreme::UniversalAppWindow*)pWindowService;

  auto coreWindow = pWnd->CoreWindow;// Windows::UI::Core::CoreWindow::GetForCurrentThread();

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

  m_Gyrometer = Windows::Devices::Sensors::Gyrometer::GetDefault();
  if ( m_Gyrometer != nullptr )
  {
    // Establish the report interval
    unsigned int minReportInterval = m_Gyrometer->MinimumReportInterval;
    unsigned int reportInterval = minReportInterval > 16 ? minReportInterval : 16;
    m_Gyrometer->ReportInterval = reportInterval;

    // Assign an event handler for the reading-changed event
    m_Gyrometer->ReadingChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Sensors::Gyrometer^, Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^>( s_Mediator, &XInputCLRMediator::OnReadingChanged );
  }

  m_Sensor = Windows::Devices::Sensors::OrientationSensor::GetDefault();
  if ( m_Sensor != nullptr )
  {
    unsigned int minReportInterval = m_Sensor->MinimumReportInterval;
    unsigned int reportInterval = minReportInterval > 16 ? minReportInterval : 16;
    m_Sensor->ReportInterval = reportInterval;

    m_Sensor->ReadingChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Sensors::OrientationSensor^, Windows::Devices::Sensors::OrientationSensorReadingChangedEventArgs^>( s_Mediator, &XInputCLRMediator::OnOrientationReadingChanged );
  }

#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
  Windows::UI::Core::SystemNavigationManager::GetForCurrentView()->AppViewBackButtonVisibility = Windows::UI::Core::AppViewBackButtonVisibility::Visible;
  Windows::UI::Core::SystemNavigationManager::GetForCurrentView()->BackRequested += ref new Windows::Foundation::EventHandler<Windows::UI::Core::BackRequestedEventArgs^>( s_Mediator, &XInputCLRMediator::OnBackRequested );
#endif

  // Detect gamepad connection and disconnection events.
#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
  Gamepad::GamepadAdded += ref new Windows::Foundation::EventHandler<Gamepad^>( s_Mediator, &XInputCLRMediator::OnGamepadChanged );
  Gamepad::GamepadRemoved += ref new Windows::Foundation::EventHandler<Gamepad^>( s_Mediator, &XInputCLRMediator::OnGamepadChanged );
#endif

  // event handlers last, so everything else is initialised. They seem to trigger right away
  m_KeyDownToken              = coreWindow->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>( s_Mediator, &XInputCLRMediator::OnKeyDown );
  m_KeyUpToken                = coreWindow->KeyUp += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>( s_Mediator, &XInputCLRMediator::OnKeyUp );
  m_OnCharToken               = coreWindow->CharacterReceived += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::CharacterReceivedEventArgs^>( s_Mediator, &XInputCLRMediator::OnCharacterReceived );
  m_PointerMovedToken         = coreWindow->PointerMoved += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>( s_Mediator, &XInputCLRMediator::OnPointerMoved );
  m_PointerPressedToken       = coreWindow->PointerPressed += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>( s_Mediator, &XInputCLRMediator::OnPointerMoved );
  m_PointerReleasedToken      = coreWindow->PointerReleased += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>( s_Mediator, &XInputCLRMediator::OnPointerMoved );
  m_PointerWheelChangedToken  = coreWindow->PointerWheelChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::PointerEventArgs ^>( s_Mediator, &XInputCLRMediator::OnPointerMoved );


#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
  auto gamepads = Gamepad::Gamepads;

  for ( int i = 0; i < (int)gamepads->Size; ++i )
  {
    m_GamePads.push_back( gamepads->GetAt( i ) );
  }
#endif

  m_Initialized = true;
  return true;
}



bool XInputUniversal::Release()
{
  auto coreWindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();

  coreWindow->KeyDown             -= m_KeyDownToken;
  coreWindow->KeyUp               -= m_KeyUpToken;
  coreWindow->CharacterReceived   -= m_OnCharToken;
  coreWindow->PointerMoved        -= m_PointerMovedToken;
  coreWindow->PointerPressed      -= m_PointerPressedToken;
  coreWindow->PointerReleased     -= m_PointerReleasedToken;
  coreWindow->PointerWheelChanged -= m_PointerWheelChangedToken;

  m_pEnvironment->RemoveService( "Input" );

  return XBasicInput::Release();
}



double    gyroX = 0.0;
double    gyroY = 0.0;
double    gyroZ = 0.0;

void XInputUniversal::OnGyrometerReadingChanged( double X, double Y, double Z )
{
  gyroX = X;
  gyroY = Y;
  gyroZ = Z;
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_LEFT ) ), ( X < -22 ) );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_RIGHT ) ), ( X > 22 ) );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_UP ) ), ( Y < -22 ) );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_DOWN ) ), ( Y > 22 ) );
}



bool XInputUniversal::IsTouchAvailable()
{
  auto devices = Windows::Devices::Input::PointerDevice::GetPointerDevices();

  if ( devices == nullptr )
  {
    return false;
  }
  unsigned int   numDevices = devices->Size;
  for ( unsigned int i = 0; i < numDevices; ++i )
  {
    auto device = devices->GetAt( i );
    if ( device == nullptr )
    {
      continue;
    }
    if ( device->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Touch )
    {
      return true;
    }
  }
  return false;
}



void XInputUniversal::OnOrientationReadingChanged( Windows::Devices::Sensors::SensorQuaternion^ Quat, Windows::Devices::Sensors::SensorRotationMatrix^ Matrix )
{
  math::matrix4     mat;

  mat.ms._11 = Matrix->M11;
  mat.ms._21 = Matrix->M21;
  mat.ms._31 = Matrix->M31;
  mat.ms._41 = 0.0f;
  mat.ms._12 = Matrix->M12;
  mat.ms._22 = Matrix->M22;
  mat.ms._32 = Matrix->M32;
  mat.ms._42 = 0.0f;
  mat.ms._13 = Matrix->M13;
  mat.ms._23 = Matrix->M23;
  mat.ms._33 = Matrix->M33;
  mat.ms._43 = 0.0f;
  mat.ms._14 = 0.0f;
  mat.ms._24 = 0.0f;
  mat.ms._34 = 0.0f;
  mat.ms._44 = 0.0f;

  GR::f32     a, b, c;

  mat.ToEulerAngles( a, b, c );

  int     gyroSensitivity = 8;

  bool    left = ( a > gyroSensitivity ) && ( a < 180 );
  bool    right = ( a > 180 ) && ( a < 360 - gyroSensitivity );
  bool    down = ( b > gyroSensitivity ) && ( b < 180 );
  bool    up = ( b > 180 ) && ( b < 360 - gyroSensitivity );

  //RotationInfo = Misc::Format( "Orientation: %1:6%,%2:6%,%3:6% - " ) << a << b << c;

  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_LEFT ) ), left );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_RIGHT ) ), right );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_UP ) ), up );
  SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( Xtreme::JOYSTICK_1_DOWN ) ), down );
}


GR::String XInputUniversal::GyroInfo()
{
  GR::String     info;

  if ( m_Gyrometer == nullptr )
  {
    info = "No Gyro found";
  }
  else
  {
    info = Misc::Format( "Gyro available, last reading %1:4%,%2:4%,%3:4%" ) << gyroX << gyroY << gyroZ;
  }

  info += RotationInfo;
  return info;
}



void XInputUniversal::OnGamepadChanged()
{
#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
  m_GamepadsChanged = true;

  auto gamepads = Gamepad::Gamepads;

  for ( unsigned int gpIndex = 0; gpIndex < gamepads->Size; ++gpIndex )
  {
    bool    wasConnectedAlready = false;
    for ( size_t j = 0; j < m_GamePads.size(); ++j )
    {
      if ( m_GamePads[j] == gamepads->GetAt( gpIndex ) )
      {
        wasConnectedAlready = true;
        break;
      }
    }
    if ( wasConnectedAlready )
    {
      continue;
    }

    // a new game pad was detected
    //m_Devices.push_back( tInputDevice( "Joystick" ) );
    m_GamePads.push_back( gamepads->GetAt( gpIndex ) );

    // TODO - only add control info if not already added
    /*
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
    }*/
  }
#endif
}



void XInputUniversal::Init()
{
}



void XInputUniversal::Exit()
{
}



bool XInputUniversal::HasDeviceForceFeedback( GR::u32 Device ) const
{
  return true;
}



GR::u32 XInputUniversal::DeviceForceFeedbackAxisCount( GR::u32 Device ) const
{
  return 0;
}



bool XInputUniversal::SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const
{
  return true;
}



// und die elementare Poll-Funktion
void XInputUniversal::Poll()
{
}



void XInputUniversal::Update( const float ElapsedTime )
{
#if OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE
  int     joyBase = 0;
  for each ( auto& gamePad in m_GamePads )
  {
    auto gpReading = gamePad->GetCurrentReading();

    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_1 ) ), ( ( gpReading.Buttons & GamepadButtons::Menu ) == GamepadButtons::Menu ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_2 ) ), ( ( gpReading.Buttons & GamepadButtons::View ) == GamepadButtons::View ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_3 ) ), ( ( gpReading.Buttons & GamepadButtons::A ) == GamepadButtons::A ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_4 ) ), ( ( gpReading.Buttons & GamepadButtons::B ) == GamepadButtons::B ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_5 ) ), ( ( gpReading.Buttons & GamepadButtons::X ) == GamepadButtons::X ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_6 ) ), ( ( gpReading.Buttons & GamepadButtons::Y ) == GamepadButtons::Y ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_7 ) ), ( ( gpReading.Buttons & GamepadButtons::DPadUp ) == GamepadButtons::DPadUp ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_8 ) ), ( ( gpReading.Buttons & GamepadButtons::DPadDown ) == GamepadButtons::DPadDown ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_9 ) ), ( ( gpReading.Buttons & GamepadButtons::DPadLeft ) == GamepadButtons::DPadLeft ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_10 ) ), ( ( gpReading.Buttons & GamepadButtons::DPadRight ) == GamepadButtons::DPadRight ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_11 ) ), ( ( gpReading.Buttons & GamepadButtons::LeftShoulder ) == GamepadButtons::LeftShoulder ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_12 ) ), ( ( gpReading.Buttons & GamepadButtons::RightShoulder ) == GamepadButtons::RightShoulder ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_13 ) ), ( ( gpReading.Buttons & GamepadButtons::LeftThumbstick ) == GamepadButtons::LeftThumbstick ) );
    SetButtonPressed( MapKeyToVKey( (Xtreme::eInputDefaultButtons)( joyBase + Xtreme::JOYSTICK_1_BUTTON_14 ) ), ( ( gpReading.Buttons & GamepadButtons::RightThumbstick ) == GamepadButtons::RightThumbstick ) );

    // TODO - should be two separate axis' !!!
    SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( joyBase + Xtreme::JOYSTICK_1_LEFT ) ),
                                       ( gpReading.LeftThumbstickX < -m_AnalogJoystickThreshold )
                                    || ( gpReading.RightThumbstickX < -m_AnalogJoystickThreshold ) );
    SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( joyBase + Xtreme::JOYSTICK_1_RIGHT ) ),
                                       ( gpReading.LeftThumbstickX > m_AnalogJoystickThreshold )
                                    || ( gpReading.RightThumbstickX > m_AnalogJoystickThreshold ) );
    SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( joyBase + Xtreme::JOYSTICK_1_DOWN ) ),
                                       ( gpReading.LeftThumbstickY < -m_AnalogJoystickThreshold )
                                    || ( gpReading.RightThumbstickY < -m_AnalogJoystickThreshold ) );
    SetButtonPressed( MapKeyToVKey( ( Xtreme::eInputDefaultButtons )( joyBase + Xtreme::JOYSTICK_1_UP ) ),
                                       ( gpReading.LeftThumbstickY > m_AnalogJoystickThreshold )
                                    || ( gpReading.RightThumbstickY > m_AnalogJoystickThreshold ) );

    if ( joyBase == 0 )
    {
      joyBase = Xtreme::JOYSTICK_2_BUTTON_1 - Xtreme::JOYSTICK_1_BUTTON_1;
    }
  }
#endif
  FrameCompleted();
}



void XInputUniversal::UpdateMouse( int X, int Y, int Buttons )
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



void XInputUniversal::SetActive( bool Active )
{
  m_InputActive = Active;
}