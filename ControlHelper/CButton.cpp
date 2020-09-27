#include <windows.h>

#include "CButton.h"



namespace WindowsWrapper
{

  CButton::CButton() :
    m_dwItemData( 0 )
  {
  }



  CButton::~CButton()
  {
  }



  void CButton::SetCheck( int nCheck )
  {
    ::SendMessageW( m_hWnd, BM_SETCHECK, nCheck, 0 );
  }



  int CButton::GetCheck() const
  {
    return ( int )::SendMessageW( m_hWnd, BM_GETCHECK, 0, 0 );
  }



  BOOL CButton::Create( LPCSTR lpszCaption, DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID )
  {
    return CWnd::Create( "BUTTON", lpszCaption, dwStyle, rect, hwndParent, nID );
  }



  HBITMAP CButton::SetBitmap( HBITMAP hbmImage )
  {
    return (HBITMAP)SendMessage( BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmImage );
  }



  void CButton::SetItemData( DWORD dwItemData )
  {
    m_dwItemData = dwItemData;
  }



  DWORD CButton::GetItemData() const
  {
    return m_dwItemData;
  }



  UINT CButton::GetButtonStyle() const
  {
    return (UINT)GetWindowLongPtr( GWL_STYLE ) & 0xff;
  }



  HICON CButton::SetIcon( HICON hIcon )
  {
    return ( HICON )::SendMessageW( m_hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon );
  }

}