#include ".\customwnd.h"

#include "GRUIMenu.h"



CClassicDisplayClass*  CCustomWnd::m_pDisplayClass = NULL;

CClassicDisplayClass   CCustomWnd::m_BasicDisplayClass;



CCustomWnd::CCustomWnd() :
  m_pWndBackgroundProducer( NULL ),
  m_hbmDoubleBuffer( NULL ),
  m_dwWantedStyles( 0 ),
  m_dwWantedExtendedStyle( 0 ),
  m_bActive( false ),
  m_bDraggingNC( false ),
  m_dwDraggingHT( 0 ),
  m_bCaptionCloseButtonDown( false ),
  m_bCaptionMaxButtonDown( false ),
  m_bCaptionMinButtonDown( false ),
  m_HDCNCArea( NULL )
{

  if ( m_pDisplayClass == NULL )
  {
    m_pDisplayClass = &m_BasicDisplayClass;
  }
  _RegisterClass();

}

CCustomWnd::~CCustomWnd()
{

  if ( m_hbmDoubleBuffer != NULL )
  {
    DeleteObject( m_hbmDoubleBuffer );
    m_hbmDoubleBuffer = NULL;
  }

}


BOOL CCustomWnd::Create( LPCTSTR lpszWindowName, 
                         DWORD dwStyle, 
                         const RECT& rect, 
                         HWND hwndParent, 
                         UINT_PTR nID, 
                         LPVOID lpParam )
{

  m_dwWantedStyles        = dwStyle;
  m_dwWantedExtendedStyle = 0;

  DWORD     dwTrueStyle = 0;

  dwTrueStyle |= ( dwStyle & WS_SYSMENU );
  dwTrueStyle |= ( dwStyle & WS_POPUP );
  dwTrueStyle |= ( dwStyle & WS_VISIBLE );
  dwTrueStyle |= ( dwStyle & WS_CHILD );
  dwTrueStyle |= ( dwStyle & WS_MINIMIZEBOX );
  dwTrueStyle |= ( dwStyle & WS_MAXIMIZEBOX );

  return CWnd::Create( "GR_CUSTOMWNDCLASS",
                       lpszWindowName,
                       dwTrueStyle,
                       rect,
                       hwndParent,
                       nID,
                       lpParam );

}



BOOL CCustomWnd::CreateEx( DWORD dwExtendedStyle,
                           LPCTSTR lpszWindowName, 
                           DWORD dwStyle, 
                           const RECT& rect, 
                           HWND hwndParent, 
                           UINT_PTR nID, 
                           LPVOID lpParam )
{

  m_dwWantedStyles        = dwStyle;
  m_dwWantedExtendedStyle = dwExtendedStyle;

  DWORD     dwTrueStyle = 0;

  dwTrueStyle |= ( dwStyle & WS_SYSMENU );
  dwTrueStyle |= ( dwStyle & WS_POPUP );
  dwTrueStyle |= ( dwStyle & WS_VISIBLE );
  dwTrueStyle |= ( dwStyle & WS_CHILD );
  dwTrueStyle |= ( dwStyle & WS_MINIMIZEBOX );
  dwTrueStyle |= ( dwStyle & WS_MAXIMIZEBOX );

  DWORD     dwTrueExStyle = 0;

  //dwTrueExStyle |= ( dwExtendedStyle & WS_EX_TOOLWINDOW );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_TOPMOST );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_ACCEPTFILES );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_APPWINDOW );

  return CWnd::CreateEx( dwTrueExStyle,
                         "GR_CUSTOMWNDCLASS",
                         lpszWindowName,
                         dwTrueStyle,
                         rect,
                         hwndParent,
                         nID,
                         lpParam );

}



BOOL CCustomWnd::CreateEx( DWORD dwExtendedStyle,
                           const char* szClassName,
                           LPCTSTR lpszWindowName, 
                           DWORD dwStyle, 
                           const RECT& rect, 
                           HWND hwndParent, 
                           UINT_PTR nID, 
                           LPVOID lpParam )
{

  m_dwWantedStyles        = dwStyle;
  m_dwWantedExtendedStyle = dwExtendedStyle;

  DWORD     dwTrueStyle = 0;

  dwTrueStyle |= ( dwStyle & WS_SYSMENU );
  dwTrueStyle |= ( dwStyle & WS_POPUP );
  dwTrueStyle |= ( dwStyle & WS_VISIBLE );
  dwTrueStyle |= ( dwStyle & WS_CHILD );
  dwTrueStyle |= ( dwStyle & WS_MINIMIZEBOX );
  dwTrueStyle |= ( dwStyle & WS_MAXIMIZEBOX );

  DWORD     dwTrueExStyle = 0;

  //dwTrueExStyle |= ( dwExtendedStyle & WS_EX_TOOLWINDOW );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_TOPMOST );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_ACCEPTFILES );
  dwTrueExStyle |= ( dwExtendedStyle & WS_EX_APPWINDOW );

  return CWnd::CreateEx( dwTrueExStyle,
                         szClassName,
                         lpszWindowName,
                         dwTrueStyle,
                         rect,
                         hwndParent,
                         nID,
                         lpParam );

}



