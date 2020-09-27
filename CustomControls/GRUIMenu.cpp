#include "GRUIMenu.h"
#include "GRUIMenuBar.h"
#include "GRUICmdManager.h"



CGRUIMenu::CGRUIMenu() :
  m_iWidth( 20 ),
  m_iItemHeight( m_pDisplayClass->m_DisplayConstants.m_iMenuItemHeight ),
  m_iSeparatorHeight( m_pDisplayClass->m_DisplayConstants.m_iMenuSeparatorHeight ),
  m_iIndent( 20 ),
  m_iShadowWidth( 4 ),
  m_bCaptureMouse( false ),
  m_pOpenedSubMenu( false ),
  m_pParentMenu( NULL ),
  m_hdcDesktop( NULL ),
  m_hdcRightSide( NULL ),
  m_hdcBottomSide( NULL ),
  m_hbmRightSide( NULL ),
  m_hbmBottomSide( NULL ),
  m_hWndMenuOwner( NULL ),
  m_iBorderWidth( m_pDisplayClass->m_DisplayConstants.m_iMenuBorderWidth ),
  m_iBorderHeight( m_pDisplayClass->m_DisplayConstants.m_iMenuBorderHeight ),
  m_dwDefaultItem( -1 ),
  m_iSelectedItem( -1 ),
  m_bTimerSet( false ),
  m_pMenuBar( NULL )
{

  _RegisterClass();

  BOOL    bDropShadowEnabled;
  SystemParametersInfo( 0x1024, 0, &bDropShadowEnabled, 0 );    // SPI_GETDROPSHADOW

  m_bDropShadowEnabled = bDropShadowEnabled ? true : false;

  // kein Windows-Schatten, also selber machen
  m_bDrawShadow = !m_bDropShadowEnabled;

}

CGRUIMenu::~CGRUIMenu()
{

  std::vector<tGRUIMenuItem>::iterator   it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( MenuItem.dwFlags & MF_POPUP )
    {
      delete (CGRUIMenu*)MenuItem.dwID;
      MenuItem.dwID = 0;
    }
    if ( MenuItem.hiconUnchecked )
    {
      DestroyIcon( MenuItem.hiconUnchecked );
      MenuItem.hiconUnchecked = NULL;
    }
    if ( MenuItem.hiconChecked )
    {
      DestroyIcon( MenuItem.hiconChecked );
      MenuItem.hiconChecked = NULL;
    }

    ++it;
  }
  m_vectMenuItems.clear();

}

BOOL CGRUIMenu::AppendMenu( UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, DWORD dwItemData )
{

  m_vectMenuItems.push_back( tGRUIMenuItem() );

  tGRUIMenuItem&    Item = m_vectMenuItems.back();

  if ( lpszNewItem )
  {
    Item.strItemText  = lpszNewItem;
  }
  Item.dwID         = nIDNewItem;
  Item.dwItemData   = dwItemData;
  Item.dwFlags      = nFlags;

  if ( nFlags & MF_POPUP )
  {
    CGRUIMenu*    pSubMenu = (CGRUIMenu*)nIDNewItem;

    pSubMenu->m_pParentMenu = this;
  }

  if ( GetSafeHwnd() )
  {
    RecalcSize();
    Invalidate();
  }

  return TRUE;

}

BOOL CGRUIMenu::TrackPopupMenu( UINT nFlags, int x, int y, HWND hwnd, LPCRECT lpRect )
{

  m_dwChosenMenuItem  = 0;
  m_iSelectedItem     = -1;

  if ( GetSafeHwnd() != NULL )
  {
    return 0;
  }

  if ( ( hwnd )
  &&   ( !( nFlags & TPM_NONOTIFY ) ) )
  {
    ::SendMessage( hwnd, WM_ENTERMENULOOP, TRUE, 0 );
  }

  m_hWndMenuOwner = hwnd;

  RECT    rc;

  rc.left = x;
  rc.top = y;
  rc.right = x + 14;
  rc.bottom = y + 14;

  CreateEx( WS_EX_TOOLWINDOW,
            "GR_CUSTOMWND_MENU",
            "",
            m_pDisplayClass->m_DisplayConstants.m_dwMenuWindowStyles,
            rc,
            hwnd,
            0 );


  RecalcSize();

  RECT  rcWnd;
  GetWindowRect( &rcWnd );

  if ( nFlags & TPM_RIGHTALIGN )
  {
    OffsetRect( &rcWnd, -( rcWnd.right - rcWnd.left ), 0 );
  }
  if ( nFlags & TPM_BOTTOMALIGN )
  {
    OffsetRect( &rcWnd, 0, -( rcWnd.bottom - rcWnd.top ) );
  }

  if ( rcWnd.top < 0 )
  {
    OffsetRect( &rcWnd, 0, -rcWnd.top );
  }
  if ( rcWnd.bottom >= GetSystemMetrics( SM_CYSCREEN ) )
  {
    OffsetRect( &rcWnd, 0, GetSystemMetrics( SM_CYSCREEN ) - rcWnd.bottom );
  }
  if ( rcWnd.left < 0 )
  {
    OffsetRect( &rcWnd, -rcWnd.left, 0 );
  }
  if ( rcWnd.right >= GetSystemMetrics( SM_CXSCREEN ) )
  {
    int   iWidth = rcWnd.right - rcWnd.left;

    rcWnd.left = x - iWidth;
    rcWnd.right = rcWnd.left + iWidth;
  }
  MoveWindow( &rcWnd );

  SaveShadowBackground();

  SetWindowPos( HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );

  SetCapture();
  if ( GetCapture() != m_hWnd )
  {
    dh::Log( "failed to capture\n" );
  }
  else
  {
    //dh::Log( "ich capture\n" );
  }

  if ( m_pMenuBar )
  {
    // auch für MenuBar mitdenken
    SetTimer( 11, 20, NULL );
  }

  while ( GetSafeHwnd() )
  {
    if ( CWnd::PumpMessage() <= 0 )
    {
      break;
    }
  }

  if ( ( hwnd )
  &&   ( ::IsWindow( hwnd ) )
  &&   ( !( nFlags & TPM_NONOTIFY ) ) )
  {
    ::SendMessage( hwnd, WM_EXITMENULOOP, TRUE, 0 );
  }

  DeleteObject( m_hbmDesktop );

  if ( !( nFlags & TPM_NONOTIFY ) )
  {
    if ( ( hwnd )
    &&   ( ::IsWindow( hwnd ) ) )
    {
      if ( ( m_dwChosenMenuItem == SC_CLOSE )
      ||   ( m_dwChosenMenuItem == SC_RESTORE )
      ||   ( m_dwChosenMenuItem == SC_MOVE )
      ||   ( m_dwChosenMenuItem == SC_SIZE )
      ||   ( m_dwChosenMenuItem == SC_MINIMIZE )
      ||   ( m_dwChosenMenuItem == SC_MAXIMIZE ) )
      {
        ::PostMessage( hwnd, WM_SYSCOMMAND, MAKEWPARAM( m_dwChosenMenuItem, 0 ), (LPARAM)GetSafeHwnd() );
      }
      else
      {
        ::PostMessage( hwnd, WM_COMMAND, MAKEWPARAM( m_dwChosenMenuItem, 0 ), (LPARAM)GetSafeHwnd() );
      }
    }
  }

  if ( nFlags & TPM_RETURNCMD )
  {
    return (BOOL)m_dwChosenMenuItem;
  }

  return TRUE;

}

