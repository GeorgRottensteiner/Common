#ifndef RAW_INPUT_H
#define RAW_INPUT_H

#include <Xtreme/Input/XBasicInput.h>


extern "C"
{
  //#include <Hidpi.h>
  #include <hidsdi.h>
}



class RawInput :   public XBasicInput
{

  protected:

    struct Axis
    {
      // Usage Page und Usage der Achse, z. B. „generic (0001) / slider (0036)“.
      USAGE                  UsagePage;
      USAGE                  Usage;

      // Index der Achse in dem Array, das Hidp_GetData() füllt.
      USHORT                 Index;

      GR::u32                DeviceControlIndex;

      // Umfang der logischen Werte dieser Achse. Liegt der tatsächliche Wert außerhalb dieses
      //  Bereichs, müssen wir annehmen, dass die Achse „nicht gesetzt“ ist. Das passiert z. B.
      //  bei Rundblickschaltern, wenn man sie nicht drückt.
      // Die HID API behandelt diese Werte durchgängig unsigned, aber die USB-HID-Spezifikation
      //  erinnert ausdrücklich, dass hier negative Werte auftreten können.
      GR::i32                LogicalMinimum;
      GR::i32                LogicalMaximum;
      // Ob wir DirectInput-Kalibrierungsdaten für diese Achse gefunden haben.
      bool                   IsCalibrated;
      // Durch DirectInput-Kalibrierung bestimmter Bereich der Achse, falls sie kalibriert ist.
      GR::i32                LogicalCalibratedMinimum;
      GR::i32                LogicalCalibratedMaximum;
      // Durch DirectInput-Kalibrierung bestimmte Mittelstellung, falls die Achse kalibriert ist.
      GR::i32                LogicalCalibratedCenter;
      // Physischer Bereich der Achse. Das verarbeiten wir hier nicht, sondern leiten es an die
      //  Anwendung weiter, damit die damit arbeiten kann.
      float                  PhysicalMinimum;
      float                  PhysicalMaximum;
      // Der Name, den der Treiber der Achse gegeben hat; z. B. „combined pedals“.
      GR::WString            Name;

      GR::i32                Value;

      Xtreme::ControlType    Type;



      Axis() :
        UsagePage( 0 ),
        Usage( 0 ),
        Index( 0 ),
        LogicalMinimum( -100000 ),
        LogicalMaximum( 100000 ),
        IsCalibrated( false ),
        LogicalCalibratedMinimum( -100000 ),
        LogicalCalibratedMaximum( 100000 ),
        LogicalCalibratedCenter( 0 ),
        PhysicalMinimum( -100000.0f ),
        PhysicalMaximum( 100000.0f ),
        Value( 0 ),
        Type( Xtreme::CT_UNKNOWN ),
        DeviceControlIndex( (GR::u32)-1 )
      {
      }
    };

    struct Button
    {
      // Usage Page und Usage des Knopfes, z. B. „buttons (0009) / secondary (0002)“.
      USAGE                  UsagePage;
      USAGE                  Usage;
      // Index des Knopfs in dem Array, das Hidp_GetData() füllt.
      USHORT                 Index;

      GR::u32                DeviceControlIndex;
      // Der Name, den der Treiber dem Knopf gegeben hat; bei PlayStation-Controllern z. B. X
      //  oder Square.
      GR::WString            Name;

      // state
      bool                   Pushed;

      Button() :
        Pushed( false ),
        UsagePage( 0 ),
        Usage( 0 ),
        Index( 0 ),
        DeviceControlIndex( (GR::u32)-1 )
      {
      }
    };


    struct DeviceInfo
    {
      HANDLE                  RawInputHandle;

      GR::WString             Name;

      // Input Report Protocol. Diese Datenstruktur ist nirgends definiert; sie hängt vom Gerät ab.
      PHIDP_PREPARSED_DATA    ToInputReportProtocol;

      // Die Größe eines einzelnen Input Reports. Hierher wissen die einzelnen HID-Zustandsobjekte,
      //  wie viel Speicher sie zur Verfügung stellen müssen.
      ULONG                   SizeOfInputReport;

      HIDP_CAPS               Caps;


      std::vector<HIDP_BUTTON_CAPS>     ButtonClasses;
      std::vector<int>                  ButtonGlobalOffset;
      std::vector<HIDP_VALUE_CAPS>      AxisClasses;
      std::vector<int>                  AxisGlobalOffset;

      std::vector<Button>     Buttons;
      std::vector<Axis>       Axis;

      GR::u32                 DeviceIndex;


      DeviceInfo() :
        RawInputHandle( INVALID_HANDLE_VALUE ),
        ToInputReportProtocol( NULL ),
        SizeOfInputReport( 0 ),
        DeviceIndex( (GR::u32)-1 )
      {
      }
    };



    bool                EnumerateDevices();

    void                InterpretKeyboardData( RAWINPUT* pRaw, DeviceInfo* pDevice );
    void                InterpretHIDData( RAWINPUT* pRaw, DeviceInfo* pDevice );
    void                HandleMouseAxis( int Delta, GR::u32 NegativeKey, GR::u32 PositiveKey );

    void                CreateVirtualKeys();



  public:

    std::vector<RAWINPUTDEVICE>     m_RegisteredDevices;


    int                 m_RawMouseX;
    int                 m_RawMouseY;



    RawInput();
    ~RawInput();



    virtual bool        Initialize( GR::IEnvironment& Environment );
    virtual bool        Release();


    virtual void        Poll();

    virtual void        Init();
    virtual void        Exit();
    virtual void        Update( const float fElapsedTime );


    virtual bool        HasDeviceForceFeedback( GR::u32 Device ) const;
    virtual GR::u32     DeviceForceFeedbackAxisCount( GR::u32 Device ) const;
    virtual bool        SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const;

    virtual BOOL        WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

};


#endif // RAW_INPUT_H