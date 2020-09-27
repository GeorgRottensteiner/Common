#ifndef INCLUDE_XWINDOW_ANDROID_H
#define INCLUDE_XWINDOW_ANDROID_H



#include <Lang/Service.h>

#include "XWindow.h"



namespace GR
{
  namespace Gamebase
  {
    class Framework;
  }
}


struct ANativeWindow;



namespace Xtreme
{
  class AndroidWindow : public GR::Service::ServiceImpl<IAppWindow>
  {
    public:

      ANativeWindow*              m_pMainWindow;

      GR::Gamebase::Framework*    m_pFrameWork;



      AndroidWindow() :
        m_pMainWindow( NULL )
      {
        GR::Service::Environment::Instance().SetService( "Window", this );
      }



      virtual ~AndroidWindow()
      {
        GR::Service::Environment::Instance().RemoveService( "Window" );
      }



      virtual GR::up Handle()
      {
        return 0;
      }



      virtual void Invalidate()
      {
      }



      virtual bool Create( const EnvironmentConfig& Config )
      {
        return false;
      }



      virtual void Close()
      {
      }
  };
}

#endif //INCLUDE_XWINDOW_ANDROID_H



