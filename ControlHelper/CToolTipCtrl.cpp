#include <windows.h>

#include "CToolTipCtrl.h"



namespace WindowsWrapper
{

  CToolTipCtrl::CToolTipCtrl()
  {
    InitCommonControls();
  }



  CToolTipCtrl::~CToolTipCtrl()
  {
  }



  BOOL CToolTipCtrl::Create( HWND hwndParent, DWORD dwStyle )
  {
    RECT    rcDummy;

    memset( &rcDummy, 0, sizeof( rcDummy ) );

    if ( !CWnd::Create( "tooltips_class32", NULL, WS_POPUP | dwStyle, rcDummy, hwndParent, 0 ) )
    {
      return FALSE;
    }

    SetOwner( hwndParent );

    return TRUE;
  }



  BOOL CToolTipCtrl::CreateEx( HWND hwndParent, DWORD dwStyle, DWORD dwExStyle )
  {
    RECT    rcDummy;

    memset( &rcDummy, 0, sizeof( rcDummy ) );

    return CWnd::CreateEx( dwExStyle, "tooltips_class32", NULL, WS_POPUP | dwStyle, rcDummy, hwndParent, 0 );
  }



  BOOL CToolTipCtrl::AddTool( HWND hWnd, LPCSTR lpszText, LPCRECT lpRectTool, UINT_PTR nIDTool )
  {
    TOOLINFOW    tInfo;

    GR::WString  utf16Text = GR::Convert::ToUTF16( lpszText );

    memset( &tInfo, 0, sizeof( tInfo ) );
    tInfo.cbSize = sizeof( tInfo );
    tInfo.uFlags = TTF_SUBCLASS;

    tInfo.hwnd = ::GetParent( hWnd );
    tInfo.hinst = NULL;
    tInfo.lpszText = (LPWSTR)utf16Text.c_str();
    if ( nIDTool == 0 )
    {
      tInfo.uFlags |= TTF_IDISHWND;
      tInfo.uId = (UINT_PTR)hWnd;

      ::GetClientRect( hWnd, &tInfo.rect );
    }
    else
    {
      tInfo.uId = nIDTool;
      if ( lpRectTool != NULL )
      {
        memcpy( &tInfo.rect, lpRectTool, sizeof( RECT ) );
      }
      else
      {
        //::GetClientRect( hWnd, &tInfo.rect );
      }
    }
    return (BOOL)SendMessage( TTM_ADDTOOL, 0, (LPARAM)&tInfo );
  }



  void CToolTipCtrl::Activate( BOOL bActivate )
  {
    SendMessage( TTM_ACTIVATE, (WPARAM)bActivate );
  }



  void CToolTipCtrl::RelayEvent( MSG* pMsg )
  {
    SendMessage( TTM_RELAYEVENT, 0, (LPARAM)pMsg );
  }



  int CToolTipCtrl::GetToolCount() const
  {
    return ( int )::SendMessageW( m_hWnd, TTM_GETTOOLCOUNT, 0, 0L );
  }




  void CToolTipCtrl::FillInToolInfo( TOOLINFO& ti, HWND hWnd, UINT_PTR nIDTool ) const
  {

    memset( &ti, 0, sizeof( TOOLINFO ) );
    ti.cbSize = sizeof( TOOLINFO );
    if ( nIDTool == 0 )
    {
      ti.hwnd = ::GetParent( hWnd );
      ti.uFlags = TTF_IDISHWND;
      ti.uId = (UINT_PTR)hWnd;
    }
    else
    {
      ti.hwnd = hWnd;
      ti.uFlags = 0;
      ti.uId = nIDTool;
    }
  }



  void CToolTipCtrl::UpdateTipText( LPCSTR lpszText, HWND hWnd, UINT_PTR nIDTool )
  {
    TOOLINFOW        ti;

    GR::WString     utf16Text = GR::Convert::ToUTF16( lpszText );

    FillInToolInfo( ti, hWnd, nIDTool );
    ti.lpszText = (LPWSTR)utf16Text.c_str();

    ::SendMessageW( m_hWnd, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti );
  }



  void CToolTipCtrl::Update()
  {
    ::SendMessageW( m_hWnd, TTM_UPDATE, 0, 0 );
  }



  void CToolTipCtrl::SetMaxTipWidth( int iWidth )
  {
    ::SendMessageW( m_hWnd, TTM_SETMAXTIPWIDTH, 0, iWidth );
  }



  void CToolTipCtrl::SetDelayTime( DWORD dwDuration, int iTime )
  {
    ::SendMessageW( m_hWnd, TTM_SETDELAYTIME, dwDuration, MAKELPARAM( iTime, 0 ) );
  }



  void CToolTipCtrl::SetDelayTime( UINT nDelay )
  {
    ::SendMessageW( m_hWnd, TTM_SETDELAYTIME, 0, nDelay );
  }


}