#include <windows.h>

#include "CListBox.h"



namespace WindowsWrapper
{

  CListBox::CListBox() :
    CWnd()
  {
  }



  CListBox::~CListBox()
  {
  }



  int CListBox::AddString( LPCSTR Text )
  {
    return (int)SendMessage( LB_ADDSTRING, 0, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );
  }



  int CListBox::AddString( const GR::String& Text )
  {
    return (int)SendMessage( LB_ADDSTRING, 0, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );
  }



  int CListBox::AddString( LPCSTR Text, DWORD_PTR dwItemData )
  {
    int iItem = (int)SendMessage( LB_ADDSTRING, 0, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );
    SetItemData( iItem, dwItemData );
    return iItem;
  }



  int CListBox::AddString( const GR::String& Text, DWORD_PTR dwItemData )
  {
    int iItem = (int)SendMessage( LB_ADDSTRING, 0, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );
    SetItemData( iItem, dwItemData );
    return iItem;
  }



  int CListBox::SetItemData( int nIndex, DWORD_PTR dwItemData )
  {
    return (int)SendMessage( LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)dwItemData );
  }



  int CListBox::SelectItem( int nSelect )
  {
    return (int)SendMessage( LB_SETCURSEL, nSelect, 0 );
  }



  int CListBox::GetCurSel() const
  {
    return ( int )::SendMessageW( m_hWnd, LB_GETCURSEL, 0, 0 );
  }



  DWORD_PTR CListBox::GetItemData( int nIndex ) const
  {
    return (DWORD_PTR)::SendMessageW( m_hWnd, LB_GETITEMDATA, (WPARAM)nIndex, 0 );
  }



  DWORD_PTR CListBox::GetCurSelItemData() const
  {
    int iItem = ( int )::SendMessageW( m_hWnd, LB_GETCURSEL, 0, 0 );
    if ( iItem == LB_ERR )
    {
      return 0;
    }
    return (DWORD_PTR)::SendMessageW( m_hWnd, LB_GETITEMDATA, (WPARAM)iItem, 0 );
  }



  void CListBox::ResetContent()
  {
    ::SendMessageW( m_hWnd, LB_RESETCONTENT, 0, 0 );
  }



  int CListBox::GetCount() const
  {
    return ( int )::SendMessageW( m_hWnd, LB_GETCOUNT, 0, 0 );
  }



  int CListBox::GetText( int nIndex, LPSTR lpszBuffer, int nMaxLength ) const
  {
    GR::String    result = GetText( nIndex );

    strcpy_s( lpszBuffer, nMaxLength, result.c_str() );

    return (int)result.length();
  }



  GR::String CListBox::GetText( int nIndex ) const
  {
    int   iLength = ( int )::SendMessageW( m_hWnd, LB_GETTEXTLEN, nIndex, 0 );

    GR::WChar* pDummy = new GR::WChar[iLength + 3];

    ::SendMessageW( m_hWnd, LB_GETTEXT, nIndex, (LPARAM)pDummy );

    GR::String    result = GR::Convert::ToUTF8( pDummy );
    
    delete[] pDummy;

    return result;
  }



  int CListBox::DeleteString( UINT nIndex )
  {
    return ( int )::SendMessageW( m_hWnd, LB_DELETESTRING, (WPARAM)nIndex, 0 );
  }



  int CListBox::InsertItem( int InsertAt, const GR::String& Text, DWORD_PTR ItemData ) 
  {
    int newIndex = SendMessage( LB_INSERTSTRING, (WPARAM)InsertAt, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );

    SetItemData( newIndex, ItemData );

    return newIndex;
  }



  void CListBox::SetItemText( int ItemIndex, const GR::String& Text )
  {
    int     selectedItem = GetCurSel();

    SendMessage( WM_SETREDRAW, FALSE );

    DWORD_PTR   itemData = GetItemData( ItemIndex );
    DeleteString( ItemIndex );
    InsertItem( ItemIndex, Text, itemData );

    if ( selectedItem != LB_ERR )
    {
      SelectItem( selectedItem );
    }

    SendMessage( WM_SETREDRAW, TRUE );
  }
}