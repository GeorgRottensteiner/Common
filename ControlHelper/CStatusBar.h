#ifndef INCLUDE_CSTATUSBAR_H
#define INCLUDE_CSTATUSBAR_H


#include "CWnd.h"

#include <commctrl.h>

#include <vector>
#include <string>



namespace WindowsWrapper
{

  class CStatusBar : public CWnd
  {

    protected:

    struct tPaneInfo
    {
      GR::String      m_strPaneText;
      UINT            m_nID;
      UINT            m_dwStyle;
      int             m_iWidth;

      tPaneInfo() :
        m_nID( 0 ),
        m_dwStyle( 0 ),
        m_iWidth( 0 )
      {
      }
    };

    std::vector<tPaneInfo>      m_vectPaneInfo;

    public:


    CStatusBar();
    virtual ~CStatusBar();

    BOOL                Create( HWND hwndParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CCS_BOTTOM, UINT nID = 0 );
    BOOL                CreateEx( HWND hwndParent, DWORD dwControlStyles, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CCS_BOTTOM, UINT nID = 0 );

    int                 AddPane( UINT nID, UINT nStyle, int cxWidth, const char* szText = "" );
    void                GetPaneInfo( int nIndex, UINT& nID, UINT& nStyle, int& cxWidth ) const;
    int                 GetPaneCount() const;
    void                SetPaneText( int nIndex, LPCSTR lpszNewText, BOOL bUpdate = TRUE );
    void                SetPaneText( int nIndex, const GR::String& strText, BOOL bUpdate = TRUE );

    void                UpdatePanes();

    void                GetItemRect( int nIndex, LPRECT lpRect ) const;

  };

}

#endif // INCLUDE_CSTATUSBAR_H



