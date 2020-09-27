#ifndef GR_UI_SLIDER_H
#define GR_UI_SLIDER_H

#include "CustomWnd.h"


class GRUISlider : public CCustomWnd
{

  protected:

    GR::i32                     m_iRange,
                                m_iSliderVisualPos,
                                m_iSliderSize,
                                m_iSliderPos;

    POINT                       m_ptDragOffset;

    bool                        m_bHorizontal,
                                m_bMouseOver,
                                m_bMouseOverSlider,
                                m_bCapturing;



  public:


    GRUISlider( bool bHorizontal = false ) :
      m_bHorizontal( bHorizontal ),
      m_bMouseOver( false ),
      m_bMouseOverSlider( false ),
      m_bCapturing( false ),
      m_iSliderVisualPos( 0 ),
      m_iSliderPos( 0 ),
      m_iRange( 100 )
    {
      m_iSliderSize = m_pDisplayClass->m_DisplayConstants.m_iSliderSize;
    }

    bool Create( DWORD dwStyle, bool bHorizontal, const RECT& rect, HWND hwndParent = NULL, UINT_PTR nID = 0, LPVOID lpParam = 0 )
    {
      m_bHorizontal = bHorizontal;
      return !!CCustomWnd::Create( "", dwStyle, rect, hwndParent, nID, lpParam );
    }

    bool GetSliderRect( RECT& rcSlider )
    {
      GetClientRect( &rcSlider );

      if ( m_bHorizontal )
      {
        rcSlider.left = m_iSliderVisualPos;
        rcSlider.right = m_iSliderVisualPos + m_iSliderSize;
      }
      else
      {
        rcSlider.top = m_iSliderVisualPos;
        rcSlider.bottom = m_iSliderVisualPos + m_iSliderSize;
      }
      return true;
    }


    virtual void Draw( HDC hdc, const RECT& rc )
    {

      RECT    rcSlider;

      GetSliderRect( rcSlider );

      m_pDisplayClass->PaintSlider( hdc, rc, rcSlider, m_bMouseOver, m_bMouseOverSlider, m_bCapturing, m_pWndBackgroundProducer == NULL );

    }

