#include "stdafx.h"
#include "CustomPlacesBar.h"

#include <WinSys/WinUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



BOOL CCustomPlacesBar::hasclass = CCustomPlacesBar::RegisterMe();



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
  return AfxRegisterClass(&wc);
}




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
  DeleteAllItems();

}


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



BOOL CCustomPlacesBar::Create( RECT &rc, CWnd *wndParent, int iID )
{

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, NULL, NULL,
                        WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, wndParent, iID ) )
  {
    return FALSE;
  }

  Initialize();

  return TRUE;

}



void CCustomPlacesBar::Initialize( DWORD dwCPStyles, int iItemHeight )
{

  m_ToolTip.Create( this );
  

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

  RECT    rc;

  GetClientRect( &rc );

  m_ToolTip.AddTool( this, "Hullehutz" );
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



void CCustomPlacesBar::OnDestroy() 
{

	CWnd::OnDestroy();

  DeleteAllItems();
	
}



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



size_t CCustomPlacesBar::DeleteString( size_t nIndex )
{

  if ( nIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  DWORD   dwDummy = 0;
  std::vector<tPBItem>::iterator   it( m_vectItems.begin() );

  std::advance( it, nIndex );

  tPBItem&    Item = *it;
  if ( Item.m_hIconGreyed )
  {
    DestroyIcon( Item.m_hIconGreyed );
    Item.m_hIconGreyed = NULL;
  }

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



size_t CCustomPlacesBar::GetText( size_t iIndex, LPTSTR lpszBuffer ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }

  wsprintf( lpszBuffer, m_vectItems[iIndex].m_strItem.c_str() );

  return m_vectItems[iIndex].m_strItem.length();

}



int CCustomPlacesBar::SetItemData( size_t iIndex, DWORD dwData )
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  m_vectItems[iIndex].m_dwItemData = dwData;

  return 0;

}



DWORD CCustomPlacesBar::GetItemData( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  return m_vectItems[iIndex].m_dwItemData;

}



size_t CCustomPlacesBar::GetCount() const
{

  return m_vectItems.size();

}



void CCustomPlacesBar::OnPaint() 
{

  CDC*      pDCPaint = GetDC();

  CDC       dc;

  RECT    rcClient,
          rc;

  GetClientRect( &rcClient );

  if ( !dc.CreateCompatibleDC( pDCPaint ) )
  {
    OutputDebugString( "CreateCompatibleDC failed\n" );
  }

  HBITMAP   hbm = CreateCompatibleBitmap( pDCPaint->GetSafeHdc(), rcClient.right - rcClient.left, rcClient.bottom - rcClient.top ),
            hbmOld;

  hbmOld = (HBITMAP)dc.SelectObject( hbm );



  HFONT hOldFont = (HFONT)SelectObject( dc.GetSafeHdc(), m_hFont );

  DRAWITEMSTRUCT    dis;

  dis.CtlID = GetDlgCtrlID();
  dis.CtlType = ODT_LISTBOX;
  dis.hDC = dc.GetSafeHdc();
  dis.hwndItem = GetSafeHwnd();

  // Hintergrund des Bars
  if ( IsWindowEnabled() )
  {
    dc.FillSolidRect( &rcClient, GetSysColor( COLOR_3DSHADOW ) );
    dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
  }
  else
  {
    dc.FillSolidRect( &rcClient, GetSysColor( COLOR_INACTIVEBORDER ) );
    dc.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
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

        ::DrawIconEx( dc.GetSafeHdc(), 
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

        dc.DrawIcon( rcIcon.left, rcIcon.top, hIcon );
      }

      // Text
      RECT      rcText = rc;

      if ( m_dwCPStyles & PBS_SMALL_ITEMS )
      {
        InflateRect( &rcText, -2, -2 );
        rcText.left += 32;
        if ( i < m_vectItems.size() )
        {
          int   iTextHeight = dc.DrawText( m_vectItems[i].m_strItem.c_str(), 
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
            dc.SetBkMode( TRANSPARENT );
            dc.SetTextColor( 0xffffff );
            OffsetRect( &rcText, 1, 1 );
            dc.DrawText( m_vectItems[i].m_strItem.c_str(), 
                        (int)m_vectItems[i].m_strItem.length(),
                        &rcText,
                        DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );

            dc.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
            OffsetRect( &rcText, -1, -1 );
          }
          dc.DrawText( m_vectItems[i].m_strItem.c_str(), 
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
          dc.SetBkMode( TRANSPARENT );
          dc.SetTextColor( 0xffffff );
          OffsetRect( &rcText, 1, 1 );
          dc.DrawText( m_vectItems[i].m_strItem.c_str(), 
                      (int)m_vectItems[i].m_strItem.length(),
                      &rcText,
                      DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );

          dc.SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
          OffsetRect( &rcText, -1, -1 );
        }

        dc.DrawText( m_vectItems[i].m_strItem.c_str(), 
                    (int)m_vectItems[i].m_strItem.length(),
                    &rcText,
                    DT_CENTER | DT_WORDBREAK | DT_EDITCONTROL );
      }

      // Selektionsrahmen
      if ( m_dwCPStyles & PBS_SMALL_ITEMS )
      {
        if ( i == m_iSelectedItem )
        {
          dc.DrawEdge( &rc, BDR_RAISED, BF_RECT );
          if ( i == m_iMouseOverItem )
          {
            InflateRect( &rc, -2, -2 );
            dc.DrawEdge( &rc, BDR_RAISEDOUTER, BF_RECT );
            InflateRect( &rc, 2, 2 );
          }
        }
        else if ( i == m_iMouseOverItem )
        {
          dc.DrawEdge( &rc, BDR_RAISEDOUTER, BF_RECT );
        }
      }
      else
      {
        if ( i == m_iSelectedItem )
        {
          InflateRect( &rcIcon, 3, 3 );

          dc.DrawEdge( &rcIcon, BDR_RAISED, BF_RECT );
        }
        else if ( i == m_iMouseOverItem )
        {
          InflateRect( &rcIcon, 3, 3 );

          dc.DrawEdge( &rcIcon, BDR_RAISEDOUTER, BF_RECT );
        }
      }

      // Focus-Rahmen
      if ( ( i == m_iFocusedItem )
      &&   ( m_bHasFocus ) )
      {
        if ( m_dwCPStyles & PBS_SMALL_ITEMS )
        {
          InflateRect( &rc, -3, -3 );
          dc.DrawFocusRect( &rc );
        }
        else
        {
          dc.DrawFocusRect( &rc );
        }
      }
    }
  }

  SelectObject( dc.GetSafeHdc(), hOldFont );

  if ( !pDCPaint->BitBlt( 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                  &dc, 0, 0, SRCCOPY ) )
  {
    OutputDebugString( "BitBlt failed\n" );
  }

  dc.SelectObject( hbmOld );

  DeleteObject( hbm );

  ReleaseDC( pDCPaint );

  ValidateRect( &rcClient );

  if ( bNeedToolTip )
  {
    // ist die Maus über dem Item, dann Tooltip aktivieren
    RECT    rcToolTip;

    GetItemRect( m_iMouseOverItem, &rcToolTip );

    ClientToScreen( &rcToolTip );

    m_ToolTip.Activate( TRUE );

    m_ToolTip.UpdateTipText( m_vectItems[m_iMouseOverItem].m_strItem.c_str(), this );
    m_ToolTip.SetWindowPos( &CWnd::wndTopMost, rcToolTip.left, rcToolTip.top, 200, 30, SWP_NOACTIVATE );
  }
  else
  {
    m_ToolTip.Activate( FALSE );
  }

}



void CCustomPlacesBar::OnLButtonDown(UINT nFlags, CPoint point) 
{

  SetFocus();
	DoLButtonDown( point );
  
	CWnd::OnLButtonDown(nFlags, point);

}

void CCustomPlacesBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{

  SetFocus();
	DoLButtonDown( point );
	
	CWnd::OnLButtonDblClk(nFlags, point);

}



void CCustomPlacesBar::DoLButtonDown( CPoint &point )
{

  BOOL    bOutside;

  size_t iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    return;
  }
  SetCurSel( iItem );

}



