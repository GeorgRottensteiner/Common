#pragma once



#include <GR/GameState/GUIGameState.h>

#include <Interface/IEventListener.h>
#include <Interface/IEventProducer.h>

#include <Xtreme/XTextureSection.h>

#include <Controls/GUILoader.h>
#include <Controls/Xtreme/GUIComponentDisplayer.h>

#include <Math/Spline.h>

#include <GR/Gamebase/Framework.h>

#include <Lang/EventQueue.h>

#include <debug/DebugService.h>

#include "Console.h"



class XFrameApp : public Console,
                  public EventListener<GR::Gamebase::tXFrameEvent>,
                  public IGlobalEventListener,
                  public GR::Gamebase::Framework

{

  public:

    GUIComponentDisplayer&    m_GUI;

    GUI::Loader               m_GUILoader;

    GUIGameStateManager<XRenderer,GUIComponent>      m_StateManager;

    DebugService              m_DebugService;

    bool                      m_ForceFixedSize;



  protected:

    GR::tString           m_ClassName;

    GR::u32               m_ETChangeState,
                          m_ETPushState,
                          m_ETPushStateOnStack,
                          m_ETPopState,
                          m_ETPopAnyState,
                          m_ETShutDown,
                          m_ETShuttingDown,
                          m_ETRendererInitialised,
                          m_ETRendererReleased;

    GR::f64               m_LastFrameTime,
                          m_LastFixedFrameTime,
                          m_FixedLogicTimeStep;

    GR::i32               m_CurScreenshotNumber;

    GR::String            m_ConsoleEingabe;

    GR::u32               m_IconID,
                          m_BitmapResourceID;

    XTextureSection       m_tsMouseCursor;

    XFont*                m_pConsoleFont;

    GR::tPoint            m_MouseHotSpot;

    GR::tRect             m_ClipRect;

    bool                  m_ConsoleVisible,
                          m_KeepMouseInsideDuringFullscreen;

    XRenderer*            m_pRenderClass;

    GR::tRect             m_SourceRect;
    GR::tRect             m_TargetRect;

    GR::u32               m_CustomMouseCursorColor;



    void                  DoSplashScreen();

    void                  DisplayConsole();
    void                  CreateConsoleFont();

    void                  InitialiseImplementationSpecificAssets();
    void                  UpdateAssets( GR::f32 ElapsedTime );

    void                  AdjustCanvas( int Width, int Height );

  public:


    XFrameApp();
    virtual ~XFrameApp();

    bool                  Create( GR::u32 Style, int iWidth, int iHeight, const char* szCaption,
                                  GR::u32 SplashScreenBMPResourceID = 0,
                                  GR::u32 IconResourceID = 0,
                                  GR::u32 MenuID = 0 );

    virtual bool          InitInstance();
    virtual void          ExitInstance();
    virtual int           Run();

#if OPERATING_SYSTEM == OS_WINDOWS
    HWND                  GetSafeHwnd();
#endif

    virtual void          DisplayFrame( XRenderer& Renderer );
    virtual void          UpdatePerDisplayFrame( const float ElapsedTime ) = 0;
    virtual void          UpdateFixedLogic();

    virtual bool          ProcessEvent( const GR::Gamebase::tXFrameEvent& Event );

    virtual void          ProcessEvent( const tGlobalEvent& Event );

    void                  Print( int iX, int iY, const char* szText, GR::u32 Color = 0xffffffff );

    void                  RenderCustomCursor( const GR::tPoint& ptMousePos );
    void                  SetCustomMouseCursor( const XTextureSection& TexSec = XTextureSection(), const GR::tPoint& ptHotSpot = GR::tPoint(), const GR::u32 Color = 0xffffffff );
#if OPERATING_SYSTEM == OS_WINDOWS
    void                  AppCursor( HCURSOR hCursor );
#endif

    void                  ClipCursor( const GR::tRect& rcClipRect = GR::tRect() );
    void                  ApplyClipCursor( const GR::tRect& ClipRect );

    bool                  SwitchRenderer( const char* szFileName = NULL );
    XRenderer*            Renderer();

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    static LRESULT CALLBACK XFrameAppProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    virtual LRESULT       WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

    BOOL                  SubclassWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
#endif

    void                  ToggleConsole();
    void                  ToggleFullscreen();
    void                  KeepMouseInsideDuringFullscreen( bool KeepInside );
    void                  DetermineBestFullscreenMatch( XRendererDisplayMode& Mode );

    bool                  ConsoleVisible() const;

    void                  ChangeWindowSize( int Width, int Height, int Depth );

    void                  ResetFrameTime();

    XTextureSection       Section( const GR::String& Name );
    XTexture*             Texture( const GR::String& Name );
    GR::u32               Sound( const GR::String& Name );
    XFont*                Font( const GR::String& Name );
    XVertexBuffer*        VertexBuffer( const GR::String& Name );
    XMesh*                Mesh( const GR::String& Name );
    XBoundingBox          MeshBounds( const GR::String& Name );
    CSpline               Spline( const GR::String& Name );
    IAnimationManager<GR::String>::tAnimType Animation( const GR::String& Name );
    tAnimationPos         AnimationPos( const GR::String& Name );
    XTextureSection       AnimationFrame( const tAnimationPos& AnimPos );

    XTextureSection       Tile( const GR::String& Tileset, const int TileIndex );
    GR::String            TileName( const GR::String& Tileset, const int TileIndex );
    GR::u32               TileFlags( const GR::String& Tileset, const int TileIndex );
    size_t                TilesetCount( const GR::String& Tileset );

    GR::f64               FixedTimeStep() const;

    virtual bool          RunDefaultModules();
    virtual void          ReleaseModules();

    void                  OnEnvironmentEvent( GR::Service::Environment::EnvironmentEvent Event, const char* ServiceName, const char* EventName, GR::IService* pService );

    void                  Present();

    virtual void          OnPauseApplication();
    virtual void          OnResumeApplication();

    int                   PreLoopRun();
    virtual void          RunLoop();
    int                   PostLoopRun();

    virtual void          OnKeyDown( char Key );
    virtual void          OnKeyUp( char Key );

    virtual void          OnDisplayFrame();
    virtual void          OnSizeChanged();


#if OPERATING_SYSTEM == OS_WEB
    void                  OneLoopIteration();
    static void           OneLoopIterationStub();
#endif

};






