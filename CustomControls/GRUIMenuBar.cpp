// GRUIMenuBar.cpp : Implementierungsdatei
//

#include "GRUIMenuBar.h"
#include "GRUIMenu.h"
#include "GRUIDockFrame.h"
#include "GRUIMDIApp.h"

#include "ClassicDisplayClass.h"

#include <WinSys/WinUtils.h>

#include <debug/debugclient.h>


// GRUIMenuBar


GRUIMenuBar::GRUIMenuBar() :
  m_hwndMaximizedMDIChild( NULL ),
  m_iHighlightedItem( -1 ),
  m_iPushedItem( -1 ),
  m_bKeepPushing( false )
{
}

GRUIMenuBar::~GRUIMenuBar()
{

  tVectItems::iterator    it( m_vectItems.begin() );
  while ( it != m_vectItems.end() )
  {
    tItemInfo&    ItemInfo = *it;

    DeleteMenuItem( ItemInfo );

    ++it;
  }
  m_vectItems.clear();

}


BOOL GRUIMenuBar::LoadHMenu( HMENU hMenu )
{

  if ( hMenu == NULL )
  {
    return FALSE;
  }

  tVectItems::iterator    it( m_vectItems.begin() );
  while ( it != m_vectItems.end() )
  {
    tItemInfo&    ItemInfo = *it;

    if ( ItemInfo.dwExtraData )
    {
      delete (CGRUIMenu*)ItemInfo.dwExtraData;
      ItemInfo.dwExtraData = 0;
    }

    ++it;
  }
  m_vectItems.clear();

  int iItems = GetMenuItemCount( hMenu );

  for ( int i = 0; i < iItems; ++i )
  {
    MENUITEMINFO    mii;

    ZeroMemory( &mii, sizeof( mii ) );
    mii.cbSize = sizeof( mii );
    mii.fMask = MIIM_ID | MIIM_SUBMENU;
    if ( GetMenuItemInfo( hMenu, i, TRUE, &mii ) )
    {
      char    szTemp[MAX_PATH];

      GetMenuString( hMenu, i, szTemp, MAX_PATH, MF_BYPOSITION );

      AddItem( szTemp, mii.wID );

      tItemInfo&    ItemInfo = m_vectItems.back();

      ItemInfo.pSubMenu = ReplaceMenu( mii.hSubMenu );
    }
  }

  if ( !m_bHorizontal )
  {
    CalcSize( GRUIControlBar::GRCBS_APPLY_SIZE | GRCBS_FLOATING | GRCBS_DOCKED_LEFT );
  }
  else
  {
    CalcSize();
  }

  m_hmenuLoaded = hMenu;

  return TRUE;

}



CGRUIMenu* GRUIMenuBar::ReplaceMenu( HMENU hMenu )
{

  if ( hMenu == NULL )
  {
    return NULL;
  }

  CGRUIMenu*    pODMenu = new CGRUIMenu();

  for ( int i = 0; i < ::GetMenuItemCount( hMenu ); ++i )
  {
    char    szTemp[MAX_PATH];

    GetMenuString( hMenu, i, szTemp, MAX_PATH, MF_BYPOSITION );

    MENUITEMINFO    ii;

    ii.cbSize = sizeof( ii );
    ii.fMask  = MIIM_ID | MIIM_SUBMENU | MIIM_STATE;
    GetMenuItemInfo( hMenu, i, TRUE, &ii );

    DWORD   dwExtraFlags = ii.fState;

    if ( ii.hSubMenu != NULL )
    {
      pODMenu->AppendMenu( dwExtraFlags | MF_POPUP, (UINT_PTR)ReplaceMenu( ii.hSubMenu ), szTemp );
    }
    else if ( ii.wID == 0 )
    {
      pODMenu->AppendMenu( dwExtraFlags | MF_SEPARATOR );
    }
    else
    {
      pODMenu->AppendMenu( dwExtraFlags | MF_STRING, ii.wID, szTemp );
    }
  }

  return pODMenu;

}



