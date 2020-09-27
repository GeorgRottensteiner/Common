#include <jni.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <android/log.h>

#include "AndroidMain.h"

#include <Xtreme/Android/AndroidInput.h>
#include <Xtreme/Android/AndroidRenderer.h>
#include <Xtreme/Android/AndroidFile.h>

#include <MasterFrame\XFrameApp.h>



GR::String     s_AndroidInternalDataPath;
GR::String     s_AndroidExternalDataPath;


// some Android NDK functions are inside std:: namespace although they shouldn't?
int wctomb( char* s, wchar_t wc ) { return wcrtomb( s, wc, NULL ); }
int mbtowc( wchar_t* pwc, const char* s, size_t n ) { return mbrtowc( pwc, s, n, NULL ); }

// 
int iswblank( wint_t c )
{
  return std::iswblank( c );
}

unsigned long long wcstoull( const wchar_t* str, wchar_t** str_end, int base )
{
  return std::wcstoull( str, str_end, base );
}

long long wcstoll( const wchar_t* str, wchar_t** str_end, int base )
{
  return std::wcstoll( str, str_end, base );
}

float       wcstof( const wchar_t* str, wchar_t** str_end )
{
  return std::wcstof( str, str_end );
}




/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display( struct engine* engine )
{
  engine->state.angle = 0;
  return 0;
}



/**
* Just the current frame in the display.
*/
static void engine_draw_frame( struct engine* pEngine, XRenderer& Renderer )
{
  // Just fill the screen with a color.
  int     width = 640;  // Renderer.Width()
  int     height = 480; // Renderer.Height()
  glClearColor( ( (float)pEngine->state.x ) / width, pEngine->state.angle, ( (float)pEngine->state.y ) / height, 1 );
  glClear( GL_COLOR_BUFFER_BIT );
}



/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display( struct engine* engine )
{
  engine->animating = 0;
}



ANativeActivity* AndroidMediator::s_pActivity = NULL;



AndroidMediator::AndroidMediator() :
  m_pSavedState( NULL ),
  m_SavedStateSize( 0 ),
  userData( NULL ),
  config( NULL ),
  looper( NULL ),
  inputQueue( NULL ),
  window( NULL ),
  activityState( 0 ),
  destroyRequested( 0 ),
  msgread( 0 ),
  msgwrite( 0 ),
  thread( 0 ),
  m_Running( false ),
  stateSaved( 0 ),
  destroyed( 0 ),
  redrawNeeded( 0 ),
  pendingInputQueue( NULL ),
  pendingWindow( NULL )
{
  s_pActivity = NULL;
}



// Process the next input event.

int32_t AndroidMediator::HandleInput( AInputEvent* event )
{
  struct engine* engine = ( struct engine* )userData;
  if ( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_MOTION )
  {
    engine->state.x = AMotionEvent_getX( event, 0 );
    engine->state.y = AMotionEvent_getY( event, 0 );
    return 1;
  }
  return 0;
}



// Process the next main command.
void AndroidMediator::HandleCmd( int32_t cmd )
{
  struct engine*    engine = ( struct engine* )userData;
  XFrameApp*        pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );

  switch ( cmd )
  {
    case APP_CMD_SAVE_STATE:
      // The system has asked us to save our current state.  Do so.
      m_pSavedState = malloc( sizeof( struct saved_state ) );
      *( ( struct saved_state* )m_pSavedState ) = engine->state;
      m_SavedStateSize = sizeof( struct saved_state );
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if ( window != NULL )
      {
        ( (AndroidRenderer*)pApp->Renderer() )->m_Window = window;
        if ( !pApp->Renderer()->Initialize( 640, //pApp->m_RenderFrame.m_DisplayMode.Width,
                                                480, //pApp->m_RenderFrame.m_DisplayMode.Height,
                                                32, //GR::Graphic::ImageData::DepthFromImageFormat( pApp->m_RenderFrame.m_DisplayMode.ImageFormat ),
                                                false,//m_RenderFrame.m_DisplayMode.FullScreen,
                                                GR::Service::Environment::Instance() ) )
        {
          return;
        }
        engine_init_display( engine );
        engine_draw_frame( engine, *pApp->Renderer() );
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      engine_term_display( engine );
      break;
    case APP_CMD_GAINED_FOCUS:
      // When our app gains focus, we start monitoring the accelerometer.
      {
        AndroidInput* pInput = (AndroidInput*)pApp->InputClass();
        if ( pInput != NULL )
        {
          pInput->SetActive();
        }
      }
      break;
    case APP_CMD_LOST_FOCUS:
      {
        AndroidInput* pInput = (AndroidInput*)pApp->InputClass();
        if ( pInput != NULL )
        {
          pInput->SetActive( false );
        }
      }
      // Also stop animating.
      engine->animating = 0;
      engine_draw_frame( engine, *pApp->Renderer() );
      break;
  }
}



