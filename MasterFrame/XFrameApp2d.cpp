#include <DateTime/Timer.h>

#include <Xtreme/XInput.h>

#include <Interface/IService.h>

#include <IO/FileStream.h>

#include <Misc/CloneFactory.h>

#include <Xtreme/NullSound/XNullSound.h>
#include <Xtreme/NullSound/XNullMusic.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont2d.h>
#include <Xtreme/XAsset/XAssetSound.h>
#include <Xtreme/XAsset/XAssetSpline.h>
#include <Xtreme/XAsset/XAssetAnimation.h>
#include <Xtreme/XAsset/XAssetTileset.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <String/XML.h>
#include <String/Path.h>
#include <String/Convert.h>
#include <String/StringUtil.h>

#include <Controls/Xtreme2d/GUIComponentDisplayer.h>

#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <Xtreme/SDL/SDLInput.h>
#include <SDL.h>
#endif

#include "XFrameApp2d.h"



#if OPERATING_SYSTEM == OS_WEB
#include <emscripten.h>
#include <emscripten/html5.h>

static XFrameApp2d* s_pInstance;
#endif

XFrameApp2d::XFrameApp2d() :
  m_pConsoleFont( NULL ),
  m_FixedLogicTimeStep( 0.05f ),
#if OPERATING_SYSTEM == OS_WINDOWS
  m_hinstCurrentRenderer( NULL ),
#endif
  m_pRenderClass( NULL ),
  m_IconID( 0 ),
  m_EnableCursor( true ),
  m_MouseHotSpot( 0, 0 ),
  m_CustomMouseCursorSet( false ),
  m_CurScreenshotNumber( 0 ),
  m_CurrentRenderWindowed( true ),
  m_CurrentRenderDepth( 16 ),
  m_RendererAccelerated( true ),
  m_NullSound( false ),
  m_GUILoader( GUIComponentDisplayer::Instance() ),
  m_GUI( GUIComponentDisplayer::Instance() )
{
#if OPERATING_SYSTEM == OS_WINDOWS
  m_Window.m_pFrameWork = this;
#endif

  srand( (unsigned int)time( NULL ) );

#if OPERATING_SYSTEM == OS_WEB
  s_pInstance = this;
#endif

  GR::Service::Environment::Instance().AddHandler( fastdelegate::MakeDelegate( this, &XFrameApp2d::OnEnvironmentEvent ) );

  GR::Service::Environment::Instance().SetService( "GlobalRegistry", this );
  GR::Service::Environment::Instance().SetService( "GlobalQueue", &EventQueue::Instance() );
  GR::Service::Environment::Instance().SetService( "ImageLoader", &ImageFormatManager::Instance() );

  GR::Service::Environment::Instance().SetService( "Logger", &m_DebugService );
  GR::Service::Environment::Instance().SetService( "WindowFrame", &m_RenderFrame );

  m_ETChangeState         = EventQueue::Instance().RegisterEvent( "App.ChangeState" );
  m_ETPushState           = EventQueue::Instance().RegisterEvent( "App.PushState" );
  m_ETPopState            = EventQueue::Instance().RegisterEvent( "App.PopState" );
  m_ETPushStateOnStack    = EventQueue::Instance().RegisterEvent( "App.PushStateOnStack" );

  m_ETShutDown            = EventQueue::Instance().RegisterEvent( "App.ShutDown" );

  m_ETToggleMode          = EventQueue::Instance().RegisterEvent( "App.ToggleWindowMode" );
  m_ETToggleAcceleration  = EventQueue::Instance().RegisterEvent( "App.ToggleAcceleration" );

  m_ETShuttingDown        = EventQueue::Instance().RegisterEvent( "App.Info.ShuttingDown" );

  m_GUI.m_pEventProducer = this;

  m_GUI.AddListener( &m_StateManager );

  EventQueue::Instance().AddListener( this );
}



XFrameApp2d::~XFrameApp2d()
{
  m_GUI.m_Background.Clear();

  EventQueue::Instance().UnregisterEvent( m_ETChangeState );
  EventQueue::Instance().UnregisterEvent( m_ETPushState );
  EventQueue::Instance().UnregisterEvent( m_ETPopState );
  EventQueue::Instance().UnregisterEvent( m_ETPushStateOnStack );

  EventQueue::Instance().UnregisterEvent( m_ETShutDown );

  EventQueue::Instance().UnregisterEvent( m_ETToggleMode );
  EventQueue::Instance().UnregisterEvent( m_ETToggleAcceleration );

  EventQueue::Instance().UnregisterEvent( m_ETShuttingDown );

  GR::Service::Environment::Instance().RemoveService( "GlobalRegistry" );
  GR::Service::Environment::Instance().RemoveService( "GlobalQueue" );
  GR::Service::Environment::Instance().RemoveService( "ImageLoader" );
  GR::Service::Environment::Instance().RemoveService( "Logger" );
  GR::Service::Environment::Instance().RemoveService( "WindowFrame" );

  GR::Service::Environment::Instance().RemoveHandler( fastdelegate::MakeDelegate( this, &XFrameApp2d::OnEnvironmentEvent ) );
}



bool XFrameApp2d::InitInstance()
{
  if ( m_pRenderClass )
  {
    m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
  }
  return true;
}



void XFrameApp2d::ExitInstance()
{
  EventProducer<GR::Gamebase::tXFrameEvent>::RemoveListener( this );
}



void XFrameApp2d::UpdateFixedLogic()
{
}



X2dRenderer* XFrameApp2d::Renderer()
{

  return m_pRenderClass;

}



