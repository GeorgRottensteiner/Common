#ifndef INCLUDE_CWND_H
#define INCLUDE_CWND_H



#include <GR/GRTypes.h>
#include <map>

#include <windows.h>

#include <Misc/map.h>

#include <debug/debugclient.h>



namespace WindowsWrapper
{
  class CWnd
  {

    public:

    static CWnd*    m_pWndCreatingNow;

    HWND            m_hWnd,
      m_hwndOwner;

    bool            m_bAttached,
      m_bModal,
      m_bSubclassed;

    MSG             m_LastMsg,
      m_LastSentMsg;

    int             m_iModalResult;


    CWnd();

    CWnd( HWND hwnd )
      : m_hWnd( hwnd ),
      m_bModal( false ),
      m_bSubclassed( false ),
      m_iModalResult( 0 )
    {
    }



    HWND            GetSafeHwnd() const
    {
      return m_hWnd;
    }



    virtual GR::u32 GetStyle() const;
    virtual GR::u32 GetExStyle() const;
    virtual BOOL  ModifyStyle( GR::u32 dwRemove, GR::u32 dwAdd, UINT nFlags = 0 );
    virtual BOOL  ModifyStyleEx( GR::u32 dwRemove, GR::u32 dwAdd, UINT nFlags = 0 );

    virtual HWND  GetOwner() const;
    virtual void  SetOwner( HWND hwndOwner );

    virtual BOOL  PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
    BOOL          PostTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    static BOOL           PumpMessage();
    static BOOL PASCAL    WalkPreTranslateTree( HWND hWndStop, MSG* pMsg );

    static HMODULE        LoadLibrary( LPCTSTR lpLibFileName );
    static BOOL           FreeLibrary( HMODULE hLibModule );
    HINSTANCE             FindResourceHandle( LPCSTR lpszName, LPCTSTR lpszType );

    static CWnd*          FromHandle( HWND hwndChild );

    virtual BOOL  Attach( HWND hWndNew );
    virtual void  OnAttach();

    virtual HWND  Detach();

    static CWnd** GetCreatingWindow();

    public:

    virtual BOOL  Create( LPCSTR lpszClassName,
      LPCSTR lpszWindowName,
      GR::u32 dwStyle,
      const RECT& rect,
      HWND hwndParent,
      UINT_PTR nID,
      LPVOID lpParam = NULL );

    // advanced creation (allows access to extended styles)
    BOOL      CreateEx( GR::u32 dwExStyle,
      LPCSTR lpszClassName,
      LPCSTR lpszWindowName,
      GR::u32 dwStyle,
      int x, int y, int nWidth, int nHeight,
      HWND hwndParent,
      UINT_PTR nIDorHMenu,
      LPVOID lpParam = NULL );

    BOOL      CreateEx( GR::u32 dwExStyle,
      LPCSTR lpszClassName,
      LPCSTR lpszWindowName,
      GR::u32 dwStyle,
      const RECT& rect,
      HWND hwndParent,
      UINT_PTR nID,
      LPVOID lpParam = NULL );

    virtual BOOL DestroyWindow();

    // Advanced: virtual AdjustWindowRect
    enum AdjustType
    {
      adjustBorder = 0, adjustOutside = 1
    };
    virtual void CalcWindowRect( LPRECT lpClientRect,
      UINT nAdjustType = adjustBorder )
    {
    }

    // Window tree access
    int GetDlgCtrlID() const;
    int SetDlgCtrlID( int nID );
    // get and set window ID, for child windows only
    HWND GetDlgItem( int nID ) const;
    // get immediate child with given ID
    void GetDlgItem( int nID, HWND* phWnd ) const;
    // as above, but returns HWND
    CWnd* GetDescendantWindow( int nID, BOOL bOnlyPerm = FALSE ) const;
    // like GetDlgItem but recursive
    void SendMessageToDescendants( UINT message, WPARAM wParam = 0,
      LPARAM lParam = 0, BOOL bDeep = TRUE, BOOL bOnlyPerm = FALSE );
    CWnd* GetTopLevelParent() const;
    CWnd* GetTopLevelOwner() const;
    CWnd* GetParentOwner() const;