void AndroidMediator::FreeSavedState()
{
  pthread_mutex_lock( &mutex );
  if ( m_pSavedState != NULL )
  {
    free( m_pSavedState );
    m_pSavedState = NULL;
    m_SavedStateSize = 0;
  }
  pthread_mutex_unlock( &mutex );
}



int8_t AndroidMediator::ReadCmd()
{
  int8_t cmd;
  if ( read( msgread, &cmd, sizeof( cmd ) ) == sizeof( cmd ) )
  {
    switch ( cmd )
    {
      case APP_CMD_SAVE_STATE:
        FreeSavedState();
        break;
    }
    return cmd;
  }
  else
  {
    dh::Log( "No data on command pipe!" );
  }
  return -1;
}



void AndroidMediator::PrintCurrentConfig()
{
  char lang[2], country[2];
  AConfiguration_getLanguage( config, lang );
  AConfiguration_getCountry( config, country );

  dh::Log( "Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
        "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
        "modetype=%d modenight=%d",
        AConfiguration_getMcc( config ),
        AConfiguration_getMnc( config ),
        lang[0], lang[1], country[0], country[1],
        AConfiguration_getOrientation( config ),
        AConfiguration_getTouchscreen( config ),
        AConfiguration_getDensity( config ),
        AConfiguration_getKeyboard( config ),
        AConfiguration_getNavigation( config ),
        AConfiguration_getKeysHidden( config ),
        AConfiguration_getNavHidden( config ),
        AConfiguration_getSdkVersion( config ),
        AConfiguration_getScreenSize( config ),
        AConfiguration_getScreenLong( config ),
        AConfiguration_getUiModeType( config ),
        AConfiguration_getUiModeNight( config ) );
}



void AndroidMediator::PreExecCmd( int8_t cmd )
{
  switch ( cmd )
  {
    case APP_CMD_INPUT_CHANGED:
      dh::Log( "APP_CMD_INPUT_CHANGED\n" );
      pthread_mutex_lock( &mutex );
      if ( inputQueue != NULL )
      {
        AInputQueue_detachLooper( inputQueue );
      }
      inputQueue = pendingInputQueue;
      if ( inputQueue != NULL )
      {
        dh::Log( "Attaching input queue to looper" );
        AInputQueue_attachLooper( inputQueue,
                                  looper, LOOPER_ID_INPUT, NULL,
                                  &inputPollSource );
      }
      pthread_cond_broadcast( &cond );
      pthread_mutex_unlock( &mutex );
      break;
    case APP_CMD_INIT_WINDOW:
      dh::Log( "APP_CMD_INIT_WINDOW\n" );
      pthread_mutex_lock( &mutex );
      window = pendingWindow;
      pthread_cond_broadcast( &cond );
      pthread_mutex_unlock( &mutex );
      break;
    case APP_CMD_TERM_WINDOW:
      dh::Log( "APP_CMD_TERM_WINDOW\n" );
      pthread_cond_broadcast( &cond );
      break;
    case APP_CMD_RESUME:
    case APP_CMD_START:
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
      dh::Log( "activityState=%d\n", cmd );
      pthread_mutex_lock( &mutex );
      activityState = cmd;
      pthread_cond_broadcast( &cond );
      pthread_mutex_unlock( &mutex );
      break;
    case APP_CMD_CONFIG_CHANGED:
      dh::Log( "APP_CMD_CONFIG_CHANGED\n" );
      AConfiguration_fromAssetManager( config, s_pActivity->assetManager );

      PrintCurrentConfig();
      break;
    case APP_CMD_DESTROY:
      dh::Log( "APP_CMD_DESTROY\n" );
      destroyRequested = 1;
      break;
  }
}