/*-_RegisterClass-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CCustomWnd::_RegisterClass()
{
  
  WNDCLASS    wc;

  wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc    = MYHELPERWINDOWPROC;
  wc.cbClsExtra     = 0;
  wc.cbWndExtra     = 0;
  wc.hInstance      = (HINSTANCE)GetModuleHandle( NULL );
  wc.hIcon          = NULL;
  wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground  = (HBRUSH)( COLOR_BTNFACE + 1 );
  wc.lpszMenuName   = NULL;
  wc.lpszClassName  = "GR_CUSTOMWNDCLASS";
  if ( !RegisterClass( &wc ) )
  {
    //dh::Log( "CSplitterWnd::_RegisterClass failed (%d)\n", GetLastError() );
  }

}



LRESULT CCustomWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_CAPTURECHANGED:
      if ( ( (HWND)lParam != GetSafeHwnd() )
      &&   ( m_bDraggingNC ) )
      {
        m_bDraggingNC = false;

        if ( m_bCaptionCloseButtonDown )
        {
          m_bCaptionCloseButtonDown = false;
          PostMessage( WM_CLOSE );
        }
        if ( m_bCaptionMinButtonDown )
        {
          m_bCaptionMinButtonDown = false;
          if ( IsIconic() )
          {
            ShowWindow( SW_RESTORE );
          }
          else
          {
            ShowWindow( SW_MINIMIZE );
          }
        }
        if ( m_bCaptionMaxButtonDown )
        {
          m_bCaptionMaxButtonDown = false;
          if ( IsZoomed() )
          {
            ShowWindow( SW_RESTORE );
          }
          else
          {
            ShowWindow( SW_MAXIMIZE );
          }
        }
      }
      break;
    case WM_SETCURSOR:
      {
        POINT   pt;

        GetCursorPos( &pt );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        DWORD   dwHitTest = NCHitTest( pt );

        switch ( dwHitTest )
        {
          case HTTOPLEFT:
          case HTBOTTOMRIGHT:
            SetCursor( LoadCursor( NULL, IDC_SIZENWSE ) );
            return TRUE;
          case HTBOTTOMLEFT:
          case HTTOPRIGHT:
            SetCursor( LoadCursor( NULL, IDC_SIZENESW ) );
            return TRUE;
          case HTLEFT:
          case HTRIGHT:
            SetCursor( LoadCursor( NULL, IDC_SIZEWE ) );
            return TRUE;
          case HTTOP:
          case HTBOTTOM:
            SetCursor( LoadCursor( NULL, IDC_SIZENS ) );
            return TRUE;
          case HTBORDER:
            SetCursor( LoadCursor( NULL, IDC_ARROW ) );
            return TRUE;
        }
      }
      SetCursor( LoadCursor( NULL, IDC_ARROW ) );
      return TRUE;
    case WM_LBUTTONDOWN:
      SetActiveWindow();
      BringWindowToTop();
      break;
    case WM_NCRBUTTONDOWN:
      {
        SetActiveWindow();
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        DWORD   dwHitTest = NCHitTest( pt );

        if ( ( dwHitTest == HTCAPTION )
        ||   ( dwHitTest == HTSYSMENU ) )
        {
          CGRUIMenu      menuSystem;

          CreateSysMenu( menuSystem );

          pt.x += rcWindow.left;
          pt.y += rcWindow.top;

          DWORD   dwResult = menuSystem.TrackPopupMenu( TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL );
          return 0;
        }
      }
      break;
    case WM_NCLBUTTONDBLCLK:
      {
        SetActiveWindow();
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        DWORD   dwHitTest = NCHitTest( pt );

        if ( dwHitTest == HTCAPTION )
        {
          if ( IsZoomed() )
          {
            ShowWindow( SW_RESTORE );
          }
          else if ( m_dwWantedStyles & WS_MAXIMIZEBOX )
          {
            ShowWindow( SW_MAXIMIZE );
          }
        }
      }
      return 0;
    case WM_NCLBUTTONDOWN:
      {
        SetActiveWindow();
        BringWindowToTop();

        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        DWORD   dwHitTest = NCHitTest( pt );

        if ( dwHitTest == HTSYSMENU )
        {
          CGRUIMenu      menuSystem;

          CreateSysMenu( menuSystem );

          pt.x += rcWindow.left;
          pt.y += rcWindow.top;

          DWORD   dwResult = menuSystem.TrackPopupMenu( TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL );
          return 0;
        }

        if ( dwHitTest == HTCLOSE )
        {
          m_bCaptionCloseButtonDown = true;
        }
        if ( dwHitTest == HTMAXBUTTON )
        {
          m_bCaptionMaxButtonDown = true;
        }
        if ( dwHitTest == HTMINBUTTON )
        {
          m_bCaptionMinButtonDown = true;
        }

        switch ( dwHitTest )
        {
          case HTCLOSE:
          case HTMAXBUTTON:
          case HTMINBUTTON:
          case HTLEFT:
          case HTRIGHT:
          case HTTOP:
          case HTTOPLEFT:
          case HTTOPRIGHT:
          case HTBOTTOM:
          case HTBOTTOMLEFT:
          case HTBOTTOMRIGHT:
          case HTCAPTION:
            if ( !m_bDraggingNC )
            {
              m_ptDragOffset = pt;
              m_bDraggingNC = true;
              m_dwDraggingHT = dwHitTest;
              SetCapture();

              if ( ( dwHitTest == HTCLOSE )
              ||   ( dwHitTest == HTMINBUTTON )
              ||   ( dwHitTest == HTMAXBUTTON ) )
              {
                SendMessage( WM_NCPAINT, 1, 0 );
              }
            }
            break;
        }
      }
      return 0;
    case WM_NCHITTEST:
      {
        POINT   pt;

        pt.x = LOWORD( lParam );
        pt.y = HIWORD( lParam );

        RECT    rcWindow;
        GetWindowRect( &rcWindow );
        pt.x -= rcWindow.left;
        pt.y -= rcWindow.top;

        return NCHitTest( pt );
      }
      break;
    case WM_LBUTTONUP:
      if ( m_bDraggingNC )
      {
        if ( GetCapture() == GetSafeHwnd() )
        {
          ReleaseCapture();
        }
      }
      break;
    case WM_MOUSEMOVE:
      if ( m_bDraggingNC )
      {
        POINT   ptMouse;

        ptMouse.x = (int)(short)LOWORD( lParam );
        ptMouse.y = (int)(short)HIWORD( lParam );

        if ( ( m_dwDraggingHT == HTCLOSE )
        ||   ( m_dwDraggingHT == HTMINBUTTON )
        ||   ( m_dwDraggingHT == HTMAXBUTTON ) )
        {
          POINT   ptLocal = ptMouse;

          ClientToScreen( &ptLocal );

          RECT    rcWindow;

          GetWindowRect( &rcWindow );

          ptLocal.x -= rcWindow.left;
          ptLocal.y -= rcWindow.top;

          RECT    rc;

          if ( m_dwDraggingHT == HTCLOSE )
          {
            if ( ( GetCaptionCloseButtonRect( rc ) )
            &&   ( PtInRect( &rc, ptLocal ) ) )
            {
              if ( !m_bCaptionCloseButtonDown )
              {
                m_bCaptionCloseButtonDown = true;
                SendMessage( WM_NCPAINT, 1, 0 );
              }
            }
            else if ( m_bCaptionCloseButtonDown )
            {
              m_bCaptionCloseButtonDown = false;
              SendMessage( WM_NCPAINT, 1, 0 );
            }
          }
          if ( m_dwDraggingHT == HTMAXBUTTON )
          {
            if ( ( GetCaptionMaximizeButtonRect( rc ) )
            &&   ( PtInRect( &rc, ptLocal ) ) )
            {
              if ( !m_bCaptionMaxButtonDown )
              {
                m_bCaptionMaxButtonDown = true;
                SendMessage( WM_NCPAINT, 1, 0 );
              }
            }
            else if ( m_bCaptionMaxButtonDown )
            {
              m_bCaptionMaxButtonDown = false;
              SendMessage( WM_NCPAINT, 1, 0 );
            }
          }
          if ( m_dwDraggingHT == HTMINBUTTON )
          {
            if ( ( GetCaptionMinimizeButtonRect( rc ) )
            &&   ( PtInRect( &rc, ptLocal ) ) )
            {
              if ( !m_bCaptionMinButtonDown )
              {
                m_bCaptionMinButtonDown = true;
                SendMessage( WM_NCPAINT, 1, 0 );
              }
            }
            else if ( m_bCaptionMinButtonDown )
            {
              m_bCaptionMinButtonDown = false;
              SendMessage( WM_NCPAINT, 1, 0 );
            }
          }
          break;
        }
        else if ( m_dwDraggingHT == HTCAPTION )
        {
          if ( GetParent() )
          {
            RECT    rcParent;

            ::GetWindowRect( GetParent(), &rcParent );

            ptMouse.x -= rcParent.left;
            ptMouse.y -= rcParent.top;
          }


          ClientToScreen( &ptMouse );

          RECT      rcNew;
          GetWindowRect( &rcNew );

          if ( ( ptMouse.x - m_ptDragOffset.x - rcNew.left )
          ||   ( ptMouse.y - m_ptDragOffset.y - rcNew.top ) )
          {
            OffsetRect( &rcNew, ptMouse.x - m_ptDragOffset.x - rcNew.left, ptMouse.y - m_ptDragOffset.y - rcNew.top );

            SetWindowPos( NULL, rcNew.left, rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, SWP_NOZORDER );
          }
          break;
        }

        ClientToScreen( &ptMouse );

        RECT      rcOld;
        GetWindowRect( &rcOld );

        RECT      rcNew = rcOld;

        if ( ( m_dwDraggingHT == HTLEFT )
        ||   ( m_dwDraggingHT == HTTOPLEFT )
        ||   ( m_dwDraggingHT == HTBOTTOMLEFT ) )
        {
          rcNew.left = ptMouse.x;
        }
        if ( ( m_dwDraggingHT == HTTOP )
        ||   ( m_dwDraggingHT == HTTOPLEFT )
        ||   ( m_dwDraggingHT == HTTOPRIGHT ) )
        {
          rcNew.top = ptMouse.y;
        }
        if ( ( m_dwDraggingHT == HTRIGHT )
        ||   ( m_dwDraggingHT == HTTOPRIGHT )
        ||   ( m_dwDraggingHT == HTBOTTOMRIGHT ) )
        {
          rcNew.right = ptMouse.x;
        }
        if ( ( m_dwDraggingHT == HTBOTTOM )
        ||   ( m_dwDraggingHT == HTBOTTOMLEFT )
        ||   ( m_dwDraggingHT == HTBOTTOMRIGHT ) )
        {
          rcNew.bottom = ptMouse.y;
        }

        if ( GetParent() )
        {
          RECT    rcParent;

          ::GetClientRect( GetParent(), &rcParent );

          POINT   ptParentOffset;

          ptParentOffset.x = rcParent.left;
          ptParentOffset.y = rcParent.top;

          ::ClientToScreen( GetParent(), &ptParentOffset );

          OffsetRect( &rcNew, -ptParentOffset.x, -ptParentOffset.y );
        }

        SetWindowPos( NULL, rcNew.left, rcNew.top, rcNew.right - rcNew.left, rcNew.bottom - rcNew.top, SWP_NOZORDER );
      }
      break;
    case WM_MOUSEACTIVATE:
      return MA_ACTIVATE;
    case WM_ACTIVATE:
      if ( LOWORD( wParam ) == WA_INACTIVE )
      {
        m_bActive = false;
        if ( m_bDraggingNC )
        {
          ReleaseCapture();
          m_bDraggingNC = false;
        }
      }
      else
      {
        m_bActive = true;
      }
      SendMessage( WM_NCPAINT, 1, 0 );
      //DrawCaption( m_bActive );
      break;
    case WM_NCPAINT:
      {
        m_bActive = ( GetActiveWindow() == GetSafeHwnd() );

        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC   hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );

        DrawCaption( hdc, m_bActive );

        EndNCPaint();
      }
      return 0;
    case WM_NCACTIVATE:
      m_bActive = !!wParam;
      if ( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION )
      {
        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        DrawCaption( hdc, m_bActive );

        EndNCPaint();
      }
      return TRUE;
      //return 0;
      break;
    case WM_NCCALCSIZE:
      {
        if ( wParam )
        {
          NCCALCSIZE_PARAMS*    pNCParams = (NCCALCSIZE_PARAMS*)lParam;

          CalculateClientRectFromWindowRect( pNCParams->rgrc[0] );
        }
        else
        {
          RECT*    pRect = (RECT*)lParam;

          CalculateClientRectFromWindowRect( *pRect );
        }
        return 0;
      }
      break;
    case WM_DESTROY:
      if ( m_hbmDoubleBuffer )
      {
        DeleteObject( m_hbmDoubleBuffer );
        m_hbmDoubleBuffer = NULL;
      }
      break;
    case WM_SIZE:
      {
        SIZE    szNew;

        szNew.cx = LOWORD( lParam );
        szNew.cy = HIWORD( lParam );

        RECT    rcWindow;

        GetWindowRect( &rcWindow );

        szNew.cx = rcWindow.right - rcWindow.left;
        szNew.cy = rcWindow.bottom - rcWindow.top;

        if ( ( szNew.cx )
        &&   ( szNew.cy ) )
        {
          if ( m_hbmDoubleBuffer )
          {
            DeleteObject( m_hbmDoubleBuffer );
            m_hbmDoubleBuffer = NULL;
          }

          HDC   hdcOrig = ::GetDC( GetDesktopWindow() );

          m_hbmDoubleBuffer = CreateCompatibleBitmap( hdcOrig, szNew.cx, szNew.cy );

          BITMAP    bmInfo;
          GetObject( m_hbmDoubleBuffer, sizeof( BITMAP ), &bmInfo );

          ::ReleaseDC( GetDesktopWindow(), hdcOrig );

          SendMessage( WM_NCPAINT, 1, 0 );

          // Childs mitteilen
          HWND    hwndChild = GetWindow( GW_CHILD );
          while ( hwndChild )
          {
            CCustomWnd*  pWndChild = (CCustomWnd*)FromHandle( hwndChild );
            if ( pWndChild )
            {
              pWndChild->OnParentResized();
            }
            hwndChild = ::GetNextWindow( hwndChild, GW_HWNDNEXT );
          }
        }
      }
      break;
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
      Invalidate();
      break;
    case WM_ERASEBKGND:
      return TRUE;
    case WM_PAINT:
      {
        m_hdcPaint = StartPaint();
          //BeginPaint( m_hWnd, &ps );

        // TODO - Clip Childs/Siblings?
        /*
        HRGN    hrgnTemp = CreateRectRgn( 0, 0, 0, 0 );

        int   iRgn = GetClipRgn( m_hdcPaint, hrgnTemp );

        dh::Log( "RegionType %d", iRgn );

        DeleteObject( hrgnTemp );
        */

        HDC       hdcDoubleBuffer = CreateCompatibleDC( m_hdcPaint );

        HBITMAP   hbmOld = (HBITMAP)SelectObject( hdcDoubleBuffer, m_hbmDoubleBuffer );

        if ( m_pWndBackgroundProducer )
        {
          RECT    rcBack;

          GetWindowRect( &rcBack );

          CCustomWnd*   pProducer = m_pWndBackgroundProducer;
          while ( pProducer->m_pWndBackgroundProducer )
          {
            pProducer = pProducer->m_pWndBackgroundProducer;
          }

          RECT    rcProducer;

          pProducer->GetWindowRect( &rcProducer );

          int     iDX = ( rcBack.left - rcProducer.left );
          int     iDY = ( rcBack.top - rcProducer.top );

          OffsetWindowOrgEx( (HDC)wParam, iDX, iDY, 0 );

          pProducer->ScreenToClient( &rcBack );

          if ( rcBack.top < 0 )
          {
            rcBack.top = 0;
          }
          if ( rcBack.left < 0 )
          {
            rcBack.left = 0;
          }

          OffsetViewportOrgEx( hdcDoubleBuffer, -iDX, -iDY, 0 );
          pProducer->DrawBackground( hdcDoubleBuffer, rcBack );
          OffsetViewportOrgEx( hdcDoubleBuffer, iDX, iDY, 0 );

          OffsetWindowOrgEx( (HDC)wParam, -iDX, -iDY, 0 );
        }
        else
        {
          RECT    rcBack;

          /*
          GetWindowRect( &rcBack );
          ScreenToClient( &rcBack );
          */
          GetClientRect( &rcBack );

          DrawBackground( hdcDoubleBuffer, rcBack );
        }
        RECT          rc;

        GetClientRect( &rc );

        Draw( hdcDoubleBuffer, rc );
        StopPaint();

        SelectObject( hdcDoubleBuffer, hbmOld );

        DeleteDC( hdcDoubleBuffer );
      }
      return 0;
  }

  return CWnd::WindowProc( uMsg, wParam, lParam );

}