void XFrameApp2d::DoSplashScreen()
{
  /*
  // Splash-Screen
  if ( m_BitmapResourceID )
  {
    HBITMAP   hbm = LoadBitmap( m_hInstance, MAKEINTRESOURCE( m_BitmapResourceID ) );
    if ( hbm )
    {
      HDC   hdcMain = GetDC( m_RenderFrame.m_hwndMain );
      HDC   hdcBitmap = CreateCompatibleDC( hdcMain );

      SetStretchBltMode( hdcMain, COLORONCOLOR );

      BITMAP    bm;
      GetObject( hbm, sizeof( BITMAP ), &bm );

      SelectObject( hdcBitmap, hbm );

      RECT    rc;
      GetClientRect( m_RenderFrame.m_hwndMain, &rc );
      StretchBlt( hdcMain, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                  hdcBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

      DeleteDC( hdcBitmap );
      ReleaseDC( m_RenderFrame.m_hwndMain, hdcMain );

      DeleteObject( hbm );
    }
    UpdateWindow( m_RenderFrame.m_hwndMain );
  }
  else*/

#if OPERATING_SYSTEM == OS_WINDOWS
  {
    // loading...
    HDC   hdcMain = GetDC( m_RenderFrame.m_hwndMain );

    RECT  rc;
    GetClientRect( m_RenderFrame.m_hwndMain, &rc );
    FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

    SelectObject( hdcMain, GetStockObject( DEFAULT_GUI_FONT ) );

    SetTextColor( hdcMain, 0xffffff );
    SetBkMode( hdcMain, TRANSPARENT );
    DrawTextA( hdcMain, "loading...", 10, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

    ReleaseDC( m_RenderFrame.m_hwndMain, hdcMain );
  }
#endif
}



void XFrameApp2d::ReleaseModules()
{
  if ( m_pRenderClass )
  {
    m_pRenderClass->Release();
    GR::Service::Environment::Instance().RemoveService( "Renderer" );
    m_pRenderClass = NULL;
#if OPERATING_SYSTEM == OS_WINDOWS
    if ( m_hinstCurrentRenderer )
    {
      FreeLibrary( m_hinstCurrentRenderer );
      m_hinstCurrentRenderer = NULL;
    }
#endif
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  // combined music/sound player
  if ( ( m_pSoundClass != NULL )
  &&   ( m_pMusicPlayer != NULL ) )
  {
    m_pMusicPlayer->Stop();
    m_pMusicPlayer = NULL;
    GR::Service::Environment::Instance().RemoveService( "Music" );
  }
#endif

  GR::Gamebase::Framework::ReleaseModules();
}



bool XFrameApp2d::SwitchRenderer( const char* szFileName )
{
  GR::Service::Environment::Instance().RemoveService( "Renderer" );
  if ( m_pRenderClass )
  {
    m_pRenderClass->Release();
  }
  m_pRenderClass = NULL;
#if OPERATING_SYSTEM == OS_WINDOWS
  if ( m_hinstCurrentRenderer )
  {
    FreeLibrary( m_hinstCurrentRenderer );
    m_hinstCurrentRenderer = NULL;
  }
#endif

  if ( szFileName == NULL )
  {
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
    return false;
  }

#if OPERATING_SYSTEM == OS_WINDOWS
  m_hinstCurrentRenderer = LoadLibraryA( szFileName );
  if ( m_hinstCurrentRenderer )
  {
    typedef X2dRenderer* (*tIFunction)( void );
    tIFunction    fGetInterface = (tIFunction)GetProcAddress( m_hinstCurrentRenderer, "GetInterface" );

    if ( fGetInterface )
    {
      m_pRenderClass = fGetInterface();

      GR::Service::Environment::Instance().SetService( "Renderer", m_pRenderClass );
    }
  }
  else
  {
    return SwitchRenderer();
  }
#endif

  if ( !m_pRenderClass->Initialize( m_UsedWidth, m_UsedHeight, m_UsedDepth, !m_CurrentRenderWindowed, GR::Service::Environment::Instance() ) )
  {
    return SwitchRenderer();
  }
  m_RenderFrame.SetSize( m_UsedWidth, m_UsedHeight );

  // 2d - no scaling, always full screen 1:1
  m_GUI.SetScreenSizes( GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ), GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) );
  m_GUI.SetRenderer( m_pRenderClass );

  EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
  return true;
}



bool XFrameApp2d::ProcessEvent( const GR::Gamebase::tXFrameEvent& Event )
{
  switch ( Event.m_Type )
  {
    case GR::Gamebase::tXFrameEvent::ET_CONSOLE_COMMAND:
      if ( Event.m_Text == "quit" )
      {
        ShutDown();
      }
      else if ( Event.m_Text.find( "=" ) != GR::String::npos )
      {
        size_t    iPos = Event.m_Text.find( "=" );

        if ( iPos > 0 )
        {
          GR::String     strVarName = Event.m_Text.substr( 0, iPos );
          GR::String     strVarValue = Event.m_Text.substr( iPos + 1 );

          SetVar( strVarName, strVarValue );

          ConsolePrint( "%s set to %s", strVarName.c_str(), strVarValue.c_str() );
        }
      }
      else if ( Event.m_Text.find( "SwitchRenderer " ) == 0 )
      {
        SwitchRenderer( Event.m_Text.substr( 15 ).c_str() );
      }
      else if ( Event.m_Text.find( "set" ) != GR::String::npos )
      {
        LocalRegistry::tMapVars::iterator    itVar( m_Vars.begin() );
        while ( itVar != m_Vars.end() )
        {
          ConsolePrint( "%s=%s", itVar->first.c_str(), itVar->second.c_str() );

          ++itVar;
        }
      }
      break;
    case GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED:
      m_pConsoleFont = NULL;
      break;
  }
  return false;
}