void AndroidMediator::PostExecCmd( int8_t cmd )
{
  switch ( cmd )
  {
    case APP_CMD_TERM_WINDOW:
      dh::Log( "APP_CMD_TERM_WINDOW\n" );
      pthread_mutex_lock( &mutex );
      window = NULL;
      pthread_cond_broadcast( &cond );
      pthread_mutex_unlock( &mutex );
      break;
    case APP_CMD_SAVE_STATE:
      dh::Log( "APP_CMD_SAVE_STATE\n" );
      pthread_mutex_lock( &mutex );
      stateSaved = 1;
      pthread_cond_broadcast( &cond );
      pthread_mutex_unlock( &mutex );
      break;
    case APP_CMD_RESUME:
      FreeSavedState();
      break;
  }
}



void AndroidMediator::Destroy()
{
  dh::Log( "android_app_destroy!" );

  FreeSavedState();
  pthread_mutex_lock( &mutex );
  if ( inputQueue != NULL )
  {
    AInputQueue_detachLooper( inputQueue );
  }
  AConfiguration_delete( config );
  destroyed = 1;
  pthread_cond_broadcast( &cond );
  pthread_mutex_unlock( &mutex );

  // Can't touch AndroidMediator object after this.
}



void AndroidMediator::ProcessInput()
{
  AInputEvent* event = NULL;
  while ( AInputQueue_getEvent( inputQueue, &event ) >= 0 )
  {
    dh::Log( "New input event: type=%d\n", AInputEvent_getType( event ) );
    if ( AInputQueue_preDispatchEvent( inputQueue, event ) )
    {
      continue;
    }
    int32_t handled = 0;

    HandleInput( event );

    AInputQueue_finishEvent( inputQueue, event, handled );
  }
}



void AndroidMediator::ProcessCmd()
{
  int8_t cmd = ReadCmd();
  PreExecCmd( cmd );
  HandleCmd( cmd );
  PostExecCmd( cmd );
}



void* AndroidMediator::AppEntry( void* param )
{
  AndroidMediator* pMediator = (AndroidMediator*)param;

  pMediator->RunMain();

  return NULL;
}



void AndroidMediator::RunMain()
{
  config = AConfiguration_new();
  AConfiguration_fromAssetManager( config, s_pActivity->assetManager );

  m_InternalDataPath = s_pActivity->internalDataPath;
  m_ExternalDataPath = s_pActivity->externalDataPath;

  s_AndroidInternalDataPath = m_InternalDataPath;
  s_AndroidExternalDataPath = m_ExternalDataPath;

  PrintCurrentConfig();

  cmdPollSource.id               = LOOPER_ID_MAIN;
  cmdPollSource.app              = this;
  cmdPollSource.ProcessHandler   = fastdelegate::MakeDelegate( this, &AndroidMediator::ProcessCmd );
  inputPollSource.id             = LOOPER_ID_INPUT;
  inputPollSource.app            = this;
  inputPollSource.ProcessHandler = fastdelegate::MakeDelegate( this, &AndroidMediator::ProcessInput );

  ALooper* newLooper = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
  ALooper_addFd( newLooper, msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &cmdPollSource );
  looper = newLooper;

  pthread_mutex_lock( &mutex );
  m_Running = true;
  pthread_cond_broadcast( &cond );
  pthread_mutex_unlock( &mutex );

  Main();

  Destroy();
}



