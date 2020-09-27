#include <windows.h>
#include <commctrl.h>

#include <ControlHelper\CSliderCtrl.h>



namespace WindowsWrapper
{

  CSliderCtrl::CSliderCtrl()
  {
  }



  CSliderCtrl::~CSliderCtrl()
  {
  }



  void CSliderCtrl::SetRange( int nMin, int nMax, BOOL bRedraw )
  {
    SendMessage( TBM_SETRANGE, (WPARAM)bRedraw, (LPARAM)MAKELONG( nMin, nMax ) );
  }



  void CSliderCtrl::SetPos( int nPos )
  {
    SendMessage( TBM_SETPOS, (WPARAM)(BOOL)nPos, (LPARAM)nPos );
  }


}