UINT GRUIMenuBar::TrackMenu( HMENU hMenu, DWORD_PTR dwExtraData, UINT nAlignFlags, int iX, int iY, HWND hwndParent, TPMPARAMS* pParams )
{

  CGRUIMenu*    pMenu = (CGRUIMenu*)dwExtraData;

  if ( pMenu )
  {

    return pMenu->TrackPopupMenu( TPM_RETURNCMD | TPM_NONOTIFY, iX, iY, hwndParent, NULL );
  }
  return 0;

}



void GRUIMenuBar::UpdatePopupMenu( size_t iPushedItem, HMENU hMenu )
{

  if ( iPushedItem >= m_vectItems.size() )
  {
    return;
  }

  tItemInfo&    ItemInfo = m_vectItems[iPushedItem];

  if ( ItemInfo.dwExtraData )
  {
    delete (CGRUIMenu*)ItemInfo.dwExtraData;
    ItemInfo.dwExtraData = 0;
  }

  ItemInfo.dwExtraData = (DWORD_PTR)ReplaceMenu( hMenu );
  ItemInfo.dwCommandID = (UINT_PTR)hMenu;

}



void GRUIMenuBar::DeleteMenuItem( tItemInfo& ItemInfo )
{

  if ( ItemInfo.pSubMenu )
  {
    delete ItemInfo.pSubMenu;
    ItemInfo.pSubMenu = NULL;
  }

}



void GRUIMenuBar::EnableSubMenuItem( int m_nIndex, UINT_PTR nID, HMENU m_hSubMenu, BOOL bOn )
{

  // jetzt muß das Item gesucht werden
  tVectItems::iterator    it( m_vectItems.begin() );
  while ( it != m_vectItems.end() )
  {
    tItemInfo&    ItemInfo = *it;

    if ( ItemInfo.dwExtraData )
    {
      CGRUIMenu*    pODMenu = (CGRUIMenu*)ItemInfo.dwExtraData;

      if ( pODMenu->EnableMenuItem( nID, bOn ) )
      {
        return;
      }
    }
    ++it;
  }

}



bool GRUIMenuBar::SetMenuItemIcons( UINT_PTR nID,
                                     DWORD dwFlags,
                                     HICON hIconChecked,
                                     HICON hIconUnchecked )
{

  tVectItems::iterator    it( m_vectItems.begin() );
  while ( it != m_vectItems.end() )
  {
    tItemInfo&    ItemInfo = *it;

    if ( ItemInfo.dwExtraData )
    {
      CGRUIMenu*    pODMenu = (CGRUIMenu*)ItemInfo.dwExtraData;

      if ( pODMenu->SetMenuItemIcons( (UINT)nID, dwFlags, hIconChecked, hIconUnchecked ) )
      {
        return true;
      }
    }
    ++it;
  }

  return false;

}



BOOL GRUIMenuBar::AddItem( DWORD dwCommandID, HICON hIcon )
{

  tItemInfo   ItemInfo;

  ItemInfo.dwCommandID = dwCommandID;
  ItemInfo.hIcon       = hIcon;
  SetRect( &ItemInfo.rcButton, 0, 0, 23, 23 );

  m_vectItems.push_back( ItemInfo );

  CalcSize();

  return TRUE;

}



BOOL GRUIMenuBar::AddItem( const char* szText, DWORD dwCommandID, HICON hIcon )
{

  tItemInfo   ItemInfo;

  ItemInfo.dwCommandID = dwCommandID;
  ItemInfo.hIcon       = hIcon;
  ItemInfo.strText     = szText;
  SetRect( &ItemInfo.rcButton, 0, 0, 23, 23 );

  m_vectItems.push_back( ItemInfo );

  CalcSize();

  return TRUE;

}



BOOL GRUIMenuBar::AddSubMenu( const char* szText, CGRUIMenu* pSubMenu, HICON hIcon )
{

  tItemInfo   ItemInfo;

  ItemInfo.pSubMenu     = pSubMenu;
  ItemInfo.hIcon        = hIcon;
  ItemInfo.strText      = szText;
  SetRect( &ItemInfo.rcButton, 0, 0, 23, 23 );

  m_vectItems.push_back( ItemInfo );

  CalcSize();

  return TRUE;

}



