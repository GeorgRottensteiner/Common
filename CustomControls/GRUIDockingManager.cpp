#include "GRUIdockingmanager.h"

#include "GRUIDockFrame.h"
#include "GRUIControlBar.h"
#include "GRUIMDIApp.h"

#include <Misc/Misc.h>


std::list<GRUIControlBar*>      GRUIDockingManager::m_listControlBars;

GRUIDockFrame*                  GRUIDockingManager::m_pDockBars[4] = { 0 };

bool                            GRUIDockingManager::m_bLoadingState = false;

GRUIMDIApp*                     GRUIDockingManager::m_pWndTopLevel = NULL;



void GRUIDockingManager::EnableDocking( DWORD dwDockStyles )
{

  RECT    rcMDIFrame;

  m_pWndTopLevel->GetClientRect( &rcMDIFrame );

  if ( dwDockStyles & GRUIDockFrame::DFP_LEFT )
  {
    if ( m_pDockBars[2] == NULL )
    {
      m_pDockBars[2] = new GRUIDockFrame();
      m_pDockBars[2]->Create( m_pWndTopLevel->GetSafeHwnd(), WS_EX_TOOLWINDOW, WS_POPUP );
      m_pDockBars[2]->m_dfPos = GRUIDockFrame::DFP_LEFT;
      m_pDockBars[2]->SetParent( m_pWndTopLevel->GetSafeHwnd() );
      m_pDockBars[2]->SetWindowText( "GRLeftDockBar" );
      m_pDockBars[2]->ModifyStyle( WS_SYSMENU | WS_POPUP | WS_CAPTION, WS_CHILD );
      m_pDockBars[2]->ModifyStyleEx( WS_EX_TOOLWINDOW, 0 );

      m_pDockBars[2]->SetWindowPos( NULL, rcMDIFrame.left, rcMDIFrame.top, 0, rcMDIFrame.bottom - rcMDIFrame.top, SWP_NOZORDER );
      m_pDockBars[2]->ShowWindow( SW_SHOW );
    }
  }
  else
  {
    if ( m_pDockBars[2] )
    {
      m_pDockBars[2]->DestroyWindow();
      delete m_pDockBars[2];
      m_pDockBars[2] = NULL;
    }
  }
  if ( dwDockStyles & GRUIDockFrame::DFP_RIGHT )
  {
    if ( m_pDockBars[3] == NULL )
    {
      m_pDockBars[3] = new GRUIDockFrame();
      m_pDockBars[3]->Create( m_pWndTopLevel->GetSafeHwnd(), WS_EX_TOOLWINDOW, WS_POPUP );
      m_pDockBars[3]->m_dfPos = GRUIDockFrame::DFP_RIGHT;
      m_pDockBars[3]->SetParent( m_pWndTopLevel->GetSafeHwnd() );
      m_pDockBars[3]->SetWindowText( "GRRightDockBar" );
      m_pDockBars[3]->ModifyStyle( WS_SYSMENU | WS_POPUP | WS_CAPTION, WS_CHILD );
      m_pDockBars[3]->ModifyStyleEx( WS_EX_TOOLWINDOW, 0 );

      m_pDockBars[3]->SetWindowPos( NULL, rcMDIFrame.right, rcMDIFrame.top, 0, rcMDIFrame.bottom - rcMDIFrame.top, SWP_NOZORDER );
      m_pDockBars[3]->ShowWindow( SW_SHOW );
    }
  }
  else
  {
    if ( m_pDockBars[3] )
    {
      m_pDockBars[3]->DestroyWindow();
      delete m_pDockBars[3];
      m_pDockBars[3] = NULL;
    }
  }
  if ( dwDockStyles & GRUIDockFrame::DFP_TOP )
  {
    if ( m_pDockBars[0] == NULL )
    {
      m_pDockBars[0] = new GRUIDockFrame();
      m_pDockBars[0]->Create( m_pWndTopLevel->GetSafeHwnd(), WS_EX_TOOLWINDOW, WS_POPUP );
      m_pDockBars[0]->m_dfPos = GRUIDockFrame::DFP_TOP;
      m_pDockBars[0]->SetParent( m_pWndTopLevel->GetSafeHwnd() );
      m_pDockBars[0]->SetWindowText( "GRUpperDockBar" );
      m_pDockBars[0]->ModifyStyle( WS_SYSMENU | WS_POPUP | WS_CAPTION, WS_CHILD );
      m_pDockBars[0]->ModifyStyleEx( WS_EX_TOOLWINDOW, 0 );

      m_pDockBars[0]->SetWindowPos( NULL, rcMDIFrame.left, rcMDIFrame.top, rcMDIFrame.right - rcMDIFrame.left, 0, SWP_NOZORDER );
      m_pDockBars[0]->ShowWindow( SW_SHOW );
    }
  }
  else
  {
    if ( m_pDockBars[0] )
    {
      m_pDockBars[0]->DestroyWindow();
      delete m_pDockBars[0];
      m_pDockBars[0] = NULL;
    }
  }
  if ( dwDockStyles & GRUIDockFrame::DFP_BOTTOM )
  {
    if ( m_pDockBars[1] == NULL )
    {
      m_pDockBars[1] = new GRUIDockFrame();
      m_pDockBars[1]->Create( m_pWndTopLevel->GetSafeHwnd(), WS_EX_TOOLWINDOW, WS_POPUP );
      m_pDockBars[1]->m_dfPos = GRUIDockFrame::DFP_BOTTOM;
      m_pDockBars[1]->SetParent( m_pWndTopLevel->GetSafeHwnd() );
      m_pDockBars[1]->SetWindowText( "GRLowerDockBar" );
      m_pDockBars[1]->ModifyStyle( WS_SYSMENU | WS_POPUP | WS_CAPTION, WS_CHILD );
      m_pDockBars[1]->ModifyStyleEx( WS_EX_TOOLWINDOW, 0 );

      m_pDockBars[1]->SetWindowPos( NULL, rcMDIFrame.left, rcMDIFrame.bottom, rcMDIFrame.right - rcMDIFrame.left, 0, SWP_NOZORDER );
      m_pDockBars[1]->ShowWindow( SW_SHOW );
    }
  }
  else
  {
    if ( m_pDockBars[1] )
    {
      m_pDockBars[1]->DestroyWindow();
      delete m_pDockBars[1];
      m_pDockBars[1] = NULL;
    }
  }

}