void CCustomWnd::DrawBackground( HDC hdc, const RECT& rcRedraw )
{
  if ( m_pDisplayClass )
  {
    m_pDisplayClass->DrawBackground( hdc, rcRedraw );
  }
  else
  {
    FillRect( hdc, &rcRedraw, GetSysColorBrush( COLOR_APPWORKSPACE ) );
  }
}

void CCustomWnd::Draw( HDC hdc, const RECT& rc )
{

  BitBlt( m_hdcPaint, 0, 0, rc.right - rc.left, rc.bottom - rc.top, 
          hdc, 0, 0, SRCCOPY );

}


void CCustomWnd::SetBackgroundProducer( CCustomWnd* pWndProducer )
{

  m_pWndBackgroundProducer = pWndProducer;
  Invalidate();

}



bool CCustomWnd::GetCaptionRect( RECT& rc )
{

  if ( !( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION ) )
  {
    return false;
  }

  GetWindowRect( &rc );
  OffsetRect( &rc, -rc.left, -rc.top );

  rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.right -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iSmallCaptionHeight;
  }
  else
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
  }

  return true;

}



bool CCustomWnd::GetCaptionCloseButtonRect( RECT& rc )
{

  if ( !( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION ) )
  {
    return false;
  }

  if ( ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  &&   ( !( m_dwWantedStyles & WS_SYSMENU ) ) )
  {
    return false;
  }

  GetWindowRect( &rc );
  OffsetRect( &rc, -rc.left, -rc.top );

  rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.right -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iSmallCaptionHeight;
  }
  else
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
  }

  rc.top += 2;
  rc.bottom -= 2;
  rc.right -= 2;
  rc.left = rc.right - ( rc.bottom - rc.top ) - 2;

  return true;

}