SIZE GRUIMenuBar::EstimateSize( DWORD dwSizingFlags )
{

  HDC         hdc = GetDC();

  bool        bEstimateHorizontal = true;

  if ( ( dwSizingFlags & GRCBS_DOCKED_LEFT )
  ||   ( dwSizingFlags & GRCBS_DOCKED_RIGHT ) )
  {
    bEstimateHorizontal = false;
  }

  SIZE      szCB = { 0, 0 };

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

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  tVectItems  vectTempItems = m_vectItems;

  // falls noch MDI-Menu-Icons oder Caption-Buttons da sind, weg
  tVectItems::iterator    it( vectTempItems.begin() );
  while ( it != vectTempItems.end() )
  {
    tItemInfo&    ItemInfo = *it;

    if ( ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUBTN )
    ||   ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUICON ) )
    {
      if ( dwSizingFlags & GRCBS_APPLY_SIZE )
      {
        DeleteMenuItem( ItemInfo );
      }
      it = vectTempItems.erase( it );
      continue;
    }

    ++it;
  }

  if ( m_hwndMaximizedMDIChild )
  {
    // vorne ein Icon dazu
    tItemInfo   ItemInfo;

    ItemInfo.dwButtonStyle = tItemInfo::IF_SYSMENUICON;
    SetRect( &ItemInfo.rcButton, 0, 0, 23, 23 );
    ItemInfo.strText = "@";

    ItemInfo.pSubMenu = ReplaceMenu( ::GetSystemMenu( m_hwndMaximizedMDIChild, FALSE ) );

    vectTempItems.insert( vectTempItems.begin(), ItemInfo );


    // hinten drei Caption-Buttons
    RECT    rc = ItemInfo.rcButton;

    int   iWidth = GetSystemMetrics( SM_CXSMICON );
    int   iHeight = GetSystemMetrics( SM_CYSMICON ) - 2;

    // Icon zentrieren
    int   iDX = 0,
          iDY = 0;

    if ( bEstimateHorizontal )
    {
      iDY = ( 23 - iHeight ) / 2;
    }
    else
    {
      iDX = ( 23 - iWidth ) / 2;
    }


    {
      tItemInfo   ItemInfo;

      ItemInfo.dwButtonStyle = tItemInfo::IF_SYSMENUBTN;
      ItemInfo.dwCommandID = SC_MINIMIZE;
      SetRect( &ItemInfo.rcButton, iDX, iDY, iDX + iWidth, iDY + iHeight );
      ItemInfo.strText = "_";

      vectTempItems.push_back( ItemInfo );
    }
    {
      tItemInfo   ItemInfo;

      ItemInfo.dwButtonStyle = tItemInfo::IF_SYSMENUBTN;
      ItemInfo.dwCommandID = SC_RESTORE;
      SetRect( &ItemInfo.rcButton, iDX, iDY, iDX + iWidth, iDY + iHeight );
      ItemInfo.strText = "·";

      vectTempItems.push_back( ItemInfo );
    }
    {
      tItemInfo   ItemInfo;

      ItemInfo.dwButtonStyle = tItemInfo::IF_SYSMENUBTN;
      ItemInfo.dwCommandID = SC_CLOSE;
      SetRect( &ItemInfo.rcButton, iDX, iDY, iDX + iWidth, iDY + iHeight );
      ItemInfo.strText = "X";

      vectTempItems.push_back( ItemInfo );
    }
  }

  for ( size_t i = 0; i < vectTempItems.size(); ++i )
  {
    tItemInfo&    ItemInfo = vectTempItems[i];

    if ( ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUICON )
    ||   ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUBTN ) )
    {
      int   iWidth  = 0;
      int   iHeight = 0;
      if ( !bEstimateHorizontal )
      {
        iHeight = ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
        if ( ItemInfo.dwCommandID == SC_RESTORE )
        {
          iHeight += 2;
        }
        szCB.cy += iHeight;
        iY += iHeight;
      }
      else
      {
        iWidth  = ItemInfo.rcButton.right - ItemInfo.rcButton.left;
        if ( ItemInfo.dwCommandID == SC_RESTORE )
        {
          iWidth += 2;
        }
        szCB.cx += iWidth;
        iX += iWidth;
      }
      OffsetRect( &ItemInfo.rcButton, iX - iWidth, iY - iHeight );
      continue;
    }

    ItemInfo.rcButton.left   = iX;
    ItemInfo.rcButton.top    = iY;


    if ( !bEstimateHorizontal )
    {
      GR::String     strVertText = "";

      for ( size_t j = 0; j < ItemInfo.strText.length(); ++j )
      {
        if ( ItemInfo.strText[j] == '&' )
        {
          strVertText += ItemInfo.strText[j + 1];
          ++j;
          continue;
        }
        strVertText += ItemInfo.strText[j];
      }
      DrawText( hdc, strVertText.c_str(), (int)strVertText.length(), &ItemInfo.rcButton, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX );

      // Text wird horizontal berechnet
      InflateRect( &ItemInfo.rcButton, 8, 4 );
      OffsetRect( &ItemInfo.rcButton, 8, 4 );
      szCB.cy += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
      if ( ItemInfo.rcButton.bottom - ItemInfo.rcButton.top > szCB.cx )
      {
        szCB.cx = ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
      }


      int   iTemp = ItemInfo.rcButton.right - ItemInfo.rcButton.left;

      ItemInfo.rcButton.right = ItemInfo.rcButton.left + ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
      ItemInfo.rcButton.bottom = ItemInfo.rcButton.top + iTemp;

      iY += ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
    }
    else
    {
      DrawText( hdc, ItemInfo.strText.c_str(), (int)ItemInfo.strText.length(), &ItemInfo.rcButton, DT_CALCRECT | DT_SINGLELINE );
      InflateRect( &ItemInfo.rcButton, 8, 4 );
      OffsetRect( &ItemInfo.rcButton, 8, 4 );

      szCB.cx += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
      if ( ItemInfo.rcButton.bottom - ItemInfo.rcButton.top > szCB.cy )
      {
        szCB.cy = ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
      }
      iX += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
    }
  }

  if ( !IsFloating() )
  {
    // angedockt kommt ein Gripper dran
    if ( !bEstimateHorizontal )
    {
      szCB.cx++;
    }
  }

  SelectObject( hdc, hOldFont );

  ReleaseDC( hdc );

  if ( dwSizingFlags & GRCBS_APPLY_SIZE )
  {
    m_bHorizontal = bEstimateHorizontal;
    m_vectItems = vectTempItems;
  }

  return szCB;

}



