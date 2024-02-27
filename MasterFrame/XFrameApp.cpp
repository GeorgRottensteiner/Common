#include <DateTime/Timer.h>

#include <Xtreme/XInput.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetSound.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetSpline.h>
#include <Xtreme/XAsset/XAssetAnimation.h>
#include <Xtreme/XAsset/XAssetTileset.h>

#include <Interface/IService.h>

#include <IO/FileChunk.h>
#include <IO/FileStream.h>

#include <Misc/CloneFactory.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Controls/Xtreme/GUIScreen.h>
#include <Controls/Xtreme/GUIComponentDisplayer.h>

#include <String/XML.h>
#include <String/Path.h>
#include <String/Convert.h>
#include <String/StringUtil.h>

#include "XFrameApp.h"


#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

#include <Xtreme/Input/XInputUniversal.h>
#include <Xtreme/DX11/DX11Renderer.h>
#include <Xtreme/Audio/XAudio2Sound.h>
//#include "OggXAudio2.h"
#elif OPERATING_SYSTEM == OS_WEB
#include <Xtreme/SDL/SDLInput.h>
#include <Xtreme/OpenGLShader/OpenGLShaderRenderClass.h>
#include <Xtreme/NullSound/XNullMusic.h>
#include <Xtreme/NullSound/XNullSound.h>
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <Xtreme/SDL/SDLInput.h>
#include <Xtreme/SDL/SDLRenderClass.h>
#include <Xtreme/NullSound/XNullMusic.h>
#include <Xtreme/NullSound/XNullSound.h>
#elif OPERATING_SYSTEM == OS_ANDROID
#include <Xtreme/Android/AndroidRenderer.h>
#include <Xtreme/Android/AndroidInput.h>
#include <Xtreme/Android/AndroidSound.h>
#include <Xtreme/Environment/AndroidMain.h>
#endif



XFrameApp*      g_pFrameApp = NULL;


XFrameApp::XFrameApp() :
  m_pConsoleFont( NULL ),
  m_FixedLogicTimeStep( 0.05f ),
  m_IconID( 0 ),
  m_BitmapResourceID( 0 ),
  m_MouseHotSpot( 0, 0 ),
  m_CurScreenshotNumber( 0 ),
  m_ClassName( _T( "XFrameWndClass" ) ),
  m_CustomMouseCursorColor( 0xffffffff ),
  m_GUILoader( GUIComponentDisplayer::Instance() ),
  m_GUI( GUIComponentDisplayer::Instance() ),
  m_ForceFixedSize( false ),
  m_KeepMouseInsideDuringFullscreen( true )
{
  g_pFrameApp = this;

  GR::Service::Environment::Instance().AddHandler( fastdelegate::MakeDelegate( this, &XFrameApp::OnEnvironmentEvent ) );

  GR::Service::Environment::Instance().SetService( "GlobalRegistry", (LocalRegistry*)this );
  GR::Service::Environment::Instance().SetService( "GlobalQueue", &EventQueue::Instance() );
  GR::Service::Environment::Instance().SetService( "ImageLoader", &ImageFormatManager::Instance() );

  GR::Service::Environment::Instance().SetService( "Logger", &m_DebugService );
  GR::Service::Environment::Instance().SetService( "WindowFrame", &m_RenderFrame );

  m_ETChangeState         = EventQueue::Instance().RegisterEvent( "App.ChangeState" );
  m_ETPushState           = EventQueue::Instance().RegisterEvent( "App.PushState" );
  m_ETPushStateOnStack    = EventQueue::Instance().RegisterEvent( "App.PushStateOnStack" );
  m_ETPopState            = EventQueue::Instance().RegisterEvent( "App.PopState" );
  m_ETPopAnyState         = EventQueue::Instance().RegisterEvent( "App.PopAnyState" );
  m_ETShutDown            = EventQueue::Instance().RegisterEvent( "App.ShutDown" );
  m_ETShuttingDown        = EventQueue::Instance().RegisterEvent( "App.Info.ShuttingDown" );
  m_ETRendererInitialised = EventQueue::Instance().RegisterEvent( "Renderer.Initialised" );
  m_ETRendererReleased    = EventQueue::Instance().RegisterEvent( "Renderer.Released" );

  m_GUI.m_pEventProducer  = this;

  EventQueue::Instance().AddListener( this );
}



XFrameApp::~XFrameApp()
{
  m_GUI.m_Background.Clear();

  EventQueue::Instance().UnregisterEvent( m_ETChangeState );
  EventQueue::Instance().UnregisterEvent( m_ETPushState );
  EventQueue::Instance().UnregisterEvent( m_ETPushStateOnStack );
  EventQueue::Instance().UnregisterEvent( m_ETPopState );
  EventQueue::Instance().UnregisterEvent( m_ETPopAnyState );
  EventQueue::Instance().UnregisterEvent( m_ETShutDown );
  EventQueue::Instance().UnregisterEvent( m_ETShuttingDown );
  EventQueue::Instance().UnregisterEvent( m_ETRendererInitialised );
  EventQueue::Instance().UnregisterEvent( m_ETRendererReleased );

  GR::Service::Environment::Instance().RemoveService( "GlobalRegistry" );
  GR::Service::Environment::Instance().RemoveService( "GlobalQueue" );
  GR::Service::Environment::Instance().RemoveService( "ImageLoader" );
  GR::Service::Environment::Instance().RemoveService( "SubclassManager" );
  GR::Service::Environment::Instance().RemoveService( "Logger" );
  GR::Service::Environment::Instance().RemoveService( "WindowFrame" );

  GR::Service::Environment::Instance().RemoveHandler( fastdelegate::MakeDelegate( this, &XFrameApp::OnEnvironmentEvent ) );
}



bool XFrameApp::InitInstance()
{
  return true;
}


void XFrameApp::ExitInstance()
{
#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
  if ( m_Window.m_pMainWindow != NULL )
  {
    SDL_DestroyWindow( m_Window.m_pMainWindow );
    m_Window.m_pMainWindow = NULL;
  }

  SDL_Quit();
#endif
}



void XFrameApp::UpdateFixedLogic()
{
}



