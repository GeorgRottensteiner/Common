#define OEMRESOURCE
#include <windows.h>

#include "CCheckListBox.h"



namespace WindowsWrapper
{

  CCheckListBox::CCheckListBox() : 
    m_ItemWidth( -1 ),
    m_ItemHeight( 14 ),
    m_Offset( 0 ),
    m_SelectedItem( -1 ),
    m_FocusedItem( -1 ),
    m_HasFocus( false ),
    m_ItemsPerLine( 1 ),
    m_OwnerDraw( false ),
    m_ID( 0 )
  {

    m_hbmCheckBox = LoadBitmap( NULL, MAKEINTRESOURCE( OBM_CHECKBOXES ) );
    m_hFont = CreateFontW( 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"MS Sans Serif" );
  }



  CCheckListBox::~CCheckListBox()
  {
    for ( size_t i = 0; i < m_Items.size(); i++ )
    {
      delete m_Items[i];
    }
    m_Items.clear();
    DeleteObject( m_hFont );
    DeleteObject( m_hbmCheckBox );
  }



  void CCheckListBox::OnDestroy()
  {
    for ( size_t i = 0; i < m_Items.size(); i++ )
    {
      delete m_Items[i];
    }
    m_Items.clear();
  }



  BOOL CCheckListBox::Attach( HWND hwndNew )
  {
    BOOL  bResult = CWnd::Attach( hwndNew );

    SetFont( m_hFont );

    return bResult;
  }



  void CCheckListBox::SetOwnerDraw( BOOL bOD )
  {
    m_OwnerDraw = !!bOD;
    Invalidate();
  }



  void CCheckListBox::SetItemSize( int iWidth, int iHeight )
  {
    if ( iWidth != 0 )
    {
      m_ItemWidth = iWidth;
    }
    if ( iHeight > 0 )
    {
      m_ItemHeight = iHeight;
    }

    if ( m_ItemWidth == -1 )
    {
      m_ItemsPerLine = 1;
    }
    else
    {
      RECT    rc;
      GetClientRect( &rc );
      m_ItemsPerLine = rc.right / m_ItemWidth;

      // eines MUSS dargestellt werden
      if ( m_ItemsPerLine == 0 )
      {
        m_ItemsPerLine = 1;
      }
    }
    Invalidate();
  }



  int CCheckListBox::AddString( const char *szItem, DWORD_PTR dwItemData, bool bCheckAble )
  {
    CCLItem *pItem = new CCLItem();
    pItem->m_Item       = szItem;
    pItem->m_ItemData   = dwItemData;
    pItem->m_CheckAble  = bCheckAble;
    m_Items.push_back( pItem );

    AdjustScrollBars();
    Invalidate();

    return (int)m_Items.size() - 1;
  }



  int CCheckListBox::AddString( const GR::String& strItem, DWORD_PTR dwItemData, bool bCheckAble )
  {
    return AddString( strItem.c_str(), dwItemData, bCheckAble );
  }



  int CCheckListBox::DeleteString( UINT nIndex )
  {
    if ( ( nIndex < 0 )
         || ( nIndex >= m_Items.size() ) )
    {
      return LB_ERR;
    }

    DWORD   dwDummy = 0;
    std::vector<CCLItem*>::iterator   it( m_Items.begin() );

    while ( it != m_Items.end() )
    {
      if ( dwDummy == nIndex )
      {
        delete *it;
        m_Items.erase( it );

        if ( m_SelectedItem >= (int)m_Items.size() )
        {
          m_SelectedItem = -1;
        }
        if ( m_FocusedItem >= (int)m_Items.size() )
        {
          m_FocusedItem = -1;
        }
        Invalidate();
        return (int)m_Items.size();
      }

      dwDummy++;
      it++;
    }

    return (int)m_Items.size();
  }



  int CCheckListBox::GetText( size_t iIndex, LPSTR lpszBuffer ) const
  {
    if ( iIndex >= m_Items.size() )
    {
      return LB_ERR;
    }

    wsprintfA( lpszBuffer, m_Items[iIndex]->m_Item.c_str() );

    return (int)m_Items[iIndex]->m_Item.length();
  }



  int CCheckListBox::GetText( size_t iIndex, GR::String& strResult ) const
  {
    if ( iIndex >= m_Items.size() )
    {
      return LB_ERR;
    }

    strResult = m_Items[iIndex]->m_Item;

    return (int)strResult.length();
  }



  int CCheckListBox::SetItemData( size_t iIndex, DWORD dwData )
  {
    if ( iIndex >= m_Items.size() )
    {
      return LB_ERR;
    }
    m_Items[iIndex]->m_ItemData = dwData;

    return 0;
  }



