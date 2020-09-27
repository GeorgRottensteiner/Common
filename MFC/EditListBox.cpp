// ODListBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include <MFC\EditListBox.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditListBox



BOOL CEditListBox::hasclass = CEditListBox::RegisterMe();
DWORD CEditListBox::dwELBM = RegisterWindowMessage( EDITLISTBOX_MESSAGE );


CEditListBox::CEditListBox()
: m_iItemWidth( -1 ),
  m_iItemHeight( 14 ),
  m_iOffset( 0 ),
  m_iSelectedItem( -1 ),
  m_iFocusedItem( -1 ),
  m_bHasFocus( FALSE ),
  m_iItemsPerLine( 1 ),
  m_bOwnerDraw( FALSE ),
  m_dwID( 0 ),
  m_bEditLabel( false ),
  m_pPopupEdit( NULL )
{

  m_hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
}

CEditListBox::~CEditListBox()
{

  if ( m_pPopupEdit )
  {
    m_pPopupEdit->DestroyWindow();
    delete m_pPopupEdit;
    m_pPopupEdit = NULL;
  }

}



/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CEditListBox::RegisterMe()
{
  WNDCLASS wc;   
  wc.style = 0;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = NULL;   // we use OnSetCursor                  
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = EDITLISTBOX_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



#define POPUPEDITCHANGE CPopupEdit::dwPEM

BEGIN_MESSAGE_MAP(CEditListBox, CWnd)
	//{{AFX_MSG_MAP(CEditListBox)
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
  ON_REGISTERED_MESSAGE( POPUPEDITCHANGE, OnPopupEditChange )
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CEditListBox 


/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CEditListBox::Create( RECT &rc, CWnd *wndParent, int iID )
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

void CEditListBox::OnDestroy() 
{

	CWnd::OnDestroy();

  ResetContent();
	
}



/*-ResetContent---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::ResetContent()
{

  for ( size_t i = 0; i < m_vectItems.size(); i++ )
  {
    delete m_vectItems[i];
  }
  m_vectItems.clear();
  Invalidate();

}



/*-SetOwnerDraw---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::SetOwnerDraw( BOOL bOD )
{

  m_bOwnerDraw = bOD;
  Invalidate();

}



/*-SetItemSize----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::SetItemSize( int iWidth, int iHeight )
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
  Invalidate();

}



/*-AddString------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CEditListBox::AddString( const char *szItem )
{

  CELItem *pItem = new CELItem();
  pItem->m_strItem = szItem;
  pItem->m_dwItemData = 0;
  m_vectItems.push_back( pItem );

  AdjustScrollBars();
  Invalidate();

  return m_vectItems.size() - 1;

}



/*-DeleteString---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CEditListBox::DeleteString( UINT nIndex )
{

  if ( ( nIndex < 0 )
  ||   ( nIndex >= m_vectItems.size() ) )
  {
    return LB_ERR;
  }

  DWORD   dwDummy = 0;
  std::vector<CELItem*>::iterator   it( m_vectItems.begin() );

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



/*-GetText--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CEditListBox::GetText( int iIndex, LPTSTR lpszBuffer ) const
{

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return LB_ERR;
  }

  wsprintf( lpszBuffer, m_vectItems[iIndex]->m_strItem.c_str() );

  return m_vectItems[iIndex]->m_strItem.length();

}



/*-SetItemText----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::SetItemText( int iIndex, const char *szBuffer )
{

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return;
  }

  m_vectItems[iIndex]->m_strItem = szBuffer;

  RECT    rc;
  GetItemRect( m_iFocusedItem, &rc );
  InvalidateRect( &rc );

}



/*-SetItemData----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CEditListBox::SetItemData( int iIndex, DWORD dwData )
{

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return LB_ERR;
  }
  m_vectItems[iIndex]->m_dwItemData = dwData;

  return 0;

}



/*-GetItemData----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

DWORD CEditListBox::GetItemData( int iIndex ) const
{

  if ( ( iIndex < 0 )
  ||   ( iIndex >= (int)m_vectItems.size() ) )
  {
    return LB_ERR;
  }
  return m_vectItems[iIndex]->m_dwItemData;

}



/*-GetCount-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CEditListBox::GetCount() const
{

  return m_vectItems.size();

}



/*-OnPaint--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::OnPaint() 
{

	CPaintDC dc( this );

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
        dis.itemID = i;
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
        rc.left += 2;
        dc.DrawText( m_vectItems[i]->m_strItem.c_str(), 
                     m_vectItems[i]->m_strItem.length(),
                     &rc,
                     DT_SINGLELINE | DT_VCENTER | DT_LEFT );
        rc.left -= 2;
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



void CEditListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{

	DoLButtonDown( point );
	
  SetFocus();
	CWnd::OnLButtonDown(nFlags, point);

}

void CEditListBox::OnLButtonDblClk(UINT nFlags, CPoint point) 
{

	DoLButtonDown( point );
	
  SetFocus();
	CWnd::OnLButtonDblClk(nFlags, point);

}



void CEditListBox::DoLButtonDown( CPoint &point )
{

  BOOL    bOutside;

  int iItem = ItemFromPoint( point, bOutside );

  if ( bOutside )
  {
    return;
  }
  if ( m_iSelectedItem != iItem )
  {
    if ( iItem != -1 )
    {
      m_iFocusedItem = iItem;
    }
    m_iSelectedItem = iItem;

    CWnd *wndParent = GetParent();
    if ( wndParent )
    {
      wndParent->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
    }

    Invalidate();
  }

}



UINT CEditListBox::ItemFromPoint( CPoint pt, BOOL& bOutside ) const
{

  RECT rcClient;


  GetClientRect( &rcClient );

  bOutside = !PtInRect( &rcClient, pt );
  if ( bOutside )
  {
    return -1;
  }

  // 2 Fälle, A) normale Listbox, B) mehrspaltige Listbox
  int   iResult = -1;
  if ( m_iItemWidth == -1 )
  {
    iResult = m_iOffset + ( pt.y / m_iItemHeight );
    if ( iResult >= (int)m_vectItems.size() )
    {
      iResult = -1;
    }
    return iResult;
  }

  int   iX = pt.x / m_iItemWidth,
        iY = pt.y / m_iItemHeight;

  if ( iX >= m_iItemsPerLine )
  {
    // in dem Restbereich zwischen Items und rechtem Rand
    return -1;
  }
  iResult = m_iOffset + iY * m_iItemsPerLine + iX;

  // da ist kein Item mehr
  if ( iResult >= (int)m_vectItems.size() )
  {
    iResult = -1;
  }

  return iResult;

}



UINT CEditListBox::ItemFromPoint( CPoint pt ) const
{

  BOOL    bOutSide;

  return ItemFromPoint( pt, bOutSide );

}



int CEditListBox::GetItemRect( int nIndex, LPRECT lpRect ) const
{

  memset( lpRect, 0, sizeof( RECT ) );

  if ( ( nIndex < 0 )
  ||   ( nIndex >= (int)m_vectItems.size() ) )
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



UINT CEditListBox::OnGetDlgCode() 
{

	return DLGC_WANTALLKEYS;

}



BOOL CEditListBox::OnEraseBkgnd( CDC* pDC ) 
{

  RECT    rc;
  GetClientRect( &rc );
  pDC->FillSolidRect( &rc, GetSysColor( COLOR_WINDOW ) );

  return TRUE;
	
	//return CWnd::OnEraseBkgnd(pDC);
}



void CEditListBox::ScrollIntoView()
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



void CEditListBox::SetCurSel( int iItem )
{

  if ( ( iItem < 0 )
  ||   ( iItem >= (int)m_vectItems.size() ) )
  {
    iItem = -1;
    if ( m_iSelectedItem != -1 )
    {
      m_iSelectedItem = iItem;
      m_iFocusedItem = iItem;

      CWnd *wndParent = GetParent();
      if ( wndParent )
      {
        wndParent->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
      }

      Invalidate();
    }
    return;
  }

  if ( m_iFocusedItem != iItem )
  {
    m_iFocusedItem = iItem;
    m_iSelectedItem = m_iFocusedItem;

    CWnd *wndParent = GetParent();
    if ( wndParent )
    {
      wndParent->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)GetSafeHwnd() );
    }

    ScrollIntoView();
    Invalidate();
  }

}



int CEditListBox::GetCurSel( ) const
{

  if ( m_iSelectedItem == -1 )
  {
    return LB_ERR;
  }
  return m_iSelectedItem;

}



void CEditListBox::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);

  m_bHasFocus = FALSE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}



/*-OnSetFocus-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::OnSetFocus( CWnd* pOldWnd ) 
{

	CWnd::OnSetFocus( pOldWnd );
	
  m_bHasFocus = TRUE;
  if ( m_iFocusedItem != -1 )
  {
    Invalidate();
  }
	
}



/*-BeginLabelEdit-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::BeginLabelEdit()
{

  if ( m_bEditLabel )
  {
    return;
  }
  if ( m_iFocusedItem == -1 )
  {
    return;
  }
  m_bEditLabel = true;
  m_pPopupEdit = new CPopupEdit();

  RECT    rc;

  char    szBuffer[MAX_PATH];

  GetItemRect( m_iFocusedItem, &rc );
  GetText( m_iFocusedItem, szBuffer );
  m_pPopupEdit->Create( WS_CHILD | ES_LEFT | WS_VISIBLE | WS_BORDER, 
                        rc,
                        this,
                        17 );
  m_pPopupEdit->SendMessage( WM_SETFONT, (WPARAM)m_hFont, TRUE );
  m_pPopupEdit->SetFocus();
  m_pPopupEdit->SetWindowText( szBuffer );
  m_pPopupEdit->SetSel( 0, -1, TRUE );

}



/*-EndLabelEdit---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::EndLabelEdit()
{

  if ( !m_bEditLabel )
  {
    return;
  }
  m_bEditLabel = false;
  if ( m_pPopupEdit )
  {
    m_pPopupEdit->DestroyWindow();
    delete m_pPopupEdit;
    m_pPopupEdit = NULL;
  }

}



/*-OnKeyDown------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

  if ( m_bEditLabel )
  {
    return;
  }
  if ( m_iFocusedItem != -1 )
  {
    if ( nChar == VK_F2 )
    {
      BeginLabelEdit();
    }
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
          SetCurSel( m_vectItems.size() - 1 );
        }
        else
        {
          SetCurSel( iItem );
        }
      }
    }
  }

	//CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

}



/*-AdjustScrollBars-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::AdjustScrollBars()
{

  RECT    rc;

  GetClientRect( &rc );

  int   iVisibleItems = rc.bottom / m_iItemHeight * m_iItemsPerLine;


  if ( iVisibleItems > (int)m_vectItems.size() )
  {
    // gibt nichts zu scrollen
    SetScrollRange( SB_VERT, 0, 0 );
    return;
  }

  SetScrollRange( SB_VERT, 0, m_vectItems.size() - iVisibleItems );

}



/*-OnVScroll------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CEditListBox::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) 
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
      m_iOffset -= iPageItems;
      if ( m_iOffset < 0 )
      {
        m_iOffset = 0;
      }
		  Invalidate();
		  break;
	  case SB_LINERIGHT:
      m_iOffset += m_iItemsPerLine;
      if ( m_iOffset >= (int)m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
		  Invalidate();
		  break;
	  case SB_PAGERIGHT:
      m_iOffset += iPageItems;
      if ( m_iOffset >= (int)m_vectItems.size() - iVisibleItems )
      {
        m_iOffset = m_vectItems.size() - iVisibleItems;
        m_iOffset -= m_iOffset % m_iItemsPerLine;
      }
  		Invalidate();
		  break;
	  case SB_RIGHT:
      m_iOffset = m_vectItems.size() - iVisibleItems;
      m_iOffset -= m_iOffset % m_iItemsPerLine;
		  Invalidate();
		  break;
	  case SB_THUMBPOSITION:
	  case SB_THUMBTRACK:
      m_iOffset = nPos;
      m_iOffset -= m_iOffset % m_iItemsPerLine;
		  Invalidate();
		  break;
	  case SB_ENDSCROLL:		//End scroll.
	  default:
		  break;
	}

	SetScrollPos( SB_VERT, m_iOffset, TRUE );

	CWnd::OnVScroll( nSBCode, nPos, pScrollBar );

}




/*-OnPopupEditChange----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

LRESULT CEditListBox::OnPopupEditChange( WPARAM wParam, LPARAM lParam ) 
{

  if ( wParam == 1 )
  {
    // den Text übernehmen
    CString   cstrGnu;
    m_pPopupEdit->GetWindowText( cstrGnu );
    SetItemText( m_iFocusedItem, LPCTSTR( cstrGnu ) );

    CWnd *wndParent = GetParent();
    if ( wndParent )
    {
      wndParent->SendMessage( dwELBM, m_iFocusedItem, m_vectItems[m_iFocusedItem]->m_dwItemData );
    }
  }
  EndLabelEdit();

  return 0;

}

void CEditListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

  if ( m_bEditLabel )
  {
    //if ( m_pPopupEdit->OnKeyUp( nChar, nRepCnt, nFlags ) )
    {
      return;
    }
  }
	
	//CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}
