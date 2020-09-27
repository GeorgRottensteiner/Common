#ifndef GR_UI_CHECKBOX_H
#define GR_UI_CHECKBOX_H

#include "CustomWnd.h"



class GRUICheckBox : public CCustomWnd
{

  protected:

    bool              m_bMouseOver,
                      m_bPushed,
                      m_bCapturing,
                      m_bChecked;

    DWORD             m_dwTimer;


    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {

      switch ( uMsg )
      {
        case WM_GETDLGCODE:
          return DLGC_WANTALLKEYS;
        case WM_LBUTTONDOWN:
          if ( m_bMouseOver )
          {
            if ( !m_bPushed )
            {
              m_bPushed = true;
              m_bCapturing = true;
              SetFocus();
              SetCapture();
              Invalidate();
            }
          }
          break;
        case WM_LBUTTONUP:
          if ( m_bCapturing )
          {
            if ( m_bPushed )
            {
              HWND    hwndParent = GetParent();

              if ( hwndParent )
              {
                m_bChecked = !m_bChecked;
                ::SendMessage( hwndParent, WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)m_hWnd );
              }
            }
            m_bCapturing = false;
            ReleaseCapture();
            m_bPushed = false;
            Invalidate();
          }
          break;
        case WM_MOUSEMOVE:
          {
            POINT   pt;
            GetCursorPos( &pt );
            ScreenToClient( &pt );

            RECT    rc;

            GetWindowRect( &rc );
            ScreenToClient( &rc );

            if ( PtInRect( &rc, pt ) )
            {
              if ( ( !m_bPushed )
              &&   ( m_bCapturing ) )
              {
                m_bPushed = true;
                Invalidate();
              }
              if ( !m_bMouseOver )
              {
                m_bMouseOver = true;
                SetTimer( 17, 20 );
                Invalidate();
              }
            }
            else
            {
              if ( m_bPushed )
              {
                m_bPushed = false;
                Invalidate();
              }
            }
          }
          break;
        case WM_TIMER:
          if ( !m_bPushed )
          {
            POINT   pt;
            GetCursorPos( &pt );
            ScreenToClient( &pt );

            RECT    rc;

            GetWindowRect( &rc );
            ScreenToClient( &rc );
            if ( !PtInRect( &rc, pt ) )
            {
              if ( m_bMouseOver )
              {
                m_bMouseOver = false;
                KillTimer( 17 );
                Invalidate();
              }
            }
          }
          break;
        case WM_DESTROY:
          if ( m_bMouseOver )
          {
            KillTimer( 17 );
          }
          break;
        case WM_KILLFOCUS:
          if ( m_bCapturing )
          {
            m_bCapturing = false;
            ReleaseCapture();
            m_bPushed = false;
            Invalidate();
          }
          break;
      }
      return CCustomWnd::WindowProc( uMsg, wParam, lParam );
    }

  public:

    GRUICheckBox() :
      m_bMouseOver( false ),
      m_bPushed( false ),
      m_bCapturing( false ),
      m_bChecked( false )
    {
    }

    virtual void Draw( HDC hdc, const RECT& rc )
    {

      char    szText[MAX_PATH];

      GetWindowText( szText, MAX_PATH );

      m_pDisplayClass->PaintCheckBox( hdc, rc, szText, m_bMouseOver, m_bPushed, m_bChecked, m_pWndBackgroundProducer == NULL );

    }

};


#endif // GR_UI_CHECKBOX_H