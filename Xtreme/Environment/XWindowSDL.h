#ifndef INCLUDE_XWINDOW_SDL_H
#define INCLUDE_XWINDOW_SDL_H



#include <Lang/Service.h>

#include "XWindow.h"

#include <SDL.h>



namespace GR
{
  namespace Gamebase
  {
    class Framework;
  }
}



namespace Xtreme
{
  class SDLWindow : public GR::Service::ServiceImpl<IAppWindow>
  {
    public:

      SDL_Window*           m_pMainWindow;
      SDL_Surface*          m_pMainSurface;

      GR::Gamebase::Framework*    m_pFrameWork;



      SDLWindow();
      virtual ~SDLWindow();

      virtual GR::up Handle();
      virtual void Invalidate();

      virtual bool Create( const EnvironmentConfig& Config );
      virtual void Close();
  };
}

#endif //INCLUDE_XWINDOW_SDL_H



