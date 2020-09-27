// BasicToolDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include <MFC\BasicToolDlg.h>

#include <Misc/Misc.h>

#include <debug\debugclient.h>

#pragma warning( disable:4786 )
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CBasicToolDlg 

/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

namespace BASICTOOLDLG
{
  std::set<HWND>    gSetToolDlgs;

  bool              gMainWndInserted = false;

  WNDPROC           gOldMainframeProc;

  static LRESULT AfxMainframeProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

  void InsertWindow( HWND hWnd )
  {
    char    szDummy[MAX_PATH];
    GetWindowText( hWnd, szDummy, MAX_PATH );
    gSetToolDlgs.insert( hWnd );
  }

  void RemoveWindow( HWND hWnd )
  {
    std::set<HWND>::iterator   it( gSetToolDlgs.begin() );
    while ( it != gSetToolDlgs.end() )
    {
      if ( *it == hWnd )
      {
        gSetToolDlgs.erase( it );
        break;
      }

      ++it;
    }
    if ( gSetToolDlgs.size() == 1 )
    {
      gSetToolDlgs.clear();
    }
  }

  static LRESULT AfxMainframeProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
  {
    LRESULT   lRes = CallWindowProc( gOldMainframeProc, hWnd, msg, wParam, lParam );
    if ( msg == WM_NCACTIVATE )
    {
      HandleNCActivate( hWnd, hWnd, wParam, lParam, ::DefWindowProc );
    }
    if ( msg == WM_ACTIVATE )
    {
      HandleActivate( hWnd, hWnd, wParam, lParam, ::DefWindowProc );
    }
    return lRes;
  }
}



LRESULT HandleNCActivate( HWND hwndMain, HWND hWnd, WPARAM wParam, LPARAM lParam, WNDPROC wndProc )
{
  HWND   hParam = (HWND)lParam;

  BOOL   fKeepActive = wParam;
  BOOL   fSyncOthers = TRUE;


  // UNDOCUMENTED FEATURE: 
  // If the other window being activated/deactivated 
  // (i.e. NOT this one) is one of our popups, then go (or stay) active.
  std::set<HWND>::iterator   it( BASICTOOLDLG::gSetToolDlgs.begin() );
  while ( it != BASICTOOLDLG::gSetToolDlgs.end() )
  {
    if ( hParam == *it )
    {
      fKeepActive = TRUE;
      fSyncOthers = FALSE;
      if ( hParam == hwndMain )
      {
        fSyncOthers = TRUE;
      }
      break;
    }
    ++it;
  }

  // If this message was sent by the synchronise-loop (below)
  // then exit normally. If we don't do this, there will be an infinite loop!
  if ( hParam == (HWND)-1 )
  {
    return DefWindowProc( hWnd, WM_NCACTIVATE, fKeepActive, 0 );
  }

  // This window is about to change (inactive/active).
  // Sync all other popups to the same state 
  if ( fSyncOthers == TRUE )
  {
    if ( hwndMain != hWnd )
    {
      //dh::Log( "Sync ausgehend von NICHTmain (%x)\n", (DWORD)hWnd );
    }
    std::set<HWND>::iterator   it( BASICTOOLDLG::gSetToolDlgs.begin() );
    while ( it != BASICTOOLDLG::gSetToolDlgs.end() )
    {
      //DO NOT send this message to ourselves!!!!
      if( *it != hWnd && *it != hParam )
      {
        ::SendMessage( *it, WM_NCACTIVATE, fKeepActive, (LONG)-1 );
      }
      ++it;
    }
  }

  return DefWindowProc( hWnd, WM_NCACTIVATE, fKeepActive, lParam);
}

