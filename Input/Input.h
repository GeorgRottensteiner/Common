#ifndef XINPUT_H
#define XINPUT_H



#include <windows.h>
#include <winuser.h>

#ifndef __NO_DIRECT_X__
#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#endif // #ifndef __NO_DIRECT_X__

#include <string>
#include <map>
#include <vector>

#include <GR/GRTypes.h>

#include <Interface/IEventProducer.h>

#include <Lang/ITask.h>



#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif


struct IIOStream;

struct tInputCommand
{

  enum eType
  {
    CMD_INVALID = 0,
    CMD_DOWN,
    CMD_UP,
    CMD_RELEASED_DOWN,
  };

  GR::u32       m_dwFlags,
                m_dwKey;

  GR::String    m_strDesc;

  eType         m_Type;


  tInputCommand() :
    m_dwFlags( 0 ),
    m_dwKey( 0 ),
    m_strDesc( "" ),
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

  eInputEventType           m_Type;

  GR::up                    m_dwParam1,
                            m_dwParam2;

  GR::String                m_strCommand;

  tInputEvent( eInputEventType eType = IE_INVALID, GR::up dwParam1 = 0, GR::up dwParam2 = 0, const GR::String& strCommand = "" ) :
    m_Type( eType ),
    m_dwParam1( dwParam1 ),
    m_dwParam2( dwParam2 ),
    m_strCommand( strCommand )
  {
  }
};

class CInputSystem : public EventProducer<tInputEvent>, 
                     public GR::ITask
{

  public:

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

      DEFKEY_LAST_ENTRY,
    };

    enum eVKeyType
    {
      VKT_UNKNOWN = 0,
      VKT_KEYBOARD        = 0x00000001,
      VKT_DEFAULT_MOUSE   = 0x00000002,
      VKT_JOYSTICK_1      = 0x00000004,
      VKT_JOYSTICK_2      = 0x00000008,
      VKT_X_AXIS          = 0x00010000,
      VKT_Y_AXIS          = 0x00020000,
    };


  protected:


    typedef std::map<eInputDefaultButtons,GR::u32>    tMapDefaultKeys;

    typedef std::map<GR::u32,GR::u32>                 tMapBinding2VKey;


    struct tVirtualKey
    {
      GR::String              m_strName;
      GR::u32                 m_iGlobalIndex;
      GR::u32                 m_iDeviceControlIndex;
      eInputDefaultButtons    m_iEnumIndex;
      bool                    m_bPressed,
                              m_bReleased;
      GR::u32                 m_dwButtonDownTicks;
      bool                    m_bButtonFirstTimeDelay,
                              m_bButtonDown;


      tVirtualKey( const GR::String& strName = "", int iGlobalIndex = 0 ) :
        m_strName( strName ),
        m_iGlobalIndex( iGlobalIndex ),
        m_iEnumIndex( KEY_INVALID ),
        m_bPressed( false ),
        m_bReleased( false ),
        m_dwButtonDownTicks( 0 ),
        m_bButtonFirstTimeDelay( false ),
        m_bButtonDown( false ),
        m_iDeviceControlIndex( -1 )
      {
      }
    };

    struct tInputCtrl
    {
      GR::String      m_strName;
      GR::u32         m_dwType;
      GR::u32         m_dwDataOffset;
      int             m_iDevice;
      GR::u32         m_iGlobalIndex;
      GR::u32         m_iVirtualIndex;
      GR::u32         m_iDeviceControlIndex;
      GR::u32         m_DeviceType;
      int             m_iMin,
                      m_iMax;


      tInputCtrl( int iDevice = -1, const GR::String& strName = GR::String(), GR::u32 dwType = 0, GR::u32 dwDataOffset = 0 ) :
        m_strName( strName ),
        m_dwType( dwType ),
        m_dwDataOffset( dwDataOffset ),
        m_iDevice( iDevice ),
        m_iGlobalIndex( (GR::u32)-1 ),
        m_iVirtualIndex( (GR::u32)-1 ),
        m_iMin( 0 ),
        m_iMax( 65535 ),
        m_DeviceType( VKT_UNKNOWN ),
        m_iDeviceControlIndex( -1 )
      {
      }
    };

    typedef std::vector<tInputCtrl>   tVectControls;

    struct tInputDevice
    {
      GR::String              m_strDevice;

#ifndef __NO_DIRECT_X__
      IDirectInputDevice8*    m_pDevice;
#endif

      GR::u32*                m_pDeviceData;

      tVectControls           m_vectControls;

#ifndef __NO_DIRECT_X__
      tInputDevice( const GR::String& strName = "", IDirectInputDevice8* pDevice = NULL ) :
#else
      tInputDevice( const GR::String& strName = "" ) :
#endif
        m_strDevice( strName )
#ifndef __NO_DIRECT_X__
        ,
        m_pDevice( pDevice ),
        m_pDeviceData( NULL )
#endif
      {
      }
    };


    
    
    typedef std::vector<tInputDevice> tVectDevices;

    typedef std::multimap<GR::String,tInputCommand>    tMapCommands;

    tMapBinding2VKey              m_mapBinding2VKey;

    tMapDefaultKeys               m_mapDefaultKeys;

    tVectControls                 m_vectControls;

    tVectDevices                  m_vectDevices;

    bool                          m_bInitialized;

    tMapCommands                  m_mapCommands;

    int                           m_iCurrentEnumDevice,
                                  m_iCurrentEnumJoystickDevice;

    GR::u32                       m_dwCurrentEnumDeviceType;

    unsigned char                 m_ucVirtualCode[240];