void XFrameApp::DoSplashScreen()
{
  // Splash-Screen
  if ( m_BitmapResourceID )
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    HBITMAP   hbm = LoadBitmap( m_Window.HInstance, MAKEINTRESOURCE( m_BitmapResourceID ) );
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
#endif
  }
  else
  {
    // loading...
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    HDC   hdcMain = GetDC( m_RenderFrame.m_hwndMain );

    RECT  rc;
    GetClientRect( m_RenderFrame.m_hwndMain, &rc );
    FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

    SelectObject( hdcMain, GetStockObject( DEFAULT_GUI_FONT ) );

    SetTextColor( hdcMain, 0xffffff );
    SetBkMode( hdcMain, TRANSPARENT );
    DrawTextA( hdcMain, "loading...", 10, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

    ReleaseDC( m_RenderFrame.m_hwndMain, hdcMain );
#endif
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  UpdateWindow( m_RenderFrame.m_hwndMain );
#endif
}



bool XFrameApp::SwitchRenderer( const char* FileName )
{
  if ( m_pRenderClass )
  {
    m_pRenderClass->Release();
    GR::Service::Environment::Instance().RemoveService( "Renderer" );
  }
  m_pRenderClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( m_hinstCurrentRenderer )
  {
    FreeLibrary( m_hinstCurrentRenderer );
    m_hinstCurrentRenderer = NULL;
  }
#endif

  if ( FileName == NULL )
  {
    ConsolePrint( "Renderer switched off" );
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
    return false;
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  m_hinstCurrentRenderer = LoadLibraryA( FileName );
  if ( m_hinstCurrentRenderer == NULL )
  {
    m_hinstCurrentRenderer = LoadLibraryA( AppPath( FileName ).c_str() );
  }
  if ( m_hinstCurrentRenderer )
  {
    typedef XRenderer* (*tIFunction)( void );
    tIFunction    fGetInterface = (tIFunction)GetProcAddress( m_hinstCurrentRenderer, "GetInterface" );

    if ( fGetInterface )
    {
      m_pRenderClass = fGetInterface();

      GR::Service::Environment::Instance().SetService( "Renderer", m_pRenderClass );
    }
  }
#endif
  ConsolePrint( "Renderer switched to %s", FileName );
  EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
  if ( m_pRenderClass )
  {
    m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
  }
  return true;
}



XRenderer* XFrameApp::Renderer()
{
  return m_pRenderClass;
}



bool XFrameApp::RunDefaultModules()
{
  if ( m_RenderFrame.m_DisplayMode.Width == 0 )
  {
    // default modes
    m_RenderFrame.m_DisplayMode.Width       = 640;
    m_RenderFrame.m_DisplayMode.Height      = 480;
    m_RenderFrame.m_DisplayMode.ImageFormat = GR::Graphic::ImageData::ImageFormatFromDepth( 16 );
    m_RenderFrame.m_DisplayMode.FullScreen  = false;
  }

#if OPERATING_SYSTEM != OS_WEB
  XRenderer* pExistingRenderer = (XRenderer*)GR::Service::Environment::Instance().Service( "Renderer" );
  if ( pExistingRenderer != NULL )
  {
    m_pRenderClass = pExistingRenderer;
    if ( !m_pRenderClass->Initialize( m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height, 32, 0, GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    m_RenderFrame.SetSize( m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );

    ConsolePrint( "Renderer switched to existing" );
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
  }
#endif

  Xtreme::XInput* pExistingInput = (Xtreme::XInput*)GR::Service::Environment::Instance().Service( "Input" );
  if ( pExistingInput != NULL )
  {
    m_pInputClass = pExistingInput;
    if ( !m_pInputClass->Initialize( GR::Service::Environment::Instance() ) )
    {
      return false;
    }
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_INPUT_SWITCHED ) );
  }

  XSound* pExistingSound = (XSound*)GR::Service::Environment::Instance().Service( "Sound" );
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

  std::list<GR::String>    potentialRenderers;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( m_CurrentRenderModule.empty() )
  {
    m_CurrentRenderModule = "DX11Renderer.dll";
  }
  if ( m_CurrentRenderModule != "DX11Renderer.dll" )
  {
    potentialRenderers.push_back( "DX11Renderer.dll" );
  }
  if ( m_CurrentRenderModule != "DX9Renderer.dll" )
  {
    potentialRenderers.push_back( "DX9Renderer.dll" );
  }
  if ( m_CurrentRenderModule != "DX8Renderer.dll" )
  {
    potentialRenderers.push_back( "DX8Renderer.dll" );
  }

  try_next_renderer:;
  SwitchRenderer( m_CurrentRenderModule.c_str() );
#elif OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  m_pInputClass = new XInputUniversal();
  if ( !m_pInputClass->Initialize( GR::Service::Environment::Instance() ) )
  {
    return false;
  }
  ( (XInputUniversal*)m_pInputClass )->SetActive();

  m_pRenderClass = new DX11Renderer();
  //m_pSoundClass = new XNullSound();
  m_pSoundClass = new Xtreme::XAudio2Sound();

  ( (DX11Renderer*)m_pRenderClass )->m_window = m_Window.CoreWindow;
  if ( !m_pRenderClass->Initialize( m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight, 32, 0, GR::Service::Environment::Instance() ) )
  {
    return false;
  }
  if ( !m_pSoundClass->Initialize( GR::Service::Environment::Instance() ) )
  {
    return false;
  }
  m_pSoundClass->SetMasterVolume( m_FXVolume );


  // call events
  EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED ) );
  if ( m_pRenderClass )
  {
    m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
  }

  m_GUI.SetScreenSizes( GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ), GR::tPoint( m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight ) );

  m_pRenderClass->SetState( XRenderer::RS_MAGFILTER, XRenderer::RSV_FILTER_POINT );
  m_pRenderClass->SetState( XRenderer::RS_MINFILTER, XRenderer::RSV_FILTER_POINT );
  m_pRenderClass->SetState( XRenderer::RS_MIPFILTER, XRenderer::RSV_FILTER_POINT );
#elif OPERATING_SYSTEM == OS_WEB
  if ( m_pRenderClass == NULL )
  {
    m_pRenderClass = new OpenGLShaderRenderClass();
    GR::Service::Environment::Instance().SetService( "Renderer", m_pRenderClass );
  }
  if ( m_pSoundClass == NULL )
  {
    m_pSoundClass = new XNullSound();
  }
  if ( m_pMusicPlayer == NULL )
  {
    m_pMusicPlayer = new XNullMusic();
  }
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
  if ( m_pRenderClass == NULL )
  {
    m_pRenderClass = new SDLRenderClass();
  }
  if ( m_pSoundClass == NULL )
  {
    m_pSoundClass = new XNullSound();
  }
  if ( m_pInputClass == NULL )
  {
    m_pInputClass = new SDLInput();
  }
  if ( m_pMusicPlayer == NULL )
  {
    m_pMusicPlayer = new XNullMusic();
  }

  if ( !m_pInputClass->Initialize( GR::Service::Environment::Instance() ) )
  {
    return false;
  }
#elif OPERATING_SYSTEM == OS_ANDROID
  m_pRenderClass = new AndroidRenderer();
  m_pInputClass = new AndroidInput();
  m_pSoundClass = new AndroidSound();
#endif
  if ( ( m_pRenderClass == NULL )
  ||   ( m_pInputClass == NULL )
  ||   ( m_pSoundClass == NULL ) )
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    if ( m_pRenderClass == NULL )
    {
      if ( potentialRenderers.empty() )
      {
        dh::Log( "-no fallback left" );
      }
      else
      {
        m_CurrentRenderModule = potentialRenderers.front();
        potentialRenderers.pop_front();
        goto try_next_renderer;
      }
    }
    
    MessageBox( GetSafeHwnd(), _T( "An error occurred. Please rerun the setup." ), _T( "Initialisation Error!" ), 0 );
#endif
    return false;
  }

  ChangeWindowSize( m_RenderFrame.m_DisplayMode.Width, 
                    m_RenderFrame.m_DisplayMode.Height,
                    GR::Graphic::ImageData::DepthFromImageFormat( m_RenderFrame.m_DisplayMode.ImageFormat ) );

  bool  initWithFullscreen = m_RenderFrame.m_DisplayMode.FullScreen;

  if ( !m_pRenderClass->Initialize( m_RenderFrame.m_DisplayMode.Width, 
                                    m_RenderFrame.m_DisplayMode.Height,
                                    GR::Graphic::ImageData::DepthFromImageFormat( m_RenderFrame.m_DisplayMode.ImageFormat ),
                                    false,//m_RenderFrame.m_DisplayMode.FullScreen,
                                    GR::Service::Environment::Instance() ) )
  {
    dh::Log( "RenderClass Initialize failed" );
    if ( potentialRenderers.empty() )
    {
      dh::Log( "-no fallback left" );
    }
    else
    {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
      m_CurrentRenderModule = potentialRenderers.front();
      potentialRenderers.pop_front();
      goto try_next_renderer;
#endif
    }
    return false;
  }
  // Important after renderer->initialize
  if ( initWithFullscreen )
  {
    XRendererDisplayMode    mode;

    //dh::Log( "initWithFullscreen setting %dx%d", m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
    mode.Width = m_RenderFrame.m_DisplayMode.Width;
    mode.Height = m_RenderFrame.m_DisplayMode.Height;
    mode.ImageFormat  = m_pRenderClass->ImageFormat();
    mode.FullScreen   = true;

    DetermineBestFullscreenMatch( mode );

    m_pRenderClass->SetMode( mode );
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_WINDOW_MODE_CHANGED ) );
  }

  m_pRenderClass->Canvas( m_EnvironmentDisplayRect );
  m_RenderFrame.SetSize( m_pRenderClass->Width(), m_pRenderClass->Height() );
  m_GUI.SetScreenSizes( GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ), m_GUI.VirtualSize() );

  AdjustCanvas( m_pRenderClass->Width(), m_pRenderClass->Height() );
  return true;
}



#if OPERATING_SYSTEM == OS_WEB
void XFrameApp::OneLoopIterationStub()
{
  g_pFrameApp->OneLoopIteration();
}



void XFrameApp::OneLoopIteration()
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
    GR::f32   ElapsedTime = (GR::f32)Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
    GR::f32   fElapsedFixedTime = (GR::f32)(GR::f64)( fNewFrameTime - m_LastFixedFrameTime );

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
      fElapsedFixedTime -= (GR::f32)m_FixedLogicTimeStep;
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
  }
}
#endif



