#include ".\GRUIControlBar.h"
#include "GRUIDockFrame.h"
#include "GRUIDockingManager.h"
#include "GRUIMDIApp.h"



GRUIControlBar::GRUIControlBar() :
  m_bFloating( true ),
  m_bHorizontal( true ),
  m_pFrameWnd( NULL ),
  m_dwCBStyle( 0 ),
  m_dwMRUDockPos( 0 ),
  m_pDockingFrame( NULL )
{

}

GRUIControlBar::~GRUIControlBar()
{
  if ( m_pFrameWnd )
  {
    delete m_pFrameWnd;
    m_pFrameWnd = NULL;
  }
}



bool GRUIControlBar::IsFloating() const
{

  return m_bFloating;

}



BOOL GRUIControlBar::Create( LPCTSTR lpWindowName, DWORD dwCBStyle, HWND hwndOwner, UINT nID )
{

  m_pFrameWnd = new GRUIDockFrame();

  DWORD     dwStyle = 0;

  if ( dwCBStyle & WS_THICKFRAME )
  {
    dwStyle |= WS_THICKFRAME;
  }

  if ( !m_pFrameWnd->Create( hwndOwner, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, dwStyle | WS_CAPTION | WS_POPUP ) )
  {
    delete m_pFrameWnd;
    m_pFrameWnd = NULL;
    return FALSE;
  }

  RECT      rcThis;
  SetRect( &rcThis, 0, 0, 40, 40 );

  if ( !CCustomWnd::Create( lpWindowName, WS_CHILD, rcThis, m_pFrameWnd->GetSafeHwnd(), nID ) )
  {
    delete m_pFrameWnd;
    m_pFrameWnd = NULL;
    return FALSE;
  }

  SetOwner( hwndOwner );

  m_dwCBStyle = dwCBStyle;

  if ( dwCBStyle & GRCBS_FREE_SIZE )
  {
    m_pFrameWnd->m_dwWantedStyles |= WS_THICKFRAME;
    SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED );
  }

  m_pFrameWnd->m_pControlBar = this;

  RECT    rc;

  SIZE   szControlBar = CalcSize();
  SetRect( &rc, 0, 0, szControlBar.cx, szControlBar.cy );

  m_pFrameWnd->CalculateWindowRectFromClientRect( rc );
  //AdjustWindowRectEx( &rc, m_pFrameWnd->GetStyle(), FALSE, m_pFrameWnd->GetExStyle() );

  m_pFrameWnd->SetWindowText( GetWindowText() + "FW" );
  m_pFrameWnd->SetWindowPos( NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER );
  MoveWindow( 0, 0, szControlBar.cx, szControlBar.cy );
  m_pFrameWnd->ShowWindow( SW_SHOW );
  ShowWindow( SW_SHOW );

  GRUIDockingManager::AddControlBar( this );

  //EnableToolTips();

  return TRUE;

}


SIZE GRUIControlBar::EstimateSize( DWORD dwSizingFlags )
{

  bool      bEstimateHorizontal = true;

  if ( ( dwSizingFlags & GRCBS_DOCKED_LEFT )
  ||   ( dwSizingFlags & GRCBS_DOCKED_RIGHT ) )
  {
    bEstimateHorizontal = false;
  }

  SIZE      szCB;

  szCB.cx = 0;
  szCB.cy = 0;

  int       iX = 0,
            iY = 0;

  if ( !IsFloating() )
  {
    // angedockt kommt ein Gripper dran
    if ( bEstimateHorizontal )
    {
      szCB.cx += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperWidth;
      iX += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperWidth;
    }
    else
    {
      szCB.cy += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperHeight;
      iY += m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperHeight;
    }
  }

  if ( dwSizingFlags & GRCBS_APPLY_SIZE )
  {
    m_bHorizontal = bEstimateHorizontal;
  }

  if ( !IsFloating() )
  {
    // angedockt kommt ein Gripper dran
    if ( !bEstimateHorizontal )
    {
      szCB.cx++;
    }
  }

  return szCB;

}



void GRUIControlBar::Draw( HDC hdc, const RECT& rc )
{
}



void GRUIControlBar::OnLButtonDown(UINT nFlags, POINT point)
{

  RECT    rcGripper;

  if ( GetGripperRect( rcGripper ) )
  {
    if ( PtInRect( &rcGripper, point ) )
    {
      ClientToScreen( &point );
      m_pFrameWnd->StartDragging( point );
      return;
    }
  }

}



void GRUIControlBar::OnLButtonUp(UINT nFlags, POINT point)
{

  if ( GetCapture() == GetSafeHwnd() )
  {
    ReleaseCapture();
  }

}