int XFrameApp2d::PreLoopRun()
{
  EventQueue::Instance().ProcessQueue();

  // cannot run first with emscripen since that would block anything below
  if ( !Create( 0,
                m_EnvironmentConfig.StartUpWidth,
                m_EnvironmentConfig.StartUpHeight,
                32,
                m_EnvironmentConfig.Caption,
                0,
                m_EnvironmentConfig.IconResourceID ) )
  {
    dh::Log( "PreLoopRun create failed" );
    return 1;
  }

  GR::IO::FileStream  ioIn( UserAppDataPath( "xtreme.cfg" ).c_str() );
  Load( ioIn );

  m_EnvironmentDisplayRect.set( 0, 0, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  if ( m_EnvironmentConfig.FixedSize )
  {
    // can't override resolution from previous save, reset to startup size
    m_RenderFrame.SetSize( m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  }
  else
  {
    m_RenderFrame.SetSize( m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
    m_EnvironmentDisplayRect.set( 0, 0, m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
  }
  std::list<GR::String>::iterator    itCL( m_StartParameter.begin() );
  while ( itCL != m_StartParameter.end() )
  {
    const GR::String& param( *itCL );

    if ( param.find( "/log:" ) == 0 )
    {
      GR::String   strLog = param.substr( 5 );

      std::list<GR::String>    listLogParams;

      GR::Strings::Split( strLog, ',', listLogParams );

      std::list<GR::String>::iterator    it( listLogParams.begin() );
      while ( it != listLogParams.end() )
      {
        m_DebugService.LogEnable( it->c_str() );

        ++it;
      }
    }
    else if ( param.find( "/logtofile:" ) == 0 )
    {
      GR::String   strLog = param.substr( 11 );

      std::list<GR::String>    listLogParams;

      GR::Strings::Split( strLog, ',', listLogParams );

      std::list<GR::String>::iterator    it( listLogParams.begin() );
      while ( it != listLogParams.end() )
      {
        m_DebugService.LogEnable( it->c_str() );
        m_DebugService.LogToFile( it->c_str() );

        ++it;
      }
    }
    ++itCL;
  }

  DoSplashScreen();

  ConsolePrint( "XFrameApp Xtreme 1.0 InitInstance done" );

  EventProducer<GR::Gamebase::tXFrameEvent>::AddListener( this );

  if ( !InitInstance() )
  {
    dh::Log( "InitInstance failed" );
    ReleaseAssets();
    SwitchRenderer( NULL );
    SwitchInput();
    SwitchSound();
    SwitchMusic();
    return 1;
  }

  m_LastFrameTime = Time::Timer::Time();
  m_LastFixedFrameTime = m_LastFrameTime;

#if OPERATING_SYSTEM == OS_WEB
  emscripten_pause_main_loop();
  m_ApplicationActive = true;
  emscripten_set_main_loop_timing( EM_TIMING_RAF, 1 );

  emscripten_set_main_loop( OneLoopIterationStub, 0, 1 );
  emscripten_resume_main_loop();
#endif

  return m_ExitCode;
}



void XFrameApp2d::Present()
{
  if ( m_pRenderClass != NULL )
  {
    m_pRenderClass->PresentScene( NULL, NULL );
  }
}



void XFrameApp2d::OnDisplayFrame()
{
  if ( m_pRenderClass )
  {
    if ( m_pRenderClass->BeginScene() )
    {
      DisplayFrame( *m_pRenderClass );

      if ( m_pInputClass )
      {
        RenderCustomCursor( m_pInputClass->MousePos() );
      }

      if ( m_ConsoleVisible )
      {
        DisplayConsole();
      }

      m_pRenderClass->EndScene();
      Present();
    }
  }
  else
  {
    if ( m_ConsoleVisible )
    {
      DisplayConsole();
    }
    else
    {
      DoSplashScreen();
    }
  }
}



#if OPERATING_SYSTEM == OS_WEB
void XFrameApp2d::OneLoopIterationStub()
{
  s_pInstance->OneLoopIteration();
}



void XFrameApp2d::OneLoopIteration()
{
  SDL_Event   sdlEvent;

  if ( !m_ApplicationActive )
  {
    while ( SDL_PollEvent( &sdlEvent ) != 0 )
    {
      //User requests quit
      if ( sdlEvent.type == SDL_QUIT )
      {
        m_ShutDown = true;
      }
      else if ( sdlEvent.type == SDL_WINDOWEVENT )
      {
        switch ( sdlEvent.window.event )
        {
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            SetActive( true );
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            SetActive( false );
            break;
          default:
            dh::Log( "unsupported window event %d", sdlEvent.window.event );
        }
      }
      else
      {
        dh::Log( "unsupported SDL event %x", sdlEvent.type );
      }
    }
  }
  else
  {
    while ( SDL_PollEvent( &sdlEvent ) != 0 )
    {
      //User requests quit
      if ( sdlEvent.type == SDL_QUIT )
      {
        m_ShutDown = true;
      }
      if ( ( sdlEvent.type >= SDL_KEYDOWN )
      &&   ( sdlEvent.type < SDL_CLIPBOARDUPDATE ) )
      {
        ( (SDLInput*)m_pInputClass )->OnSDLEvent( sdlEvent );
      }
    }

    GR::f64   fNewFrameTime = Time::Timer::Time();
    GR::f32   ElapsedTime = ( GR::f32 )Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
    GR::f32   fElapsedFixedTime = ( GR::f32 )( GR::f64 )( fNewFrameTime - m_LastFixedFrameTime );

    //dh::Log( "fNewFrameTime %lf.6", fNewFrameTime );

    if ( ElapsedTime > 10.0f )
    {
      ElapsedTime = 0.0f;
    }
    //dh::Log( "TTElapsedTime %.6f", elapsedTime );
    //dh::Log( "ElapsedTime %.6f", ElapsedTime );

    if ( m_ConsoleVisible )
    {
      ElapsedTime = 0.0f;
      m_LastFixedFrameTime = fNewFrameTime;
      m_LastFrameTime = fNewFrameTime;
    }

    int     iSafetyLoopCounter = 0;
    while ( fElapsedFixedTime >= m_FixedLogicTimeStep )
    {
      iSafetyLoopCounter++;
      if ( iSafetyLoopCounter > 25 )
      {
        // Safety-Break um eventuelle Endlosloops zu verhindern
        m_LastFixedFrameTime = fNewFrameTime;
        break;
      }
      m_StateManager.UpdateCurrentState();
      UpdateFixedLogic();
      EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
      fElapsedFixedTime -= ( GR::f32 )m_FixedLogicTimeStep;
      m_LastFixedFrameTime += m_FixedLogicTimeStep;
    }
    m_LastFrameTime = fNewFrameTime;

    if ( m_pInputClass )
    {
      m_pInputClass->Update( ElapsedTime );
    }

    if ( ( m_pRenderClass )
    &&   ( m_pInputClass ) )
    {
      if ( m_pInputClass->ReleasedKeyPressed( Xtreme::KEY_F3 ) )
      {
        ++m_CurScreenshotNumber;
        GR::String     screenShotFile = Misc::Format( "shot%1%.tga" ) << m_CurScreenshotNumber;
        m_pRenderClass->SaveScreenShot( screenShotFile.c_str() );
      }
    }
    m_StateManager.UpdateStatesPerDisplayFrame( ElapsedTime );
    UpdatePerDisplayFrame( ElapsedTime );
    UpdateAssets( ElapsedTime );
    EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
    EventQueue::Instance().ProcessQueue();
    OnDisplayFrame();
  }
}
#endif



void XFrameApp2d::RunLoop()
{
#if OPERATING_SYSTEM == OS_WEB
  // nothing to do
#else


  while ( !m_ShutDown )
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    MSG   msg;
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
    SDL_Event   sdlEvent;
#elif OPERATING_SYSTEM == OS_ANDROID
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;
#endif

    if ( !m_ApplicationActive )
    {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessOneAndAllPending );
#elif OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      BOOL    bRet = GetMessage( &msg, NULL, 0, 0 );
      if ( bRet == 0 )
      {
        break;
      }
      else if ( bRet == -1 )
      {
        // Fehler!
        break;
      }
      if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
      {
        continue;
      }
      TranslateMessage( &msg );
      DispatchMessage( &msg );
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
      while ( SDL_PollEvent( &sdlEvent ) != 0 )
      {
        //User requests quit
        if ( sdlEvent.type == SDL_QUIT )
        {
          m_ShutDown = true;
        }
        else if ( sdlEvent.type == SDL_WINDOWEVENT )
        {
          switch ( sdlEvent.window.event )
          {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
              SetActive( true );
              break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
              SetActive( false );
              break;
            default:
              dh::Log( "unsupported window event %d", sdlEvent.window.event );
          }
        }
        else
        {
          dh::Log( "unsupported SDL event %x", sdlEvent.type );
        }
      }
#elif OPERATING_SYSTEM == OS_ANDROID
      while ( ( ident = ALooper_pollAll( m_ApplicationActive ? 0 : -1, NULL, &events, (void**)&source ) ) >= 0 )
      {
        // Process this event.
        if ( source != NULL )
        {
          //source->process( state, source );
          source->ProcessHandler();
        }

        // If a sensor has data, process it now.
        if ( ident == LOOPER_ID_USER )
        {
          AndroidInput* pInput = (AndroidInput*)m_pInputClass;

          pInput->Poll();
        }

        /*
        // Check if we are exiting.
        if ( destroyRequested != 0 )
        {
          engine_term_display( &engine );
          return;
        }*/
      }
#endif
    }
    else
    {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( CoreProcessEventsOption::ProcessAllIfPresent );
#elif OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
      {
        BOOL    bRet = GetMessage( &msg, NULL, 0, 0 );

        if ( bRet == 0 )
        {
          goto main_loop_done;
        }
        else if ( bRet == -1 )
        {
          // Fehler!
          goto main_loop_done;
        }
        else
        {
          ::TranslateMessage( &msg );
          ::DispatchMessage( &msg );
        }
      }
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
      while ( SDL_PollEvent( &sdlEvent ) != 0 )
      {
        //User requests quit
        if ( sdlEvent.type == SDL_QUIT )
        {
          m_ShutDown = true;
        }
        if ( ( sdlEvent.type >= SDL_KEYDOWN )
        &&   ( sdlEvent.type < SDL_CLIPBOARDUPDATE ) )
        {
          ( (SDLInput*)m_pInputClass )->OnSDLEvent( sdlEvent );
        }
      }
#elif OPERATING_SYSTEM == OS_ANDROID
      while ( ( ident = ALooper_pollAll( m_ApplicationActive ? 0 : -1, NULL, &events, (void**)&source ) ) >= 0 )
      {
        // Process this event.
        if ( source != NULL )
        {
          //source->process( state, source );
          source->ProcessHandler();
        }

        // If a sensor has data, process it now.
        if ( ident == LOOPER_ID_USER )
        {
          AndroidInput* pInput = (AndroidInput*)m_pInputClass;

          pInput->Poll();
        }

        /*
        // Check if we are exiting.
        if ( destroyRequested != 0 )
        {
        engine_term_display( &engine );
        return;
        }*/
      }
#endif
      GR::f64   fNewFrameTime = Time::Timer::Time();
      GR::f32   ElapsedTime = ( GR::f32 )Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
      GR::f32   fElapsedFixedTime = ( GR::f32 )( GR::f64 )( fNewFrameTime - m_LastFixedFrameTime );

      if ( ElapsedTime > 10.0f )
      {
        ElapsedTime = 0.0f;
      }
      //dh::Log( "TTElapsedTime %.6f", elapsedTime );
      //dh::Log( "ElapsedTime %.6f", ElapsedTime );

      if ( m_ConsoleVisible )
      {
        ElapsedTime = 0.0f;
        m_LastFixedFrameTime = fNewFrameTime;
        m_LastFrameTime = fNewFrameTime;
      }

      int     iSafetyLoopCounter = 0;
      while ( fElapsedFixedTime >= m_FixedLogicTimeStep )
      {
        iSafetyLoopCounter++;
        if ( iSafetyLoopCounter > 25 )
        {
          // Safety-Break um eventuelle Endlosloops zu verhindern
          m_LastFixedFrameTime = fNewFrameTime;
          break;
        }
        m_StateManager.UpdateCurrentState();
        UpdateFixedLogic();
        EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
        fElapsedFixedTime -= ( GR::f32 )m_FixedLogicTimeStep;
        m_LastFixedFrameTime += m_FixedLogicTimeStep;
      }
      m_LastFrameTime = fNewFrameTime;

      if ( m_pInputClass )
      {
        m_pInputClass->Update( ElapsedTime );
      }

      if ( ( m_pRenderClass )
        && ( m_pInputClass ) )
      {
        if ( m_pInputClass->ReleasedKeyPressed( Xtreme::KEY_F3 ) )
        {
          ++m_CurScreenshotNumber;
          GR::String     screenShotFile = Misc::Format( "shot%1%.tga" ) << m_CurScreenshotNumber;
          m_pRenderClass->SaveScreenShot( screenShotFile.c_str() );
        }
      }
      m_StateManager.UpdateStatesPerDisplayFrame( ElapsedTime );
      UpdatePerDisplayFrame( ElapsedTime );
      UpdateAssets( ElapsedTime );
      EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
      EventQueue::Instance().ProcessQueue();

      OnDisplayFrame();
    }
    if ( m_ShutDown )
    {
      GLOBAL_QUEUE.SendEvent( "App.Info.ShuttingDown" );

      m_StateManager.PopAllStates();
      EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_SHUTDOWN ) );

      GR::IO::FileStream    ioOut( UserAppDataPath( "xtreme.cfg" ).c_str(), IIOStream::OT_WRITE_ONLY );
      Save( ioOut );

      m_GUI.DeleteAllControls();

      if ( m_pRenderClass )
      {
        m_pRenderClass->ReleaseAssets();
      }

      ReleaseModules();

      ReleaseAssets();

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      DestroyWindow( m_RenderFrame.m_hwndMain );
#endif
    }
  }
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  main_loop_done : ;
#endif

#endif
}



