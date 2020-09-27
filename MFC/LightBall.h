#if !defined(AFX_LIGHTBALL_H__3301BA0F_416C_4FC3_93EB_DEB401F4D1DD__INCLUDED_)
#define AFX_LIGHTBALL_H__3301BA0F_416C_4FC3_93EB_DEB401F4D1DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightBall.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightBall window

class CLightBall : public CWnd
{
// Construction
public:
	CLightBall();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightBall)
	//}}AFX_VIRTUAL

// Implementation
public:

	  virtual ~CLightBall();


    #define LIGHTBALL_CLASS_NAME _T( "LightBall" )
    #define LIGHTBALL_MESSAGE _T( "LightBallValueChanged" )

    static BOOL hasclass;
    static BOOL RegisterMe();
    static DWORD dwLBM;

    GR::tVector   m_vectLightDir;


    void SetLightDirection( const GR::tVector& vectLightDir );

    inline GR::tVector GetLightDir() { return m_vectLightDir; }


	// Generated message map functions
protected:
	//{{AFX_MSG(CLightBall)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void ValuesChanged();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTBALL_H__3301BA0F_416C_4FC3_93EB_DEB401F4D1DD__INCLUDED_)
