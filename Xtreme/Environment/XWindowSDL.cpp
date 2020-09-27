#include "XWindowSDL.h"

#include <SDL.h>

#include <debug/debugclient.h>



namespace Xtreme
{
  SDLWindow::SDLWindow() :
    m_pMainWindow( NULL ),
    m_pMainSurface( NULL )
  {
    GR::Service::Environment::Instance().SetService( "Window", this );
  }



  SDLWindow::~SDLWindow()
  {
    GR::Service::Environment::Instance().RemoveService( "Window" );
  }



  GR::up SDLWindow::Handle()
  {
    return ( GR::up )m_pMainWindow;
  }



  void SDLWindow::Invalidate()
  {
    //::InvalidateRect( Hwnd, NULL, FALSE );
  }



  bool SDLWindow::Create( const EnvironmentConfig& Config )
  {
    m_pMainWindow = SDL_CreateWindow( Config.Caption.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      Config.StartUpWidth, Config.StartUpHeight,
                                      SDL_WINDOW_SHOWN );
    if ( m_pMainWindow == nullptr )
    {
      dh::Log( "SDL_CreateWindow failed (%s)", SDL_GetError() );
      return false;
    }
    return true;
  }



  void SDLWindow::Close()
  {
    if ( m_pMainWindow != NULL )
    {
      SDL_DestroyWindow( m_pMainWindow );
      m_pMainWindow = NULL;
    }
  }



}