  DWORD_PTR CCheckListBox::GetItemData( size_t iIndex ) const
  {
    if ( iIndex >= m_Items.size() )
    {
      return LB_ERR;
    }
    return m_Items[iIndex]->m_ItemData;
  }



  size_t CCheckListBox::GetCount() const
  {
    return m_Items.size();
  }



  void CCheckListBox::OnPaint()
  {
    PAINTSTRUCT   ps;

    HDC   dc = BeginPaint( m_hWnd, &ps );


    HFONT hOldFont = (HFONT)SelectObject( dc, m_hFont );

    DRAWITEMSTRUCT    dis;

    dis.CtlID = m_ID;
    dis.CtlType = ODT_LISTBOX;
    dis.hDC = dc;
    dis.hwndItem = GetSafeHwnd();

    RECT    rcClient,
      rc;

    GetClientRect( &rcClient );

    for ( size_t i = m_Offset; i < m_Items.size(); i++ )
    {
      if ( GetItemRect( i, &rc ) != LB_ERR )
      {
        if ( rc.top >= rcClient.bottom )
        {
          // unten raus
          break;
        }
        if ( rc.bottom >= rcClient.bottom )
        {
          RECT    rc2;

          rc2 = rc;
          rc2.bottom = rcClient.bottom - 1;

          HRGN  hrgn = CreateRectRgnIndirect( &rc2 );
          SelectClipRgn( dc, hrgn );
        }
        if ( m_OwnerDraw )
        {
          dis.itemAction = ODA_DRAWENTIRE;
          dis.itemData = m_Items[i]->m_ItemData;
          dis.itemID = (UINT)i;
          dis.itemState = 0;
          dis.rcItem = rc;
          if ( i == m_SelectedItem )
          {
            dis.itemState |= ODS_SELECTED;
          }
          if ( i == m_FocusedItem )
          {
            dis.itemState |= ODS_FOCUS;
          }

          ::SendMessageW( ::GetParent( m_hWnd ), WM_DRAWITEM, m_ID, (LPARAM)&dis );
          continue;
        }

        if ( i == m_SelectedItem )
        {
          ::SetBkColor( dc, GetSysColor( COLOR_HIGHLIGHT ) );
          ::ExtTextOut( dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
          SetTextColor( dc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        }
        else
        {
          ::SetBkColor( dc, GetSysColor( COLOR_WINDOW ) );
          ::ExtTextOut( dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
          SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
        }
        if ( i < m_Items.size() )
        {
          // Checkbox malen
          if ( m_Items[i]->m_CheckAble )
          {
            HDC   hdcBitmap = CreateCompatibleDC( dc );
            HBITMAP hbmOld = (HBITMAP)SelectObject( hdcBitmap, m_hbmCheckBox );

            int   iXOffset = 0;
            if ( m_Items[i]->m_Checked )
            {
              iXOffset = 13;
            }
            BitBlt( dc,
                    rc.left + 1,
                    rc.top + 1,//( rc.bottom - rc.top - 12 ) / 2,
                    12,
                    12,
                    hdcBitmap,
                    iXOffset,
                    0,
                    SRCCOPY );

            SelectObject( hdcBitmap, hbmOld );
            DeleteDC( hdcBitmap );
          }

          rc.left += 16;
          ::DrawTextW( dc,
                       GR::Convert::ToUTF16( m_Items[i]->m_Item ).c_str(),
                       (int)m_Items[i]->m_Item.length(),
                       &rc,
                       DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX );
          rc.left -= 16;
        }
        if ( ( m_HasFocus )
        &&   ( m_FocusedItem == i ) )
        {
          DrawFocusRect( dc, &rc );
        }
      }
    }

    SelectObject( dc, hOldFont );

    EndPaint( m_hWnd, &ps );

  }



  void CCheckListBox::OnLButtonDown( UINT nFlags, POINT pt )
  {

    DoLButtonDown( pt );

    SetFocus();

  }

  void CCheckListBox::OnLButtonDblClk( UINT nFlags, POINT pt )
  {

    DoLButtonDown( pt );

    SetFocus();

  }



  void CCheckListBox::DoLButtonDown( POINT& pt )
  {

    BOOL    bOutside;

    int iItem = ItemFromPoint( pt, bOutside );

    if ( bOutside )
    {
      return;
    }

    bool    bSendMessage = false;
    if ( ( pt.x < 13 )
    &&   ( iItem != -1 )
    &&   ( m_Items[iItem]->m_CheckAble ) )
    {
      // die Checkbox erwischt
      SetCheck( iItem, 1 - GetCheck( iItem ) );
      bSendMessage = true;
    }
    if ( m_SelectedItem != iItem )
    {
      if ( iItem != -1 )
      {
        m_FocusedItem = iItem;
      }
      m_SelectedItem = iItem;
      Invalidate();

      bSendMessage = true;
    }
    if ( ( bSendMessage )
         && ( ::GetParent( m_hWnd ) ) )
    {
      ::SendMessageW( ::GetParent( m_hWnd ), WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), LBN_SELCHANGE ), (LPARAM)m_hWnd );
    }

  }



  UINT CCheckListBox::ItemFromPoint( POINT pt, BOOL& bOutside ) const
  {
    RECT rcClient;


    GetClientRect( &rcClient );

    bOutside = !PtInRect( &rcClient, pt );
    if ( bOutside )
    {
      return -1;
    }

    // 2 Fälle, A) normale Listbox, B) mehrspaltige Listbox
    size_t   iResult = -1;
    if ( m_ItemWidth == -1 )
    {
      iResult = m_Offset + ( pt.y / m_ItemHeight );
      if ( iResult >= (int)m_Items.size() )
      {
        iResult = -1;
      }
      return (UINT)iResult;
    }

    int   iX = pt.x / m_ItemWidth,
      iY = pt.y / m_ItemHeight;

    if ( iX >= m_ItemsPerLine )
    {
      // in dem Restbereich zwischen Items und rechtem Rand
      return -1;
    }
    iResult = m_Offset + iY * m_ItemsPerLine + iX;

    // da ist kein Item mehr
    if ( iResult >= (int)m_Items.size() )
    {
      iResult = -1;
    }

    return (UINT)iResult;

  }



  UINT CCheckListBox::ItemFromPoint( POINT pt ) const
  {

    BOOL    bOutSide;

    return ItemFromPoint( pt, bOutSide );

  }



  int CCheckListBox::GetItemRect( size_t nIndex, LPRECT lpRect ) const
  {

    memset( lpRect, 0, sizeof( RECT ) );

    if ( nIndex >= (int)m_Items.size() )
    {
      return LB_ERR;
    }

    RECT    rcClient;


    GetClientRect( &rcClient );

    if ( ( rcClient.right == 0 )
         || ( rcClient.bottom == 0 ) )
    {
      return LB_ERR;
    }


    // 2 Fälle, A) normale Listbox, B) mehrspaltige Listbox
    if ( m_ItemWidth == -1 )
    {
      lpRect->left = 0;
      lpRect->right = rcClient.right;
      lpRect->top = (LONG)( nIndex - m_Offset ) * m_ItemHeight;
      lpRect->bottom = lpRect->top + m_ItemHeight;
      return 0;
    }

    lpRect->left = (LONG)( nIndex % m_ItemsPerLine ) * m_ItemWidth;
    lpRect->top = (LONG)( ( nIndex - m_Offset ) / m_ItemsPerLine ) * m_ItemHeight;
    lpRect->right = lpRect->left + m_ItemWidth;
    lpRect->bottom = lpRect->top + m_ItemHeight;

    return 0;

  }



  UINT CCheckListBox::OnGetDlgCode()
  {

    return DLGC_WANTALLKEYS;

  }



  BOOL CCheckListBox::OnEraseBkgnd( HDC hdc )
  {

    RECT    rc;
    GetClientRect( &rc );

    ::SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
    ::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );

    return TRUE;

  }