size_t CCustomPlacesBar::ItemFromPoint( CPoint pt, BOOL& bOutside ) const
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



size_t CCustomPlacesBar::ItemFromPoint( CPoint pt ) const
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



BOOL CCustomPlacesBar::OnEraseBkgnd( CDC* pDC ) 
{

  return TRUE;

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
    GetParent()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
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



void CCustomPlacesBar::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

  m_bHasFocus = FALSE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}



void CCustomPlacesBar::OnSetFocus(CWnd* pOldWnd) 
{

	CWnd::OnSetFocus( pOldWnd );
	
  m_bHasFocus = TRUE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}

void CCustomPlacesBar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

  if ( m_iFocusedItem != -1 )
  {
    if ( nChar == VK_UP )
    {
      if ( m_iFocusedItem > 0 )
      {
        m_iFocusedItem--;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_DOWN )
    {
      if ( m_iFocusedItem + 1 < m_vectItems.size() )
      {
        m_iFocusedItem++;
        ScrollIntoView( m_iFocusedItem );
        Invalidate();
      }
    }
    else if ( nChar == VK_RETURN )
    {
      SetCurSel( m_iFocusedItem );
    }
  }

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

}



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



void CCustomPlacesBar::OnSize( UINT nType, int cx, int cy )
{

  SetItemSize( m_iItemWidth, m_iItemHeight );
  
  AdjustScrollBars();
  Invalidate();

}



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



