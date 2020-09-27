#pragma once

#include "CustomWnd.h"

#include <list>


class GRUIControlBar;

class GRUIDockFrame : public CCustomWnd
{

  public:

    enum eDockFramePos
    {
      DFP_FREE        = 0x0000,
      DFP_TOP         = 0x0001,
      DFP_BOTTOM      = 0x0002,
      DFP_LEFT        = 0x0004,
      DFP_RIGHT       = 0x0008,
      DFP_ANY         = DFP_TOP | DFP_BOTTOM | DFP_LEFT | DFP_RIGHT,
    };

	  GRUIDockFrame();           // Dynamische Erstellung verwendet geschützten Konstruktor
	  virtual ~GRUIDockFrame();

    void              StartDragging( POINT point );

    eDockFramePos     DockPos() const;

  protected:

    std::list<GRUIControlBar*>   m_listControlBars;

    eDockFramePos     m_dfPos;

    GRUIControlBar*   m_pControlBar;

    bool              m_bDragging;

    POINT             m_ptDragOffset;

    void              InsertControlBar( GRUIControlBar* pBar );
    void              RemoveControlBar( GRUIControlBar* pBar );

    void              RepositionControlBars( GRUIControlBar* pBar, POINT& ptBarPos, DWORD dwDockPos );

    virtual void      CalculateNeededSize( RECT& rcRemainingSize );

    virtual LRESULT   WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    virtual void      OnSizeParent( RECT& rcMDI );

    virtual void      Draw( HDC, const RECT& rc );
    virtual void      DrawBackground( HDC hdc, const RECT& rc );

    friend class GRUIControlBar;
    friend class GRUIDockingManager;
    friend class GRUIMDIApp;


  public:

    virtual BOOL      Create( HWND hwndParent, DWORD dwExStyle, DWORD dwStyle );

    void NcLButtonDown(UINT nHitTest, POINT point);
    void OnSize(UINT nType, int cx, int cy);
    int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    UINT OnNcHitTest(POINT point);
    void NcMouseMove(UINT nHitTest, POINT point);
    void MouseMove(UINT nFlags, POINT point);
    void NcLButtonDblClk(UINT nHitTest, POINT point);
    void LButtonUp(UINT nFlags, POINT point);
    void OnClose();
    void OnContextMenu(CWnd* /*pWnd*/, POINT /*point*/);
    void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

    bool IsCaptionActive();
};