    virtual void DrawBackground( HDC hdc, const RECT& rcRedraw )
    {
    }

    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {

      switch ( uMsg )
      {
        case WM_KEYDOWN:
          switch ( wParam )
          {
            case VK_UP:
            case VK_LEFT:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_LINEUP );
              break;
            case VK_RIGHT:
            case VK_DOWN:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_LINEDOWN );
              break;
            case VK_PRIOR:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEUP );
              break;
            case VK_NEXT:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEDOWN );
              break;
            case VK_HOME:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_TOP );
              break;
            case VK_END:
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_BOTTOM );
              break;
          }
          break;
        case WM_GETDLGCODE:
          return DLGC_WANTALLKEYS;
        case WM_LBUTTONDOWN:
          if ( m_bMouseOver )
          {
            POINT   ptMouse;

            RECT    rcSlider;

            ptMouse.x = LOWORD( lParam ); 
            ptMouse.y = HIWORD( lParam );

            GetSliderRect( rcSlider );

            if ( PtInRect( &rcSlider, ptMouse ) )
            {
              if ( !m_bCapturing )
              {
                m_bCapturing = true;
                m_ptDragOffset.x = ptMouse.x - rcSlider.left;
                m_ptDragOffset.y = ptMouse.y - rcSlider.top;
                SetFocus();
                SetCapture();
                Invalidate();
              }
            }
            else
            {
              if ( m_bHorizontal )
              {
                if ( ptMouse.x < rcSlider.left )
                {
                  SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEUP );
                }
                else if ( ptMouse.x >= rcSlider.right )
                {
                  SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEDOWN );
                }
              }
              else
              {
                if ( ptMouse.y < rcSlider.top )
                {
                  SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEUP );
                }
                else if ( ptMouse.y >= rcSlider.bottom )
                {
                  SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_PAGEDOWN );
                }
              }
            }
          }
          break;
        case WM_LBUTTONUP:
          if ( m_bCapturing )
          {
            m_bCapturing = false;
            ReleaseCapture();
            Invalidate();
          }
          break;
        case WM_HSCROLL:
        case WM_VSCROLL:
          switch ( LOWORD( wParam ) )
          {
            case SB_TOP:
              if ( m_iSliderVisualPos > 0 )
              {
                m_iSliderVisualPos = 0;
                UpdateSliderPos();
              }
              break;
            case SB_LINEUP:
              if ( m_iSliderVisualPos > 0 )
              {
                --m_iSliderVisualPos;
                UpdateSliderPos();
              }
              break;
            case SB_PAGEUP:
              if ( m_iSliderVisualPos >= 20 )
              {
                m_iSliderVisualPos -= 20;
              }
              else
              {
                m_iSliderVisualPos = 0;
              }
              UpdateSliderPos();
              break;
            case SB_PAGEDOWN:
              {
                RECT    rc;

                GetClientRect( &rc );

                int   iBorder = m_bHorizontal ? rc.right : rc.bottom;

                if ( m_iSliderVisualPos >= iBorder - m_iSliderSize - 20 )
                {
                  m_iSliderVisualPos = iBorder - m_iSliderSize;
                }
                else
                {
                  m_iSliderVisualPos += 20;
                }
                UpdateSliderPos();
              }
              break;
            case SB_LINEDOWN:
              {
                RECT    rc;

                GetClientRect( &rc );

                int   iBorder = m_bHorizontal ? rc.right : rc.bottom;

                if ( m_iSliderVisualPos >= iBorder - m_iSliderSize - 1 )
                {
                  m_iSliderVisualPos = iBorder - m_iSliderSize;
                }
                else
                {
                  m_iSliderVisualPos++;
                }
                UpdateSliderPos();
              }
              break;
            case SB_BOTTOM:
              {
                RECT    rc;

                GetClientRect( &rc );

                int   iBorder = m_bHorizontal ? rc.right : rc.bottom;

                if ( m_iSliderVisualPos < iBorder - m_iSliderSize )
                {
                  m_iSliderVisualPos = iBorder - m_iSliderSize;
                  UpdateSliderPos();
                }
              }
              break;
            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
              // TODO m_iSliderVisualPos = 
              break;
          }
          break;
        case 0x020a://WM_MOUSEWHEEL:
          {
            int   iDelta = (short)HIWORD( wParam );

            iDelta /= 120; //WHEEL_DELTA;

            while ( iDelta > 0 )
            {
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_LINEUP, 0 );
              --iDelta;
            }
            while ( iDelta < 0 )
            {
              SendMessage( m_bHorizontal ? WM_HSCROLL : WM_VSCROLL, SB_LINEDOWN, 0 );
              ++iDelta;
            }
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

            if ( m_bCapturing )
            {
              if ( !m_bHorizontal )
              {
                int     iNewPos = pt.y - rc.top - m_ptDragOffset.y;

                if ( iNewPos < 0 )
                {
                  iNewPos = 0;
                }
                if ( iNewPos >= rc.bottom - m_iSliderSize )
                {
                  iNewPos = rc.bottom - m_iSliderSize;
                }
                if ( m_iSliderVisualPos != iNewPos )
                {
                  m_iSliderVisualPos = iNewPos;

                  UpdateSliderPos();
                  HWND    hwndParent = GetParent();
                  if ( hwndParent )
                  {
                    ::SendMessage( hwndParent, WM_VSCROLL, MAKEWPARAM( SB_THUMBPOSITION , m_iSliderPos ), (LPARAM)m_hWnd );
                  }
                }
              }
              else
              {
                int     iNewPos = pt.x - rc.left - m_ptDragOffset.x;

                if ( iNewPos < 0 )
                {
                  iNewPos = 0;
                }
                if ( iNewPos >= rc.right - m_iSliderSize )
                {
                  iNewPos = rc.right - m_iSliderSize;
                }
                if ( m_iSliderVisualPos != iNewPos )
                {
                  m_iSliderVisualPos = iNewPos;

                  UpdateSliderPos();
                  HWND    hwndParent = GetParent();
                  if ( hwndParent )
                  {
                    ::SendMessage( hwndParent, WM_HSCROLL, MAKEWPARAM( SB_THUMBPOSITION , m_iSliderPos ), (LPARAM)m_hWnd );
                  }
                }
              }
            }

            if ( PtInRect( &rc, pt ) )
            {
              if ( !m_bMouseOver )
              {
                m_bMouseOver = true;
                SetTimer( 17, 20 );
                Invalidate();
              }
            }
            GetSliderRect( rc );
            if ( PtInRect( &rc, pt ) )
            {
              if ( !m_bMouseOverSlider )
              {
                m_bMouseOverSlider = true;
                Invalidate();
              }
            }
            else
            {
              if ( m_bMouseOverSlider )
              {
                m_bMouseOverSlider = false;
                Invalidate();
              }
            }
          }
          break;
        case WM_TIMER:
          {
            POINT   pt;
            GetCursorPos( &pt );
            ScreenToClient( &pt );

            RECT    rc;

            GetSliderRect( rc );
            if ( !PtInRect( &rc, pt ) )
            {
              if ( m_bMouseOverSlider )
              {
                m_bMouseOverSlider = false;
              }
            }

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
            Invalidate();
          }
          break;
      }
      return CCustomWnd::WindowProc( uMsg, wParam, lParam );
    }

    void UpdateSliderPos()
    {
      RECT    rc;

      GetClientRect( &rc );

      if ( m_bHorizontal )
      {
        m_iSliderPos = m_iRange * m_iSliderVisualPos / ( rc.right - rc.left - m_iSliderSize );
      }
      else
      {
        m_iSliderPos = m_iRange * m_iSliderVisualPos / ( rc.bottom - rc.top - m_iSliderSize );
      }
      Invalidate();
    }

};


#endif // GR_UI_SLIDER_H