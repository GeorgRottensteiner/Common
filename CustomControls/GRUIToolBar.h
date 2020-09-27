#pragma once

#include <vector>
#include <windows.h>
#include <commctrl.h>

#include "GRUIControlBar.h"



class GRUIToolBar : public GRUIControlBar
{

  protected:

    typedef std::vector<tToolBarButtonInfo>    tVectButtons;

    size_t                      m_iHighlightedButton,
                                m_iPushedButton;

    tVectButtons                m_vectButtons;

    HIMAGELIST                  m_hImageList;

    DWORD                       m_dwTimerID;


  public:


    GRUIToolBar();
    virtual ~GRUIToolBar();

    BOOL                        AddButton( DWORD dwCommandID );
    BOOL                        AddButton( DWORD dwCommandID, HICON hIcon );
    BOOL                        AddCheckButton( DWORD dwCommandID, HICON hIcon );
    BOOL                        AddRadioButton( DWORD dwCommandID, HICON hIcon );
    BOOL                        AddSeparator();

    virtual SIZE                EstimateSize( DWORD dwSizingFlags = GRCBS_APPLY_SIZE | GRCBS_FLOATING );

    virtual INT_PTR             OnToolHitTest( POINT point, TOOLINFO* pTI ) const;

    BOOL                        LoadToolBar( HINSTANCE hInst, DWORD dwResourceID );
    BOOL                        LoadToolBar( HINSTANCE hInst, LPCTSTR lpszResourceName);

    BOOL                        LoadBitmap( HINSTANCE hInstImageWell, LPCTSTR lpszResourceName );

    virtual void                Draw( HDC hdc, const RECT& rc );

    void                        EnableControl( UINT nIndex, BOOL bOn );
    void                        SetCheck( UINT nIndex, BOOL bSet );

    void                        LButtonDown(UINT nFlags, POINT point);
    void                        LButtonUp(UINT nFlags, POINT point);
    void                        MouseMove(UINT nFlags, POINT point);
    void                        OnDestroy();
    void                        OnTimer( UINT_PTR nIDEvent);

    virtual LRESULT             WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

};