void XFrameApp::RunLoop()
{
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
      BOOL    returnValue = GetMessage( &msg, NULL, 0, 0 );
      if ( returnValue == 0 )
      {
        break;
      }
      else if ( returnValue == -1 )
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
        BOOL    returnValue = GetMessage( &msg, NULL, 0, 0 );

        if ( returnValue == 0 )
        {
          goto main_loop_done;
        }
        else if ( returnValue == -1 )
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
             && ( sdlEvent.type < SDL_CLIPBOARDUPDATE ) )
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
      GR::f64   newFrameTime = Time::Timer::Time();
      GR::f32   elapsedTime = ( GR::f32 )Time::Timer::Time( Time::Timer::TF_GETELAPSEDTIME );
      GR::f32   elapsedFixedTime = ( GR::f32 )( GR::f64 )( newFrameTime - m_LastFixedFrameTime );

      if ( elapsedTime > 10.0f )
      {
        elapsedTime = 0.0f;
      }
      //dh::Log( "TTElapsedTime %.6f", elapsedTime );
      //dh::Log( "ElapsedTime %.6f", ElapsedTime );

      if ( m_ConsoleVisible )
      {
        elapsedTime = 0.0f;
        m_LastFixedFrameTime = newFrameTime;
        m_LastFrameTime = newFrameTime;
      }

      int     safetyLoopCounter = 0;
      while ( elapsedFixedTime >= m_FixedLogicTimeStep )
      {
        safetyLoopCounter++;
        if ( safetyLoopCounter > 25 )
        {
          // Safety-Break um eventuelle Endlosloops zu verhindern
          m_LastFixedFrameTime = newFrameTime;
          break;
        }
        m_StateManager.UpdateCurrentState();
        UpdateFixedLogic();
        EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
        elapsedFixedTime -= ( GR::f32 )m_FixedLogicTimeStep;
        m_LastFixedFrameTime += m_FixedLogicTimeStep;
      }
      m_LastFrameTime = newFrameTime;

      if ( m_pInputClass )
      {
        m_pInputClass->Update( elapsedTime );
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
      m_StateManager.UpdateStatesPerDisplayFrame( elapsedTime );
      UpdatePerDisplayFrame( elapsedTime );
      UpdateAssets( elapsedTime );
      EventProducer<GR::Gamebase::tXFrameEvent>::ProcessEventQueue();
      EventQueue::Instance().ProcessQueue();

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
    }
    if ( m_ShutDown )
    {
      GLOBAL_QUEUE.SendEvent( "App.Info.ShuttingDown" );

      ExitInstance();
      m_StateManager.PopAllStates();
      EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_SHUTDOWN ) );

      GR::IO::FileStream    ioOut( UserAppDataPath( "xtreme.cfg" ), IIOStream::OT_WRITE_ONLY );
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
  main_loop_done:;
#endif
}



int XFrameApp::PreLoopRun()
{
#if OPERATING_SYSTEM == OS_WINDOWS
  GR::u32   windowStates = WS_DLGFRAME | WS_VISIBLE | WS_OVERLAPPED | WS_SYSMENU;

  if ( !m_EnvironmentConfig.FixedSize )
  {
    windowStates |= WS_SIZEBOX | WS_MAXIMIZEBOX;
  }
#else
  GR::u32 windowStates = 0;
#endif

  Create( windowStates, 
          m_EnvironmentConfig.StartUpWidth, 
          m_EnvironmentConfig.StartUpHeight, 
          m_EnvironmentConfig.Caption.c_str(), 
          0, 
          m_EnvironmentConfig.IconResourceID );

  GR::IO::FileStream  ioIn( UserAppDataPath( "xtreme.cfg" ).c_str() );
  Load( ioIn );

  m_EnvironmentDisplayRect.Set( 0, 0, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  if ( m_EnvironmentConfig.FixedSize )
  {
    // can't override resolution from previous save, reset to startup size
    m_RenderFrame.SetSize( m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  }
  else
  {
    m_RenderFrame.SetSize( m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
    m_EnvironmentDisplayRect.Set( 0, 0, m_RenderFrame.m_DisplayMode.Width, m_RenderFrame.m_DisplayMode.Height );
  }
  auto   itCL( m_StartParameter.begin() );
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
        m_DebugService.LogEnable( *it );

        ++it;
      }
    }
    else if ( param.find( "/logtofile:" ) == 0 )
    {
      GR::String   strLog = param.substr( 11 );

      std::list<GR::String>    listLogParams;

      GR::Strings::Split( strLog, ',', listLogParams );

      auto it( listLogParams.begin() );
      while ( it != listLogParams.end() )
      {
        m_DebugService.LogEnable( *it );
        m_DebugService.LogToFile( *it );

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
    g_pFrameApp = NULL;
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



//#include <Xtreme/Environment/AndroidMain.h>
int XFrameApp::Run()
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



int XFrameApp::PostLoopRun()
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



void XFrameApp::ReleaseModules()
{
  if ( m_pRenderClass )
  {
    /*
    if ( m_pRenderClass->IsFullscreen() )
    {
      m_pRenderClass->ToggleFullscreen();
    }*/
    m_pRenderClass->Release();
    GR::Service::Environment::Instance().RemoveService( "Renderer" );
    m_pRenderClass = NULL;
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
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



void XFrameApp::Present()
{
  GR::tRect*   pRcSource = NULL;
  GR::tRect*   pRcTarget = NULL;

  if ( !m_SourceRect.Empty() )
  {
    pRcSource = &m_SourceRect;
  }
  if ( !m_TargetRect.Empty() )
  {
    pRcTarget = &m_TargetRect;
  }
  m_pRenderClass->PresentScene( pRcSource, pRcTarget );
}



#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
LRESULT CALLBACK XFrameApp::XFrameAppProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  if ( message == WM_NCCREATE )
  {
    CREATESTRUCT*   pCS = (CREATESTRUCT*)lParam;

    g_pFrameApp = (XFrameApp*)pCS->lpCreateParams;
    g_pFrameApp->m_RenderFrame.m_hwndMain = hWnd;
  }

  if ( g_pFrameApp )
  {
    if ( g_pFrameApp->m_Window.SubclassManager.Empty() )
    {
      return g_pFrameApp->WindowProc( message, wParam, lParam );
    }
    return g_pFrameApp->m_Window.SubclassManager.CallChain( hWnd, message, wParam, lParam );
  }
  return ::DefWindowProc( hWnd, message, wParam, lParam );
}



BOOL XFrameApp::SubclassWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  return (BOOL)WindowProc( message, wParam, lParam );
}



LRESULT XFrameApp::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
  {
    case WM_GETICON:
      return NULL;
    case WM_COPYDATA:
      {
        COPYDATASTRUCT*   pCDS = (COPYDATASTRUCT*)lParam;

        if ( ( pCDS->lpData )
        &&   ( pCDS->cbData ) )
        {
          GR::String result;

          result.append( (GR::Char*)pCDS->lpData, pCDS->cbData - 1 );

          if ( result.find( "[set]" ) == 0 )
          {
            size_t    iSepPos = result.find( '=', 5 );
            if ( iSepPos != GR::String::npos )
            {
              SetVar( result.substr( 5, iSepPos - 5 ), result.substr( iSepPos + 1 ) );
              InvalidateRect( m_RenderFrame.m_hwndMain, NULL, FALSE );
            }
          }
        }
      }
      return TRUE;
    case WM_KEYDOWN:
      if ( wParam == VK_F11 )
      {
        ToggleConsole();
      }
      if ( m_pRenderClass == NULL )
      {
        InvalidateWindow();
      }
      break;
    case WM_CHAR:
      if ( m_ConsoleVisible )
      {
        if ( wParam == 13 )
        {
          // Enter
          EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_CONSOLE_COMMAND, m_ConsoleEingabe ) );
          m_ConsoleEingabe = "";
        }
        else if ( wParam == 8 )
        {
          if ( m_ConsoleEingabe.length() )
          {
            m_ConsoleEingabe.resize( m_ConsoleEingabe.length() - 1 );
          }
        }
        else if ( (int)wParam >= 32 )
        {
          m_ConsoleEingabe += (char)wParam;
        }
        if ( m_pRenderClass == NULL )
        {
          InvalidateRect( m_RenderFrame.m_hwndMain, NULL, FALSE );
        }
      }
      break;
    case WM_CLOSE:
      ShutDown();
      return 0;
    case WM_DESTROY:
      //ExitInstance();
      EventProducer<GR::Gamebase::tXFrameEvent>::RemoveListener( this );

      PostQuitMessage( 0 );
      return 0;
    case WM_PAINT:
      OnDisplayFrame();
      ValidateRect( m_RenderFrame.m_hwndMain, NULL );
      return 0;
    case WM_ERASEBKGND:
      if ( m_pRenderClass )
      {
        return TRUE;
      }
      break;
    case WM_SIZE:
      if ( m_pRenderClass )
      {
        m_pRenderClass->OnResized();

        m_pConsoleFont = NULL;

        m_RenderFrame.SetSize( m_pRenderClass->Width(), m_pRenderClass->Height() );
        m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
        EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED ) );
      }
      break;
    case WM_EXITSIZEMOVE:
      if ( m_pRenderClass )
      {
        m_pRenderClass->OnResized();

        m_pConsoleFont = NULL;

        m_RenderFrame.SetSize( m_pRenderClass->Width(), m_pRenderClass->Height() );
        m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
        EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED ) );
      }
      break;
    case WM_SYSCHAR:
      if ( wParam == VK_RETURN )
      {
        if ( m_pRenderClass )
        {
          if ( !m_pRenderClass->IsFullscreen() )
          {
          }
          ToggleFullscreen();

          if ( !m_ClipRect.Empty() )
          {
            // reset cliprect to adjust changed windows pos
            ApplyClipCursor( m_ClipRect );
          }
          return 0;
        }
      }
      break;
    case WM_SETCURSOR:
      if ( ( LOWORD( lParam ) == HTCLIENT )
      &&   ( ( !m_EnableCursor )
      ||     ( m_CustomMouseCursorSet ) ) )
      {
        SetCursor( NULL );
        return TRUE;
      }
      if ( LOWORD( lParam ) == HTCLIENT )
      {
        SetCursor( m_Window.HCursor );
        return TRUE;
      }
      break;
    case WM_ACTIVATE:
      m_ApplicationActive = ( LOWORD( wParam ) != WA_INACTIVE );

      if ( !m_ClipRect.Empty() )
      {
        if ( !m_ApplicationActive )
        {
          // Entclippen
          ::ClipCursor( NULL );
        }
        else if ( !m_ClipRect.Empty() )
        {
          // Cliprect wiederherstellen
          ApplyClipCursor( m_ClipRect );
        }
        else if ( ( m_KeepMouseInsideDuringFullscreen )
        &&        ( m_pRenderClass->IsFullscreen() ) )
        {
          ApplyClipCursor( GR::tRect( 0, 0, m_pRenderClass->Width(), m_pRenderClass->Height() ) );
        }
      }

      if ( m_ApplicationActive )
      {
        m_StateManager.OnResumeApplication();
      }
      else
      {
        m_StateManager.OnPauseApplication();
      }
      break;
  }
  return ::DefWindowProc( m_RenderFrame.m_hwndMain, message, wParam, lParam );
}
#endif



