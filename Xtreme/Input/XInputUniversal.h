#ifndef XINPUT_UNIVERSAL_H
#define XINPUT_UNIVERSAL_H

#include <Xtreme/Input/XBasicInput.h>



class XFrameApp;
 
class XInputUniversal : public XBasicInput
{
  private:

    static char                   m_KeyName[240][25];

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
    std::vector<Windows::Gaming::Input::Gamepad^>     m_GamePads;
#endif

    bool                          m_GamepadsChanged;

    double                        m_AnalogJoystickThreshold;

    XFrameApp*                    m_pFrameApp;

	  Windows::Devices::Sensors::Gyrometer^		m_Gyrometer;

    Windows::Devices::Sensors::OrientationSensor^     m_Sensor;

    Windows::Foundation::EventRegistrationToken       m_KeyDownToken;
    Windows::Foundation::EventRegistrationToken       m_KeyUpToken;
    Windows::Foundation::EventRegistrationToken       m_OnCharToken;
    Windows::Foundation::EventRegistrationToken       m_PointerMovedToken;
    Windows::Foundation::EventRegistrationToken       m_PointerPressedToken;
    Windows::Foundation::EventRegistrationToken       m_PointerReleasedToken;
    Windows::Foundation::EventRegistrationToken       m_PointerWheelChangedToken;



  public:

    XInputUniversal();

    virtual ~XInputUniversal();


    virtual bool                  HasDeviceForceFeedback( GR::u32 Device ) const;
    virtual GR::u32               DeviceForceFeedbackAxisCount( GR::u32 Device ) const;
    virtual bool                  SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const;
    virtual bool                  IsTouchAvailable();

    // und die elementare Poll-Funktion
    virtual void                  Poll();


    virtual void                  Init();
    virtual void                  Exit();
    virtual void                  Update( const float fElapsedTime );

    virtual bool                  Initialize( GR::IEnvironment& Environment );
    virtual bool                  Release();

    void OnGamepadChanged();
	  void OnGyrometerReadingChanged( double X, double Y, double Z );
    void OnOrientationReadingChanged( Windows::Devices::Sensors::SensorQuaternion^ Quat, Windows::Devices::Sensors::SensorRotationMatrix^ Matrix);


    void UpdateMouse( int X, int Y, int Buttons );

    GR::String                    GyroInfo();

    GR::String                    RotationInfo;

    void                          SetActive( bool Active = true );

    friend ref class XInputCLRMediator;
};




#endif // XINPUT_UNIVERSAL_H