void GRUIMenuBar::DrawBackground( HDC hdc, const RECT& rcRedraw )
{

  m_pDisplayClass->FillSolidRect( hdc, &rcRedraw, GetSysColor( COLOR_BTNFACE ) );

}



void GRUIMenuBar::Draw( HDC hdc, const RECT& rc )
{

  int   iX = 0,
        iY = 0;


  RECT  rcGripper;

  if ( GetGripperRect( rcGripper ) )
  {
    m_pDisplayClass->PaintToolBarGripper( hdc, rcGripper, m_bHorizontal );
  }

  HFONT   hFontToUse = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
  if ( !m_bHorizontal )
  {
    LOGFONT   lf;

    memset( &lf, 0, sizeof( lf ) );

    GetObject( (HFONT)GetStockObject( DEFAULT_GUI_FONT ), sizeof( lf ), &lf );

    lf.lfEscapement = 2700;//900;

    hFontToUse = CreateFontIndirect( &lf );
  }

  HFONT   hOldFont = (HFONT)SelectObject( hdc, hFontToUse );

  for ( size_t i = 0; i < m_vectItems.size(); ++i )
  {
    tItemInfo&    ItemInfo = m_vectItems[i];

    m_pDisplayClass->PaintMenuBarItem( hdc, m_bHorizontal, ItemInfo );

  }

  SelectObject( hdc, hOldFont );

  if ( !m_bHorizontal )
  {
    DeleteObject( hFontToUse );
  }

  CCustomWnd::Draw( hdc, rc );

}