    // Message Functions
#undef SendMessage
    virtual LRESULT SendMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 );
    BOOL PostMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 );

    BOOL SendNotifyMessage( UINT message, WPARAM wParam, LPARAM lParam );
    BOOL SendChildNotifyLastMsg( LRESULT* pResult = NULL );

    // Message processing for modeless dialog-like windows
    BOOL IsDialogMessage( LPMSG lpMsg );

    // Window Text Functions
    void            SetWindowText( LPCSTR lpszString );
    inline void     SetWindowText( const GR::String& s )
    {
      SetWindowText( s.c_str() );
    }
    void            SetWindowText( GR::u32 dwResourceID = 0 );
    int             GetWindowText( GR::Char* lpszStringBuf, int nMaxCount ) const;
    GR::String      GetWindowText() const;
    int             GetWindowTextLength() const;

    void            SetDefaultTextForAllControls();

    void            SetFont( HFONT hFont, BOOL bRedraw = TRUE );
    HFONT           GetFont() const;

    // CMenu Functions - non-Child windows only
    HMENU           GetMenu() const;
    /*
    CMenu* GetMenu() const;
    BOOL SetMenu(CMenu* pMenu);
    void DrawMenuBar();
    CMenu* GetSystemMenu(BOOL bRevert) const;
    BOOL HiliteMenuItem(CMenu* pMenu, UINT nIDHiliteItem, UINT nHilite);
    */

    // Window Size and Position Functions
    bool            IsIconic() const;
    bool            IsZoomed() const;

    void MoveWindow( int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
    void MoveWindow( int x, int y, BOOL bRepaint = TRUE );
    void MoveWindow( const RECT* pRect, BOOL bRepaint = TRUE );
    int SetWindowRgn( HRGN hRgn, BOOL bRedraw );
    int GetWindowRgn( HRGN hRgn ) const;

    virtual BOOL          SetWindowPos( HWND hwndAfter, int x, int y, int cx, int cy, UINT nFlags );
    UINT ArrangeIconicWindows();
    void BringWindowToTop();
    void GetWindowRect( LPRECT lpRect ) const;
    void GetClientRect( LPRECT lpRect ) const;

    BOOL GetWindowPlacement( WINDOWPLACEMENT* lpwndpl ) const;
    BOOL SetWindowPlacement( const WINDOWPLACEMENT* lpwndpl );

    // Coordinate Mapping Functions
    void ClientToScreen( LPPOINT lpPoint ) const;
    void ClientToScreen( LPRECT lpRect ) const;
    void ScreenToClient( LPPOINT lpPoint ) const;
    void ScreenToClient( LPRECT lpRect ) const;
    void MapWindowPoints( CWnd* pwndTo, LPPOINT lpPoint, UINT nCount ) const;
    void MapWindowPoints( CWnd* pwndTo, LPRECT lpRect ) const;

    bool IsWindow() const;

    // Update/Painting Functions
      /*
      CDC* BeginPaint(LPPAINTSTRUCT lpPaint);
      void EndPaint(LPPAINTSTRUCT lpPaint);
      CDC* GetDC();
      CDC* GetWindowDC();
      int ReleaseDC(CDC* pDC);
      void Print(CDC* pDC, GR::u32 dwFlags) const;
      void PrintClient(CDC* pDC, GR::u32 dwFlags) const;
      */
    HDC   GetDC();
    int   ReleaseDC( HDC hdc );

    void UpdateWindow();
    void SetRedraw( BOOL bRedraw = TRUE );
    BOOL GetUpdateRect( LPRECT lpRect, BOOL bErase = FALSE );
    //int GetUpdateRgn(CRgn* pRgn, BOOL bErase = FALSE);
    void Invalidate( BOOL bErase = TRUE );
    void InvalidateRect( LPCRECT lpRect, BOOL bErase = TRUE );
    //void InvalidateRgn(CRgn* pRgn, BOOL bErase = TRUE);
    void ValidateRect( LPCRECT lpRect );
    //void ValidateRgn(CRgn* pRgn);
    BOOL ShowWindow( int nCmdShow );
    BOOL IsWindowVisible() const;
    void ShowOwnedPopups( BOOL bShow = TRUE );

    //CDC* GetDCEx(CRgn* prgnClip, GR::u32 flags);
    BOOL LockWindowUpdate();    // for backward compatibility
    void UnlockWindowUpdate();
    /*
    BOOL RedrawWindow(LPCRECT lpRectUpdate = NULL,
      CRgn* prgnUpdate = NULL,
      UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    */
    BOOL EnableScrollBar( int nSBFlags, UINT nArrowFlags = ESB_ENABLE_BOTH );

    // Timer Functions
    UINT_PTR SetTimer( UINT nIDEvent, UINT nElapse, TIMERPROC lpfnTimer = NULL );
    BOOL KillTimer( int nIDEvent );

    // ToolTip Functions
    BOOL EnableToolTips( BOOL bEnable = TRUE );
    BOOL EnableTrackingToolTips( BOOL bEnable = TRUE );
    static void PASCAL CancelToolTips( BOOL bKeys = FALSE );
    void FilterToolTipMessage( MSG* pMsg );

    // for command hit testing (used for automatic tooltips)
    //virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

  // Window State Functions
    BOOL IsWindowEnabled() const;
    BOOL EnableWindow( BOOL bEnable = TRUE );

    // the active window applies only to top-level (frame windows)
    static HWND PASCAL  GetActiveWindow();
    HWND                SetActiveWindow();

    // the foreground window applies only to top-level windows (frame windows)
    BOOL                SetForegroundWindow();
    static HWND PASCAL  GetForegroundWindow();

    // capture and focus apply to all windows
    static HWND PASCAL GetCapture();
    HWND SetCapture();
    static HWND PASCAL GetFocus();
    HWND SetFocus();

    // Obsolete and non-portable APIs - not recommended for new code
    void CloseWindow();
    BOOL OpenIcon();

    // Dialog-Box Item Functions
    // (NOTE: Dialog-Box Items/Controls are not necessarily in dialog boxes!)
    void CheckDlgButton( int nIDButton, UINT nCheck );
    void CheckRadioButton( int nIDFirstButton, int nIDLastButton,
      int nIDCheckButton );
    int GetCheckedRadioButton( int nIDFirstButton, int nIDLastButton );
    int DlgDirList( LPTSTR lpPathSpec, int nIDListBox,
      int nIDStaticPath, UINT nFileType );
    int DlgDirListComboBox( LPTSTR lpPathSpec, int nIDComboBox,
      int nIDStaticPath, UINT nFileType );
    BOOL DlgDirSelect( LPTSTR lpString, int nIDListBox );
    BOOL DlgDirSelectComboBox( LPTSTR lpString, int nIDComboBox );

    UINT GetDlgItemInt( int nID, BOOL* lpTrans = NULL,
      BOOL bSigned = TRUE ) const;
    int GetDlgItemText( int nID, LPTSTR lpStr, int nMaxCount ) const;
    //int GetDlgItemText(int nID, CString& rString) const;
    CWnd* GetNextDlgGroupItem( CWnd* pWndCtl, BOOL bPrevious = FALSE ) const;

    HWND                GetNextDlgTabItem( HWND hwndParent, BOOL bPrevious = FALSE ) const;

    UINT IsDlgButtonChecked( int nIDButton ) const;
    LRESULT SendDlgItemMessage( int nID, UINT message,
      WPARAM wParam = 0, LPARAM lParam = 0 );
    void SetDlgItemInt( int nID, UINT nValue, BOOL bSigned = TRUE );
    void              SetDlgItemText( int nID, LPCTSTR lpszString );
    void              SetDlgItemText( int nID, const GR::String& strText );

    // Scrolling Functions
    int GetScrollPos( int nBar ) const;
    void GetScrollRange( int nBar, LPINT lpMinPos, LPINT lpMaxPos ) const;
    void ScrollWindow( int xAmount, int yAmount,
      LPCRECT lpRect = NULL,
      LPCRECT lpClipRect = NULL );
    int SetScrollPos( int nBar, int nPos, BOOL bRedraw = TRUE );
    void SetScrollRange( int nBar, int nMinPos, int nMaxPos,
      BOOL bRedraw = TRUE );
    void ShowScrollBar( UINT nBar, BOOL bShow = TRUE );
    void EnableScrollBarCtrl( int nBar, BOOL bEnable = TRUE );
    //virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
        // return sibling scrollbar control (or NULL if none)

    /*
    int ScrollWindowEx(int dx, int dy,
          LPCRECT lpRectScroll, LPCRECT lpRectClip,
          CRgn* prgnUpdate, LPRECT lpRectUpdate, UINT flags);
          */
    BOOL SetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo,
      BOOL bRedraw = TRUE );
    BOOL GetScrollInfo( int nBar, LPSCROLLINFO lpScrollInfo, UINT nMask = SIF_ALL );
    int GetScrollLimit( int nBar );

    // Window Access Functions
    CWnd* ChildWindowFromPoint( POINT point ) const;
    CWnd* ChildWindowFromPoint( POINT point, UINT nFlags ) const;
    //CWnd* GetNextWindow(UINT nFlag = GW_HWNDNEXT) const;
    CWnd* GetTopWindow() const;

    HWND                GetWindow( UINT nCmd ) const;
    CWnd* GetLastActivePopup() const;

    BOOL IsChild( const CWnd* pWnd ) const;
    HWND                GetParent() const;
    void                SetParent( HWND hwndNewParent );
    static CWnd* PASCAL WindowFromPoint( POINT point );

    // Alert Functions
    BOOL FlashWindow( BOOL bInvert );
    int         MessageBox( LPCTSTR lpszText, LPCTSTR lpszCaption = NULL, UINT nType = MB_OK );

    // Clipboard Functions
    BOOL ChangeClipboardChain( HWND hWndNext );
    HWND SetClipboardViewer();
    BOOL OpenClipboard();
    static CWnd* PASCAL GetClipboardOwner();
    static CWnd* PASCAL GetClipboardViewer();
    static CWnd* PASCAL GetOpenClipboardWindow();

    // Caret Functions
      //void CreateCaret(CBitmap* pBitmap);
    void CreateSolidCaret( int nWidth, int nHeight );
    void CreateGrayCaret( int nWidth, int nHeight );
    //static CPoint PASCAL GetCaretPos();
    static void PASCAL SetCaretPos( POINT point );
    void HideCaret();
    void ShowCaret();

    // Shell Interaction Functions
    void DragAcceptFiles( BOOL bAccept = TRUE );

    // Icon Functions
    HICON SetIcon( HICON hIcon, BOOL bBigIcon );
    HICON GetIcon( BOOL bBigIcon ) const;

    // Context Help Functions
    BOOL SetWindowContextHelpId( GR::u32 dwContextHelpId );
    GR::u32 GetWindowContextHelpId() const;

    // Dialog Data support
    public:
    BOOL UpdateData( BOOL bSaveAndValidate = TRUE );
    // data wnd must be same type as this

    virtual void WinHelp( GR::u32 dwData, UINT nCmd = HELP_CONTEXT )
    {
    }

    // Layout and other functions
    public:
    enum RepositionFlags
    {
      reposDefault = 0, reposQuery = 1, reposExtra = 2
    };
    void RepositionBars( UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver,
      UINT nFlag = reposDefault, LPRECT lpRectParam = NULL,
      LPCRECT lpRectClient = NULL, BOOL bStretch = TRUE );

    // dialog support
    void            CenterWindow( HWND hwndAlternateParent );
    void            CenterWindow();

    int RunModalLoop( GR::u32 dwFlags = 0 );
    virtual BOOL ContinueModal()
    {
      return TRUE;
    }
    virtual void EndModalLoop( int nResult );


    LONG_PTR      SetWindowLongPtr( int nIndex, LONG_PTR dwNewLong );
    LONG_PTR      GetWindowLongPtr( int nIndex ) const;

    // Overridables and other helpers (for implementation of derived classes)
    protected:

    virtual BOOL  OnInitDialog();


    public:
    // for modality
    virtual void BeginModalState()
    {
    }
    virtual void EndModalState()
    {
    }

    protected:
    // for processing Windows messages
    //virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam){return 0;}
    virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

    // for handling default processing
    LRESULT Default();
    virtual LRESULT DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam )
    {
      return 0;
    }

    // for custom cleanup after WM_NCDESTROY
    virtual void PostNcDestroy()
    {
    }

    // for notifications from parent
    virtual BOOL OnChildNotify( UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult )
    {
      return TRUE;
    }
    // return TRUE if parent should not process this message

