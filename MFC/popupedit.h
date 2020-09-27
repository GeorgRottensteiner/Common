#if !defined(AFX_POPUPEDIT_H__31C7D1DB_4F12_43D7_8480_BAE0CC3913D0__INCLUDED_)
#define AFX_POPUPEDIT_H__31C7D1DB_4F12_43D7_8480_BAE0CC3913D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// popupedit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPopupEdit window

#define POPUPEDIT_MESSAGE _T( "PopupEditEndLabel" )


class CPopupEdit : public CEdit
{
// Construction
public:
	CPopupEdit();

// Attributes
public:

// Operations
public:


  static DWORD     dwPEM;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopupEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPopupEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPopupEdit)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPUPEDIT_H__31C7D1DB_4F12_43D7_8480_BAE0CC3913D0__INCLUDED_)