//#include <Xtreme/Environment/AndroidMain.h>
int XFrameApp2d::Run()
{
  /*
  AAssetDir* assetDir = AAssetManager_openDir( AndroidMediator::s_pActivity->assetManager, "" );

  const char* filename;
  std::vector<char> buffer;

  while ( ( filename = AAssetDir_getNextFileName( assetDir ) ) != NULL )
  {
    dh::Log( "filename %s", filename );
  }*/

  if ( ( !m_AssetProjectToLoad.empty() )
  &&   ( !Xtreme::Asset::XAssetLoader::Instance().LoadAssets( this->AppPath( m_AssetProjectToLoad ).c_str() ) ) )
  {
    return -1;
  }

  int result = PreLoopRun();
  if ( result != 0 )
  {
    return result;
  }

  RunLoop();

  return PostLoopRun();
}



int XFrameApp2d::PostLoopRun()
{
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( m_Window.HMenu )
  {
    DestroyMenu( m_Window.HMenu );
    m_Window.HMenu = NULL;
  }
#endif
  ReleaseModules();
  return m_ExitCode;
}



bool XFrameApp2d::Create( GR::u32 Style, int iWidth, int iHeight, GR::u32 Depth, const GR::String& Caption,
                          GR::u32 IconResourceID,
                          GR::u32 MenuID )
{
  m_IconID            = IconResourceID;

  m_UsedWidth   = iWidth;
  m_UsedHeight  = iHeight;
  m_UsedDepth   = Depth;

  if ( !m_Window.Create( m_EnvironmentConfig ) )
  {
    return false;
  }

  // make sure the Component Displayer is registered as service
  GUIComponentDisplayer::Instance();

  DoSplashScreen();

  EventProducer<GR::Gamebase::tXFrameEvent>::AddListener( this );

  InitialiseAssets();
  InitialiseImplementationSpecificAssets();
  return true;
}