BOOL CGRUIMenu::EnableMenuItem( UINT_PTR nID, BOOL bOn )
{

  // auch rekursiv durchsuchen!
  std::vector<tGRUIMenuItem>::iterator   it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( MenuItem.dwFlags & MF_POPUP )
    {
      CGRUIMenu*  pODSubMenu = (CGRUIMenu*)MenuItem.dwID;

      if ( pODSubMenu->EnableMenuItem( nID, bOn ) )
      {
        return TRUE;
      }
    }
    if ( MenuItem.dwID == nID )
    {
      if ( bOn )
      {
        MenuItem.dwFlags &= ~MF_DISABLED;
      }
      else
      {
        MenuItem.dwFlags |= MF_DISABLED;
      }
      return TRUE;
    }
    ++it;
  }

  return FALSE;

}

BOOL CGRUIMenu::SetMenuItemIcons( UINT uPosition, UINT uFlags, HICON hIconChecked, HICON hIconUnchecked )
{

  tGRUIMenuItem*  pItem = FindMenuItem( uPosition, uFlags );

  if ( pItem == NULL )
  {
    return FALSE;
  }

  pItem->hiconChecked   = hIconChecked;
  pItem->hiconUnchecked = hIconUnchecked;

  if ( GetSafeHwnd() )
  {
    RecalcSize();
    Invalidate();
  }

  return TRUE;

}


/*
void CGRUIMenu::DrawMenuItem( CGRUIMenu* pMenu, HDC dc, RECT& rc, tGRUIMenuItem& MenuItem )
{
  bool    bSelected = false;

  POINT     ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );
  if ( ( PtInRect( &rc, ptMouse ) )
  &&   ( !( MenuItem.dwFlags & MF_DISABLED ) )
  &&   ( !( MenuItem.dwFlags & MF_SEPARATOR ) ) )
  {
    // selektierte Menü-items
    bSelected = true;
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        //FillSolidRect( dc, &rc, GetSysColor( COLOR_HIGHLIGHT ) );
        SetTextColor( dc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        break;
      case ODMS_XP:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_iIndent;
          //FillSolidRect( dc, &rc2, GetSysColor( COLOR_BTNFACE ) );

          rc2 = rc;
          rc2.left = m_iIndent;
          //FillSolidRect( dc, &rc2, RGB( 255, 255, 255 ) );

          HBRUSH    hBrush = CreateSolidBrush( RGB( 145, 176, 207 ) );
          HBRUSH    hOldBrush = (HBRUSH)SelectObject( dc, hBrush );

          rc2 = rc;
          InflateRect( &rc2, -1, -1 );

          HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 38, 87, 137 ) );
          HPEN  hOldPen = (HPEN)SelectObject( dc, hPen );
          Rectangle( dc, rc2.left, rc2.top, rc2.right, rc2.bottom );
          SelectObject( dc, hOldPen );
          DeleteObject( hPen );

          SelectObject( dc, hOldBrush );
          DeleteObject( hBrush );

          SetTextColor( dc, RGB( 255, 255, 255 ) );
        }
        break;
      case ODMS_BLACKMARBLE:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_iIndent;
          //FillSolidRect( dc, &rc2, RGB( 32, 32, 32 ) );

          rc2 = rc;
          rc2.left = m_iIndent;
          //FillSolidRect( dc, &rc2, RGB( 0, 0, 0 ) );

          SetTextColor( dc, 0 );

          HBRUSH    hBrush = CreateSolidBrush( RGB( 255, 32, 32 ) );
          HBRUSH    hOldBrush = (HBRUSH)SelectObject( dc, hBrush );

          rc2 = rc;
          InflateRect( &rc2, -1, -1 );

          HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 255, 200, 32 ) );
          HPEN  hOldPen = (HPEN)SelectObject( dc, hPen );
          Rectangle( dc, rc2.left, rc2.top, rc2.right, rc2.bottom );
          SelectObject( dc, hOldPen );
          DeleteObject( hPen );

          SelectObject( dc, hOldBrush );
          DeleteObject( hBrush );
        }
        break;
    }
  }
  else
  {
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        FillSolidRect( dc, &rc, GetSysColor( COLOR_BTNFACE ) );
        SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
        break;
      case ODMS_XP:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_iIndent;
          //FillSolidRect( dc, &rc2, GetSysColor( COLOR_BTNFACE ) );

          rc2 = rc;
          rc2.left = m_iIndent;
          //FillSolidRect( dc, &rc2, RGB( 255, 255, 255 ) );

          SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
        }
        break;
      case ODMS_BLACKMARBLE:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_iIndent;
          FillSolidRect( dc, &rc2, RGB( 64, 64, 64 ) );

          rc2 = rc;
          rc2.left = m_iIndent;
          FillSolidRect( dc, &rc2, RGB( 0, 0, 0 ) );

          SetTextColor( dc, RGB( 128, 128, 128 ) );
        }
        break;
    }
  }

  if ( MenuItem.dwFlags & MF_SEPARATOR )
  {
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        Draw3dRect( dc, 2, rc.top + 1, rc.right - rc.left - 4, 2, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
        break;
      case ODMS_XP:
        FillSolidRect( dc, m_iIndent + 2, rc.top, rc.right - rc.left - m_iIndent - 4, 1, GetSysColor( COLOR_3DSHADOW ) );
        break;
    }
  }
  else
  {
    rc.left = m_iIndent + 3;
    if ( MenuItem.dwFlags & MF_DISABLED )
    {
      SetTextColor( dc, RGB( 128, 128, 128 ) );
    }

    int   iTimes = 1;
    if ( m_dwDefaultItem == MenuItem.dwID )
    {
      iTimes = 2;
    }

    RECT    rcT = rc;
    for ( int iT = 0; iT < iTimes; ++iT )
    {
      rc = rcT;
      rc.left += iT;
      if ( MenuItem.strItemText.find( '\t' ) != GR::String::npos )
      {
        GR::String   strLeft   = MenuItem.strItemText.substr( 0, MenuItem.strItemText.find( '\t' ) );
        GR::String   strRight  = MenuItem.strItemText.substr( MenuItem.strItemText.find( '\t' ) + 1 );

        DrawText( dc, rc, strLeft.c_str(), strLeft.length(), DT_LEFT | DT_SINGLELINE | DT_VCENTER );
        rc.right -= 20;
        DrawText( dc, rc, strRight.c_str(), strRight.length(), DT_RIGHT | DT_SINGLELINE | DT_VCENTER );
      }
      else
      {
        DrawText( dc, rc, MenuItem.strItemText.c_str(), MenuItem.strItemText.length(), DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER );
      }
    }

    if ( MenuItem.dwFlags & MF_CHECKED )
    {
      rc.left = 2;
      rc.right = m_iIndent;
      if ( MenuItem.hiconChecked )
      {
        DisplayMenuIcon( rc, dc, MenuItem.hiconChecked );
      }
      else
      {
        DisplayMenuIcon( rc, dc, m_hIconCheck );
      }
    }
    else if ( MenuItem.hiconChecked )
    {
      rc.left = 2;
      rc.right = m_iIndent;

      if ( ( m_odStyle == ODMS_XP )
      &&   ( bSelected ) )
      {
        HICON   hIconGreyed = CWinUtils::CreateGrayscaleIcon( MenuItem.hiconChecked, true );
        DisplayMenuIcon( rc, dc, hIconGreyed );
        DestroyIcon( hIconGreyed );
        OffsetRect( &rc, -2, -2 );
        DisplayMenuIcon( rc, dc, MenuItem.hiconChecked );
        OffsetRect( &rc, 2, 2 );
      }
      else if ( ( m_odStyle == ODMS_XP )
      &&        ( !bSelected ) )
      {
        HICON   hIconGreyed = CWinUtils::CreateGrayscaleIcon( MenuItem.hiconChecked );
        DisplayMenuIcon( rc, dc, hIconGreyed );
        DestroyIcon( hIconGreyed );
      }
      else
      {
        DisplayMenuIcon( rc, dc, MenuItem.hiconChecked );
      }
    }
    else if ( MenuItem.hiconUnchecked )
    {
      rc.left = 2;
      rc.right = m_iIndent;

      DisplayMenuIcon( rc, dc, MenuItem.hiconUnchecked );
    }

    // Popup-Menü - Pfeil nach rechts
    if ( MenuItem.dwFlags & MF_POPUP )
    {
      rc.right = m_iWidth;
      rc.left = rc.right - 24;

      DisplayMenuIcon( rc, dc, m_hIconPopupArrow );
    }
  }

}
*/

