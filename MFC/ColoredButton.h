#pragma once


// CColoredButton

class CColoredButton : public CButton
{

	  DECLARE_DYNAMIC(CColoredButton)


  public:

	  CColoredButton();
	  virtual ~CColoredButton();

    afx_msg BOOL    OnEraseBkgnd( CDC* pDC );
    virtual void    DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

    void            SetBackground( COLORREF rgbColor, COLORREF rgbDisabledColor );
    void            SetColor( DWORD dwColor );


  protected:

	  DECLARE_MESSAGE_MAP()

    COLORREF                m_rgbBackground,
                            m_rgbDisabledBackground,
                            m_rgbTextColor;

public:
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnSetFocus(CWnd* pOldWnd);
};


