// TransparentButton.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "TransparentButton.h"


// CTransparentButton

IMPLEMENT_DYNAMIC(CTransparentButton, CButton)
CTransparentButton::CTransparentButton() :
  m_pWndBackground( NULL )
{

  m_hbmCheckBox = LoadBitmap( NULL, MAKEINTRESOURCE( 32759 ) ); // OBM_CHECKBOXES

}

CTransparentButton::~CTransparentButton()
{

  DeleteObject( m_hbmCheckBox );

}


BEGIN_MESSAGE_MAP(CTransparentButton, CButton)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
END_MESSAGE_MAP()



// CTransparentButton-Meldungshandler


BOOL CTransparentButton::OnEraseBkgnd(CDC* pDC)
{

  return TRUE;
}

void CTransparentButton::SetAlternateBackgroundWnd( CWnd* pWndBackground )
{

  m_pWndBackground = pWndBackground;
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

  CPaintDC dc( this ); // device context for painting

  CFont* pOldFont = dc.SelectObject( GetFont() );
  dc.SetBkMode( TRANSPARENT );

  CString   cstrCaption;

  RECT    rcText;

  GetClientRect( &rcText );

  GetWindowText( cstrCaption );

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

    dc.SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
    dc.SetBkMode( TRANSPARENT );
    dc.SetBkColor( GetSysColor( COLOR_BTNFACE ) );

    dc.DrawText( cstrCaption, &rc, DT_CALCRECT );

    rc.left -= 4;
    rc.right += 4;

    HPEN            penDark = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_3DSHADOW ) ),
                    penLight = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_3DHILIGHT ) );

    HPEN penOld = (HPEN)dc.SelectObject( penLight );

    dc.MoveTo( rc.left - 4, rcFrame.top );
    dc.LineTo( rcFrame.left, rcFrame.top );
    dc.LineTo( rcFrame.left, rcFrame.bottom );
    dc.LineTo( rcFrame.right, rcFrame.bottom );
    dc.LineTo( rcFrame.right , rcFrame.top );
    dc.LineTo( rc.right - 4, rcFrame.top );

    dc.SelectObject( penDark );

    OffsetRect( &rcFrame, -1, -1 );
    dc.MoveTo( rc.left - 4 - 1, rcFrame.top );
    dc.LineTo( rcFrame.left, rcFrame.top );
    dc.LineTo( rcFrame.left, rcFrame.bottom );
    dc.LineTo( rcFrame.right, rcFrame.bottom );
    dc.LineTo( rcFrame.right , rcFrame.top );
    dc.LineTo( rc.right - 4, rcFrame.top );

    dc.SelectObject( penOld );

    DeleteObject( penLight );
    DeleteObject( penDark );

    dc.DrawText( cstrCaption, &rc, DT_LEFT );
  }
  else if ( dwStyle & BS_RIGHTBUTTON )
  {
    rcText.left += 2;
    dc.DrawText( cstrCaption, &rcText, DT_SINGLELINE );

    RECT      rcCheckBox;

    GetClientRect( &rcCheckBox );

    rcCheckBox.left = rcCheckBox.right - 13 - 1;

    CDC   dcBitmap;
    
    dcBitmap.CreateCompatibleDC( &dc );
    HBITMAP hbmOld = (HBITMAP)dcBitmap.SelectObject( m_hbmCheckBox );

    int   iXOffset = 0;
    if ( GetCheck() )
    {
      iXOffset = 13;
    }
    dc.BitBlt( rcCheckBox.left + 1,
               rcCheckBox.top + 1,
               13,
               13,
               &dcBitmap,
               iXOffset,
               0,
               SRCCOPY );

    dcBitmap.SelectObject( hbmOld );
    dcBitmap.DeleteDC();
  }
  else if ( dwStyle & BS_CHECKBOX )
  {
    rcText.left += 17 + 1;
    dc.DrawText( cstrCaption, &rcText, DT_SINGLELINE );

    RECT      rcCheckBox;

    GetClientRect( &rcCheckBox );

    rcCheckBox.right = rcCheckBox.left + 13 + 1;

    CDC   dcBitmap;
    
    dcBitmap.CreateCompatibleDC( &dc );
    HBITMAP hbmOld = (HBITMAP)dcBitmap.SelectObject( m_hbmCheckBox );

    int   iXOffset = 0;
    if ( GetCheck() )
    {
      iXOffset = 13;
    }
    dc.BitBlt( rcCheckBox.left,
               rcCheckBox.top + 1,
               13,
               13,
               &dcBitmap,
               iXOffset,
               0,
               SRCCOPY );

    dcBitmap.SelectObject( hbmOld );
    dcBitmap.DeleteDC();
  }

  dc.SelectObject( pOldFont );

}

void CTransparentButton::PreSubclassWindow()
{

  m_pWndBackground = GetParent();

  CButton::PreSubclassWindow();
}