// --------------------------------------------------------------------
// best actual entry point, Native activity interaction (called from main thread)
// --------------------------------------------------------------------

AndroidMediator* AndroidMediator::CreateApp( ANativeActivity* activity, void* savedState, size_t savedStateSize )
{
  AndroidMediator*  pMediator = new AndroidMediator();

  XFrameApp*        pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );
  pApp->SetVarUP( "AndroidMediator", ( GR::up )pMediator );

  return pMediator->Create( activity, savedState, savedStateSize );
}



AndroidMediator* AndroidMediator::Create( ANativeActivity* pActivity, void* savedState, size_t savedStateSize )
{
  s_pActivity = pActivity;

  pthread_mutex_init( &mutex, NULL );
  pthread_cond_init( &cond, NULL );

  if ( savedState != NULL )
  {
    savedState = malloc( savedStateSize );
    savedStateSize = savedStateSize;
    memcpy( savedState, savedState, savedStateSize );
  }

  int msgpipe[2];
  if ( pipe( msgpipe ) )
  {
    dh::Log( "could not create pipe: %s", strerror( errno ) );
    return NULL;
  }
  msgread = msgpipe[0];
  msgwrite = msgpipe[1];

  pthread_attr_t attr;
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  pthread_create( &thread, &attr, AppEntry, this );

  // Wait for thread to start.
  pthread_mutex_lock( &mutex );
  while ( !m_Running )
  {
    pthread_cond_wait( &cond, &mutex );
  }
  pthread_mutex_unlock( &mutex );

  return this;
}



void AndroidMediator::WriteCmd( int8_t cmd )
{
  if ( write( msgwrite, &cmd, sizeof( cmd ) ) != sizeof( cmd ) )
  {
    dh::Log( "Failure writing android_app cmd: %s\n", strerror( errno ) );
  }
}



void AndroidMediator::SetInput( AInputQueue* inputQueue )
{
  pthread_mutex_lock( &mutex );

  pendingInputQueue = inputQueue;

  WriteCmd( APP_CMD_INPUT_CHANGED );

  while ( inputQueue != pendingInputQueue )
  {
    pthread_cond_wait( &cond, &mutex );
  }
  pthread_mutex_unlock( &mutex );
}



void AndroidMediator::SetWindow( ANativeWindow* window )
{
  pthread_mutex_lock( &mutex );
  if ( pendingWindow != NULL )
  {
    WriteCmd( APP_CMD_TERM_WINDOW );
  }
  pendingWindow = window;
  if ( window != NULL )
  {
    WriteCmd( APP_CMD_INIT_WINDOW );
  }
  while ( window != pendingWindow )
  {
    pthread_cond_wait( &cond, &mutex );
  }
  pthread_mutex_unlock( &mutex );
}



void AndroidMediator::SetActivityState( int8_t cmd )
{
  pthread_mutex_lock( &mutex );

  WriteCmd( cmd );
  while ( activityState != cmd )
  {
    pthread_cond_wait( &cond, &mutex );
  }
  pthread_mutex_unlock( &mutex );
}



void AndroidMediator::FreeSelf()
{
  pthread_mutex_lock( &mutex );
  WriteCmd( APP_CMD_DESTROY );
  while ( !destroyed )
  {
    pthread_cond_wait( &cond, &mutex );
  }
  pthread_mutex_unlock( &mutex );

  close( msgread );
  close( msgwrite );
  pthread_cond_destroy( &cond );
  pthread_mutex_destroy( &mutex );

  // uh oh?
  delete this;
}



void AndroidMediator::onDestroy( ANativeActivity* activity )
{
  dh::Log( "Destroy: %p\n", activity );
  AndroidMediator*  pMediator = (AndroidMediator*)activity->instance;

  pMediator->FreeSelf();
}



void AndroidMediator::onStart( ANativeActivity* activity )
{
  dh::Log( "Start: %p\n", activity );
  AndroidMediator*  pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetActivityState( APP_CMD_START );
}