bool XFrameApp::ProcessEvent( const GR::Gamebase::tXFrameEvent& Event )
{
  switch ( Event.m_Type )
  {
    case GR::Gamebase::tXFrameEvent::ET_MUSIC_PLAYER_SWITCHED:
      if ( m_pMusicPlayer != NULL )
      {
        if ( !VarExists( "Music.Volume" ) )
        {
          SetVarI( "Music.Volume", m_MusicVolume );
        }
        m_pMusicPlayer->SetVolume( m_MusicVolume );
      }
      break;
    case GR::Gamebase::tXFrameEvent::ET_RENDERER_SWITCHED:
      if ( ( m_pRenderClass )
      &&   ( m_pRenderClass->Width() ) )
      {
        m_GUI.SetRenderer( m_pRenderClass );
        m_RenderFrame.m_DisplayMode.Width       = m_pRenderClass->Width();
        m_RenderFrame.m_DisplayMode.Height      = m_pRenderClass->Height();
        //m_RenderFrame.m_DisplayMode.ImageFormat = GR::Graphic::ImageData::ImageFormatFromDepth( m_CurrentRenderDepth );
        m_RenderFrame.m_DisplayMode.FullScreen  = m_pRenderClass->IsFullscreen();
      }
      InvalidateWindow();
      if ( ( m_pRenderClass )
      &&   ( m_pRenderClass->Width() ) )
      {
        m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_SIZE_CHANGED, 0, GR::tPoint( m_pRenderClass->Width(), m_pRenderClass->Height() ) ) );
      }
      break;
    case GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED:
    case GR::Gamebase::tXFrameEvent::ET_WINDOW_MODE_CHANGED:
      if ( m_pRenderClass->Width() )
      {
        m_RenderFrame.m_DisplayMode.FullScreen = m_pRenderClass->IsFullscreen();

        if ( m_RenderFrame.m_DisplayMode.FullScreen )
        {
          //XRendererDisplayMode    mode = m_StoredWindowedMode;
          //dh::Log( "called DetermineBestFullscreenMatch from processevent" );
          //DetermineBestFullscreenMatch( mode );

          if ( m_KeepMouseInsideDuringFullscreen )
          {
            if ( m_ClipRect.Empty() )
            {
              ApplyClipCursor( GR::tRect( 0, 0, m_pRenderClass->Width(), m_pRenderClass->Height() ) );
            }
          }
        }
        else if ( m_EnvironmentConfig.FixedSize )
        {
          // TODO - restore remembered window size
          m_EnvironmentDisplayRect.Set( 0, 0, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
        }
        else
        {
          // TODO - restore remembered window size
          m_EnvironmentDisplayRect.Set( 0, 0, m_pRenderClass->Width(), m_pRenderClass->Height() );
        }
        if ( !m_RenderFrame.m_DisplayMode.FullScreen )
        {
          if ( m_KeepMouseInsideDuringFullscreen )
          {
            if ( m_ClipRect.Empty() )
            {
              ApplyClipCursor( GR::tRect() );
            }
          }
        }

        m_pRenderClass->Canvas( m_EnvironmentDisplayRect );

        AdjustCanvas( m_pRenderClass->Width(), m_pRenderClass->Height() );


        int     offsetX = ( m_pRenderClass->Width() - m_EnvironmentDisplayRect.Width() ) / 2;
        int     offsetY = ( m_pRenderClass->Height() - m_EnvironmentDisplayRect.Height() ) / 2;

        m_pRenderClass->Offset( GR::tPoint( offsetX, offsetY ) );
        m_RenderFrame.SetSize( m_pRenderClass->Width(), m_pRenderClass->Height() );
        m_RenderFrame.SendEvent( GR::tRenderFrameEvent( GR::tRenderFrameEvent::RFE_WINDOW_DISPLAY_MODE_CHANGED ) );
      }
      break;
    case GR::Gamebase::tXFrameEvent::ET_CONSOLE_COMMAND:
      if ( Event.m_Text == "quit" )
      {
        ShutDown();
      }
      else if ( Event.m_Text.find( "=" ) != GR::String::npos )
      {
        size_t    pos = Event.m_Text.find( "=" );

        if ( pos > 0 )
        {
          GR::String     varName = Event.m_Text.substr( 0, pos );
          GR::String     varValue = Event.m_Text.substr( pos + 1 );

          SetVar( varName, varValue );

          ConsolePrint( "%s set to %s", varName.c_str(), varValue.c_str() );
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
  }
  return false;
}



bool XFrameApp::Create( GR::u32 Style, int Width, int Height, const char* Caption,
                         GR::u32 SplashScreenBMPResourceID,
                         GR::u32 IconResourceID,
                         GR::u32 MenuID )
{
  m_BitmapResourceID  = SplashScreenBMPResourceID;
  m_IconID            = IconResourceID;

#if OPERATING_SYSTEM == OS_WINDOWS
  Style |= WS_BORDER;

  m_ForceFixedSize = !( Style & WS_DLGFRAME );
#endif

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  RECT      rcClientSize;

  SetRect( &rcClientSize, 0, 0, Width, Height );
  AdjustWindowRectEx( &rcClientSize, Style, ( MenuID != 0 ), WS_EX_WINDOWEDGE );
  
  WNDCLASS    WndClass;

  ZeroMemory( &WndClass, sizeof( WndClass ) );
  WndClass.style          = CS_HREDRAW | CS_VREDRAW;
  WndClass.lpfnWndProc    = (WNDPROC)XFrameAppProc;
  WndClass.hInstance      = m_Window.HInstance;
  WndClass.hIcon          = LoadIcon( m_Window.HInstance, MAKEINTRESOURCE( m_IconID ) );
  WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
  WndClass.hbrBackground  = NULL;
  WndClass.lpszMenuName   = NULL;
  if ( MenuID )
  {
    WndClass.lpszMenuName = MAKEINTRESOURCE( MenuID );
  }
  WndClass.lpszClassName  = m_ClassName.c_str();
  if ( !RegisterClass( &WndClass ) )
  {
    return false;
  }

  if ( MenuID )
  {
    m_Window.HMenu = LoadMenu( m_Window.HInstance, MAKEINTRESOURCE( MenuID ) );
  }

  // center window
  int     posX = ( GetSystemMetrics( SM_CXSCREEN ) - ( rcClientSize.right - rcClientSize.left ) ) / 2;
  int     posY = ( GetSystemMetrics( SM_CYSCREEN ) - ( rcClientSize.bottom - rcClientSize.top ) ) / 2;
#endif
  m_RenderFrame.SetSize( Width, Height );
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP

#ifdef UNICODE
  m_RenderFrame.m_hwndMain = CreateWindow( m_ClassName.c_str(), 
                                            GR::Convert::ToStringW( Caption ).c_str(),
                                            Style,
                                            posX,
                                            posY,
                                            rcClientSize.right - rcClientSize.left,
                                            rcClientSize.bottom - rcClientSize.top,
                                            NULL,
                                            m_Window.HMenu,
                                            m_Window.HInstance,
                                            this );
#else
  m_RenderFrame.m_hwndMain = CreateWindow( m_ClassName.c_str(), 
                                            Caption,
                                            Style,
                                            posX,
                                            posY,
                                            rcClientSize.right - rcClientSize.left,
                                            rcClientSize.bottom - rcClientSize.top,
                                            NULL,
                                            m_Window.HMenu,
                                            m_Window.HInstance,
                                            this );
#endif
  if ( m_RenderFrame.m_hwndMain == NULL )
  {
    //dh::Log( "In Created %x", GetLastError() );
    if ( m_Window.HMenu )
    {
      DestroyMenu( m_Window.HMenu );
      m_Window.HMenu = NULL;
    }
  }
  else
  {
    m_Window.SubclassManager.AddHandler( "MainApp", fastdelegate::MakeDelegate( this, &XFrameApp::SubclassWindowProc ) );
  }
  m_Window.Hwnd = m_RenderFrame.m_hwndMain;

  // find ideal aspect ratio of monitor
  /*
  HMONITOR mon = MonitorFromWindow( m_Window.Hwnd, MONITOR_DEFAULTTONEAREST );
  MONITORINFO   mInfo;

  mInfo.cbSize = sizeof( mInfo );

  GR::tPoint  nativeSize;

  if ( GetMonitorInfo( mon, &mInfo ) )
  {
    m_RenderFrame.m_NativeScreenSize.set( mInfo.rcMonitor.right - mInfo.rcMonitor.left, mInfo.rcMonitor.bottom - mInfo.rcMonitor.top );
  }
  else*/
  {
    // will that ever happen?
    m_RenderFrame.m_NativeScreenSize.Set( m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  }
  //dh::Log( "NativeSize: %dx%d   Ratio %.2f", nativeSize.x, nativeSize.y, (float)nativeSize.x / (float)nativeSize.y );

#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
  dh::Log( "SDL_CreateWindow" );
  m_Window.m_pMainWindow = SDL_CreateWindow( m_EnvironmentConfig.Caption.c_str(), 
                                             SDL_WINDOWPOS_UNDEFINED, 
                                             SDL_WINDOWPOS_UNDEFINED, 
                                             m_EnvironmentConfig.StartUpWidth,
                                             m_EnvironmentConfig.StartUpHeight,
                                             SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
  if ( m_Window.m_pMainWindow == NULL )
  {
    return false;
  }
  dh::Log( "SDL_CreateWindow 2" );
  m_Window.m_pMainSurface = SDL_GetWindowSurface( m_Window.m_pMainWindow );
  dh::Log( "SDL_CreateWindow 3" );
#endif

  InitialiseAssets();
  InitialiseImplementationSpecificAssets();

#if OPERATING_SYSTEM == OS_WINDOWS
  return ( !!m_RenderFrame.m_hwndMain );
#else
  return true;
#endif
}



void XFrameApp::InitialiseImplementationSpecificAssets()
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
  int     numTilesets = (int)pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET );
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



void XFrameApp::UpdateAssets( GR::f32 ElapsedTime )
{
  Xtreme::Asset::XAssetLoader*    pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
  }
  int     numTilesets = (int)pLoader->AssetTypeCount( Xtreme::Asset::XA_TILESET );
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



void XFrameApp::SetCustomMouseCursor( const XTextureSection& TexSec, const GR::tPoint& ptHotSpot, const GR::u32 Color )
{
  m_CustomMouseCursorColor = Color;
  m_tsMouseCursor   = TexSec;
  m_MouseHotSpot  = ptHotSpot;

  m_CustomMouseCursorSet = ( m_tsMouseCursor.m_Width != 0 ) && ( m_tsMouseCursor.m_Height != 0 );

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( m_CustomMouseCursorSet )
  {
    AppCursor( NULL );
  }
  else
  {
    AppCursor( LoadCursor( NULL, IDC_ARROW ) );
  }
#endif
}



void XFrameApp::RenderCustomCursor( const GR::tPoint& ptMousePos )
{
  if ( ( m_CustomMouseCursorSet )
  &&   ( m_EnableCursor )
  &&   ( m_pRenderClass ) )
  {
    GR::tPoint    ptOffset = ptMousePos - m_MouseHotSpot;

    if ( ( m_tsMouseCursor.m_pTexture )
    &&   ( GR::Graphic::ImageData::FormatHasAlpha( m_tsMouseCursor.m_pTexture->m_ImageFormat ) ) )
    {
      m_pRenderClass->SetShader( XRenderer::ST_ALPHA_TEST );
    }
    else
    {
      m_pRenderClass->SetShader( XRenderer::ST_FLAT );
    }
    m_pRenderClass->RenderTextureSection2d( ptOffset.x, ptOffset.y,
                                            m_tsMouseCursor, 
                                            m_CustomMouseCursorColor );
  }
}



void XFrameApp::ToggleConsole()
{
  m_ConsoleVisible = !m_ConsoleVisible;
  if ( m_ConsoleVisible )
  {
    if ( m_pInputClass )
    {
      m_pInputClass->Release();
    }

    if ( m_pRenderClass == NULL )
    {
      InvalidateWindow();
    }
  }
  else
  {
    if ( m_pInputClass )
    {
      m_pInputClass->Initialize( GR::Service::Environment::Instance() );
    }
  }
}



void XFrameApp::DisplayConsole()
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
      m_pRenderClass->SetShader( XRenderer::ST_ALPHA_TEST );

      int     iY = m_RenderFrame.m_DisplayMode.Height - 30;

      std::list<GR::String>::reverse_iterator    it( m_listConsoleLines.rbegin() );
      while ( it != m_listConsoleLines.rend() )
      {
        GR::String&    strLine = *it;

        m_pRenderClass->RenderText2d( m_pConsoleFont, 4, iY, strLine.c_str() );

        iY -= 14;
        if ( iY < -14 )
        {
          break;
        }

        ++it;
      }
      m_pRenderClass->RenderText2d( m_pConsoleFont, 4, m_RenderFrame.m_DisplayMode.Height - 15, ( m_ConsoleEingabe + "_" ).c_str() );
    }
  }
  else
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    HDC   hdcMain = GetDC( m_RenderFrame.m_hwndMain );

    RECT  rc;
    GetClientRect( m_RenderFrame.m_hwndMain, &rc );
    FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

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

    rc.top = rc.bottom - 15;
    rc.bottom = rc.top + 14;
    DrawTextA( hdcMain, ( m_ConsoleEingabe + "_" ).c_str(), (int)m_ConsoleEingabe.length() + 1, &rc, DT_LEFT | DT_SINGLELINE );

    ReleaseDC( m_RenderFrame.m_hwndMain, hdcMain );