void CGRUIMenu::DrawBackground( HDC hdc, const RECT& rcRedraw )
{
  if ( m_pWndBackgroundProducer )
  {
    CCustomWnd::DrawBackground( hdc, rcRedraw );
  }
}

void CGRUIMenu::Draw( HDC hdc, const RECT& rc2 )
{
  RECT      rcClient,
            rc = rc2;

  rcClient = rc;

  if ( m_bDrawShadow )
  {
    rcClient.right  -= m_iShadowWidth;
    rcClient.bottom -= m_iShadowWidth;
  }

  m_iItemHeight = m_pDisplayClass->m_DisplayConstants.m_iMenuItemHeight;

  rc = rcClient;

  SetBkMode( hdc, TRANSPARENT );

  POINT     ptMouse;

  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

  int       iIndex = 0;

  std::vector<tGRUIMenuItem>::iterator   it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    GetItemRect( iIndex, rc );

    m_pDisplayClass->PaintMenuItem( hdc, rc, m_iIndent, MenuItem, m_pWndBackgroundProducer == NULL );
    //DrawMenuItem( hdc, rc, MenuItem );


    iIndex++;
    ++it;
  }

  CCustomWnd::Draw( hdc, rc2 );

}

void CGRUIMenu::PostNcDestroy()
{

  if ( m_pOpenedSubMenu )
  {
    m_pOpenedSubMenu->DestroyWindow();
    m_pOpenedSubMenu = NULL;
  }
  if ( m_pParentMenu )
  {
    m_pParentMenu->m_pOpenedSubMenu = NULL;
  }

  // Schatten wieder wegmachen
  if ( m_bDrawShadow )
  {
    if ( m_hdcDesktop )
    {
      DeleteDC( m_hdcDesktop );
      m_hdcDesktop = NULL;
    }
    if ( m_hdcRightSide )
    {
      DeleteDC( m_hdcRightSide );
      m_hdcRightSide = NULL;
    }
    if ( m_hdcBottomSide )
    {
      DeleteDC( m_hdcBottomSide );
      m_hdcBottomSide = NULL;
    }
    if ( m_hbmRightSide )
    {
      DeleteObject( m_hbmRightSide );
      m_hbmRightSide = NULL;
    }
    if ( m_hbmBottomSide )
    {
      DeleteObject( m_hbmBottomSide );
      m_hbmBottomSide = NULL;
    }
  }

  CCustomWnd::PostNcDestroy();
}

void CGRUIMenu::OnMouseMove( UINT nFlags, POINT point)
{

  CCustomWnd::OnMouseMove( nFlags, point );

  if ( !m_bCaptureMouse )
  {
    m_bCaptureMouse = true;
    if ( GetCapture() != m_hWnd )
    {
      SetCapture();
    }
  }
  else
  {
    ClientToScreen( &point );

    HWND    hwnd = ::WindowFromPoint( point );

    if ( hwnd != GetSafeHwnd() )
    {
      CGRUIMenu*    pOtherMenu = IsPartOfMenuChain( hwnd );

      if ( pOtherMenu )
      {
        if ( GetCapture() == m_hWnd )
        {
          ReleaseCapture();
        }
        m_bCaptureMouse = false;

        pOtherMenu->m_bCaptureMouse = true;
        pOtherMenu->SetCapture();
        Invalidate();
      }
      /*
      else if ( GetCapture() == this )
      {
        ReleaseCapture();
        m_bCaptureMouse = false;
        GetTopLevelMenu()->DestroyChildChain();
      }
      */
    }
    else
    {
      if ( !m_bTimerSet )
      {
        m_bTimerSet = true;
        SetTimer( 17, 20 );
      }

      ScreenToClient( &point );

      size_t  iMouseOverItem = ItemFromPoint( point );

      if ( iMouseOverItem != m_iSelectedItem )
      {
        if ( m_iSelectedItem != -1 )
        {
          m_vectMenuItems[m_iSelectedItem].dwFlags &= ~MF_HILITE;
        }
        m_iSelectedItem = iMouseOverItem;

        if ( m_pDisplayClass->m_pCmdManager )
        {
          if ( m_iSelectedItem != -1 )
          {
            m_pDisplayClass->m_pCmdManager->HoverOverItem( m_vectMenuItems[m_iSelectedItem].dwID );
          }
          else
          {
            m_pDisplayClass->m_pCmdManager->HoverOverItem( 0 );
          }
        }

        if ( m_iSelectedItem != -1 )
        {
          m_vectMenuItems[m_iSelectedItem].dwFlags |= MF_HILITE;
        }
        Invalidate();
      }
    }
  }

}

