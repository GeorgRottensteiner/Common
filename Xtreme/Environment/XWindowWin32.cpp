#include "XWindowWin32.h"
#include <GR/Gamebase/Framework.h>

#include <debug/debugclient.h>



namespace Xtreme
{
  Win32Window::Win32Window() :
    Hwnd( NULL ),
    HMenu( NULL ),
    HInstance( GetModuleHandle( NULL ) ),
    HCursor( LoadCursor( NULL, IDC_ARROW ) ),
    m_StoredStyles( 0 ),
    m_pFrameWork( NULL )
  {
    GR::Service::Environment::Instance().SetService( "SubclassManager", &SubclassManager );
    GR::Service::Environment::Instance().SetService( "Window", this );
  }



  Win32Window::~Win32Window()
  {
    GR::Service::Environment::Instance().RemoveService( "Window" );
  }



  GR::up Win32Window::Handle()
  {
    return (GR::up)Hwnd;
  }



  void Win32Window::Invalidate()
  {
    ::InvalidateRect( Hwnd, NULL, FALSE );
  }



  bool Win32Window::Create( const EnvironmentConfig& Config )
  {
    GR::u32   windowStates = WS_VISIBLE | WS_OVERLAPPED | WS_SYSMENU;

    if ( !Config.FixedSize )
    {
      windowStates |= WS_DLGFRAME | WS_SIZEBOX | WS_MAXIMIZEBOX;
    }

    windowStates |= WS_CAPTION;

    m_StoredStyles = windowStates;

    RECT      rcClientSize;

    SetRect( &rcClientSize, 0, 0, Config.StartUpWidth, Config.StartUpHeight );
    AdjustWindowRectEx( &rcClientSize, windowStates, FALSE, WS_EX_WINDOWEDGE );

    WNDCLASS    WndClass;
    HINSTANCE   hInstance = (HINSTANCE)GetModuleHandle( NULL );

    ZeroMemory( &WndClass, sizeof( WndClass ) );
    WndClass.style          = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc    = (WNDPROC)XFrameAppProc;
    WndClass.cbClsExtra     = 0;
    WndClass.cbWndExtra     = 0;
    WndClass.hInstance      = hInstance;
    WndClass.hIcon          = LoadIcon( hInstance, MAKEINTRESOURCE( Config.IconResourceID ) );
    WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground  = NULL;
    WndClass.lpszMenuName   = NULL;
    WndClass.lpszClassName  = "XFrameAppClass";
    if ( !RegisterClass( &WndClass ) )
    {
      dh::Log( "RegisterClass failed" );
      return false;
    }

    //m_RenderFrame.m_hwndMain
    Hwnd = CreateWindow( "XFrameAppClass", Config.Caption.c_str(), windowStates, 0, 0,
                         rcClientSize.right - rcClientSize.left,
                         rcClientSize.bottom - rcClientSize.top,
                         NULL,
                         NULL,
                         hInstance,
                         this );
    if ( Hwnd == NULL )
    {
      dh::Log( "CreateWindow failed" );
      return false;
    }
    SubclassManager.AddHandler( "XFrameApp2d", fastdelegate::MakeDelegate( this, &Win32Window::SubclassWindowProc ) );
    return true;
  }



  void Win32Window::Close()
  {
  }


  Win32Window* pWindow = NULL;


  LRESULT CALLBACK Win32Window::XFrameAppProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
  {
    if ( message == WM_NCCREATE )
    {
      CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;

      pWindow = (Win32Window*)pCS->lpCreateParams;
    }

    if ( pWindow )
    {
      if ( pWindow->SubclassManager.Empty() )
      {
        return pWindow->WindowProc( hWnd, message, wParam, lParam );
      }
      return pWindow->SubclassManager.CallChain( hWnd, message, wParam, lParam );
    }
    return ::DefWindowProc( hWnd, message, wParam, lParam );
  }



  BOOL Win32Window::SubclassWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
  {
    return (BOOL)WindowProc( hWnd, message, wParam, lParam );
  }



  BOOL Win32Window::WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
  {
    switch ( message )
    {
      case WM_GETICON:
        return NULL;
      case WM_KEYDOWN:
        m_pFrameWork->OnKeyDown( (char)wParam );
        break;
      case WM_CHAR:
        m_pFrameWork->OnCharEntered( wParam );
        break;
      case WM_CREATE:
        {
          RECT    rc;

          GetWindowRect( hWnd, &rc );

          SetWindowPos( hWnd, NULL,
                        ( GetSystemMetrics( SM_CXSCREEN ) - ( rc.right - rc.left ) ) / 2,
                        ( GetSystemMetrics( SM_CYSCREEN ) - ( rc.bottom - rc.top ) ) / 2,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER );
        }
        break;
      case WM_CLOSE:
        m_pFrameWork->ShutDown();
        return 0;
      case WM_DESTROY:
        //ExitInstance();
        //EventProducer<GR::Gamebase::tXFrameEvent>::RemoveListener( this );
        PostQuitMessage( 0 );
        return 0;
      case WM_PAINT:
        {
          m_pFrameWork->OnDisplayFrame();
          ValidateRect( hWnd, NULL );
        }
        return TRUE;
      case WM_ERASEBKGND:
        // TODO ?
        //if ( m_pFrameWork->Renderer() )
        {
          return TRUE;
        }
        break;
      case WM_SIZE:
        m_pFrameWork->OnSizeChanged();
        break;
      case WM_EXITSIZEMOVE:
        m_pFrameWork->OnSizeChanged();
        break;
      case WM_SYSCHAR:
        if ( wParam == VK_RETURN )
        {
          GLOBAL_QUEUE.PostEvent( "App.ToggleWindowMode" );
        }
        break;
      case WM_SETCURSOR:
        if ( ( LOWORD( lParam ) == HTCLIENT )
        &&   ( !m_pFrameWork->CursorEnabled() ) )
        {
          ::SetCursor( NULL );
          return TRUE;
        }
        if ( LOWORD( lParam ) == HTCLIENT )
        {
          ::SetCursor( HCursor );
          return TRUE;
        }
        break;
      case WM_ACTIVATE:
        m_pFrameWork->SetActive( ( LOWORD( wParam ) != WA_INACTIVE ) );
        break;
    }
    return ( BOOL )::DefWindowProc( hWnd, message, wParam, lParam );
  }

}