void GRUIDockingManager::AddControlBar( GRUIControlBar* pBar )
{

  m_listControlBars.push_back( pBar );

}



void GRUIDockingManager::RemoveControlBar( GRUIControlBar* pBar )
{
  std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
  while ( it != m_listControlBars.end() )
  {
    if ( *it == pBar )
    {
      m_listControlBars.erase( it );
      return;
    }
    ++it;
  }
}



DWORD GRUIDockingManager::CheckForDockingPosition( GRUIControlBar* pBar, POINT& ptPos )
{

  DWORD     dwDockFlags = 0;

  SIZE      szBarH = pBar->CalcSize( GRUIControlBar::GRCBS_DOCKED | GRUIControlBar::GRCBS_DOCKED_TOP );
  SIZE      szBarV = pBar->CalcSize( GRUIControlBar::GRCBS_DOCKED | GRUIControlBar::GRCBS_DOCKED_LEFT );

  POINT     ptOrigPos = ptPos;

  for ( int i = 0; i < 4; ++i )
  {
    if ( m_pDockBars[i] )
    {
      RECT    rc;

      m_pDockBars[i]->GetClientRect( &rc );

      m_pDockBars[i]->ClientToScreen( &rc );
      //AfxGetMainWnd()->GetTopLevelFrame()->ScreenToClient( &rc );

      // an den MDIClient-Rahmen andocken?
      if ( ( ptPos.y >= rc.top )
      &&   ( ptPos.y < rc.bottom ) )
      {
        if ( i == 2 )
        {
          if ( abs( (int)ptPos.x - (int)rc.left ) < 20 )
          {
            ptPos.x = rc.left;
            dwDockFlags |= GRUIDockFrame::DFP_LEFT;
          }
          // prüfen an Unterkante von vorhandenen Bars
          std::list<GRUIControlBar*>::iterator   it( m_pDockBars[i]->m_listControlBars.begin() );
          while ( it != m_pDockBars[i]->m_listControlBars.end() )
          {
            GRUIControlBar*    pChildBar = *it;

            if ( pChildBar != pBar )
            {
              RECT    rcChildBar;
              pChildBar->GetClientRect( &rcChildBar );
              pChildBar->ClientToScreen( &rcChildBar );
              if ( abs( (int)ptPos.x - (int)rcChildBar.right ) < 20 )
              {
                ptPos.x = rcChildBar.right;
                dwDockFlags |= GRUIDockFrame::DFP_LEFT;
              }
            }

            ++it;
          }
        }
        else if ( i == 3 )
        {
          if ( abs( (int)ptPos.x - (int)rc.right ) < 20 )
          {
            ptPos.x = rc.right - szBarV.cx;
            dwDockFlags |= GRUIDockFrame::DFP_RIGHT;
          }
          else
          {
            // prüfen an Oberkante von vorhandenen Bars
            std::list<GRUIControlBar*>::iterator   it( m_pDockBars[i]->m_listControlBars.begin() );
            while ( it != m_pDockBars[i]->m_listControlBars.end() )
            {
              GRUIControlBar*    pChildBar = *it;

              if ( pChildBar != pBar )
              {
                RECT    rcChildBar;
                pChildBar->GetClientRect( &rcChildBar );
                pChildBar->ClientToScreen( &rcChildBar );
                if ( abs( (int)ptPos.x - (int)rcChildBar.left ) < 20 )
                {
                  ptPos.x = rcChildBar.left - szBarV.cx;
                  dwDockFlags |= GRUIDockFrame::DFP_RIGHT;
                }
              }
              ++it;
            }
          }
        }
        /*
        else if ( ( abs( (int)ptPos.x - (int)rc.right ) < 40 )
        &&        ( i == 3 ) )
        {
          ptPos.x = rc.left;
          dwDockFlags |= CBRS_ALIGN_RIGHT;
        }
        */
      }
      if ( ( ptPos.x >= rc.left )
      &&   ( ptPos.x < rc.right ) )
      {
        if ( i == 0 )
        {
          if ( abs( (int)ptPos.y - (int)rc.top ) < 20 )
          {
            ptPos.y = rc.top;
            dwDockFlags |= GRUIDockFrame::DFP_TOP;
          }
          // prüfen an Unterkante von vorhandenen Bars
          std::list<GRUIControlBar*>::iterator   it( m_pDockBars[i]->m_listControlBars.begin() );
          while ( it != m_pDockBars[i]->m_listControlBars.end() )
          {
            GRUIControlBar*    pChildBar = *it;

            if ( pChildBar != pBar )
            {
              RECT    rcChildBar;
              pChildBar->GetClientRect( &rcChildBar );
              pChildBar->ClientToScreen( &rcChildBar );
              if ( abs( (int)ptPos.y - (int)rcChildBar.bottom ) < 20 )
              {
                ptPos.y = rcChildBar.bottom;
                dwDockFlags |= GRUIDockFrame::DFP_TOP;
              }
            }

            ++it;
          }
        }
        else if ( i == 1 )
        {
          if ( abs( (int)ptPos.y - (int)rc.bottom ) < 20 )
          {
            ptPos.y = rc.bottom - szBarH.cy;
            dwDockFlags |= GRUIDockFrame::DFP_BOTTOM;
          }
          else
          {
            // prüfen an Oberkante von vorhandenen Bars
            std::list<GRUIControlBar*>::iterator   it( m_pDockBars[i]->m_listControlBars.begin() );
            while ( it != m_pDockBars[i]->m_listControlBars.end() )
            {
              GRUIControlBar*    pChildBar = *it;

              if ( pChildBar != pBar )
              {
                RECT    rcChildBar;
                pChildBar->GetClientRect( &rcChildBar );
                pChildBar->ClientToScreen( &rcChildBar );
                if ( abs( (int)ptPos.y - (int)rcChildBar.top ) < 20 )
                {
                  ptPos.y = rcChildBar.top - szBarH.cy;
                  dwDockFlags |= GRUIDockFrame::DFP_BOTTOM;
                }
              }
              ++it;
            }
          }
        }
      }

    }
  }

  if ( ( pBar->m_dwCBStyle & GRUIControlBar::GRCBS_DOCKED_BOTTOM )
  &&   ( dwDockFlags & GRUIDockFrame::DFP_BOTTOM ) )
  {
    // falls der Bar schon angedockt ist, wird dieser Dockframe bevorzugt
    dwDockFlags &= GRUIDockFrame::DFP_BOTTOM;
    ptPos.x = ptOrigPos.x;
  }
  if ( ( pBar->m_dwCBStyle & GRUIControlBar::GRCBS_DOCKED_TOP )
  &&   ( dwDockFlags & GRUIDockFrame::DFP_TOP ) )
  {
    // falls der Bar schon angedockt ist, wird dieser Dockframe bevorzugt
    dwDockFlags &= GRUIDockFrame::DFP_TOP;
    ptPos.x = ptOrigPos.x;
  }
  if ( ( pBar->m_dwCBStyle & GRUIControlBar::GRCBS_DOCKED_LEFT )
  &&   ( dwDockFlags & GRUIDockFrame::DFP_LEFT ) )
  {
    // falls der Bar schon angedockt ist, wird dieser Dockframe bevorzugt
    dwDockFlags &= GRUIDockFrame::DFP_LEFT;
    ptPos.y = ptOrigPos.y;
  }
  if ( ( pBar->m_dwCBStyle & GRUIControlBar::GRCBS_DOCKED_RIGHT )
  &&   ( dwDockFlags & GRUIDockFrame::DFP_RIGHT ) )
  {
    // falls der Bar schon angedockt ist, wird dieser Dockframe bevorzugt
    dwDockFlags &= GRUIDockFrame::DFP_RIGHT;
    ptPos.y = ptOrigPos.y;
  }

  return dwDockFlags;

}