SIZE GRUIControlBar::CalcSize( DWORD dwSizingFlags )
{

  SIZE     szCB;
  
  szCB.cx = 20;
  szCB.cy = 20;

  if ( m_pFrameWnd == NULL )
  {
    szCB.cx = 0;
    szCB.cy = 0;
    return szCB;
  }

  if ( ( IsFloating() )
  &&   ( !m_pFrameWnd->IsWindowVisible() ) )
  {
    szCB.cx = 0;
    szCB.cy = 0;
    return szCB;
  }

  if ( ( !IsFloating() )
  &&   ( m_dwCBStyle & GRCBS_HIDDEN ) )
  {
    szCB.cx = 0;
    szCB.cy = 0;
    return szCB;
  }

  szCB = EstimateSize( dwSizingFlags );

  if ( dwSizingFlags & GRCBS_APPLY_SIZE )
  {
    RECT    rc;

    rc.left = 0;
    rc.top = 0;
    rc.right = szCB.cx;
    rc.bottom = szCB.cy;

    m_pFrameWnd->CalculateWindowRectFromClientRect( rc );

    m_pFrameWnd->SetWindowPos( NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER );

    m_pFrameWnd->GetClientRect( &rc );
  }

  return szCB;

}



LRESULT GRUIControlBar::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_CREATE:
      CalcSize();
      break;
    case WM_LBUTTONDOWN:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonDown( (UINT)wParam, pt );
      }
      break;
    case WM_LBUTTONUP:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonUp( (UINT)wParam, pt );
      }
      break;
    case WM_DESTROY:
      break;
  }

  return CCustomWnd::WindowProc( uMsg, wParam, lParam );

}



void GRUIControlBar::Dock( DWORD dwDockPos, int iX, int iY )
{

  POINT   ptPos;
  ptPos.x = iX;
  ptPos.y = iY;

  GRUIDockingManager::DockControlBar( this, dwDockPos, ptPos );

  m_ptMRUDockedPos  = ptPos;
  m_dwMRUDockPos    = dwDockPos;

  ::ScreenToClient( GetParent(), &ptPos );

  if ( ptPos.y < 0 )
  {
    ptPos.y = 0;
  }
  if ( ptPos.x < 0 )
  {
    ptPos.x = 0;
  }

  RECT    rcOld;
  GetWindowRect( &rcOld );

  POINT   pt;

  pt.x = 0;
  pt.y = 0;

  ::ScreenToClient( GetParent(), &pt );
  OffsetRect( &rcOld, pt.x, pt.y );

  if ( ( rcOld.left != ptPos.x )
  ||   ( rcOld.top != ptPos.y ) )
  {
    SetWindowPos( NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  }

  /*
  if ( ( GetParent() )
  &&   ( !m_bFloating ) )
  {
    m_pFrameWnd->ClientToScreen( &ptPos );
    m_pFrameWnd->RepositionControlBars( this, ptPos, dwDockPos );

    SetWindowPos( NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  }
  */

  GRUIDockingManager::m_pWndTopLevel->RecalcLayout();

}



void GRUIControlBar::Float( int iX, int iY )
{

  m_ptMRUFloatingPos.x  = iX;
  m_ptMRUFloatingPos.y  = iY;
  if ( IsFloating() )
  {
    if ( m_pFrameWnd )
    {
      m_pFrameWnd->SetWindowPos( NULL, iX, iY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
    }
    return;
  }

  m_bFloating = true;

  // DockBar-Größe ändern
  if ( m_pDockingFrame )
  {
    m_pDockingFrame->RemoveControlBar( this );
    m_pDockingFrame = NULL;
  }
  m_dwCBStyle &= ~( GRCBS_DOCKED_LEFT | GRCBS_DOCKED_RIGHT | GRCBS_DOCKED_TOP | GRCBS_DOCKED_BOTTOM | GRCBS_DOCKED );

  CalcSize();

  SetParent( m_pFrameWnd->GetSafeHwnd() );
  SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  m_pFrameWnd->SetWindowPos( NULL, iX, iY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  m_pFrameWnd->ShowWindow( SW_SHOW );

  GRUIDockingManager::m_pWndTopLevel->RecalcLayout();

}



void GRUIControlBar::ToggleState()
{

  if ( IsFloating() )
  {
    if ( m_dwMRUDockPos )
    {
      POINT   ptDockPos = m_ptMRUDockedPos;

      // Koordinaten anpassen lassen
      if ( GRUIDockingManager::CheckForDockingPosition( this, ptDockPos ) )
      {
        Dock( m_dwMRUDockPos, ptDockPos.x, ptDockPos.y );
      }
    }
  }
  else
  {
    Float( m_ptMRUFloatingPos.x, m_ptMRUFloatingPos.y );
  }

}



bool GRUIControlBar::GetGripperRect( RECT& rc )
{

  if ( IsFloating() )
  {
    return false;
  }

  SetRectEmpty( &rc );

  GetClientRect( &rc );

  // angedockt kommt ein Gripper dran
  if ( m_bHorizontal )
  {
    rc.right = rc.left + m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperWidth;
  }
  else
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iToolBarGripperHeight;
  }

  return true;

}