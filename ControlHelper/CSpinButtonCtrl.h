#ifndef INCLUDE_CSPINBUTTONCONTROL_H
#define INCLUDE_CSPINBUTTONCONTROL_H



#include <ControlHelper\CWnd.h>



namespace WindowsWrapper
{

  class CSpinButtonCtrl : public CWnd
  {

    // Constructors
    public:


    CSpinButtonCtrl();
    virtual ~CSpinButtonCtrl();


    void SetRange( int nLower, int nUpper );


  };

}

#endif //__INCLUDE_CSPINBUTTONCONTROL_H__