CGRUIMenu* CGRUIMenu::IsPartOfMenuChain( HWND hWnd )
{

  if ( hWnd == GetSafeHwnd() )
  {
    return this;
  }
  CGRUIMenu*    pMenu = this;
  while ( pMenu->m_pParentMenu )
  {
    pMenu = pMenu->m_pParentMenu;
    if ( pMenu->GetSafeHwnd() == hWnd )
    {
      return pMenu;
    }
  }

  pMenu = this;
  while ( pMenu->m_pOpenedSubMenu )
  {
    pMenu = pMenu->m_pOpenedSubMenu;
    if ( pMenu->GetSafeHwnd() == hWnd )
    {
      return pMenu;
    }
  }
  return NULL;

}

CGRUIMenu* CGRUIMenu::GetTopLevelMenu()
{

  CGRUIMenu*    pMenu = this;
  while ( pMenu )
  {
    if ( pMenu->m_pParentMenu == NULL )
    {
      break;
    }
    pMenu = pMenu->m_pParentMenu;
  }
  return pMenu;

}

void CGRUIMenu::OnLButtonDown( UINT nFlags, POINT point )
{

  int   iItem = ItemFromPoint( point );

  if ( iItem != -1 )
  {
    tGRUIMenuItem&    MenuItem = m_vectMenuItems[iItem];

    if ( MenuItem.dwFlags & MF_DISABLED )
    {
      //BASE::OnLButtonDown(nFlags, point);
      return;
    }
    if ( MenuItem.dwFlags & MF_POPUP )
    {
      if ( m_pOpenedSubMenu )
      {
        if ( m_pOpenedSubMenu == (CGRUIMenu*)MenuItem.dwID )
        {
          // das Submenü ist schon offen
          return;
        }

        // altes geöffnetes Submenü schließen
        m_pOpenedSubMenu->DestroyChildChain();
        m_pOpenedSubMenu = NULL;
        if ( m_bDrawShadow )
        {
          ::RedrawWindow( GetDesktopWindow(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
        }
      }

      RECT    rcItem;

      GetItemRect( iItem, rcItem );
      ClientToScreen( &rcItem );

      CGRUIMenu*    pSubMenu = (CGRUIMenu*)MenuItem.dwID;

      OffsetRect( &rcItem, rcItem.right - rcItem.left, 0 );

      m_pOpenedSubMenu = pSubMenu;
      if ( pSubMenu->GetSafeHwnd() == NULL )
      {
        pSubMenu->CreateEx( 0, "GR_CUSTOMWND_MENU", "", m_pDisplayClass->m_DisplayConstants.m_dwMenuWindowStyles, rcItem, m_hWnd, 0 );
        pSubMenu->RecalcSize();
        pSubMenu->SetOwner( m_hWnd);
        pSubMenu->SaveShadowBackground();
      }
      pSubMenu->SetWindowPos( m_hWnd, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );
      //pSubMenu->ShowWindow( SW_SHOWNA );
      return;
    }
    else if ( MenuItem.dwFlags & MF_SEPARATOR )
    {
      //BASE::OnLButtonDown(nFlags, point);
      return;
    }

    // ein Menü-Item wurde ausgewählt
    CGRUIMenu*    pMenu = GetTopLevelMenu();

    MenuItem.dwFlags &= ~MF_HILITE;
    pMenu->m_dwChosenMenuItem = MenuItem.dwID;
    // das oberste Menü zerstört alle anderen Childs mit sich

    pMenu->DestroyChildChain();
  }
  else
  {
    if ( GetCapture() == m_hWnd )
    {
      ReleaseCapture();
    }

    CGRUIMenu*    pMenu = GetTopLevelMenu();
    pMenu->DestroyWindow();

    //mouse_event( MOUSEEVENTF_ABSOLUTE, point.x * 65535 / iWidth, point.y * 65535 / iHeight, 0, 0 );
    // Mousedown an Fenster darunter weitergeben
    mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0 );
    return;
  }

  //BASE::OnLButtonDown(nFlags, point);
}

void CGRUIMenu::DestroyChildChain()
{

  if ( m_pOpenedSubMenu )
  {
    m_pOpenedSubMenu->DestroyChildChain();
    m_pOpenedSubMenu = NULL;
  }

  if ( m_hWnd )
  {
    RECT    rc;
    GetWindowRect( &rc );

    DestroyWindow();

    if ( m_bDrawShadow )
    {
      HWND    hwndDesktop = GetDesktopWindow();

      ::InvalidateRect( hwndDesktop, &rc, TRUE );
      ::RedrawWindow( hwndDesktop, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
      ::UpdateWindow( hwndDesktop );

      for ( int i = 0; i < 20; ++i )
      {
        MSG   msg;
        if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
          if ( msg.message == WM_QUIT )
          {
            break;
          }
          CWnd::PumpMessage();
        }
      }
    }
  }

}

bool CGRUIMenu::GetItemRect( size_t iIndex, RECT& rc )
{

  memset( &rc, 0, sizeof( RECT ) );

  if ( iIndex >= m_vectMenuItems.size() )
  {
    return false;
  }

  rc = m_vectMenuItems[iIndex].rcSize;
  return true;

}

int CGRUIMenu::ItemFromPoint( const POINT& pt )
{

  RECT    rc;

  GetClientRect( &rc );

  if ( !PtInRect( &rc, pt ) )
  {
    return -1;
  }

  int iTopHeight = 0;

  int   iIndex = 0;
  std::vector<tGRUIMenuItem>::iterator    it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( PtInRect( &MenuItem.rcSize, pt ) )
  {
        return iIndex;
      }

    ++iIndex;
    ++it;
  }

    return -1;

}

