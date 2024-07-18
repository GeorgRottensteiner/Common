#include <windows.h>
#include <WinUser.h>
#include <WinSys/WinUtils.h>

#include "CWnd.h"

#include <map>
#pragma comment( lib, "shell32.lib" )



namespace WindowsWrapper
{
  namespace GLOBAL
  {
    namespace CWND
    {
      CWndHelper*       pGlobalHelper = NULL;

      HANDLE            g_hFileMapping = INVALID_HANDLE_VALUE;

      HANDLE            g_hGlobalFile = INVALID_HANDLE_VALUE;


      CWndHelper::~CWndHelper()
      {
        if ( g_hGlobalFile != INVALID_HANDLE_VALUE )
        {
          CloseHandle( g_hGlobalFile );
          g_hGlobalFile = INVALID_HANDLE_VALUE;
        }
        if ( g_hFileMapping != INVALID_HANDLE_VALUE )
        {
          if ( FAILED( CloseHandle( g_hFileMapping ) ) )
          {
            dh::Log( "Close FileMapping Handle -failed!" );
          }
          g_hFileMapping = INVALID_HANDLE_VALUE;
        }
      }

      static CWndHelper*    GetHelper()
      {
        if ( pGlobalHelper )
        {
          return pGlobalHelper;
        }

        char          szMappedFileName[MAX_PATH];


        sprintf_s( szMappedFileName, MAX_PATH, "GR_CWND_HELPER_%0x", GetCurrentProcessId() );

        if ( g_hGlobalFile == INVALID_HANDLE_VALUE )
        {
          g_hGlobalFile = OpenFileMappingA( FILE_MAP_READ, FALSE, szMappedFileName );
          if ( g_hGlobalFile != NULL )
          {
            LPVOID   pTemp = MapViewOfFile( g_hGlobalFile, FILE_MAP_READ, 0, 0, 0 );

            if ( pTemp )
            {
              memcpy( &pGlobalHelper, pTemp, 4 );
              UnmapViewOfFile( pTemp );
            }
            CloseHandle( g_hGlobalFile );
            g_hGlobalFile = INVALID_HANDLE_VALUE;
            return pGlobalHelper;
          }
          g_hGlobalFile = INVALID_HANDLE_VALUE;
        }

        if ( g_hFileMapping == INVALID_HANDLE_VALUE )
        {
          g_hFileMapping = CreateFileMappingA( INVALID_HANDLE_VALUE,
                                              NULL,
                                              PAGE_READWRITE,
                                              0,
                                              sizeof( pGlobalHelper ),
                                              szMappedFileName );
          if ( g_hFileMapping != NULL )
          {
            if ( GetLastError() != ERROR_ALREADY_EXISTS )
            {
              static    CWndHelper    g_Helper;

              // das FileMapping-Objekt ist neu erzeugt worden

              g_hGlobalFile = OpenFileMappingA( FILE_MAP_WRITE, FALSE, szMappedFileName );
              if ( g_hGlobalFile != NULL )
              {
                LPVOID   pTemp = MapViewOfFile( g_hGlobalFile, FILE_MAP_WRITE, 0, 0, 0 );

                DWORD_PTR   dwAddress = (DWORD_PTR)&g_Helper;
                memcpy( pTemp, &dwAddress, sizeof( DWORD_PTR ) );

                UnmapViewOfFile( pTemp );

                CloseHandle( g_hGlobalFile );
              }
              g_hGlobalFile = INVALID_HANDLE_VALUE;
              return GetHelper();
            }
          }

        }
        return NULL;
        /*
        char    szBuffer[MAX_PATH];
        if ( !GetEnvironmentVariable( "GRCWnd-WNDHELPER", szBuffer, MAX_PATH ) )
        {
          static CWndHelper     wndHelper;

          pGlobalHelper = &wndHelper;
          wsprintf( szBuffer, "%d", pGlobalHelper );
          SetEnvironmentVariable( "GRCWnd-WNDHELPER", szBuffer );
        }
        else
        {
          pGlobalHelper = (CWndHelper*)_atoi64( szBuffer );
        }
        return pGlobalHelper;
        */
      }

      CWnd* CWndHelper::FindEntry( HWND hwnd )
      {
        /*
        mapHWNDToCWnd::iterator   it( mapCWndToHwnd.find( hwnd ) );
        if ( it == mapCWndToHwnd.end() )
        {
          return NULL;
        }
        return it->second;
        */
        GR::map<HWND,CWnd*>::tNode*   pNode = mapCWnd.find( hwnd );
        if ( pNode )
        {
          return pNode->Content;
        }
        return NULL;
      }

      void CWndHelper::InsertWindow( HWND hWnd, CWnd* pWnd )
      {
        mapCWnd.insert( hWnd, pWnd );
      }

      void CWndHelper::RemoveWindow( HWND hWnd )
      {
        GR::map<HWND,CWnd*>::tNode*   pNode = mapCWnd.find( hWnd );
        if ( pNode )
        {
          mapCWnd.erase( pNode );
        }
        /*
        mapHWNDToCWnd::iterator   it( mapCWndToHwnd.find( hWnd ) );
        if ( it != mapCWndToHwnd.end() )
        {
          mapCWndToHwnd.erase( it );
        }
        */
      }

