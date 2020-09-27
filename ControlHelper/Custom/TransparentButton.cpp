// TransparentButton.cpp : Implementierungsdatei
//

#include "TransparentButton.h"


// CTransparentButton

CTransparentButton::CTransparentButton() :
  m_hwndBackground( NULL )
{

  m_hbmCheckBox = LoadBitmap( NULL, MAKEINTRESOURCE( 32759 ) ); // OBM_CHECKBOXES

}

CTransparentButton::~CTransparentButton()
{

  DeleteObject( m_hbmCheckBox );

}


void CTransparentButton::SetAlternateBackgroundWnd( HWND hWnd )
{

  m_hwndBackground = hWnd;
  Invalidate();

}



void CTransparentButton::OnPaint()
{

  if ( !( GetButtonStyle() & ( BS_GROUPBOX | BS_CHECKBOX | BS_AUTOCHECKBOX ) ) )
  {
    Default();
    return;
  }

  DWORD   dwStyle = GetButtonStyle();

  PAINTSTRUCT ps;

  HDC     dc = BeginPaint( m_hWnd, &ps );

  HFONT   hOldFont = (HFONT)SelectObject( dc, GetFont() );
  SetBkMode( dc, TRANSPARENT );

  GR::String   strCaption;

  RECT    rcText;

  GetClientRect( &rcText );

  strCaption = GetWindowText();

  if ( ( dwStyle & BS_GROUPBOX ) == BS_GROUPBOX )
  {
    RECT    rc = rcText;

    GetClientRect( &rc );

    rc.left += 1;
    rc.bottom -= 1;
    rc.top += 8;

    RECT    rcFrame = rc;


    GetClientRect( &rc );

    rc.bottom = rc.top + 8;
    rc.left = 20;

    SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
    SetBkMode( dc, TRANSPARENT );
    SetBkColor( dc, GetSysColor( COLOR_BTNFACE ) );

    DrawText( dc, strCaption.c_str(), (int)strCaption.length(), &rc, DT_CALCRECT );

    rc.left -= 4;
    rc.right += 4;

    HPEN            penDark = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) ),
                    penLight = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_3DHILIGHT ) );

    HPEN penOld = (HPEN)SelectObject( dc, penLight );

    POINT   pt;
    MoveToEx( dc, rc.left - 4, rcFrame.top, &pt );
    LineTo( dc, rcFrame.left, rcFrame.top );
    LineTo( dc, rcFrame.left, rcFrame.bottom );
    LineTo( dc, rcFrame.right, rcFrame.bottom );
    LineTo( dc, rcFrame.right , rcFrame.top );
    LineTo( dc, rc.right - 4, rcFrame.top );

    SelectObject( dc, penDark );

    OffsetRect( &rcFrame, -1, -1 );
    MoveToEx( dc, rc.left - 4 - 1, rcFrame.top, &pt );
    LineTo( dc, rcFrame.left, rcFrame.top );
    LineTo( dc, rcFrame.left, rcFrame.bottom );
    LineTo( dc, rcFrame.right, rcFrame.bottom );
    LineTo( dc, rcFrame.right , rcFrame.top );
    LineTo( dc, rc.right - 4, rcFrame.top );

    SelectObject( dc, penOld );

    DeleteObject( penLight );
    DeleteObject( penDark );

    DrawText( dc, strCaption.c_str(), (int)strCaption.length(), &rc, DT_LEFT );
  }
  else if ( dwStyle & BS_RIGHTBUTTON )
  {
    rcText.left += 2;
    DrawText( dc, strCaption.c_str(), (int)strCaption.length(), &rcText, DT_SINGLELINE );

    RECT      rcCheckBox;

    GetClientRect( &rcCheckBox );

    rcCheckBox.left = rcCheckBox.right - 13 - 1;

    HDC   hdcBitmap;

    hdcBitmap = CreateCompatibleDC( dc );
    HBITMAP hbmOld = (HBITMAP)SelectObject( hdcBitmap, m_hbmCheckBox );

    int   iXOffset = 0;
    if ( GetCheck() )
    {
      iXOffset = 13;
    }
    BitBlt( dc, rcCheckBox.left + 1,
               rcCheckBox.top + 1,
               13,
               13,
               hdcBitmap,
               iXOffset,
               0,
               SRCCOPY );

    SelectObject( hdcBitmap, hbmOld );
    DeleteDC( hdcBitmap );
  }
  else if ( dwStyle & BS_CHECKBOX )
  {
    rcText.left += 17 + 1;
    DrawText( dc, strCaption.c_str(), (int)strCaption.length(), &rcText, DT_SINGLELINE );

    RECT      rcCheckBox;

    GetClientRect( &rcCheckBox );

    rcCheckBox.right = rcCheckBox.left + 13 + 1;

    HDC   dcBitmap;

    dcBitmap = CreateCompatibleDC( dc );
    HBITMAP hbmOld = (HBITMAP)SelectObject( dcBitmap, m_hbmCheckBox );

    int   iXOffset = 0;
    if ( GetCheck() )
    {
      iXOffset = 13;
    }
    BitBlt( dc, rcCheckBox.left,
               rcCheckBox.top + 1,
               13,
               13,
               dcBitmap,
               iXOffset,
               0,
               SRCCOPY );

    SelectObject( dcBitmap, hbmOld );
    DeleteDC( dcBitmap );
  }

  SelectObject( dc, hOldFont );

}



void CTransparentButton::OnAttach()
{

  m_hwndBackground = GetParent();

  CButton::OnAttach();

}



LRESULT CTransparentButton::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{

  switch ( message )
  {
    case WM_ERASEBKGND:
      return TRUE;
    case WM_PAINT:
      OnPaint();
      break;
  }

  return CWnd::WindowProc( message, wParam, lParam );

}