  void CCheckListBox::ScrollIntoView()
  {

    if ( m_SelectedItem == -1 )
    {
      // witzisch
      return;
    }
    while ( m_SelectedItem < m_Offset )
    {
      m_Offset -= m_ItemsPerLine;
    }
    RECT    rc;
    GetClientRect( &rc );
    int   iVisibleItems = rc.bottom / m_ItemHeight * m_ItemsPerLine;
    while ( m_SelectedItem > m_Offset + iVisibleItems )
    {
      m_Offset += m_ItemsPerLine;
    }
    Invalidate();

  }



  void CCheckListBox::SetCurSel( size_t iItem )
  {
    if ( iItem >= m_Items.size() )
    {
      iItem = -1;
      if ( m_SelectedItem != -1 )
      {
        m_SelectedItem = iItem;
        m_FocusedItem = iItem;
        Invalidate();
      }
      return;
    }

    if ( m_FocusedItem != iItem )
    {
      m_FocusedItem = iItem;
      m_SelectedItem = m_FocusedItem;
      ScrollIntoView();
      Invalidate();
    }
  }



  size_t CCheckListBox::GetCurSel() const
  {

    if ( m_SelectedItem == -1 )
    {
      return LB_ERR;
    }
    return m_SelectedItem;

  }



  void CCheckListBox::OnKillFocus( HWND hwndNewWindow )
  {
    m_HasFocus = false;
    if ( m_FocusedItem != -1 )
    {
      Invalidate();
    }
  }



