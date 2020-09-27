#ifndef INCLUDE_CSLIDERCTRL_H
#define INCLUDE_CSLIDERCTRL_H



#pragma comment( lib, "comctl32.lib" )



#include <commctrl.h>
#include <ControlHelper\CWnd.h>



namespace WindowsWrapper
{

  class CCommonCtrlDummyForSlider
  {
    public:

    CCommonCtrlDummyForSlider()
    {
      InitCommonControls();
    }
  };

  static CCommonCtrlDummyForSlider    commoncontroldummyforslider;


  class CSliderCtrl : public CWnd
  {

    // Constructors
    public:


    CSliderCtrl();
    virtual ~CSliderCtrl();


    virtual void        SetRange( int nMin, int nMax, BOOL bRedraw = FALSE );
    virtual void        SetPos( int nPos );


  };

}

#endif //__INCLUDE_CSLIDERCTRL_H__



