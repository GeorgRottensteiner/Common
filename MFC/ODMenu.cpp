// e:\projekte\common\mfc\ODMenu.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "ODMenu.h"

#include <string>

#include <MFC/MemDC.h>

#include <WinSys/WinUtils.h>

#include <debug/debugclient.h>
#include ".\odmenu.h"



struct ODMenu::tODMenuItem
{
  GR::String      ItemText;
  DWORD           ID;
  DWORD           ItemData;
  DWORD           Flags;
  HICON           hiconUnchecked,
                  hiconChecked;
  RECT            Size;

  ODMenu::tODMenuItem() :
    ID( 0 ),
    ItemData( 0 ),
    Flags( 0 ),
    ItemText( "" ),
    hiconUnchecked( NULL ),
    hiconChecked( NULL )
  {
    SetRectEmpty( &Size );
  }

  tODMenuItem::~tODMenuItem()
  {
  }
};



CBitmap ODMenu::s_bmDesktop;



IMPLEMENT_DYNAMIC(ODMenu, CWnd)
ODMenu::ODMenu( eODMenuStyle odStyle ) :
  m_Width( 20 ),
  m_ItemHeight( 12 ),
  m_Indent( 20 ),
  m_ShadowWidth( 4 ),
  m_CaptureMouse( false ),
  m_pOpenedSubMenu( false ),
  m_pParentMenu( NULL ),
  m_hdcDesktop( NULL ),
  m_hdcRightSide( NULL ),
  m_hdcBottomSide( NULL ),
  m_hbmRightSide( NULL ),
  m_hbmBottomSide( NULL ),
  m_pWndMenuOwner( NULL ),
  m_hIconCheck( NULL ),
  m_hIconPopupArrow( NULL ),
  m_BorderWidth( GetSystemMetrics( SM_CXDLGFRAME ) ),
  m_BorderHeight( GetSystemMetrics( SM_CYDLGFRAME ) ),
  m_odStyle( odStyle ),
  m_DefaultItem( -1 )
{
  BOOL    bDropShadowEnabled;
  SystemParametersInfo( 0x1024, 0, &bDropShadowEnabled, 0 );    // SPI_GETDROPSHADOW

  m_DropShadowEnabled = bDropShadowEnabled ? true : false;

  // kein Windows-Schatten, also selber machen
  m_DrawShadow = !m_DropShadowEnabled;

  switch ( m_odStyle )
  {
    case ODMS_XP:
      m_BorderWidth  = 1;
      m_BorderHeight = 1;
      m_Indent       = 24;
      break;
    case ODMS_BLACKMARBLE:
      m_BorderWidth  = 1;
      m_BorderHeight = 1;
      m_Indent       = 24;
      break;
  }

  HBITMAP   hbmCheck = LoadBitmap( NULL, MAKEINTRESOURCE( 32760 ) );  // OEM_CHECKBOX
  if ( hbmCheck )
  {
    m_hIconCheck = Win::Util::BitmapToIconEx( 0, hbmCheck, RGB( 255, 255, 255 ) );
    DeleteObject( hbmCheck );
  }

  HBITMAP   hbmArrow = LoadBitmap( NULL, MAKEINTRESOURCE( 32739 ) );  // OBM_MNARROW
  if ( hbmArrow )
  {
    m_hIconPopupArrow = Win::Util::BitmapToIconEx( 0, hbmArrow, RGB( 255, 255, 255 ) );
    DeleteObject( hbmArrow );
  }
}



