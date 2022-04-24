#ifndef DXINPUT_H
#define DXINPUT_H


#include <windows.h>
#include <winuser.h>

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

#include <map>
#include <vector>

#include <GR/GRTypes.h>

#include <Lang/ITask.h>

#include <Xtreme/Input/XBasicInput.h>



#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif


struct IIOStream;

class CDXInput : public XBasicInput
{

  private:

    struct DeviceInfo
    {
      IDirectInputDevice8*      pDevice;
      LPDIRECTINPUTEFFECT       pEffect;
      std::vector<GR::u32>      AxisOffsets;
      GR::u32                   AxisCount;
      LONG                      AxisValue[2];

      DeviceInfo() :
        pDevice( NULL ),
        pEffect( NULL ),
        AxisCount( 0 )
      {
        AxisValue[0] = 0;
        AxisValue[1] = 0;
      }
    };


    bool                          CreateFFEffect( DeviceInfo* pInfo );

    static BOOL CALLBACK          EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );


  protected:

    int                           m_CurrentEnumDevice,
                                  m_CurrentEnumJoystickDevice;

    GR::u32                       m_CurrentEnumDeviceType;

    int                           m_CurrentMouseButtonsEnumerated;

    LPDIRECTINPUT8                m_lpDirectInput;

    bool                          m_Exclusive;

    bool                          PollDevice( tInputDevice& Device );
    void                          ParseDeviceData( tInputDevice& Device );
    bool                          SetDataFormat( tInputDevice& Device );

    // Controls den virtuelle Keys zuordnen
    void                          CreateVirtualKeys();


  public:


    CDXInput();
    ~CDXInput();


    bool                          Initialize( GR::IEnvironment& Environment );
    bool                          Release();

    bool                          SaveBindings( IIOStream& Stream );
    bool                          LoadBindings( IIOStream& Stream );


    // und die elementare Poll-Funktion
    void                          Poll();


    virtual void                  Init();
    virtual void                  Exit();
    virtual void                  Update( const float ElapsedTime );

    static BOOL CALLBACK          EnumDevicesProc( const DIDEVICEINSTANCE* lpddi, LPVOID pvRef );
    static BOOL CALLBACK          EnumObjects( LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef );

    virtual BOOL                  WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

    virtual bool                  HasDeviceForceFeedback( GR::u32 Device ) const;
    virtual GR::u32               DeviceForceFeedbackAxisCount( GR::u32 Device ) const;
    virtual bool                  SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const;


};


#endif // DXINPUT_H