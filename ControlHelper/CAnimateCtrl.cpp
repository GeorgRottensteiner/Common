#include <windows.h>
#include <commctrl.h>

#include "CAnimateCtrl.h"



namespace WindowsWrapper
{

  CAnimateCtrl::CAnimateCtrl()
  {
  }



  CAnimateCtrl::~CAnimateCtrl()
  {
  }



  BOOL CAnimateCtrl::Open( DWORD dwAVIResourceID )
  {
    return ( BOOL )::SendMessageW( m_hWnd, ACM_OPEN, 0, dwAVIResourceID );
  }



  BOOL CAnimateCtrl::Play( UINT nFrom, UINT nTo, UINT nRep )
  {
    return ( BOOL )::SendMessageW( m_hWnd, ACM_PLAY, nRep, MAKELPARAM( nFrom, nTo ) );
  }



  BOOL CAnimateCtrl::Stop()
  {
    return ( BOOL )::SendMessageW( m_hWnd, ACM_STOP, 0, 0 );
  }

}