ODMenu::~ODMenu()
{
  std::vector<tODMenuItem>::iterator   it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( MenuItem.Flags & MF_POPUP )
    {
      delete (ODMenu*)MenuItem.ID;
      MenuItem.ID = 0;
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
  m_MenuItems.clear();

  DestroyIcon( m_hIconCheck );
  DestroyIcon( m_hIconPopupArrow );
}


BOOL ODMenu::hasclass = ODMenu::RegisterMe();


BOOL ODMenu::RegisterMe()
{
  WNDCLASS wc;
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon
  wc.hCursor = NULL;   // we use OnSetCursor
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu
  wc.lpszClassName = ODMENU_CLASS_NAME;

  BOOL    bDropShadowEnabled;
  SystemParametersInfo( 0x1024, 0, &bDropShadowEnabled, 0 );    // SPI_GETDROPSHADOW

  if ( bDropShadowEnabled )
  {
    wc.style |= 0x00020000; // CS_DROPSHADOW;
  }

  return AfxRegisterClass(&wc);
}




BEGIN_MESSAGE_MAP(ODMenu, CWnd)
  ON_WM_PAINT()
  ON_WM_KILLFOCUS()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDOWN()
  ON_WM_ERASEBKGND()
  ON_WM_CREATE()
  ON_WM_SYSCOMMAND()
  ON_WM_CHAR()
  ON_WM_SYSKEYDOWN()
  ON_WM_NCPAINT()
ON_WM_ACTIVATEAPP()
ON_WM_NCLBUTTONDOWN()
//ON_WM_SETFOCUS()
ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// ODMenu-Meldungshandler



void ODMenu::DisplayMenuIcon( RECT& rc, HDC hdc, HICON hIcon )
{
  ICONINFO    ii;

  GetIconInfo( hIcon, &ii );

  BITMAP    bm;

  GetObject( ii.hbmColor, sizeof( BITMAP ), &bm );



  DeleteObject( ii.hbmColor );
  DeleteObject( ii.hbmMask );

  ::DrawIconEx( hdc,
          rc.left + ( rc.right - rc.left - bm.bmWidth ) / 2,
          rc.top + ( rc.bottom - rc.top - bm.bmHeight ) / 2,
                hIcon,
                bm.bmWidth,
                bm.bmHeight,
                0,
                0,
                DI_NORMAL );
}



void ODMenu::DisplayMenuBitmap( RECT& rc, HDC hdc, HBITMAP hbm )
{
  BITMAP      bm;

  GetObject( hbm, sizeof( BITMAP ), &bm );

  HDC         hdc2 = CreateCompatibleDC( hdc );
  SelectObject( hdc2, hbm );

  ::BitBlt( hdc,
            rc.left + ( rc.right - rc.left - bm.bmWidth ) / 2,
            rc.top + ( rc.bottom - rc.top - bm.bmHeight ) / 2,
            bm.bmWidth,
            bm.bmHeight,
            hdc2,
            0, 0,
            SRCCOPY );

  SelectObject( hdc2, NULL );
  DeleteDC( hdc2 );
}



void ODMenu::DrawMenuItem( CDC& dc, RECT& rc, tODMenuItem& MenuItem )
{
  bool    bSelected = false;

  POINT     ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );
  if ( ( PtInRect( &rc, ptMouse ) )
  &&   ( !( MenuItem.Flags & MF_DISABLED ) )
  &&   ( !( MenuItem.Flags & MF_SEPARATOR ) ) )
  {
    // selektierte Menü-items
    bSelected = true;
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        dc.FillSolidRect( &rc, GetSysColor( COLOR_HIGHLIGHT ) );
        dc.SetTextColor( GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        break;
      case ODMS_XP:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_Indent;
          dc.FillSolidRect( &rc2, GetSysColor( COLOR_BTNFACE ) );

          rc2 = rc;
          rc2.left = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 255, 255, 255 ) );

          HBRUSH    hBrush = CreateSolidBrush( RGB( 145, 176, 207 ) );
          HBRUSH    hOldBrush = (HBRUSH)dc.SelectObject( hBrush );

          rc2 = rc;
          InflateRect( &rc2, -1, -1 );

          HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 38, 87, 137 ) );
          HPEN  hOldPen = (HPEN)dc.SelectObject( hPen );
          dc.Rectangle( &rc2 );
          dc.SelectObject( hOldPen );
          DeleteObject( hPen );

          dc.SelectObject( hOldBrush );
          DeleteObject( hBrush );

          dc.SetTextColor( RGB( 255, 255, 255 ) );
        }
        break;
      case ODMS_BLACKMARBLE:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 32, 32, 32 ) );

          rc2 = rc;
          rc2.left = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 0, 0, 0 ) );

          dc.SetTextColor( 0 );

          HBRUSH    hBrush = CreateSolidBrush( RGB( 255, 32, 32 ) );
          HBRUSH    hOldBrush = (HBRUSH)dc.SelectObject( hBrush );

          rc2 = rc;
          InflateRect( &rc2, -1, -1 );

          HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 255, 200, 32 ) );
          HPEN  hOldPen = (HPEN)dc.SelectObject( hPen );
          dc.Rectangle( &rc2 );
          dc.SelectObject( hOldPen );
          DeleteObject( hPen );

          dc.SelectObject( hOldBrush );
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
        dc.FillSolidRect( &rc, GetSysColor( COLOR_BTNFACE ) );
        dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
        break;
      case ODMS_XP:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_Indent;
          dc.FillSolidRect( &rc2, GetSysColor( COLOR_BTNFACE ) );

          rc2 = rc;
          rc2.left = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 255, 255, 255 ) );

          dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
        }
        break;
      case ODMS_BLACKMARBLE:
        {
          RECT    rc2;

          rc2 = rc;
          rc2.right = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 64, 64, 64 ) );

          rc2 = rc;
          rc2.left = m_Indent;
          dc.FillSolidRect( &rc2, RGB( 0, 0, 0 ) );

          dc.SetTextColor( RGB( 128, 128, 128 ) );
        }
        break;
    }
  }

  if ( MenuItem.Flags & MF_SEPARATOR )
  {
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
      dc.Draw3dRect( 2, rc.top + 1, rc.right - rc.left - 4, 2, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
        break;
      case ODMS_XP:
        dc.FillSolidRect( m_Indent + 2, rc.top, rc.right - rc.left - m_Indent - 4, 1, GetSysColor( COLOR_3DSHADOW ) );
        break;
    }
  }
  else
  {
    rc.left = m_Indent + 3;
    if ( MenuItem.Flags & MF_DISABLED )
    {
      dc.SetTextColor( RGB( 128, 128, 128 ) );
    }

    int   iTimes = 1;
    if ( m_DefaultItem == MenuItem.ID )
    {
      iTimes = 2;
    }

    RECT    rcT = rc;
    for ( int iT = 0; iT < iTimes; ++iT )
    {
      rc = rcT;
      rc.left += iT;
      if ( MenuItem.ItemText.find( '\t' ) != GR::String::npos )
      {
        GR::WString   strLeft = GR::Convert::ToUTF16( MenuItem.ItemText.substr( 0, MenuItem.ItemText.find( '\t' ) ) );
        GR::WString   strRight = GR::Convert::ToUTF16( MenuItem.ItemText.substr( MenuItem.ItemText.find( '\t' ) + 1 ) );

        dc.DrawText( strLeft.c_str(), strLeft.length(), &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER );
        rc.right -= 20;
        dc.DrawText( strRight.c_str(), strRight.length(), &rc, DT_RIGHT | DT_SINGLELINE | DT_VCENTER );
      }
      else
      {
        dc.DrawText( GR::Convert::ToUTF16( MenuItem.ItemText ).c_str(), GR::Convert::ToUTF16( MenuItem.ItemText ).length(), &rc, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER );
      }
    }

    if ( MenuItem.Flags & MF_CHECKED )
    {
      rc.left = 2;
      rc.right = m_Indent;
      if ( MenuItem.hiconChecked )
      {
        DisplayMenuIcon( rc, dc.GetSafeHdc(), MenuItem.hiconChecked );
      }
      else
      {
        DisplayMenuIcon( rc, dc.GetSafeHdc(), m_hIconCheck );
      }
    }
    else if ( MenuItem.hiconChecked )
    {
      rc.left = 2;
      rc.right = m_Indent;

      if ( ( m_odStyle == ODMS_XP )
      &&   ( bSelected ) )
      {
        HICON   hIconGreyed = Win::Util::CreateGrayscaleIcon( MenuItem.hiconChecked, true );
        DisplayMenuIcon( rc, dc.GetSafeHdc(), hIconGreyed );
        DestroyIcon( hIconGreyed );
        OffsetRect( &rc, -2, -2 );
        DisplayMenuIcon( rc, dc.GetSafeHdc(), MenuItem.hiconChecked );
        OffsetRect( &rc, 2, 2 );
      }
      else if ( ( m_odStyle == ODMS_XP )
      &&        ( !bSelected ) )
      {
        HICON   hIconGreyed = Win::Util::CreateGrayscaleIcon( MenuItem.hiconChecked );
        DisplayMenuIcon( rc, dc.GetSafeHdc(), hIconGreyed );
        DestroyIcon( hIconGreyed );
      }
      else
      {
        DisplayMenuIcon( rc, dc.GetSafeHdc(), MenuItem.hiconChecked );
      }
    }
    else if ( MenuItem.hiconUnchecked )
    {
      rc.left = 2;
      rc.right = m_Indent;

      DisplayMenuIcon( rc, dc.GetSafeHdc(), MenuItem.hiconUnchecked );
    }

    // Popup-Menü - Pfeil nach rechts
    if ( MenuItem.Flags & MF_POPUP )
    {
      rc.right = m_Width;
      rc.left = rc.right - 24;

      DisplayMenuIcon( rc, dc.GetSafeHdc(), m_hIconPopupArrow );
    }
  }

}