  void CCheckListBox::OnSetFocus( HWND hwndOldWindow )
  {
    m_HasFocus = true;
    if ( m_FocusedItem != -1 )
    {
      Invalidate();
    }
  }



  void CCheckListBox::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
  {

    if ( m_FocusedItem != -1 )
    {
      if ( nChar == VK_LEFT )
      {
        size_t   iItem = m_FocusedItem - 1;
        if ( iItem < 0 )
        {
          iItem = 0;
        }
        SetCurSel( iItem );
      }
      else if ( nChar == VK_RIGHT )
      {
        size_t   iItem = m_FocusedItem + 1;
        if ( iItem >= m_Items.size() )
        {
          iItem = m_FocusedItem;
        }
        SetCurSel( iItem );
      }
      if ( m_ItemWidth != -1 )
      {
        // Up/Down ist unterschiedlich!
        RECT    rc;
        GetClientRect( &rc );
        if ( nChar == VK_UP )
        {
          if ( (int)m_FocusedItem >= rc.right / m_ItemWidth )
          {
            SetCurSel( m_FocusedItem - ( rc.right / m_ItemWidth ) );
          }
          else
          {
            SetCurSel( 0 );
          }
        }
        else if ( nChar == VK_DOWN )
        {
          size_t   iItem = m_FocusedItem + rc.right / m_ItemWidth;
          if ( iItem >= m_Items.size() )
          {
            SetCurSel( m_Items.size() - 1 );
          }
          else
          {
            SetCurSel( iItem );
          }
        }
      }
    }

  }



  void CCheckListBox::AdjustScrollBars()
  {

    RECT    rc;

    GetClientRect( &rc );

    size_t   iVisibleItems = rc.bottom / m_ItemHeight * m_ItemsPerLine;


    if ( iVisibleItems >= m_Items.size() )
    {
      // gibt nichts zu scrollen
      SetScrollRange( SB_VERT, 0, 0 );
      EnableScrollBar( SB_VERT, ESB_DISABLE_BOTH );
      ShowScrollBar( SB_VERT, FALSE );
      return;
    }

    EnableScrollBar( SB_VERT, ESB_ENABLE_BOTH );

    ShowScrollBar( SB_VERT );
    SetScrollRange( SB_VERT, 0, (int)m_Items.size() - (int)iVisibleItems );

  }



  void CCheckListBox::OnVScroll( UINT nSBCode, UINT nPos, HWND hwndScrollBar )
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

    int   iVisibleItems = rc.bottom / m_ItemHeight * m_ItemsPerLine,
      iPageItems = ( iVisibleItems * 3 / 4 ) - ( iVisibleItems * 3 / 4 ) % m_ItemsPerLine;

    bool bChanged = false;

    switch ( nSBCode )
    {
      case SB_LEFT:
        if ( m_Offset > 0 )
        {
          m_Offset = 0;
          bChanged = true;
        }
        break;
      case SB_LINELEFT:
        if ( m_Offset > 0 )
        {
          m_Offset -= m_ItemsPerLine;
          if ( m_Offset < 0 )
          {
            m_Offset = 0;
          }
          bChanged = true;
        }
        break;
      case SB_PAGELEFT:
        if ( m_Offset > 0 )
        {
          if ( iPageItems > (int)m_Offset )
          {
            m_Offset = 0;
          }
          else
          {
            m_Offset -= iPageItems;
          }
          bChanged = true;
        }
        break;
      case SB_LINERIGHT:
        {
          size_t iNewOffset = m_Offset;

          iNewOffset += m_ItemsPerLine;
          if ( iNewOffset >= m_Items.size() - iVisibleItems )
          {
            iNewOffset = m_Items.size() - iVisibleItems;
            iNewOffset -= iNewOffset % m_ItemsPerLine;
          }
          if ( iNewOffset != m_Offset )
          {
            m_Offset = iNewOffset;
            bChanged = true;
          }
        }
        break;
      case SB_PAGERIGHT:
        {
          size_t iNewOffset = m_Offset;

          iNewOffset += iPageItems;
          if ( iNewOffset >= m_Items.size() - iVisibleItems )
          {
            iNewOffset = m_Items.size() - iVisibleItems;
            iNewOffset -= iNewOffset % m_ItemsPerLine;
          }
          if ( iNewOffset != m_Offset )
          {
            m_Offset = iNewOffset;
            bChanged = true;
          }
        }
        break;
      case SB_RIGHT:
        {
          size_t iNewOffset = m_Offset;

          iNewOffset += iPageItems;

          iNewOffset = m_Items.size() - iVisibleItems;
          iNewOffset -= iNewOffset % m_ItemsPerLine;

          if ( iNewOffset != m_Offset )
          {
            m_Offset = iNewOffset;
            bChanged = true;
          }
        }
        break;
      case SB_THUMBPOSITION:
      case SB_THUMBTRACK:
        {
          size_t iNewOffset = m_Offset;

          iNewOffset = nPos;
          iNewOffset -= iNewOffset % m_ItemsPerLine;
          if ( iNewOffset < 0 )
          {
            iNewOffset = 0;
          }
          if ( iNewOffset != m_Offset )
          {
            m_Offset = iNewOffset;
            bChanged = true;
          }
        }
        break;
      case SB_ENDSCROLL:		//End scroll.
      default:
        break;
    }