void CGRUIMenu::RecalcSize()
{

  if ( !m_hWnd )
  {
    return;
  }

  m_iIndent       = m_pDisplayClass->m_DisplayConstants.m_iMenuItemIndent;

  HDC   dc = GetDC();

  RECT    rc;

  GetClientRect( &rc );

  m_iItemHeight = m_pDisplayClass->m_DisplayConstants.m_iMenuItemHeight;

  int     iMaxWidth = 0,
          iHeight = 0;

  rc.bottom = 0;

  std::vector<tGRUIMenuItem>::iterator    it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( MenuItem.dwFlags & MF_SEPARATOR )
    {
      iHeight += m_pDisplayClass->m_DisplayConstants.m_iMenuSeparatorHeight;
      rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iMenuSeparatorHeight;
    }
    else
    {
      int   iItemHeight = m_iItemHeight;

      HICON   hIconCmd = NULL;
      if ( m_pDisplayClass->m_pCmdManager )
      {
        hIconCmd = m_pDisplayClass->m_pCmdManager->GetIcon( MenuItem.dwID );
        if ( hIconCmd == NULL )
        {
          hIconCmd = MenuItem.hiconChecked;
        }
      }

      if ( hIconCmd )
      {
        ICONINFO    ii;

        GetIconInfo( hIconCmd, &ii );

        BITMAP      bm;

        GetObject( ii.hbmColor, sizeof( BITMAP ), &bm );

        bm.bmHeight += 6;
        if ( m_iIndent < bm.bmWidth + 4 )
        {
          m_iIndent = bm.bmWidth + 4;
        }

        if ( bm.bmHeight > iItemHeight )
        {
          iItemHeight = bm.bmHeight;
        }

        DeleteObject( ii.hbmColor );
        DeleteObject( ii.hbmMask );
      }

      GR::String     strMenuItemText = MenuItem.strItemText;

      if ( ( m_pDisplayClass->m_pCmdManager )
      &&   ( m_pDisplayClass->m_pCmdManager->IsKnownCommand( MenuItem.dwID ) )
      &&   ( m_pDisplayClass->m_pCmdManager->HasShortCut( MenuItem.dwID ) ) )
      {
        strMenuItemText += "\t";
        strMenuItemText += m_pDisplayClass->m_pCmdManager->GetShortCutDesc( MenuItem.dwID );
      }

      if ( strMenuItemText.find( '\t' ) != GR::String::npos )
      {
        // hier be tabs
        DrawText( dc, strMenuItemText.c_str(), (int)strMenuItemText.length(), &rc, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );

        rc.right += 20;
      }
      else
      {
        DrawText( dc, strMenuItemText.c_str(), (int)strMenuItemText.length(), &rc, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );
      }
      if ( iItemHeight > rc.bottom - rc.top )
      {
        rc.bottom = rc.top + iItemHeight;
      }
      if ( m_iIndent + 4 + rc.right - rc.left > iMaxWidth )
      {
        iMaxWidth = m_iIndent + 4 + rc.right - rc.left;
      }
      iHeight += rc.bottom - rc.top;
    }

    MenuItem.rcSize = rc;

    rc.top = rc.bottom;
    ++it;
  }

  iMaxWidth += 20;    // für den Popup-Pfeil muß Platz sein

  // max. Breiten anpassen
  std::vector<tGRUIMenuItem>::iterator    itMI( m_vectMenuItems.begin() );
  while ( itMI != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *itMI;

    MenuItem.rcSize.right = MenuItem.rcSize.left + iMaxWidth;

    ++itMI;
  }

  if ( m_bDrawShadow )
  {
    iMaxWidth += m_iShadowWidth;
    iHeight += m_iShadowWidth;
  }

  m_iWidth = iMaxWidth;

  ReleaseDC( dc );

  SetWindowPos( NULL, 0, 0,
                iMaxWidth + 2 * m_iBorderWidth,
                iHeight + 2 * m_iBorderHeight,
                SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED );

  SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_FRAMECHANGED );

}

COLORREF CGRUIMenu::ShadowColor( int iX, int iY, int iShadowValue )
{

  COLORREF    rgbDesktop = GetPixel( m_hdcDesktop, iX, iY );

  iShadowValue *= 50;
	iShadowValue >>= 2;

	int r = GetRValue( rgbDesktop ) - iShadowValue;
	int g = GetGValue( rgbDesktop ) - iShadowValue;
	int b = GetBValue( rgbDesktop ) - iShadowValue;

	if ( r < 0 ) r = 0;
	if ( g < 0 ) g = 0;
	if ( b < 0 ) b = 0;

	return RGB( r, g, b );

}

tGRUIMenuItem* CGRUIMenu::FindMenuItem( UINT uPosition, UINT uFlags )
{

  if ( uFlags == MF_BYPOSITION )
  {
    if ( uPosition >= m_vectMenuItems.size() )
    {
      return NULL;
    }
    return &m_vectMenuItems[uPosition];
  }

  std::vector<tGRUIMenuItem>::iterator    it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( MenuItem.dwFlags & MF_POPUP )
    {
      CGRUIMenu*    pODMenu = (CGRUIMenu*)MenuItem.dwID;

      tGRUIMenuItem*    pItem = pODMenu->FindMenuItem( uPosition, uFlags );
      if ( pItem )
      {
        return pItem;
      }
    }
    if ( MenuItem.dwID == uPosition )
    {
      return &MenuItem;
    }

    ++it;
  }

  return NULL;

}

void CGRUIMenu::DeleteAllMenuItems()
{

  std::vector<tGRUIMenuItem>::iterator   it( m_vectMenuItems.begin() );
  while ( it != m_vectMenuItems.end() )
  {
    tGRUIMenuItem&    MenuItem = *it;

    if ( MenuItem.dwFlags & MF_POPUP )
    {
      delete (CGRUIMenu*)MenuItem.dwID;
      MenuItem.dwID = 0;
    }
    if ( MenuItem.hiconUnchecked )
    {
      DestroyIcon( MenuItem.hiconUnchecked );
      MenuItem.hiconUnchecked = NULL;
    }
    if ( MenuItem.hiconChecked )
    {
      DestroyIcon( MenuItem.hiconChecked );
      MenuItem.hiconChecked = NULL;
    }

    ++it;
  }

  m_vectMenuItems.clear();
  RecalcSize();

}

int CGRUIMenu::OnCreate()
{

  if ( m_bDrawShadow )
  {
    m_hdcDesktop = ::CreateDC( "DISPLAY", NULL, NULL, NULL );

    m_hdcRightSide  = ::CreateCompatibleDC( m_hdcDesktop );
    m_hdcBottomSide = ::CreateCompatibleDC( m_hdcDesktop );

    m_hbmRightSide  = CreateCompatibleBitmap( m_hdcDesktop, m_iShadowWidth, GetSystemMetrics( SM_CYSCREEN ) );
    m_hbmBottomSide = CreateCompatibleBitmap( m_hdcDesktop, GetSystemMetrics( SM_CXSCREEN ), m_iShadowWidth );
    m_hbmSavedRightSide  = CreateCompatibleBitmap( m_hdcDesktop, m_iShadowWidth, GetSystemMetrics( SM_CYSCREEN ) );
    m_hbmSavedBottomSide = CreateCompatibleBitmap( m_hdcDesktop, GetSystemMetrics( SM_CXSCREEN ), m_iShadowWidth );
  }

  return 0;
}

