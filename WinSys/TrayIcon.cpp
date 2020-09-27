#include "TrayIcon.h"

#include <String/Convert.h>



TrayIcon::TrayIcon( DWORD dwIconResourceID, const GR::String strDescription,
                     HWND hwndToNotify, DWORD dwNotifyMessage ) :
  m_dwResourceID( dwIconResourceID ),
  m_strDescriptionText( strDescription ),
  m_hwndNotify( hwndToNotify ),
  m_dwNotifyMessage( dwNotifyMessage ),
  m_hInstance( GetModuleHandle( NULL ) )
{

  m_dwWMTaskbarCreated = ::RegisterWindowMessageW( L"TaskbarCreated" );

  WNDCLASSW   wc;

  wc.style          = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc    = (WNDPROC)TrayIconProc;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = m_hInstance;
  wc.hIcon          = NULL;
  wc.hCursor        = NULL;
  wc.hbrBackground  = NULL;
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = L"GRTrayIconClass";
  RegisterClassW( &wc );

  m_hwndMySecretWindow = CreateWindowW( L"GRTrayIconClass",
                               L"",
                               WS_OVERLAPPED,
                               0,
                               0,
                               100,
                               100,
                               NULL,
                               NULL,
                               m_hInstance,
                               NULL );
  SetWindowLongPtr( m_hwndMySecretWindow, GWLP_USERDATA, (LONG)(LONG_PTR)this );
}



TrayIcon::~TrayIcon()
{
  DestroyWindow( m_hwndMySecretWindow );

  UnregisterClassW( L"GRTrayIconClass", m_hInstance );

  m_hwndMySecretWindow = NULL;
  ChangeIcon();
}



void TrayIcon::ChangeNotifier( HWND hwndToNotify, GR::u32 dwNotifyMessage )
{
  m_hwndNotify      = hwndToNotify;
  m_dwNotifyMessage = dwNotifyMessage;

  ChangeIcon( m_dwResourceID, m_strDescriptionText.c_str() );
}



void TrayIcon::ChangeIcon( DWORD dwResourceID, const char* szDescription )
{

  memset( &m_nidIcon, 0, sizeof( NOTIFYICONDATA ) );
  m_nidIcon.cbSize  = sizeof( NOTIFYICONDATA );
  m_nidIcon.hWnd    = m_hwndNotify;
  m_nidIcon.uID     = 17;
  m_nidIcon.uFlags  = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  if ( dwResourceID )
  {
    m_nidIcon.hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE( dwResourceID ) );
  }
  m_nidIcon.uCallbackMessage = m_dwNotifyMessage;
  wsprintfW( m_nidIcon.szTip, GR::Convert::ToUTF16( szDescription ).c_str() );

  m_dwResourceID        = dwResourceID;
  m_strDescriptionText  = szDescription;

  if ( dwResourceID == 0 )
  {
    // Icon entfernen
    Shell_NotifyIcon( NIM_DELETE, &m_nidIcon );
    if ( m_nidIcon.hIcon )
    {
      DestroyIcon( m_nidIcon.hIcon );
      m_nidIcon.hIcon = NULL;
    }
    return;
  }

  if ( !Shell_NotifyIcon( NIM_MODIFY, &m_nidIcon ) )
  {
    // Explorer crashed? - neu Adden
    Shell_NotifyIcon( NIM_ADD, &m_nidIcon );
  }
  DestroyIcon( m_nidIcon.hIcon );

}



LRESULT CALLBACK TrayIcon::TrayIconProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  TrayIcon*    pTrayIcon = (TrayIcon*)(LONG_PTR)GetWindowLongPtr( hWnd, GWLP_USERDATA );
  if ( pTrayIcon )
  {
    return pTrayIcon->WindowProc( hWnd, uMsg, wParam, lParam );
  }

  return DefWindowProc( hWnd, uMsg, wParam, lParam );

}



LRESULT CALLBACK TrayIcon::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  if ( uMsg == m_dwWMTaskbarCreated )
  {
    // der Explorer ist neu gestartet worden, Icon wieder anlegen
    ChangeIcon( m_dwResourceID, m_strDescriptionText.c_str() );
  }
  return DefWindowProc( hWnd, uMsg, wParam, lParam );

}