#endif
  }
}



void XFrameApp::CreateConsoleFont()
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



#if OPERATING_SYSTEM == OS_WINDOWS
HWND XFrameApp::GetSafeHwnd()
{
  return m_RenderFrame.m_hwndMain;
}
#endif



void XFrameApp::Print( int iX, int iY, const char* szText, GR::u32 Color )
{
  if ( m_pRenderClass == NULL )
  {
    return;
  }
  CreateConsoleFont();

  if ( m_pConsoleFont )
  {
    m_pRenderClass->RenderText2d( m_pConsoleFont, iX, iY, szText, Color );
  }
}



void XFrameApp::ProcessEvent( const tGlobalEvent& Event )
{
  if ( Event.m_Type == m_ETChangeState )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      // übergebene Parameter
      GR::Strings::ParameterList        paramList;

      paramList = Event.m_Params;

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
      IGameState<XRenderer>*    pGameState = static_cast<IGameState<XRenderer>*>( pClone );
      IGameState<XRenderer>*    pOldState = NULL;

      if ( Event.m_Params.HasParam( "OldState" ) )
      {
        pOldState = (IGameState<XRenderer>*)GR::Convert::ToUP( Event.m_Params["OldState"] );
      }
      if ( pGameState )
      {
        pGameState->m_ParamInit = paramList;

        m_StateManager.ChangeState( pGameState, pOldState );
      }
      else
      {
        dh::Log( "XFrameApp: Trying to change to unregistered state %s", Event.m_Param.c_str() );
      }
    }
    else
    {
      IGameState<XRenderer>*    pState = (IGameState<XRenderer>*)Event.m_Param1;
      if ( pState == NULL )
      {
        dh::Log( "XFrameApp: ChangeState event without set state encountered!" );
      }
      else
      {
        pState->m_ParamInit = Event.m_Params;
        m_StateManager.ChangeState( (IGameState<XRenderer>*)Event.m_Param1 );
      }
    }
  }
  else if ( ( Event.m_Type == m_ETPopState )
  ||        ( Event.m_Type == m_ETPopAnyState ) )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      // übergebene Parameter
      GR::Strings::ParameterList        paramList;

      paramList = Event.m_Params;

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

      if ( Event.m_Type == m_ETPopAnyState )
      {
        m_StateManager.PopAnyState( paramList );
      }
      else
      {
        m_StateManager.PopState( paramList );
      }
    }
    else
    {
      if ( Event.m_Type == m_ETPopAnyState )
      {
        m_StateManager.PopAnyState( Event.m_Params );
      }
      else
      {
        m_StateManager.PopState( Event.m_Params, (IGameState<XRenderer>*)Event.m_Param1 );
      }
    }
  }
  else if ( ( Event.m_Type == m_ETPushState )
  ||        ( Event.m_Type == m_ETPushStateOnStack ) )
  {
    if ( ( Event.m_Param1 == 0 )
    &&   ( !Event.m_Param.empty() ) )
    {
      // übergebene Parameter
      GR::Strings::ParameterList        paramList;

      paramList = Event.m_Params;

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
      //GUIGameStateManager<XRenderer>::GameStateType   
      IGameState<XRenderer>*    pGameState = static_cast<IGameState<XRenderer>*>( pClone );
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
        dh::Log( "XFrameApp: Trying to push unregistered state %s", strParam.c_str() );
      }
    }
    else
    {
      IGameState<XRenderer>*    pState = (IGameState<XRenderer>*)Event.m_Param1;

      pState->m_ParamInit.Merge( Event.m_Params );

      if ( Event.m_Type == m_ETPushStateOnStack )
      {
        m_StateManager.PushStateOnStack( pState );
      }
      else
      {
        m_StateManager.PushState( pState );
      }
    }
  }
  else if ( Event.m_Type == m_ETShutDown )
  {
    ShutDown( (GR::u32)Event.m_Param1 );
  }
  else if ( Event.m_Type == m_ETRendererInitialised )
  {
  }
  else if ( Event.m_Type == m_ETRendererReleased )
  {
  }
}