void XFrameApp2d::InitialiseImplementationSpecificAssets()
{
  Xtreme::Asset::XAssetLoader*    pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
  }

  GR::Strings::XMLElement*    pXMLGUIGeneral = pLoader->AssetInfo( Xtreme::Asset::XA_GUI, -1 );
  if ( pXMLGUIGeneral )
  {
    // general GUI settings
    m_pTableGUIText = m_TextDB.GetTable( pXMLGUIGeneral->Attribute( "TextTable" ) );
    m_GUILoader.Initialize( pXMLGUIGeneral, m_pTableGUIText, this );
  }

  // init tileset animations
  int     numTilesets = pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET );
  for ( int i = 0; i < numTilesets; ++i )
  {
    Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, i );
    if ( pTileset == NULL )
    {
      continue;
    }
    for ( size_t j = 0; j < pTileset->m_Tiles.size(); ++j )
    {
      if ( !pTileset->m_Tiles[j].Animation.empty() )
      {
        pTileset->m_Tiles[j].AnimPos = AnimationPos( pTileset->m_Tiles[j].Animation );
      }
    }
  }
}



void XFrameApp2d::UpdateAssets( GR::f32 ElapsedTime )
{
  Xtreme::Asset::XAssetLoader*    pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
  }
  int     numTilesets = pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET );
  for ( int i = 0; i < numTilesets; ++i )
  {
    Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, i );
    if ( pTileset == NULL )
    {
      continue;
    }
    for ( size_t j = 0; j < pTileset->m_Tiles.size(); ++j )
    {
      if ( !pTileset->m_Tiles[j].Animation.empty() )
      {
        m_AnimationManager.AdvanceAnimFrame( pTileset->m_Tiles[j].AnimPos, ElapsedTime * 1000.0f );
      }
    }
  }
}



void XFrameApp2d::SetCustomMouseCursor( XTextureSection& TexSec, GR::tPoint& ptHotSpot, GR::u32 TransparentColor )
{
  m_tsMouseCursor   = TexSec;
  m_MouseHotSpot  = ptHotSpot;
  m_MouseCursorColorKey = TransparentColor;

  m_CustomMouseCursorSet = ( m_tsMouseCursor.m_Width != 0 ) && ( m_tsMouseCursor.m_Height != 0 );
}



void XFrameApp2d::RenderCustomCursor( const GR::tPoint& ptMousePos )
{
  if ( ( m_CustomMouseCursorSet )
  &&   ( m_pRenderClass ) )
  {
    GR::tPoint    ptOffset = ptMousePos - m_MouseHotSpot;
    m_pRenderClass->RenderTextureSectionColorKeyed( ptOffset.x, ptOffset.y,
                                                    m_tsMouseCursor,
                                                    m_MouseCursorColorKey );
  }
}



void XFrameApp2d::ToggleConsole()
{
  m_ConsoleVisible = !m_ConsoleVisible;
  if ( m_ConsoleVisible )
  {
#if OPERATING_SYSTEM == OS_WINDOWS
    if ( m_pRenderClass == NULL )
    {
      InvalidateRect( m_RenderFrame.m_hwndMain, NULL, FALSE );
    }
#endif
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_CONSOLE_TOGGLED, 1 ) );
  }
  else
  {
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_CONSOLE_TOGGLED, 0 ) );
  }
}



void XFrameApp2d::DisplayConsole()
{
  if ( m_pRenderClass )
  {
    if ( m_pConsoleFont == NULL )
    {
      // neuen Font erzeugen
      CreateConsoleFont();
    }
    else
    {
      int     y = m_RenderFrame.m_DisplayMode.Height - 30;

      std::list<GR::String>::reverse_iterator    it( m_listConsoleLines.rbegin() );
      while ( it != m_listConsoleLines.rend() )
      {
        GR::String&    strLine = *it;

        m_pRenderClass->RenderText( m_pConsoleFont, 4, y, strLine, 0xffff00ff );

        y -= 14;
        if ( y < -14 )
        {
          break;
        }

        ++it;
      }
      m_pRenderClass->RenderText( m_pConsoleFont, 4, 465, ( m_ConsoleEingabe + "_" ) );
    }
  }
  else
  {
#if OPERATING_SYSTEM == OS_WINDOWS

    HDC   hdcMain = GetDC( m_RenderFrame.m_hwndMain );

    RECT  rc;
    GetClientRect( m_RenderFrame.m_hwndMain, &rc );
    //FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

    SelectObject( hdcMain, GetStockObject( DEFAULT_GUI_FONT ) );

    SetTextColor( hdcMain, 0xffffff );
    SetBkMode( hdcMain, TRANSPARENT );

    int     iY = rc.bottom - 30;

    std::list<GR::String>::reverse_iterator    it( m_listConsoleLines.rbegin() );
    while ( it != m_listConsoleLines.rend() )
    {
      GR::String&    strLine = *it;

      rc.top = iY;
      rc.bottom = iY + 14;

      DrawTextA( hdcMain, strLine.c_str(), (int)strLine.length(), &rc, DT_LEFT | DT_SINGLELINE );

      iY -= 14;
      if ( iY < -14 )
      {
        break;
      }

      ++it;
    }

    rc.top = 465;
    rc.bottom = rc.top + 14;
    DrawTextA( hdcMain, ( m_ConsoleEingabe + "_" ).c_str(), (int)m_ConsoleEingabe.length() + 1, &rc, DT_LEFT | DT_SINGLELINE );

    ReleaseDC( m_RenderFrame.m_hwndMain, hdcMain );
#endif
  }
}



void XFrameApp2d::CreateConsoleFont()
{
  if ( m_pConsoleFont )
  {
    return;
  }

  if ( m_pRenderClass == NULL )
  {
    return;
  }
  m_pConsoleFont = m_pRenderClass->CreateFontSquare();
}



void XFrameApp2d::Print( int iX, int iY, const char* szText, GR::u32 Color )
{
  if ( m_pRenderClass == NULL )
  {
    return;
  }
  CreateConsoleFont();

  if ( m_pConsoleFont )
  {
    m_pRenderClass->RenderText( m_pConsoleFont, iX, iY, szText, Color );
  }
}



