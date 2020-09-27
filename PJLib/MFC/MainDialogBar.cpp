
//////////////////////////////////////////////////////////////////////
//
// MainDialogBar.h: interface for the CMainDialogBar class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_MAINDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_)

#define AFX_MAINDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "InitDialogBar.h"

// CMainDialogBar window

class CMainDialogBar : public CInitDialogBar
{
	DECLARE_DYNAMIC(CMainDialogBar)

	// Construction
	public:
	CMainDialogBar();
	virtual ~CMainDialogBar();

	// Attributes
	public:
	protected:

	// Control Objects
	CBitmapButton m_OKButton;

	// Control Variables
	BOOL m_CheckBox;

	// Operations
	public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainDialogBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

	protected:
	virtual BOOL OnInitDialogBar();

	// Implementation
	public:

	// Generated message map functions
	protected:
	//{{AFX_MSG(CMainDialogBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////
/

#endif // !defined(AFX_MAINDIALOGBAR_H__46B4D2B3_C982_11D1_8902_0060979C2EFD__INCLUDED_)

////////////////////////////////////////////////////////////////////////////
//
// MainDialogBar.cpp: implementation of the CMainDialogBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "spectra.h"
#include "MainDialogBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMainDialogBar, CInitDialogBar)

BEGIN_MESSAGE_MAP(CMainDialogBar, CInitDialogBar)
//{{AFX_MSG_MAP(CMainDialogBar)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMainDialogBar::CMainDialogBar()
{
	//Set Initial conditions for controls
	m_CheckBox = 1;
}


CMainDialogBar::~CMainDialogBar()
{

}


BOOL CMainDialogBar::OnInitDialogBar()
{
	// Support for DDX mechanism
	// If you do not want DDX then
	// do not call base class
	CInitDialogBar::OnInitDialogBar();

	// Update any controls NOT supported by DDX
	// CBitmapButton is one
	m_OKButtton.AutoLoad(IDOK, this);

	return TRUE;
}

void CMainDialogBar::DoDataExchange(CDataExchange* pDX)
{
	ASSERT(pDX);

	CInitDialogBar::DoDataExchange(pDX);

	// DDX_??? functions to associate control with
	// data or control object
	// Call UpdateData(TRUE) to get data at any time
	// Call UpdateData(FALSE) to set data at any time

	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Check(pDX, IDC_CHECK1, m_CheckBox);
	//}}AFX_DATA_MAP
}

