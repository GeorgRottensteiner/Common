// GRMiniDockFrameWnd.cpp : Implementierungsdatei
//

#include "GRUIDockFrame.h"

#include <debug/debugclient.h>

#include "GRUIControlBar.h"
#include "GRUIDockingManager.h"
#include "GRUIMDIApp.h"

// GRUIDockFrame


GRUIDockFrame::GRUIDockFrame() :
  m_pControlBar( NULL ),
  m_dfPos( DFP_FREE ),
  m_bDragging( false )
{

  m_listControlBars.clear();

}

GRUIDockFrame::~GRUIDockFrame()
{

}


void GRUIDockFrame::NcLButtonDown(UINT nHitTest, POINT point)
{

	// special activation for floating toolbars
  if ( nHitTest == HTCAPTION )
  {
    StartDragging( point );
  }
	//ActivateTopParent();

}



void GRUIDockFrame::StartDragging( POINT point )
{

	RECT rRect = {NULL};
	GetWindowRect( &rRect );

  if ( rRect.left < -30000 )
  {
    // not floating
    m_ptDragOffset.x = 0;
    m_ptDragOffset.y = 0;
  }
  else
  {
    m_ptDragOffset = point;
  }

  m_bDragging = true;
  SetCapture();

}



void GRUIDockFrame::OnSize(UINT nType, int cx, int cy)
{
  if ( m_pControlBar )
  {
    RECT    rc;
    GetClientRect( &rc );
    if ( m_pControlBar->IsFloating() )
    {
      m_pControlBar->MoveWindow( 0, 0, rc.right - rc.left, rc.bottom - rc.top );
    }
    else
    {
      m_pControlBar->SetWindowPos( NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE );
    }
  }
}



