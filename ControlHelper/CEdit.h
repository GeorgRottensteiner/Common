#ifndef INCLUDE_CEDIT_H
#define INCLUDE_CEDIT_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CEdit : public CWnd
  {

    // Constructors
    public:


    CEdit();
    virtual ~CEdit();

    BOOL Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID );
    BOOL CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent = NULL, UINT nID = 0 );

    void AddString( const GR::String& strText );
    void SetLimitText( UINT nMax );

    void SetSel( int nStartChar, int nEndChar, BOOL bNoScroll = 0 );

    BOOL              SetReadOnly( BOOL bReadOnly = TRUE );

    int               CharFromPos( POINT& pt ) const;


    virtual LRESULT   WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

  };

}

#endif //__INCLUDE_CEDIT_H__