void CGRUIMenu::SaveShadowBackground()
{

  if ( m_bDrawShadow )
  {
    SaveDesktop( m_hWnd );

    GetWindowRect( &m_rcShadow );

    SelectObject( m_hdcRightSide, m_hbmSavedRightSide );
    SelectObject( m_hdcBottomSide, m_hbmSavedBottomSide );

    HDC     hdcDesktop = CreateCompatibleDC( m_hdcRightSide );

    ::BitBlt( m_hdcRightSide, 0, 0, m_iShadowWidth, m_rcShadow.bottom - m_rcShadow.top,
              hdcDesktop, m_rcShadow.right - m_iShadowWidth, m_rcShadow.top, SRCCOPY );
    ::BitBlt( m_hdcBottomSide, 0, 0, m_rcShadow.right - m_rcShadow.left, m_iShadowWidth,
              hdcDesktop, m_rcShadow.left, m_rcShadow.bottom - m_iShadowWidth, SRCCOPY );

    DeleteDC( hdcDesktop );

    /*
    BitBlt( m_hdcRightSide, 0, 0, m_iShadowWidth, GetSystemMetrics( SM_CYSCREEN ), m_hdcDesktop, m_rcShadow.right - m_iShadowWidth, m_rcShadow.top, SRCCOPY );
    BitBlt( m_hdcBottomSide, 0, 0, GetSystemMetrics( SM_CXSCREEN ), m_iShadowWidth, m_hdcDesktop, m_rcShadow.left, m_rcShadow.bottom - m_iShadowWidth, SRCCOPY );
    */

    SelectObject( m_hdcRightSide, m_hbmRightSide );
    SelectObject( m_hdcBottomSide, m_hbmBottomSide );

    // Schatten erzeugen
    for ( int i = 0; i < m_iShadowWidth; i++ )
    {
      for ( int j = 0; j < m_iShadowWidth; j++ )
      {
        int   iValue = m_iShadowWidth - i;
        if ( iValue > j )
        {
          iValue = j;
        }
        // Ecke rechts oben
        SetPixel( m_hdcRightSide, i, j,
                  ShadowColor( m_rcShadow.right - m_iShadowWidth + i, m_rcShadow.top + j, iValue ) );
        // Ecke links unten
        SetPixel( m_hdcBottomSide, m_iShadowWidth - 1 - i, m_iShadowWidth - 1 - j,
                  ShadowColor( m_rcShadow.left + m_iShadowWidth - 1- i, m_rcShadow.bottom + m_iShadowWidth - 1- j, iValue ) );
      }
    }
    // Ecke rechts unten
    for ( int i = 0; i < m_iShadowWidth; i++ )
    {
      for ( int j = 0; j < m_iShadowWidth; j++ )
      {
        int   iValue = i;
        if ( j > iValue )
        {
          iValue = j;
        }
        SetPixel( m_hdcRightSide, i, m_rcShadow.bottom - m_rcShadow.top - m_iShadowWidth + j,
                  ShadowColor( m_rcShadow.right - m_iShadowWidth + i, m_rcShadow.bottom - m_iShadowWidth + j, m_iShadowWidth - iValue ) );
      }
    }
    for ( int i = 0; i < m_iShadowWidth; i++ )
    {
      for ( int j = 0; j < m_rcShadow.bottom - m_rcShadow.top - 2 * m_iShadowWidth; j++ )
      {
        SetPixel( m_hdcRightSide, i, j + m_iShadowWidth,
                  ShadowColor( m_rcShadow.right - m_iShadowWidth + i, m_rcShadow.top + j + m_iShadowWidth, m_iShadowWidth - i ) );
      }
    }
    for ( int i = m_iShadowWidth; i < m_rcShadow.right - m_rcShadow.left; i++ )
    {
      for ( int j = 0; j < m_iShadowWidth; j++ )
      {
        SetPixel( m_hdcBottomSide, i, j,
                  ShadowColor( m_rcShadow.left + i, m_rcShadow.bottom - m_iShadowWidth + j, m_iShadowWidth - j ) );
      }
    }
  }

}

void CGRUIMenu::CompleteClose()
{

  CGRUIMenu*    pMenu = GetTopLevelMenu();
  pMenu->DestroyChildChain();

}

void CGRUIMenu::OnSysCommand(UINT nID, LPARAM lParam)
{

  CompleteClose();

  //CWnd::OnSysCommand( nID, lParam );

}