    if ( bChanged )
    {
      Invalidate();
    }

    SetScrollPos( SB_VERT, (int)m_Offset, TRUE );

  }



  void CCheckListBox::SetCheck( size_t nIndex, int nCheck )
  {
    if ( nIndex >= (int)m_Items.size() )
    {
      return;
    }
    if ( nCheck )
    {
      m_Items[nIndex]->m_Checked = true;
    }
    else
    {
      m_Items[nIndex]->m_Checked = false;
    }
    RECT    rc;
    GetItemRect( nIndex, &rc );

    InvalidateRect( &rc, FALSE );
  }



  int CCheckListBox::GetCheck( size_t nIndex )
  {
    if ( nIndex >= (int)m_Items.size() )
    {
      return 0;
    }
    if ( m_Items[nIndex]->m_Checked )
    {
      return 1;
    }
    return 0;
  }



  HWND CCheckListBox::Detach()
  {
    for ( size_t i = 0; i < m_Items.size(); i++ )
    {
      delete m_Items[i];
    }
    m_Items.clear();

    return CWnd::Detach();
  }



  BOOL CCheckListBox::Create( DWORD dwStyle, RECT& rc, HWND hwndParent, int iID )
  {

    return CWnd::Create( "LISTBOX", "", LBS_OWNERDRAWFIXED | dwStyle, rc, hwndParent, iID );

  }



  BOOL CCheckListBox::CreateEx( DWORD dwExStyle, DWORD dwStyle, RECT& rc, HWND hwndParent, int iID )
  {

    return CWnd::CreateEx( dwExStyle, "LISTBOX", "", LBS_OWNERDRAWFIXED | dwStyle, rc, hwndParent, iID );

  }



  void CCheckListBox::ResetContent()
  {

    for ( size_t i = 0; i < m_Items.size(); i++ )
    {
      delete m_Items[i];
    }
    m_Items.clear();
    Invalidate();

  }



  LRESULT CCheckListBox::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    if ( uMsg == WM_DESTROY )
    {
      OnDestroy();
    }
    else if ( uMsg == WM_PAINT )
    {
      OnPaint();
      return TRUE;
    }
    else if ( uMsg == WM_ERASEBKGND )
    {
      return OnEraseBkgnd( (HDC)wParam );
    }
    else if ( uMsg == WM_LBUTTONDOWN )
    {
      POINT   pt;

      pt.x = LOWORD( lParam );
      pt.y = HIWORD( lParam );
      OnLButtonDown( (UINT)wParam, pt );
    }
    else if ( uMsg == WM_LBUTTONDBLCLK )
    {
      POINT   pt;

      pt.x = LOWORD( lParam );
      pt.y = HIWORD( lParam );
      OnLButtonDblClk( (UINT)wParam, pt );
    }
    else if ( uMsg == WM_GETDLGCODE )
    {
      return OnGetDlgCode();
    }
    else if ( uMsg == WM_SETFOCUS )
    {
      OnSetFocus( (HWND)wParam );
    }
    else if ( uMsg == WM_KILLFOCUS )
    {
      OnKillFocus( (HWND)wParam );
    }
    else if ( uMsg == WM_KEYDOWN )
    {
      OnKeyDown( (UINT)wParam, LOWORD( lParam ), HIWORD( lParam ) );
    }
    else if ( uMsg == WM_VSCROLL )
    {
      OnVScroll( (int)LOWORD( wParam ), (short int)HIWORD( wParam ), (HWND)lParam );
    }

    return CWnd::WindowProc( uMsg, wParam, lParam );

  }

}