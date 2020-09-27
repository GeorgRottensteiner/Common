#include <windows.h>

#include "CStatusBar.h"



namespace WindowsWrapper
{

  CStatusBar::CStatusBar() :
    CWnd()
  {
  }



  CStatusBar::~CStatusBar()
  {
  }



  BOOL CStatusBar::Create( HWND hwndParent, DWORD dwStyle, UINT nID )
  {
    RECT    rc;

    memset( &rc, 0, sizeof( RECT ) );
    return CWnd::Create( STATUSCLASSNAMEA,
                         "",
                         dwStyle,
                         rc,
                         hwndParent,
                         nID );
  }



  BOOL CStatusBar::CreateEx( HWND hwndParent, DWORD dwControlStyles, DWORD dwStyle, UINT nID )
  {
    RECT    rc;

    memset( &rc, 0, sizeof( RECT ) );
    return CWnd::Create( STATUSCLASSNAMEA,
                         "",
                         dwStyle | dwControlStyles,
                         rc,
                         hwndParent,
                         nID );
  }



  int CStatusBar::AddPane( UINT nID, UINT nStyle, int cxWidth, const char* szText )
  {
    tPaneInfo     paneInfo;

    paneInfo.m_nID = nID;
    paneInfo.m_dwStyle = nStyle;
    paneInfo.m_iWidth = cxWidth;
    paneInfo.m_strPaneText = szText;

    m_vectPaneInfo.push_back( paneInfo );

    UpdatePanes();

    return (int)m_vectPaneInfo.size() - 1;
  }



  int CStatusBar::GetPaneCount() const
  {
    return (int)m_vectPaneInfo.size();
  }



  void CStatusBar::GetPaneInfo( int nIndex, UINT& nID, UINT& nStyle, int& cxWidth ) const
  {
  }



  void CStatusBar::UpdatePanes()
  {
    SendMessage( SB_SIMPLE, FALSE );

    LPINT     aWidth = new INT[m_vectPaneInfo.size()];

    for ( size_t i = 0; i < m_vectPaneInfo.size(); ++i )
    {
      aWidth[i] = m_vectPaneInfo[i].m_iWidth;
    }

    SendMessage( SB_SETPARTS, m_vectPaneInfo.size(), (LPARAM)aWidth );

    delete[] aWidth;

    for ( size_t i = 0; i < m_vectPaneInfo.size(); ++i )
    {
      SendMessage( SB_SETTEXT, i | m_vectPaneInfo[i].m_dwStyle, (LPARAM)GR::Convert::ToUTF16( m_vectPaneInfo[i].m_strPaneText ).c_str() );
    }
  }



  void CStatusBar::SetPaneText( int nIndex, LPCSTR lpszNewText, BOOL bUpdate )
  {
    if ( ( nIndex < 0 )
         || ( nIndex >= (int)m_vectPaneInfo.size() ) )
    {
      return;
    }
    m_vectPaneInfo[nIndex].m_strPaneText = lpszNewText;

    SendMessage( SB_SETTEXT,
                 nIndex | m_vectPaneInfo[nIndex].m_dwStyle,
                 (LPARAM)GR::Convert::ToUTF16( m_vectPaneInfo[nIndex].m_strPaneText ).c_str() );
  }



  void CStatusBar::SetPaneText( int nIndex, const GR::String& strText, BOOL bUpdate )
  {
    SetPaneText( nIndex, strText.c_str(), bUpdate );
  }



  void CStatusBar::GetItemRect( int nIndex, LPRECT lpRect ) const
  {
    if ( m_hWnd )
    {
      if ( !::SendMessageW( m_hWnd, SB_GETRECT, nIndex, (LPARAM)lpRect ) )
      {
        ::SetRectEmpty( lpRect );
      }
    }
  }

}