void CGRUIMenu::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{

  if ( nChar == VK_ESCAPE )
  {
    CompleteClose();
  }

  CCustomWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CGRUIMenu::NotActiveAnymore( HWND hwndNew )
{

  if ( GetCapture() == GetSafeHwnd() )
  {
    ReleaseCapture();
    m_bCaptureMouse = false;
  }

  // ist das neue Wnd ein Child von uns?
  bool    bIsDescendant = false;
  while ( hwndNew )
  {
    if ( hwndNew == GetSafeHwnd() )
    {
      bIsDescendant = true;
      break;
    }
    hwndNew = ::GetWindow( hwndNew, GW_OWNER );
  }

  // ist das neue Wnd ein Parent von uns?
  bool    bIsParent = false;
  bool    bIsSibling = false;
  CGRUIMenu*    pMenu = m_pParentMenu;
  while ( pMenu )
  {
    if ( pMenu->GetSafeHwnd() == hwndNew )
    {
      bIsParent = true;
    }
    if ( ( pMenu->m_pOpenedSubMenu )
    &&   ( pMenu->m_pOpenedSubMenu->GetSafeHwnd() == hwndNew ) )
    {
      bIsSibling = true;
    }

    if ( pMenu->m_pParentMenu == NULL )
    {
      break;
    }
    pMenu = pMenu->m_pParentMenu;
  }

  if ( bIsParent )
  {
    DestroyChildChain();
  }
  else if ( !bIsDescendant )
  {
    // weder Parent noch Child
    DestroyChildChain();
  }

}

void CGRUIMenu::OnSysKeyDown()
{

  CompleteClose();

}

/*
virtual void CGRUIMenu::DrawBackground2( HDC hdc, const RECT& rcBG )
{

  if ( !m_bDrawShadow )
  {
    Default();
    return;
  }

  RECT    rc = rcBG;

  rc.right  -= m_iShadowWidth;
  rc.bottom -= m_iShadowWidth;

  OffsetRect( &rc, -rc.left, -rc.top );

  if ( hdc )
  {
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        FillSolidRect( hdc, rc.left, rc.top, m_iBorderWidth, rc.bottom - rc.top, GetSysColor( COLOR_BTNFACE ) );
        FillSolidRect( hdc, rc.left, rc.top, rc.right - rc.left, m_iBorderHeight, GetSysColor( COLOR_BTNFACE ) );
        FillSolidRect( hdc, rc.right - m_iBorderWidth, rc.top, m_iBorderWidth, rc.bottom - rc.top, GetSysColor( COLOR_BTNFACE ) );
        FillSolidRect( hdc, rc.left, rc.bottom - m_iBorderHeight, rc.right - rc.left, m_iBorderHeight, GetSysColor( COLOR_BTNFACE ) );

        DrawEdge( hdc, &rc, EDGE_RAISED, BF_RECT );
        break;
      case ODMS_XP:
        FillSolidRect( hdc, rc.left, rc.top, m_iBorderWidth, rc.bottom - rc.top, 0 );
        FillSolidRect( hdc, rc.left, rc.top, rc.right - rc.left, m_iBorderHeight, 0 );
        FillSolidRect( hdc, rc.right - m_iBorderWidth, rc.top, m_iBorderWidth, rc.bottom - rc.top, 0 );
        FillSolidRect( hdc, rc.left, rc.bottom - m_iBorderHeight, rc.right - rc.left, m_iBorderHeight, 0 );
        break;
      case ODMS_BLACKMARBLE:
        FillSolidRect( hdc, rc.left, rc.top, m_iBorderWidth, rc.bottom - rc.top, 0 );
        FillSolidRect( hdc, rc.left, rc.top, rc.right - rc.left, m_iBorderHeight, 0 );
        FillSolidRect( hdc, rc.right - m_iBorderWidth, rc.top, m_iBorderWidth, rc.bottom - rc.top, 0 );
        FillSolidRect( hdc, rc.left, rc.bottom - m_iBorderHeight, rc.right - rc.left, m_iBorderHeight, 0 );
        break;
    }

    if ( m_bDrawShadow )
    {
      ::BitBlt( hdc, 0, rc.bottom, rc.right - rc.left + m_iShadowWidth, m_iShadowWidth,
                m_hdcBottomSide, rc.left, 0, SRCCOPY );
      ::BitBlt( hdc, rc.right, 0, m_iShadowWidth, rc.bottom - rc.top + m_iShadowWidth,
                m_hdcRightSide, 0, rc.top, SRCCOPY );
    }
    ReleaseDC( hdc );
  }

}
*/

BOOL CGRUIMenu::DestroyWindow()
{

  if ( GetCapture() == GetSafeHwnd() )
  {
    ReleaseCapture();
    m_bCaptureMouse = false;
  }

  return CCustomWnd::DestroyWindow();
}

void CGRUIMenu::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
  if ( !bActive )
  {
    NotActiveAnymore( NULL );
  }

}

void CGRUIMenu::SaveDesktop( HWND hWnd )
{

  if ( m_hbmDesktop == NULL )
  {
    // Get the desktop hDC...
    HDC hdcDsk = ::GetWindowDC( 0 );

    HDC dc;

    dc = CreateCompatibleDC( hdcDsk );

    RECT  rect;
    ::GetWindowRect( hWnd, &rect );

    m_hbmDesktop = CreateCompatibleBitmap( hdcDsk, rect.right - rect.left, rect.bottom - rect.top );
    HBITMAP  hbmOld = (HBITMAP)SelectObject( dc, m_hbmDesktop );
    BitBlt( dc, 0, 0, rect.right - rect.left + 10, rect.bottom - rect.top + 10, hdcDsk, rect.left, rect.top, SRCCOPY );

    SelectObject( dc, hbmOld );
    // Release the desktop hDC...
    ::ReleaseDC( 0, hdcDsk );
  }

}

int CGRUIMenu::GetMenuItemCount() const
{

  return (int)m_vectMenuItems.size();

}

BOOL CGRUIMenu::CheckMenuItem( UINT_PTR nID, UINT uCheck )
{

  tGRUIMenuItem*  pItem = FindMenuItem( (UINT)nID, uCheck );

  if ( pItem == NULL )
  {
    return FALSE;
  }
  if ( uCheck & MF_CHECKED )
  {
    pItem->dwFlags |= MF_CHECKED;
  }
  else
  {
    pItem->dwFlags &= ~MF_CHECKED;
  }
  return TRUE;

}

LRESULT CGRUIMenu::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
  {
    case WM_TIMER:
      {
        POINT   pt;

        GetCursorPos( &pt );

        if ( wParam == 11 )
        {
          // der Menubar ist auch aktiv
          m_pMenuBar->ScreenToClient( &pt );
          size_t iMBItem = m_pMenuBar->MenuItemFromPoint( pt );
          if ( ( iMBItem != m_pMenuBar->m_iPushedItem )
          &&   ( iMBItem != -1 ) )
          {
            // über einem neuen Item beim MenuBar
            DestroyChildChain();
            m_pMenuBar->m_bReOpenPopupMenu = true;
            return 0;
          }
        }
        else if ( wParam == 17 )
        {
          ScreenToClient( &pt );

          int   iMouseOverItem = ItemFromPoint( pt );
          if ( iMouseOverItem != m_iSelectedItem )
          {
            if ( m_iSelectedItem != -1 )
            {
              m_vectMenuItems[m_iSelectedItem].dwFlags &= ~MF_HILITE;
            }
            m_iSelectedItem = iMouseOverItem;
            if ( m_iSelectedItem != -1 )
            {
              m_vectMenuItems[m_iSelectedItem].dwFlags |= MF_HILITE;
            }
            Invalidate();
            if ( m_iSelectedItem == -1 )
            {
              m_bTimerSet = false;
              KillTimer( 17 );
            }
          }
        }
      }
      break;
    case WM_ACTIVATE:
      OnActivateApp( (BOOL)wParam, 0 );
      break;
    case WM_CREATE:
      OnCreate();
      break;
    case WM_CHAR:
      OnChar( (UINT)wParam, (UINT)lParam & 0x0f, 0 );
      break;
    case WM_SYSKEYDOWN:
      OnSysKeyDown();
      break;
    case WM_NCDESTROY:
      PostNcDestroy();
      break;
    case WM_KILLFOCUS:
      NotActiveAnymore( (HWND)wParam );
      break;
    case WM_MOUSEMOVE:
      {
        POINT   pt;

        pt.x = (int)(short)LOWORD( lParam );
        pt.y = (int)(short)HIWORD( lParam );
        OnMouseMove( (UINT)wParam, pt );
      }
      return 0;
    case WM_LBUTTONDOWN:
      {
        POINT   pt;

        pt.x = (int)(short)LOWORD( lParam );
        pt.y = (int)(short)HIWORD( lParam );
        OnLButtonDown( (UINT)wParam, pt );
      }
      break;
    case WM_RBUTTONDOWN:
      {
        POINT   pt;

        pt.x = (int)(short)LOWORD( lParam );
        pt.y = (int)(short)HIWORD( lParam );
        OnRButtonDown( (UINT)wParam, pt );
      }
      break;
    case WM_LBUTTONUP:
      {
        POINT   pt;

        pt.x = (int)(short)LOWORD( lParam );
        pt.y = (int)(short)HIWORD( lParam );
        OnLButtonUp( (UINT)wParam, pt );
      }
      break;
  }

  return CCustomWnd::WindowProc( message, wParam, lParam );
}

