#include <windows.h>

#include <ControlHelper\CTreeCtrl.h>



namespace WindowsWrapper
{

  CTreeCtrl::CTreeCtrl()
  {
  }



  CTreeCtrl::~CTreeCtrl()
  {
  }



  BOOL CTreeCtrl::Attach( HWND hwndNew )
  {
    BOOL  bResult = CWnd::Attach( hwndNew );

    ::SendMessageW( m_hWnd, TVM_SETBKCOLOR, 0, GetSysColor( COLOR_WINDOW ) );

    return bResult;
  }



  HTREEITEM CTreeCtrl::InsertItem( LPCTSTR lpszItem, HTREEITEM hParent, HTREEITEM hInsertAfter )
  {

    TVINSERTSTRUCT    tvis;

    tvis.hParent = hParent;
    tvis.hInsertAfter = hInsertAfter;
    tvis.item.mask = TVIF_TEXT;
    tvis.item.pszText = (LPTSTR)lpszItem;

    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis );
  }



  HTREEITEM CTreeCtrl::InsertItem( GR::String& strItem, HTREEITEM hParent, HTREEITEM hInsertAfter )
  {
    GR::WString      uniCode = GR::Convert::ToUTF16( strItem );

    TVINSERTSTRUCTW    tvis;

    tvis.hParent      = hParent;
    tvis.hInsertAfter = hInsertAfter;
    tvis.item.mask    = TVIF_TEXT;
    tvis.item.pszText = (LPWSTR)uniCode.c_str();

    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_INSERTITEMW, 0, (LPARAM)&tvis );
  }



  HTREEITEM CTreeCtrl::InsertItemAndData( LPCTSTR lpszItem, DWORD dwData, HTREEITEM hParent, HTREEITEM hInsertAfter )
  {
    TVINSERTSTRUCT    tvis;

    tvis.hParent = hParent;
    tvis.hInsertAfter = hInsertAfter;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = (LPTSTR)lpszItem;
    tvis.item.lParam = dwData;

    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis );
  }



  HTREEITEM CTreeCtrl::InsertItemAndData( GR::String& strItem, DWORD dwData, HTREEITEM hParent, HTREEITEM hInsertAfter )
  {
    TVINSERTSTRUCTW     tvis;
    GR::WString        uniCode = GR::Convert::ToUTF16( strItem );

    tvis.hParent = hParent;
    tvis.hInsertAfter = hInsertAfter;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = (LPWSTR)uniCode.c_str();
    tvis.item.lParam = dwData;

    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_INSERTITEMW, 0, (LPARAM)&tvis );
  }



  HTREEITEM CTreeCtrl::GetSelectedItem()
  {
    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0 );
  }



  HTREEITEM CTreeCtrl::GetParentItem( HTREEITEM hItem )
  {
    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem );
  }



  int CTreeCtrl::GetItemLevel( HTREEITEM hItem )
  {
    int   iLevel = 0;

    while ( hItem != NULL )
    {
      hItem = ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem );
      iLevel++;
    }
    return iLevel - 1;
  }



  DWORD_PTR CTreeCtrl::GetItemData( HTREEITEM hItem )
  {
    TVITEM    tvItem;

    tvItem.mask = TVIF_PARAM | TVIF_HANDLE;
    tvItem.hItem = hItem;
    ::SendMessageW( m_hWnd, TVM_GETITEM, 0, (LPARAM)&tvItem );
    return tvItem.lParam;
  }



  HIMAGELIST CTreeCtrl::SetImageList( HIMAGELIST hImageList, int nImageListType )
  {
    return ( HIMAGELIST )::SendMessageW( m_hWnd, TVM_SETIMAGELIST, (WPARAM)nImageListType, (LPARAM)hImageList );
  }



  BOOL CTreeCtrl::SetItemImage( HTREEITEM hItem, int nImage, int nSelectedImage )
  {
    TVITEM    tvItem;

    tvItem.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvItem.hItem = hItem;
    tvItem.iImage = nImage;
    tvItem.iSelectedImage = nSelectedImage;
    return ( BOOL )::SendMessageW( m_hWnd, TVM_SETITEM, 0, (LPARAM)&tvItem );
  }



  BOOL CTreeCtrl::DeleteItem( HTREEITEM hItem )
  {
    return ( BOOL )::SendMessageW( m_hWnd, TVM_DELETEITEM, 0, (LPARAM)hItem );
  }



  BOOL CTreeCtrl::DeleteAllItems()
  {
    return ( BOOL )::SendMessageW( m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT );
  }



  BOOL CTreeCtrl::GetItemRect( HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly )
  {
    return (BOOL)SendMessage( TVM_GETITEMRECT, (WPARAM)bTextOnly, (LPARAM)lpRect );
  }



  HTREEITEM CTreeCtrl::HitTest( POINT& pt, UINT* pFlags ) const
  {
    TVHITTESTINFO hti;

    hti.pt = pt;
    HTREEITEM h = ( HTREEITEM )::SendMessageW( m_hWnd, TVM_HITTEST, 0, (LPARAM)&hti );
    if ( pFlags != NULL )
    {
      *pFlags = hti.flags;
    }
    return h;
  }



  UINT CTreeCtrl::GetCount() const
  {
    return ( UINT )::SendMessageW( m_hWnd, TVM_GETCOUNT, 0, 0 );
  }



  BOOL CTreeCtrl::ItemHasChildren( HTREEITEM hItem ) const
  {
    TVITEM item;
    item.hItem = hItem;
    item.mask = TVIF_CHILDREN;
    ::SendMessageW( m_hWnd, TVM_GETITEM, 0, (LPARAM)&item );
    return item.cChildren;
  }



  HTREEITEM CTreeCtrl::GetRootItem() const
  {
    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0 );
  }



  HTREEITEM CTreeCtrl::GetChildItem( HTREEITEM hItem )
  {
    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem );
  }



  HTREEITEM CTreeCtrl::GetNextSiblingItem( HTREEITEM hItem )
  {
    return ( HTREEITEM )::SendMessageW( m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem );
  }



  HTREEITEM CTreeCtrl::GetNextItem( HTREEITEM hItem )
  {
    HTREEITEM hti = NULL;

    if ( ItemHasChildren( hItem ) )
    {
      hti = GetChildItem( hItem );
    }

    if ( hti == NULL )
    {
      while ( ( hti = GetNextSiblingItem( hItem ) ) == NULL )
      {
        if ( ( hItem = GetParentItem( hItem ) ) == NULL )
        {
          return NULL;
        }
      }
    }
    return hti;
  }



  BOOL CTreeCtrl::SetItemText( HTREEITEM hItem, const GR::String& strText )
  {
    GR::WString          text = GR::Convert::ToUTF16( strText );

    TVITEMW item;
    item.hItem    = hItem;
    item.mask     = TVIF_TEXT;
    item.pszText  = (LPWSTR)text.c_str();
    return ( BOOL )::SendMessageW( m_hWnd, TVM_SETITEMW, 0, (LPARAM)&item );
  }



  GR::String CTreeCtrl::GetItemText( HTREEITEM hItem ) const
  {
    TVITEMW item;
    item.hItem = hItem;
    item.mask  = TVIF_TEXT;

    GR::WChar*       pData = NULL;

    int nLen = 128;
    int nRes;
    do
    {
      if ( pData )
      {
        delete[] pData;
        pData = NULL;
      }

      nLen *= 2;
      pData = new GR::WChar[nLen];
      item.pszText = pData;
      item.cchTextMax = nLen;
      ::SendMessageW( m_hWnd, TVM_GETITEMW, 0, (LPARAM)&item );
      nRes = lstrlenW( item.pszText );
    }
    while ( nRes == nLen - 1 );

    GR::WString      resultText = pData;

    GR::String       result = GR::Convert::ToUTF8( resultText );

    delete[] pData;
    return result;
  }



  BOOL CTreeCtrl::Expand( HTREEITEM hItem, UINT nCode )
  {
    return (BOOL)SendMessage( TVM_EXPAND, (WPARAM)nCode, (LPARAM)hItem );
  }



  BOOL CTreeCtrl::SetItem( HTREEITEM hItem, UINT nMask, LPCTSTR lpszItem, int nImage,
                           int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam )
  {
    TVITEM item;
    item.hItem = hItem;
    item.mask = nMask;
    item.pszText = (LPTSTR)lpszItem;
    item.iImage = nImage;
    item.iSelectedImage = nSelectedImage;
    item.state = nState;
    item.stateMask = nStateMask;
    item.lParam = lParam;
    return ( BOOL )::SendMessageW( m_hWnd, TVM_SETITEM, 0, (LPARAM)&item );
  }



  BOOL CTreeCtrl::SetItemData( HTREEITEM hItem, DWORD_PTR dwData )
  {
    return SetItem( hItem, TVIF_PARAM, NULL, 0, 0, 0, 0, (LPARAM)dwData );
  }



  BOOL CTreeCtrl::CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent, UINT nID )
  {
    InitCommonControls();
    return CWnd::CreateEx( dwExStyle, WC_TREEVIEWA, "", dwStyle, rc, hwndParent, nID );
  }



  HTREEITEM CTreeCtrl::FindItem( const GR::String& strText, HTREEITEM hItemSearchRoot )
  {
    while ( hItemSearchRoot )
    {
      if ( GetItemText( hItemSearchRoot ) == strText )
      {
        return hItemSearchRoot;
      }
      hItemSearchRoot = GetNextItem( hItemSearchRoot );
    }
    return NULL;
  }



  BOOL CTreeCtrl::SelectItem( HTREEITEM hItem )
  {
    return ( BOOL )::SendMessageW( m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem );
  }

}