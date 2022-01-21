#include <OS/OS.h>

#include <SDL.h>

#include <Lang/Service.h>

#include <GR/Gamebase/Framework.h>

#include <Xtreme/Environment/EnvironmentConfig.h>

#include <Xtreme/SDL/SDLInput.h>
#include <Xtreme/SDL/SDLSound.h>

#include <Xtreme/OpenGL/OpenGLRenderClass.h>

#include <emscripten.h>



int main( int argc, char* argv[] )
{
  GR::IEnvironment&     environment = GR::Service::Environment::Instance();


  GR::Gamebase::Framework* pApp = (GR::Gamebase::Framework*)environment.Service( "Application" );

  if ( pApp == NULL )
  {
    return 1;
  }

  // activate and enable "user app data" path to persist
  MAIN_THREAD_EM_ASM( 
    var profilePath = '/app';
    try
    {
      console.log( 'mkdir profilePath' );
      FS.mkdir( profilePath );
    }
    catch ( e )
    {
      console.log( e );
    }
    FS.mount( IDBFS, {}, profilePath );

    // this crap must be run async -> so put mkdir in callback method
    FS.syncfs( true, function( err )
    {
      if ( err ) throw err;

      try
      {
        console.log( 'mkdir /app/data' );
        FS.mkdir( '/app/data' );
        console.log( 'mkdir /app/data done' );
      }
      catch ( e )
      {
        console.log( e );
      }
    } );
  );

  pApp->Configure( pApp->m_EnvironmentConfig );

  // MUST BE CALLED BEFORE SDL_init!!
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
  SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 4 );

  // Turn on double buffering with a 24bit Z buffer.
  // You may need to change this to 16 or 32 for your system
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );

  if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER ) != 0 )
  {
    dh::Log( "SDL_Init failed (%s)", SDL_GetError() );
    return 1;
  }

  SDLSound* pSound = new SDLSound();
  //environment.SetService( "Renderer", new OpenGLRenderClass() );
  environment.SetService( "Input", new SDLInput() );
  environment.SetService( "Sound", (XSoundBase*)pSound );
  environment.SetService( "Music", (XMusic*)pSound );

  pApp->Run();

  return 0;
}