#ifndef ANDROID_INPUT_H
#define ANDROID_INPUT_H

#include <Xtreme/Input/XBasicInput.h>

#include <android/sensor.h>
#include <android/looper.h>



class XFrameApp;
class AndroidMediator;
 
class AndroidInput : public XBasicInput
{
  private:

    static char                   m_KeyName[240][25];

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
    std::vector<Windows::Gaming::Input::Gamepad^>     m_GamePads;
#endif

    bool                          m_GamepadsChanged;

    double                        m_AnalogJoystickThreshold;

    XFrameApp*                    m_pFrameApp;

    ASensorManager*               m_pSensorManager;
    const ASensor*                m_pAccelerometerSensor;
    ASensorEventQueue*            m_pSensorEventQueue;

    ALooper*                      m_pLooper;

    AndroidMediator*              m_pMediator;




  public:

    AndroidInput();

    virtual ~AndroidInput();


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


    void UpdateMouse( int X, int Y, int Buttons );

    void                          SetActive( bool Active = true );


};




#endif // ANDROID_INPUT_H
