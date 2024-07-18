#include <windows.h>

#include "CEdit.h"



namespace WindowsWrapper
{

  CEdit::CEdit() :
    CWnd()
  {
  }



  CEdit::~CEdit()
  {
  }



  BOOL CEdit::Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID )
  {
    return CWnd::Create( "EDIT",
                         "",
                         dwStyle,
                         rect,
                         hwndParent,
                         nID );
  }



  BOOL CEdit::CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent, UINT nID )
  {
    return CWnd::CreateEx( dwExStyle, "EDIT", "", dwStyle, rc, hwndParent, nID );
  }



  void CEdit::SetLimitText( UINT nMax )
  {
    SendMessage( EM_SETLIMITTEXT, (WPARAM)nMax, 0 );
  }



  void CEdit::AddString( const GR::String& Text )
  {
    int iStartPos = 0,
      iEndPos = 0,
      iSize = 0,
      iNewTextLength = (int)Text.length();

    ::SendMessageW( m_hWnd, EM_SETSEL, -1, -1 );
    ::SendMessageW( m_hWnd, EM_GETSEL, (WPARAM)&iStartPos, (LPARAM)&iEndPos );
    iSize = iEndPos;
    while ( iSize + iNewTextLength > 30000 )
    {
      int iFirstLineLength = ( int )::SendMessageW( m_hWnd, EM_LINELENGTH, 0, 0 );
      ::SendMessageW( m_hWnd, EM_SETSEL, 0, iFirstLineLength + 1 );
      ::SendMessageW( m_hWnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)L"" );
      iSize -= iFirstLineLength + 1;
    }
    int iZeilen = ( int )::SendMessageW( m_hWnd, EM_GETLINECOUNT, 0, 0 );
    ::SendMessageW( m_hWnd, EM_SETSEL, -1, -1 );
    ::SendMessageW( m_hWnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)GR::Convert::ToUTF16( Text ).c_str() );
  }



  void CEdit::SetSel( int nStartChar, int nEndChar, BOOL bNoScroll )
  {
    SendMessage( EM_SETSEL, nStartChar, nEndChar );
    if ( !bNoScroll )
    {
      SendMessage( EM_SCROLLCARET, 0, 0 );
    }
  }



  BOOL CEdit::SetReadOnly( BOOL bReadOnly )
  {
    return (BOOL)SendMessage( EM_SETREADONLY, bReadOnly, 0 );
  }



  int CEdit::CharFromPos( POINT& pt ) const
  {
    return ( int )::SendMessageW( m_hWnd, EM_CHARFROMPOS, 0, MAKELPARAM( pt.x, pt.y ) );
  }



  LRESULT CEdit::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
  {
    if ( message == WM_CHAR )
    {
      if ( wParam == 1 )
      {
        SetSel( 0, -1 );
        return 1;
      }
    }
    return CWnd::WindowProc( message, wParam, lParam );
  }



}
