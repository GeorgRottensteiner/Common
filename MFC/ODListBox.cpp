#include "stdafx.h"
#include <MFC\ODListBox.h>
#include <MFC/MemDC.h>

#include <String/Convert.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL CODListBox::hasclass = CODListBox::RegisterMe();



BOOL CODListBox::RegisterMe()
{
  WNDCLASS wc;   
  wc.style = CS_DBLCLKS;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = ODLISTBOX_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



CODListBox::CODListBox()
: m_iItemWidth( -1 ),
  m_iItemHeight( 14 ),
  m_iOffset( 0 ),
  m_iSelectedItem( -1 ),
  m_iFocusedItem( -1 ),
  m_bHasFocus( FALSE ),
  m_iItemsPerLine( 1 ),
  m_bOwnerDraw( FALSE ),
  m_dwID( 0 )
{
}

CODListBox::~CODListBox()
{
}


BEGIN_MESSAGE_MAP(CODListBox, CWnd)
	//{{AFX_MSG_MAP(CODListBox)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CODListBox 


/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CODListBox::Create( RECT &rc, CWnd *wndParent, int iID )
{

  if ( !CWnd::CreateEx( WS_EX_NOPARENTNOTIFY | WS_EX_LEFT | WS_EX_RIGHTSCROLLBAR | WS_EX_CLIENTEDGE, NULL, NULL,
                        WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_CHILD, rc, wndParent, iID ) )
  {
    return FALSE;
  }

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
  m_dwID = iID;

  return TRUE;

}



/*-OnDestroy------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CODListBox::OnDestroy() 
{

	CWnd::OnDestroy();
	
  for ( size_t i = 0; i < m_vectItems.size(); i++ )
  {
    delete m_vectItems[i];
  }
  m_vectItems.clear();
	
}



/*-SetOwnerDraw---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CODListBox::SetOwnerDraw( BOOL bOD )
{

  m_bOwnerDraw = bOD;
  Invalidate();

}



/*-SetItemSize----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CODListBox::SetItemSize( int iWidth, int iHeight )
{

  if ( iWidth != 0 )
  {
    m_iItemWidth = iWidth;
  }
  if ( iHeight > 0 )
  {
    m_iItemHeight = iHeight;
  }

  if ( m_iItemWidth == -1 )
  {
    m_iItemsPerLine = 1;
  }
  else
  {
    RECT    rc;
    GetClientRect( &rc );
    m_iItemsPerLine = rc.right / m_iItemWidth;

    // eines MUSS dargestellt werden
    if ( m_iItemsPerLine == 0 )
    {
      m_iItemsPerLine = 1;
    }
  }

  m_iOffset -= m_iOffset % m_iItemsPerLine;

  AdjustScrollBars();
  Invalidate();

}



size_t CODListBox::AddString( const GR::Char* Item )
{
  CODItem *pItem      = new CODItem();
  pItem->m_strItem    = Item;
  pItem->m_dwItemData = 0;
  m_vectItems.push_back( pItem );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;
}



size_t CODListBox::DeleteString( size_t nIndex )
{
  if ( ( nIndex < 0 )
  ||   ( nIndex >= m_vectItems.size() ) )
  {
    return LB_ERR;
  }

  DWORD   dwDummy = 0;
  std::vector<CODItem*>::iterator   it( m_vectItems.begin() );

  while ( it != m_vectItems.end() )
  {
    if ( dwDummy == nIndex )
    {
      delete *it;
      m_vectItems.erase( it );

      if ( m_iSelectedItem >= (int)m_vectItems.size() )
      {
        m_iSelectedItem = -1;
      }
      if ( m_iFocusedItem >= (int)m_vectItems.size() )
      {
        m_iFocusedItem = -1;
      }
      Invalidate();
      return m_vectItems.size();
    }

    dwDummy++;
    it++;
  }

  return m_vectItems.size();

}



size_t CODListBox::GetText( int iIndex, LPTSTR lpszBuffer ) const
{
  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return LB_ERR;
  }

  wsprintf( lpszBuffer, GR::Convert::ToUTF16( m_vectItems[iIndex]->m_strItem ).c_str() );

  return m_vectItems[iIndex]->m_strItem.length();
}



GR::String CODListBox::GetText( int iIndex ) const
{

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return "";
  }
  return GR::Convert::ToUTF8( m_vectItems[iIndex]->m_strItem );
}



int CODListBox::SetItemData( size_t iIndex, DWORD_PTR dwData )
{
  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  m_vectItems[iIndex]->m_dwItemData = dwData;

  return 0;
}



DWORD_PTR CODListBox::GetItemData( size_t iIndex ) const
{

  if ( iIndex >= m_vectItems.size() )
  {
    return LB_ERR;
  }
  return m_vectItems[iIndex]->m_dwItemData;

}



size_t CODListBox::GetCount() const
{

  return m_vectItems.size();

}



void CODListBox::OnPaint() 
{

	CPaintDC pdc( this );

  RECT    rcB;
  GetClientRect( &rcB );

  MFCExtension::CMemDC    dc( &pdc, rcB );

  dc.FillSolidRect( &rcB, GetSysColor( COLOR_BTNFACE ) );

  HFONT hOldFont = (HFONT)SelectObject( dc.GetSafeHdc(), m_hFont );

  DRAWITEMSTRUCT    dis;

  dis.CtlID = m_dwID;
  dis.CtlType = ODT_LISTBOX;
  dis.hDC = dc.GetSafeHdc();
  dis.hwndItem = GetSafeHwnd();

  RECT    rcClient,
          rc;

  GetClientRect( &rcClient );

  for ( size_t i = m_iOffset; i < m_vectItems.size(); i++ )
  {
    if ( GetItemRect( i, &rc ) != LB_ERR )
    {
      if ( m_bOwnerDraw )
      {
        dis.itemAction = ODA_DRAWENTIRE;
        dis.itemData = m_vectItems[i]->m_dwItemData;
        dis.itemID = (UINT)i;
        dis.itemState = 0;
        dis.rcItem = rc;
        if ( i == m_iSelectedItem )
        {
          dis.itemState |= ODS_SELECTED;
        }
        if ( i == m_iFocusedItem )
        {
          dis.itemState |= ODS_FOCUS;
        }
        GetOwner()->SendMessage( WM_DRAWITEM, m_dwID, (LPARAM)&dis );
        continue;
      }

      if ( i == m_iSelectedItem )
      {
        dc.FillSolidRect( &rc, GetSysColor( COLOR_HIGHLIGHT ) );
        dc.SetTextColor( GetSysColor( COLOR_HIGHLIGHTTEXT ) );
      }
      else
      {
        dc.FillSolidRect( &rc, GetSysColor( COLOR_WINDOW ) );
        dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
      }
      if ( i < m_vectItems.size() )
      {
        dc.DrawText( GR::Convert::ToUTF16( m_vectItems[i]->m_strItem ).c_str(), 
                     (int)m_vectItems[i]->m_strItem.length(),
                     &rc,
                     DT_SINGLELINE | DT_VCENTER | DT_CENTER );
      }
      if ( ( m_bHasFocus )
      &&   ( m_iFocusedItem == i ) )
      {
        dc.DrawFocusRect( &rc );
      }
    }
  }
  SelectObject( dc.GetSafeHdc(), hOldFont );
}



void CODListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{

	DoLButtonDown( point );
	
  SetFocus();
	CWnd::OnLButtonDown(nFlags, point);

}

void CODListBox::OnLButtonDblClk(UINT nFlags, CPoint point) 
{

	DoLButtonDown( point );
	
  SetFocus();
	CWnd::OnLButtonDblClk(nFlags, point);

}



void CODListBox::DoLButtonDown( CPoint &point )
{

  BOOL    bOutside;

  size_t iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    return;
  }
  if ( m_iSelectedItem != iItem )
  {
    if ( m_iSelectedItem != -1 )
    {
      InvalidateItem( m_iSelectedItem );
    }
    if ( iItem != -1 )
    {
      m_iFocusedItem = iItem;
    }
    m_iSelectedItem = iItem;
    InvalidateItem( m_iSelectedItem );

    if ( GetParent() )
    {
      GetParent()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
    }
    //Invalidate();
  }

}



size_t CODListBox::ItemFromPoint( CPoint pt, BOOL& bOutside ) const
{

  RECT rcClient;


  GetClientRect( &rcClient );

  bOutside = !PtInRect( &rcClient, pt );
  if ( bOutside )
  {
    return LB_ERR;
  }

  // 2 Fälle, A) normale Listbox, B) mehrspaltige Listbox
  size_t iResult = LB_ERR;
  if ( m_iItemWidth == -1 )
  {
    iResult = m_iOffset + ( pt.y / m_iItemHeight );
    if ( iResult >= (int)m_vectItems.size() )
    {
      iResult = LB_ERR;
    }
    return iResult;
  }

  int   iX = pt.x / m_iItemWidth,
        iY = pt.y / m_iItemHeight;

  if ( iX >= m_iItemsPerLine )
  {
    // in dem Restbereich zwischen Items und rechtem Rand
    return LB_ERR;
  }
  iResult = m_iOffset + iY * m_iItemsPerLine + iX;

  // da ist kein Item mehr
  if ( iResult >= m_vectItems.size() )
  {
    iResult = LB_ERR;
  }

  return iResult;

}



size_t CODListBox::ItemFromPoint( CPoint pt ) const
{

  BOOL    bOutSide;

  return ItemFromPoint( pt, bOutSide );

}



void CODListBox::InvalidateItem( int nIndex )
{

  RECT    rc;

  if ( GetItemRect( nIndex, &rc ) == LB_ERR )
  {
    return;
  }
  InvalidateRect( &rc, FALSE );

}



int CODListBox::GetItemRect( size_t nIndex, LPRECT lpRect ) const
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


  // 2 Fälle, A) normale Listbox, B) mehrspaltige Listbox
  if ( m_iItemWidth == -1 )
  {
    lpRect->left = 0;
    lpRect->right = rcClient.right;
    lpRect->top = ( nIndex - m_iOffset ) * m_iItemHeight;
    lpRect->bottom = lpRect->top + m_iItemHeight;
    return 0;
  }

  lpRect->left = ( nIndex % m_iItemsPerLine ) * m_iItemWidth;
  lpRect->top = ( ( nIndex - m_iOffset ) / m_iItemsPerLine ) * m_iItemHeight;
  lpRect->right = lpRect->left + m_iItemWidth;
  lpRect->bottom = lpRect->top + m_iItemHeight;

  return 0;

}



UINT CODListBox::OnGetDlgCode() 
{

	return DLGC_WANTALLKEYS;

}



BOOL CODListBox::OnEraseBkgnd( CDC* pDC ) 
{

  return TRUE;

}



void CODListBox::ScrollIntoView()
{

  if ( m_iSelectedItem == -1 )
  {
    // witzisch
    return;
  }
  while ( m_iSelectedItem < m_iOffset )
  {
    m_iOffset -= m_iItemsPerLine;
  }
  RECT    rc;
  GetClientRect( &rc );
  int   iVisibleItems = rc.bottom / m_iItemHeight * m_iItemsPerLine;
  while ( m_iSelectedItem > m_iOffset + iVisibleItems )
  {
    m_iOffset += m_iItemsPerLine;
  }
  Invalidate();

}



void CODListBox::SetCurSel( int iItem )
{

  if ( ( iItem < 0 )
  ||   ( iItem >= (int)m_vectItems.size() ) )
  {
    iItem = -1;
    if ( m_iSelectedItem != -1 )
    {
      m_iSelectedItem = iItem;
      m_iFocusedItem = iItem;
      Invalidate();
    }
    return;
  }

  if ( m_iFocusedItem != iItem )
  {
    m_iFocusedItem = iItem;
    m_iSelectedItem = m_iFocusedItem;
    ScrollIntoView();
    Invalidate();
  }

}



int CODListBox::GetCurSel( ) const
{

  if ( m_iSelectedItem == -1 )
  {
    return LB_ERR;
  }
  return m_iSelectedItem;

}



void CODListBox::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

  m_bHasFocus = FALSE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}



void CODListBox::OnSetFocus(CWnd* pOldWnd) 
{

	CWnd::OnSetFocus( pOldWnd );
	
  m_bHasFocus = TRUE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}

void CODListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

  if ( m_iFocusedItem != -1 )
  {
    if ( nChar == VK_LEFT )
    {
      int   iItem = m_iFocusedItem - 1;
      if ( iItem < 0 )
      {
        iItem = 0;
      }
      SetCurSel( iItem );
    }
    else if ( nChar == VK_RIGHT )
    {
      int   iItem = m_iFocusedItem + 1;
      if ( iItem >= (int)m_vectItems.size() )
      {
        iItem = m_iFocusedItem;
      }
      SetCurSel( iItem );
    }
    if ( m_iItemWidth != -1 )
    {
      // Up/Down ist unterschiedlich!
      RECT    rc;
      GetClientRect( &rc );
      if ( nChar == VK_UP )
      {
        if ( m_iFocusedItem >= rc.right / m_iItemWidth )
        {
          SetCurSel( m_iFocusedItem - ( rc.right / m_iItemWidth ) );
        }
        else
        {
          SetCurSel( 0 );
        }
      }
      else if ( nChar == VK_DOWN )
      {
        int   iItem = m_iFocusedItem + rc.right / m_iItemWidth;
        if ( iItem >= (int)m_vectItems.size() )
        {
          SetCurSel( (int)m_vectItems.size() - 1 );
        }
        else
        {
          SetCurSel( iItem );
        }
      }
    }
  }

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

}



/*-AdjustScrollBars-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CODListBox::AdjustScrollBars()
{

  RECT    rc;

  GetClientRect( &rc );

  int   iVisibleItems = rc.bottom / m_iItemHeight * m_iItemsPerLine;


  if ( iVisibleItems >= (int)m_vectItems.size() )
  {
    // gibt nichts zu scrollen
    SetScrollRange( SB_VERT, 0, 0 );
    return;
  }

  SetScrollRange( SB_VERT, 0, (int)m_vectItems.size() );// - iVisibleItems );

}



void CODListBox::OnSize( UINT nType, int cx, int cy )
{

  SetItemSize( m_iItemWidth, m_iItemHeight );
  
  AdjustScrollBars();
  Invalidate();

}



void CODListBox::ResetContent()
{

  for ( size_t i = 0; i < m_vectItems.size(); i++ )
  {
    delete m_vectItems[i];
  }
  m_vectItems.clear();

  m_iOffset = 0;

  AdjustScrollBars();
  Invalidate();

}



void CODListBox::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) 
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

  int   iVisibleItems = rc.bottom / m_iItemHeight * m_iItemsPerLine,
        iPageItems = ( iVisibleItems * 3 / 4 ) - ( iVisibleItems * 3 / 4 ) % m_iItemsPerLine;

  int   iOldOffset = m_iOffset;

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
        m_iOffset -= m_iItemsPerLine;
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
        m_iOffset -= iPageItems;
        if ( m_iOffset < 0 )
        {
          m_iOffset = 0;
        }
  		  Invalidate();
      }
		  break;
	  case SB_LINERIGHT:
      m_iOffset += m_iItemsPerLine;
      if ( m_iOffset >= (int)m_vectItems.size() )
      {
        m_iOffset = (int)m_vectItems.size() - 1;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
      /*
      if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
      */
		  Invalidate();
		  break;
	  case SB_PAGERIGHT:
      m_iOffset += iPageItems;
      if ( m_iOffset >= (int)m_vectItems.size() )
      {
        m_iOffset = (int)m_vectItems.size() - 1;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
      /*
      if ( m_iOffset >= m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
      */
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_iOffset = (int)m_vectItems.size() - iVisibleItems;
      m_iOffset -= m_iOffset % m_iItemsPerLine;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      if ( nPos - m_iOffset % m_iItemsPerLine != m_iOffset )
      {
        m_iOffset = nPos;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
		    Invalidate();
      }
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}

  if ( iOldOffset != m_iOffset )
  {
	  SetScrollPos( SB_VERT, m_iOffset, TRUE );
  }

	CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

}



void CODListBox::PreSubclassWindow()
{

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

  CWnd::PreSubclassWindow();
}



BOOL CODListBox::PreTranslateMessage( MSG* pMsg )
{
  if ( pMsg->message == WM_MOUSEWHEEL )
  {
    int zDelta = GET_WHEEL_DELTA_WPARAM( pMsg->wParam );

    zDelta /= WHEEL_DELTA; 
    for ( int i = 0; i < abs( zDelta ); ++i )
    {
      if ( zDelta < 0 )
      {
        OnVScroll( SB_LINEDOWN, 0, NULL );
      }
      else
      {
        OnVScroll( SB_LINEUP, 0, NULL );
      }
    }
    return TRUE;
  }

  return CWnd::PreTranslateMessage( pMsg );
}