bool CCustomWnd::GetCaptionMaximizeButtonRect( RECT& rc )
{

  if ( !( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION ) )
  {
    return false;
  }
  if ( ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  &&   ( !( m_dwWantedStyles & WS_SYSMENU ) ) )
  {
    return false;
  }
  if ( !( m_dwWantedStyles & WS_MAXIMIZEBOX ) )
  {
    return false;
  }

  GetWindowRect( &rc );
  OffsetRect( &rc, -rc.left, -rc.top );

  rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.right -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iSmallCaptionHeight;
  }
  else
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
  }

  rc.top += 2;
  rc.bottom -= 2;
  rc.right -= 2 + ( rc.bottom - rc.top ) + 2 + 2;
  rc.left = rc.right - ( rc.bottom - rc.top ) - 2;

  return true;

}



bool CCustomWnd::GetCaptionMinimizeButtonRect( RECT& rc )
{

  if ( !( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION ) )
  {
    return false;
  }
  if ( ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  &&   ( !( m_dwWantedStyles & WS_SYSMENU ) ) )
  {
    return false;
  }
  if ( !( m_dwWantedStyles & WS_MINIMIZEBOX ) )
  {
    return false;
  }

  GetWindowRect( &rc );
  OffsetRect( &rc, -rc.left, -rc.top );

  rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.right -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iSmallCaptionHeight;
  }
  else
  {
    rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
  }

  rc.top += 2;
  rc.bottom -= 2;
  rc.right -= 2 + 2 * ( ( rc.bottom - rc.top ) + 2 ) + 2;
  rc.left = rc.right - ( rc.bottom - rc.top ) - 2;

  return true;

}