      static CHooker  HookHelper;


    };
  };

  HHOOK CHooker::m_hHook = NULL;

  CWnd* CWnd::m_pWndCreatingNow   = NULL;



  LRESULT CALLBACK CHooker::CBTHookProc( int nCode, WPARAM wParam, LPARAM lParam )
  {
	  if ( nCode != HCBT_CREATEWND )
	  {
		  // wait for HCBT_CREATEWND just pass others on...
		  return CallNextHookEx( m_hHook, nCode, wParam, lParam );
	  }

    CWnd** ppWnd = CWnd::GetCreatingWindow();

    if ( *ppWnd == NULL )
    {
      //return 0;
      return CallNextHookEx( m_hHook, nCode, wParam, lParam );
    }

    HWND    hwnd = (HWND)wParam;

    CWnd*   pWndNew = *ppWnd;


    *ppWnd = NULL;

    if ( pWndNew->m_hWnd )
    {
      dh::Log( "OVERRIDE HWND!!!!\n" );
    }
    pWndNew->m_hWnd = hwnd;

    GLOBAL::CWND::GetHelper()->InsertWindow( hwnd, pWndNew );

    WNDPROC   wndProcOld = (WNDPROC)pWndNew->GetWindowLongPtr( GWLP_WNDPROC );
    if ( wndProcOld != (WNDPROC)MYHELPERWINDOWPROC )
    {
      pWndNew->m_oldWndProc = wndProcOld;
      pWndNew->SetWindowLongPtr( GWLP_WNDPROC, (LONG_PTR)MYHELPERWINDOWPROC );
    }

    //return 0;
    return CallNextHookEx( m_hHook, nCode, wParam, lParam );

  }



  LRESULT CALLBACK MYHELPERWINDOWPROC( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    CWnd*   pWnd = GLOBAL::CWND::GetHelper()->FindEntry( hwnd );

    if ( pWnd == NULL )
    {
      return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
    }

    /*
    if ( uMsg == WM_INITDIALOG )
    {
      // wird bereits in DialogProc abgefackelt
      INT_PTR   iResult = pWnd->OnInitDialog();

      pWnd->SetWindowLongPtr( DWL_MSGRESULT, iResult );
      return iResult;
      //return 0;
      //dh::Log( "MYHELPERWINDOWPROC WM_INITDIALOG\n" );
    }
    */

    LRESULT   lResult = 0;

    // die eigene Window-Proc macht das
    if ( hwnd == pWnd->m_hWnd )
    {
      pWnd->m_LastMsg.hwnd = hwnd;
      pWnd->m_LastMsg.lParam = lParam;
      pWnd->m_LastMsg.message = uMsg;
      pWnd->m_LastMsg.wParam = wParam;

      lResult = pWnd->WindowProc( uMsg, wParam, lParam );
    }

    pWnd->m_LastSentMsg = pWnd->m_LastMsg;

    return lResult;

  }


  #include <debug/debugclient.h>



  INT_PTR CALLBACK MYHELPERDIALOGPROC( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    CWnd*   pWnd = GLOBAL::CWND::GetHelper()->FindEntry( hWnd );
    if ( pWnd )
    {
      if ( uMsg == WM_INITDIALOG )
      {
        return pWnd->OnInitDialog();
      }
    }
    return FALSE;

  }



  BOOL CWnd::PumpMessage()
  {

    MSG     msgPump;


    BOOL    bRet = GetMessage( &msgPump, NULL, 0, 0 );

    //dh::Log( "Msg Wnd %x Msg %x", msgPump.hwnd, msgPump.message );
    if ( bRet == 0 )
    {
      return 0;
    }
    else if ( bRet == -1 )
    {
      // Fehler!
      return -1;
    }
    else
    {
      if ( CWnd::WalkPreTranslateTree( NULL, &msgPump ) )
      {
        return bRet;
      }
	    ::TranslateMessage( &msgPump );
	    ::DispatchMessage( &msgPump );
      return bRet;
    }
    ::DefWindowProc( msgPump.hwnd, msgPump.message, msgPump.wParam, msgPump.lParam );
    return bRet;

  }



  BOOL PASCAL CWnd::WalkPreTranslateTree( HWND hWndStop, MSG* pMsg )
  {

    LRESULT   lResult;

	  for ( HWND hWnd = pMsg->hwnd; hWnd != NULL; hWnd = ::GetParent( hWnd ) )
	  {
      CWnd*   pWnd = GLOBAL::CWND::GetHelper()->FindEntry( hWnd );
      if ( pWnd )
      {
        if ( pWnd->PreTranslateMessage( pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, lResult ) )
        {
          return TRUE;
        }
      }
	  }
	  return FALSE;       // no special processing

  }



  CWnd::CWnd()
    : m_hWnd( NULL ),
      m_hwndOwner( NULL ),
      m_oldWndProc( NULL ),
      m_bAttached( false ),
      m_bModal( false ),
      m_bSubclassed( false )
  {
  }



