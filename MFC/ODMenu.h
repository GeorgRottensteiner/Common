#pragma once


// ODMenu
#include <vector>

class ODMenu : public CWnd
{

  public:

    enum eODMenuStyle
    {
      ODMS_CLASSIC = 0,
      ODMS_XP,
      ODMS_BLACKMARBLE,
    };

    DECLARE_DYNAMIC( ODMenu )

  protected:

    CWnd*                       m_pWndMenuOwner;

    HICON                       m_hIconCheck,
                                m_hIconPopupArrow;

    DWORD                       m_ChosenMenuItem;

    DWORD                       m_DefaultItem;

    static ODMenu*              g_pODMenu;

    static CBitmap              s_bmDesktop;

    struct tODMenuItem;

    std::vector<tODMenuItem>    m_MenuItems;

    int                         m_Width,
                                m_ItemHeight,
                                m_Indent,
                                m_BorderWidth,
                                m_BorderHeight,
                                m_ShadowWidth;

    bool                        m_CaptureMouse,
                                m_DrawShadow,
                                m_DropShadowEnabled;

    ODMenu*                     m_pParentMenu;
    ODMenu*                     m_pOpenedSubMenu;

    HDC                         m_hdcDesktop,
                                m_hdcRightSide,
                                m_hdcBottomSide;

    HBITMAP                     m_hbmRightSide,
                                m_hbmBottomSide,
                                m_hbmSavedRightSide,
                                m_hbmSavedBottomSide;

    RECT                        m_rcShadow;

    eODMenuStyle                m_odStyle;



    DECLARE_MESSAGE_MAP()
	
    #define ODMENU_CLASS_NAME _T( "ODMenu" )

    static BOOL hasclass;
    static BOOL RegisterMe();

  public:


	  ODMenu( eODMenuStyle odStyle = ODMS_CLASSIC );
	  virtual ~ODMenu();

    virtual BOOL        AppendMenu( UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL, DWORD dwItemData = 0 );
    virtual void        DeleteAllMenuItems();
    int                 GetMenuItemCount() const;

    virtual int         ItemFromPoint( const POINT& pt );
    virtual bool        GetItemRect( int iIndex, RECT& rc );

    virtual BOOL        EnableMenuItem( UINT_PTR nID, BOOL bOn = TRUE );
    virtual BOOL        CheckMenuItem( UINT_PTR nID, UINT uCheck );

    ODMenu*            GetTopLevelMenu();
    ODMenu*            IsPartOfMenuChain( HWND hWnd );

    BOOL                SetMenuItemIcons( UINT uPosition, UINT uFlags, HICON hIconChecked, HICON hIconUnchecked );

    virtual void        DisplayMenuIcon( RECT& rc, HDC hdc, HICON hIcon );
    virtual void        DisplayMenuBitmap( RECT& rc, HDC hdc, HBITMAP hbm );
    virtual void        DrawMenuItem( CDC& dc, RECT& rc, tODMenuItem& MenuItem );

    BOOL                TrackPopupMenu( UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect = NULL );

    void                Clone( HMENU hMenu, HWND hwndSysOwner = NULL );

	  
    afx_msg void OnPaint();
    afx_msg void OnKillFocus(CWnd* pNewWnd);


protected:

    ODMenu*            ReplaceMenu( HMENU hMenu );

    tODMenuItem*        FindMenuItem( UINT uPosition, UINT uFlags = MF_BYCOMMAND );

  virtual void PostNcDestroy();

    virtual void        RecalcSize();

    virtual COLORREF    ShadowColor( int iX, int iY, int iShadowValue );

    void                SaveShadowBackground();

    void                CompleteClose();
    void                DestroyChildChain();
    void                NotActiveAnymore( CWnd* pNewWnd );

    void                SaveDesktop( HWND hWnd );


public:
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnNcPaint();
  virtual BOOL DestroyWindow();
  afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
protected:
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
  afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


