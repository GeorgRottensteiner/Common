#ifndef __NO_DIRECT_X__
#pragma message( "__NO_DIRECT_X__ sollte definiert sein!\n" )
#define __NO_DIRECT_X__
#endif

#pragma warning( disable : 4786 )

#include <windows.h>

#include <string>
#include <map>

#include <Xtreme/Input/WinInput.h>

#pragma comment( lib, "dxguid.lib" )

#include <Grafik/GDIPage.h>
#include <Grafik/Image.h>

#include <WinSys/SubclassManager.h>

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <Lang/EventQueue.h>

#include <Debug/DebugService.h>

#include <GR/GameState/IGameState.h>

#include <MasterFrame/XWindowWin32.h>



typedef std::map<DWORD,GR::Graphic::Image*>    tMapImages;

struct tEvent
{
  DWORD         Type;
  int           Param;
  GR::String    TextParam;

  tEvent::tEvent( DWORD Type = 0, int Param = 0, const GR::String& TextParam = "" ) :
    Type( Type ),
    Param( Param ),
    TextParam( TextParam )
  {
  }
};



class CDeskGame : public EventListener<tEvent>,
                  public EventProducer<tEvent>,
                  public IGameStateManager<GR::Graphic::GFXPage>,
                  public IGlobalEventListener
{
  protected:

    GR::String         m_Caption,
                        m_BaseKey,
                        m_AppPath;

    HWND                m_HwndMain,
                        m_HwndPage;

    DWORD               m_MenuId,
                        m_IconId,
                        m_Depth,
                        m_ExtendedClientStyles;

    GR::u32             m_ETChangeState,
                        m_ETPushState,
                        m_ETPopState,
                        m_ETShutDown;

    int                 m_Width,
                        m_Height,
                        m_FixedWidth,
                        m_FixedHeight;

    bool                m_GameActive,
                        m_GameMinimized,
                        m_GameShutDown,
                        m_FixedSize;

    GR::f64             m_LastFrameTime,
                        m_LastFixedFrameTime,
                        m_FixedLogicTimeStep;

    tMapImages          m_Images;

    CWinInput           m_Input;

    DebugService        m_Debugger;

    Xtreme::Win32Window m_Window;

    GR::Graphic::GDIPage*  m_pPage;


    virtual BOOL        DeskGameWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    virtual LRESULT CALLBACK  DeskGamePageWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    void                CleanUp();

  public:


    virtual bool        InitInstance();
    virtual void        DisplayFrame();
    virtual void        UpdateFrame();
    virtual void        UpdatePerDisplayFrame( const float ElapsedTime );
    virtual void        ExitInstance();
    virtual int         Run();
    void                ShutDown();

    // Registry-Funktionen
    void                SetBaseKey( const char* BaseKey );
    void                SetSetting( const char* Key, const char* Value );
    GR::String         GetSetting( const char* Key );
    void                SetSettingI( const char* Key, int Value );
    int                 GetSettingI( const char* Key, int Default = 0 );

    GR::String         AppPath( const char* Path );

    void                SetWindowValues( const char* Caption, DWORD Icon, DWORD MenuId, DWORD ExtendedClientStyles = WS_EX_CLIENTEDGE );
    void                SetSize( int Width, int Height, DWORD Depth, bool FixedSize = false );
    void                Invalidate( RECT* rc = NULL );

    void                EnableMenuItem( DWORD Id );
    void                DisableMenuItem( DWORD Id );

    void                AddEvent( DWORD EventId, DWORD EventValue = 0, GR::String Value = GR::String() );
    virtual bool        ProcessEvent( const tEvent& Event );
    void                WinPrint( int X, int Y, const char* Text, DWORD Color = 0xffffff );

    virtual void        ProcessEvent( const tGlobalEvent& Event );

    virtual void        SetClientSize( int Width, int Height );

    Xtreme::XInput*     InputClass();

    bool                IsPageScaled();
    void                GetMousePos( GR::tPoint& Pos );


    GR::Graphic::Image*         LoadImage( DWORD Id );


    CDeskGame();
    virtual ~CDeskGame();


    friend BOOL CALLBACK DeskGameWndProc( HWND HwndDlg, UINT Msg, WPARAM WParam, LPARAM LParam );
    friend BOOL CALLBACK DeskGamePageWndProc( HWND HwndDlg, UINT Msg, WPARAM WParam, LPARAM LParam );

};