#if !defined(AFX_ICONBUTTON_H__59FCC7FC_F75A_4659_8624_D535BF384E65__INCLUDED_)
#define AFX_ICONBUTTON_H__59FCC7FC_F75A_4659_8624_D535BF384E65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// iconbutton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIconButton window

class CIconButton : public CWnd
{

  protected:

    HICON       m_hIcon;

    bool        m_bPushed;

  // Construction
  public:

    #define ICONBUTTON_CLASS_NAME _T( "IconButton" )

    static BOOL hasclass;
    static BOOL RegisterMe();


	  CIconButton();


    void SetIcon( HICON icon );

  // Operations
  public:

  // Overrides
	  // ClassWizard generated virtual function overrides
	  //{{AFX_VIRTUAL(CIconButton)
	  //}}AFX_VIRTUAL

  // Implementation
  public:
	  virtual ~CIconButton();

	  // Generated message map functions
  protected:
	  //{{AFX_MSG(CIconButton)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG

	  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONBUTTON_H__59FCC7FC_F75A_4659_8624_D535BF384E65__INCLUDED_)
