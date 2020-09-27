#include <windows.h>

#include "CApp.h"



namespace WindowsWrapper
{

  CApp::CApp() :
    CResizeWndImpl<CWnd>(),
    m_hInstance( NULL ),
    m_ExitCode( 0 )
  {
  }



  CApp::~CApp()
  {
  }



  BOOL CApp::Create( GR::u32 Style, int Width, int Height, const char* Name, UINT IDMenu )
  {
    if ( m_hWnd != NULL )
    {
      return false;
    }

    m_hInstance = GetModuleHandle( NULL );

    WNDCLASSW    WndClass;

    ZeroMemory( &WndClass, sizeof( WndClass ) );
    WndClass.style          = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc    = ::DefWindowProc;
    WndClass.cbClsExtra     = 0;
    WndClass.cbWndExtra     = 0;
    WndClass.hInstance      = m_hInstance;
    WndClass.hIcon          = NULL;//LoadIcon( m_hInstance, MAKEINTRESOURCE( m_dwIconID ) );
    WndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground  = (HBRUSH)GetStockObject( WHITE_BRUSH );
    WndClass.lpszMenuName   = NULL;
    if ( IDMenu )
    {
      WndClass.lpszMenuName = MAKEINTRESOURCEW( IDMenu );
    }
    WndClass.lpszClassName = L"AppClass";
    if ( !RegisterClassW( &WndClass ) )
    {
      return FALSE;
    }

    RECT    rc;

    rc.left   = ( GetSystemMetrics( SM_CXSCREEN ) - Width ) / 2;
    rc.top    = ( GetSystemMetrics( SM_CYSCREEN ) - Height ) / 2;
    rc.right  = rc.left + Width;
    rc.bottom = rc.top + Height;

    return CResizeWndImpl<CWnd>::Create( "AppClass", Name, Style, rc, NULL, 0 );
  }




  bool CApp::InitInstance()
  {
    return true;
  }


  void CApp::ExitInstance()
  {
  }



  void CApp::ShutDown( int ExitCode )
  {
    m_ExitCode = ExitCode;
    DestroyWindow();
  }



  int CApp::Run()
  {
    if ( !InitInstance() )
    {
      return m_ExitCode;
    }

    if ( m_hWnd )
    {
      while ( CResizeWndImpl<CWnd>::PumpMessage() );
    }

    ExitInstance();

    return m_ExitCode;
  }



  LRESULT CApp::WindowProc( UINT Message, WPARAM wParam, LPARAM lParam )
  {
    switch ( Message )
    {
      case WM_CLOSE:
        DestroyWindow();
        return 0;
      case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;
    }
    return CResizeWndImpl<CWnd>::WindowProc( Message, wParam, lParam );
  }



  HICON CApp::LoadIcon( UINT IDResource ) const
  {
    return ::LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDResource ) );
  }

}