#pragma comment( lib, "comctl32.lib" )


#include <windows.h>
#include <commctrl.h>

#include "CListCtrl.h"



namespace WindowsWrapper
{

  CListCtrl::CListCtrl()
  {

    InitCommonControls();

  }



  CListCtrl::~CListCtrl()
  {
  }



  int CListCtrl::InsertColumn( int nCol, LPCSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem )
  {
    LVCOLUMNW      lvc;

    GR::WChar     szTemp[MAX_PATH];

    wsprintfW( szTemp, GR::Convert::ToUTF16( lpszColumnHeading ).c_str() );
    ZeroMemory( &lvc, sizeof( LVCOLUMN ) );

    lvc.mask = LVCF_TEXT | LVCF_FMT;
    lvc.pszText = szTemp;
    lvc.cchTextMax = (int)wcslen( szTemp );
    lvc.fmt = nFormat;

    if ( nWidth != -1 )
    {
      lvc.mask |= LVCF_WIDTH;
      lvc.cx = nWidth;
    }
    if ( nSubItem != -1 )
    {
      lvc.mask |= LVCF_SUBITEM;
      lvc.iSubItem = nSubItem;
    }
    return (int)SendMessageW( m_hWnd, LVM_INSERTCOLUMNW, nCol, (LPARAM)&lvc );
  }



  int CListCtrl::InsertItem( int iItem, const GR::String& strItem )
  {
    LVITEMW        lvItem;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

    GR::WString     utf16Text = GR::Convert::ToUTF16( strItem );

    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (LPWSTR)utf16Text.c_str();
    lvItem.cchTextMax = (int)utf16Text.length();
    lvItem.iItem = iItem;

    return (int)::SendMessageW( m_hWnd, LVM_INSERTITEMW, 0, (LPARAM)&lvItem );
  }



  int CListCtrl::InsertItem( int nItem, const GR::String& strItem, int nImage, GR::up dwItemData )
  {
    LVITEMW        lvItem;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

    GR::WString     utf16Text = GR::Convert::ToUTF16( strItem );

    lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvItem.pszText = (LPWSTR)utf16Text.c_str();
    lvItem.cchTextMax = (int)utf16Text.length();
    lvItem.iItem = nItem;
    lvItem.iImage = nImage;
    lvItem.lParam = dwItemData;

    return (int)SendMessageW( m_hWnd, LVM_INSERTITEMW, 0, (LPARAM)&lvItem );
  }



  int CListCtrl::AppendItem( const GR::String& strItem, int iImage, GR::up dwItemData )
  {
    return InsertItem( GetItemCount(), strItem, iImage, dwItemData );
  }



  BOOL CListCtrl::DeleteItem( int nItem )
  {
    return ( BOOL )::SendMessageW( m_hWnd, LVM_DELETEITEM, nItem, 0L );
  }



  BOOL CListCtrl::DeleteAllItems()
  {
    return ( BOOL )::SendMessageW( m_hWnd, LVM_DELETEALLITEMS, 0, 0 );
  }



  BOOL CListCtrl::GetItem( LVITEM* pItem ) const
  {

    return ( BOOL )::SendMessageW( m_hWnd, LVM_GETITEM, 0, (LPARAM)pItem );

  }



  BOOL CListCtrl::GetCheck( int nItem ) const
  {

    int nState = ( int )::SendMessageW( m_hWnd, LVM_GETITEMSTATE, (WPARAM)nItem, (LPARAM)LVIS_STATEIMAGEMASK );

    // Return zero if it's not checked, or nonzero otherwise.
    return ( (BOOL)( nState >> 12 ) - 1 );

  }



  int CListCtrl::GetItemImage( int nItem ) const
  {
    LVITEM    lvi;

    memset( &lvi, 0, sizeof( LVITEM ) );
    lvi.iItem = nItem;
    lvi.mask = LVIF_IMAGE;
    ::SendMessageW( m_hWnd, LVM_GETITEM, 0, (LPARAM)&lvi );

    return lvi.iImage;
  }



  DWORD_PTR CListCtrl::GetItemData( int nItem ) const
  {
    LVITEM    lvi;

    memset( &lvi, 0, sizeof( LVITEM ) );

    lvi.iItem = nItem;
    lvi.mask = LVIF_PARAM;
    ::SendMessageW( m_hWnd, LVM_GETITEM, 0, (LPARAM)&lvi );

    return (DWORD_PTR)lvi.lParam;
  }



