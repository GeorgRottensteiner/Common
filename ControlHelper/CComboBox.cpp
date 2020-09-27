#include <windows.h>
#include <commctrl.h>

#include <ControlHelper\CComboBox.h>



namespace WindowsWrapper
{

  CComboBox::CComboBox()
  {
  }



  CComboBox::~CComboBox()
  {
  }



  int CComboBox::AddString( LPCSTR lpszString )
  {
    return (int)SendMessage( CB_ADDSTRING, 0, (LPARAM)GR::Convert::ToUTF16( lpszString ).c_str() );
  }



  int CComboBox::AddString( const GR::String& strText )
  {
    return AddString( strText.c_str() );
  }



  int CComboBox::AddString( LPCSTR lpszString, DWORD dwItemData )
  {
    int   iItem = AddString( lpszString );
    if ( iItem != CB_ERR )
    {
      SendMessage( CB_SETITEMDATA, (WPARAM)iItem, (LPARAM)dwItemData );
    }
    return iItem;
  }




  int CComboBox::SetItemData( int nIndex, DWORD dwItemData )
  {
    return (int)SendMessage( CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)dwItemData );
  }



  int CComboBox::SelectItem( int nSelect )
  {
    return (int)SendMessage( CB_SETCURSEL, nSelect, 0 );
  }



  int CComboBox::GetCurSel() const
  {
    return ( int )::SendMessageW( m_hWnd, CB_GETCURSEL, 0, 0 );
  }



  DWORD_PTR CComboBox::GetCurSelItemData() const
  {
    int iItem = ( int )::SendMessageW( m_hWnd, CB_GETCURSEL, 0, 0 );
    if ( iItem == CB_ERR )
    {
      return 0;
    }
    return ::SendMessageW( m_hWnd, CB_GETITEMDATA, (WPARAM)iItem, 0 );
  }



  DWORD_PTR CComboBox::GetItemData( int nIndex ) const
  {
    return ::SendMessageW( m_hWnd, CB_GETITEMDATA, (WPARAM)nIndex, 0 );
  }



  LRESULT CComboBox::GetCount() const
  {
    return ::SendMessageW( m_hWnd, CB_GETCOUNT, 0, 0 );
  }



  void CComboBox::ResetContent()
  {
    ::SendMessageW( m_hWnd, CB_RESETCONTENT, 0, 0 );
  }



  LRESULT CComboBox::FindString( int nIndexStart, LPCTSTR lpszFind )
  {
    return ::SendMessageW( m_hWnd, CB_FINDSTRING, nIndexStart, (LPARAM)lpszFind );
  }



  LRESULT CComboBox::FindStringExact( int nIndexStart, LPCTSTR lpszFind )
  {
    return ::SendMessageW( m_hWnd, CB_FINDSTRINGEXACT, nIndexStart, (LPARAM)lpszFind );
  }



  GR::String CComboBox::GetLBText( int nIndex )
  {
    LRESULT   iLength = ::SendMessageW( m_hWnd, CB_GETLBTEXTLEN, nIndex, 0 );

    TCHAR* pDummy = new TCHAR[iLength + 1];

    ::SendMessageW( m_hWnd, CB_GETLBTEXT, nIndex, (LPARAM)pDummy );
    
    GR::String strString = GR::Convert::ToUTF8( pDummy );

    delete[] pDummy;

    return strString;
  }



  void CComboBox::SetCurSelByItemData( DWORD_PTR dwFindItemData )
  {
    LRESULT   iCount = GetCount();

    for ( int i = 0; i < iCount; ++i )
    {
      DWORD_PTR   dwItemData = GetItemData( i );
      if ( dwItemData == dwFindItemData )
      {
        SelectItem( i );
        return;
      }
    }
  }



  BOOL CComboBox::SetEditSel( int nStartChar, int nEndChar )
  {
    return ( BOOL )::SendMessageW( m_hWnd, CB_SETEDITSEL, 0, MAKELONG( nStartChar, nEndChar ) );
  }



  BOOL CComboBox::GetDroppedState()
  {
    return ( BOOL )::SendMessageW( m_hWnd, CB_GETDROPPEDSTATE, 0, 0L );
  }



  void CComboBox::ShowDropDown( BOOL bShowIt )
  {
    ::SendMessageW( m_hWnd, CB_SHOWDROPDOWN, bShowIt, 0 );
  }



  HWND CComboBox::GetEditHandle()
  {
    return FindWindowEx( m_hWnd, NULL, WC_EDIT, NULL );
  }

}