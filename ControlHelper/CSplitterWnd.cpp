#include <windows.h>

#include <map>

#include <Debug\debugclient.h>

#include <ControlHelper\CSplitterWnd.h>



namespace WindowsWrapper
{

  CSplitterWnd::CSplitterWnd() :
    m_bHorizontal( true ),
    m_iSplitterSize( 3 ),
    m_bDragging( false ),
    m_bAllowDragging( true ),
    m_hwndFirstPane( NULL ),
    m_hwndSecondPane( NULL )
  {

    _RegisterClass();

  }



  CSplitterWnd::~CSplitterWnd()
  {


  }



  BOOL CSplitterWnd::Create( HWND hwndParent,
                             const RECT& rect,
                             bool bHorizontal,
                             SplitterWndMode swMode,
                             int iSplitModeValue,
                             HWND hwndFirstPane,
                             HWND hwndSecondPane,
                             DWORD dwStyle,
                             DWORD dwID )
  {
    m_rcComplete = rect;

    RECT    rc( rect );

    if ( bHorizontal )
    {
      rc.left = iSplitModeValue;
      rc.right = rc.left + 3;
    }
    else
    {
      rc.top = iSplitModeValue;
      rc.bottom = rc.top + 3;
    }

    BOOL    bResult = CWnd::Create( "SPLITTERWNDCLASS",
                                    "Splitter",
                                    dwStyle,
                                    rc,
                                    hwndParent,
                                    dwID );

    if ( bResult )
    {
      m_bHorizontal = bHorizontal;
      m_hwndFirstPane = hwndFirstPane;
      m_hwndSecondPane = hwndSecondPane;
      m_swMode = swMode;
      m_iSplitModeValue = iSplitModeValue;
      m_hwndParent = hwndParent;

      if ( m_hwndParent )
      {
        m_iSplitterPos = m_bHorizontal ? rc.left : rc.top;
      }

      UpdateSplitterPos();
    }

    return bResult;
  }



  void CSplitterWnd::SetSplitMode( SplitterWndMode swMode, int iValue )
  {

    m_swMode = swMode;
    m_iSplitModeValue = iValue;

    UpdateSplitterPos();

  }



  void CSplitterWnd::UpdateSplitterPos()
  {

    int   iNewPos = -1;

    if ( m_swMode == SWM_RELATIVE )
    {
      if ( m_bHorizontal )
      {
        iNewPos = ( ( m_rcComplete.right - m_rcComplete.left - m_iSplitterSize ) * m_iSplitModeValue ) / 100;
      }
      else
      {
        iNewPos = ( ( m_rcComplete.bottom - m_rcComplete.top - m_iSplitterSize ) * m_iSplitModeValue ) / 100;
      }
    }
    else if ( m_swMode == SWM_ABSOLUTE_FIRST_PANE )
    {
      iNewPos = m_iSplitModeValue;
    }
    else if ( m_swMode == SWM_ABSOLUTE_SECOND_PANE )
    {
      if ( m_bHorizontal )
      {
        iNewPos = ( m_rcComplete.right - m_rcComplete.left ) - m_iSplitModeValue;
      }
      else
      {
        iNewPos = ( m_rcComplete.bottom - m_rcComplete.top ) - m_iSplitModeValue;
      }
    }
    else if ( m_swMode == SWM_FREE )
    {
      iNewPos = m_iSplitterPos;
    }

    if ( m_bHorizontal )
    {
      if ( iNewPos > m_rcComplete.right )
      {
        iNewPos = m_rcComplete.right;
      }
    }
    else
    {
      if ( iNewPos > m_rcComplete.bottom )
      {
        iNewPos = m_rcComplete.bottom;
      }
    }

    if ( iNewPos < 0 )
    {
      iNewPos = 0;
    }

    if ( iNewPos != m_iSplitterPos )
    {

      m_iSplitterPos = iNewPos;
      Invalidate();
    }

  }