void XFrameApp2d::ProcessEvent( const tGlobalEvent& Event )
{
  if ( Event.m_Type == m_ETChangeState )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      GR::Strings::ParameterList        paramList( Event.m_Params );

      GR::String           strParam = Event.m_Param;

      size_t    iParamPos = Event.m_Param.find( '(' );
      if ( iParamPos != GR::String::npos )
      {
        size_t    iParamEndPos = Event.m_Param.find( ')', iParamPos );

        if ( ( iParamEndPos != GR::String::npos )
        &&   ( iParamEndPos > iParamPos ) )
        {
          std::list<GR::String>    listParams;

          GR::Strings::Split( Event.m_Param.substr( iParamPos + 1, iParamEndPos - iParamPos - 1 ), ',', listParams );

          std::list<GR::String>::iterator    it( listParams.begin() );
          while ( it != listParams.end() )
          {
            GR::String    strParam( GR::Strings::Trim( *it ) );

            size_t    iEqualPos = strParam.find( '=' );
            if ( iEqualPos != GR::String::npos )
            {
              paramList.SetParam( strParam.substr( 0, iEqualPos ), strParam.substr( iEqualPos + 1 ) );
            }

            ++it;
          }
        }
        strParam = Event.m_Param.substr( 0, iParamPos );
      }

      // State erzeugen
      ICloneAble*   pClone = CloneFactory::Instance().CreateObject( strParam );
      GUIGameState<X2dRenderer, GUIComponent>*    pGameState = static_cast<GUIGameState<X2dRenderer, GUIComponent>*>( pClone );
      if ( pGameState )
      {
        pGameState->m_ParamInit = paramList;
        m_StateManager.ChangeState( pGameState );
      }
      else
      {
        dh::Log( "XFrameApp: Trying to change to unregistered state %s", Event.m_Param.c_str() );
      }
    }
    else
    {
      m_StateManager.ChangeState( (GUIGameState<X2dRenderer, GUIComponent>*)Event.m_Param1 );
    }
  }
  else if ( Event.m_Type == m_ETPopState )
  {
    m_StateManager.PopState();
  }
  else if ( ( Event.m_Type == m_ETPushState )
  ||        ( Event.m_Type == m_ETPushStateOnStack ) )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      GR::Strings::ParameterList        paramList( Event.m_Params );

      GR::String           strParam = Event.m_Param;

      size_t    iParamPos = Event.m_Param.find( '(' );
      if ( iParamPos != GR::String::npos )
      {
        size_t    iParamEndPos = Event.m_Param.find( ')', iParamPos );

        if ( ( iParamEndPos != GR::String::npos )
        &&   ( iParamEndPos > iParamPos ) )
        {
          std::list<GR::String>    listParams;

          GR::Strings::Split( Event.m_Param.substr( iParamPos + 1, iParamEndPos - iParamPos - 1 ), ',', listParams );

          std::list<GR::String>::iterator    it( listParams.begin() );
          while ( it != listParams.end() )
          {
            GR::String    strParam( GR::Strings::Trim( *it ) );

            size_t    iEqualPos = strParam.find( '=' );
            if ( iEqualPos != GR::String::npos )
            {
              paramList.SetParam( strParam.substr( 0, iEqualPos ), strParam.substr( iEqualPos + 1 ) );
            }

            ++it;
          }
        }
        strParam = Event.m_Param.substr( 0, iParamPos );
      }

      ICloneAble*   pClone = CloneFactory::Instance().CreateObject( strParam );
      GUIGameState<X2dRenderer, GUIComponent>*    pGameState = static_cast<GUIGameState<X2dRenderer, GUIComponent>*>( pClone );
      if ( pGameState )
      {
        pGameState->m_ParamInit = paramList;
        if ( Event.m_Type == m_ETPushStateOnStack )
        {
          m_StateManager.PushStateOnStack( pGameState );
        }
        else
        {
          m_StateManager.PushState( pGameState );
        }
      }
      else
      {
        dh::Log( "XFrameApp: Trying to push unregistered state %s", Event.m_Param.c_str() );
      }
    }
    else
    {
      if ( Event.m_Type == m_ETPushStateOnStack )
      {
        m_StateManager.PushStateOnStack( (GUIGameState<X2dRenderer, GUIComponent>*)Event.m_Param1 );
      }
      else
      {
        m_StateManager.PushState( (GUIGameState<X2dRenderer,GUIComponent>*)Event.m_Param1 );
      }
    }
  }
  else if ( Event.m_Type == m_ETShutDown )
  {
    ShutDown( (GR::u32)Event.m_Param1 );
  }
  else if ( Event.m_Type == m_ETToggleMode )
  {
    if ( m_pRenderClass )
    {
      m_pRenderClass->ToggleFullscreen();

      m_CurrentRenderWindowed = !m_pRenderClass->IsFullscreen();
    }
  }
  else if ( Event.m_Type == m_ETToggleAcceleration )
  {
    SetAcceleration( !IsAccelerated() );
  }
}



bool XFrameApp2d::ConsoleVisible() const
{
  return m_ConsoleVisible;
}



#if OPERATING_SUB_SYSTEM != OS_SUB_SDL
void XFrameApp2d::AppCursor( HCURSOR hCursor )
{
  m_Window.HCursor = hCursor;

  POINT   pt;
  GetCursorPos( &pt );

  if ( ::SendMessage( m_RenderFrame.m_hwndMain, WM_NCHITTEST, 0, MAKELPARAM( pt.x, pt.y ) ) == HTCLIENT )
  {
    SetCursor( m_Window.HCursor );
  }
}



void XFrameApp2d::SetCursor( HCURSOR hCursor )
{
  if ( m_Window.HCursor )
  {
    DestroyCursor( m_Window.HCursor );
  }
  m_Window.HCursor = hCursor;

  if ( m_Window.HCursor == NULL )
  {
    m_Window.HCursor = LoadCursor( NULL, IDC_ARROW );
  }
  ::SetCursor( m_Window.HCursor );
}



#endif