void ODMenu::OnPaint()
{
  CPaintDC DCPaint( this );

  RECT      rc,
            rcClient;

  GetClientRect( &rc );
  rcClient = rc;

  if ( m_DrawShadow )
  {
    rcClient.right  -= m_ShadowWidth;
    rcClient.bottom -= m_ShadowWidth;
  }

  MFCExtension::CMemDC    dc( &DCPaint, rcClient );

  dc.SelectObject( GetStockObject( DEFAULT_GUI_FONT ) );

  m_ItemHeight = dc.DrawText( _T( "ÄÖÜyg" ), 5, &rc, DT_CALCRECT );
  m_ItemHeight += 4;

  switch ( m_odStyle )
  {
    case ODMS_XP:
      m_ItemHeight += 8;
      break;
  }

  rc = rcClient;

  dc.SetBkMode( TRANSPARENT );

  POINT     ptMouse;

  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

  int       iIndex = 0;

  std::vector<tODMenuItem>::iterator   it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    GetItemRect( iIndex, rc );

    DrawMenuItem( dc, rc, MenuItem );


    iIndex++;
    ++it;
  }

}

void ODMenu::OnKillFocus(CWnd* pNewWnd)
{

  CWnd::OnKillFocus(pNewWnd);

  NotActiveAnymore( pNewWnd );

}



BOOL ODMenu::AppendMenu( UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem, DWORD ItemData )
{

  m_MenuItems.push_back( tODMenuItem() );

  tODMenuItem&    Item = m_MenuItems.back();

  if ( lpszNewItem )
  {
    Item.ItemText  = GR::Convert::ToUTF8( lpszNewItem );
  }
  Item.ID         = nIDNewItem;
  Item.ItemData   = ItemData;
  Item.Flags      = nFlags;

  if ( nFlags & MF_POPUP )
  {
    ODMenu*    pSubMenu = (ODMenu*)nIDNewItem;

    pSubMenu->m_pParentMenu = this;
  }

  if ( GetSafeHwnd() )
  {
    RecalcSize();
  Invalidate();
  }

  return TRUE;

}



void ODMenu::PostNcDestroy()
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
  if ( m_DrawShadow )
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

  CWnd::PostNcDestroy();
}



void ODMenu::OnMouseMove(UINT nFlags, CPoint point)
{

  CWnd::OnMouseMove(nFlags, point);

  if ( !m_CaptureMouse )
  {
    m_CaptureMouse = true;
    if ( GetCapture() != this )
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
      ODMenu*    pOtherMenu = IsPartOfMenuChain( hwnd );

      if ( pOtherMenu )
    {
      if ( GetCapture() == this )
      {
      ReleaseCapture();
      }
      m_CaptureMouse = false;

        pOtherMenu->m_CaptureMouse = true;
        pOtherMenu->SetCapture();
        Invalidate();
      }
      /*
      else if ( GetCapture() == this )
      {
        ReleaseCapture();
        m_CaptureMouse = false;
        GetTopLevelMenu()->DestroyChildChain();
      }
      */
    }
    else
    {
      Invalidate();
    }
  }

}