bool CCustomWnd::GetSysMenuRect( RECT& rc )
{

  if ( !( m_dwWantedStyles & WS_CAPTION ) )
  {
    return false;
  }
  if ( !( m_dwWantedStyles & WS_SYSMENU ) )
  {
    return false;
  }

  if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
  {
    return false;
  }

  GetWindowRect( &rc );
  OffsetRect( &rc, -rc.left, -rc.top );

  rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
  rc.right = rc.left + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
  rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  rc.bottom = rc.top + m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;

  return true;

}



void CCustomWnd::CreateSysMenu( CGRUIMenu& menuSystem )
{

  if ( m_dwWantedStyles & WS_MAXIMIZEBOX )
  {
    menuSystem.AppendMenu( MF_STRING, SC_RESTORE, "&Wiederherstellen" );
  }
  menuSystem.AppendMenu( MF_STRING, SC_MOVE,    "&Verschieben" );

  if ( m_dwWantedStyles & WS_MAXIMIZEBOX )
  {
    menuSystem.AppendMenu( MF_STRING, SC_SIZE,    "&Größe" );
  }
  if ( m_dwWantedStyles & WS_MINIMIZEBOX )
  {
    menuSystem.AppendMenu( MF_STRING, SC_MINIMIZE, "Mi&nimieren" );
  }
  if ( m_dwWantedStyles & WS_MAXIMIZEBOX )
  {
    menuSystem.AppendMenu( MF_STRING, SC_MAXIMIZE, "Ma&ximieren" );
  }
  menuSystem.AppendMenu( MF_SEPARATOR );
  menuSystem.AppendMenu( MF_STRING, SC_CLOSE,   "S&chließen" );

  if ( IsZoomed() )
  {
    menuSystem.EnableMenuItem( SC_MAXIMIZE, FALSE );
    menuSystem.EnableMenuItem( SC_MOVE, FALSE );
    menuSystem.EnableMenuItem( SC_SIZE, FALSE );
  }
  else if ( IsIconic() )
  {
    menuSystem.EnableMenuItem( SC_MINIMIZE, FALSE );
    menuSystem.EnableMenuItem( SC_MOVE, FALSE );
    menuSystem.EnableMenuItem( SC_SIZE, FALSE );
  }
  else
  {
    menuSystem.EnableMenuItem( SC_RESTORE, FALSE );
  }

}



