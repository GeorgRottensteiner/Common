#ifndef INCLUDE_XWINDOWUNIVERSALAPP_H
#define INCLUDE_XWINDOWUNIVERSALAPP_H



#include <WinSys/SubclassManager.h>

#include <Lang/Service.h>

#include <agile.h>

#include "XWindow.h"

#include <windows.h>


namespace Xtreme
{
  ref class ManagedUniversalAppWindow : public Windows::ApplicationModel::Core::IFrameworkView
  {
    public:

      // IFrameworkView Methods.
      virtual void Initialize( Windows::ApplicationModel::Core::CoreApplicationView^ applicationView );
      virtual void SetWindow( Windows::UI::Core::CoreWindow^ window );
      virtual void Load( Platform::String^ entryPoint );
      virtual void Run();
      virtual void Uninitialize();
  };



  class UniversalAppWindow : public GR::Service::ServiceImpl<IAppWindow>
  {
    public:

      HWND                            Hwnd;

      HMENU                           HMenu;

      HINSTANCE                       HInstance;

      HCURSOR                         HCursor;

      Platform::Agile<Windows::UI::Core::CoreWindow>  CoreWindow;


      UniversalAppWindow() :
        Hwnd( NULL ),
        HMenu( NULL ),
        HInstance( NULL ),
        HCursor( NULL ),
        CoreWindow( nullptr )
      {
        //GR::Service::Environment::Instance().SetService( "SubclassManager", &SubclassManager );
        GR::Service::Environment::Instance().SetService( "Window", this );
      }

      virtual ~UniversalAppWindow()
      {
        GR::Service::Environment::Instance().RemoveService( "Window" );
      }



      virtual void Invalidate()
      {
      }



      virtual GR::up Handle()
      {
        return (GR::up)Hwnd;
      }

      virtual bool Create( const EnvironmentConfig& Config )
      {
        return true;
      }

      virtual void Close()
      {
      }


    private:

      //ManagedUniversalAppWindow^   ManagedWindow;
  };

}

#endif //INCLUDE_XWINDOWUNIVERSALAPP_H



