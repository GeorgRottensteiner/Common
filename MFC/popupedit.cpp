// popupedit.cpp : implementation file
//

#include "stdafx.h"
#include <MFC\PopupEdit.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit

DWORD CPopupEdit::dwPEM = RegisterWindowMessage( POPUPEDIT_MESSAGE );



CPopupEdit::CPopupEdit()
{
}

CPopupEdit::~CPopupEdit()
{
}


BEGIN_MESSAGE_MAP(CPopupEdit, CEdit)
	//{{AFX_MSG_MAP(CPopupEdit)
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit message handlers

void CPopupEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

	if ( nChar == VK_ESCAPE )
  {
    CWnd *pWnd = GetParent();
    if ( pWnd )
    {
      pWnd->SendMessage( dwPEM, 0, 0 );
      return;
    }
  }
	else if ( nChar == VK_RETURN )
  {
    CWnd *pWnd = GetParent();
    if ( pWnd )
    {
      pWnd->SendMessage( dwPEM, 1, 0 );
      return;
    }
  }
	
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}