bool XFrameApp::ConsoleVisible() const
{

  return m_ConsoleVisible;

}



#if OPERATING_SYSTEM == OS_WINDOWS
void XFrameApp::AppCursor( HCURSOR hCursor )
{
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  m_Window.HCursor = hCursor;
#endif

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  POINT   pt;
  GetCursorPos( &pt );

  bool    bOnNCClient = ( ::SendMessage( m_RenderFrame.m_hwndMain, WM_NCHITTEST, 0, MAKELPARAM( pt.x, pt.y ) ) == HTCLIENT );

  if ( m_CustomMouseCursorSet )
  {
    if ( bOnNCClient )
    {
      SetCursor( NULL );
    }
    return;
  }

  if ( bOnNCClient )
  {
    SetCursor( m_Window.HCursor );
  }
#endif
}
#endif



void XFrameApp::ClipCursor( const GR::tRect& ClipRect )
{
  m_ClipRect = ClipRect;
  ApplyClipCursor( ClipRect );
}



void XFrameApp::ApplyClipCursor( const GR::tRect& ClipRect )
{
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( ClipRect.Empty() )
  {
    ::ClipCursor( NULL );
  }
  else
  {
    POINT   pt;

    pt.x = 0;
    pt.y = 0;

    ::ClientToScreen( m_RenderFrame.m_hwndMain, &pt );

    GR::tRect   innerClipRect( ClipRect );

    innerClipRect.Offset( pt.x, pt.y );

    RECT    rcClip;

    SetRect( &rcClip, innerClipRect.Left, innerClipRect.Top, innerClipRect.Right, innerClipRect.Bottom );
    ::ClipCursor( &rcClip );
  }
#endif
}



void XFrameApp::ResetFrameTime()
{
  GR::f64     NewFrameTime = Time::Timer::Time();

  m_LastFixedFrameTime = NewFrameTime;
  m_LastFrameTime      = NewFrameTime;
}



XTextureSection XFrameApp::Section( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return XTextureSection();
  }

  Xtreme::Asset::XAsset* pSection = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, Name.c_str() );
  if ( pSection == NULL )
  {
    dh::Log( "XFrameApp::Section Section (%s) not found", Name.c_str() );
    return XTextureSection();
  }
  return *(XTextureSection*)pSection->Handle( "Section" );
}



XTexture* XFrameApp::Texture( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Texture No AssetLoader found" );
    return NULL;
  }

  Xtreme::Asset::XAssetImage* pImage = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, Name.c_str() );
  if ( pImage == NULL )
  {
    dh::Log( "XFrameApp::Texture Texture %s not found", Name.c_str() );
    return NULL;
  }
  return pImage->Texture();
}



XFont* XFrameApp::Font( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Font No AssetLoader found" );
    return NULL;
  }

  Xtreme::Asset::XAsset* pMesh = pLoader->Asset( Xtreme::Asset::XA_FONT, Name.c_str() );
  if ( pMesh == NULL )
  {
    dh::Log( "XFrameApp::Font Font %s not found", Name.c_str() );
    return NULL;
  }
  return (XFont*)pMesh->Handle( "Font" );
}



GR::u32 XFrameApp::Sound( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Sound No AssetLoader found" );
    return 0;
  }

  Xtreme::Asset::XAsset* pSound = (Xtreme::Asset::XAsset*)pLoader->Asset( Xtreme::Asset::XA_SOUND, Name.c_str() );
  if ( pSound == NULL )
  {
    dh::Log( "XFrameApp::Sound Sound %s not found", Name.c_str() );
    return 0;
  }
  return (GR::u32)pSound->Handle( "Sound" );
}



XVertexBuffer* XFrameApp::VertexBuffer( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::VertexBuffer No AssetLoader found" );
    return NULL;
  }

  Xtreme::Asset::XAsset* pMesh = pLoader->Asset( Xtreme::Asset::XA_MESH, Name.c_str() );
  if ( pMesh == NULL )
  {
    dh::Log( "XFrameApp::VertexBuffer VertexBuffer %s not found", Name.c_str() );
    return NULL;
  }
  return (XVertexBuffer*)pMesh->Handle( "VertexBuffer" );
}



XMesh* XFrameApp::Mesh( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Mesh No AssetLoader found" );
    return 0;
  }

  Xtreme::Asset::XAsset* pMesh = pLoader->Asset( Xtreme::Asset::XA_MESH, Name.c_str() );
  if ( pMesh == NULL )
  {
    dh::Log( "XFrameApp::Mesh Mesh %s not found", Name.c_str() );
    return 0;
  }
  return (XMesh*)pMesh->Handle( "Mesh" );
}



XBoundingBox XFrameApp::MeshBounds( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::MeshBounds No AssetLoader found" );
    return XBoundingBox();
  }

  Xtreme::Asset::XAsset* pMesh = pLoader->Asset( Xtreme::Asset::XA_MESH, Name.c_str() );
  if ( pMesh == NULL )
  {
    dh::Log( "XFrameApp::MeshBounds MeshBounds %s not found", Name.c_str() );
    return XBoundingBox();
  }
  return *(XBoundingBox*)pMesh->Handle( "BoundingBox" );
}



