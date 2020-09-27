#ifndef INCLUDE_CANIMATECTRL_H
#define INCLUDE_CANIMATECTRL_H



#include "CWnd.h"



namespace WindowsWrapper
{
  class CAnimateCtrl : public CWnd
  {

    // Constructors
    private:


    public:


    CAnimateCtrl();
    virtual ~CAnimateCtrl();


    BOOL          Open( DWORD dwAVIResourceID );

    BOOL          Play( UINT nFrom = 0, UINT nTo = -1, UINT nRep = -1 );

    BOOL          Stop();


  };

}

#endif // INCLUDE_CANIMATECTRL_H