  CWnd::~CWnd()
  {

    if ( ( !m_bAttached )
    &&   ( m_hWnd ) )
    {
      DestroyWindow();
    }
    HWND    hwndOld = Detach();
    if ( hwndOld )
    {
      GLOBAL::CWND::GetHelper()->RemoveWindow( hwndOld );
    }

  }



  BOOL CWnd::Attach( HWND hWndNew )
  {

    if ( IsWindow() )
    {
      if ( m_bAttached )
      {
        SetWindowLongPtr( GWLP_WNDPROC, (LONG_PTR)m_oldWndProc );

        GLOBAL::CWND::GetHelper()->RemoveWindow( m_hWnd );
      }
    }
    m_hWnd = hWndNew;
    if ( IsWindow() )
    {
      CWnd*   pWndDummy = GLOBAL::CWND::GetHelper()->FindEntry( m_hWnd );

      if ( pWndDummy )
      {
        // already attached!
        /*
        m_oldWndProc = (WNDPROC)GetWindowLong( m_hWnd, GWL_WNDPROC );
        SetWindowLong( m_hWnd, GWL_WNDPROC, (LONG)MYHELPERWINDOWPROC );
        */
      }
      else
      {
        m_oldWndProc = (WNDPROC)GetWindowLongPtr( GWLP_WNDPROC );
        SetWindowLongPtr( GWLP_WNDPROC, (LONG_PTR)MYHELPERWINDOWPROC );

        GLOBAL::CWND::GetHelper()->InsertWindow( m_hWnd, this );
        m_bAttached = true;
        OnAttach();
      }
    }
    return TRUE;

  }



  HWND CWnd::Detach()
  {

    if ( m_hWnd == NULL )
    {
      return NULL;
    }

    if ( !m_bAttached )
    {
      return NULL;
    }

    HWND    hwndDummy = m_hWnd;
    if ( IsWindow() )
    {
      if ( m_oldWndProc )
      {
        SetWindowLongPtr( GWLP_WNDPROC, (LONG_PTR)m_oldWndProc );
      }
    }

    GLOBAL::CWND::GetHelper()->RemoveWindow( m_hWnd );
    m_hWnd = NULL;
    m_bAttached = false;

    return hwndDummy;

  }



  BOOL CWnd::PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {

    return FALSE;

  }



  BOOL CWnd::PostTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {
    if ( m_hWnd == NULL )
    {
      return FALSE;
    }

    if ( hwnd != m_hWnd )
    {
      return FALSE;
    }

    if ( uMsg == WM_NCDESTROY )
    {
      Detach();
      GLOBAL::CWND::GetHelper()->RemoveWindow( hwnd );
      if ( m_oldWndProc )
      {
        SetWindowLongPtr( GWLP_WNDPROC, (LONG_PTR)m_oldWndProc );
        m_oldWndProc = NULL;
      }
    }

    return FALSE;
  }



  void CWnd::GetClientRect( LPRECT lpRect ) const
  {
    ::GetClientRect( m_hWnd, lpRect );
  }



  void CWnd::GetWindowRect( LPRECT lpRect ) const
  {
    ::GetWindowRect( m_hWnd, lpRect );
  }



  GR::u32 CWnd::GetStyle() const
  {

    return (GR::u32)GetWindowLongPtr( GWL_STYLE );
 

  }



  GR::u32 CWnd::GetExStyle() const
  {

    return (GR::u32)GetWindowLongPtr( GWL_EXSTYLE );
 

  }



  LRESULT CWnd::SendMessage( UINT message, WPARAM wParam, LPARAM lParam )
  {
    return ::SendMessageW( m_hWnd, message, wParam, lParam );
  }



  LRESULT CWnd::SendDlgItemMessage( int nID, UINT message, WPARAM wParam, LPARAM lParam )
  {

    return ::SendDlgItemMessage( m_hWnd, nID, message, wParam, lParam );

  }



  BOOL CWnd::PostMessage( UINT message, WPARAM wParam, LPARAM lParam )
  {
    return ::PostMessage( m_hWnd, message, wParam, lParam );
  }



  BOOL CWnd::ModifyStyle( GR::u32 dwRemove, GR::u32 dwAdd, UINT nFlags )
  {

    GR::u32   dwStyle = GetStyle();

    dwStyle &= ~dwRemove;
    dwStyle |= dwAdd;

    SetWindowLongPtr( GWL_STYLE, dwStyle );

    if ( nFlags )
    {
      ::SetWindowPos( m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | nFlags );
    }
    return TRUE;

  }



  BOOL CWnd::ModifyStyleEx( GR::u32 dwRemove, GR::u32 dwAdd, UINT nFlags )
  {

    GR::u32   dwStyle = GetExStyle();

    dwStyle &= ~dwRemove;
    dwStyle |= dwAdd;

    SetWindowLongPtr( GWL_EXSTYLE, dwStyle );

    if ( nFlags )
    {
      ::SetWindowPos( m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE | nFlags );
    }
    return TRUE;

  }



