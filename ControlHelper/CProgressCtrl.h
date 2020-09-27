#ifndef CPROGRESSCTRL_H
#define CPROGRESSCTRL_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CProgressCtrl : public CWnd
  {

    // Constructors
    private:


    public:


    CProgressCtrl();
    virtual ~CProgressCtrl();

    BOOL                Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID );

    void                SetRange( short nLower, short nUpper );
    int                 SetPos( int nPos );
    int                 SetStep( int nStep );

  };

}

#endif // CPROGRESSCTRL_H