void CCustomPlacesBar::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) 
{

  int     iMin,
          iMax;


  GetScrollRange( SB_VERT, &iMin, &iMax );
  if ( iMax == 0 )
  {
    return;
  }

  RECT    rc;

  GetClientRect( &rc );

  size_t  iVisibleItems = rc.bottom / m_iItemHeight;

  size_t  iPageItems = ( iVisibleItems * 3 / 4 );

  size_t  iOldOffset = m_iOffset;

	switch ( nSBCode )
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
      if ( nPos != m_iOffset )
      {
        m_iOffset = nPos;
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

	CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

  POINT   ptMouse;
  GetCursorPos( &ptMouse );
  ScreenToClient( &ptMouse );

  m_iMouseOverItem = ItemFromPoint( ptMouse );

}



void CCustomPlacesBar::OnMouseMove(UINT nFlags, CPoint point)
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

  CWnd::OnMouseMove(nFlags, point);
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

  CWnd::OnTimer(nIDEvent);
}



void CCustomPlacesBar::OnEnable(BOOL bEnable)
{
  CWnd::OnEnable(bEnable);

  Invalidate();

}



BOOL CCustomPlacesBar::PreTranslateMessage(MSG* pMsg)
{

  if ( m_ToolTip.GetSafeHwnd() )
  {
    m_ToolTip.RelayEvent( pMsg );
  }



  return CWnd::PreTranslateMessage(pMsg);
}



size_t CCustomPlacesBar::GetItemCount() const
{

  return m_vectItems.size();

}



void CCustomPlacesBar::DeleteAllItems()
{

  std::vector<tPBItem>::iterator   it( m_vectItems.begin() );
  while ( it != m_vectItems.end() )
  {
    tPBItem&    Item = *it;
    if ( Item.m_hIconGreyed )
    {
      DestroyIcon( Item.m_hIconGreyed );
      Item.m_hIconGreyed = NULL;
    }
    ++it;
  }
  m_vectItems.clear();

}