  void CWnd::Invalidate( BOOL bErase )
  {

    ::InvalidateRect( m_hWnd, NULL, bErase );

  }



  void CWnd::InvalidateRect( LPCRECT lpRect, BOOL bErase )
  {

    ::InvalidateRect( m_hWnd, lpRect, bErase );

  }



  void CWnd::ValidateRect( LPCRECT lpRect )
  {

    ::ValidateRect( m_hWnd, lpRect );

  }



  HWND CWnd::SetFocus()
  {

    return ::SetFocus( m_hWnd );

  }



  void CWnd::GetScrollRange( int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
  {

    ::GetScrollRange( m_hWnd, nBar, lpMinPos, lpMaxPos );

  }



  int CWnd::SetScrollPos( int nBar, int nPos, BOOL bRedraw )
  {

    return ::SetScrollPos( m_hWnd, nBar, nPos, bRedraw );

  }



  void CWnd::SetScrollRange( int nBar, int nMinPos, int nMaxPos, BOOL bRedraw )
  {

    ::SetScrollRange( m_hWnd, nBar, nMinPos, nMaxPos, bRedraw );

  }



  BOOL CWnd::EnableScrollBar( int nSBFlags, UINT nArrowFlags )
  {

    return ::EnableScrollBar( m_hWnd, nSBFlags, nArrowFlags );

  }



  void CWnd::ShowScrollBar(UINT nBar, BOOL bShow )
  {

    ::ShowScrollBar( m_hWnd, nBar, bShow );

  }



  void CWnd::SetFont( HFONT hFont, BOOL bRedraw )
  {
    ::SendMessageW( m_hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM( bRedraw, 0 ) );
  }



  HFONT CWnd::GetFont() const
  {
    return (HFONT)::SendMessageW( m_hWnd, WM_GETFONT, 0, 0 );
  }



  BOOL CWnd::LockWindowUpdate()
  {

    return ::LockWindowUpdate( m_hWnd );

  }



  void CWnd::UnlockWindowUpdate()
  {

    ::LockWindowUpdate( NULL );

  }



  BOOL CWnd::EnableWindow( BOOL bEnable /*=TRUE*/ )
  {

    return ::EnableWindow( m_hWnd, bEnable );

  }



  BOOL CWnd::ShowWindow( int nCmdShow )
  {

    return ::ShowWindow( m_hWnd, nCmdShow );

  }



  void CWnd::MoveWindow( int x, int y, BOOL bRepaint /*= TRUE */ )
  {

    RECT    rc;
    ::GetWindowRect( m_hWnd, &rc );
    ::MoveWindow( m_hWnd, x, y, rc.right - rc.left, rc.bottom - rc.top, bRepaint );

  }



  void CWnd::MoveWindow( int x, int y, int nWidth, int nHeight, BOOL bRepaint /* = TRUE*/ )
  {

    ::MoveWindow( m_hWnd, x, y, nWidth, nHeight, bRepaint );

  }



  void CWnd::MoveWindow( const RECT* pRect, BOOL bRepaint )
  {

    ::MoveWindow( m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, bRepaint );

  }



  void CWnd::SetWindowText( LPCSTR lpszString )
  {
    ::SetWindowTextW( m_hWnd, GR::Convert::ToUTF16( lpszString ).c_str() );
  }



  GR::String CWnd::GetWindowText() const
  {
    int   iLength = (int)::SendMessageW( m_hWnd, WM_GETTEXTLENGTH, 0, 0 );

    GR::WChar* pDummy = new GR::WChar[iLength + 3];

    ::SendMessageW( m_hWnd, WM_GETTEXT, iLength + 1, (LPARAM)pDummy );

    GR::String   strResult = GR::Convert::ToUTF8( pDummy );
    delete[] pDummy;

    return strResult;
  }



  int CWnd::GetWindowText( GR::Char* lpszStringBuf, int nMaxCount ) const
  {
    GR::String      caption = GetWindowText();

    strcpy_s( lpszStringBuf, nMaxCount, caption.c_str() );

    return (int)caption.length();
  }



  int CWnd::GetWindowTextLength() const
  {

    return ::GetWindowTextLength( m_hWnd );

  }



  int CWnd::GetDlgCtrlID() const
  {

    return ::GetDlgCtrlID( m_hWnd );

  }



  HWND CWnd::GetOwner() const
  {

    if ( m_hwndOwner )
    {
      return m_hwndOwner;
    }
    return GetParent();

  }



  void CWnd::SetOwner( HWND hwndOwner )
  {

    m_hwndOwner = hwndOwner;

  }



  void CWnd::CenterWindow( HWND hwndAlternateParent )
  {

    RECT    myRect;


    GetWindowRect( &myRect );

    if ( hwndAlternateParent )
    {
      RECT    rcParent;


      ::GetWindowRect( hwndAlternateParent, &rcParent );

      MoveWindow( rcParent.left + ( ( rcParent.right - rcParent.left ) - ( myRect.right - myRect.left ) ) / 2,
                  rcParent.top + ( ( rcParent.bottom - rcParent.top ) - ( myRect.bottom - myRect.top ) ) / 2 );
      return;
    }
    HWND    hwndParent = GetParent();
    if ( ::IsWindow( hwndParent ) )
    {
      RECT    rcParent;


      ::GetWindowRect( hwndParent, &rcParent );

      MoveWindow( rcParent.left + ( ( rcParent.right - rcParent.left ) - ( myRect.right - myRect.left ) ) / 2,
                  rcParent.top + ( ( rcParent.bottom - rcParent.top ) - ( myRect.bottom - myRect.top ) ) / 2 );
      return;
    }
    MoveWindow( ( GetSystemMetrics( SM_CXSCREEN ) - ( myRect.right - myRect.left ) ) / 2,
                ( GetSystemMetrics( SM_CYSCREEN ) - ( myRect.bottom - myRect.top ) ) / 2 );

  }



  void CWnd::CenterWindow()
  {

    RECT    myRect;


    GetWindowRect( &myRect );

    MoveWindow( ( GetSystemMetrics( SM_CXVIRTUALSCREEN ) - ( myRect.right - myRect.left ) ) / 2,
                ( GetSystemMetrics( SM_CYVIRTUALSCREEN ) - ( myRect.bottom - myRect.top ) ) / 2 );

  }



  BOOL CWnd::Create( LPCSTR lpszClassName,
                     LPCSTR lpszWindowName,
                     GR::u32 dwStyle,
                     const RECT& rect,
                     HWND hwndParent,
                     UINT_PTR nID,
                     LPVOID lpParam )
  {
    //CWnd::m_pWndCreatingNow = this;
    *GetCreatingWindow() = this;

    m_hWnd = ::CreateWindowW( GR::Convert::ToUTF16( lpszClassName ).c_str(),
                              ( lpszWindowName == NULL ) ? L"" : GR::Convert::ToUTF16( lpszWindowName ).c_str(),
                              dwStyle,
                              rect.left, rect.top,
                              rect.right - rect.left,
                              rect.bottom - rect.top,
                              hwndParent,
                              HMENU( nID ),
                              (HINSTANCE)GetModuleHandle( NULL ),
                              lpParam );
    if ( m_hWnd )
    {
      if ( hwndParent )
      {
        m_hwndOwner = hwndParent;
      }
      return TRUE;
    }

    return FALSE;

  }



  BOOL CWnd::CreateEx( GR::u32 dwExStyle, 
                       LPCSTR lpszClassName,
                       LPCSTR lpszWindowName,
                       GR::u32 dwStyle,
                       int x, int y, int nWidth, int nHeight,
                       HWND hwndParent,
                       UINT_PTR nIDorHMenu,
                       LPVOID lpParam )
  {

    //CWnd::m_pWndCreatingNow = this;
    *GetCreatingWindow() = this;

    m_hWnd = ::CreateWindowExW( dwExStyle,
                                GR::Convert::ToUTF16( lpszClassName ).c_str(),
                                ( lpszWindowName == NULL ) ? L"" : GR::Convert::ToUTF16( lpszWindowName ).c_str(),
                                dwStyle,
                                x, y,
                                nWidth, nHeight,
                                hwndParent,
                                (HMENU)nIDorHMenu,
                                (HINSTANCE)GetModuleHandle( NULL ),
                                lpParam );
    if ( m_hWnd )
    {
      if ( hwndParent )
      {
        m_hwndOwner = hwndParent;
      }
      /*
      GLOBAL::CWND::mapCWnds[m_hWnd] = this;
      m_oldWndProc = (WNDPROC)GetWindowLong( m_hWnd, GWL_WNDPROC );
      SetWindowLong( m_hWnd, GWL_WNDPROC, (LONG)MYHELPERWINDOWPROC );
      */
      return TRUE;
    }
    return FALSE;

  }



  BOOL CWnd::CreateEx( GR::u32 dwExStyle, 
                       LPCSTR lpszClassName,
                       LPCSTR lpszWindowName,
                       GR::u32 dwStyle,
                       const RECT& rect,
                       HWND hwndParent,
                       UINT_PTR nIDorHMenu,
                       LPVOID lpParam )
  {

    //CWnd::m_pWndCreatingNow = this;
    *GetCreatingWindow() = this;
    m_hWnd = ::CreateWindowExW( dwExStyle,
                                GR::Convert::ToUTF16( lpszClassName ).c_str(),
                                ( lpszWindowName == NULL ) ? L"" : GR::Convert::ToUTF16( lpszWindowName ).c_str(),
                                dwStyle,
                                rect.left, rect.top,
                                rect.right - rect.left,
                                rect.bottom - rect.top,
                                hwndParent,
                                (HMENU)nIDorHMenu,
                                (HINSTANCE)GetModuleHandle( NULL ),
                                lpParam );

    if ( m_hWnd )
    {
      if ( hwndParent )
      {
        m_hwndOwner = hwndParent;
      }
      /*
      GLOBAL::CWND::mapCWnds[m_hWnd] = this;
      m_oldWndProc = (WNDPROC)GetWindowLong( m_hWnd, GWL_WNDPROC );
      SetWindowLong( m_hWnd, GWL_WNDPROC, (LONG)MYHELPERWINDOWPROC );
      */
      return TRUE;
    }
    return FALSE;

  }



  HWND CWnd::SetCapture()
  {

    return ::SetCapture( m_hWnd );

  }



  void CWnd::UpdateWindow()
  {

    ::UpdateWindow( m_hWnd );

  }



  void CWnd::ClientToScreen( LPRECT lpRect ) const
  {

    POINT   ptLO,
            ptRU;

    ptLO.x = lpRect->left;
    ptLO.y = lpRect->top;
    ptRU.x = lpRect->right;
    ptRU.y = lpRect->bottom;
    ::ClientToScreen( m_hWnd, &ptLO );
    ::ClientToScreen( m_hWnd, &ptRU );

    lpRect->left    = ptLO.x;
    lpRect->top     = ptLO.y;
    lpRect->right   = ptRU.x;
    lpRect->bottom  = ptRU.y;

  }



  void CWnd::ScreenToClient( LPRECT lpRect ) const
  {

    POINT   ptLO,
            ptRU;

    ptLO.x = lpRect->left;
    ptLO.y = lpRect->top;
    ptRU.x = lpRect->right;
    ptRU.y = lpRect->bottom;
    ::ScreenToClient( m_hWnd, &ptLO );
    ::ScreenToClient( m_hWnd, &ptRU );

    lpRect->left    = ptLO.x;
    lpRect->top     = ptLO.y;
    lpRect->right   = ptRU.x;
    lpRect->bottom  = ptRU.y;

  }



  void CWnd::ScreenToClient( POINT* pPoint ) const
  {

    ::ScreenToClient( m_hWnd, pPoint );

  }



  void CWnd::ClientToScreen( POINT* pPoint ) const
  {

    ::ClientToScreen( m_hWnd, pPoint );

  }



  bool CWnd::IsWindow() const
  {

    if ( ::IsWindow( m_hWnd ) )
    {
      return true;
    }
    return false;

  }



  bool CWnd::IsIconic() const
  {

    return !!::IsIconic( m_hWnd );

  }



  bool CWnd::IsZoomed() const
  {

    return !!::IsZoomed( m_hWnd );

  }



  BOOL CWnd::IsWindowVisible() const
  {

    return ::IsWindowVisible( m_hWnd );

  }



  void CWnd::CloseWindow()
  {

    if ( m_hWnd )
    {
      ::CloseWindow( m_hWnd );
    }

  }



  BOOL CWnd::DestroyWindow()
  {

    if ( m_hWnd )
    {
      ::DestroyWindow( m_hWnd );
      m_hWnd = NULL;
    }

    return TRUE;
  }



  LRESULT CWnd::Default()
  {

    if ( ( m_oldWndProc )
    &&   ( m_oldWndProc != (WNDPROC)MYHELPERWINDOWPROC ) )
    {
      LRESULT lRes = CallWindowProc( m_oldWndProc, m_LastMsg.hwnd, m_LastMsg.message, m_LastMsg.wParam, m_LastMsg.lParam );
      PostTranslateMessage( m_LastMsg.hwnd, m_LastMsg.message, m_LastMsg.wParam, m_LastMsg.lParam, lRes );
      return lRes;
    }

    return ::DefWindowProc( m_LastMsg.hwnd, m_LastMsg.message, m_LastMsg.wParam, m_LastMsg.lParam );

  }



  BOOL CWnd::GetUpdateRect( LPRECT lpRect, BOOL bErase )
  {

    return ::GetUpdateRect( m_hWnd, lpRect, bErase );

  }



  void CWnd::EndModalLoop( int nResult )
  {

    if ( m_bModal )
    {
      m_iModalResult = nResult;
      m_bModal = false;
      PostMessage( WM_NULL );
    }

  }



  BOOL CWnd::SetWindowPos( HWND hwndAfter, int x, int y, int cx, int cy, UINT nFlags )
  {

    return ::SetWindowPos( m_hWnd, hwndAfter, x, y, cx, cy, nFlags );

  }



  BOOL CWnd::CreateDlgIndirect( LPCDLGTEMPLATE lpDialogTemplate, HWND hwndParent, HINSTANCE hInst )
  {

    *GetCreatingWindow() = this;

    m_bSubclassed = true;

    m_hWnd = ::CreateDialogIndirect( hInst, lpDialogTemplate, hwndParent, MYHELPERDIALOGPROC );

    if ( m_hWnd )
    {
      return TRUE;
    }
    return FALSE;

  }



  HWND CWnd::GetDlgItem( int nID ) const
  {

    return ::GetDlgItem( m_hWnd, nID );

  }



  int CWnd::MessageBox( LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType )
  {

    return ::MessageBox( m_hWnd, lpszText, lpszCaption, nType );

  }



  HWND CWnd::SetActiveWindow()
  {

    return (HWND)::SetActiveWindow( m_hWnd );

  }



  HWND CWnd::GetActiveWindow()
  {

    return (HWND)::GetActiveWindow();

  }



  BOOL CWnd::SetForegroundWindow()
  {

    return ::SetForegroundWindow( m_hWnd );

  }

  HWND CWnd::GetForegroundWindow()
  {

    return (HWND)::GetForegroundWindow();

  }



  HWND CWnd::GetParent() const
  {

    return ::GetParent( m_hWnd );

  }



  void CWnd::SetDlgItemText( int nID, LPCTSTR lpszString )
  {

    ::SetDlgItemText( m_hWnd, nID, lpszString );

  }



  void CWnd::SetDlgItemText( int nID, const GR::String& Text )
  {
    ::SetDlgItemTextW( m_hWnd, nID, GR::Convert::ToUTF16( Text ).c_str() );
  }



  void CWnd::SetParent( HWND hwndParent )
  {
    ::SetParent( m_hWnd, hwndParent );
  }



  void CWnd::DragAcceptFiles( BOOL bAccept )
  {
    ::DragAcceptFiles( m_hWnd, bAccept );
  }



  UINT_PTR CWnd::SetTimer( UINT nIDEvent, UINT nElapse, TIMERPROC lpfnTimer )
  {
    return ::SetTimer( m_hWnd, nIDEvent, nElapse, lpfnTimer );
  }



  BOOL CWnd::KillTimer( int nIDEvent )
  {

    return ::KillTimer( m_hWnd, nIDEvent );

  }



  HDC CWnd::GetDC()
  {

    return ::GetDC( m_hWnd );

  }



  int CWnd::ReleaseDC( HDC hdc )
  {

    return ::ReleaseDC( m_hWnd, hdc );

  }



  LRESULT CWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    if ( ( m_oldWndProc != NULL )
    &&   ( m_oldWndProc != (WNDPROC)MYHELPERWINDOWPROC )
    &&   ( m_oldWndProc != ::DefWindowProc ) )
    {
      LRESULT lRes = CallWindowProc( m_oldWndProc, m_hWnd, uMsg, wParam, lParam );
      PostTranslateMessage( m_hWnd, uMsg, wParam, lParam, lRes );

      if ( uMsg == WM_NCDESTROY )
      {
        m_hWnd = NULL;
      }
      return lRes;
    }


    LRESULT lRes = ::DefWindowProc( m_hWnd, uMsg, wParam, lParam );
    PostTranslateMessage( m_hWnd, uMsg, wParam, lParam, lRes );

    if ( uMsg == WM_NCDESTROY )
    {
      m_hWnd = NULL;
    }

    return lRes;

  }


