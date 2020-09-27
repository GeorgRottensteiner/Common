#pragma once


#include <Grafik/Gradient.h>


// CGradientCtrl

class CGradientCtrl : public CWnd
{
	DECLARE_DYNAMIC(CGradientCtrl)


  protected:

    DWORD             m_dwStyle;

    bool              m_Dragging;


    void              DrawLeftPeg( CDC& dc, GR::Graphic::Gradient::tPeg& Peg );
    void              DrawRightPeg( CDC& dc, GR::Graphic::Gradient::tPeg& Peg );

    bool              GetLeftPegRect( GR::Graphic::Gradient::tPeg& Peg, RECT& rc ) const;
    bool              GetRightPegRect( GR::Graphic::Gradient::tPeg& Peg, RECT& rc ) const;

    void              StopDragging();

    GR::Graphic::Gradient::tPeg*  m_pSelectedPeg;


  public:

    enum    eGradientCtrlStyles
    {
      GCS_NO_PEGS     = 0x00000001,
      GCS_PEGS_LEFT   = 0x00000002,
      GCS_PEGS_RIGHT  = 0x00000004,
      GCS_PEGS_BOTH   = GCS_PEGS_LEFT | GCS_PEGS_RIGHT,

      GCS_DEFAULT     = GCS_PEGS_BOTH,
    };


    #define GRGRADIENTCTRL_CLASS_NAME _T( "GRGradientCtrl" )

    static BOOL hasclass;
    static BOOL RegisterMe();

    static DWORD      m_dwNotifyMessage;



	  CGradientCtrl();
	  virtual ~CGradientCtrl();

    GR::Graphic::Gradient           m_Gradient;


    void                            AddPeg( DWORD dwColor, float fPosition );

    GR::Graphic::Gradient::tPeg*    PegFromPoint( POINT& point, bool bIncludeStartAndEndPegs = true );


  protected:
	  DECLARE_MESSAGE_MAP()

  public:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};