void CGRUIMenu::OnNcLButtonDown( UINT nHitTest, POINT point )
{

  ScreenToClient( &point );

  //OnLButtonDown( 0, point );
  // TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

  //CCustomWnd::OnNcLButtonDown(nHitTest, point);
}

CGRUIMenu* CGRUIMenu::ReplaceMenu( HMENU hMenu )
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

  pODMenu->m_dwDefaultItem = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED );

  return pODMenu;

}

void CGRUIMenu::Clone( HMENU hMenu, HWND hwndSysOwner )
{

  // bildet ein HMenu nach
  DeleteAllMenuItems();

  int iItems = ::GetMenuItemCount( hMenu );

  for ( int i = 0; i < iItems; ++i )
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
      AppendMenu( dwExtraFlags | MF_POPUP, (UINT_PTR)ReplaceMenu( ii.hSubMenu ), szTemp );
    }
    else if ( ii.wID == 0 )
    {
      AppendMenu( dwExtraFlags | MF_SEPARATOR );
    }
    else
    {
      AppendMenu( dwExtraFlags | MF_STRING, ii.wID, szTemp );
    }
  }

  m_dwDefaultItem = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED );

    // per Hand anpassen - PFUIPFUIPFUI
  if ( hwndSysOwner )
  {
    DWORD_PTR   dwStyle = ::GetWindowLongPtr( hwndSysOwner, GWL_STYLE );

    if ( ( !( dwStyle & WS_THICKFRAME ) )
    ||   ( dwStyle & ( WS_MAXIMIZE | WS_MINIMIZE ) ) )
    {
      EnableMenuItem( SC_SIZE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_SIZE );
    }

    if ( dwStyle & WS_MAXIMIZE )
    {
      EnableMenuItem( SC_MOVE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_MOVE );
    }
    if ( ( dwStyle & WS_MINIMIZE )
    ||   ( !( dwStyle & WS_MINIMIZEBOX ) ) )
    {
      EnableMenuItem( SC_MINIMIZE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_MINIMIZE );
    }
    if ( ( dwStyle & WS_MAXIMIZE )
    ||   ( !( dwStyle & WS_MAXIMIZEBOX ) ) )
    {
      EnableMenuItem( SC_MAXIMIZE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_MAXIMIZE );
    }
    if ( ( !( dwStyle & WS_MINIMIZE ) )
    &&   ( !( dwStyle & WS_MAXIMIZE ) ) )
    {
      EnableMenuItem( SC_RESTORE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_RESTORE );
    }

    DWORD   dwClassStyle = ::GetClassLong( hwndSysOwner, GCL_STYLE );
    if ( dwClassStyle & CS_NOCLOSE )
    {
      EnableMenuItem( SC_CLOSE, FALSE );
    }
    else
    {
      EnableMenuItem( SC_CLOSE );
    }
  }

}

void CGRUIMenu::OnRButtonDown( UINT nFlags, POINT point )
{

  int   iItem = ItemFromPoint( point );

  if ( iItem != -1 )
  {
    tGRUIMenuItem&    MenuItem = m_vectMenuItems[iItem];

    if ( MenuItem.dwFlags & MF_DISABLED )
    {
      //CCustomWnd::OnLButtonDown(nFlags, point);
      return;
    }
    if ( MenuItem.dwFlags & MF_POPUP )
    {
      if ( m_pOpenedSubMenu )
      {
        if ( m_pOpenedSubMenu == (CGRUIMenu*)MenuItem.dwID )
        {
          // das Submenü ist schon offen
          return;
        }

        // altes geöffnetes Submenü schließen
        m_pOpenedSubMenu->DestroyChildChain();
        m_pOpenedSubMenu = NULL;
        if ( m_bDrawShadow )
        {
          ::RedrawWindow( GetDesktopWindow(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
        }
      }

      RECT    rcItem;

      GetItemRect( iItem, rcItem );
      ClientToScreen( &rcItem );

      CGRUIMenu*    pSubMenu = (CGRUIMenu*)MenuItem.dwID;

      OffsetRect( &rcItem, rcItem.right - rcItem.left, 0 );

      m_pOpenedSubMenu = pSubMenu;
      if ( pSubMenu->GetSafeHwnd() == NULL )
      {
        pSubMenu->CreateEx( 0, "GR_CUSTOMWND_MENU", "", m_pDisplayClass->m_DisplayConstants.m_dwMenuWindowStyles, rcItem, m_hWnd, 0 );
        pSubMenu->RecalcSize();
        pSubMenu->SetOwner( m_hWnd );
        pSubMenu->SaveShadowBackground();
      }
      pSubMenu->SetWindowPos( m_hWnd, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );
      //pSubMenu->ShowWindow( SW_SHOWNA );
      return;
    }
    else if ( MenuItem.dwFlags & MF_SEPARATOR )
    {
      //CCustomWnd::OnLButtonDown(nFlags, point);
      return;
    }

    // ein Menü-Item wurde ausgewählt
    CGRUIMenu*    pMenu = GetTopLevelMenu();

    pMenu->m_dwChosenMenuItem = MenuItem.dwID;
    // das oberste Menü zerstört alle anderen Childs mit sich
    pMenu->DestroyChildChain();
  }
  else
  {
    if ( GetCapture() == GetSafeHwnd() )
    {
      ReleaseCapture();
    }

    CGRUIMenu*    pMenu = GetTopLevelMenu();
    pMenu->DestroyWindow();

    //mouse_event( MOUSEEVENTF_ABSOLUTE, point.x * 65535 / iWidth, point.y * 65535 / iHeight, 0, 0 );
    // Mousedown an Fenster darunter weitergeben
    mouse_event( MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0 );
    return;
  }

  //CCustomWnd::OnRButtonDown(nFlags, point);
}

BOOL CGRUIMenu::_RegisterClass()
{
  WNDCLASS wc;
  wc.style = 0;//CS_OWNDC;
  wc.lpfnWndProc = MYHELPERWINDOWPROC; // must be this value
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );   // we use OnSetCursor
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu
  wc.lpszClassName = "GR_CUSTOMWND_MENU";

  BOOL    bDropShadowEnabled;
  SystemParametersInfo( 0x1024, 0, &bDropShadowEnabled, 0 );    // SPI_GETDROPSHADOW

  if ( bDropShadowEnabled )
  {
    wc.style |= 0x00020000; // CS_DROPSHADOW;
  }
  return RegisterClass( &wc );
}




