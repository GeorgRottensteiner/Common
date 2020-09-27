#ifndef GR_UI_MENU_H
#define GR_UI_MENU_H

#include <vector>

#include <WinSys/WinUtils.h>

#include "CustomWnd.h"


class GRUIMenuBar;

class CGRUIMenu : public CCustomWnd
{

  protected:

    GRUIMenuBar*                m_pMenuBar;

    HBITMAP                     m_hbmDesktop;

    HWND                        m_hWndMenuOwner;  

    DWORD_PTR                   m_dwChosenMenuItem;

    DWORD                       m_dwDefaultItem;

    static CGRUIMenu*           g_pODMenu;

    std::vector<tGRUIMenuItem>  m_vectMenuItems;

    int                         m_iWidth,
                                m_iItemHeight,
                                m_iIndent,
                                m_iSeparatorHeight,
                                m_iBorderWidth,
                                m_iBorderHeight,
                                m_iShadowWidth;

    size_t                      m_iSelectedItem;

    bool                        m_bCaptureMouse,
                                m_bDrawShadow,
                                m_bDropShadowEnabled,
                                m_bTimerSet;

    CGRUIMenu*                  m_pParentMenu;
    CGRUIMenu*                  m_pOpenedSubMenu;

    HDC                         m_hdcDesktop,
                                m_hdcRightSide,
                                m_hdcBottomSide;

    HBITMAP                     m_hbmRightSide,
                                m_hbmBottomSide,
                                m_hbmSavedRightSide,
                                m_hbmSavedBottomSide;

    RECT                        m_rcShadow;


  public:

    CGRUIMenu();
    virtual ~CGRUIMenu();

    BOOL AppendMenu( UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, DWORD dwItemData = 0 );
    BOOL TrackPopupMenu( UINT nFlags, int x, int y, HWND hwnd, LPCRECT lpRect );
    BOOL EnableMenuItem( UINT_PTR nID, BOOL bOn = TRUE );
    BOOL SetMenuItemIcons( UINT uPosition, UINT uFlags, HICON hIconChecked, HICON hIconUnchecked );


  protected:

    //void DrawMenuItem( CGRUIMenu* pMenu, HDC dc, RECT& rc, tGRUIMenuItem& MenuItem );

    virtual void DrawBackground( HDC hdc, const RECT& rcRedraw );
    virtual void Draw( HDC hdc, const RECT& rc2 );
    void PostNcDestroy();
    void OnMouseMove( UINT nFlags, POINT point);
    CGRUIMenu* IsPartOfMenuChain( HWND hWnd );
    CGRUIMenu* GetTopLevelMenu();
    void OnLButtonDown( UINT nFlags, POINT point );
    void DestroyChildChain();
    bool GetItemRect( size_t iIndex, RECT& rc );
    int ItemFromPoint( const POINT& pt );
    void RecalcSize();
    COLORREF ShadowColor( int iX, int iY, int iShadowValue );
    tGRUIMenuItem* FindMenuItem( UINT uPosition, UINT uFlags );
    void DeleteAllMenuItems();
    int OnCreate();
    void SaveShadowBackground();
    void CompleteClose();
    void OnSysCommand(UINT nID, LPARAM lParam);
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void NotActiveAnymore( HWND hwndNew );
    void OnSysKeyDown();
    //virtual void DrawBackground2( HDC hdc, const RECT& rcBG );
    BOOL DestroyWindow();
    void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    void SaveDesktop( HWND hWnd );
    int GetMenuItemCount() const;
    BOOL CheckMenuItem( UINT_PTR nID, UINT uCheck );
    virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
    void OnNcLButtonDown( UINT nHitTest, POINT point );
    CGRUIMenu* ReplaceMenu( HMENU hMenu );
    void Clone( HMENU hMenu, HWND hwndSysOwner );

    void OnRButtonDown( UINT nFlags, POINT point );

    BOOL _RegisterClass();


    friend class GRUIMenuBar;

};

#endif // GR_UI_MENU_H