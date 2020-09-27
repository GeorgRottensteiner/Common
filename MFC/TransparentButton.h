#pragma once


// CTransparentButton

class CTransparentButton : public CButton
{
	DECLARE_DYNAMIC(CTransparentButton)

public:
	CTransparentButton();
	virtual ~CTransparentButton();

  void            SetAlternateBackgroundWnd( CWnd* pWndBackground );

protected:

  CWnd*           m_pWndBackground;

    HBITMAP       m_hbmCheckBox;
	DECLARE_MESSAGE_MAP()
public:
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
protected:
  virtual void PreSubclassWindow();
};