  int CListCtrl::GetItemCount() const
  {
    return ( int )::SendMessageW( m_hWnd, LVM_GETITEMCOUNT, 0, 0 );
  }



  int CListCtrl::GetColumnCount() const
  {
    HWND    hwndHeader = ( HWND )::SendMessageW( m_hWnd, LVM_GETHEADER, 0, 0 );

    if ( hwndHeader == NULL )
    {
      return 0;
    }

    return ( int )::SendMessageW( hwndHeader, HDM_GETITEMCOUNT, 0, 0 );
  }



  BOOL CListCtrl::SetItemText( int nItem, int nSubItem, LPCSTR lpszText )
  {
    LVITEMW    lvItem;

    GR::WString     utf16Text = GR::Convert::ToUTF16( lpszText );

    lvItem.iItem = nItem;
    lvItem.iSubItem = nSubItem;
    lvItem.pszText = (LPWSTR)utf16Text.c_str();

    return ( BOOL )::SendMessageW( m_hWnd, LVM_SETITEMTEXTW, nItem, (LPARAM)&lvItem );
  }



  BOOL CListCtrl::SetItemText( int nItem, int nSubItem, const GR::String& Text )
  {
    GR::WString     text = GR::Convert::ToUTF16( Text );

    GR::WChar*      TempText = new GR::WChar[text.length() + 1];

    memcpy( TempText, text.c_str(), ( 1 + text.length() ) * sizeof( GR::WChar ) );


    LVITEMW    lvItem;

    lvItem.iItem = nItem;
    lvItem.iSubItem = nSubItem;
    lvItem.pszText = TempText;

    BOOL Result = ( BOOL )::SendMessageW( m_hWnd, LVM_SETITEMTEXTW, nItem, (LPARAM)&lvItem );

    delete[] TempText;

    return Result;
  }



  DWORD CListCtrl::SetExtendedStyle( DWORD dwNewStyle )
  {
    return (DWORD)SendMessage( LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwNewStyle );
  }



  int CListCtrl::HitTest( POINT& pt, UINT* pFlags ) const
  {
    RECT    rcHeader;

    if ( GetHeaderControlRect( rcHeader ) )
    {
      ScreenToClient( &rcHeader );
      if ( PtInRect( &rcHeader, pt ) )
      {
        // auf der Header-Control
        return -1;
      }
    }

    LVHITTESTINFO     hti;

    hti.pt = pt;

    int nRes = ( int )::SendMessageW( m_hWnd, LVM_HITTEST, 0, (LPARAM)&hti );

    if ( pFlags != NULL )
    {
      *pFlags = hti.flags;
    }
    return nRes;

  }



  BOOL CListCtrl::GetColumn( int nCol, LVCOLUMN* pColumn ) const
  {

    return ( BOOL )::SendMessageW( m_hWnd, LVM_GETCOLUMN, nCol, (LPARAM)pColumn );

  }



  GR::String CListCtrl::GetItemText( int nItem, int nSubItem ) const
  {
    LVITEMW      lvi;

    memset( &lvi, 0, sizeof( LVITEM ) );

    GR::WChar* pDummy = NULL;

    lvi.iSubItem = nSubItem;
    int nLen = 128;
    int nRes;
    do
    {
      nLen *= 2;
      lvi.cchTextMax = nLen;

      pDummy = new GR::WChar[nLen];
      lvi.pszText = pDummy;
      nRes = ( int )::SendMessageW( m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi );
    }
    while ( nRes == nLen - 1 );

    GR::String   strResult = GR::Convert::ToUTF8( pDummy );

    delete[] pDummy;

    return strResult;

  }



  int CListCtrl::GetColumnWidth( int nCol ) const
  {
    return ( int )::SendMessageW( m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0 );
  }



  int CListCtrl::GetItemPosition( int nItem, POINT* lpPoint ) const
  {
    return ( int )::SendMessageW( m_hWnd, LVM_GETITEMPOSITION, nItem, (LPARAM)lpPoint );
  }



  BOOL CListCtrl::GetItemRect( int nItem, RECT* lpRect, unsigned int nCode ) const
  {
    lpRect->left = nCode;
    return ( BOOL )::SendMessageW( m_hWnd, LVM_GETITEMRECT, (WPARAM)nItem, (LPARAM)lpRect );
  }



