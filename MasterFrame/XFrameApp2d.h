#ifndef XFRAMEAPP2D_H
#define XFRAMEAPP2D_H



#include <GR/GameState/GUIGameState.h>

#include <GR/LocalRegistry/LocalRegistry.h>

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <Xtreme/XInput.h>
#include <Xtreme/XTextureSection.h>

#include <Controls/GUILoader.h>
#include <Controls/Xtreme2d/GUIComponentDisplayer.h>

#include <GR/Gamebase/Framework.h>

#include <Xtreme/XMusic.h>
#include <Xtreme/XSound.h>

#include <Interface/IAnimationManager.h>

#include <debug/DebugService.h>

#include <Math/Spline.h>

#include <Lang/EventQueue.h>

#include "Console.h"


#if OPERATING_SYSTEM == OS_WEB
#include <emscripten.h>
#include <emscripten/html5.h>
#endif



struct tXFrameEvent
{
  enum eEventType
  {
    ET_INVALID = 0,
    ET_RENDERER_SWITCHED,
    ET_SOUND_SWITCHED,
    ET_DISPLAY_MODE_CHANGED,
    ET_INPUT_SWITCHED,
    ET_USER,
    ET_CONSOLE_COMMAND,
    ET_CONSOLE_TOGGLED,
    ET_SHUTDOWN,
    ET_MUSIC_PLAYER_SWITCHED,
  };

  eEventType      m_Type;

  GR::up          m_Param1,
                  m_Param2;
  GR::String     m_Text;

  tXFrameEvent( eEventType eType, GR::up Param1 = 0, GR::up Param2 = 0 ) :
    m_Type( eType ),
    m_Param1( Param1 ),
    m_Param2( Param2 ),
    m_Text( "" )
  {
  }
  tXFrameEvent( eEventType eType, const GR::String& strParam, GR::up Param1 = 0, GR::up Param2 = 0 ) :
    m_Type( eType ),
    m_Param1( Param1 ),
    m_Param2( Param2 ),
    m_Text( strParam )
  {
  }

};

class XFrameApp2d : public Console,
                    public EventListener<GR::Gamebase::tXFrameEvent>,
                    public IGlobalEventListener,
                    public GR::Gamebase::Framework
{

  protected:

    GR::u32               m_ETChangeState,
                          m_ETPushState,
                          m_ETPopState,
                          m_ETShutDown,
                          m_ETPushStateOnStack,
                          m_ETToggleAcceleration,
                          m_ETToggleMode,
                          m_ETShuttingDown;

    DebugService          m_DebugService;

    GR::f64               m_LastFrameTime,
                          m_LastFixedFrameTime,
                          m_FixedLogicTimeStep;

    GR::i32               m_CurScreenshotNumber;

    GR::u32               m_UsedWidth,
                          m_UsedHeight,
                          m_UsedDepth;

    GR::String            m_CommandLine,
                          m_ConsoleEingabe,

                          m_CurrentRenderModule,
                          m_CurrentSoundModule,
                          m_CurrentInputModule;

    GR::u32               m_CurrentRenderDepth,
                          m_MouseCursorColorKey;

    bool                  m_CurrentRenderWindowed,
                          m_RendererAccelerated,
                          m_NullSound;

    std::list<GR::String>  m_StartParameter;

    GR::u32               m_IconID;

    X2dRenderer*          m_pRenderClass;

    XTextureSection       m_tsMouseCursor;

    X2dFont*              m_pConsoleFont;

    GR::tPoint            m_MouseHotSpot;

    bool                  m_ConsoleVisible;

#if OPERATING_SYSTEM == OS_WINDOWS
    HMODULE               m_hinstCurrentRenderer;
#endif

    bool                  m_EnableCursor,
                          m_CustomMouseCursorSet;

    void                  DoSplashScreen();

    void                  DisplayConsole();
    void                  CreateConsoleFont();


  private:

    bool                      SwitchRenderer( const char* szFileName = NULL );

    void                      InitialiseImplementationSpecificAssets();
    void                      UpdateAssets( GR::f32 ElapsedTime );


  public:

    GUIComponentDisplayer&    m_GUI;

    GUI::Loader               m_GUILoader;

    GUIGameStateManager<X2dRenderer, GUIComponent>      m_StateManager;



	  XFrameApp2d();
    virtual ~XFrameApp2d();

    bool                  Create( GR::u32 Style, int iWidth, int iHeight, GR::u32 Depth,
                                  const GR::String& Caption,
                                  GR::u32 IconResourceID = 0,
                                  GR::u32 MenuID = 0 );

    virtual bool          InitInstance();
    virtual void          ExitInstance();
    virtual int           Run();

    int                   PreLoopRun();
    void                  RunLoop();
    int                   PostLoopRun();

    void                  Present();

    void                  DisplayFrame( X2dRenderer& Renderer );
    virtual void          UpdatePerDisplayFrame( const float fElapsedTime ) = 0;
    virtual void          UpdateFixedLogic();

    virtual bool          ProcessEvent( const GR::Gamebase::tXFrameEvent& Event );

    virtual void          ProcessEvent( const tGlobalEvent& Event );

    X2dRenderer*          Renderer();

    void                  Print( int iX, int iY, const char* szText, GR::u32 Color = 0xffffffff );

    void                  RenderCustomCursor( const GR::tPoint& ptMousePos );
    void                  SetCustomMouseCursor( XTextureSection& TexSec, GR::tPoint& ptHotSpot, GR::u32 TransparentColor );
#if OPERATING_SUB_SYSTEM != OS_SUB_SDL
    void                  AppCursor( HCURSOR hCursor );
    void                  SetCursor( HCURSOR hCursor = NULL );
#endif

    void                  OnEnvironmentEvent( GR::Service::Environment::EnvironmentEvent Event, const char* ServiceName, const char* EventName, GR::IService* pService );

    void                  ToggleConsole();
    bool                  ConsoleVisible() const;

    virtual bool          RunDefaultModules();
    virtual void          ReleaseModules();

    bool                  IsAccelerated() const;
    bool                  SetAcceleration( bool bAccelerate = true );

    XTextureSection       Section( const GR::String& strName );
    XTexture*             Texture( const GR::String& strName );
    X2dFont*              Font( const GR::String& strName );
    GR::u32               Sound( const GR::String& strName );
    CSpline               Spline( const GR::String& strName );
    IAnimationManager<GR::String>::tAnimType     Animation( const GR::String& strName );
    tAnimationPos         AnimationPos( const GR::String& strName );
    XTextureSection       AnimationFrame( const tAnimationPos& AnimPos );

    XTextureSection       Tile( const GR::String& Tileset, const int TileIndex );
    GR::String            TileName( const GR::String& Tileset, const int TileIndex );
    GR::u32               TileFlags( const GR::String& Tileset, const int TileIndex );
    size_t                TilesetCount( const GR::String& Tileset );

    GR::f64               FixedTimeStep() const;

    virtual void          OnCharEntered( char Key );
    virtual void          OnKeyDown( char Key );
    virtual void          OnKeyUp( char Key );

    virtual void          OnDisplayFrame();
    virtual void          OnSizeChanged();


#if OPERATING_SYSTEM == OS_WEB
    void                  OneLoopIteration();
    static void           OneLoopIterationStub();
#endif

};



#endif //XFRAMEAPP2D_H



