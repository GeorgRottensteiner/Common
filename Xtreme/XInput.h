#ifndef XTREME_XINPUT_H
#define XTREME_XINPUT_H


#include <Lang/ITask.h>

#include <Lang/FastDelegate.h>

#include <Interface/IEventProducer.h>
#include <Interface/IService.h>

#include <GR/GRTypes.h>
#include <String/GRstring.h>

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
#include <Windows.h>
#endif



namespace GR
{
  struct IEnvironment;
}

struct IIOStream;

namespace Xtreme
{
  enum ControlType
  {
    CT_UNKNOWN          = 0,
    CT_BUTTON,
    CT_AXIS,
    CT_RELATIVE_AXIS,
    CT_HAT_SWITCH
  };

  struct tInputCommand
  {

    enum eType
    {
      CMD_INVALID = 0,
      CMD_DOWN,
      CMD_UP,
      CMD_RELEASED_DOWN,
    };

    GR::u32       m_Flags,
                  m_Key;

    GR::string    m_Desc;

    eType         m_Type;


    tInputCommand() :
      m_Flags( 0 ),
      m_Key( 0 ),
      m_Desc( "" ),
      m_Type( CMD_INVALID )
    {
    }
  };

  struct tInputEvent
  {

    enum eInputEventType
    {
      IE_INVALID,
      IE_MOUSE_UPDATE,
      IE_CHAR_ENTERED,
      IE_MOUSEWHEEL_UP,
      IE_MOUSEWHEEL_DOWN,
      IE_WIN_KEY_DOWN,
      IE_WIN_KEY_UP,
      IE_KEY_DOWN,
      IE_KEY_UP,
      IE_VKEY_DOWN,
      IE_VKEY_UP,
      IE_COMMAND,
    };

    enum eKeyFlags
    {
      KF_CTRL_PUSHED  = 0x00000001,
      KF_SHIFT_PUSHED = 0x00000002,
      KF_ALT_PUSHED   = 0x00000004,
    };

    eInputEventType           Type;

    GR::up                    Param1,
                              Param2;

    GR::string                Command;



    tInputEvent( eInputEventType eType = IE_INVALID, GR::up Param1 = 0, GR::up Param2 = 0, const GR::String& Command = GR::String() ) :
      Type( eType ),
      Param1( Param1 ),
      Param2( Param2 ),
      Command( Command )
    {
    }
  };


  typedef fastdelegate::FastDelegate1<const tInputEvent&> tEventHandlerFunction;
  typedef fastdelegate::FastDelegate0<>                   tKeyHandlerFunction;

  enum eInputDefaultButtons
  {
    KEY_INVALID = 0,
    FIRST_KEYBOARD_KEY = 1,
    KEY_ESCAPE = FIRST_KEYBOARD_KEY,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUALS,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_UE,
    KEY_PLUS,
    KEY_ENTER,
    KEY_LCONTROL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_OE,
    KEY_AE,
    KEY_GRAVE,
    KEY_LSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_KOMMA,
    KEY_PUNKT,
    KEY_SLASH,
    KEY_RSHIFT,
    KEY_NUMPAD_MULTIPLY,
    KEY_LALT,
    KEY_SPACE,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_NUMLOCK,
    KEY_SCROLLLOCK,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_NUMPAD_MINUS,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_PLUS,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_0,
    KEY_NUMPAD_KOMMA    = 83,
    KEY_GREATER         = 86,
    KEY_F11             = 87,
    KEY_F12             = 88,
    KEY_NUMPAD_ENTER    = 156,
    KEY_RCONTROL        = 157,
    KEY_NUMPAD_DIVIDE   = 181,
    KEY_RALT            = 184,
    KEY_HOME            = 199,
    KEY_UP              = 200,
    KEY_PAGEUP          = 201,
    KEY_LEFT            = 203,
    KEY_RIGHT           = 205,
    KEY_END             = 207,
    KEY_DOWN            = 208,
    KEY_PAGEDOWN        = 209,
    KEY_INSERT          = 210,
    KEY_DELETE          = 211,

    LAST_KEYBOARD_KEY = KEY_DELETE,

    MOUSE_LEFT          = 212,
    MOUSE_RIGHT,
    MOUSE_UP,
    MOUSE_DOWN,
    MOUSE_BUTTON_1,
    MOUSE_BUTTON_2,
    MOUSE_BUTTON_3,