DWORD CCustomWnd::NCHitTest( const POINT& pt )
{

  RECT    rc;

  GetWindowRect( &rc );

  OffsetRect( &rc, -rc.left, -rc.top );

  if ( !PtInRect( &rc, pt ) )
  {
    return HTNOWHERE;
  }

  if ( ( GetSysMenuRect( rc ) )
  &&   ( PtInRect( &rc, pt ) ) )
  {
    return HTSYSMENU;
  }

  if ( ( GetCaptionCloseButtonRect( rc ) )
  &&   ( PtInRect( &rc, pt ) ) )
  {
    return HTCLOSE;
  }
  if ( ( GetCaptionMaximizeButtonRect( rc ) )
  &&   ( PtInRect( &rc, pt ) ) )
  {
    return HTMAXBUTTON;
  }
  if ( ( GetCaptionMinimizeButtonRect( rc ) )
  &&   ( PtInRect( &rc, pt ) ) )
  {
    return HTMINBUTTON;
  }

  if ( ( GetCaptionRect( rc ) )
  &&   ( PtInRect( &rc, pt ) ) )
  {
    return HTCAPTION;
  }


  GetWindowRect( &rc );

  OffsetRect( &rc, -rc.left, -rc.top );

  if ( ( m_dwWantedExtendedStyle & WS_EX_WINDOWEDGE )
  ||   ( m_dwWantedExtendedStyle & WS_EX_CLIENTEDGE )
  ||   ( m_dwWantedStyles & WS_THICKFRAME ) )
  {
    DWORD   dwFlags = 0;

    if ( ( pt.x >= 0 )
    &&   ( pt.x < m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth ) )
    {
      dwFlags |= 1;
    }
    if ( ( pt.x >= rc.right - m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth )
    &&   ( pt.x < rc.right ) )
    {
      dwFlags |= 2;
    }
    if ( ( pt.y >= 0 )
    &&   ( pt.y < m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight ) )
    {
      dwFlags |= 4;
    }
    if ( ( pt.y >= rc.bottom - m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight )
    &&   ( pt.y < rc.bottom ) )
    {
      dwFlags |= 8;
    }
    if ( ( pt.x >= 0 )
    &&   ( pt.x < m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( dwFlags & 4 ) )
    {
      dwFlags |= 5;
    }
    if ( ( dwFlags & 1 )
    &&   ( pt.y >= 0 )
    &&   ( pt.y < m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight ) )
    {
      dwFlags |= 5;
    }
    if ( ( dwFlags & 2 )
    &&   ( pt.y >= 0 )
    &&   ( pt.y < m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight ) )
    {
      dwFlags |= 6;
    }
    if ( ( pt.x >= rc.right - m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( pt.x < rc.right )
    &&   ( dwFlags & 4 ) )
    {
      dwFlags |= 6;
    }
    if ( ( pt.x >= 0 )
    &&   ( pt.x < m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( dwFlags & 8 ) )
    {
      dwFlags |= 9;
    }
    if ( ( pt.y >= rc.bottom - m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( pt.y < rc.bottom )
    &&   ( dwFlags & 1 ) )
    {
      dwFlags |= 9;
    }
    if ( ( pt.y >= rc.bottom - m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( pt.y < rc.bottom )
    &&   ( dwFlags & 2 ) )
    {
      dwFlags |= 10;
    }
    if ( ( pt.x >= rc.right - m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight )
    &&   ( pt.x < rc.right )
    &&   ( dwFlags & 8 ) )
    {
      dwFlags |= 10;
    }

    if ( ( dwFlags )
    &&   ( !( m_dwWantedStyles & WS_THICKFRAME ) ) )
    {
      return HTBORDER;
    }


    switch ( dwFlags )
    {
      case 1:
        return HTLEFT;
      case 2:
        return HTRIGHT;
      case 4:
        return HTTOP;
      case 5:
        return HTTOPLEFT;
      case 6:
        return HTTOPRIGHT;
      case 8:
        return HTBOTTOM;
      case 9:
        return HTBOTTOMLEFT;
      case 10:
        return HTBOTTOMRIGHT;
    }
  }

  return HTCLIENT;

}



void CCustomWnd::CalculateClientRectFromWindowRect( RECT& rc )
{

  if ( m_dwWantedExtendedStyle & WS_EX_CLIENTEDGE )
  {
    rc.left += m_pDisplayClass->m_DisplayConstants.m_iClientEdgeWidth;
    rc.top += m_pDisplayClass->m_DisplayConstants.m_iClientEdgeHeight;
    rc.right-= m_pDisplayClass->m_DisplayConstants.m_iClientEdgeWidth;
    rc.bottom -= m_pDisplayClass->m_DisplayConstants.m_iClientEdgeHeight;
  }

  if ( ( m_dwWantedStyles & WS_THICKFRAME )
  ||   ( m_dwWantedStyles & WS_DLGFRAME ) )
  {
    rc.left += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
    rc.top += m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
    rc.right -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderWidth;
    rc.bottom -= m_pDisplayClass->m_DisplayConstants.m_iSizeBorderHeight;
  }
  if ( ( m_dwWantedStyles & WS_CAPTION ) == WS_CAPTION )
  {
    if ( m_dwWantedExtendedStyle & WS_EX_TOOLWINDOW )
    {
      rc.top += m_pDisplayClass->m_DisplayConstants.m_iSmallCaptionHeight;
    }
    else
    {
      rc.top += m_pDisplayClass->m_DisplayConstants.m_iCaptionHeight;
    }
  }

}



void CCustomWnd::CalculateWindowRectFromClientRect( RECT& rc )
{

  RECT    rcOrig;

  SetRectEmpty( &rcOrig );

  CalculateClientRectFromWindowRect( rcOrig );

  rc.left -= rcOrig.left;
  rc.right -= rcOrig.right;
  rc.top -= rcOrig.top;
  rc.bottom -= rcOrig.bottom;

}



void CCustomWnd::DrawCaption( HDC hdc, bool bActive )
{

  RECT    rcCaption;
  if ( GetCaptionRect( rcCaption ) )
  {
    m_pDisplayClass->PaintWindowCaption( hdc, rcCaption, m_hWnd, bActive );

    if ( GetCaptionCloseButtonRect( rcCaption ) )
    {
      m_pDisplayClass->PaintWindowCaptionCloseButton( hdc, rcCaption, m_dwWantedStyles, m_bCaptionCloseButtonDown, true );
    }
    if ( GetCaptionMaximizeButtonRect( rcCaption ) )
    {
      m_pDisplayClass->PaintWindowCaptionMaximizeButton( hdc, rcCaption, m_dwWantedStyles, m_bCaptionMaxButtonDown, true, IsZoomed() );
    }
    if ( GetCaptionMinimizeButtonRect( rcCaption ) )
    {
      m_pDisplayClass->PaintWindowCaptionMinimizeButton( hdc, rcCaption, m_dwWantedStyles, m_bCaptionMinButtonDown, true, IsIconic() );
    }
  }

}



HDC CCustomWnd::BeginNCPaint()
{

  m_HDCNCArea = GetWindowDC( m_hWnd );
  RECT    rc;

  GetWindowRect( &rc );
  //HRGN    hrgnTemp = CreateRectRgn( rc.left, rc.top, rc.right, rc.bottom );

  //m_HDCNCArea = GetDCEx( m_hWnd, hrgnTemp, DCX_WINDOW | DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN | 0x10000 );

  //HRGN    hrgnTemp = CreateRectRgn( rc.left, rc.top, rc.right, rc.bottom );
  //HRGN    hrgnTemp = CreateRectRgn( 0, 0, rc.right - rc.left, rc.bottom - rc.top );

  /*
  HWND    hwndPrev = GetWindow( GW_HWNDPREV );
  while ( hwndPrev )
  {
    if ( hwndPrev != m_hWnd )
    {
      RECT    rcChild;

      ::GetWindowRect( hwndPrev, &rcChild );

      OffsetRect( &rcChild, -rc.left, -rc.top );

      HRGN    hrgnChild = CreateRectRgn( rcChild.left, rcChild.top, rcChild.right, rcChild.bottom );
      HRGN    hrgnResult = CreateRectRgn( 0, 0, 0, 0 );

      CombineRgn( hrgnResult, hrgnTemp, hrgnChild, RGN_DIFF );
      DeleteObject( hrgnChild );
      DeleteObject( hrgnTemp );
      hrgnTemp = hrgnResult;
    }

    hwndPrev = ::GetNextWindow( hwndPrev, GW_HWNDPREV );
  }
  */

  /*
  HWND    hwndParent = GetParent();
  if ( hwndParent )
  {
    HWND    hwndChilds = ::GetWindow( hwndParent, GW_CHILD );
    while ( hwndChilds )
    {
      if ( hwndChilds != m_hWnd )
      {
        RECT    rcChild;

        ::GetWindowRect( hwndChilds, &rcChild );

        OffsetRect( &rcChild, -rc.left, -rc.top );

        HRGN    hrgnChild = CreateRectRgn( rcChild.left, rcChild.top, rcChild.right, rcChild.bottom );
        HRGN    hrgnResult = CreateRectRgn( 0, 0, 0, 0 );

        CombineRgn( hrgnResult, hrgnTemp, hrgnChild, RGN_DIFF );
        DeleteObject( hrgnChild );
        DeleteObject( hrgnTemp );
        hrgnTemp = hrgnResult;
      }

      hwndChilds = ::GetNextWindow( hwndChilds, GW_HWNDNEXT );
    }
  }

  int   iType = SelectClipRgn( m_HDCNCArea, hrgnTemp );
  //dh::Log( "ClipRgn Type %d", iType );

  DeleteObject( hrgnTemp );
  */

  m_HDCNCDoubleBuffer = CreateCompatibleDC( m_HDCNCArea );

  m_hbmNCOld = (HBITMAP)SelectObject( m_HDCNCDoubleBuffer, m_hbmDoubleBuffer );

  return m_HDCNCDoubleBuffer;

}



void CCustomWnd::EndNCPaint()
{

  RECT          rc;

  GetWindowRect( &rc );

  RECT        rcClient = rc;

  CalculateClientRectFromWindowRect( rcClient );

  // oben
  BitBlt( m_HDCNCArea, 0, 0, rc.right - rc.left, rcClient.top - rc.top,
          m_HDCNCDoubleBuffer, 0, 0, SRCCOPY );
  // links
  BitBlt( m_HDCNCArea, 0, rcClient.top - rc.top, rcClient.left - rc.left, rcClient.bottom - rcClient.top,
          m_HDCNCDoubleBuffer, 0, rcClient.top - rc.top, SRCCOPY );
  // rechts
  BitBlt( m_HDCNCArea, rcClient.right - rc.left, rcClient.top - rc.top, rc.right - rcClient.right, rcClient.bottom - rcClient.top,
          m_HDCNCDoubleBuffer, rcClient.right - rc.left, rcClient.top - rc.top, SRCCOPY );
  // unten
  BitBlt( m_HDCNCArea, 0, rcClient.bottom - rc.top, rc.right - rc.left, rc.bottom - rcClient.bottom,
          m_HDCNCDoubleBuffer, 0, rcClient.bottom - rc.top, SRCCOPY );

  SelectObject( m_HDCNCDoubleBuffer, m_hbmNCOld );
  DeleteDC( m_HDCNCDoubleBuffer );

  ReleaseDC( m_HDCNCArea );

}



HDC CCustomWnd::StartPaint()
{

  m_hdcPaint = BeginPaint( m_hWnd, &m_psPaint );

  return m_hdcPaint;

}



void CCustomWnd::StopPaint()
{

  EndPaint( m_hWnd, &m_psPaint );
  m_hdcPaint = NULL;

}



void CCustomWnd::OnParentResized()
{

}



void CCustomWnd::OnParentRecalcClientSize( RECT& rcClient )
{

}