void AndroidMediator::onResume( ANativeActivity* activity )
{
  dh::Log( "Resume: %p\n", activity );
  AndroidMediator*  pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetActivityState( APP_CMD_RESUME );
}



void* AndroidMediator::onSaveInstanceState( ANativeActivity* activity, size_t* outLen )
{
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  return pMediator->SaveInstanceState( activity, outLen );
}



void* AndroidMediator::SaveInstanceState( ANativeActivity* activity, size_t* outLen )
{
  void* savedStateResult = NULL;

  dh::Log( "SaveInstanceState: %p\n", activity );
  pthread_mutex_lock( &mutex );
  stateSaved = 0;
  WriteCmd( APP_CMD_SAVE_STATE );
  while ( !stateSaved )
  {
    pthread_cond_wait( &cond, &mutex );
  }

  if ( m_pSavedState != NULL )
  {
    savedStateResult  = m_pSavedState;
    *outLen           = m_SavedStateSize;
    m_pSavedState     = NULL;
    m_SavedStateSize  = 0;
  }

  pthread_mutex_unlock( &mutex );

  return savedStateResult;
}



void AndroidMediator::onPause( ANativeActivity* activity )
{
  dh::Log( "Pause: %p\n", activity );
  AndroidMediator*  pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetActivityState( APP_CMD_PAUSE );
}



void AndroidMediator::onStop( ANativeActivity* activity )
{
  dh::Log( "Stop: %p\n", activity );
  AndroidMediator*  pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetActivityState( APP_CMD_STOP );
}



void AndroidMediator::onConfigurationChanged( ANativeActivity* activity )
{
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  dh::Log( "ConfigurationChanged: %p\n", activity );

  pMediator->WriteCmd( APP_CMD_CONFIG_CHANGED );
}



void AndroidMediator::onLowMemory( ANativeActivity* activity )
{
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  dh::Log( "LowMemory: %p\n", activity );
  pMediator->WriteCmd( APP_CMD_LOW_MEMORY );
}



void AndroidMediator::onWindowFocusChanged( ANativeActivity* activity, int focused )
{
  dh::Log( "WindowFocusChanged: %p -- %d\n", activity, focused );
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  pMediator->WriteCmd( focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS );
}



void AndroidMediator::onNativeWindowCreated( ANativeActivity* activity, ANativeWindow* window )
{
  dh::Log( "NativeWindowCreated: %p -- %p\n", activity, window );
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetWindow( window );
}



void AndroidMediator::onNativeWindowDestroyed( ANativeActivity* activity, ANativeWindow* window )
{
  dh::Log( "NativeWindowDestroyed: %p -- %p\n", activity, window );
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetWindow( NULL );
}



void AndroidMediator::onInputQueueCreated( ANativeActivity* activity, AInputQueue* queue )
{
  dh::Log( "InputQueueCreated: %p -- %p\n", activity, queue );
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetInput( queue );
}



void AndroidMediator::onInputQueueDestroyed( ANativeActivity* activity, AInputQueue* queue )
{
  dh::Log( "InputQueueDestroyed: %p -- %p\n", activity, queue );
  AndroidMediator* pMediator = (AndroidMediator*)activity->instance;

  pMediator->SetInput( NULL );
}



// actual entry point!
void ANativeActivity_onCreate( ANativeActivity* activity, void* savedState, size_t savedStateSize )
{
  dh::Log( "Creating: %p\n", activity );
  activity->callbacks->onDestroy                = AndroidMediator::onDestroy;
  activity->callbacks->onStart                  = AndroidMediator::onStart;
  activity->callbacks->onResume                 = AndroidMediator::onResume;
  activity->callbacks->onSaveInstanceState      = AndroidMediator::onSaveInstanceState;
  activity->callbacks->onPause                  = AndroidMediator::onPause;
  activity->callbacks->onStop                   = AndroidMediator::onStop;
  activity->callbacks->onConfigurationChanged   = AndroidMediator::onConfigurationChanged;
  activity->callbacks->onLowMemory              = AndroidMediator::onLowMemory;
  activity->callbacks->onWindowFocusChanged     = AndroidMediator::onWindowFocusChanged;
  activity->callbacks->onNativeWindowCreated    = AndroidMediator::onNativeWindowCreated;
  activity->callbacks->onNativeWindowDestroyed  = AndroidMediator::onNativeWindowDestroyed;
  activity->callbacks->onInputQueueCreated      = AndroidMediator::onInputQueueCreated;
  activity->callbacks->onInputQueueDestroyed    = AndroidMediator::onInputQueueDestroyed;

  activity->instance = AndroidMediator::CreateApp( activity, savedState, savedStateSize );
}