    JOYSTICK_1_LEFT     = 219,
    JOYSTICK_1_RIGHT    = 220,
    JOYSTICK_1_UP       = 221,
    JOYSTICK_1_DOWN     = 222,
    JOYSTICK_1_BUTTON_1,
    JOYSTICK_1_BUTTON_2,
    JOYSTICK_1_BUTTON_3,
    JOYSTICK_1_BUTTON_4,
    JOYSTICK_1_BUTTON_5,
    JOYSTICK_1_BUTTON_6,
    JOYSTICK_1_BUTTON_7,
    JOYSTICK_1_BUTTON_8,
    JOYSTICK_1_BUTTON_9,
    JOYSTICK_1_BUTTON_10,
    JOYSTICK_1_BUTTON_11,
    JOYSTICK_1_BUTTON_12,
    JOYSTICK_1_BUTTON_13,
    JOYSTICK_1_BUTTON_14,
    JOYSTICK_1_BUTTON_15,
    JOYSTICK_1_BUTTON_16,
    JOYSTICK_1_BUTTON_17,
    JOYSTICK_1_BUTTON_18,
    JOYSTICK_1_BUTTON_19,
    JOYSTICK_1_BUTTON_20,
    JOYSTICK_1_BUTTON_21,
    JOYSTICK_1_BUTTON_22,
    JOYSTICK_1_BUTTON_23,
    JOYSTICK_1_BUTTON_24,
    JOYSTICK_1_BUTTON_25,
    JOYSTICK_1_BUTTON_26,
    JOYSTICK_1_BUTTON_27,
    JOYSTICK_1_BUTTON_28,
    JOYSTICK_1_BUTTON_29,
    JOYSTICK_1_BUTTON_30,
    JOYSTICK_1_BUTTON_31,
    JOYSTICK_1_BUTTON_32,

    JOYSTICK_2_LEFT,
    JOYSTICK_2_RIGHT,
    JOYSTICK_2_UP,
    JOYSTICK_2_DOWN,
    JOYSTICK_2_BUTTON_1,
    JOYSTICK_2_BUTTON_2,
    JOYSTICK_2_BUTTON_3,
    JOYSTICK_2_BUTTON_4,
    JOYSTICK_2_BUTTON_5,
    JOYSTICK_2_BUTTON_6,
    JOYSTICK_2_BUTTON_7,
    JOYSTICK_2_BUTTON_8,
    JOYSTICK_2_BUTTON_9,
    JOYSTICK_2_BUTTON_10,
    JOYSTICK_2_BUTTON_11,
    JOYSTICK_2_BUTTON_12,
    JOYSTICK_2_BUTTON_13,
    JOYSTICK_2_BUTTON_14,
    JOYSTICK_2_BUTTON_15,
    JOYSTICK_2_BUTTON_16,
    JOYSTICK_2_BUTTON_17,
    JOYSTICK_2_BUTTON_18,
    JOYSTICK_2_BUTTON_19,
    JOYSTICK_2_BUTTON_20,
    JOYSTICK_2_BUTTON_21,
    JOYSTICK_2_BUTTON_22,
    JOYSTICK_2_BUTTON_23,
    JOYSTICK_2_BUTTON_24,
    JOYSTICK_2_BUTTON_25,
    JOYSTICK_2_BUTTON_26,
    JOYSTICK_2_BUTTON_27,
    JOYSTICK_2_BUTTON_28,
    JOYSTICK_2_BUTTON_29,
    JOYSTICK_2_BUTTON_30,
    JOYSTICK_2_BUTTON_31,
    JOYSTICK_2_BUTTON_32,

    MOUSE_WHEEL_UP,
    MOUSE_WHEEL_DOWN,