ODMenu* ODMenu::IsPartOfMenuChain( HWND hWnd )
{

  if ( hWnd == GetSafeHwnd() )
  {
    return this;
  }
  ODMenu*    pMenu = this;
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



ODMenu* ODMenu::GetTopLevelMenu()
{

  ODMenu*    pMenu = this;
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



void ODMenu::OnLButtonDown(UINT nFlags, CPoint point)
{

  int   iItem = ItemFromPoint( point );

  if ( iItem != -1 )
  {
    tODMenuItem&    MenuItem = m_MenuItems[iItem];

    if ( MenuItem.Flags & MF_DISABLED )
    {
      CWnd::OnLButtonDown(nFlags, point);
      return;
    }
    if ( MenuItem.Flags & MF_POPUP )
    {
      if ( m_pOpenedSubMenu )
      {
        if ( m_pOpenedSubMenu == (ODMenu*)MenuItem.ID )
        {
          // das Submenü ist schon offen
          return;
        }

        // altes geöffnetes Submenü schließen
        m_pOpenedSubMenu->DestroyChildChain();
        m_pOpenedSubMenu = NULL;
        if ( m_DrawShadow )
        {
          GetDesktopWindow()->RedrawWindow();
        }
      }

      RECT    rcItem;

      GetItemRect( iItem, rcItem );
      ClientToScreen( &rcItem );

      ODMenu*    pSubMenu = (ODMenu*)MenuItem.ID;

      OffsetRect( &rcItem, rcItem.right - rcItem.left, 0 );

      m_pOpenedSubMenu = pSubMenu;
      if ( pSubMenu->GetSafeHwnd() == NULL )
      {
        pSubMenu->CreateEx( 0, ODMENU_CLASS_NAME, _T( "" ), WS_POPUP | WS_DLGFRAME, rcItem, this, 0 );
        pSubMenu->RecalcSize();
        pSubMenu->SetOwner( this );
        pSubMenu->SaveShadowBackground();
      }
      pSubMenu->SetWindowPos( this, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );
      //pSubMenu->ShowWindow( SW_SHOWNA );
      return;
    }
    else if ( MenuItem.Flags & MF_SEPARATOR )
    {
      CWnd::OnLButtonDown(nFlags, point);
      return;
    }

    // ein Menü-Item wurde ausgewählt
    ODMenu*    pMenu = GetTopLevelMenu();

    pMenu->m_ChosenMenuItem = MenuItem.ID;
    // das oberste Menü zerstört alle anderen Childs mit sich
    pMenu->DestroyChildChain();
  }
  else
  {
    if ( GetCapture() == this )
    {
      ReleaseCapture();
    }
    ODMenu*    pMenu = GetTopLevelMenu();
    //pMenu->DestroyChildChain();
    pMenu->DestroyWindow();

    //mouse_event( MOUSEEVENTF_ABSOLUTE, point.x * 65535 / iWidth, point.y * 65535 / iHeight, 0, 0 );
    // Mousedown an Fenster darunter weitergeben
    mouse_event( MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0 );
    return;
  }

  CWnd::OnLButtonDown(nFlags, point);
}



void ODMenu::DestroyChildChain()
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

    if ( m_DrawShadow )
    {
      CWnd*   pWndDesktop = CWnd::GetDesktopWindow();

      pWndDesktop->InvalidateRect( &rc );
      pWndDesktop->RedrawWindow( &rc );
      pWndDesktop->UpdateWindow();

      for ( int i = 0; i < 20; ++i )
      {
        MSG   msg;
        if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
          if ( msg.message == WM_QUIT )
          {
            break;
          }
          AfxPumpMessage();
        }
      }
    }
  }

}



bool ODMenu::GetItemRect( int iIndex, RECT& rc )
{

  memset( &rc, 0, sizeof( RECT ) );

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_MenuItems.size() ) )
  {
    return false;
  }

  rc = m_MenuItems[iIndex].Size;
      return true;

}



int ODMenu::ItemFromPoint( const POINT& pt )
{

  RECT    rc;

  GetClientRect( &rc );

  if ( !PtInRect( &rc, pt ) )
  {
    return -1;
  }

  int iTopHeight = 0;

  int   iIndex = 0;
  std::vector<tODMenuItem>::iterator    it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( PtInRect( &MenuItem.Size, pt ) )
  {
        return iIndex;
      }

    ++iIndex;
    ++it;
  }

    return -1;

}



BOOL ODMenu::OnEraseBkgnd(CDC* pDC)
{

  return TRUE;

}



void ODMenu::RecalcSize()
{

  if ( !m_hWnd )
  {
    return;
  }

  switch ( m_odStyle )
  {
    case ODMS_CLASSIC:
      m_Indent       = 20;
      break;
    case ODMS_XP:
      m_BorderWidth  = 1;
      m_BorderHeight = 1;

      m_Indent       = 24;
      break;
  }


  CDC* pDC = GetDC();

  pDC->SelectObject( GetStockObject( DEFAULT_GUI_FONT ) );

  RECT    rc;

  GetClientRect( &rc );

  m_ItemHeight = pDC->DrawText( _T( "ÄÖÜyg" ), 5, &rc, DT_CALCRECT );

  m_ItemHeight += 4;

  switch ( m_odStyle )
  {
    case ODMS_XP:
      m_ItemHeight += 8;
      break;
  }

  int     iMaxWidth = 0,
          iHeight = 0;

  rc.bottom = 0;

  std::vector<tODMenuItem>::iterator    it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( MenuItem.Flags & MF_SEPARATOR )
    {
      switch ( m_odStyle )
      {
        case ODMS_CLASSIC:
      iHeight += 4;
          rc.bottom = rc.top + 4;
          break;
        case ODMS_XP:
          iHeight += 1;
          rc.bottom = rc.top + 1;
          break;
      }
    }
    else
    {
      int   iItemHeight = m_ItemHeight;

      if ( MenuItem.hiconChecked )
      {
        ICONINFO    ii;

        GetIconInfo( MenuItem.hiconChecked, &ii );

        BITMAP      bm;

        GetObject( ii.hbmColor, sizeof( BITMAP ), &bm );

        bm.bmHeight += 6;
        if ( m_Indent < bm.bmWidth + 4 )
        {
          m_Indent = bm.bmWidth + 4;
        }

        if ( bm.bmHeight > iItemHeight )
        {
          iItemHeight = bm.bmHeight;
        }

        DeleteObject( ii.hbmColor );
        DeleteObject( ii.hbmMask );
      }

      if ( MenuItem.ItemText.find( '\t' ) != GR::String::npos )
      {
        // hier be tabs
        pDC->DrawText( GR::Convert::ToUTF16( MenuItem.ItemText ).c_str(), MenuItem.ItemText.length(), &rc, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );

        rc.right += 20;
      }
      else
      {
        pDC->DrawText( GR::Convert::ToUTF16( MenuItem.ItemText ).c_str(), MenuItem.ItemText.length(), &rc, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );
      }
      if ( iItemHeight > rc.bottom - rc.top )
      {
        rc.bottom = rc.top + iItemHeight;
      }
      if ( m_Indent + 4 + rc.right - rc.left > iMaxWidth )
      {
        iMaxWidth = m_Indent + 4 + rc.right - rc.left;
      }
      iHeight += rc.bottom - rc.top;
    }

    MenuItem.Size = rc;

    rc.top = rc.bottom;
    ++it;
  }

  iMaxWidth += 20;    // für den Popup-Pfeil muß Platz sein

  // max. Breiten anpassen
  std::vector<tODMenuItem>::iterator    itMI( m_MenuItems.begin() );
  while ( itMI != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *itMI;

    MenuItem.Size.right = MenuItem.Size.left + iMaxWidth;

    ++itMI;
  }

  if ( m_DrawShadow )
  {
    iMaxWidth += m_ShadowWidth;
    iHeight += m_ShadowWidth;
  }

  m_Width = iMaxWidth;

  ReleaseDC( pDC );

  SetWindowPos( NULL, 0, 0,
                iMaxWidth + 2 * m_BorderWidth,
                iHeight + 2 * m_BorderHeight,
                SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE );

}



