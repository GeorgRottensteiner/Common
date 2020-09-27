/*----------------------------------------------------------------------------+
 | Programmname       :                                                       |
 +----------------------------------------------------------------------------+
 | Autor              :                                                       |
 | Datum              :                                                       |
 | Version            :                                                       |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <GR\TrayApp\TrayApp.h>

#include <ControlHelper/CWnd.h>



/*-Globale Variablen----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CTrayApp*      g_pTrayApp = NULL;



/*-MainWndProc----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CALLBACK MainWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  return g_pTrayApp->MainWndProc( hwndDlg, uMsg, wParam, lParam );

}



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CTrayApp::CTrayApp() :
  m_hInstance( NULL ),
  m_strAppPath( "" ),
  m_dwResourceID( 0 )
{

  g_pTrayApp = this;

  m_dwWMTaskbarCreated = ::RegisterWindowMessage( "TaskbarCreated" );

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CTrayApp::~CTrayApp()
{

  g_pTrayApp = NULL;

}



/*-ShutDown-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CTrayApp::ShutDown()
{

  m_WndMain.DestroyWindow();

}



/*-AppPath--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

GR::String CTrayApp::AppPath( const char* szPath )
{

  if ( m_strAppPath.length() == 0 )
  {
    char    szTempPath[MAX_PATH];
    GetModuleFileName( NULL, szTempPath, MAX_PATH );
    while ( szTempPath[strlen( szTempPath ) - 1] != 92 )
    {
      szTempPath[strlen( szTempPath ) - 1] = 0;
    }
    m_strAppPath = szTempPath;
    if ( m_strAppPath.length() >= 10)
    {
      if ( ( toupper( m_strAppPath[m_strAppPath.length() - 8] ) == 'R' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 7] ) == 'E' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 6] ) == 'L' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 5] ) == 'E' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 4] ) == 'A' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 3] ) == 'S' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 2] ) == 'E' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 9] ) == '\\' ) )
      {
        m_strAppPath = m_strAppPath.substr( 0, m_strAppPath.length() - 8 );
      }
    }
    if ( m_strAppPath.length() >= 8 )
    {
      if ( ( toupper( m_strAppPath[m_strAppPath.length() - 6] ) == 'D' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 5] ) == 'E' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 4] ) == 'B' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 3] ) == 'U' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 2] ) == 'G' )
      &&   ( toupper( m_strAppPath[m_strAppPath.length() - 7] ) == '\\' ) )
      {
        m_strAppPath = m_strAppPath.substr( 0, m_strAppPath.length() - 6 );
      }
    }
  }

  GR::String   strResult = m_strAppPath + szPath;

  return strResult;

}



/*-InitInstance---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CTrayApp::InitInstance()
{

  return true;

}



/*-ExitInstance---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CTrayApp::ExitInstance()
{

}



/*-ChangeIcon-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CTrayApp::ChangeIcon( DWORD dwResourceID, const char* szDescription )
{

  m_nidIcon.cbSize  = sizeof( NOTIFYICONDATA );
  m_nidIcon.hWnd    = m_WndMain.GetSafeHwnd();
  m_nidIcon.uID     = 0;
  m_nidIcon.uFlags  = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  m_nidIcon.hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE( dwResourceID ) );
  m_nidIcon.uCallbackMessage = WM_NOTIFY_SIZER;
  wsprintf( m_nidIcon.szTip, szDescription );

  m_dwResourceID        = dwResourceID;
  m_strDescriptionText  = szDescription;

  if ( !Shell_NotifyIcon( NIM_MODIFY, &m_nidIcon ) )
  {
    // Explorer crashed? - neu Adden
    Shell_NotifyIcon( NIM_ADD, &m_nidIcon );
  }
  DestroyIcon( m_nidIcon.hIcon );

}



/*-OnNotify-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CTrayApp::OnNotify( WPARAM wParam, LPARAM lParam )
{
}



/*-MainWndProc----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

LRESULT CALLBACK CTrayApp::MainWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  if ( uMsg == m_dwWMTaskbarCreated )
  {
    // der Explorer ist neu gestartet worden, Icon wieder anlegen
    ChangeIcon( m_dwResourceID, m_strDescriptionText.c_str() );
  }

  switch ( uMsg )
  {
    case WM_NOTIFY_SIZER:
      OnNotify( wParam, lParam );
      break;
    case WM_DESTROY:
      ExitInstance();
      // Icon entfernen
      m_nidIcon.cbSize  = sizeof( NOTIFYICONDATA );
      m_nidIcon.hWnd    = hWnd;
      m_nidIcon.uID     = 0;
      Shell_NotifyIcon( NIM_DELETE, &m_nidIcon );
      PostQuitMessage( 0 );
      break;
  }
  return DefWindowProc( hWnd, uMsg, wParam, lParam );

}



/*-Run------------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CTrayApp::Run( DWORD dwIconResource, const char* szDescription )
{


  //MSG               msg;

  WNDCLASS          wc;


  m_hInstance       = GetModuleHandle( NULL );

  wc.style          = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_DBLCLKS | CS_SAVEBITS;
  wc.lpfnWndProc    = (WNDPROC)::MainWndProc;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = m_hInstance;
  wc.hIcon          = LoadIcon( m_hInstance, MAKEINTRESOURCE( dwIconResource ) );
  wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground  = (HBRUSH)GetStockObject( BLACK_BRUSH );
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = "TRAYAPPCLASS";
  if ( !RegisterClass( &wc ) )
  {
    return 0;
  }

  m_WndMain.CreateEx( 0,
                    "TRAYAPPCLASS",
                           szDescription,
                           WS_POPUP,
                           0, 0,
                           100,
                           100,
                           NULL,
                           NULL,
                           NULL );
  if ( m_WndMain.GetSafeHwnd() == NULL )
  {
    return 0;
  }

  m_nidIcon.cbSize  = sizeof( NOTIFYICONDATA );
  m_nidIcon.hWnd    = m_WndMain.GetSafeHwnd();
  m_nidIcon.uID     = 0;
  m_nidIcon.uFlags  = NIF_TIP | NIF_ICON | NIF_MESSAGE;
  m_nidIcon.uCallbackMessage = WM_NOTIFY_SIZER;
  m_nidIcon.hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE( dwIconResource ) );

  wsprintf( m_nidIcon.szTip, szDescription );
  Shell_NotifyIcon( NIM_ADD, &m_nidIcon );
  DestroyIcon( m_nidIcon.hIcon );

  if ( !InitInstance() )
  {
    return 0;
  }


  while ( 1 )
  {
    if ( WindowsWrapper::CWnd::PumpMessage() == 0 )
    {
      break;
    }
    /*
    if ( !GetMessage( &msg, NULL, 0, 0 ) )
    {
      break;
    }
    if ( IsDialogMessage( ::GetParent( msg.hwnd ), &msg ) )
    {
      continue;
    }
    TranslateMessage( &msg );
    DispatchMessage( &msg );
    */
  }

  return 0;//msg.wParam;

}