void GRUIDockingManager::DockControlBar( GRUIControlBar* pBar, DWORD dwDockPos, POINT& ptBarPos )
{

  GRUIDockFrame*    pDockFrame = NULL;
  if ( dwDockPos & GRUIDockFrame::DFP_LEFT )
  {
    pDockFrame = m_pDockBars[2];
  }
  else if ( dwDockPos & GRUIDockFrame::DFP_RIGHT )
  {
    pDockFrame = m_pDockBars[3];
  }
  else if ( dwDockPos & GRUIDockFrame::DFP_TOP )
  {
    pDockFrame = m_pDockBars[0];
  }
  else if ( dwDockPos & GRUIDockFrame::DFP_BOTTOM )
  {
    pDockFrame = m_pDockBars[1];
  }
  if ( pDockFrame == NULL )
  {
    //TRACE( "KREISCH _ FEHLER!!!\n" );
    return;
  }

  if ( pBar->GetParent() != pDockFrame->GetSafeHwnd() )
  {
    if ( !pBar->IsFloating() )
    {
      // der Bar steckt in einem anderen DockFrame
      GRUIDockFrame*    pOldDockFrame = pBar->m_pDockingFrame;

      pOldDockFrame->RemoveControlBar( pBar );
      pBar->m_pDockingFrame = NULL;
    }
    pBar->SetParent( pDockFrame->GetSafeHwnd() );
    pBar->m_pDockingFrame = pDockFrame;
    
    DWORD   dwSizeFlags = GRUIControlBar::GRCBS_APPLY_SIZE | GRUIControlBar::GRCBS_DOCKED;

    if ( dwDockPos & GRUIDockFrame::DFP_LEFT )
    {
      dwSizeFlags |= GRUIControlBar::GRCBS_DOCKED_LEFT;
      pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_DOCKED_LEFT;
    }
    if ( dwDockPos & GRUIDockFrame::DFP_RIGHT )
    {
      dwSizeFlags |= GRUIControlBar::GRCBS_DOCKED_RIGHT;
      pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_DOCKED_RIGHT;
    }
    if ( dwDockPos & GRUIDockFrame::DFP_TOP )
    {
      dwSizeFlags |= GRUIControlBar::GRCBS_DOCKED_TOP;
      pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_DOCKED_TOP;
    }
    if ( dwDockPos & GRUIDockFrame::DFP_BOTTOM )
    {
      dwSizeFlags |= GRUIControlBar::GRCBS_DOCKED_BOTTOM;
      pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_DOCKED_BOTTOM;
    }
    if ( dwSizeFlags )
    {
      dwSizeFlags |= GRUIControlBar::GRCBS_DOCKED;
      pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_DOCKED;
    }
    pBar->m_bFloating = false;

    pBar->CalcSize( dwSizeFlags );

    pDockFrame->InsertControlBar( pBar );
  }

  if ( !m_bLoadingState )
  {
    pDockFrame->RepositionControlBars( pBar, ptBarPos, dwDockPos );

    POINT   ptTemp = ptBarPos;

    pDockFrame->ScreenToClient( &ptTemp );

    pBar->SetWindowPos( NULL, ptTemp.x, ptTemp.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    m_pWndTopLevel->RecalcLayout();
  }

  pBar->m_bFloating = false;

  // DockBar-Größe ändern
  RECT    rcOld;
  pBar->m_pFrameWnd->GetWindowRect( &rcOld );
  if ( rcOld.left > -32000 )
  {
    pBar->m_pFrameWnd->SetWindowPos( NULL, -32000, -32000, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
  }

}



void GRUIDockingManager::ShowControlBar( GRUIControlBar* pBar, BOOL bShow )
{

  if ( pBar == NULL )
  {
    return;
  }
  if ( bShow == pBar->IsWindowVisible() )
  {
    return;
  }

  if ( !bShow )
  {
    // einfach
    pBar->m_dwCBStyle |= GRUIControlBar::GRCBS_HIDDEN;
    if ( pBar->IsFloating() )
    {
      pBar->m_pFrameWnd->ShowWindow( SW_HIDE );
      return;
    }
    // der Bar muß "entdockt" werden
    pBar->ShowWindow( SW_HIDE );
  }
  else
  {
    // einfach
    pBar->m_dwCBStyle &= ~GRUIControlBar::GRCBS_HIDDEN;
    if ( pBar->IsFloating() )
    {
      pBar->m_pFrameWnd->ShowWindow( SW_SHOW );
      return;
    }
    // der Bar muß "entdockt" werden
    pBar->ShowWindow( SW_SHOW );
  }

  RECT    rcBar;

  pBar->GetClientRect( &rcBar );
  pBar->ClientToScreen( &rcBar );
  POINT   ptBar;

  ptBar.x = rcBar.left;
  ptBar.y = rcBar.top;
  pBar->m_pFrameWnd->RepositionControlBars( pBar, ptBar, pBar->m_dwMRUDockPos );

  pBar->m_pFrameWnd->ScreenToClient( &ptBar );
  pBar->SetWindowPos( NULL, ptBar.x, ptBar.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

  m_pWndTopLevel->RecalcLayout();

}



void GRUIDockingManager::SaveBarState()
{

  /*
  CWinApp* pApp = AfxGetApp();

  int   iCount = (int)m_listControlBars.size();

  pApp->WriteProfileInt( "BarStates", "BarCount", iCount );

  int   iBarIndex = 0;

  char  szTemp[MAX_PATH];

  std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
  while ( it != m_listControlBars.end() )
  {
    GRUIControlBar*   pBar = *it;

    wsprintf( szTemp, "Bar%dID", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->GetDlgCtrlID() );
    wsprintf( szTemp, "Bar%dStyle", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_dwCBStyle );
    wsprintf( szTemp, "Bar%dDocked", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->IsFloating() ? 0 : 1 );
    wsprintf( szTemp, "Bar%dMRUDockPosX", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_ptMRUDockedPos.x );
    wsprintf( szTemp, "Bar%dMRUDockPosY", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_ptMRUDockedPos.y );
    wsprintf( szTemp, "Bar%dMRUFloatPosX", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_ptMRUFloatingPos.x );
    wsprintf( szTemp, "Bar%dMRUFloatPosY", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_ptMRUFloatingPos.y );
    wsprintf( szTemp, "Bar%dMRUDockPos", iBarIndex );
    pApp->WriteProfileInt( "BarStates", szTemp, pBar->m_dwMRUDockPos );

    WINDOWPLACEMENT   wpl;

    wpl.length = sizeof( wpl );

    if ( pBar->IsFloating() )
    {
      pBar->m_pFrameWnd->GetWindowPlacement( &wpl );
    }
    else
    {
      pBar->GetWindowPlacement( &wpl );
    }

    wsprintf( szTemp, "Bar%dPlacement", iBarIndex );
    pApp->WriteProfileBinary( "BarStates", szTemp, (BYTE*)&wpl, sizeof( wpl ) );

    ++iBarIndex;
    ++it;
  }
  */

}



void GRUIDockingManager::LoadBarState()
{

  /*
  CWinApp* pApp = AfxGetApp();

  int   iCount = pApp->GetProfileInt( "BarStates", "BarCount", 0 );

  char  szTemp[MAX_PATH];

  m_bLoadingState = true;
  for ( int iBarIndex = 0; iBarIndex < iCount; ++iBarIndex )
  {
    wsprintf( szTemp, "Bar%dID", iBarIndex );
    int   iBarID = pApp->GetProfileInt( "BarStates", szTemp, 0 );

    if ( iBarID == 0 )
    {
      continue;
    }

    GRUIControlBar*    pThisBar = NULL;

    std::list<GRUIControlBar*>::iterator   it( m_listControlBars.begin() );
    while ( it != m_listControlBars.end() )
    {
      GRUIControlBar*    pBar = *it;

      if ( pBar->GetDlgCtrlID() == iBarID )
      {
        pThisBar = pBar;
        break;
      }
      ++it;
    }
    if ( pThisBar == NULL )
    {
      continue;
    }

    pThisBar->SetWindowPos( NULL, -32000, -32000, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

    WINDOWPLACEMENT* pWPL = NULL;

    UINT   dwDataSize = 0;

    wsprintf( szTemp, "Bar%dPlacement", iBarIndex );
    pApp->GetProfileBinary( "BarStates", szTemp, (LPBYTE*)&pWPL, &dwDataSize );
    if ( dwDataSize != sizeof( WINDOWPLACEMENT ) )
    {
      delete[] pWPL;
      pWPL = NULL;
      continue;
    }

    wsprintf( szTemp, "Bar%dStyle", iBarIndex );
    DWORD   dwBarStyles = pApp->GetProfileInt( "BarStates", szTemp, GRUIControlBar::GRCBS_FIXED_SIZE );

    
    wsprintf( szTemp, "Bar%dMRUDockPosX", iBarIndex );
    pThisBar->m_ptMRUDockedPos.x = pApp->GetProfileInt( "BarStates", szTemp, 0 );
    wsprintf( szTemp, "Bar%dMRUDockPosY", iBarIndex );
    pThisBar->m_ptMRUDockedPos.y = pApp->GetProfileInt( "BarStates", szTemp, 0 );
    wsprintf( szTemp, "Bar%dMRUFloatPosX", iBarIndex );
    pThisBar->m_ptMRUFloatingPos.x = pApp->GetProfileInt( "BarStates", szTemp, 0 );
    wsprintf( szTemp, "Bar%dMRUFloatPosY", iBarIndex );
    pThisBar->m_ptMRUFloatingPos.y = pApp->GetProfileInt( "BarStates", szTemp, 0 );
    wsprintf( szTemp, "Bar%dMRUDockPos", iBarIndex );
    pThisBar->m_dwMRUDockPos = pApp->GetProfileInt( "BarStates", szTemp, 0 );

    if ( dwBarStyles & GRUIControlBar::GRCBS_DOCKED )
    {
      DWORD   dwDockPos = 0;

      CGRMiniDockFrameWnd*    pDockFrame = NULL;

      if ( dwBarStyles & GRUIControlBar::GRCBS_DOCKED_LEFT )
      {
        dwDockPos |= CBRS_ALIGN_LEFT;
        pDockFrame = m_pDockBars[2];
      }
      if ( dwBarStyles & GRUIControlBar::GRCBS_DOCKED_RIGHT )
      {
        dwDockPos |= CBRS_ALIGN_RIGHT;
        pDockFrame = m_pDockBars[3];
      }
      if ( dwBarStyles & GRUIControlBar::GRCBS_DOCKED_TOP )
      {
        dwDockPos |= CBRS_ALIGN_TOP;
        pDockFrame = m_pDockBars[0];
      }
      if ( dwBarStyles & GRUIControlBar::GRCBS_DOCKED_BOTTOM )
      {
        dwDockPos |= CBRS_ALIGN_BOTTOM;
        pDockFrame = m_pDockBars[1];
      }

      if ( pDockFrame )
      {
        pDockFrame->ClientToScreen( &pWPL->rcNormalPosition );
        pThisBar->Dock( dwDockPos, pWPL->rcNormalPosition.left, pWPL->rcNormalPosition.top );
      }
    }
    else
    {
      pThisBar->m_bFloating = false;
      pThisBar->Float( pWPL->rcNormalPosition.left, pWPL->rcNormalPosition.top );
    }

    if ( dwBarStyles & GRUIControlBar::GRCBS_HIDDEN )
    {
      ShowControlBar( pThisBar, FALSE );
    }

    pThisBar->m_dwCBStyle = dwBarStyles;

    delete[] pWPL;
  }

  */
  m_bLoadingState = false;

}