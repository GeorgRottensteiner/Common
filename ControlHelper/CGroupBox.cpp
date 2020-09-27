#include <windows.h>
#include <winuser.h>

#include "CGroupBox.h"



namespace WindowsWrapper
{

  CGroupBox::CGroupBox() :
    m_hFont( NULL ),
    m_bChecked( false )
  {
    m_hFont = CreateFontW( 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NONANTIALIASED_QUALITY, 0, L"Tahoma" );
    m_hbmCheckBox = LoadBitmap( NULL, MAKEINTRESOURCE( 32759 ) ); // OBM_CHECKBOXES
  }



  CGroupBox::~CGroupBox()
  {
    DeleteObject( m_hFont );
    DeleteObject( m_hbmCheckBox );
  }



  BOOL CGroupBox::PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {
    return FALSE;//CButton::PreTranslateMessage( hWnd, uMsg, wParam, lParam, lResult );
  }



  void CGroupBox::OnPaint( HDC hdc )
  {
    RECT    rc;

    GetClientRect( &rc );

    GR::String     strCaption = GetWindowText();

    HBRUSH          brushDark = CreateSolidBrush( GetSysColor( COLOR_3DSHADOW ) ),
      brushLight = CreateSolidBrush( GetSysColor( COLOR_3DHILIGHT ) );

    rc.top += 8;
    rc.left++;
    rc.right;
    rc.bottom;

    FrameRect( hdc, &rc, brushLight );

    OffsetRect( &rc, -1, -1 );

    FrameRect( hdc, &rc, brushDark );

    GetClientRect( &rc );

    rc.bottom = rc.top + 8;
    rc.left = 20;

    HFONT   hOldFont = (HFONT)SelectObject( hdc, m_hFont );

    SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
    SetBkMode( hdc, OPAQUE );
    SetBkColor( hdc, GetSysColor( COLOR_BTNFACE ) );

    strCaption = "        " + strCaption + " ";

    GR::WString   utf16Text = GR::Convert::ToUTF16( strCaption );
    DrawTextW( hdc, utf16Text.c_str(), (int)strCaption.length(), &rc, DT_CALCRECT );

    rc.left -= 4;
    rc.right += 4;

    DrawTextW( hdc, utf16Text.c_str(), (int)strCaption.length(), &rc, DT_LEFT );

    // Checkbox darstellen
    GetCheckBoxRect( rc );

    HDC   hdcBitmap = CreateCompatibleDC( hdc );
    HBITMAP hbmOld = (HBITMAP)SelectObject( hdcBitmap, m_hbmCheckBox );

    int   iXOffset = 0;
    if ( m_bChecked )
    {
      iXOffset = 13;
    }
    BitBlt( hdc,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            hdcBitmap,
            iXOffset,
            0,
            SRCCOPY );

    SelectObject( hdcBitmap, hbmOld );
    DeleteDC( hdcBitmap );

    SelectObject( hdc, hOldFont );

    DeleteObject( brushDark );
    DeleteObject( brushLight );
  }



  void CGroupBox::GetCheckBoxRect( RECT& rc )
  {

    GetClientRect( &rc );

    rc.left += 21;
    rc.top++;
    rc.right = rc.left + 13;
    rc.bottom = rc.top + 13;

  }



  void CGroupBox::GetLabelRect( RECT& rc )
  {
    GetClientRect( &rc );

    rc.bottom = rc.top + 8;
    rc.left = 20;

    GR::String     strCaption = GetWindowText();

    HDC   hdc = GetDC();

    strCaption = "        " + strCaption + " ";

    HFONT   hOldFont = (HFONT)SelectObject( hdc, m_hFont );

    DrawTextW( hdc, GR::Convert::ToUTF16( strCaption ).c_str(), (int)strCaption.length(), &rc, DT_CALCRECT );

    SelectObject( hdc, hOldFont );

    ReleaseDC( hdc );

    rc.left -= 4;
    rc.right += 4;
  }



  void CGroupBox::SetCheck( int iCheckState )
  {

    bool    bCheckNow = ( iCheckState != 0 );

    m_bChecked = bCheckNow;

    EnableContainingControls( m_bChecked );

    RECT    rcCheckBox;

    GetCheckBoxRect( rcCheckBox );

    InvalidateRect( &rcCheckBox );

  }



  int CGroupBox::GetCheck() const
  {

    return m_bChecked ? 1 : 0;

  }



  void CGroupBox::EnableContainingControls( bool bEnable )
  {

    HWND    hwndParent = GetParent();

    if ( hwndParent == NULL )
    {
      return;
    }

    RECT    rcClient;

    GetWindowRect( &rcClient );

    HWND    hwndTemp = ::GetWindow( hwndParent, GW_CHILD );

    while ( hwndTemp )
    {
      RECT    rcChild;

      ::GetWindowRect( hwndTemp, &rcChild );

      if ( ( rcChild.left >= rcClient.left )
           && ( rcChild.top >= rcClient.top )
           && ( rcChild.right <= rcClient.right )
           && ( rcChild.bottom <= rcClient.bottom )
           && ( hwndTemp != m_hWnd ) )
      {
        ::EnableWindow( hwndTemp, bEnable );
      }

      hwndTemp = ::GetNextWindow( hwndTemp, GW_HWNDNEXT );
    }

  }



  void CGroupBox::OnAttach()
  {

    EnableContainingControls( !!GetCheck() );

  }



  LRESULT CGroupBox::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
  {

    switch ( message )
    {
      case WM_PAINT:
        if ( wParam == 0 )
        {
          HDC   hdc = GetDC();
          OnPaint( hdc );
          ReleaseDC( hdc );
        }
        else
        {
          OnPaint( (HDC)wParam );
        }

        RECT    rc;

        GetClientRect( &rc );
        ValidateRect( &rc );
        return TRUE;
      case WM_LBUTTONDOWN:
      case WM_LBUTTONDBLCLK:
        {
          RECT    rc;

          GetLabelRect( rc );

          POINT   pt;

          pt.x = LOWORD( lParam );
          pt.y = HIWORD( lParam );
          if ( PtInRect( &rc, pt ) )
          {
            SetCheck( !m_bChecked );
          }
        }
        break;
      case WM_GETDLGCODE:
        return DLGC_BUTTON;
      case WM_NCHITTEST:
        {
          RECT    rc;

          GetLabelRect( rc );

          POINT   pt;

          pt.x = LOWORD( lParam );
          pt.y = HIWORD( lParam );

          ScreenToClient( &pt );
          if ( PtInRect( &rc, pt ) )
          {
            return HTCLIENT;
          }
        }
        break;
      case WM_ERASEBKGND:
        return TRUE;
    }
    return CButton::WindowProc( message, wParam, lParam );

  }

}