CSpline XFrameApp::Spline( const GR::String& Name )
{

  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Spline No AssetLoader found" );
    return CSpline();
  }

  Xtreme::Asset::XAssetSpline* pSpline = (Xtreme::Asset::XAssetSpline*)pLoader->Asset( Xtreme::Asset::XA_SPLINE, Name.c_str() );
  if ( pSpline == NULL )
  {
    dh::Log( "XFrameApp::Spline Spline %s not found", Name.c_str() );
    return CSpline();
  }
  return pSpline->Spline();

}



XTextureSection XFrameApp::Tile( const GR::String& Tileset, const int TileIndex )
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



GR::String XFrameApp::TileName( const GR::String& Tileset, const int TileIndex )
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



size_t XFrameApp::TilesetCount( const GR::String& Tileset )
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



GR::u32 XFrameApp::TileFlags( const GR::String& Tileset, const int TileIndex )
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



IAnimationManager<GR::String>::tAnimType XFrameApp::Animation( const GR::String& Name )
{
  Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)GR::Service::Environment::Instance().Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    dh::Log( "XFrameApp::Animation Missing AssetLoader Service" );
    return IAnimationManager<GR::String>::tAnimType();
  }

  Xtreme::Asset::XAssetAnimation* pAnim = (Xtreme::Asset::XAssetAnimation*)pLoader->Asset( Xtreme::Asset::XA_ANIMATION, Name.c_str() );
  if ( pAnim == NULL )
  {
    dh::Log( "XFrameApp::Animation Animation %s not found", Name.c_str() );
    return IAnimationManager<GR::String>::tAnimType();
  }
  return pAnim->Animation();
}



tAnimationPos XFrameApp::AnimationPos( const GR::String& strName )
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



XTextureSection XFrameApp::AnimationFrame( const tAnimationPos& AnimPos )
{
  return Section( m_AnimationManager.GetAnimFrame( AnimPos ) );
}



void XFrameApp::OnEnvironmentEvent( GR::Service::Environment::EnvironmentEvent Event, const char* ServiceName, const char* EventName, GR::IService* pService )
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



void XFrameApp::AdjustCanvas( int Width, int Height )
{
  if ( !m_pRenderClass->IsReady() )
  {
    return;
  }
  //dh::Log( "AdjustCanvas called" );

  //GR::tPoint  origVirtualSize = m_pRenderClass->VirtualSize();

  //dh::Log( "-orig virtual size %dx%d", origVirtualSize.x, origVirtualSize.y );

  /*
  GR::tFPoint   virtualSizeFactor;

  int       origWidth = m_pRenderClass->Width();
  int       origHeight = m_pRenderClass->Height();

  virtualSizeFactor.x = ( GR::f32 )origVirtualSize.x / origWidth;
  virtualSizeFactor.y = ( GR::f32 )origVirtualSize.y / origHeight;

  dh::Log( "-virtual size factor %.2fx%.2f", virtualSizeFactor.x, virtualSizeFactor.y );*/

  if ( m_EnvironmentConfig.KeepAspectRatio )
  {
    // determine whether we require borders
    float     origRatio = (float)m_EnvironmentConfig.StartUpWidth / m_EnvironmentConfig.StartUpHeight;

    float     newRatio = (float)Width / Height;

    //dh::Log( "origRatio %.2f, newRatio %.2f", origRatio, newRatio );

    if ( origRatio != newRatio )
    {
      int     canvasWidth = Width;
      int     canvasHeight = Height;
      if ( origRatio > newRatio )
      {
        // new height is bigger than width
        canvasHeight = (int)( Width / origRatio );
        canvasWidth = Width;
      }
      else
      {
        // new width is bigger than height
        canvasWidth = (int)( Height * origRatio );
        canvasHeight = Height;
      }
      m_pRenderClass->Canvas( GR::tRect( ( Width - canvasWidth ) / 2, ( Height - canvasHeight ) / 2, canvasWidth, canvasHeight ) );

      //dh::Log( "Canvas set to %d,%d %dx%d", ( Width - canvasWidth ) / 2, ( Height - canvasHeight ) / 2, canvasWidth, canvasHeight );
    }
    else
    {
      m_pRenderClass->Canvas( GR::tRect( 0, 0, Width, Height ) );
      //dh::Log( "Canvas set to %d,%d %dx%d", 0, 0, Width, Height );
    }
  }
  else
  {
    // allow full screen
    //dh::Log( "FS Canvas set to %d,%d %dx%d", 0, 0, Width, Height );
    m_pRenderClass->Canvas( GR::tRect( 0, 0, Width, Height ) );

    if ( m_EnvironmentConfig.FixedVirtualSizeFactor.x != 0 )
    {
      m_pRenderClass->VirtualSize( GR::tPoint( (int)( Width * m_EnvironmentConfig.FixedVirtualSizeFactor.x ), (int)( Height * m_EnvironmentConfig.FixedVirtualSizeFactor.y ) ) );
    }
    else
    {
      m_pRenderClass->VirtualSize( GR::tPoint( Width, Height ) );
    }
    //dh::Log( "-new virtual size %dx%d", (int)( Width * virtualSizeFactor.x ), (int)( Height * virtualSizeFactor.y ) );

    auto vp = m_pRenderClass->Viewport();
    vp.Width = Width;
    vp.Height = Height;
    m_pRenderClass->SetViewport( vp );
  }
}



void XFrameApp::ChangeWindowSize( int Width, int Height, int Depth )
{
  bool      wasFullscreen = m_pRenderClass->IsFullscreen();
  int       offsetX = 0;
  int       offsetY = 0;
  int       origWidth   = m_pRenderClass->Width();
  int       origHeight  = m_pRenderClass->Height();
  GR::tPoint  origVirtualSize = m_pRenderClass->VirtualSize();

  GR::tFPoint   virtualSizeFactor;

  virtualSizeFactor.x = ( GR::f32 )origVirtualSize.x / origWidth;
  virtualSizeFactor.y = ( GR::f32 )origVirtualSize.y / origHeight;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  RECT      rcClientSize;

  SetRect( &rcClientSize, 0, 0, Width, Height );
#endif
  if ( !wasFullscreen )
  {
    //dh::Log( "!wasFullscreen" );
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    AdjustWindowRectEx( &rcClientSize, GetWindowLong( m_RenderFrame.m_hwndMain, GWL_STYLE ), ( GetMenu( m_RenderFrame.m_hwndMain ) != NULL ), GetWindowLong( m_RenderFrame.m_hwndMain, GWL_EXSTYLE ) );

    offsetX = ( GetSystemMetrics( SM_CXSCREEN ) - ( rcClientSize.right - rcClientSize.left ) ) / 2;
    offsetY = ( GetSystemMetrics( SM_CYSCREEN ) - ( rcClientSize.bottom - rcClientSize.top ) ) / 2;

    RECT    rcMain;

    GetWindowRect( m_RenderFrame.m_hwndMain, &rcMain );

    if ( ( rcMain.left != offsetX )
    ||   ( rcMain.top != offsetY )
    ||   ( rcMain.right - rcMain.left != rcClientSize.right - rcClientSize.left )
    ||   ( rcMain.bottom - rcMain.top != rcClientSize.bottom - rcClientSize.top ) )
    {
      // only set when it changes
      SetWindowPos( m_RenderFrame.m_hwndMain, NULL, offsetX, offsetY, rcClientSize.right - rcClientSize.left, rcClientSize.bottom - rcClientSize.top, 0 );
    }

    AdjustCanvas( Width, Height );
#endif
  }
  else
  {
    //dh::Log( "wasFullscreen, re-calling SetMode" );
    if ( m_pRenderClass->IsReady() )
    {
      XRendererDisplayMode    mode( Width, Height, GR::Graphic::ImageData::ImageFormatFromDepth( Depth ), true );
      m_pRenderClass->SetMode( mode );
    }

    // adjust window size (does that even work in full screen?)
    //SetWindowPos( m_RenderFrame.m_hwndMain, NULL, 0, 0, Width, Height, 0 );
  }
  m_RenderFrame.SetSize( Width, Height );
  m_GUI.SetScreenSizes( GR::tPoint( Width, Height ), m_GUI.VirtualSize() );
}