  BOOL CListCtrl::GetSubItemRect( int iItem, int iSubItem, RECT& rcTarget )
  {

    if ( ( iItem < 0 )
         || ( iItem >= GetItemCount() ) )
    {
      return FALSE;
    }

    if ( ( iSubItem < 0 )
         || ( iSubItem >= GetColumnCount() ) )
    {
      return FALSE;
    }

    RECT    rcItem;
    GetItemRect( iItem, &rcItem, LVIR_BOUNDS );

    POINT   ptItemPos;
    GetItemPosition( iItem, &ptItemPos );

    rcTarget.left = ptItemPos.x;
    rcTarget.top = ptItemPos.y;

    int   iOffset = 0;
    while ( iOffset < iSubItem )
    {
      rcTarget.left += GetColumnWidth( iOffset );
      iOffset++;
    }

    rcTarget.right = rcTarget.left + GetColumnWidth( iSubItem );
    rcTarget.bottom = rcTarget.top + rcItem.bottom - rcItem.top;

    return TRUE;

  }



  BOOL CListCtrl::CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent, UINT nID )
  {
    return CWnd::CreateEx( dwExStyle, WC_LISTVIEWA, "", dwStyle, rc, hwndParent, nID );
  }



  BOOL CListCtrl::SetItemData( int nItem, DWORD_PTR dwData )
  {

    LVITEM    lvItem;

    memset( &lvItem, 0, sizeof( lvItem ) );

    lvItem.iItem = nItem;
    lvItem.lParam = (LPARAM)dwData;
    lvItem.mask = LVIF_PARAM;

    return ( BOOL )::SendMessageW( m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvItem );

  }



  HIMAGELIST CListCtrl::SetImageList( HIMAGELIST hImageList, int nImageListType )
  {

    return ( HIMAGELIST )::SendMessageW( m_hWnd, LVM_SETIMAGELIST, nImageListType, (LPARAM)hImageList );

  }



  POSITION CListCtrl::GetFirstSelectedItemPosition() const
  {

    return (POSITION)(DWORD_PTR)( 1 + GetNextItem( -1, LVIS_SELECTED ) );

  }


  int CListCtrl::GetNextSelectedItem( POSITION& pos ) const
  {

    DWORD_PTR nOldPos = (DWORD_PTR)pos - 1;
    pos = (POSITION)(DWORD_PTR)( 1 + GetNextItem( (UINT)nOldPos, LVIS_SELECTED ) );
    return (UINT)nOldPos;

  }



  int CListCtrl::GetNextItem( int nItem, int nFlags ) const
  {

    return ( int )::SendMessageW( m_hWnd, LVM_GETNEXTITEM, nItem, MAKELPARAM( nFlags, 0 ) );

  }




  bool CListCtrl::GetHeaderControlRect( RECT& rcHeader ) const
  {

    if ( !( GetStyle() & LVS_REPORT ) )
    {
      SetRectEmpty( &rcHeader );
      return false;
    }

    HWND    hwndHeader = ( HWND )::SendMessageW( m_hWnd, LVM_GETHEADER, 0, 0 );

    if ( hwndHeader == NULL )
    {
      SetRectEmpty( &rcHeader );
      return false;
    }
    ::GetWindowRect( hwndHeader, &rcHeader );

    return true;

  }



  UINT CListCtrl::GetSelectedCount() const
  {

    return ( UINT )::SendMessageW( m_hWnd, LVM_GETSELECTEDCOUNT, 0, 0L );

  }



  BOOL CListCtrl::SetItemImage( int iItem, int nImage )
  {
    return SetItem( iItem, 0, LVIF_IMAGE, NULL, nImage, 0, 0, 0 );
  }



  BOOL CListCtrl::SetItem( int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem, int nImage, UINT nState, UINT nStateMask, LPARAM lParam )
  {
    LVITEM    lvi;

    lvi.mask = nMask;
    lvi.iItem = nItem;
    lvi.iSubItem = nSubItem;
    lvi.stateMask = nStateMask;
    lvi.state = nState;
    lvi.pszText = (LPTSTR)lpszItem;
    lvi.iImage = nImage;
    lvi.lParam = lParam;

    return ( BOOL )::SendMessageW( m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvi );
  }


}