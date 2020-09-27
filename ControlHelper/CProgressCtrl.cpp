#include <windows.h>
#include <commctrl.h>

#include "CProgressCtrl.h"



namespace WindowsWrapper
{

  CProgressCtrl::CProgressCtrl()
  {
  }



  CProgressCtrl::~CProgressCtrl()
  {
  }



  void CProgressCtrl::SetRange( short nLower, short nUpper )
  {

    SendMessage( PBM_SETRANGE, 0, MAKELPARAM( nLower, nUpper ) );

  }



  int CProgressCtrl::SetPos( int nPos )
  {

    return (int)SendMessage( PBM_SETPOS, nPos, 0L );

  }




  BOOL CProgressCtrl::Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID )
  {
    return CWnd::Create( PROGRESS_CLASSA, NULL, dwStyle, rect, hwndParent, nID );
  }



  int CProgressCtrl::SetStep( int nStep )
  {
    return ( int )::SendMessageW( m_hWnd, PBM_SETSTEP, nStep, 0L );
  }

}