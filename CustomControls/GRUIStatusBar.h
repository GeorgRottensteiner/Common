#ifndef GR_UI_STATUSBAR_H
#define GR_UI_STATUSBAR_H

#include "CustomWnd.h"

#include <list>


class GRUIStatusBar : public CCustomWnd
{

  protected:

    struct tPaneInfo
    {
      RECT            m_rcPane;
      DWORD           m_iAlignment;
      GR::String     m_strText;

      tPaneInfo() :
        m_iAlignment( DT_LEFT ),
        m_strText( "" )
      {
        SetRectEmpty( &m_rcPane );
      }
    };

    typedef std::list<tPaneInfo>    tListPanes;


    tListPanes                      m_listPanes;


  public:

    virtual void Draw( HDC hdc, const RECT& rc )
    {
      char    szText[MAX_PATH];
      GetWindowText( szText, MAX_PATH );

      RECT    rcTemp = rc;

      if ( !m_listPanes.empty() )
      {
        // TODO - Pane-Breiten einberechnen, Panes darstellen
      }

      m_pDisplayClass->FillSolidRect( hdc, &rcTemp, GetSysColor( COLOR_BTNFACE ) );

      int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

      HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

      ::DrawText( hdc, szText, (int)strlen( szText ), &rcTemp, DT_LEFT | DT_VCENTER | DT_SINGLELINE );

      SelectObject( hdc, hOldFont );

      SetBkMode( hdc, iOldBkMode );

      CCustomWnd::Draw( hdc, rc );
    }

    bool Create( const GR::String& strText, HWND hwndParent, bool bAlignToBottom = true )
    {
      RECT    rc;
      SetRectEmpty( &rc );
      if ( hwndParent )
      {
        ::GetWindowRect( hwndParent, &rc );

        rc.top = rc.bottom - 20;
        ScreenToClient( &rc );
      }
      if ( !CCustomWnd::Create( strText.c_str(), WS_VISIBLE | WS_CHILD, rc, hwndParent, 0 ) )
      {
        return false;
      }
      OnParentResized();
      return true;
    }

    virtual void OnParentRecalcClientSize( RECT& rcClient )
    {
      RECT    rc;

      GetClientRect( &rc );

      rcClient.bottom -= ( rc.bottom - rc.top );
    }

    virtual void OnParentResized()
    {
      HWND    hwndParent = GetParent();

      if ( hwndParent == NULL )
      {
        return;
      }

      RECT    rcParentClient;

      ::GetClientRect( hwndParent, &rcParentClient );

      //if ( bAlignToBottom )
      {
        RECT    rc = rcParentClient;

        rc.top = rc.bottom - 20;

        SetWindowPos( NULL, 0, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_SHOWWINDOW );
        Invalidate();
      }
    }

    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {

      switch ( uMsg )
      {
        case WM_SETTEXT:
          CCustomWnd::WindowProc( uMsg, wParam, lParam );
          Invalidate();
          return TRUE;
      }

      return CCustomWnd::WindowProc( uMsg, wParam, lParam );
    }

};


#endif // GR_UI_STATUSBAR_H