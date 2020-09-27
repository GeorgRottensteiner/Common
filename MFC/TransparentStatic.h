#pragma once

// CTransparentStatic

class CTransparentStatic : public CStatic
{
	DECLARE_DYNAMIC(CTransparentStatic)

public:
	CTransparentStatic();
	virtual ~CTransparentStatic();

protected:
	DECLARE_MESSAGE_MAP()
public:

  CString                 m_strText;

  CWnd*                   m_pWndBackground;

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

  void PreSubclassWindow();
  virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);

  virtual void            DoPaint( CDC* pDC );

  void                    SetWindowText( LPCTSTR lpszString );
  void                    SetAlternateBackgroundWnd( CWnd* pWndBackground );
};


