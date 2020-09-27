#include ".\gruimdiview.h"
#include ".\gruimdidocument.h"
#include ".\gruimdiapp.h"



GRUIMDIView::GRUIMDIView( GRUIMDIDocument* pDoc ) :
  m_pDocument( pDoc )
{
}

GRUIMDIView::~GRUIMDIView(void)
{
}




void GRUIMDIView::Draw( HDC hdc, const RECT& rc )
{

  m_pDisplayClass->FillSolidRect( hdc, &rc, RGB( 255, 0, 255 ) );

  CCustomWnd::Draw( hdc, rc );

}



void GRUIMDIView::DrawBackground( HDC hdc, const RECT& rc )
{

}



LRESULT GRUIMDIView::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_NCCREATE:
      GRUIMDIApp::m_listSyncWindows.push_back( m_hWnd );
      break;
    case WM_NCDESTROY:
      GRUIMDIApp::m_listSyncWindows.remove( m_hWnd );
      break;
    case WM_CLOSE:
      if ( ( m_pDocument->ViewCount() == 1 )
      &&   ( !m_pDocument->CanClose() ) )
      {
        return 0;
      }
      break;
    case WM_NCPAINT:
      {
        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        DrawCaption( hdc, IsCaptionActive() );
        EndNCPaint();
      }
      return 0;
    case WM_NCACTIVATE:
    case WM_ACTIVATE:
      //if ( !wParam )
      if ( uMsg == WM_ACTIVATE )
      {
        if ( wParam != WA_INACTIVE )
        {
          dh::Log( "View Activated" );
        }
        else
        {
          dh::Log( "View deActivated" );
        }
      }

      {
        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        DrawCaption( hdc, IsCaptionActive() );
        EndNCPaint();
      }
      return TRUE;
    case WM_NCLBUTTONDOWN:
      GRUIMDIApp::m_pMDIApp->SetActiveView( this );
      break;
    case WM_LBUTTONDOWN:
      GRUIMDIApp::m_pMDIApp->SetActiveView( this );
      break;
  }
  return CCustomWnd::WindowProc( uMsg, wParam, lParam );

}



bool GRUIMDIView::IsCaptionActive()
{

  if ( !::IsWindowEnabled( GRUIMDIApp::m_pMDIApp->GetSafeHwnd() ) )
  {
    return false;
  }

  std::list<HWND>   listHwnd;

  GRUIMDIApp::GetPopupList( GRUIMDIApp::m_pMDIApp->GetSafeHwnd(), listHwnd, TRUE);

  std::list<HWND>::iterator   it( listHwnd.begin() );
  while ( it != listHwnd.end() )
  {
    if ( GetActiveWindow() == *it )
    {
      // das andere Window ist ja auch eins von unseren
      return true;
    }

    ++it;
  }

  return false;

}



HDC GRUIMDIView::BeginNCPaint()
{

  m_HDCNCArea = GetWindowDC( m_hWnd );

  RECT    rc;

  GetWindowRect( &rc );

  HRGN    hrgnTemp = CreateRectRgn( 0, 0, rc.right - rc.left, rc.bottom - rc.top );

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
      else
      {
        break;
      }

      hwndChilds = ::GetNextWindow( hwndChilds, GW_HWNDNEXT );
    }
  }

  SelectClipRgn( m_HDCNCArea, hrgnTemp );

  DeleteObject( hrgnTemp );

  m_HDCNCDoubleBuffer = CreateCompatibleDC( m_HDCNCArea );

  m_hbmNCOld = (HBITMAP)SelectObject( m_HDCNCDoubleBuffer, m_hbmDoubleBuffer );

  return m_HDCNCDoubleBuffer;

}



HDC GRUIMDIView::StartPaint()
{

  m_hdcTemp = GetDC();

  RECT    rc;

  GetClientRect( &rc );

  HRGN    hrgnTemp = CreateRectRgn( 0, 0, rc.right - rc.left, rc.bottom - rc.top );

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

        ScreenToClient( &rcChild );

        HRGN    hrgnChild = CreateRectRgn( rcChild.left, rcChild.top, rcChild.right, rcChild.bottom );
        HRGN    hrgnResult = CreateRectRgn( 0, 0, 0, 0 );

        CombineRgn( hrgnResult, hrgnTemp, hrgnChild, RGN_DIFF );
        DeleteObject( hrgnChild );
        DeleteObject( hrgnTemp );
        hrgnTemp = hrgnResult;
      }
      else
      {
        break;
      }

      hwndChilds = ::GetNextWindow( hwndChilds, GW_HWNDNEXT );
    }
  }

  SelectClipRgn( m_hdcTemp, hrgnTemp );

  DeleteObject( hrgnTemp );

  return m_hdcTemp;

}



void GRUIMDIView::StopPaint()
{

  RECT    rcClient;

  GetClientRect( &rcClient );

  BitBlt( m_hdcTemp, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
          m_hdcPaint, 0, 0, SRCCOPY );

  ValidateRect( &rcClient );

  ReleaseDC( m_hdcPaint );

  m_hdcPaint = NULL;

}



void GRUIMDIView::OnMDIActivate()
{
}



void GRUIMDIView::OnMDIDeactivate()
{
}