void OpenFile( const GR::String& Filename )
{
  AAssetManager* pManager = AndroidMediator::s_pActivity->assetManager;

  GR::IO::AndroidFile     afile;

  if ( !afile.Open( Filename.c_str() ) )
  {
    dh::Log( "Failed to open file %s", Filename.c_str() );
    return;
  }

  afile.Close();

  AAsset* file = AAssetManager_open( pManager, Filename.c_str(), AASSET_MODE_BUFFER );

  /*
  AAssetDir* assetDir = AAssetManager_openDir( pManager, "" );

  const char* filename;
  std::vector<char> buffer;

  while ( ( filename = AAssetDir_getNextFileName( assetDir ) ) != NULL )
  {
    //search for desired file
    dh::Log( "File: (%s)", filename );
    if ( Filename == filename )
    {
      AAsset *asset = AAssetManager_open( mgr, filename, AASSET_MODE_STREAMING );

      //holds size of searched file
      off64_t length = AAsset_getLength64( asset );
      //keeps track of remaining bytes to read
      off64_t remaining = AAsset_getRemainingLength64( asset );
      size_t Mb = 1000 * 1024; // 1Mb is maximum chunk size for compressed assets
      size_t currChunk;
      buffer.reserve( length );

      //while we have still some data to read
      while ( remaining != 0 )
      {
        //set proper size for our next chunk
        if ( remaining >= Mb )
        {
          currChunk = Mb;
        }
        else
        {
          currChunk = remaining;
        }
        char chunk[currChunk];

        //read data chunk
        if ( AAsset_read( asset, chunk, currChunk ) > 0 ) // returns less than 0 on error
        {
          //and append it to our vector
          buffer.insert( buffer.end(), chunk, chunk + currChunk );
          remaining = AAsset_getRemainingLength64( asset );
        }
      }
      AAsset_close( asset );
    }

  }*/
}



/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void AndroidMediator::Main()
{
  XFrameApp*        pApp = (XFrameApp*)GR::Service::Environment::Instance().Service( "Application" );



  struct engine engine;

  memset( &engine, 0, sizeof( engine ) );
  userData        = &engine;
  engine.app      = this;

  if ( m_pSavedState != NULL )
  {
    // We are starting with a previous saved state; restore from it.
    engine.state = *(struct saved_state*)m_pSavedState;
  }

  engine.animating = 1;

  pApp->m_Window.m_pMainWindow = window;

  //OpenFile( "texts.dat" );

  // loop waiting for stuff to do.
  if ( pApp->Run() != 0 )
  {
    return;
  }

  while ( true )
  {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ( ( ident = ALooper_pollAll( engine.animating ? 0 : -1, NULL, &events, (void**)&source ) ) >= 0 )
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
        AndroidInput* pInput = (AndroidInput*)pApp->InputClass();

        pInput->Poll();
      }

      // Check if we are exiting.
      if ( destroyRequested != 0 )
      {
        engine_term_display( &engine );
        return;
      }
    }

    if ( engine.animating )
    {
      // Done with events; draw next animation frame.
      engine.state.angle += .01f;
      if ( engine.state.angle > 1 )
      {
        engine.state.angle = 0;
      }

      // Drawing is throttled to the screen update rate, so there
      // is no need to do timing here.
      engine_draw_frame( &engine, *pApp->Renderer() );
    }
  }
}