  CWnd** CWnd::GetCreatingWindow()
  {
    GR::Char   szBuffer[MAX_PATH];

    if ( GetEnvironmentVariableA( "GRCWnd-CreatingWnd", szBuffer, MAX_PATH ) )
    {
      return (CWnd**)_atoi64( szBuffer );
    }

    sprintf_s( szBuffer, MAX_PATH, "%lld", (long long)&m_pWndCreatingNow );
    SetEnvironmentVariableA( "GRCWnd-CreatingWnd", szBuffer );

    return &m_pWndCreatingNow;
  }



  LONG_PTR CWnd::SetWindowLongPtr( int nIndex, LONG_PTR dwNewLong )
  {

    #pragma warning( disable : 4244 )
    return (LONG_PTR)(LONG)::SetWindowLongPtr( m_hWnd, nIndex, (LONG_PTR)dwNewLong );
    #pragma warning( default : 4244 )


  }



  LONG_PTR CWnd::GetWindowLongPtr( int nIndex ) const
  {

    return ::GetWindowLongPtr( m_hWnd, nIndex );

  }



  BOOL CWnd::IsWindowEnabled() const
  {

    return ::IsWindowEnabled( m_hWnd );

  }



  void CWnd::OnAttach()
  {
  }



  HWND CWnd::GetNextDlgTabItem( HWND hwndParent, BOOL bPrevious ) const
  {

    return ::GetNextDlgTabItem( hwndParent, m_hWnd, bPrevious );

  }



