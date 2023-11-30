#ifndef ABSTRACTMASTER_H_INCLUDED
#define ABSTRACTMASTER_H_INCLUDED



#include <string>
#include <Misc\Misc.h>
#include <Xtreme/Audio/DXSound.h>

#include <MasterFrame/Console.h>
#include <Xtreme/Environment/XWindowWin32.h>

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <Xtreme/Input/DXInput.h>
#include <Xtreme/Input/WinInput.h>

#include <GR\LocalRegistry\LocalRegistry.h>

#include <IO/WADFileSystem.h>

#include <Debug/DebugService.h>

#include <WinSys/SubclassManager.h>



enum MasterFrameEventType
{
  MF_EVENT_NULL                 = 0x80000000,   // "Leer"-Nachricht
  MF_EVENT_SCREENMODE_CHANGED   = 0x80000001,   // Windowed/Fullscreen wurde umgeschalten
  MF_EVENT_WINDOWS_MESSAGE      = 0x80000100,   // Windows-Message vom Hauptfenster
  MF_EVENT_INPUT_MESSAGE        = 0x80000101,   // ein Event vom Input-System (MouseWheel!)
  MF_EVENT_CONSOLE_COMMAND      = 0x80000200,   // ein unbekanntes Konsolen-Kommando (iParam1 == char *)
  MF_EVENT_CONSOLE_TOGGLED      = 0x80000201,   // Konsole wurde an/ausgeschaltet
  MF_EVENT_POP_STATE            = 0x80000300,   // ein State wird gezappt
  MF_EVENT_USER                 = 0x80010000,   // eine beliebige User-Nachricht, dieser Wert bis 0xffffffff sind frei verfügbar
};

class MasterFrameEvent
{

  public:

    GR::String           m_strEvent;

    MasterFrameEventType  m_mfEvent;

    DWORD_PTR             m_dwParam1,
                          m_dwParam2,
                          m_dwParam3,
                          m_dwParam4;

    MasterFrameEvent()
      : m_mfEvent( MF_EVENT_NULL ),
        m_dwParam1( 0 ),
        m_dwParam2( 0 ),
        m_dwParam3( 0 ),
        m_dwParam4( 0 ),
        m_strEvent( "" )
    {
    }

    // Copy construktor
    MasterFrameEvent( const MasterFrameEvent& rhs ) :
      m_strEvent( rhs.m_strEvent ),
      m_dwParam1( rhs.m_dwParam1 ),
      m_dwParam2( rhs.m_dwParam2 ),
      m_dwParam3( rhs.m_dwParam3 ),
      m_dwParam4( rhs.m_dwParam4 ),
      m_mfEvent( rhs.m_mfEvent )
    {

    }

    // Zuweisungsoperator
    MasterFrameEvent& operator = ( const MasterFrameEvent& rhs )
    {
      if ( &rhs == this )
      {
        return *this;
      }
      m_strEvent = rhs.m_strEvent;
      m_dwParam1 = rhs.m_dwParam1;
      m_dwParam2 = rhs.m_dwParam2;
      m_dwParam3 = rhs.m_dwParam3;
      m_dwParam4 = rhs.m_dwParam4;
      m_mfEvent = rhs.m_mfEvent;

      return *this;

    }

    ~MasterFrameEvent()
    {
    }

};



