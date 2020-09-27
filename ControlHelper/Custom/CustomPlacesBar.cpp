// CustomPlacesBar.cpp: Implementierungsdatei
//

#include "CustomPlacesBar.h"

#include <WinSys/WinUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



BOOL CCustomPlacesBar::hasclass = CCustomPlacesBar::RegisterMe();



/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CCustomPlacesBar::RegisterMe()
{
  WNDCLASS wc;   
  wc.style = 0;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = CUSTOMPLACESBAR_CLASS_NAME;

  return RegisterClass( &wc );
}




/////////////////////////////////////////////////////////////////////////////
// CCustomPlacesBar

CCustomPlacesBar::CCustomPlacesBar()
: m_iItemWidth( -1 ),
  m_iItemHeight( 14 ),
  m_iOffset( 0 ),
  m_iSelectedItem( -1 ),
  m_iMouseOverItem( -1 ),
  m_iFocusedItem( -1 ),
  m_bHasFocus( FALSE ),
  m_iTimerID( -1 ),
  m_dwCPStyles( PBS_DEFAULT ),
  m_ToolTip()
{
}

CCustomPlacesBar::~CCustomPlacesBar()
{
}


/*
BEGIN_MESSAGE_MAP(CCustomPlacesBar, CWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_GETDLGCODE()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
  ON_WM_ENABLE()
END_MESSAGE_MAP()
*/


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCustomPlacesBar 

/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CCustomPlacesBar::Create( RECT &rc, HWND hwndParent, int iID )
{

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, NULL, NULL,
                        WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, hwndParent, iID ) )
  {
    return FALSE;
  }

  Initialize();

  return TRUE;

}