void GRUIDockFrame::MouseMove( UINT nFlags, POINT point )
{
  if ( ( m_bDragging )
  &&   ( m_pControlBar ) )
  {
    RECT      rc;
    GetWindowRect( &rc );

    SIZE szSize = { rc.right - rc.left, rc.bottom - rc.top };

    POINT   ptMouse;
    GetCursorPos( &ptMouse );

    POINT   ptPos;

    /*
    ptPos.x = ptMouse.x - m_ptDragOffset.x;
    ptPos.y = ptMouse.y - m_ptDragOffset.y;
    */

    ptPos = ptMouse;

    DWORD   dwDockFlags = GRUIDockingManager::CheckForDockingPosition( m_pControlBar, ptPos );
    if ( dwDockFlags )
    {
      // irgendwo angedockt?
      m_pControlBar->Dock( dwDockFlags, ptPos.x, ptPos.y );
    }
    else
    {
      m_pControlBar->Float( ptPos.x - m_ptDragOffset.x, ptPos.y - m_ptDragOffset.y );
    }
    if ( m_pControlBar->IsFloating() )
    {
      RECT    rcOld;

      GetWindowRect( &rcOld );

      if ( ( rcOld.left != ptPos.x - m_ptDragOffset.x )
      ||   ( rcOld.top != ptPos.y - m_ptDragOffset.y ) )
      {
        SetWindowPos( NULL, ptPos.x - m_ptDragOffset.x, ptPos.y - m_ptDragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
      }
    }
  }
}



BOOL GRUIDockFrame::Create( HWND hwndParent, DWORD dwExStyle, DWORD dwStyle )
{

  RECT    rectDefault;
  SetRect( &rectDefault, 0, 0, 40, 40 );
  return CCustomWnd::CreateEx( dwExStyle, "", dwStyle, rectDefault, hwndParent, 0 );

}



void GRUIDockFrame::NcLButtonDblClk(UINT nHitTest, POINT point )
{
  if ( ( m_dfPos == DFP_FREE )
  &&   ( nHitTest == HTCAPTION ) )
  {
    if ( m_pControlBar )
    {
      //m_pControlBar->ToggleState();
    }
  }
}


void GRUIDockFrame::LButtonUp(UINT nFlags, POINT point)
{
  if ( m_bDragging )
  {
    if ( GetCapture() == GetSafeHwnd() )
    {
      ReleaseCapture();
    }
  }
}



void GRUIDockFrame::InsertControlBar( GRUIControlBar* pBar )
{

  m_listControlBars.push_back( pBar );

}



void GRUIDockFrame::RemoveControlBar( GRUIControlBar* pBar )
{

  //CFrameWnd*    pTopFrame = AfxGetMainWnd()->GetTopLevelFrame();

  RECT    rc;

  GetWindowRect( &rc );
  //pTopFrame->ScreenToClient( &rc );

  std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
  while ( it != m_listControlBars.end() )
  {
    GRUIControlBar*  pThatBar = *it;

    if ( pBar == pThatBar )
    {
      it = m_listControlBars.erase( it );
      break;
    }
    ++it;
  }

}



void GRUIDockFrame::OnSizeParent( RECT& rcMDI )
{

  if ( m_dfPos == DFP_FREE )
  {
    return;
  }

  //AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;

  RECT    rcRemainingSize;
  RECT    rc;
  RECT    rcClientInMDI;

  GetClientRect( &rc );

  rcClientInMDI = rc;
  ClientToScreen( &rcClientInMDI );
  GRUIDockingManager::m_pWndTopLevel->ScreenToClient( &rcClientInMDI );

  CalculateNeededSize( rcRemainingSize );

  int   iRemainingWidth = rcRemainingSize.right - rcRemainingSize.left;
  int   iRemainingHeight = rcRemainingSize.bottom - rcRemainingSize.top;


  switch ( m_dfPos )
  {
    case DFP_TOP:
      SetWindowPos( NULL, rcMDI.left, rcMDI.top, rcMDI.right - rcMDI.left, iRemainingHeight, SWP_NOZORDER );
      break;
    case DFP_BOTTOM:
      SetWindowPos( NULL, rcMDI.left, rcMDI.bottom - iRemainingHeight, rcMDI.right - rcMDI.left, iRemainingHeight, SWP_NOZORDER );
      break;
    case DFP_LEFT:
      SetWindowPos( NULL, rcMDI.left, rcMDI.top, iRemainingWidth, rcMDI.bottom - rcMDI.top, SWP_NOZORDER );
      break;
    case DFP_RIGHT:
      SetWindowPos( NULL, rcMDI.right - iRemainingWidth, rcMDI.top, iRemainingWidth, rcMDI.bottom - rcMDI.top, SWP_NOZORDER );
      break;
  }

  GetClientRect( &rc );

  RECT    rcAvail = rcMDI;

  SIZE    szDockBar;
  
  szDockBar.cx = rc.right - rc.left;
  szDockBar.cy = rc.bottom - rc.top;

	szDockBar.cx = min( szDockBar.cx, rcMDI.right - rcMDI.left );
	szDockBar.cy = min( szDockBar.cy, rcMDI.bottom - rcMDI.top );

  switch ( m_dfPos )
  {
    case DFP_TOP:
      rcMDI.top += szDockBar.cy;
      break;
    case DFP_BOTTOM:
			rcMDI.bottom -= szDockBar.cy;
      break;
    case DFP_LEFT:
      rcMDI.left += szDockBar.cx;
      break;
    case DFP_RIGHT:
			rcMDI.right -= szDockBar.cx;
      break;
  }

}



void GRUIDockFrame::CalculateNeededSize( RECT& rcRemainingSize )
{

  RECT    rcNow;

  GetClientRect( &rcNow );

  SetRectEmpty( &rcRemainingSize );

  std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
  while ( it != m_listControlBars.end() )
  {
    GRUIControlBar*  pThatBar = *it;

    if ( !( pThatBar->m_dwCBStyle & GRUIControlBar::GRCBS_HIDDEN ) )
    {
      RECT    rcBar;

      pThatBar->GetClientRect( &rcBar );

      // neue, tatsächliche Position der Bars benutzen
      pThatBar->ClientToScreen( &rcBar );
      ScreenToClient( &rcBar );

      /*
      if ( rcBar.top < 0 )
      {
        OffsetRect( &rcBar, 0, -rcBar.top );
        pThatBar->SetWindowPos( NULL, rcBar.left, rcBar.top, rcBar.right - rcBar.left, rcBar.bottom - rcBar.top, SWP_NOZORDER );
      }
      */
      
      UnionRect( &rcRemainingSize, &rcRemainingSize, &rcBar );
    }

    ++it;
  }

  if ( GRUIDockingManager::m_bLoadingState )
  {
    return;
  }

  int   iDeltaX = 0,
        iDeltaY = 0;


  if ( m_dfPos == DFP_TOP )
  {
    if ( rcRemainingSize.top > 0 )
    {
      // oben ist eine Lücke, alle ChildBars nachrutschen lassen
      iDeltaY = -rcRemainingSize.top;
    }
  }
  else if ( m_dfPos == DFP_LEFT )
  {
    if ( rcRemainingSize.left > 0 )
    {
      // links ist eine Lücke, alle ChildBars nachrutschen lassen
      iDeltaX = -rcRemainingSize.left;
    }
  }
  else if ( m_dfPos == DFP_BOTTOM )
  {
    if ( rcRemainingSize.top < 0 )
    {
      iDeltaY = -rcRemainingSize.top;
    }
    else if ( rcRemainingSize.top > 0 )
    {
      // oben ist eine Lücke, alle ChildBars nachrutschen lassen
      iDeltaY = -rcRemainingSize.top;
    }
  }
  else if ( m_dfPos == DFP_RIGHT )
  {
    if ( rcRemainingSize.left < 0 )
    {
      iDeltaX = -rcRemainingSize.left;
    }
    else if ( rcRemainingSize.left > 0 )
    {
      // oben ist eine Lücke, alle ChildBars nachrutschen lassen
      iDeltaX = -rcRemainingSize.left;
    }
  }

  if ( ( iDeltaX != 0 )
  ||   ( iDeltaY != 0 ) )
  {
    std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
    while ( it != m_listControlBars.end() )
    {
      GRUIControlBar*  pThatBar = *it;

      RECT    rc;
      pThatBar->GetClientRect( &rc );
      pThatBar->ClientToScreen( &rc );
      ScreenToClient( &rc );

      OffsetRect( &rc, iDeltaX, iDeltaY );
      pThatBar->MoveWindow( &rc );

      pThatBar->m_ptMRUDockedPos.x += iDeltaX;
      pThatBar->m_ptMRUDockedPos.y += iDeltaY;

      ++it;
    }
    OffsetRect( &rcRemainingSize, iDeltaX, iDeltaY );
  }

}



void GRUIDockFrame::RepositionControlBars( GRUIControlBar* pBar, POINT& ptBarPos, DWORD dwDockPos )
{

  POINT   ptNewBarPos = ptBarPos;

  RECT    rcDockBar;

  GetClientRect( &rcDockBar );
  ClientToScreen( &rcDockBar );

  if ( ( ( m_dfPos == DFP_TOP )
  ||     ( m_dfPos == DFP_BOTTOM ) )
  &&   ( ptNewBarPos.x < rcDockBar.left ) )
  {
    ptNewBarPos.x = rcDockBar.left;
  }
  if ( ( ( m_dfPos == DFP_LEFT )
  ||     ( m_dfPos == DFP_RIGHT ) )
  &&   ( ptNewBarPos.y < rcDockBar.top ) )
  {
    ptNewBarPos.y = rcDockBar.top;
  }
  if ( m_dfPos == DFP_TOP )
  {
    if ( ptNewBarPos.y < rcDockBar.top )
    {
      ptNewBarPos.y = rcDockBar.top;
    }
  }

  // Bars anordnen
  std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
  while ( it != m_listControlBars.end() )
  {
    GRUIControlBar*  pThatBar = *it;

    if ( pBar != pThatBar )
    {
      RECT    rcNewBar;

      pBar->GetClientRect( &rcNewBar );
      OffsetRect( &rcNewBar, ptNewBarPos.x, ptNewBarPos.y );
      ScreenToClient( &rcNewBar );

      RECT    rcOldBar;

      pThatBar->GetClientRect( &rcOldBar );
      pThatBar->ClientToScreen( &rcOldBar );
      ScreenToClient( &rcOldBar );

      RECT    rcTemp;

      if ( IntersectRect( &rcTemp, &rcNewBar, &rcOldBar ) )
      {
        // die überlappen sich!
        if ( ( m_dfPos == DFP_LEFT )
        ||   ( m_dfPos == DFP_RIGHT ) )
        {
          ClientToScreen( &rcOldBar );
          ptNewBarPos.y = rcOldBar.bottom;

          if ( ptNewBarPos.y + rcNewBar.bottom - rcNewBar.top > rcDockBar.bottom )
          {
            // außerhalb des Rects
            if ( m_dfPos == DFP_LEFT )
            {
              ptNewBarPos.x += rcNewBar.right - rcNewBar.left;
            }
            else if ( m_dfPos == DFP_RIGHT )
            {
              ptNewBarPos.x -= rcNewBar.right - rcNewBar.left;
            }
            ptNewBarPos.y = rcDockBar.top;
          }
        }
        else
        {
          ClientToScreen( &rcOldBar );
          ptNewBarPos.x = rcOldBar.right;

          if ( ptNewBarPos.x + rcNewBar.right - rcNewBar.left > rcDockBar.right )
          {
            // außerhalb des Rects
            if ( m_dfPos == DFP_TOP )
            {
              ptNewBarPos.y += rcNewBar.bottom - rcNewBar.top;
            }
            else if ( m_dfPos == DFP_BOTTOM )
            {
              ptNewBarPos.y -= rcNewBar.bottom - rcNewBar.top;
            }
            ptNewBarPos.x = rcDockBar.left;
          }
        }

        it = m_listControlBars.begin();
        continue;
      }
    }
    ++it;
  }

  ptBarPos = ptNewBarPos;

}



void GRUIDockFrame::OnClose()
{

  if ( m_dfPos == DFP_FREE )
  {
    //m_pControlBar->Show( FALSE );
  }

}



GRUIDockFrame::eDockFramePos GRUIDockFrame::DockPos() const
{

  return m_dfPos;

}


/*
void GRUIDockFrame::OnContextMenu( HWND hWnd, POINT point )
{

  if ( pWnd != this )
  {
    return;
  }
  
  HMENU   hPopupMenu = CreatePopupMenu();

  int   iIndex = 1;
  std::list<GRUIControlBar*>::iterator   it( CGRDockingManager::m_listControlBars.begin() );
  while ( it != CGRDockingManager::m_listControlBars.end() )
  {
    GRUIControlBar*    pBar = *it;

    CString   cstrGnu;

    pBar->GetWindowText( cstrGnu );

    DWORD   dwFlags = MF_STRING | MF_ENABLED;

    if ( pBar->IsWindowVisible() )
    {
      dwFlags |= MF_CHECKED;
    }

    AppendMenu( hPopupMenu, dwFlags, iIndex++, LPCTSTR( cstrGnu ) );

    ++it;
  }

  DWORD dwResult = TrackPopupMenu( hPopupMenu, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, 0, AfxGetMainWnd()->GetSafeHwnd(), 0 );

  if ( dwResult )
  {
    int   iIndex = 1;
    std::list<CGRControlBar*>::iterator   it( CGRDockingManager::m_listControlBars.begin() );
    while ( it != CGRDockingManager::m_listControlBars.end() )
    {
      CGRControlBar*    pBar = *it;

      if ( iIndex == dwResult )
      {
        pBar->Show( !pBar->IsWindowVisible() );
        break;
      }

      ++iIndex;
      ++it;
    }
  }

  DestroyMenu( hPopupMenu );

}
*/



void GRUIDockFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{

  if ( m_dfPos == DFP_FREE )
  {
    if ( !m_pControlBar->SendMessage( WM_GETMINMAXINFO, 0, (LPARAM)lpMMI ) )
    {
      return;
    }
  }

}



LRESULT GRUIDockFrame::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_CAPTURECHANGED:
      if ( (HWND)lParam != GetSafeHwnd() )
      {
        m_bDragging = false;
      }
      break;
    case WM_NCCREATE:
      GRUIMDIApp::m_listSyncWindows.push_back( m_hWnd );
      break;
    case WM_DESTROY:
      GRUIMDIApp::m_listSyncWindows.remove( m_hWnd );
      break;
    case WM_MOUSEMOVE:
      {
        POINT   pt;

        pt.x = (int)(short)LOWORD( lParam );
        pt.y = (int)(short)HIWORD( lParam );

        MouseMove( (UINT)wParam, pt );
        if ( m_bDragging )
        {
          return 0;
        }
      }
      break;
    case WM_LBUTTONUP:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        LButtonUp( (UINT)wParam, pt );
      }
      break;
    case WM_NCLBUTTONDOWN:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        NcLButtonDown( NCHitTest( pt ), pt );

        if ( m_bDragging )
        {
          return 0;
        }
      }
      break;
    case WM_SIZE:
      OnSize( (UINT)wParam, LOWORD( lParam ), HIWORD( lParam ) );
      break;
    case WM_NCPAINT:
      {
        bool    bDisplayActive = ( GetActiveWindow() == GetSafeHwnd() );

        std::list<HWND>   listHwnd;

        HWND   hParam = (HWND)lParam;

        if ( GRUIMDIApp::m_pMDIApp )
        {
          GRUIMDIApp::GetPopupList( GRUIMDIApp::m_pMDIApp->GetSafeHwnd(), listHwnd, TRUE);
        }

        std::list<HWND>::iterator   it( listHwnd.begin() );
        while ( it != listHwnd.end() )
        {
          if ( hParam == *it )
          {
            // das andere Window ist ja auch eins von unseren
            bDisplayActive = true;
            break;
          }

          ++it;
        }

        bDisplayActive = true;

        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        if ( GetCaptionRect( rc ) )
        {
          m_pDisplayClass->PaintWindowCaption( hdc, rc, m_hWnd, IsCaptionActive() );
        }
        EndNCPaint();
      }
      return 0;
    case WM_NCACTIVATE:
    case WM_ACTIVATE:
      //if ( !wParam )
      {
        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        if ( GetCaptionRect( rc ) )
        {
          m_pDisplayClass->PaintWindowCaption( hdc, rc, m_hWnd, IsCaptionActive() );
        }
        EndNCPaint();
      }
      return TRUE;
  }

  return CCustomWnd::WindowProc( uMsg, wParam, lParam );

}



void GRUIDockFrame::Draw( HDC hdc, const RECT& rc )
{

  if ( !m_listControlBars.empty() )
  {
    m_pDisplayClass->PaintDockBarBackground( hdc, rc );
  }
  CCustomWnd::Draw( hdc, rc );

}



bool GRUIDockFrame::IsCaptionActive()
{

  //if ( !::IsWindowEnabled( GRUIMDIApp::m_pMDIApp->GetSafeHwnd() ) )
  if ( !IsWindowEnabled() )
  {
    return false;
  }

  std::list<HWND>   listHwnd;

  if ( GRUIMDIApp::m_pMDIApp )
  {
    GRUIMDIApp::GetPopupList( GRUIMDIApp::m_pMDIApp->GetSafeHwnd(), listHwnd, TRUE);
  }

  std::list<HWND>::iterator   it( listHwnd.begin() );
  while ( it != listHwnd.end() )
  {
    if ( GetActiveWindow() == *it )
    {
      // das andere Window ist ja auch eins von unseren
      return true;
    }

    ++it;
  }

  return false;

}



void GRUIDockFrame::DrawBackground( HDC hdc, const RECT& rc )
{
}