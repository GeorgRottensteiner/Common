#pragma once

#include "ClassicDisplayClass.h"

#include "GRUIControlBar.h"
#include "GRUIMenu.h"

// GRUIMenuBar


class GRUIMenuBar : public GRUIControlBar
{

  public:


	  GRUIMenuBar();
	  virtual ~GRUIMenuBar();

    virtual BOOL                LoadHMenu( HMENU hMenu );
    CGRUIMenu*                  ReplaceMenu( HMENU hMenu );

    virtual UINT                TrackMenu( HMENU hMenu, DWORD_PTR dwExtraData, UINT nAlignFlags, int iX, int iY, 
                                          HWND hwndParent, TPMPARAMS* pParams = NULL );

    virtual void                UpdatePopupMenu( size_t iPushedItem, HMENU hMenu );

    virtual SIZE                EstimateSize( DWORD dwSizingFlags );

    virtual void                DeleteMenuItem( tItemInfo& ItemInfo );

    BOOL                        AddItem( DWORD dwCommandID, HICON hIcon = NULL );
    BOOL                        AddItem( const char* szText, DWORD dwCommandID, HICON hIcon = NULL );
    BOOL                        AddSubMenu( const char* szText, CGRUIMenu* pSubMenu, HICON hIcon = NULL );

    virtual void                EnableSubMenuItem( int m_nIndex, UINT_PTR nID, HMENU m_hSubMenu, BOOL bOn );

    bool                        SetMenuItemIcons( UINT_PTR nID,
                                                  DWORD dwFlags,
                                                  HICON hIconChecked,
                                                  HICON hIconUnchecked );


  protected:

    typedef std::vector<tItemInfo>    tVectItems;

    bool                        m_bTrackingMenu,
                                m_bKeepPushing;

    size_t                      m_iHighlightedItem,
                                m_iPushedItem;

    tVectItems                  m_vectItems;

    DWORD                       m_dwTimerID;

    HWND                        m_hwndMaximizedMDIChild;

    HMENU                       m_hmenuLoaded;

    HMENU                       m_hMenuDefault;

    bool                        m_bReOpenPopupMenu;


    virtual void                Draw( HDC hdc, const RECT& rc );
    virtual void                DrawBackground( HDC hdc, const RECT& rcRedraw );

    virtual LRESULT             WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    size_t                      MenuItemFromPoint( POINT point );

    void                        OnLButtonDown( UINT nFlags, POINT point );
    void                        OnLButtonUp(UINT nFlags, POINT point);
    void                        OnMouseMove( UINT nFlags, POINT point );

    void                        ShowPopupMenu( size_t iItem, BOOL bMouseActivated );
    void                        PushButton( size_t iIndex );

    friend class CGRUIMenu;

};


