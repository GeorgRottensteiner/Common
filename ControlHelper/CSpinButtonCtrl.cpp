#include <windows.h>
#include <commctrl.h>

#include <ControlHelper\CSpinButtonCtrl.h>



namespace WindowsWrapper
{

  CSpinButtonCtrl::CSpinButtonCtrl()
  {
  }



  CSpinButtonCtrl::~CSpinButtonCtrl()
  {
  }



  void CSpinButtonCtrl::SetRange( int nLower, int nUpper )
  {
    SendMessage( UDM_SETRANGE, 0, MAKELONG( (short)nUpper, (short)nLower ) );
  }

}