#ifndef __NO_DIRECT_X__
    LPDIRECTINPUT8                m_lpDirectInput;

#endif // __NO_DIRECT_X__

    WNDPROC                       m_lpfnInputOldWndProc;

    bool                          m_bExclusive;

    BOOL                          m_bInputDX,
                                  m_bInputSubClassed,
                                  m_bInputActive,
                                  m_bInputMButtonsSwapped;

    tVirtualKey*                  m_pVirtualKey;

    GR::u32                       m_iVirtualKeys;

    GR::u32                         m_dwPollTicks,
                                  m_dwKeyboardDelayTicks,
                                  m_dwKeyboardRepeatDelayTicks;

    int                           m_iInputJoyX,
                                  m_iInputJoyY,
                                  m_iInputJoyTreshold,
                                  m_iInputWMMouseX,
                                  m_iInputWMMouseY,
                                  m_iInputMouseSensitivity,
                                  m_iInputMouseTreshold,
                                  m_iInputMouseX,
                                  m_iInputMouseY,
                                  m_iInputMouseZ,
                                  m_iInputMouseRelX,
                                  m_iInputMouseRelY;

    WORD                          m_wInputJoystickID;

    HINSTANCE                     m_hWinMMHandle;

    HWND                          m_hwndInput,
                                  m_hwndRelativeMouseTo;


    void                          SetButtonPressed( GR::u32 dwButton, bool bPressed );

#ifndef __NO_DIRECT_X__
    bool                          PollDevice( tInputDevice& Device );
    void                          ParseDeviceData( tInputDevice& Device );
    bool                          SetDataFormat( tInputDevice& Device );

    void                          CreateVirtualKeys();
#endif


    CInputSystem();


  public:

    
    ~CInputSystem();


    static CInputSystem&          Instance();
    

    BOOL                          Create( HINSTANCE hInst, HWND hWnd, BOOL bAllowDirectInput, bool bExclusive = false );
    BOOL                          Release();

    bool                          IsInitialized();

    void                          SetAlternateMousePosWindow( HWND hwndRelative );

    // Einstellungen
    BOOL                          SetAbilities( int iSensitivity, int iTreshold, int iJTreshold );
    bool                          IsJoystickAvailable();

    // Kommandos
    void                          AddCommand( const char *szCommand, GR::u32 dwKey, GR::u32 dwFlags = tInputCommand::CMD_DOWN );
    void                          RemoveCommand( const char *szCommand );
    bool                          IsCommand( const GR::String& strCommand, bool bDoNotModifyFlags = false ) const;
    bool                          IsCommandByKey( GR::u32 dwKey, GR::String& strCommand ) const;

    // Tasten-Events
    GR::u32                       PressedVKey() const;
    GR::u32                       GetVirtualCode( GR::u32 dwNr ) const;
    
    bool                          KeyPressed( eInputDefaultButtons edbKey ) const;
    bool                          KeyReleased( eInputDefaultButtons edbKey ) const;
    bool                          ReleasedKeyPressed( eInputDefaultButtons edbKey ) const;
    GR::u32                       MapKeyToVKey( eInputDefaultButtons edbKey ) const;

    bool                          VKeyPressed( GR::u32 dwKey ) const;
    bool                          ReleasedVKeyPressed( GR::u32 dwKey ) const;
    bool                          VKeyReleased( GR::u32 dwKey ) const;
    bool                          VKeySetReleaseFlag( GR::u32 dwKey ) const;
    bool                          VKeyDown( GR::u32 dwKey ) const;   // prüft mit Anschlag und Delay
    void                          ClearMouseFlags();

    bool                          ShiftPressed() const;
    bool                          CtrlPressed() const;
    bool                          AltPressed() const;

    // Mouse
    int                           MouseX() const;
    int                           MouseY() const;
    int                           MouseZ() const;
    GR::tPoint                    MousePos() const;
    int                           MouseDeltaX() const;
    int                           MouseDeltaY() const;
    GR::u32                       MouseButton() const;
    const char*                   GetKeyName( GR::u32 dwKey ) const;

    const char*                   GetControlName( GR::u32 dwKey ) const;
    GR::u32                       GetControlType( GR::u32 dwKey ) const;
    int                           GetControlDevice( GR::u32 dwKey ) const;
    GR::u32                       GetControlCount() const;
    const char*                   GetDeviceName( GR::u32 iDevice ) const;

    void                          AddBinding( const GR::u32 dwHandle, const GR::u32 dwKey );
    void                          RemoveBinding( const GR::u32 dwHandle );

    bool                          SaveBindings( IIOStream& Stream );
    bool                          LoadBindings( IIOStream& Stream );
    void                          ClearAllBindings();
    GR::u32                       BoundKey( const GR::u32 dwBindHandle ) const;


    // und die elementare Poll-Funktion
    void                          Poll();


    virtual void                  Init();
    virtual void                  Exit();
    virtual void                  Update( const float fElapsedTime );


#ifndef __NO_DIRECT_X__
    static BOOL CALLBACK          EnumDevicesProc( const DIDEVICEINSTANCE* lpddi, LPVOID pvRef );
    static BOOL CALLBACK          EnumObjects( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef );
#endif

    LRESULT                       WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );
    
    static LRESULT CALLBACK       InputSubClassProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam);

};



#endif // XINPUT_H