#pragma once

#include <vector>
#include <windows.h>
#include <commctrl.h>

#include "customwnd.h"


class GRUIDockFrame;

class GRUIControlBar : public CCustomWnd
{

  private:

    bool                        m_bFloating;


  protected:

    bool                        m_bHorizontal;

    GRUIDockFrame*              m_pFrameWnd;
    GRUIDockFrame*              m_pDockingFrame;

    DWORD                       m_dwCBStyle,
                                m_dwMRUDockPos;

    POINT                       m_ptMRUDockedPos,
                                m_ptMRUFloatingPos;


  public:

    enum eGRControlBarStyles
    {
      GRCBS_FIXED_SIZE        = 0x00000001,   // the size cannot be changed
      GRCBS_CONTENT_SIZE      = 0x00000002,   // the size depends on content, but can be wrapping (toolbar behaviour)
      GRCBS_FREE_SIZE         = 0x00000004,   // free sizing (dlg-like)
      GRCBS_DOCK_LEFT         = 0x00000010,
      GRCBS_DOCK_RIGHT        = 0x00000020,
      GRCBS_DOCK_TOP          = 0x00000040,
      GRCBS_DOCK_BOTTOM       = 0x00000080,
      GRCBS_DOCK_ANY          = GRCBS_DOCK_LEFT | GRCBS_DOCK_RIGHT | GRCBS_DOCK_TOP | GRCBS_DOCK_BOTTOM,
      GRCBS_APPLY_SIZE        = 0x00010000,
      GRCBS_DOCKED            = 0x00020000,
      GRCBS_DOCKED_LEFT       = 0x00040000,
      GRCBS_DOCKED_RIGHT      = 0x00080000,
      GRCBS_DOCKED_TOP        = 0x00100000,
      GRCBS_DOCKED_BOTTOM     = 0x00200000,
      GRCBS_FLOATING          = 0x00400000,
      GRCBS_HIDDEN            = 0x00800000,
    };


    GRUIControlBar();
    virtual ~GRUIControlBar();

    virtual BOOL                Create( LPCTSTR lpWindowName, DWORD dwCBStyle, HWND hwndOwner, UINT nID = 0 );

    virtual SIZE                EstimateSize( DWORD dwSizingFlags = GRCBS_APPLY_SIZE | GRCBS_FLOATING );
    SIZE                        CalcSize( DWORD dwSizingFlags = GRCBS_APPLY_SIZE | GRCBS_FLOATING );

    virtual void                Draw( HDC hdc, const RECT& rc );

    virtual bool                IsFloating() const;

    void                        Dock( DWORD dwDockPos, int iX, int iY );
    void                        Float( int iX, int iY );
    void                        ToggleState();

    void                        OnLButtonDown(UINT nFlags, POINT point);
    void                        OnLButtonUp(UINT nFlags, POINT point);

    virtual LRESULT             WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    bool                        GetGripperRect( RECT& rc );


    friend class GRUIDockingManager;
    friend class GRUIDockFrame;

};