void GRUIMenuBar::OnLButtonDown( UINT nFlags, POINT point )
{

  Default();

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

  size_t iOverButton = MenuItemFromPoint( point );

  if ( iOverButton != m_iHighlightedItem )
  {
    if ( m_iHighlightedItem != -1 )
    {
      InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
      m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
      m_iHighlightedItem = -1;
    }
    m_iHighlightedItem  = iOverButton;
  }
  if ( m_iPushedItem != iOverButton )
  {
    m_iPushedItem       = iOverButton;
    if ( iOverButton != -1 )
    {
      m_vectItems[iOverButton].dwButtonStyle |= tItemInfo::IF_HILIGHTED | tItemInfo::IF_PUSHED;
      InvalidateRect( &m_vectItems[iOverButton].rcButton );

      m_bKeepPushing = true;
      PushButton( m_iPushedItem );
      return;
      //SetCapture();
    }
  }
  if ( iOverButton == -1 )
  {
    m_bKeepPushing = false;
  }

}



void GRUIMenuBar::OnLButtonUp(UINT nFlags, POINT point )
{

  if ( m_bTrackingMenu )
  {
    return;
  }

  if ( GetCapture() == GetSafeHwnd() )
  {
    ReleaseCapture();
  }
  m_bKeepPushing = false;
  if ( ( m_iHighlightedItem != -1 )
  &&   ( m_iPushedItem != -1 )
  &&   ( m_iHighlightedItem == m_iPushedItem ) )
  {
    tItemInfo&    ItemInfo = m_vectItems[m_iPushedItem];
    if ( ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUBTN )
    ||   ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUICON ) )
    {
      // TODO
      //GetTopLevelFrame()->PostMessage( WM_COMMAND, MAKEWPARAM( ItemInfo.dwCommandID, 0 ), (LPARAM)GetSafeHwnd() );
    }
  }

  if ( m_iHighlightedItem != -1 )
  {
    InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
    m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
    m_iHighlightedItem = -1;
  }
  if ( m_iPushedItem != -1 )
  {
    InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
    m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
    m_iPushedItem = -1;
  }

}



void GRUIMenuBar::OnMouseMove( UINT nFlags, POINT point )
{

  size_t   iOverButton = MenuItemFromPoint( point );

  if ( iOverButton != m_iHighlightedItem )
  {
    if ( m_iHighlightedItem != -1 )
    {
      InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
      m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
      m_iHighlightedItem = -1;
    }
    if ( m_iPushedItem != -1 )
    {
      if ( m_vectItems[m_iPushedItem].dwButtonStyle & tItemInfo::IF_PUSHED )
      {
        m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
        InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
      }
    }
    m_iHighlightedItem  = iOverButton;
    if ( iOverButton != -1 )
    {
      if ( ( nFlags & MK_LBUTTON )
      ||   ( m_bKeepPushing ) )
      {
        if ( m_iPushedItem != iOverButton )
        {
          if ( m_iPushedItem != -1 )
          {
            m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
          }
          m_iPushedItem = iOverButton;
          if ( m_iPushedItem != -1 )
          {
            m_vectItems[m_iPushedItem].dwButtonStyle |= tItemInfo::IF_PUSHED;
            InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
            PushButton( m_iPushedItem );
            return;
          }
        }
      }

      if ( ( m_iHighlightedItem == m_iPushedItem )
      &&   ( m_iHighlightedItem != -1 ) )
      {
        m_vectItems[m_iHighlightedItem].dwButtonStyle |= tItemInfo::IF_PUSHED;
      }
      if ( m_iPushedItem == -1 )
      {
        m_vectItems[m_iHighlightedItem].dwButtonStyle |= tItemInfo::IF_HILIGHTED;
      }
      InvalidateRect( &m_vectItems[iOverButton].rcButton );
      if ( m_dwTimerID == 0 )
      {
        m_dwTimerID = (DWORD)SetTimer( 7, 50, NULL );
      }
    }
  }

  if ( ( m_dwTimerID != 0 )
  &&   ( m_iHighlightedItem == -1 ) )
  {
    KillTimer( m_dwTimerID );
    m_dwTimerID = 0;
  }

}



