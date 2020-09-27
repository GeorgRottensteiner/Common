#ifndef WIN_INPUT_H
#define WIN_INPUT_H



#include <Xtreme/Input/XBasicInput.h>



class CWinInput : public XBasicInput
{

  private:

    static char                   m_KeyName[240][25];



  public:

    int                           m_InputMouseX,
                                  m_InputMouseY,
                                  m_InputMouseRelX,
                                  m_InputMouseRelY;

    WORD                          m_wInputJoystickID;

    HINSTANCE                     m_hWinMMHandle;


    CWinInput();
    virtual ~CWinInput();


    virtual bool                  Initialize( GR::IEnvironment& Environment );
    virtual bool                  Release();

    void                          Poll();


    virtual void                  Init();
    virtual void                  Exit();
    virtual void                  Update( const float ElapsedTime );

    virtual BOOL                  WindowProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

    virtual bool                  HasDeviceForceFeedback( GR::u32 Device ) const;
    virtual GR::u32               DeviceForceFeedbackAxisCount( GR::u32 Device ) const;
    virtual bool                  SetDeviceForce( GR::u32 Device, GR::u32 Axis, GR::i32 Value ) const;

};



#endif // WIN_INPUT_H