class MasterFrame : public LocalRegistry,
                    public Console,
                    public EventListener<MasterFrameEvent>,
                    public EventProducer<MasterFrameEvent>,
                    public EventListener<Xtreme::tInputEvent>
{

  public:

    enum eFlags
    {
      MFF_FULLSCREEN        = 0x00000001,   // im Fullscreen-Modus starten
      MFF_CAPTION           = 0x00000002,   // das Hauptfenster wird mit Caption dargestellt
      MFF_COMPATIBILITY     = 0x00000004,   // spezielle DirectX-Funktionen (höher als 3) werden nicht benutzt
      MFF_PAUSED            = 0x00000008,   // hält Updateframe an
      MFF_SHOW_CONSOLE      = 0x00000010,   // Konsole wird angezeigt
      MFF_RESIZEABLE        = 0x00000020,   // Fenstergröße ist veränderbar (nur Windowed)
      MFF_HIDE_MOUSE        = 0x00000040,   // Mauszeiger abschalten
      MFF_WININPUT          = 0x00000080,   // kein DX-Input

      MFF_CONSOLE_LINES     = 50,
      MFF_VERSION           = 101,
    };

    struct DataStorage
    {
      GR::WString         m_strAppPath;
    };


  protected:

    HINSTANCE                         m_hInstance;

    HCURSOR                           m_hCursor;

    DWORD                             m_BitmapResourceID;

    GR::String                        m_CommandLine,
                                      m_AppPath,
                                      m_Caption;

    GR::String                        m_LastConsoleEntry,
                                      m_ConsoleEntry;

    bool                              m_ShutDownRequested,
                                      m_ExclusiveInput,
                                      m_DoNotFlipPages,
                                      m_DoNotAllowConsole,
                                      m_LockFPS;

    DebugService                      m_DebugService;

    LocalRegistry                     m_MasterRegistry;

    WINDOWPLACEMENT                   m_wplMain;

    Xtreme::Win32Window               m_Window;

    bool                              m_InputInitialized,
                                      m_SoundInitialized,
                                      m_FullScreen,
                                      m_UseFrameTime;      // Frame-Dauer benutzen

    GR::u32                           m_Flags,
                                      m_MouseButtons,
                                      m_Frames,
                                      m_IconID;

    float                             m_FPS;

	  GR::f64                           m_LastFrameTicks,
                                      m_ElapsedTicks,
                                      m_StartTime;

    int                               m_MouseX,
                                      m_MouseY;

    std::list<GR::String>             m_StartParameter;


  public:

    bool                              m_ApplicationActive;

    Xtreme::XInput*                   m_pInput;


    // zu überschreibende Funktionen
    virtual bool                      InitInstance() = 0;
    virtual void                      ExitInstance() = 0;

    virtual void Reset();
    virtual bool CleanUp();

    virtual bool                      ProcessEvent( const MasterFrameEvent& Event ) = 0;

    virtual bool                      ProcessEvent( const Xtreme::tInputEvent& Event );

    virtual int Run() = 0;

    void SetCommandLine( const char* szCommandLine )
    {
      m_CommandLine = szCommandLine;
      SetVar( "CommandLine", szCommandLine );
    }
    const char* CommandLine()
    {
      return m_CommandLine.c_str();
    }

    virtual void ShutDown( void );

    virtual void ToggleConsole( void );
    virtual void Pause( void );
    virtual void Resume( void );
    virtual void ProcessCommandTokens( GR::Tokenizer::TokenSequence& m_TokenSequence, GR::String& strCommand );

    void          ParseCommandLine();
    bool          ParameterSwitch( const char* szSwitch );

    GR::String    AppPath( const char *szFileName ) const;

    virtual bool  ToggleFullScreen();

    void          EnableCursor( bool Enable );
    void          AppCursor( HCURSOR hCursor );

    void          AddEvent( const char* szEvent, MasterFrameEventType mfeType = MF_EVENT_NULL, DWORD_PTR dwParam1 = 0, DWORD_PTR dwParam2 = 0, DWORD_PTR dwParam3 = 0, DWORD_PTR dwParam4 = 0 );
    void          AddEvent( MasterFrameEventType mfeType, DWORD_PTR dwParam1 = 0, DWORD_PTR dwParam2 = 0, DWORD_PTR dwParam3 = 0, DWORD_PTR dwParam4 = 0 );
    void          SendEventNow( const char* szEvent, MasterFrameEventType mfeType = MF_EVENT_NULL, DWORD_PTR dwParam1 = 0, DWORD_PTR dwParam2 = 0, DWORD_PTR dwParam3 = 0, DWORD_PTR dwParam4 = 0 );
    void          SendEventNow( MasterFrameEventType mfeType, DWORD_PTR dwParam1 = 0, DWORD_PTR dwParam2 = 0, DWORD_PTR dwParam3 = 0, DWORD_PTR dwParam4 = 0 );

    DWORD         LoadSound( const char* szFileName, bool bLoad3d = false );

    virtual HWND  GetHWND() {return m_Window.Hwnd;}
    virtual HINSTANCE GetInstance() {return m_hInstance; }

    bool          IsTimed() { return m_UseFrameTime; }
    bool          IsPaused()
    {
      return ( m_Flags & MFF_PAUSED ) ? true : false;
    }
    bool ConsoleVisible() { return (bool)!!( m_Flags & MFF_SHOW_CONSOLE ); }
    bool IsFullScreen() { return m_FullScreen; }

    GR::f64       GetFrameTime() { return m_MasterRegistry.GetVarF( "frametime" ) / 1000.0f; }
    GR::f64       GetLastFrameTime() { return m_StartTime; }

    void LockFPS( GR::u32 FPS );
    inline GR::u32 GetLockedFPS() { return (GR::u32)m_MasterRegistry.GetVarI( "lockedfps" ); }

    void          SetFrameTime( float FrameTime ) { m_MasterRegistry.SetVarF( "frametime", FrameTime * 1000.0f ); }
    void          SetLastFrameTime( GR::f64 LastFrameTime ) { m_StartTime = LastFrameTime; }
    void          SetLastDisplayFrameTime( float LastFrameTime ) { m_LastFrameTicks = LastFrameTime; }

    void SetInstance( HINSTANCE hInst ) { m_hInstance = hInst;}

    virtual bool Create( int iNewWidth, int iNewHeight, unsigned char ucDepth, DWORD dwFlags );

    virtual BOOL         WindowProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

    MasterFrame();
    virtual ~MasterFrame();

    static LRESULT FAR PASCAL           MasterFrameWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
    friend int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );


    bool                    ConfigureApp( const GR::String& strAssetFile, const GR::String& strAppPath );
    DataStorage&            Storage();

    GR::String              UserAppDataPath( const GR::String& strAppPath = GR::String() );

};



#endif // ABSTRACTMASTER_H_INCLUDED