  int CWnd::SetWindowRgn( HRGN hRgn, BOOL bRedraw )
  {

    return ::SetWindowRgn( m_hWnd, hRgn, bRedraw ); 

  }



  int CWnd::GetWindowRgn( HRGN hRgn ) const
  {
    return ::GetWindowRgn( m_hWnd, hRgn );
  }


  void CWnd::BringWindowToTop()
  {

    ::BringWindowToTop( m_hWnd );

  }



  HMODULE CWnd::LoadLibrary( LPCTSTR lpLibFileName )
  {

    HMODULE   hModLib = ::LoadLibrary( lpLibFileName );
    if ( hModLib )
    {
      GLOBAL::CWND::GetHelper()->mapDlls.insert( hModLib, hModLib );
    }
    return hModLib;

  }



  BOOL CWnd::FreeLibrary( HMODULE hLibModule )
  {

    GR::map<HMODULE,HMODULE>::tNode*    pNode = GLOBAL::CWND::GetHelper()->mapDlls.find( hLibModule );
    if ( pNode )
    {
      GLOBAL::CWND::GetHelper()->mapDlls.erase( pNode );
    }

    return ::FreeLibrary( hLibModule );

  }



  HINSTANCE CWnd::FindResourceHandle( LPCSTR lpszName, LPCTSTR lpszType )
  {
    GLOBAL::CWND::CWndHelper*   pHelper = GLOBAL::CWND::GetHelper();

    GR::WString      utf16String;
    LPCWSTR           resourceType = 0;

    if ( IS_INTRESOURCE( lpszType ) )
    {
      resourceType = (LPCWSTR)lpszType;
    }
    else
    {
      utf16String = GR::Convert::ToUTF16( lpszType );
      resourceType = utf16String.c_str();
    }

    GR::map<HMODULE,HMODULE>::tNode*    pNode = pHelper->mapDlls.m_pTail;

    while ( pNode )
    {
      HMODULE hInstDLL = pNode->Key;

      if ( IS_INTRESOURCE( lpszName ) )
      {
        if ( ::FindResourceW( hInstDLL, (LPCWSTR)lpszName, resourceType ) )
        {
          return hInstDLL;
        }
      }
      else
      {
        if ( ::FindResourceW( hInstDLL, GR::Convert::ToUTF16( lpszName ).c_str(), resourceType ) )
        {
          return hInstDLL;
        }
      }
      pNode = pNode->pPrev;
    }

    HINSTANCE hInst = GetModuleHandleW( NULL );

    if ( IS_INTRESOURCE( lpszName ) )
    {
      if ( ::FindResourceW( hInst, (LPCWSTR)lpszName, resourceType ) )
      {
        return hInst;
      }
    }
    else
    {
      if ( ::FindResourceW( hInst, GR::Convert::ToUTF16( lpszName ).c_str(), resourceType ) )
      {
        return hInst;
      }
    }
    return NULL;
  }