/*-Initialize-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::Initialize( DWORD dwCPStyles, int iItemHeight )
{

  m_ToolTip.Create( GetSafeHwnd() );
  

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

  RECT    rc;

  GetClientRect( &rc );

  m_ToolTip.AddTool( GetSafeHwnd(), "Hullehutz" );
  m_ToolTip.Activate( FALSE );

  m_dwCPStyles = dwCPStyles;
  if ( m_dwCPStyles & PBS_SMALL_ITEMS )
  {
    if ( iItemHeight < 32 )
    {
      iItemHeight = 32;
    }

    SetItemSize( rc.right - rc.left, iItemHeight );
  }
  else
  {
    if ( iItemHeight < 70 )
    {
      iItemHeight = 70;
    }
    SetItemSize( rc.right - rc.left, iItemHeight );
  }

}



/*-OnDestroy------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::OnDestroy() 
{

  m_vectItems.clear();
	
}



/*-SetItemSize----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::SetItemSize( int iWidth, int iHeight )
{

  if ( iWidth != 0 )
  {
    m_iItemWidth = iWidth;
  }
  if ( iHeight > 0 )
  {
    m_iItemHeight = iHeight;
  }

  AdjustScrollBars();
  Invalidate();

}



/*-AddString------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CCustomPlacesBar::AddString( const char *szItem, HICON hIcon, DWORD dwItemData )
{

  m_vectItems.push_back( tPBItem() );

  tPBItem&    Item = m_vectItems.back();

  Item.m_strItem      = szItem;
  Item.m_hIcon        = hIcon;
  if ( hIcon )
  {
    Item.m_hIconGreyed  = CWinUtils::CreateGrayscaleIcon( hIcon );
  }
  Item.m_dwItemData   = dwItemData;

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



/*-DeleteString---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CCustomPlacesBar::DeleteString( size_t nIndex )
{

  if ( nIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  DWORD   dwDummy = 0;
  std::vector<tPBItem>::iterator   it( m_vectItems.begin() );

  std::advance( it, nIndex );

  m_vectItems.erase( it );

  if ( m_iSelectedItem == nIndex )
  {
    m_iSelectedItem = -1;
  }
  if ( m_iFocusedItem == nIndex )
  {
    m_iFocusedItem = -1;
  }
  if ( m_iMouseOverItem == nIndex )
  {
    m_iMouseOverItem = -1;
  }
  Invalidate();
  return m_vectItems.size();

}



/*-GetText--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CCustomPlacesBar::GetText( size_t iIndex, LPTSTR lpszBuffer ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  wsprintf( lpszBuffer, m_vectItems[iIndex].m_strItem.c_str() );

  return m_vectItems[iIndex].m_strItem.length();

}



/*-SetItemData----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CCustomPlacesBar::SetItemData( size_t iIndex, DWORD dwData )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  m_vectItems[iIndex].m_dwItemData = dwData;

  return 0;

}



/*-GetItemData----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

DWORD CCustomPlacesBar::GetItemData( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  return m_vectItems[iIndex].m_dwItemData;

}



/*-GetCount-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CCustomPlacesBar::GetCount() const
{

  return m_vectItems.size();

}



/*-OnPaint--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::OnPaint() 
{

  HDC       hdcPaint = GetDC();

  HDC       dc;

  RECT    rcClient,
          rc;

  GetClientRect( &rcClient );

  dc = CreateCompatibleDC( hdcPaint );
  if ( dc == NULL )
  {
    OutputDebugString( "CreateCompatibleDC failed\n" );
  }

  HBITMAP   hbm = CreateCompatibleBitmap( hdcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top ),
            hbmOld;

  hbmOld = (HBITMAP)SelectObject( dc, hbm );



  HFONT hOldFont = (HFONT)SelectObject( dc, m_hFont );

  DRAWITEMSTRUCT    dis;

  dis.CtlID     = GetDlgCtrlID();
  dis.CtlType   = ODT_LISTBOX;
  dis.hDC       = dc;
  dis.hwndItem  = GetSafeHwnd();

  // Hintergrund des Bars
  if ( IsWindowEnabled() )
  {
	  ::SetBkColor( dc, GetSysColor( COLOR_3DSHADOW ) );
	  ::ExtTextOut( dc, 0, 0, ETO_OPAQUE, &rcClient, NULL, 0, NULL);
    SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
  }
  else
  {
	  ::SetBkColor( dc, GetSysColor( COLOR_INACTIVEBORDER ) );
	  ::ExtTextOut( dc, 0, 0, ETO_OPAQUE, &rcClient, NULL, 0, NULL);
    SetTextColor( dc, GetSysColor( COLOR_GRAYTEXT ) );
  }

  // die einzelnen Items darstellen
  bool    bNeedToolTip = false;

  for ( size_t i = m_iOffset; i < m_vectItems.size(); i++ )
  {
    if ( GetItemRect( i, &rc ) != LB_ERR )
    {
      HICON   hIcon = m_vectItems[i].m_hIcon;

      if ( !IsWindowEnabled() )
      {
        hIcon = m_vectItems[i].m_hIconGreyed;
      }

      if ( hIcon == NULL )
      {
        hIcon = LoadIcon( NULL, IDI_QUESTION );
      }

      RECT      rcIcon = rc;

      // Icon darstellen
      if ( m_dwCPStyles & PBS_SMALL_ITEMS )
      {
        rcIcon.left = 8;
        rcIcon.right = rcIcon.left + 16;
        rcIcon.top += ( m_iItemHeight - 16 ) / 2;
        rcIcon.bottom = rcIcon.top + 16;

        ::DrawIconEx( dc, 
                      rcIcon.left, rcIcon.top,
                      hIcon,
                      16, 16,
                      0, NULL,
                      DI_NORMAL );
      }
      else
      {
        rcIcon.left = ( ( rcIcon.right - rcIcon.left ) - 32 ) / 2;
        rcIcon.right -= rcIcon.left;
        //rcIcon.top += 6;
        rcIcon.top += 6 + ( rc.bottom - rc.top - 60 ) / 2;
        rcIcon.bottom = rcIcon.top + 32;

        DrawIcon( dc, rcIcon.left, rcIcon.top, hIcon );
      }

      // Text
      RECT      rcText = rc;

      if ( m_dwCPStyles & PBS_SMALL_ITEMS )
      {
        InflateRect( &rcText, -2, -2 );
        rcText.left += 32;
        if ( i < m_vectItems.size() )
        {
          int   iTextHeight = DrawText( dc, m_vectItems[i].m_strItem.c_str(), 
                      (int)m_vectItems[i].m_strItem.length(),
                      &rcText,
                      DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL | DT_CALCRECT );

          if ( iTextHeight > rc.bottom - rc.top - 4 )
          {
            // Text zu lang

            if ( i == m_iMouseOverItem )
            {
              bNeedToolTip = true;
            }
            iTextHeight = rc.bottom - rc.top - 4;
          }
          rcText.top = rc.top + 2 + ( rc.bottom - rc.top - 4 - iTextHeight ) / 2;
          rcText.bottom = rcText.top + iTextHeight;
          rcText.right = rc.right - 2;    // wurde durch Calcrect verkleinert

          if ( !IsWindowEnabled() )
          {
            SetBkMode( dc, TRANSPARENT );
            SetTextColor( dc, 0xffffff );
            OffsetRect( &rcText, 1, 1 );
            DrawText( dc, m_vectItems[i].m_strItem.c_str(), 
                        (int)m_vectItems[i].m_strItem.length(),
                        &rcText,
                        DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );

            SetTextColor( dc, GetSysColor( COLOR_GRAYTEXT ) );
            OffsetRect( &rcText, -1, -1 );
          }
          DrawText( dc, m_vectItems[i].m_strItem.c_str(), 
                      (int)m_vectItems[i].m_strItem.length(),
                      &rcText,
                      DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );
        }
      }
      else
      {
        rcText.top += 44 + ( rc.bottom - rc.top - 60 ) / 2;

        if ( !IsWindowEnabled() )
        {
          SetBkMode( dc, TRANSPARENT );
          SetTextColor( dc, 0xffffff );
          OffsetRect( &rcText, 1, 1 );
          DrawText( dc, m_vectItems[i].m_strItem.c_str(), 
                      (int)m_vectItems[i].m_strItem.length(),
                      &rcText,
                      DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );

          SetTextColor( dc, GetSysColor( COLOR_GRAYTEXT ) );
          OffsetRect( &rcText, -1, -1 );
        }

        DrawText( dc, m_vectItems[i].m_strItem.c_str(), 
                    (int)m_vectItems[i].m_strItem.length(),
                    &rcText,
                    DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );
      }

      // Selektionsrahmen
      if ( m_dwCPStyles & PBS_SMALL_ITEMS )
      {
        if ( i == m_iSelectedItem )
        {
          DrawEdge( dc, &rc, BDR_RAISED, BF_RECT );
          if ( i == m_iMouseOverItem )
          {
            InflateRect( &rc, -2, -2 );
            DrawEdge( dc, &rc, BDR_RAISEDOUTER, BF_RECT );
            InflateRect( &rc, 2, 2 );
          }
        }
        else if ( i == m_iMouseOverItem )
        {
          DrawEdge( dc, &rc, BDR_RAISEDOUTER, BF_RECT );
        }
      }
      else
      {
        if ( i == m_iSelectedItem )
        {
          InflateRect( &rcIcon, 3, 3 );

          DrawEdge( dc, &rcIcon, BDR_RAISED, BF_RECT );
        }
        else if ( i == m_iMouseOverItem )
        {
          InflateRect( &rcIcon, 3, 3 );

          DrawEdge( dc, &rcIcon, BDR_RAISEDOUTER, BF_RECT );
        }
      }

      // Focus-Rahmen
      if ( ( i == m_iFocusedItem )
      &&   ( m_bHasFocus ) )
      {
        if ( m_dwCPStyles & PBS_SMALL_ITEMS )
        {
          InflateRect( &rc, -3, -3 );
          DrawFocusRect( dc, &rc );
        }
        else
        {
          DrawFocusRect( dc, &rc );
        }
      }
    }
  }

  SelectObject( dc, hOldFont );

  if ( !BitBlt( hdcPaint, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                dc, 0, 0, SRCCOPY ) )
  {
    OutputDebugString( "BitBlt failed\n" );
  }

  SelectObject( dc, hbmOld );

  DeleteObject( hbm );

  ReleaseDC( hdcPaint );

  ValidateRect( &rcClient );

  if ( bNeedToolTip )
  {
    // ist die Maus über dem Item, dann Tooltip aktivieren
    RECT    rcToolTip;

    GetItemRect( m_iMouseOverItem, &rcToolTip );

    ClientToScreen( &rcToolTip );

    m_ToolTip.Activate( TRUE );

    m_ToolTip.UpdateTipText( m_vectItems[m_iMouseOverItem].m_strItem.c_str(), GetSafeHwnd() );
    m_ToolTip.SetWindowPos( HWND_TOPMOST, rcToolTip.left, rcToolTip.top, 200, 30, SWP_NOACTIVATE );
  }
  else
  {
    m_ToolTip.Activate( FALSE );
  }

}



void CCustomPlacesBar::DoLButtonDown( POINT& pt )
{

  BOOL    bOutside;

  size_t iItem = ItemFromPoint( pt, bOutside );

  if ( bOutside )
  {
    return;
  }
  SetCurSel( iItem );

}



size_t CCustomPlacesBar::ItemFromPoint( const POINT& pt, BOOL& bOutside ) const
{

  RECT rcClient;


  GetClientRect( &rcClient );

  bOutside = !PtInRect( &rcClient, pt );
  if ( bOutside )
  {
    return LB_ERR;
  }

  int   iY = pt.y / m_iItemHeight;

  size_t iResult = m_iOffset + iY;

  // da ist kein Item mehr
  if ( iResult >= m_vectItems.size() )
  {
    iResult = LB_ERR;
  }

  return iResult;

}



size_t CCustomPlacesBar::ItemFromPoint( const POINT& pt ) const
{

  BOOL    bOutSide;

  return ItemFromPoint( pt, bOutSide );

}



void CCustomPlacesBar::InvalidateItem( size_t nIndex )
{

  RECT    rc;

  if ( GetItemRect( nIndex, &rc ) == LB_ERR )
  {
    return;
  }
  InvalidateRect( &rc, FALSE );

}



int CCustomPlacesBar::GetItemRect( size_t nIndex, LPRECT lpRect ) const
{

  memset( lpRect, 0, sizeof( RECT ) );

  if ( nIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  RECT    rcClient;


  GetClientRect( &rcClient );

  if ( ( rcClient.right == 0 )
  ||   ( rcClient.bottom == 0 ) )
  {
    return LB_ERR;
  }


  lpRect->left = 0;
  lpRect->right = rcClient.right;
  lpRect->top = LONG( ( nIndex - m_iOffset ) * m_iItemHeight );
  lpRect->bottom = lpRect->top + m_iItemHeight;

  return 0;

}



UINT CCustomPlacesBar::OnGetDlgCode() 
{

	return DLGC_WANTALLKEYS;

}



void CCustomPlacesBar::ScrollIntoView( size_t iItem )
{

  if ( iItem == -1 )
  {
    iItem = m_iSelectedItem;
  }
  if ( iItem == -1 )
  {
    // witzisch
    return;
  }
  while ( iItem < m_iOffset )
  {
    m_iOffset--;
  }
  RECT    rc;
  GetClientRect( &rc );
  int   iVisibleItems = rc.bottom / m_iItemHeight;
  while ( iItem >= m_iOffset + iVisibleItems )
  {
    m_iOffset++;
  }

  SetScrollPos( SB_VERT, (int)m_iOffset, TRUE );

  POINT   ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

  m_iMouseOverItem = ItemFromPoint( ptMouse );

  Invalidate();

}



void CCustomPlacesBar::SetCurSel( size_t iItem )
{

  if ( iItem >= m_vectItems.size() )
  {
    iItem = -1;
  }
  if ( m_iSelectedItem != iItem )
  {
    m_iSelectedItem = iItem;
    m_iFocusedItem = iItem;
    ScrollIntoView();
    Invalidate();
  }
  if ( GetParent() )
  {
    ::SendMessage( GetParent(), WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
  }

}



size_t CCustomPlacesBar::GetCurSel( ) const
{

  if ( m_iSelectedItem == -1 )
  {
    return LB_ERR;
  }
  return m_iSelectedItem;

}



/*-AdjustScrollBars-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::AdjustScrollBars()
{

  RECT    rc;

  GetClientRect( &rc );

  int   iVisibleItems = rc.bottom / m_iItemHeight;


  if ( iVisibleItems > (int)m_vectItems.size() )
  {
    // gibt nichts zu scrollen
    SetScrollRange( SB_VERT, 0, 0 );
    return;
  }

  SetScrollRange( SB_VERT, 0, int( m_vectItems.size() - iVisibleItems ) );

}



/*-AdjustScrollBars-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::OnSize( UINT nType, int cx, int cy )
{

  SetItemSize( m_iItemWidth, m_iItemHeight );
  
  AdjustScrollBars();
  Invalidate();

}



/*-ResetContent---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomPlacesBar::ResetContent()
{

  m_vectItems.clear();

  m_iOffset         = 0;
  m_iSelectedItem   = -1;
  m_iMouseOverItem  = -1;
  m_iFocusedItem    = -1;

  AdjustScrollBars();
  Invalidate();

}



void CCustomPlacesBar::OnMouseMove( UINT nFlags, POINT point )
{

  BOOL      bOutside = FALSE;

  size_t iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    if ( m_iMouseOverItem != -1 )
    {
      m_iMouseOverItem = -1;
      m_ToolTip.Activate( FALSE );
      Invalidate();
    }
  }
  else
  {
    if ( m_iMouseOverItem != iItem )
    {
      m_iMouseOverItem = iItem;
      Invalidate();
    }
  }
  if ( m_iMouseOverItem != -1 )
  {
    if ( m_iTimerID == -1 )
    {
      SetTimer( m_iTimerID, 50, NULL );
    }
  }

  Default();

}



void CCustomPlacesBar::OnTimer(UINT nIDEvent)
{

  if ( nIDEvent == m_iTimerID )
  {
    POINT   ptMousePos;

    GetCursorPos( &ptMousePos );
    ScreenToClient( &ptMousePos );

    RECT    rc;

    GetClientRect( &rc );

    if ( !PtInRect( &rc, ptMousePos ) )
    {
      m_ToolTip.Activate( FALSE );
      m_iMouseOverItem = -1;
      Invalidate();
      KillTimer( m_iTimerID );
      m_iTimerID = -1;
    }
  }

  Default();

}



BOOL CCustomPlacesBar::PreTranslateMessage( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
{

  if ( m_ToolTip.GetSafeHwnd() )
  {
    MSG   msg;

    msg.hwnd = hwnd;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    m_ToolTip.RelayEvent( &msg );
  }

  return CWnd::PreTranslateMessage( hwnd, message, wParam, lParam, lResult );
}



size_t CCustomPlacesBar::GetItemCount() const
{

  return m_vectItems.size();

}


LRESULT CCustomPlacesBar::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
  
  switch ( message )
  {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );
        SetFocus();
	      DoLButtonDown( pt );
      }
      break;
    case WM_ERASEBKGND:
      return TRUE;
    case WM_KILLFOCUS:
      m_bHasFocus = FALSE;
      if ( m_iFocusedItem != -1 )
      {
        Invalidate();
      }
      break;
    case WM_SETFOCUS:
      m_bHasFocus = TRUE;
      if ( m_iFocusedItem != -1 )
      {
        Invalidate();
      }
      break;
    case WM_KEYDOWN:
      if ( m_iFocusedItem != -1 )
      {
        if ( wParam == VK_UP )
        {
          if ( m_iFocusedItem > 0 )
          {
            m_iFocusedItem--;
            ScrollIntoView( m_iFocusedItem );
            Invalidate();
          }
        }
        else if ( wParam == VK_DOWN )
        {
          if ( m_iFocusedItem + 1 < m_vectItems.size() )
          {
            m_iFocusedItem++;
            ScrollIntoView( m_iFocusedItem );
            Invalidate();
          }
        }
        else if ( wParam == VK_RETURN )
        {
          SetCurSel( m_iFocusedItem );
        }
      }
      break;
    case WM_VSCROLL:
      {
        int     iMin,
                iMax;


        GetScrollRange( SB_VERT, &iMin, &iMax );
        if ( iMax == 0 )
        {
          break;
        }

        RECT    rc;

        GetClientRect( &rc );

        size_t  iVisibleItems = rc.bottom / m_iItemHeight;

        size_t  iPageItems = ( iVisibleItems * 3 / 4 );

        size_t  iOldOffset = m_iOffset;

	      switch ( LOWORD( wParam ) )
	      {
	        case SB_LEFT:
            if ( m_iOffset > 0 )
            {
              m_iOffset = 0;
	  	        Invalidate();
            }
		        break;
	        case SB_LINELEFT:
            if ( m_iOffset > 0 )
            {
              m_iOffset--;
              if ( m_iOffset < 0 )
              {
                m_iOffset = 0;
              }
  		        Invalidate();
            }
		        break;
	        case SB_PAGELEFT:
            if ( m_iOffset )
            {
              if ( m_iOffset >= iPageItems )
              {
                m_iOffset -= iPageItems;
              }
              else
              {
                m_iOffset = 0;
              }
  		        Invalidate();
            }
		        break;
	        case SB_LINERIGHT:
            m_iOffset++;
            if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
            {
              m_iOffset = m_vectItems.size() - iVisibleItems;
            }
		        Invalidate();
		        break;
	        case SB_PAGERIGHT:
            m_iOffset += iPageItems;
            if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
            {
              m_iOffset = m_vectItems.size() - iVisibleItems;
            }
  		      Invalidate();
		        break;
	        case SB_RIGHT:
            m_iOffset = m_vectItems.size() - iVisibleItems;
		        Invalidate();
		        break;
	        case SB_THUMBPOSITION:
	        case SB_THUMBTRACK:
            if ( HIWORD( wParam ) != m_iOffset )
            {
              m_iOffset = HIWORD( wParam );
		          Invalidate();
            }
		        break;
	        case SB_ENDSCROLL:		//End scroll.
	        default:
		        break;
	      }

        if ( iOldOffset != m_iOffset )
        {
	        SetScrollPos( SB_VERT, (int)m_iOffset, TRUE );
        }

	      Default();

        POINT   ptMouse;
        GetCursorPos( &ptMouse );
        ScreenToClient( &ptMouse );

        m_iMouseOverItem = ItemFromPoint( ptMouse );
        return 0;
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
    case WM_TIMER:
      OnTimer( (UINT)wParam );
      break;
    case WM_ENABLE:
      Invalidate();
      break;
    case WM_PAINT:
      OnPaint();
      return 0;
  }

  return CWnd::WindowProc( message, wParam, lParam );

}
