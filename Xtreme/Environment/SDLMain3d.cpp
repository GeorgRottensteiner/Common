#include <SDL.h>

#include <Lang/Service.h>

#include <GR/Gamebase/Framework.h>

#include <Xtreme/Environment/EnvironmentConfig.h>

#include <Xtreme/SDL/SDLInput.h>
#include <Xtreme/OpenGL/OpenGLRenderClass.h>
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
  SDLInput* pInput = new SDLInput();

  pInput->SetActive( true );

  environment.SetService( "Renderer", new OpenGLRenderClass() );
  environment.SetService( "Input", pInput );
  environment.SetService( "Sound", (XSoundBase*)pSound );
  environment.SetService( "Music", (XMusic*)pSound );

  pApp->Run();

  delete pSound;

  return 0;
}