    DEFKEY_LAST_ENTRY,
  };



class XInput: public GR::IService,
              public IEventProducer<Xtreme::tInputEvent>, 
              public GR::ITask
              
{

  public:


  protected:


  public:

    
    virtual bool                  Initialize( GR::IEnvironment& Environment ) = 0;
    virtual bool                  Release() = 0;

    virtual bool                  IsInitialized() const = 0;

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
    virtual void                  SetAlternateMousePosWindow( HWND hwndRelative ) = 0;
#endif

    // Einstellungen
    virtual bool                  SetAbilities( int iSensitivity, int iTreshold, int iJTreshold ) = 0;
    virtual bool                  IsJoystickAvailable() = 0;
    virtual bool                  IsTouchAvailable() = 0;

    // Kommandos
    virtual void                  AddCommand( const char* Command, GR::u32 Key, GR::u32 Flags = tInputCommand::CMD_DOWN ) = 0;
    virtual void                  RemoveCommand( const char* Command ) = 0;
    virtual bool                  IsCommand( const char* Command, bool DoNotModifyFlags = false ) const = 0;
    virtual bool                  IsCommandByKey( GR::u32 Key, const char* Command ) const = 0;

    // Handler
    virtual bool                  AddHandler( tKeyHandlerFunction Function, const GR::u32 Key, GR::u32 Flags = tInputCommand::CMD_RELEASED_DOWN ) = 0;
    virtual bool                  RemoveHandler( tKeyHandlerFunction Function ) = 0;

    // Tasten-Events
    virtual GR::u32               PressedVKey() const = 0;
    virtual GR::u32               GetVirtualCode( GR::u32 Nr ) const = 0;
    
    virtual bool                  KeyPressed( eInputDefaultButtons Key ) const = 0;
    virtual bool                  KeyReleased( eInputDefaultButtons Key ) const = 0;
    virtual bool                  ReleasedKeyPressed( eInputDefaultButtons Key ) const = 0;
    virtual GR::u32               MapKeyToVKey( eInputDefaultButtons Key ) const = 0;

    virtual bool                  VKeyPressed( GR::u32 Key ) const = 0;
    virtual bool                  ReleasedVKeyPressed( GR::u32 Key ) const = 0;
    virtual bool                  VKeyReleased( GR::u32 Key ) const = 0;
    virtual bool                  VKeySetReleaseFlag( GR::u32 Key ) const = 0;
    virtual bool                  VKeyDown( GR::u32 Key ) const = 0;   // prüft mit Anschlag und Delay
    virtual void                  ClearMouseFlags() = 0;

    virtual bool                  ShiftPressed() const = 0;
    virtual bool                  CtrlPressed() const = 0;
    virtual bool                  AltPressed() const = 0;

    // Mouse
    virtual int                   MouseX() const = 0;
    virtual int                   MouseY() const = 0;
    virtual GR::tPoint            MousePos() const = 0;
    virtual GR::u32               MouseButton() const = 0;
    virtual const GR::Char*       GetKeyName( GR::u32 Key ) const = 0;

    virtual const GR::Char*       GetAnalogControlName( GR::u32 Key ) const = 0;
    virtual ControlType           GetAnalogControlType( GR::u32 Key ) const = 0;
    virtual int                   GetAnalogControlDevice( GR::u32 Key ) const = 0;
    virtual int                   GetAnalogControlValue( GR::u32 Key ) const = 0;
    virtual int                   GetAnalogControlMinValue( GR::u32 Key ) const = 0;
    virtual int                   GetAnalogControlMaxValue( GR::u32 Key ) const = 0;
    virtual GR::u32               GetAnalogControlCount() const = 0;

    virtual const GR::Char*       GetControlName( GR::u32 Key ) const = 0;
    virtual GR::u32               GetControlType( GR::u32 Key ) const = 0;
    virtual int                   GetControlDevice( GR::u32 Key ) const = 0;
    virtual GR::u32               GetControlCount() const = 0;

    virtual const GR::Char*       GetDeviceName( GR::u32 iDevice ) const = 0;
    virtual bool                  HasDeviceForceFeedback( GR::u32 Device ) const = 0;
    virtual GR::u32               DeviceForceFeedbackAxisCount( GR::u32 Device ) const = 0;
    virtual bool                  SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const = 0;

    virtual void                  AddBinding( const GR::u32 Handle, const GR::u32 Key ) = 0;
    virtual void                  RemoveBinding( const GR::u32 Handle ) = 0;

    virtual bool                  SaveBindings( IIOStream& Stream ) = 0;
    virtual bool                  LoadBindings( IIOStream& Stream ) = 0;
    virtual void                  ClearAllBindings() = 0;
    virtual GR::u32               BoundKey( const GR::u32 BindHandle ) const = 0;


    // und die elementare Poll-Funktion
    virtual void                  Poll() = 0;


    virtual void                  Init() = 0;
    virtual void                  Exit() = 0;
    virtual void                  Update( const float ElapsedTime ) = 0;

};

}

#endif // XTREME_XINPUT_H