LRESULT HandleActivate( HWND hwndMain, HWND hWnd, WPARAM wParam, LPARAM lParam, WNDPROC wndProc )
{

  BOOL fKeepActive = (wParam != WA_INACTIVE);

  if ( fKeepActive == FALSE )
  {
    std::set<HWND>::iterator   it( BASICTOOLDLG::gSetToolDlgs.begin() );
    while ( it != BASICTOOLDLG::gSetToolDlgs.end() )
    {
      if ( *it == (HWND)lParam )
      {
        //return CallWindowProc( wndProc, hWnd, WM_ACTIVATE, wParam, lParam );
        return DefWindowProc( hWnd, WM_ACTIVATE, wParam, lParam );
      }
      ++it;
    }
  }

  std::set<HWND>::iterator   it( BASICTOOLDLG::gSetToolDlgs.begin() );
  while ( it != BASICTOOLDLG::gSetToolDlgs.end() )
  {
    char    szDummy[MAX_PATH];
    GetWindowText( *it, szDummy, MAX_PATH );
    ::SendMessage( *it, WM_NCACTIVATE, fKeepActive, 0 );
    ++it;
  }
  return DefWindowProc( hWnd, WM_ACTIVATE, wParam, lParam);
}


/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CBasicToolDlg::CBasicToolDlg(DWORD dwID, CWnd* pParent /*=NULL*/)
	: CDialog(dwID, pParent)
{

  m_bMinimized  = FALSE;
  m_iYOffset    = 0;
  m_hIcon       = NULL;
  m_hMenu       = NULL;
	//{{AFX_DATA_INIT(CBasicToolDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}




/*-DoDataExchange-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBasicToolDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBasicToolDlg, CDialog)
	//{{AFX_MSG_MAP(CBasicToolDlg)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CBasicToolDlg 



/*-OnPaint--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::OnPaint() 
{

  if ( m_bMinimized )
  {
	  CPaintDC dc(this); // device context for painting

    RECT    rc;


    rc = m_rectSmall;
    dc.DrawFrameControl( &rc, DFC_BUTTON, DFCS_BUTTONPUSH );

    dc.SelectObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

    if ( m_hIcon )
    {
      RECT    rc2 = rc;

      rc2.top = ( rc.bottom - rc.top - 16 ) / 2;
      rc2.bottom = rc.bottom - rc2.top;
      rc2.left += 2;
      DrawIconEx( dc.m_ps.hdc,
                  rc2.left,
                  rc2.top,
                  m_hIcon,
                  16,
                  16,
                  0,
                  NULL,
                  DI_NORMAL );
    }

    CString cstrText;
    
    GetWindowText( cstrText );

    rc.left += 20;
    InflateRect( &rc, -4, -4 );
    dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
    dc.SetBkColor( GetSysColor( COLOR_WINDOW ) );
    dc.SetBkMode( TRANSPARENT );
    dc.DrawText( cstrText, &rc, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOCLIP );
  }
  else
  {
    Default();
  }

}



/*-OnCreate-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CBasicToolDlg::OnCreate( LPCREATESTRUCT lpCreateStruct ) 
{ 

	if ( CDialog::OnCreate(lpCreateStruct) == -1 )
  {
		return -1;
  }

  BASICTOOLDLG::InsertWindow( GetSafeHwnd() );

  //ModifyStyle( 0xffffffff, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_CAPTION );

  m_dwMaximizedStyle    = GetStyle();
  m_dwMaximizedStyleEx  = GetExStyle();

  Minimize();

  if ( !BASICTOOLDLG::gMainWndInserted )
  {
    HWND    hwndMainFrame = GetTopLevelParent()->GetSafeHwnd();
    if ( hwndMainFrame )
    {
      BASICTOOLDLG::gMainWndInserted = true;
      BASICTOOLDLG::gSetToolDlgs.insert( hwndMainFrame );
      BASICTOOLDLG::gOldMainframeProc = (WNDPROC)SetWindowLong( hwndMainFrame, GWL_WNDPROC, (LONG)BASICTOOLDLG::AfxMainframeProc );
    }
  }


	return 0;
}



/*-OnLButtonDown--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::OnLButtonDown( UINT nHitTest, CPoint point ) 
{

  Restore();
	
	CDialog::OnLButtonDown(nHitTest, point);
}



/*-Minimize-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::Minimize()
{

  if ( m_bMinimized )
  {
    return;
  }

  m_dwMaximizedStyle    = GetStyle();
  m_dwMaximizedStyleEx  = GetExStyle();
  if ( GetMenu() )
  {
    m_hMenu               = GetMenu()->GetSafeHmenu();
  }
  ::SetMenu( m_hWnd, NULL );

  GetWindowRect( &m_rectOrig );

  m_bMinimized = TRUE;
  ModifyStyle( WS_CAPTION | WS_THICKFRAME, 0 );

  m_rectSmall.left = m_rectOrig.right - m_rectOrig.left;
  m_rectSmall.top = 0;
  m_rectSmall.right = m_rectSmall.left + 100;
  m_rectSmall.bottom = m_rectSmall.top + 20;

  HRGN hrgnSmall = CreateRectRgn( m_rectSmall.left, m_rectSmall.top, 
                                  m_rectSmall.left + 100, m_rectSmall.bottom );

  SetWindowRgn( hrgnSmall, TRUE );

  MoveWindow( GetSystemMetrics( SM_CXSCREEN ) - ( m_rectOrig.right - m_rectOrig.left + 100 ), 
              m_iYOffset, 
              m_rectOrig.right - m_rectOrig.left + 100, 
              20 );
  //MoveWindow( GetSystemMetrics( SM_CXSCREEN ) - 100, 0, 100, 20 );asrsd
  Invalidate();

}



/*-Restore--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::Restore()
{

  if ( !m_bMinimized )
  {
    return;
  }

  m_bMinimized = FALSE;
  ModifyStyle( 0, m_dwMaximizedStyle | WS_POPUP );
  ModifyStyleEx( 0, m_dwMaximizedStyleEx );
  SetWindowRgn( NULL, TRUE );
  MoveWindow( &m_rectOrig, TRUE );

  ::SetMenu( m_hWnd, m_hMenu );

  Invalidate();

}



/*-SetRestoredPosition--------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::SetRestoredPosition( int iX, int iY )
{

  m_rectOrig.right -= m_rectOrig.left - iX;
  m_rectOrig.bottom -= m_rectOrig.top - iY;
  m_rectOrig.left = iX;
  m_rectOrig.top  = iY;
  if ( !m_bMinimized )
  {
    MoveWindow( &m_rectOrig, TRUE );
    Invalidate();
  }

}



/*-GetRestoredX---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CBasicToolDlg::GetRestoredX()
{

  return m_rectOrig.left;

}



/*-GetRestoredY---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CBasicToolDlg::GetRestoredY()
{

  return m_rectOrig.top;

}



/*-OnClose--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::OnClose() 
{

	Minimize();

}



/*-SetMinimizedPos------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::SetMinimizedPos( int iY )
{

  m_iYOffset = iY;

}


/*-SetMinimizedPos------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::OnOK() 
{

}



/*-SetMinimizedPos------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::OnCancel() 
{

}



/*-SetIcon--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBasicToolDlg::SetIcon( HICON hIcon )
{

  m_hIcon = hIcon;

}



LRESULT CBasicToolDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

  if ( message == WM_NCACTIVATE )
  {
    return HandleNCActivate( GetParent()->GetSafeHwnd(), GetSafeHwnd(), wParam, lParam, ::DefWindowProc );
  }
  else if ( message == WM_ACTIVATE )
  {
    return HandleActivate( GetParent()->GetSafeHwnd(), GetSafeHwnd(), wParam, lParam, ::DefWindowProc );
  }
  else if ( message == WM_NCPAINT )
  {
  }
  else if ( message == WM_DESTROY )
  {
    BASICTOOLDLG::RemoveWindow( GetSafeHwnd() );
  }
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CBasicToolDlg::OnNcDestroy() 
{

  BASICTOOLDLG::RemoveWindow( GetSafeHwnd() );

	CDialog::OnNcDestroy();
	
	// TODO: Add your message handler code here
	
}



