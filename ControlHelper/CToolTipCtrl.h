#ifndef INCLUDE_CTOOLTIPCTRL_H
#define INCLUDE_CTOOLTIPCTRL_H



#include "CWnd.h"

#include "commctrl.h"



namespace WindowsWrapper
{

  class CToolTipCtrl : public CWnd
  {

    // Constructors
    public:


    CToolTipCtrl();
    virtual ~CToolTipCtrl();

    virtual BOOL      Create( HWND hwndParent, DWORD dwStyle = 0 );
    virtual BOOL      CreateEx( HWND hwndParent, DWORD dwStyle = 0, DWORD dwStyleEx = 0 );

    void              Activate( BOOL bActivate );
    BOOL              AddTool( HWND hWnd, LPCSTR lpszText = LPSTR_TEXTCALLBACKA, LPCRECT lpRectTool = NULL, UINT_PTR nIDTool = 0 );
    void              RelayEvent( MSG* lpMsg );
    int               GetToolCount() const;
    void              UpdateTipText( LPCSTR lpszText, HWND hWnd, UINT_PTR nIDTool = 0 );

    void              SetMaxTipWidth( int iWidth );
    void              SetDelayTime( DWORD dwDuration, int iTime );
    void              SetDelayTime( UINT nDelay );

    void              Update();

    void              FillInToolInfo( TOOLINFO& ti, HWND hWnd, UINT_PTR nIDTool ) const;

  };

}

#endif // INCLUDE_CTOOLTIPCTRL_H