LRESULT GRUIMenuBar::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_LBUTTONDOWN:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        OnLButtonDown( (UINT)wParam, pt );
      }
      return 0;
    case WM_LBUTTONUP:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnLButtonUp( (UINT)wParam, pt );
      }
      break;
    case WM_MOUSEMOVE:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        OnMouseMove( (UINT)wParam, pt );
      }
      break;
  }

  return GRUIControlBar::WindowProc( uMsg, wParam, lParam );

}



size_t GRUIMenuBar::MenuItemFromPoint( POINT point )
{

  size_t   iOverButton = -1;

  for ( size_t i = 0; i < m_vectItems.size(); ++i )
  {
    tItemInfo&    ItemInfo = m_vectItems[i];

    if ( !( ItemInfo.dwButtonStyle & tItemInfo::IF_DISABLED ) )
    {
      if ( PtInRect( &ItemInfo.rcButton, point ) )
      {
        iOverButton = i;
        return iOverButton;
      }
    }
  }

  return iOverButton;

}



void GRUIMenuBar::ShowPopupMenu( size_t iItem, BOOL bMouseActivated )
{

  INT nID = 0;

  //while ( true )
  {
    POINT   point;

    if ( m_bHorizontal )
    {
      point.x = m_vectItems[m_iPushedItem].rcButton.left;
      point.y = m_vectItems[m_iPushedItem].rcButton.bottom;
    }
    else
    {
      point.x = m_vectItems[m_iPushedItem].rcButton.right;
      point.y = m_vectItems[m_iPushedItem].rcButton.top;
    }

	  /// Select first popup item, if selected by keyboard
	  if ( FALSE == bMouseActivated )
	  {
		  keybd_event( VK_DOWN, 0, 0, 0 );
		  keybd_event( VK_DOWN, 0, KEYEVENTF_KEYUP, 0 );
	  }

	  TPMPARAMS tpm;
	  tpm.cbSize = sizeof(TPMPARAMS);
	  GetClientRect( &tpm.rcExclude );
	  INT nAlignFlags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD;

    if ( !IsFloating() )
    {
      if ( m_dwMRUDockPos & GRUIDockFrame::DFP_RIGHT )
      {
        nAlignFlags &= ~TPM_LEFTALIGN;
        nAlignFlags |= TPM_RIGHTALIGN;
        point.x = m_vectItems[m_iPushedItem].rcButton.left;
      }
      else if ( m_dwMRUDockPos & GRUIDockFrame::DFP_BOTTOM )
      {
        nAlignFlags &= ~TPM_TOPALIGN;
        nAlignFlags |= TPM_BOTTOMALIGN;
        point.y = m_vectItems[m_iPushedItem].rcButton.top;
      }
    }

    RECT    m_rctCur;
    GetClientRect( &m_rctCur );
	  if ( !m_bHorizontal )
	  {
		  tpm.rcExclude.left = m_rctCur.left;
		  tpm.rcExclude.right = m_rctCur.right;
		  nAlignFlags |= TPM_HORIZONTAL;
	  }
	  else
	  {
		  tpm.rcExclude.top = m_rctCur.top;
		  tpm.rcExclude.bottom = m_rctCur.bottom;
		  nAlignFlags |= TPM_VERTICAL;
	  }

	  ClientToScreen( &point );
	  ClientToScreen( &tpm.rcExclude );
    m_bReOpenPopupMenu = false;

    SetForegroundWindow();

    size_t   iOldPushed = m_iPushedItem;

    m_bTrackingMenu = true;

    m_vectItems[m_iPushedItem].pSubMenu->m_pMenuBar = this;
    nID = m_vectItems[m_iPushedItem].pSubMenu->TrackPopupMenu( nAlignFlags, point.x, point.y, GetParent(), NULL );
    m_bTrackingMenu = false;

    UpdateWindow();

    POINT     ptMouse;

    GetCursorPos( &ptMouse );

    ScreenToClient( &ptMouse );

    size_t    iMenuItem = MenuItemFromPoint( ptMouse );


    if ( m_bReOpenPopupMenu )
    {
      if ( m_iPushedItem != -1 )
      {
        m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
        InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
        m_iPushedItem = -1;
      }
      m_iPushedItem = iMenuItem;

      if ( m_iPushedItem != -1 )
      {
        PostMessage( WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM( point.x, point.y ) );
        /*
        mouse_event( MOUSEEVENTF_ABSOLUTE,
                    ptMouse.x * 65535 / GetSystemMetrics( SM_CXSCREEN ),
                    ptMouse.y * 65535 / GetSystemMetrics( SM_CYSCREEN ), 0, 0 );
        mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0 );
        */
      }
    }
    if ( ( m_iPushedItem != -1 )
    &&   ( m_iPushedItem != iOldPushed )
    &&   ( nID == 0 ) )
    {
      // ein anderes Menü-Item ausgewählt...
      int iWidth = GetSystemMetrics( SM_CXSCREEN );
      int iHeight = GetSystemMetrics( SM_CYSCREEN );

      {

        if ( m_iPushedItem != -1 )
        {
          m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
          InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
          m_iPushedItem = -1;
        }
        if ( m_iHighlightedItem != -1 )
        {
          m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
          InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
          m_iHighlightedItem = -1;
        }
      }
    }

    if ( ( nID == 0 )
    &&   ( iMenuItem == -1 ) )
    {
      //mouse_event( MOUSEEVENTF_LEFTUP, 0, 0, 0, 0 );
      if ( m_iPushedItem != -1 )
      {
        m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
        InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
        m_iPushedItem = -1;
      }
      if ( m_iHighlightedItem != -1 )
      {
        m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
        InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
        m_iHighlightedItem = -1;
      }
    }

    if ( nID )
    {
      // etwas ausgewählt
      if ( m_iPushedItem != -1 )
      {
        m_vectItems[m_iPushedItem].dwButtonStyle &= ~tItemInfo::IF_PUSHED;
        InvalidateRect( &m_vectItems[m_iPushedItem].rcButton );
        m_iPushedItem = -1;
      }
      if ( m_iHighlightedItem != -1 )
      {
        m_vectItems[m_iHighlightedItem].dwButtonStyle &= ~tItemInfo::IF_HILIGHTED;
        InvalidateRect( &m_vectItems[m_iHighlightedItem].rcButton );
        m_iHighlightedItem = -1;
      }
      //break;
    }

  }

  if ( ( nID )
  ||   ( !m_bReOpenPopupMenu ) )
  {
    // entweder vom Popupmenü etwas ausgewählt oder irgendwo ausserhalb geklickt
    m_bKeepPushing = false;
  }

  if ( nID )
  {
    // TODO
    if ( GRUIMDIApp::m_pMDIApp )
    {
      GRUIMDIApp::m_pMDIApp->SendMessage( WM_EXITMENULOOP, (WPARAM)FALSE, 0 );
      GRUIMDIApp::m_pMDIApp->PostMessage( WM_COMMAND, nID, 0 );
    }
    //GetTopLevelFrame()->SendMessage( WM_EXITMENULOOP, (WPARAM)FALSE );
    //GetTopLevelFrame()->PostMessage( WM_COMMAND, nID );
  }

	UpdateWindow();

}



void GRUIMenuBar::PushButton( size_t iIndex )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return;
  }

  tItemInfo&    ItemInfo = m_vectItems[m_iPushedItem];
  if ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUBTN )
  {
    SetCapture();
  }
  else if ( ItemInfo.pSubMenu == NULL )
  {
    // TODO
    //GetTopLevelFrame()->PostMessage( WM_COMMAND, MAKEWPARAM( m_vectItems[m_iPushedItem].dwCommandID, 0 ), (LPARAM)GetSafeHwnd() );
  }
  else
  {
    if ( !m_bTrackingMenu )
    {
      ShowPopupMenu( m_iPushedItem, TRUE );
    }
  }

}