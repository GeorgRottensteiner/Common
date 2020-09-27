#include <SDL.h>

#include <Lang/Service.h>

#include <GR/Gamebase/Framework.h>

#include <Xtreme/Environment/EnvironmentConfig.h>

#include <Xtreme/SDL/SDLInput.h>
#include <Xtreme/SDL/SDLRenderer2d.h>
#include <Xtreme/SDL/SDLSound.h>



int main( int argc, char* argv[] )
{
  GR::IEnvironment&     environment = GR::Service::Environment::Instance();


  GR::Gamebase::Framework* pApp = (GR::Gamebase::Framework*)environment.Service( "Application" );

  if ( pApp == NULL )
  {
    return 1;
  }

  pApp->Configure( pApp->m_EnvironmentConfig );

  if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
  {
    dh::Log( "SDL_Init failed (%s)", SDL_GetError() );
    return 1;
  }

  SDLSound* pSound = new SDLSound();

  environment.SetService( "Renderer", new SDLRenderer2d() );
  environment.SetService( "Input", new SDLInput() );
  environment.SetService( "Sound", (XSoundBase*)pSound );
  environment.SetService( "Music", (XMusic*)pSound );

  pApp->Run();

  delete pSound;

  return 0;
}