#include <Lang/Service.h>

#include <GR/Gamebase/Framework.h>

#include <Xtreme/Environment/EnvironmentConfig.h>



INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
  GR::Gamebase::Framework*   pApp = (GR::Gamebase::Framework*)GR::Service::Environment::Instance().Service( "Application" );

  if ( pApp == NULL )
  {
    return 1;
  }

  pApp->Configure( pApp->m_EnvironmentConfig );

  pApp->Run();

  return 0;
}