COLORREF ODMenu::ShadowColor( int iX, int iY, int iShadowValue )
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



ODMenu::tODMenuItem* ODMenu::FindMenuItem( UINT uPosition, UINT uFlags )
{

  if ( uFlags == MF_BYPOSITION )
  {
    if ( uPosition >= m_MenuItems.size() )
    {
      return NULL;
    }
    return &m_MenuItems[uPosition];
  }

  std::vector<tODMenuItem>::iterator    it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( MenuItem.Flags & MF_POPUP )
    {
      ODMenu*    pODMenu = (ODMenu*)MenuItem.ID;

      tODMenuItem*    pItem = pODMenu->FindMenuItem( uPosition, uFlags );
      if ( pItem )
      {
        return pItem;
      }
    }
    if ( MenuItem.ID == uPosition )
    {
      return &MenuItem;
    }

    ++it;
  }
  return NULL;
}



BOOL ODMenu::SetMenuItemIcons( UINT uPosition, UINT uFlags, HICON hIconChecked, HICON hIconUnchecked )
{
  tODMenuItem*  pItem = FindMenuItem( uPosition, uFlags );

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



BOOL ODMenu::TrackPopupMenu( UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect )
{

  m_ChosenMenuItem = 0;
  if ( GetSafeHwnd() != NULL )
  {
    return 0;
  }

  if ( ( pWnd )
  &&   ( !( nFlags & TPM_NONOTIFY ) ) )
  {
    pWnd->SendMessage( WM_ENTERMENULOOP, TRUE );
  }

  m_pWndMenuOwner = pWnd;

  RECT    rc;

  rc.left = x;
  rc.top = y;
  rc.right = x;
  rc.bottom = y;

  CreateEx( 0, ODMENU_CLASS_NAME, _T( "" ), WS_POPUP | WS_DLGFRAME, rc, pWnd, 0 );


  RecalcSize();

  RECT  rcWnd;
  GetWindowRect( &rcWnd );
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

  SetWindowPos( &CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );

  SetCapture();
  if ( GetCapture() != this )
  {
    dh::Log( "failed to capture\n" );
  }
  else
  {
    //dh::Log( "ich capture\n" );
  }
  while ( GetSafeHwnd() )
  {
    MSG msg;
    if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
    {
      /*
      if ( ( msg.message == WM_LBUTTONDOWN )
      ||   ( msg.message == WM_NCLBUTTONDOWN ) )
      {
        if ( msg.hwnd != GetSafeHwnd() )
        {
          if ( !( nFlags & TPM_NONOTIFY ) )
          {
            DestroyWindow();
            break;
          }
        }
      }
      */
      if ( msg.message == WM_QUIT )
      {
        break;
      }
      if ( ( ( msg.hwnd )
      &&     ( ::IsWindow( msg.hwnd ) ) )
      ||   ( msg.hwnd == NULL ) )
      {
        GetMessage( &msg, 0, 0, 0 );

        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    else
    {
        PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
    }
  }
    //AfxPumpMessage();
  }

  if ( ( pWnd )
  &&   ( pWnd->GetSafeHwnd() )
  &&   ( ::IsWindow( pWnd->GetSafeHwnd() ) )
  &&   ( !( nFlags & TPM_NONOTIFY ) ) )
  {
    pWnd->SendMessage( WM_EXITMENULOOP, TRUE );
  }

  s_bmDesktop.DeleteObject();

  if ( !( nFlags & TPM_NONOTIFY ) )
  {
    if ( ( pWnd )
    &&   ( pWnd->GetSafeHwnd() )
    &&   ( ::IsWindow( pWnd->GetSafeHwnd() ) ) )
    {
      pWnd->PostMessage( WM_COMMAND, MAKEWPARAM( m_ChosenMenuItem, 0 ), (LPARAM)GetSafeHwnd() );
    }
  }

  if ( nFlags & TPM_RETURNCMD )
  {
    return m_ChosenMenuItem;
  }

  return TRUE;
}



void ODMenu::DeleteAllMenuItems()
{
  std::vector<tODMenuItem>::iterator   it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( MenuItem.Flags & MF_POPUP )
    {
      delete (ODMenu*)MenuItem.ID;
      MenuItem.ID = 0;
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

  m_MenuItems.clear();
  RecalcSize();
}



int ODMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  switch ( m_odStyle )
  {
    case ODMS_XP:
    case ODMS_BLACKMARBLE:
      ModifyStyle( WS_DLGFRAME, WS_BORDER );
      break;
  }

  if ( m_DrawShadow )
  {
    m_hdcDesktop = ::CreateDC( _T( "DISPLAY" ), NULL, NULL, NULL );

    m_hdcRightSide  = ::CreateCompatibleDC( m_hdcDesktop );
    m_hdcBottomSide = ::CreateCompatibleDC( m_hdcDesktop );

    m_hbmRightSide  = CreateCompatibleBitmap( m_hdcDesktop, m_ShadowWidth, GetSystemMetrics( SM_CYSCREEN ) );
    m_hbmBottomSide = CreateCompatibleBitmap( m_hdcDesktop, GetSystemMetrics( SM_CXSCREEN ), m_ShadowWidth );
    m_hbmSavedRightSide  = CreateCompatibleBitmap( m_hdcDesktop, m_ShadowWidth, GetSystemMetrics( SM_CYSCREEN ) );
    m_hbmSavedBottomSide = CreateCompatibleBitmap( m_hdcDesktop, GetSystemMetrics( SM_CXSCREEN ), m_ShadowWidth );
  }

  return 0;
}



void ODMenu::SaveShadowBackground()
{

  if ( m_DrawShadow )
  {
    SaveDesktop( m_hWnd );

    GetWindowRect( &m_rcShadow );

    SelectObject( m_hdcRightSide, m_hbmSavedRightSide );
    SelectObject( m_hdcBottomSide, m_hbmSavedBottomSide );

    HDC     hdcDesktop = CreateCompatibleDC( m_hdcRightSide );

    ::BitBlt( m_hdcRightSide, 0, 0, m_ShadowWidth, m_rcShadow.bottom - m_rcShadow.top,
              hdcDesktop, m_rcShadow.right - m_ShadowWidth, m_rcShadow.top, SRCCOPY );
    ::BitBlt( m_hdcBottomSide, 0, 0, m_rcShadow.right - m_rcShadow.left, m_ShadowWidth,
              hdcDesktop, m_rcShadow.left, m_rcShadow.bottom - m_ShadowWidth, SRCCOPY );

    DeleteDC( hdcDesktop );

    /*
    BitBlt( m_hdcRightSide, 0, 0, m_ShadowWidth, GetSystemMetrics( SM_CYSCREEN ), m_hdcDesktop, m_rcShadow.right - m_ShadowWidth, m_rcShadow.top, SRCCOPY );
    BitBlt( m_hdcBottomSide, 0, 0, GetSystemMetrics( SM_CXSCREEN ), m_ShadowWidth, m_hdcDesktop, m_rcShadow.left, m_rcShadow.bottom - m_ShadowWidth, SRCCOPY );
    */

    SelectObject( m_hdcRightSide, m_hbmRightSide );
    SelectObject( m_hdcBottomSide, m_hbmBottomSide );

    // Schatten erzeugen
    for ( int i = 0; i < m_ShadowWidth; i++ )
    {
      for ( int j = 0; j < m_ShadowWidth; j++ )
      {
        int   iValue = m_ShadowWidth - i;
        if ( iValue > j )
        {
          iValue = j;
        }
        // Ecke rechts oben
        SetPixel( m_hdcRightSide, i, j,
                  ShadowColor( m_rcShadow.right - m_ShadowWidth + i, m_rcShadow.top + j, iValue ) );
        // Ecke links unten
        SetPixel( m_hdcBottomSide, m_ShadowWidth - 1 - i, m_ShadowWidth - 1 - j,
                  ShadowColor( m_rcShadow.left + m_ShadowWidth - 1- i, m_rcShadow.bottom + m_ShadowWidth - 1- j, iValue ) );
      }
    }
    // Ecke rechts unten
    for ( int i = 0; i < m_ShadowWidth; i++ )
    {
      for ( int j = 0; j < m_ShadowWidth; j++ )
      {
        int   iValue = i;
        if ( j > iValue )
        {
          iValue = j;
        }
        SetPixel( m_hdcRightSide, i, m_rcShadow.bottom - m_rcShadow.top - m_ShadowWidth + j,
                  ShadowColor( m_rcShadow.right - m_ShadowWidth + i, m_rcShadow.bottom - m_ShadowWidth + j, m_ShadowWidth - iValue ) );
      }
    }
    for ( int i = 0; i < m_ShadowWidth; i++ )
    {
      for ( int j = 0; j < m_rcShadow.bottom - m_rcShadow.top - 2 * m_ShadowWidth; j++ )
      {
        SetPixel( m_hdcRightSide, i, j + m_ShadowWidth,
                  ShadowColor( m_rcShadow.right - m_ShadowWidth + i, m_rcShadow.top + j + m_ShadowWidth, m_ShadowWidth - i ) );
      }
    }
    for ( int i = m_ShadowWidth; i < m_rcShadow.right - m_rcShadow.left; i++ )
    {
      for ( int j = 0; j < m_ShadowWidth; j++ )
      {
        SetPixel( m_hdcBottomSide, i, j,
                  ShadowColor( m_rcShadow.left + i, m_rcShadow.bottom - m_ShadowWidth + j, m_ShadowWidth - j ) );
      }
    }
  }

}



void ODMenu::CompleteClose()
{

  ODMenu*    pMenu = GetTopLevelMenu();
  pMenu->DestroyChildChain();

}



void ODMenu::OnSysCommand(UINT nID, LPARAM lParam)
{

  CompleteClose();

  //CWnd::OnSysCommand( nID, lParam );

}

void ODMenu::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if ( nChar == VK_ESCAPE )
  {
    CompleteClose();
  }

  CWnd::OnChar(nChar, nRepCnt, nFlags);
}



void ODMenu::NotActiveAnymore( CWnd* pNewWnd )
{

  if ( ( GetCapture() )
  &&   ( GetCapture()->GetSafeHwnd() == GetSafeHwnd() ) )
  {
    ReleaseCapture();
    m_CaptureMouse = false;
  }

  // ist das neue Wnd ein Child von uns?
  bool    bIsDescendant = false;
  while ( pNewWnd )
  {
    if ( pNewWnd->GetSafeHwnd() == GetSafeHwnd() )
    {
      bIsDescendant = true;
      break;
    }
    pNewWnd = pNewWnd->GetOwner();
  }

  // ist das neue Wnd ein Parent von uns?
  bool    bIsParent = false;
  bool    bIsSibling = false;
  ODMenu*    pMenu = m_pParentMenu;
  while ( pMenu )
  {
    if ( pMenu->GetSafeHwnd() == pNewWnd->GetSafeHwnd() )
    {
      bIsParent = true;
    }
    if ( ( pMenu->m_pOpenedSubMenu )
    &&   ( pMenu->m_pOpenedSubMenu->GetSafeHwnd() == pNewWnd->GetSafeHwnd() ) )
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



void ODMenu::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

  CompleteClose();

}



void ODMenu::OnNcPaint()
{

  if ( !m_DrawShadow )
  {
    Default();
    return;
  }

  RECT    rc;

  GetWindowRect( &rc );

  rc.right  -= m_ShadowWidth;
  rc.bottom -= m_ShadowWidth;

  OffsetRect( &rc, -rc.left, -rc.top );

  CDC*    pDC = GetDCEx( NULL, DCX_WINDOW );

  if ( pDC )
  {
    switch ( m_odStyle )
    {
      case ODMS_CLASSIC:
        pDC->FillSolidRect( rc.left, rc.top, m_BorderWidth, rc.bottom - rc.top, GetSysColor( COLOR_BTNFACE ) );
        pDC->FillSolidRect( rc.left, rc.top, rc.right - rc.left, m_BorderHeight, GetSysColor( COLOR_BTNFACE ) );
        pDC->FillSolidRect( rc.right - m_BorderWidth, rc.top, m_BorderWidth, rc.bottom - rc.top, GetSysColor( COLOR_BTNFACE ) );
        pDC->FillSolidRect( rc.left, rc.bottom - m_BorderHeight, rc.right - rc.left, m_BorderHeight, GetSysColor( COLOR_BTNFACE ) );

    pDC->DrawEdge( &rc, EDGE_RAISED, BF_RECT );
        break;
      case ODMS_XP:
        pDC->FillSolidRect( rc.left, rc.top, m_BorderWidth, rc.bottom - rc.top, 0 );
        pDC->FillSolidRect( rc.left, rc.top, rc.right - rc.left, m_BorderHeight, 0 );
        pDC->FillSolidRect( rc.right - m_BorderWidth, rc.top, m_BorderWidth, rc.bottom - rc.top, 0 );
        pDC->FillSolidRect( rc.left, rc.bottom - m_BorderHeight, rc.right - rc.left, m_BorderHeight, 0 );
        break;
      case ODMS_BLACKMARBLE:
        pDC->FillSolidRect( rc.left, rc.top, m_BorderWidth, rc.bottom - rc.top, 0 );
        pDC->FillSolidRect( rc.left, rc.top, rc.right - rc.left, m_BorderHeight, 0 );
        pDC->FillSolidRect( rc.right - m_BorderWidth, rc.top, m_BorderWidth, rc.bottom - rc.top, 0 );
        pDC->FillSolidRect( rc.left, rc.bottom - m_BorderHeight, rc.right - rc.left, m_BorderHeight, 0 );
        break;
    }

    if ( m_DrawShadow )
    {
      ::BitBlt( pDC->GetSafeHdc(), 0, rc.bottom, rc.right - rc.left + m_ShadowWidth, m_ShadowWidth,
                m_hdcBottomSide, rc.left, 0, SRCCOPY );
      ::BitBlt( pDC->GetSafeHdc(), rc.right, 0, m_ShadowWidth, rc.bottom - rc.top + m_ShadowWidth,
                m_hdcRightSide, 0, rc.top, SRCCOPY );
    }
    ReleaseDC( pDC );
  }


}



BOOL ODMenu::DestroyWindow()
{
  if ( ( GetCapture() )
  &&   ( GetCapture()->GetSafeHwnd() == GetSafeHwnd() ) )
  {
    ReleaseCapture();
    m_CaptureMouse = false;
  }

  return CWnd::DestroyWindow();
}



void ODMenu::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
  CWnd::OnActivateApp(bActive, dwThreadID);

  if ( !bActive )
  {
    NotActiveAnymore( NULL );
  }
}



void ODMenu::SaveDesktop( HWND hWnd )
{
  if ( s_bmDesktop.GetSafeHandle() == NULL )
  {
    // Get the desktop hDC...
    HDC hDcDsk = ::GetWindowDC(0) ;
    CDC* pDcDsk = CDC::FromHandle(hDcDsk);

    CDC dc;
    dc.CreateCompatibleDC(pDcDsk);

    CRect rect;
    ::GetWindowRect( hWnd,rect);

    s_bmDesktop.CreateCompatibleBitmap( pDcDsk, rect.Width(), rect.Height() );
    CBitmap* pbmOld = dc.SelectObject( &s_bmDesktop );
    dc.BitBlt(0,0,rect.Width() + 10,rect.Height() + 10,pDcDsk, rect.left, rect.top,SRCCOPY);

    dc.SelectObject( pbmOld );
    // Release the desktop hDC...
    ::ReleaseDC(0,hDcDsk);
  }
}



BOOL ODMenu::EnableMenuItem( UINT_PTR nID, BOOL bOn )
{
  // auch rekursiv durchsuchen!
  std::vector<tODMenuItem>::iterator   it( m_MenuItems.begin() );
  while ( it != m_MenuItems.end() )
  {
    tODMenuItem&    MenuItem = *it;

    if ( MenuItem.Flags & MF_POPUP )
    {
      ODMenu*  pODSubMenu = (ODMenu*)MenuItem.ID;

      if ( pODSubMenu->EnableMenuItem( nID, bOn ) )
      {
        return TRUE;
      }
    }
    if ( MenuItem.ID == nID )
    {
      if ( bOn )
      {
        MenuItem.Flags &= ~MF_DISABLED;
      }
      else
      {
        MenuItem.Flags |= MF_DISABLED;
      }
      return TRUE;
    }
    ++it;
  }

  return FALSE;
}



int ODMenu::GetMenuItemCount() const
{
  return (int)m_MenuItems.size();
}



BOOL ODMenu::CheckMenuItem( UINT_PTR nID, UINT uCheck )
{
  tODMenuItem*  pItem = FindMenuItem( nID, uCheck );

  if ( pItem == NULL )
  {
    return FALSE;
  }
  if ( uCheck & MF_CHECKED )
  {
    pItem->Flags |= MF_CHECKED;
  }
  else
  {
    pItem->Flags &= ~MF_CHECKED;
  }
  return TRUE;
}



LRESULT ODMenu::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  if ( message == WM_CAPTURECHANGED )
  {
    //dh::Log( "lost capture\n" );
  }
  // TODO: Fügen Sie hier Ihren spezialisierten Code ein, und/oder rufen Sie die Basisklasse auf.

  return CWnd::WindowProc(message, wParam, lParam);
}



