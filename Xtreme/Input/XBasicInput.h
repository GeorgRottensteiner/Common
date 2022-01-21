#ifndef XBASIC_INPUT_H
#define XBASIC_INPUT_H

#include <GR/GRTypes.h>

#include <Lang/ITask.h>
#include <Lang/Service.h>

#include <map>
#include <vector>

#include <Xtreme/XInput.h>



class IDebugService;



namespace Xtreme
{
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
}



class XBasicInput : public EventProducerImpl<Xtreme::tInputEvent,GR::Service::ServiceImpl<Xtreme::XInput> >

{

  protected:

    struct tVirtualKey
    {
      GR::String                    m_Name;
      GR::u32                       m_UserData;
      GR::u32                       m_UserData2;
      GR::u32                       m_GlobalIndex;
      GR::u32                       m_DeviceControlIndex;
      Xtreme::eInputDefaultButtons  m_EnumIndex;
      bool                          m_Pressed,
                                    m_Released,
                                    m_WasPressed,
                                    m_WasReleasedWhilePressed;
      GR::u32                       m_ButtonDownTicks;
      bool                          m_ButtonFirstTimeDelay,
                                    m_ButtonDown;


      tVirtualKey( const GR::String& Name = GR::String(), int GlobalIndex = 0 ) :
        m_Name( Name ),
        m_GlobalIndex( GlobalIndex ),
        m_EnumIndex( Xtreme::KEY_INVALID ),
        m_Pressed( false ),
        m_Released( false ),
        m_WasPressed( false ),
        m_WasReleasedWhilePressed( false ),
        m_ButtonDownTicks( 0 ),
        m_ButtonFirstTimeDelay( false ),
        m_ButtonDown( false ),
        m_DeviceControlIndex( -1 ),
        m_UserData( 0 ),
        m_UserData2( 0 )
      {
      }
    };

    struct tInputCtrl
    {
      GR::String        m_Name;
      Xtreme::ControlType   m_Type;
      GR::u32               m_APIDataOffset;
      GR::u32               m_APIType;
      int                   m_Device;
      GR::u32               m_GlobalIndex;
      GR::u32               m_VirtualIndex;
      GR::u32               m_AnalogIndex;
      GR::u32               m_DeviceControlIndex;
      GR::u32               m_DeviceType;
      GR::u32               m_VirtualKeyCode;
      int                   m_Min,
                            m_Max,
                            m_Value;




      tInputCtrl( int Device = -1, const GR::String& Name = GR::String(), Xtreme::ControlType Type = Xtreme::CT_UNKNOWN, GR::u32 APIDataOffset = 0 ) :
        m_Name( Name ),
        m_Type( Type ),
        m_APIType( 0 ),
        m_APIDataOffset( APIDataOffset ),
        m_Device( Device ),
        m_GlobalIndex( (GR::u32)-1 ),
        m_VirtualIndex( (GR::u32)-1 ),
        m_Min( 0 ),
        m_Max( 65535 ),
        m_DeviceType( Xtreme::VKT_UNKNOWN ),
        m_DeviceControlIndex( -1 ),
        m_VirtualKeyCode( 0 ),
        m_Value( 0 ),
        m_AnalogIndex( (GR::u32)-1 )
      {
      }
    };

    typedef std::vector<tInputCtrl>                           tVectControls;

    struct tInputDevice
    {
      GR::String          m_Device;

      GR::up*                 m_pDevicePointer;

      GR::u32*                m_pDeviceData;

      tVectControls           m_Controls;
      tVectControls           m_AnalogControls;



      tInputDevice( const GR::String& Name = "", GR::up* pDevice = NULL ) :
        m_Device( Name ),
        m_pDevicePointer( pDevice ),
        m_pDeviceData( NULL )
      {
      }
    };


  private:

    static XBasicInput*           m_pGlobalInputInstance;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    WNDPROC                       m_lpfnInputOldWndProc;
#endif

  protected:

    typedef std::multimap<GR::string,Xtreme::tInputCommand>  tMapCommands;

    typedef std::map<Xtreme::eInputDefaultButtons,GR::u32>        tMapDefaultKeys;
    typedef std::multimap<Xtreme::eInputDefaultButtons,GR::u32>   tMapVirtualKeys;

    typedef std::map<GR::u32,GR::u32>                         tMapBinding2VKey;

    typedef std::vector<tInputDevice>                         tVectDevices;

    typedef std::map<std::pair<GR::u32,GR::u32>,Xtreme::tKeyHandlerFunction>  tMapKeyHandler;

    tMapCommands                  m_Commands;

    tMapDefaultKeys               m_DefaultKeys;

    tMapVirtualKeys               m_VirtualKeyToXtremeKey;

    tMapBinding2VKey              m_Binding2VKey;

    tVectControls                 m_Controls;
    tVectControls                 m_AnalogControls;

    tVectDevices                  m_Devices;

    tMapKeyHandler                m_KeyHandler;

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE ) && ( OPERATING_SUB_SYSTEM != OS_SUB_SDL ) && ( OPERATING_SUB_SYSTEM != OS_SUB_UNIVERSAL_APP )
    HWND                          m_hwndInput,
                                  m_hwndRelativeMouseTo;