// Implementation
    public:
    virtual ~CWnd();
    virtual BOOL CheckAutoCenter()
    {
      return TRUE;
    }

    public:

    UINT m_nFlags;      // see WF_ flags above

    protected:
    static const UINT m_nMsgDragList;
    int m_nModalResult; // for return values from CWnd::RunModalLoop

    // for creating dialogs and dialog-like windows
    BOOL CreateDlg( LPCTSTR lpszTemplateName, HWND hwndParent );
    BOOL CreateDlgIndirect( LPCDLGTEMPLATE lpDialogTemplate, HWND hwndParent );
    BOOL CreateDlgIndirect( LPCDLGTEMPLATE lpDialogTemplate, HWND hwndParent, HINSTANCE hInst );


    protected:

    WNDPROC   m_oldWndProc;


    friend LRESULT CALLBACK MYHELPERWINDOWPROC( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    friend INT_PTR CALLBACK MYHELPERDIALOGPROC( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    friend class CHooker;

    // Überschreibungen
    virtual void                    OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual void                    OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
    virtual void                    OnSize( UINT nType, int cx, int cy );
    virtual void                    OnEnable( BOOL bEnable );
    virtual void                    OnMouseMove( UINT nFlags, POINT point );
    virtual void                    OnLButtonUp( UINT nFlags, POINT point );
    virtual void                    OnLButtonDown( UINT nFlags, POINT point );
    virtual void                    OnNcLButtonDown( UINT nHitTest, POINT point );
    virtual void                    OnNcLButtonDblClk( UINT nHitTest, POINT point );
    virtual void                    OnGetMinMaxInfo( MINMAXINFO* lpMMI );
    virtual void                    OnClose();
    virtual void                    OnDestroy();
    virtual void                    OnTimer( UINT_PTR nIDEvent );

  };

  namespace GLOBAL
  {
    namespace CWND
    {
      typedef std::map<HWND, CWnd*>    mapHWNDToCWnd;

      class CWndHelper
      {
        public:

        GR::map<HWND, CWnd*>         mapCWnd;

        GR::map<HMODULE, HMODULE>    mapDlls;

        //mapHWNDToCWnd     mapCWndToHwnd;

        CWnd*    FindEntry( HWND hwnd );
        void     InsertWindow( HWND hWnd, CWnd* pWnd );
        void     RemoveWindow( HWND hWnd );

        ~CWndHelper();
      };

      CWndHelper*    GetHelper();

    };
  };


  class CHooker
  {
    public:

    static HHOOK      m_hHook;

    CHooker()
    {
      SetEnvironmentVariableA( "GRCWnd-CreatingWnd", NULL );
      SetEnvironmentVariableA( "GRCWnd-WNDHELPER", NULL );
      m_hHook = SetWindowsHookEx( WH_CBT,
        CBTHookProc,
        GetModuleHandle( NULL ),
        GetCurrentThreadId() );
    }

    ~CHooker()
    {
      if ( m_hHook )
      {
        UnhookWindowsHookEx( m_hHook );
        m_hHook = NULL;
      }
    }

    static LRESULT CALLBACK CHooker::CBTHookProc( int nCode, WPARAM wParam, LPARAM lParam );

  };

}


#endif INCLUDE_CWND_H