bool XFrameApp2d::RunDefaultModules()
{
  if ( m_RenderFrame.m_DisplayMode.Width == 0 )
  {
    // default modes
    m_RenderFrame.m_DisplayMode.Width         = 640;
    m_RenderFrame.m_DisplayMode.Height        = 480;
    m_RenderFrame.m_DisplayMode.ImageFormat   = GR::Graphic::ImageData::ImageFormatFromDepth( 16 );
    m_RenderFrame.m_DisplayMode.FullScreen    = false;
  }

  X2dRenderer* pExistingRenderer = (X2dRenderer*)GR::Service::Environment::Instance().Service( "Renderer" );
  if ( pExistingRenderer != NULL )
  {
    m_pRenderClass = pExistingRenderer;
    if ( !m_pRenderClass->Initialize( m_UsedWidth, m_UsedHeight, m_UsedDepth, !m_CurrentRenderWindowed, GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    m_RenderFrame.SetSize( m_UsedWidth, m_UsedHeight );
    m_GUI.SetScreenSizes( GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ), GR::tPoint( 640, 480 ) );

    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
  }

  Xtreme::XInput* pExistingInput = ( Xtreme::XInput*)GR::Service::Environment::Instance().Service( "Input" );
  if ( pExistingInput != NULL )
  {
    m_pInputClass = pExistingInput;
    if ( !m_pInputClass->Initialize( GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_INPUT_SWITCHED ) );
  }

  XSound* pExistingSound = ( XSound* )GR::Service::Environment::Instance().Service( "Sound" );
  if ( pExistingSound != NULL )
  {
    m_pSoundClass = pExistingSound;
    if ( !m_pSoundClass->Initialize( GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_SOUND_SWITCHED ) );
  }

  XMusic* pExistingMusic = (XMusic*)GR::Service::Environment::Instance().Service( "Music" );
  if ( pExistingMusic != NULL )
  {
    m_pMusicPlayer = pExistingMusic;
    if ( !m_pMusicPlayer->Initialize( GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_MUSIC_PLAYER_SWITCHED ) );
  }

  GR::Gamebase::Framework::RunDefaultModules();

  if ( m_pRenderClass == NULL )
  {
    if ( m_CurrentRenderModule.empty() )
    {
      if ( m_RendererAccelerated )
      {
        m_CurrentRenderModule = "DX82DRenderer.dll";
      }
      else
      {
        m_CurrentRenderModule = "ddrawrenderer.dll";
      }
    }
    if ( SwitchRenderer( m_CurrentRenderModule.c_str() ) == NULL )
    {
      m_RendererAccelerated = false;
      if ( !m_RendererAccelerated )
      {
        if ( SwitchRenderer( "ddrawrenderer.dll" ) == NULL )
        {
          return false;
        }
        m_CurrentRenderModule = "ddrawrenderer.dll";
      }
    }
  }

  if ( ( m_pRenderClass == NULL )
  ||   ( m_pInputClass == NULL )
  ||   ( m_pSoundClass == NULL ) )
  {
    dh::Log( "An error occurred. Please rerun the setup. - Initialisation Error!" );
    return false;
  }
  return true;
}



bool XFrameApp2d::IsAccelerated() const
{
  return m_RendererAccelerated;
}



bool XFrameApp2d::SetAcceleration( bool Accelerate )
{
  if ( Accelerate != IsAccelerated() )
  {
    if ( Accelerate )
    {
      m_RendererAccelerated = true;
      if ( SwitchRenderer( "dx82drenderer.dll" ) == NULL )
      {
        m_RendererAccelerated = false;
        SwitchRenderer( "ddrawrenderer.dll" );
        return false;
      }
    }
    else
    {
      m_RendererAccelerated = false;
      SwitchRenderer( "ddrawrenderer.dll" );
    }
  }
  return true;
}



XTextureSection XFrameApp2d::Section( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp2d::Section No loader registered" );
    return XTextureSection();
  }

  Xtreme::Asset::XAssetImageSection* pSection = (Xtreme::Asset::XAssetImageSection*)pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, strName.c_str() );
  if ( pSection == NULL )
  {
    dh::Log( "XFrameApp2d::Section Section not found (%s)", strName.c_str() );

    return XTextureSection();
  }
  return pSection->Section();
}



XTexture* XFrameApp2d::Texture( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return NULL;
  }

  Xtreme::Asset::XAssetImage* pImage = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, strName.c_str() );
  if ( pImage == NULL )
  {
    return NULL;
  }
  return pImage->Texture();
}



X2dFont* XFrameApp2d::Font( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return NULL;
  }

  Xtreme::Asset::XAssetFont2d* pImage = (Xtreme::Asset::XAssetFont2d*)pLoader->Asset( Xtreme::Asset::XA_FONT, strName.c_str() );
  if ( pImage == NULL )
  {
    return NULL;
  }
  return pImage->Font();
}



GR::u32 XFrameApp2d::Sound( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return 0;
  }

  Xtreme::Asset::XAssetSound* pSound = (Xtreme::Asset::XAssetSound*)pLoader->Asset( Xtreme::Asset::XA_SOUND, strName.c_str() );
  if ( pSound == NULL )
  {
    return 0;
  }
  return pSound->Handle();
}



CSpline XFrameApp2d::Spline( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return CSpline();
  }

  Xtreme::Asset::XAssetSpline* pSpline = (Xtreme::Asset::XAssetSpline*)pLoader->Asset( Xtreme::Asset::XA_SPLINE, strName.c_str() );
  if ( pSpline == NULL )
  {
    return CSpline();
  }
  return pSpline->Spline();
}



IAnimationManager<GR::String>::tAnimType XFrameApp2d::Animation( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp2d::Animation Missing AssetLoader Service" );
    return IAnimationManager<GR::String>::tAnimType();
  }

  Xtreme::Asset::XAssetAnimation* pAnim = (Xtreme::Asset::XAssetAnimation*)pLoader->Asset( Xtreme::Asset::XA_ANIMATION, strName.c_str() );
  if ( pAnim == NULL )
  {
    dh::Log( "XFrameApp2d::Animation Animation %s not found", strName.c_str() );
    return IAnimationManager<GR::String>::tAnimType();
  }
  return pAnim->Animation();
}



tAnimationPos XFrameApp2d::AnimationPos( const GR::String& strName )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp2d::AnimationPos Missing AssetLoader Service" );
    return tAnimationPos();
  }

  Xtreme::Asset::XAssetAnimation* pAnim = (Xtreme::Asset::XAssetAnimation*)pLoader->Asset( Xtreme::Asset::XA_ANIMATION, strName.c_str() );
  if ( pAnim == NULL )
  {
    dh::Log( "XFrameApp2d::AnimationPos Animation %s not found", strName.c_str() );
    return tAnimationPos();
  }
  return tAnimationPos( pAnim->m_AnimID );
}



XTextureSection XFrameApp2d::AnimationFrame( const tAnimationPos& AnimPos )
{
  return Section( m_AnimationManager.GetAnimFrame( AnimPos ) );
}