void XFrameApp::DetermineBestFullscreenMatch( XRendererDisplayMode& Mode )
{
  if ( m_pRenderClass == NULL )
  {
    return;
  }

  std::vector<XRendererDisplayMode>       modeList( m_pRenderClass->NumberDisplayModes() );
  if ( modeList.empty() )
  {
    return;
  }

  if ( !m_pRenderClass->ListDisplayModes( &modeList[0], modeList.size() * sizeof( XRendererDisplayMode ) ) )
  {
    dh::Log( "failed to retrieve modes" );
    return;
  }


  //dh::Log( "fullscreen for %dx%d (startup %dx%d)", Mode.Width, Mode.Height, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  //dh::Log( "Found %d modes", modeList.size() );
  // is there a a matching mode?
  std::vector<XRendererDisplayMode>::iterator   it( modeList.begin() );
  while ( it != modeList.end() )
  {
    auto&  mode( *it );

    if ( ( mode.Width == Mode.Width )
    &&   ( mode.Height == Mode.Height )
    &&   ( mode.FullScreen ) )
    {
      // full match!
      dh::Log( "-full match" );
      return;
    }
    ++it;
  }

  // find best fit
  float                   factorX = 5000.0f;
  float                   factorY = 5000.0f;

  XRendererDisplayMode    bestMode( Mode );

  it = modeList.begin();
  while ( it != modeList.end() )
  {
    auto&  mode( *it );

    float   ratio = (float)mode.Width / mode.Height;

    //dh::Log( "check mode %dx%d (fs %d), Ratio %.2f", mode.Width, mode.Height, mode.FullScreen, ratio );
    if ( ( mode.Width < Mode.Width )
    ||   ( mode.Height < Mode.Height ) )
    {
      // too small, not fullscreen
      ++it;
      continue;
    }
    //dh::Log( "fullscreen for %dx%d", mode.Width, mode.Height );

    float     curXFactor = (float)mode.Width / Mode.Width;
    float     curYFactor = (float)mode.Height / Mode.Height;

    if ( ( curXFactor < factorX )
    &&   ( curYFactor < factorY ) )
    {
      //dh::Log( "next best match: %dx%d", mode.Width, mode.Height );
      factorX = curXFactor;
      factorY = curYFactor;
      bestMode = mode;
    }
    /*
    if ( !m_EnvironmentConfig.FixedSize )
    {
      // allow bigger modes
    }*/
    ++it;
  }
  Mode = bestMode;
  Mode.FullScreen = true;
  //dh::Log( "using mode : %dx%d", Mode.Width, Mode.Height );

  if ( m_EnvironmentConfig.FixedSize )
  {
    int     xOffset = ( Mode.Width - m_EnvironmentConfig.StartUpWidth ) / 2;
    int     yOffset = ( Mode.Height - m_EnvironmentConfig.StartUpHeight ) / 2;

    dh::Log( "Offset %d,%d", xOffset, yOffset );
    m_EnvironmentDisplayRect.Set( xOffset, yOffset, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );
  }
}



void XFrameApp::KeepMouseInsideDuringFullscreen( bool KeepInside )
{
  m_KeepMouseInsideDuringFullscreen = KeepInside;
  if ( m_pRenderClass->IsFullscreen() )
  {
  }
}



void XFrameApp::ToggleFullscreen()
{
  EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_PRE_WINDOW_MODE_CHANGE ) );

  XRendererDisplayMode    mode;

  mode.Width        = m_pRenderClass->Width();
  mode.Height       = m_pRenderClass->Height();
  mode.ImageFormat  = m_pRenderClass->ImageFormat();
  mode.FullScreen   = !m_pRenderClass->IsFullscreen();


  if ( mode.FullScreen )
  {
    m_StoredWindowedMode = mode;
    m_StoredWindowedMode.FullScreen = false;
    DetermineBestFullscreenMatch( mode );
  }
  else
  {
    // TODO - restore remembered window size
    m_EnvironmentDisplayRect.Set( 0, 0, m_EnvironmentConfig.StartUpWidth, m_EnvironmentConfig.StartUpHeight );

    mode = m_StoredWindowedMode;
  }
  m_pRenderClass->Canvas( m_EnvironmentDisplayRect );

  if ( m_pRenderClass->SetMode( mode ) )
  {
    if ( !m_pRenderClass->IsFullscreen() )
    {
      //dh::Log( "calling changewindowsize" );
      // does window size match?
      ChangeWindowSize( mode.Width, mode.Height, GR::Graphic::ImageData::DepthFromImageFormat( mode.ImageFormat ) );
    }
    else
    {
      AdjustCanvas( mode.Width, mode.Height );
    }
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_WINDOW_MODE_CHANGED ) );
  }
  else
  {
    AdjustCanvas( mode.Width, mode.Height );
  }
  m_RenderFrame.m_DisplayMode.FullScreen = m_pRenderClass->IsFullscreen();
}



void XFrameApp::DisplayFrame( XRenderer& Renderer )
{
  Renderer.Clear();
  Renderer.SetState( XRenderer::RS_ZWRITE, XRenderer::RSV_ENABLE );

  m_GUI.SetRenderer( &Renderer );
  m_GUI.SetExtents( m_RenderFrame.Width(), m_RenderFrame.Height() );
  m_GUI.DisplayBackground();

  XViewport   vp = Renderer.Viewport();

  std::set<GUIComponent*>   displayedStateScreens;

  int   clipRects = m_GUI.m_ClipRects.size();

  auto it( m_StateManager.m_GameStateStack.begin() );
  while ( it != m_StateManager.m_GameStateStack.end() )
  {
    GUIGameState<XRenderer,GUIComponent>*    pGameState = (GUIGameState<XRenderer,GUIComponent>*)*it;

    Renderer.SetViewport( vp );
    Renderer.Offset( GR::tPoint() );

    m_GUI.DisplayComponentFull( (GUIComponent*)pGameState->m_pStateScreen, 
                                GR::tPoint( 0, 0 ), 
                                GR::tRect( 0, 0, Renderer.Width(), Renderer.Height() ) );
    displayedStateScreens.insert( (GUIComponent*)pGameState->m_pStateScreen );
    ++it;
  }

  int   clipRects2 = m_GUI.m_ClipRects.size();

  // display all controls that are not direct part of the states (hottips etc.)
  for ( size_t i = 0; i < m_GUI.m_Components.size(); ++i )
  {
    GUIComponent*   pComp = (GUIComponent*)m_GUI.m_Components[i];

    if ( displayedStateScreens.find( pComp ) == displayedStateScreens.end() )
    {
      Renderer.SetViewport( vp );
      m_GUI.DisplayComponentFromParent( pComp );
    }
  }

  int   clipRects3 = m_GUI.m_ClipRects.size();

  if ( clipRects != clipRects2 )
  {
    dh::Log( "Gamestate cliprects mismatch!" );
  }
  if ( clipRects != clipRects3 )
  {
    dh::Log( "GUI cliprects mismatch!" );
  }

  Renderer.SetViewport( vp );
}



void XFrameApp::OnPauseApplication()
{
  m_StateManager.OnPauseApplication();
}



void XFrameApp::OnResumeApplication()
{
  m_StateManager.OnResumeApplication();
}



GR::f64 XFrameApp::FixedTimeStep() const
{
  return m_FixedLogicTimeStep;
}



void XFrameApp::OnKeyDown( char Key )
{
#ifdef VK_F11
  if ( Key == VK_F11 )
  {
    ToggleConsole();
  }
#endif
  if ( Renderer() == NULL )
  {
    m_Window.Invalidate();
  }
}



void XFrameApp::OnKeyUp( char Key )
{
}



void XFrameApp::OnDisplayFrame()
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
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    else
    {
      PAINTSTRUCT   ps;

      HDC   hdcMain = BeginPaint( m_RenderFrame.m_hwndMain, &ps );

      RECT  rc;
      GetClientRect( m_RenderFrame.m_hwndMain, &rc );

      switch ( rand() % 2 )
      {
        case 0:
          FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
          break;
        case 1:
          FillRect( hdcMain, &rc, (HBRUSH)GetStockObject( WHITE_BRUSH ) );
          break;
      }

      EndPaint( m_RenderFrame.m_hwndMain, &ps );
    }
#endif
  }
}



void XFrameApp::OnSizeChanged()
{
  if ( m_pRenderClass )
  {
    m_pRenderClass->OnResized();
    EventProducer<GR::Gamebase::tXFrameEvent>::SendEvent( GR::Gamebase::tXFrameEvent( GR::Gamebase::tXFrameEvent::ET_DISPLAY_MODE_CHANGED ) );
  }
}