void ODMenu::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
  ScreenToClient( &point );

  //OnLButtonDown( 0, point );
  // TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

  CWnd::OnNcLButtonDown(nHitTest, point);
}



ODMenu* ODMenu::ReplaceMenu( HMENU hMenu )
{
  if ( hMenu == NULL )
  {
    return NULL;
  }

  ODMenu*    pODMenu = new ODMenu( m_odStyle );

  for ( int i = 0; i < ::GetMenuItemCount( hMenu ); ++i )
  {
    GR::WChar    szTemp[MAX_PATH];

    GetMenuStringW( hMenu, i, szTemp, MAX_PATH, MF_BYPOSITION );

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
  pODMenu->m_DefaultItem = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED );
  return pODMenu;
}



void ODMenu::Clone( HMENU hMenu, HWND hwndSysOwner )
{
  // bildet ein HMenu nach
  DeleteAllMenuItems();

  int iItems = ::GetMenuItemCount( hMenu );

  for ( int i = 0; i < iItems; ++i )
  {
    GR::WChar    szTemp[MAX_PATH];

    GetMenuStringW( hMenu, i, szTemp, MAX_PATH, MF_BYPOSITION );

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

  m_DefaultItem = GetMenuDefaultItem( hMenu, FALSE, GMDI_USEDISABLED );

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



void ODMenu::OnRButtonDown(UINT nFlags, CPoint point)
{
  int   iItem = ItemFromPoint( point );

  if ( iItem != -1 )
  {
    tODMenuItem&    MenuItem = m_MenuItems[iItem];

    if ( MenuItem.Flags & MF_DISABLED )
    {
      CWnd::OnLButtonDown(nFlags, point);
      return;
    }
    if ( MenuItem.Flags & MF_POPUP )
    {
      if ( m_pOpenedSubMenu )
      {
        if ( m_pOpenedSubMenu == (ODMenu*)MenuItem.ID )
        {
          // das Submenü ist schon offen
          return;
        }

        // altes geöffnetes Submenü schließen
        m_pOpenedSubMenu->DestroyChildChain();
        m_pOpenedSubMenu = NULL;
        if ( m_DrawShadow )
        {
          GetDesktopWindow()->RedrawWindow();
        }
      }

      RECT    rcItem;

      GetItemRect( iItem, rcItem );
      ClientToScreen( &rcItem );

      ODMenu*    pSubMenu = (ODMenu*)MenuItem.ID;

      OffsetRect( &rcItem, rcItem.right - rcItem.left, 0 );

      m_pOpenedSubMenu = pSubMenu;
      if ( pSubMenu->GetSafeHwnd() == NULL )
      {
        pSubMenu->CreateEx( 0, ODMENU_CLASS_NAME, _T( "" ), WS_POPUP | WS_DLGFRAME, rcItem, this, 0 );
        pSubMenu->RecalcSize();
        pSubMenu->SetOwner( this );
        pSubMenu->SaveShadowBackground();
      }
      pSubMenu->SetWindowPos( this, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW );
      //pSubMenu->ShowWindow( SW_SHOWNA );
      return;
    }
    else if ( MenuItem.Flags & MF_SEPARATOR )
    {
      CWnd::OnLButtonDown(nFlags, point);
      return;
    }

    // ein Menü-Item wurde ausgewählt
    ODMenu*    pMenu = GetTopLevelMenu();

    pMenu->m_ChosenMenuItem = MenuItem.ID;
    // das oberste Menü zerstört alle anderen Childs mit sich
    pMenu->DestroyChildChain();
  }
  else
  {
    if ( GetCapture() == this )
    {
      ReleaseCapture();
    }
    ODMenu*    pMenu = GetTopLevelMenu();
    //pMenu->DestroyChildChain();
    pMenu->DestroyWindow();

    //mouse_event( MOUSEEVENTF_ABSOLUTE, point.x * 65535 / iWidth, point.y * 65535 / iHeight, 0, 0 );
    // Mousedown an Fenster darunter weitergeben
    mouse_event( MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0 );
    return;
  }

  CWnd::OnRButtonDown(nFlags, point);
}