void XFrameApp2d::OnEnvironmentEvent( GR::Service::Environment::EnvironmentEvent Event, const char* ServiceName, const char* EventName, GR::IService* pService )
{
  if ( Event == GR::Service::Environment::EE_SERVICE_NOTIFY )
  {
    if ( strcmp( ServiceName, "GUI" ) == 0 )
    {
      if ( strcmp( EventName, "AssetsLoaded" ) == 0 )
      {
        Xtreme::Asset::XAssetLoader*    pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
        if ( pLoader == NULL )
        {
          return;
        }

        GR::Strings::XMLElement*    pXMLGUIGeneral = pLoader->AssetInfo( Xtreme::Asset::XA_GUI, -1 );
        if ( pXMLGUIGeneral )
        {
          // general GUI settings
          m_GUI.SetDefaultFont( Font( pXMLGUIGeneral->Attribute( "Font" ) ) );
          if ( pXMLGUIGeneral->HasAttribute( "EdgeTopLeft" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_TOP_LEFT, Section( pXMLGUIGeneral->Attribute( "EdgeTopLeft" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeTop" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_TOP, Section( pXMLGUIGeneral->Attribute( "EdgeTop" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeTopRight" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_TOP_RIGHT, Section( pXMLGUIGeneral->Attribute( "EdgeTopRight" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeLeft" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_LEFT, Section( pXMLGUIGeneral->Attribute( "EdgeLeft" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeRight" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_RIGHT, Section( pXMLGUIGeneral->Attribute( "EdgeRight" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeBottomLeft" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_BOTTOM_LEFT, Section( pXMLGUIGeneral->Attribute( "EdgeBottomLeft" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeBottom" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_BOTTOM, Section( pXMLGUIGeneral->Attribute( "EdgeBottom" ) ) );
          }
          if ( pXMLGUIGeneral->HasAttribute( "EdgeBottomRight" ) )
          {
            m_GUI.SetDefaultTextureSection( GUI::BT_EDGE_BOTTOM_RIGHT, Section( pXMLGUIGeneral->Attribute( "EdgeBottomRight" ) ) );
          }
        }
      }
    }
  }
}



void XFrameApp2d::DisplayFrame( X2dRenderer& Renderer )
{
  // Clear the viewport
  Renderer.RenderQuad( 0, 0, Renderer.Width(), Renderer.Height(), 0xff000000 );

  m_GUI.SetRenderer( &Renderer );
  m_GUI.SetExtents( m_RenderFrame.Width(), m_RenderFrame.Height() );
  m_GUI.DisplayBackground();

  XViewport   vp = Renderer.Viewport();

  std::set<GUIComponent*>   displayedStateScreens;

  std::list<IGameState<X2dRenderer>*>::iterator    it( m_StateManager.m_GameStateStack.begin() );
  while ( it != m_StateManager.m_GameStateStack.end() )
  {
    GUIGameState<X2dRenderer, GUIComponent>*    pGameState = (GUIGameState<X2dRenderer, GUIComponent>*)*it;

    Renderer.SetViewport( vp );
    m_GUI.DisplayComponentFromParent( (GUIComponent*)pGameState->m_pStateScreen );
    displayedStateScreens.insert( (GUIComponent*)pGameState->m_pStateScreen );
    ++it;
  }

  // display all controls that are not direct part of the states (hottips etc.)
  GUI::tListComponents::iterator    itC( m_GUI.m_Components.begin() );
  while ( itC != m_GUI.m_Components.end() )
  {
    GUIComponent*   pComp = (GUIComponent*)*itC;
    ++itC;

    if ( displayedStateScreens.find( pComp ) == displayedStateScreens.end() )
    {
      Renderer.SetViewport( vp );
      m_GUI.DisplayComponentFromParent( pComp );
    }
  }
  Renderer.SetViewport( vp );
}



XTextureSection XFrameApp2d::Tile( const GR::String& Tileset, const int TileIndex )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Tile Missing AssetLoader Service" );
    return XTextureSection();
  }

  Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, Tileset.c_str() );
  if ( pTileset == NULL )
  {
    dh::Log( "XFrameApp::Tile Tileset %s not found", Tileset.c_str() );
    return XTextureSection();
  }
  if ( (size_t)TileIndex >= pTileset->m_Tiles.size() )
  {
    dh::Log( "XFrameApp::Tile Tile %d not found", TileIndex );
    return XTextureSection();
  }
  if ( pTileset->m_Tiles[TileIndex].Animation.empty() )
  {
    return Section( pTileset->Section( TileIndex ) );
  }
  return AnimationFrame( pTileset->AnimationPos( TileIndex ) );
}



GR::String XFrameApp2d::TileName( const GR::String& Tileset, const int TileIndex )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::TileName Missing AssetLoader Service" );
    return GR::String();
  }

  Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, Tileset.c_str() );
  if ( pTileset == NULL )
  {
    dh::Log( "XFrameApp::TileName Tileset %s not found", Tileset.c_str() );
    return GR::String();
  }
  if ( pTileset->m_Tiles[TileIndex].Animation.empty() )
  {
    return pTileset->Section( TileIndex );
  }
  return pTileset->m_Tiles[TileIndex].Section;
}



size_t XFrameApp2d::TilesetCount( const GR::String& Tileset )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::TilesetCount Missing AssetLoader Service" );
    return 0;
  }

  Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, Tileset.c_str() );
  if ( pTileset == NULL )
  {
    dh::Log( "XFrameApp::TilesetCount Tileset %s not found", Tileset.c_str() );
    return 0;
  }
  return pTileset->m_Tiles.size();
}



GR::u32 XFrameApp2d::TileFlags( const GR::String& Tileset, const int TileIndex )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::TileFlags Missing AssetLoader Service" );
    return 0;
  }

  Xtreme::Asset::XAssetTileset* pTileset = (Xtreme::Asset::XAssetTileset*)pLoader->Asset( Xtreme::Asset::XA_TILESET, Tileset.c_str() );
  if ( pTileset == NULL )
  {
    dh::Log( "XFrameApp::TileFlags Tile %d not found", TileIndex );
    return 0;
  }
  return pTileset->Flags( TileIndex );
}



GR::f64 XFrameApp2d::FixedTimeStep() const
{
  return m_FixedLogicTimeStep;
}



void XFrameApp2d::OnCharEntered( char Key )
{
  if ( m_ConsoleVisible )
  {
    if ( Key == 13 )
    {
      // Enter
      EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_CONSOLE_COMMAND, m_ConsoleEingabe ) );
      m_ConsoleEingabe = "";
    }
    else if ( Key == 8 )
    {
      if ( m_ConsoleEingabe.length() )
      {
        m_ConsoleEingabe.resize( m_ConsoleEingabe.length() - 1 );
      }
    }
    else if ( (int)Key >= 32 )
    {
      m_ConsoleEingabe += (char)Key;
    }
    // no renderer, update GDI console
    if ( m_pRenderClass == NULL )
    {
      m_Window.Invalidate();
    }
  }
}



void XFrameApp2d::OnKeyDown( char Key )
{
  if ( Key == 0x7a ) // VK_F11
  {
    ToggleConsole();
  }
  if ( Renderer() == NULL )
  {
    m_Window.Invalidate();
  }
}



void XFrameApp2d::OnKeyUp( char Key )
{
}



void XFrameApp2d::OnSizeChanged()
{
  if ( m_pRenderClass )
  {
    m_pRenderClass->OnResized();
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED ) );
  }
}