  void CSplitterWnd::GetSplitterRect( RECT* pRect )
  {

    RECT    rc;

    ::GetClientRect( m_hWnd, &rc );

    if ( !m_bHorizontal )
    {
      pRect->left = rc.left;
      pRect->right = rc.right;

      pRect->top = m_iSplitterPos;
      pRect->bottom = pRect->top + m_iSplitterSize;
    }
    else
    {
      pRect->top = rc.top;
      pRect->bottom = rc.bottom;

      pRect->left = m_iSplitterPos;
      pRect->right = pRect->left + m_iSplitterSize;
    }

  }



  void CSplitterWnd::_RegisterClass()
  {
    WNDCLASSA    wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MYHELPERWINDOWPROC;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = (HINSTANCE)GetModuleHandle( NULL );
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)( COLOR_BTNFACE + 1 );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "SPLITTERWNDCLASS";
    if ( !RegisterClassA( &wc ) )
    {
      //dh::Log( "CSplitterWnd::_RegisterClass failed (%d)\n", GetLastError() );
    }
  }




  LRESULT CSplitterWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    if ( ( uMsg == WM_WINDOWPOSCHANGING )
         || ( uMsg == WM_WINDOWPOSCHANGED ) )
    {
      WINDOWPOS*  pPos = (LPWINDOWPOS)lParam;

      //Resize( pPos->cx, pPos->cy );
    }
    if ( uMsg == WM_MOUSEMOVE )
    {
      if ( m_bDragging )
      {
        RECT    rc;
        if ( m_hwndParent )
        {
          ::GetClientRect( m_hwndParent, &rc );
        }
        RECT    rcMyRect;
        GetWindowRect( &rcMyRect );

        if ( m_hwndParent )
        {
          POINT   pt;

          pt.x = rcMyRect.left;
          pt.y = rcMyRect.top;
          ::ScreenToClient( m_hwndParent, &pt );

          OffsetRect( &rcMyRect, pt.x - rcMyRect.left, pt.y - rcMyRect.top );
        }

        POINT   pt;
        GetCursorPos( &pt );
        ::ScreenToClient( m_hwndParent, &pt );

        int   iBottomLength = 0;

        if ( !m_bHorizontal )
        {
          if ( pt.y < 0 )
          {
            pt.y = 0;
          }
          if ( pt.y > rc.bottom - m_iSplitterSize )
          {
            pt.y = rc.bottom - m_iSplitterSize;
          }

          m_iSplitterPos = pt.y;

          iBottomLength = rc.bottom - m_iSplitterPos - m_iSplitterSize;

          ::MoveWindow( m_hwndFirstPane, rcMyRect.left, rc.top, rcMyRect.right - rcMyRect.left, rc.top + m_iSplitterPos, TRUE );
          ::MoveWindow( m_hwndSecondPane, rcMyRect.left, rc.top + m_iSplitterPos + m_iSplitterSize, rcMyRect.right - rcMyRect.left, iBottomLength, TRUE );
          MoveWindow( rc.left, rc.top + m_iSplitterPos );
        }
        else
        {
          if ( pt.x < 0 )
          {
            pt.x = 0;
          }
          if ( pt.x > rc.right - m_iSplitterSize )
          {
            pt.x = rc.right - m_iSplitterSize;
          }

          m_iSplitterPos = pt.x;

          iBottomLength = rc.right - m_iSplitterPos - m_iSplitterSize;

          RECT    rcPane;
          ::GetWindowRect( m_hwndFirstPane, &rcPane );
          ::MoveWindow( m_hwndFirstPane, rc.left, rcMyRect.top, rc.left + m_iSplitterPos, rcMyRect.bottom - rcMyRect.top, TRUE );

          ::GetWindowRect( m_hwndSecondPane, &rcPane );
          ::MoveWindow( m_hwndSecondPane, rc.left + m_iSplitterPos + m_iSplitterSize, rcMyRect.top,
                        iBottomLength, rcMyRect.bottom - rcMyRect.top, TRUE );
          MoveWindow( rc.left + m_iSplitterPos, rcMyRect.top );
        }
      }
    }
    if ( uMsg == WM_LBUTTONUP )
    {
      if ( m_bDragging )
      {
        ReleaseCapture();
        m_bDragging = false;
        Invalidate( FALSE );
        ::InvalidateRect( m_hwndFirstPane, NULL, FALSE );
        ::InvalidateRect( m_hwndSecondPane, NULL, FALSE );
      }
    }
    if ( uMsg == WM_SETCURSOR )
    {
      if ( m_bAllowDragging )
      {
        if ( !m_bHorizontal )
        {
          SetCursor( LoadCursor( NULL, IDC_SIZENS ) );
        }
        else
        {
          SetCursor( LoadCursor( NULL, IDC_SIZEWE ) );
        }
        UINT    msg = HIWORD( lParam );
        if ( msg == WM_LBUTTONDOWN )
        {
          if ( ( !m_bDragging )
               && ( m_bAllowDragging ) )
          {
            m_bDragging = true;

            SetCapture();
          }
        }
        return TRUE;
      }
    }
    return CWnd::WindowProc( uMsg, wParam, lParam );
  }



  void CSplitterWnd::Resize( int iWidth, int iHeight )
  {

    if ( ( m_hwndFirstPane == NULL )
         || ( m_hwndSecondPane == NULL ) )
    {
      return;
    }

    bool      bChangeSize = false;

    if ( ( iWidth != m_rcComplete.right - m_rcComplete.left )
         || ( iHeight != m_rcComplete.bottom - m_rcComplete.top ) )
    {
      bChangeSize = true;
    }

    m_rcComplete.right = m_rcComplete.left + iWidth;
    m_rcComplete.bottom = m_rcComplete.top + iHeight;

    RECT    rc( m_rcComplete );
    if ( m_bHorizontal )
    {
      rc.left = m_iSplitterPos;
      rc.right = rc.left + 3;
    }
    else
    {
      rc.top = m_iSplitterPos;
      rc.bottom = rc.top + 3;
    }
    ::MoveWindow( m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );

    if ( !m_bHorizontal )
    {
      UpdateSplitterPos();
      int   iBottomLength = iHeight - m_iSplitterPos - m_iSplitterSize;

      RECT    rcSplitter;
      GetSplitterRect( &rcSplitter );

      ::MoveWindow( m_hwndFirstPane, m_rcComplete.left, m_rcComplete.top, iWidth, m_rcComplete.top + m_iSplitterPos, TRUE );
      ::MoveWindow( m_hwndSecondPane, m_rcComplete.left, m_rcComplete.top + m_iSplitterPos + m_iSplitterSize, iWidth, iBottomLength, TRUE );

      Invalidate();
      UpdateWindow();
    }
    else
    {
      UpdateSplitterPos();

      int   iBottomLength = iWidth - m_iSplitterPos - m_iSplitterSize;

      RECT    rcSplitter;
      GetSplitterRect( &rcSplitter );

      ::MoveWindow( m_hwndFirstPane, m_rcComplete.left, m_rcComplete.top,
                    m_rcComplete.left + m_iSplitterPos, iHeight, TRUE );
      ::MoveWindow( m_hwndSecondPane, m_rcComplete.left + m_iSplitterPos + m_iSplitterSize, m_rcComplete.top,
                    iBottomLength, iHeight, TRUE );

      Invalidate();
      UpdateWindow();
    }

  }



  void CSplitterWnd::SetPane( int iPane, HWND hwndPane )
  {

    if ( iPane == 1 )
    {
      m_hwndFirstPane = hwndPane;
    }
    else if ( iPane == 2 )
    {
      m_hwndSecondPane = hwndPane;
    }

  }



  void CSplitterWnd::AllowDragging( bool bAllow )
  {

    m_bAllowDragging = bAllow;

  }



  void CSplitterWnd::SetExtents( int iWidth, int iHeight )
  {

    if ( m_bHorizontal )
    {
      MoveWindow( m_iSplitterPos, 0, m_iSplitterSize, iHeight );
    }
    else
    {
      MoveWindow( 0, m_iSplitterPos, iWidth, m_iSplitterSize );
    }
    Resize( iWidth, iHeight );

  }


}