  void CWnd::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
  {
  }



  void CWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) 
  {
  }


  void CWnd::OnClose()
  {
  }

  void CWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
  {
  }

  void CWnd::OnSize( UINT nType, int cx, int cy )
  {
  }



  void CWnd::OnEnable( BOOL bEnable )
  {
  }



  void CWnd::OnDestroy()
  {
  }

  void CWnd::OnTimer( UINT_PTR nIDEvent )
  {
  }

  void CWnd::OnMouseMove( UINT nFlags, POINT point )
  {
  }

  void CWnd::OnLButtonUp( UINT nFlags, POINT point )
  {
  }

  void CWnd::OnLButtonDown( UINT nFlags, POINT point )
  {
  }

  void CWnd::OnNcLButtonDown( UINT nHitTest, POINT point )
  {
  }

  void CWnd::OnNcLButtonDblClk( UINT nHitTest, POINT point )
  {
  }

  BOOL CWnd::OnInitDialog()
  {

    return FALSE;

  }



  HICON CWnd::SetIcon( HICON hIcon, BOOL bBigIcon )
  {
    return (HICON)::SendMessageW( m_hWnd, WM_SETICON, bBigIcon ? ICON_BIG : ICON_SMALL, (LPARAM)hIcon );
  }



  void CWnd::CreateSolidCaret( int nWidth, int nHeight )
  {

    ::CreateCaret( m_hWnd, (HBITMAP)0, nWidth, nHeight);

  }



  void CWnd::HideCaret()
  {

    ::HideCaret( m_hWnd );

  }



  void PASCAL CWnd::SetCaretPos( POINT point )
  {

    ::SetCaretPos( point.x, point.y );

  }



  void CWnd::ShowCaret()
  {

    ::ShowCaret( m_hWnd );

  }



  BOOL CWnd::OpenClipboard()
  {

    return ::OpenClipboard( m_hWnd );

  }



  HMENU CWnd::GetMenu() const
  {

    return ::GetMenu( m_hWnd );

  }


  HWND CWnd::GetCapture()
  {

    return ::GetCapture();

  }



  HWND CWnd::GetFocus()
  {
    return ::GetFocus();
  }



  HWND CWnd::GetWindow( UINT nCmd ) const
  {
    return (HWND)::GetWindow( m_hWnd, nCmd );
  }



  void CWnd::SetWindowText( GR::u32 dwResourceID )
  {
    if ( dwResourceID == 0 )
    {
      SetWindowText( Win::Util::LoadString( GetDlgCtrlID() ) );
    }
    else
    {
      SetWindowText( Win::Util::LoadString( dwResourceID ) );
    }
  }



  void CWnd::SetDefaultTextForAllControls()
  {
    HWND    hwndChild = GetWindow( GW_CHILD );
    while ( hwndChild )
    {
      // alle Childs durchackern
      GR::u32   dwID = ::GetDlgCtrlID( hwndChild );
      if ( dwID != -1 )
      {
        ::SetWindowTextW( hwndChild, GR::Convert::ToUTF16( Win::Util::LoadString( ::GetDlgCtrlID( hwndChild ) ) ).c_str() );
      }

      hwndChild = ::GetWindow( hwndChild, GW_HWNDNEXT );
    }
  }



  CWnd* CWnd::FromHandle( HWND hwndChild )
  {
    return GLOBAL::CWND::GetHelper()->FindEntry( hwndChild );
  }

}
