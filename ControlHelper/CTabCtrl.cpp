#include <windows.h>

#include <ControlHelper\CTabCtrl.h>

#include <commctrl.h>



namespace WindowsWrapper
{

  CTabCtrl::CTabCtrl()
  {
  }



  CTabCtrl::~CTabCtrl()
  {
  }



  BOOL CTabCtrl::Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID )
  {
    return CWnd::Create( WC_TABCONTROLA, NULL, dwStyle, rect, hwndParent, nID );
  }



  BOOL CTabCtrl::InsertItem( int nItem, const GR::String& Item, LPARAM lItemData )
  {
    TCITEMW    tcItem;

    GR::WString   utf16Text = GR::Convert::ToUTF16( Item );

    tcItem.mask = TCIF_TEXT | TCIF_PARAM;
    tcItem.pszText = (LPWSTR)utf16Text.c_str();
    tcItem.lParam = lItemData;

    return (BOOL)SendMessage( TCM_INSERTITEM, nItem, (LPARAM)&tcItem );
  }



  int CTabCtrl::GetCurSel() const
  {
    return ( int )::SendMessageW( m_hWnd, TCM_GETCURSEL, 0, 0 );
  }



  void CTabCtrl::AdjustRect( BOOL bLarger, LPRECT lpRect )
  {
    ::SendMessageW( m_hWnd, TCM_ADJUSTRECT, (WPARAM)bLarger, (LPARAM)lpRect );
  }



  int CTabCtrl::GetItemCount() const
  {
    return ( int )::SendMessageW( m_hWnd, TCM_GETITEMCOUNT, 0, 0L );
  }



  int CTabCtrl::SetCurSel( int nItem )
  {

    return ( int )::SendMessageW( m_hWnd, TCM_SETCURSEL, nItem, 0L );

  }



  int CTabCtrl::HitTest( TCHITTESTINFO* pHitTestInfo ) const
  {

    return ( int )::SendMessageW( m_hWnd, TCM_HITTEST, 0, (LPARAM)pHitTestInfo );

  }



  BOOL CTabCtrl::GetItem( int nItem, TCITEM* pTabCtrlItem ) const
  {

    return ( BOOL )::SendMessageW( m_hWnd, TCM_GETITEM, nItem, (LPARAM)pTabCtrlItem );

  }


  BOOL CTabCtrl::GetItemRect( int nItem, LPRECT lpRect )
  {

    return ( BOOL )::SendMessageW( m_hWnd, TCM_GETITEMRECT, nItem, (LPARAM)lpRect );

  }



  LPARAM CTabCtrl::GetItemData( int nItem ) const
  {

    TCITEM    tcItem;

    memset( &tcItem, 0, sizeof( tcItem ) );
    tcItem.mask = TCIF_PARAM;

    if ( !GetItem( nItem, &tcItem ) )
    {
      return 0;
    }
    return tcItem.lParam;

  }



  BOOL CTabCtrl::SetItemText( int nItem, const GR::String& Text )
  {
    GR::WString     text = GR::Convert::ToUTF16( Text );

    GR::WChar*      pTemp = new GR::WChar[text.length() + 1];

    memcpy( pTemp, text.c_str(), text.length() + 1 );

    TCITEM    tcItem;

    tcItem.mask = TCIF_TEXT;
    tcItem.pszText = pTemp;

    BOOL Result = ( BOOL )::SendMessageW( m_hWnd, TCM_SETITEM, nItem, (LPARAM)&tcItem );

    delete[] pTemp;

    return Result;
  }

}