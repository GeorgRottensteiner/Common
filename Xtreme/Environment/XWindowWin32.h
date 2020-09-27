#ifndef INCLUDE_XWINDOWWIN32_H
#define INCLUDE_XWINDOWWIN32_H



#include <WinSys/SubclassManager.h>

#include <Lang/Service.h>

#include "XWindow.h"

#include <windows.h>



namespace GR
{
  namespace Gamebase
  {
    class Framework;
  }
}



namespace Xtreme
{
  class Win32Window : public GR::Service::ServiceImpl<IAppWindow>
  {
    public:

      HWND                  Hwnd;

      HMENU                 HMenu;

      HINSTANCE             HInstance;

      HCURSOR               HCursor;

      SubclassManager       SubclassManager;

      GR::u32               m_StoredStyles;

      GR::Gamebase::Framework*    m_pFrameWork;



      Win32Window();
      virtual ~Win32Window();

      virtual GR::up Handle();
      virtual void Invalidate();

      virtual bool Create( const EnvironmentConfig& Config );
      virtual void Close();


      static LRESULT WINAPI XFrameAppProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
      virtual BOOL          WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
      BOOL                  SubclassWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

  };
}

#endif //INCLUDE_XWINDOWWIN32_H