#endif

    IDebugService*                m_pDebugger;

    bool                          m_Initialized,
                                  m_InputMButtonsSwapped;

    tVirtualKey*                  m_pVirtualKey;

    GR::u32                       m_VirtualKeys;

    GR::u32                       m_PollTicks,
                                  m_KeyboardDelayTicks,
                                  m_KeyboardRepeatDelayTicks;

    bool                          m_InputActive;

    int                           m_InputJoyTreshold,
                                  m_InputWMMouseX,
                                  m_InputWMMouseY,
                                  m_InputMouseSensitivity,
                                  m_InputMouseTreshold;

    unsigned char                 m_VirtualCode[240];


  public:

    XBasicInput();
    virtual ~XBasicInput();


    virtual bool                  Initialize( GR::IEnvironment& Environment );
    virtual bool                  Release();

    bool                          IsInitialized() const;

#if ( OPERATING_SYSTEM == OS_WINDOWS ) && ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP )
    virtual BOOL                  WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK       InputSubClassProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam);

    bool                          SubclassInputWindow();
    bool                          UnSubclassInputWindow();

    virtual void                  SetAlternateMousePosWindow( HWND hwndRelative );
#endif

    // Handler
    virtual bool                  AddHandler( Xtreme::tKeyHandlerFunction Function, const GR::u32 Key, GR::u32 Flags = Xtreme::tInputCommand::CMD_RELEASED_DOWN );
    virtual bool                  RemoveHandler( Xtreme::tKeyHandlerFunction Function );

    // Kommandos
    void                          AddCommand( const char* Command, GR::u32 Key, GR::u32 Flags = Xtreme::tInputCommand::CMD_DOWN );
    void                          RemoveCommand( const char* Command );
    bool                          IsCommand( const char* Command, bool DoNotModifyFlags = false ) const;
    bool                          IsCommandByKey( GR::u32 Key, const char* Command ) const;

    // Bindings
    void                          AddBinding( const GR::u32 Handle, const GR::u32 Key );
    void                          RemoveBinding( const GR::u32 Handle );

    bool                          SaveBindings( IIOStream& Stream );
    bool                          LoadBindings( IIOStream& Stream );
    void                          ClearAllBindings();
    GR::u32                       BoundKey( const GR::u32 BindHandle ) const;

    GR::u32                       MapKeyToVKey( Xtreme::eInputDefaultButtons Key ) const;

    bool                          KeyPressed( Xtreme::eInputDefaultButtons Key ) const;
    bool                          KeyReleased( Xtreme::eInputDefaultButtons Key ) const;
    bool                          ReleasedKeyPressed( Xtreme::eInputDefaultButtons Key ) const;
    bool                          VKeyPressed( GR::u32 Key ) const;
    bool                          ReleasedVKeyPressed( GR::u32 Key ) const;
    bool                          VKeyReleased( GR::u32 Key ) const;
    bool                          VKeySetReleaseFlag( GR::u32 Key ) const;
    bool                          VKeyDown( GR::u32 Key ) const;   // prüft mit Anschlag und Delay

    void                          SetButtonPressed( GR::u32 Button, bool Pressed );

    // Tasten-Events
    GR::u32                       PressedVKey() const;
    GR::u32                       GetVirtualCode( GR::u32 Nr ) const;

    void                          ClearMouseFlags();

    bool                          ShiftPressed() const;
    bool                          CtrlPressed() const;
    bool                          AltPressed() const;

    // Mouse
    int                           MouseX() const;
    int                           MouseY() const;
    GR::tPoint                    MousePos() const;
    GR::u32                       MouseButton() const;
    const GR::Char*               GetKeyName( GR::u32 Key ) const;

    const GR::Char*               GetControlName( GR::u32 Key ) const;
    GR::u32                       GetControlType( GR::u32 Key ) const;
    int                           GetControlDevice( GR::u32 Key ) const;
    GR::u32                       GetControlCount() const;

    virtual const GR::Char*       GetAnalogControlName( GR::u32 Key ) const;
    virtual Xtreme::ControlType   GetAnalogControlType( GR::u32 Key ) const;
    virtual int                   GetAnalogControlDevice( GR::u32 Key ) const;
    virtual int                   GetAnalogControlValue( GR::u32 Key ) const;
    virtual int                   GetAnalogControlMinValue( GR::u32 Key ) const;
    virtual int                   GetAnalogControlMaxValue( GR::u32 Key ) const;
    virtual GR::u32               GetAnalogControlCount() const;

    const GR::Char*               GetDeviceName( GR::u32 Device ) const;

    // Einstellungen
    bool                          SetAbilities( int Sensitivity, int Treshold, int JoystickTreshold );
    virtual bool                  IsJoystickAvailable();
    virtual bool                  IsTouchAvailable();


    // interne Belange (Control-Handling)
    GR::u32                       AddDevice( const tInputDevice& Device );
    GR::u32                       AddDeviceControl( const GR::u32 DeviceIndex, const tInputCtrl& Control );
    GR::u32                       AddDeviceAnalogControl( const GR::u32 DeviceIndex, const tInputCtrl& Control );


    void                          LogDirect( const GR::String& System, const GR::String& Text );
    void                          Log( const GR::String& System, const char* Format, ... );

    // update waspressed and wasreleasedwhenpressed for all potential controls
    void                          FrameCompleted();

};



#endif // XINPUT_H