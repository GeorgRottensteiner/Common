#if !defined(AFX_RANGEEDIT_H__5A59E343_1C75_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_RANGEEDIT_H__5A59E343_1C75_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RangeEdit.h : Header-Datei
//

#include "stdafx.h"

#include <string>

#include <GR/GRTypes.h>



class RangeEdit : public CWnd
{

  public:

    enum eSliderMode
    {
      SM_LINEAR = 0x00000001,
      SM_DEFAULT = SM_LINEAR,
      SM_EXPONENTIELL = 0x00000002
    };

    enum eMode
    {
      MODE_INTEGER = 0,
      MODE_FLOAT
    };



  protected:

    //{{AFX_MSG(CRangeEdit)
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnKillFocus( CWnd* pNewWnd );
    afx_msg void OnSetFocus( CWnd* pOldWnd );
    afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
    afx_msg void OnEnable( BOOL bEnable );
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()



    eSliderMode     m_SliderMode;

    HWND            m_hwndPopup;

                      // 0x00000001 = Button gedrückt
                      // 0x00000002 = dragging small bar
    DWORD           m_ControlFlags;

    eMode           m_Mode;

    COLORREF        m_bgrBarColor;

    int             m_Minimum,
                    m_Maximum,
                    m_Position,
                    m_CaretX,
                    m_Step,

                    m_FirstVisibleChar,
                    m_CursorPosInText,
                    m_SelectionAnchor,

                    m_DigitsPreComma,
                    m_DigitsPostComma;

    float           m_MinimumF,
                    m_MaximumF,
                    m_PositionF,
                    m_StepF;

    GR::String      m_Content,
                    m_TempContent;

    bool            m_MouseDownInEdit;

    bool            m_Focus;


    #define RANGEEDIT_CLASS_NAME _T( "RangeEdit" )

    static BOOL     hasclass;
    static BOOL     RegisterMe();


    void GetEditRect( RECT& rc );
    void GetSliderRect( RECT& rc );
    void GetButtonRect( RECT& rc );

    void UpdatePositionFromString();
    void UpdatePositionFromValue();

    int  CursorPosFromPoint( POINT& pt );

    void PositionChanged();

    void RecalcCaretPos();

    void StopButton();

    bool HasSelection();

    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );



  public:



	  RangeEdit();
	  virtual ~RangeEdit();

    virtual void SetRange( int Min, int Max, int Step = 1 );
    virtual void SetRange( float Min, float Max, float Step = 0.02f, int DigitsPreComma = -1, int DigitsPostComma = 2 );

    virtual void SetPosition( int Pos );
    virtual void SetPosition( float Pos );

    int          GetPosition() const;
    float        GetPositionF() const;


    void         SetMode( eMode Mode );

    virtual void SetBarColor( COLORREF bgrColor );

    virtual void DrawPopup( HDC hdc, const RECT& Target );


    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

};


#endif // AFX_RANGEEDIT_H__5A59E343_1C75_11D6_AC02_00104B6795C0__INCLUDED_
