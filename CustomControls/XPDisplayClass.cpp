#include <WinSys/WinUtils.h>

#include "XPDisplayClass.h"


CXPDisplayClass::CXPDisplayClass()
{

  m_DisplayConstants.m_iMenuItemIndent      = 24;
  m_DisplayConstants.m_iMenuItemHeight      = 24;
  m_DisplayConstants.m_iMenuSeparatorHeight = 1;
  m_DisplayConstants.m_iMenuBorderWidth     = GetSystemMetrics( SM_CXBORDER );
  m_DisplayConstants.m_iMenuBorderHeight    = GetSystemMetrics( SM_CYBORDER );
  m_DisplayConstants.m_dwMenuWindowStyles   = WS_BORDER | WS_POPUP;

}



void CXPDisplayClass::PaintMenuItem( HDC dc, const RECT& rcItem, tGRUIMenuItem& MenuItem, bool bPaintBackground )
{

  if ( bPaintBackground )
  {
    if ( ( MenuItem.dwFlags & MF_HILITE )
    &&   ( !( MenuItem.dwFlags & MF_DISABLED ) )
    &&   ( !( MenuItem.dwFlags & MF_SEPARATOR ) ) )
    {
      RECT    rc2;

      rc2 = rcItem;
      rc2.right = m_DisplayConstants.m_iMenuItemIndent;
      FillSolidRect( dc, &rc2, GetSysColor( COLOR_BTNFACE ) );

      rc2 = rcItem;
      rc2.left = m_DisplayConstants.m_iMenuItemIndent;
      FillSolidRect( dc, &rc2, RGB( 255, 255, 255 ) );

      HBRUSH    hBrush = CreateSolidBrush( RGB( 145, 176, 207 ) );
      HBRUSH    hOldBrush = (HBRUSH)SelectObject( dc, hBrush );

      rc2 = rcItem;
      InflateRect( &rc2, -1, -1 );

      HPEN hPen = CreatePen( PS_SOLID, 1, RGB( 38, 87, 137 ) );
      HPEN  hOldPen = (HPEN)SelectObject( dc, hPen );
      Rectangle( dc, rc2.left, rc2.top, rc2.right, rc2.bottom );
      SelectObject( dc, hOldPen );
      DeleteObject( hPen );

      SelectObject( dc, hOldBrush );
      DeleteObject( hBrush );
    }
    else
    {
      RECT    rc2;

      rc2 = rcItem;
      rc2.right = m_DisplayConstants.m_iMenuItemIndent;
      FillSolidRect( dc, &rc2, GetSysColor( COLOR_BTNFACE ) );

      rc2 = rcItem;
      rc2.left = m_DisplayConstants.m_iMenuItemIndent;
      FillSolidRect( dc, &rc2, RGB( 255, 255, 255 ) );
    }

  }

  bool    bSelected = false;

  HFONT   hOldFont = (HFONT)SelectObject( dc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  if ( ( MenuItem.dwFlags & MF_HILITE )
  &&   ( !( MenuItem.dwFlags & MF_DISABLED ) )
  &&   ( !( MenuItem.dwFlags & MF_SEPARATOR ) ) )
  {
    // selektierte Menü-items
    bSelected = true;
    SetTextColor( dc, RGB( 255, 255, 255 ) );
  }
  else
  {
    SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
  }

  if ( MenuItem.dwFlags & MF_SEPARATOR )
  {
    FillSolidRect( dc, m_DisplayConstants.m_iMenuItemIndent + 2,
                   rcItem.top, rcItem.right - rcItem.left - m_DisplayConstants.m_iMenuItemIndent - 4, 1,
                   GetSysColor( COLOR_3DSHADOW ) );
  }
  else
  {
    RECT    rc = rcItem;

    rc.left = m_DisplayConstants.m_iMenuItemIndent;
    if ( MenuItem.dwFlags & MF_DISABLED )
    {
      SetTextColor( dc, RGB( 128, 128, 128 ) );
    }

    int   iTimes = 1;
    if ( MenuItem.dwFlags & MF_DEFAULT )
    {
      iTimes = 2;
    }

    for ( int iT = 0; iT < iTimes; ++iT )
    {
      RECT rcT = rc;
      rcT.left += 4 + iT;
      if ( MenuItem.strItemText.find( '\t' ) != GR::String::npos )
      {
        GR::String   strLeft   = MenuItem.strItemText.substr( 0, MenuItem.strItemText.find( '\t' ) );
        GR::String   strRight  = MenuItem.strItemText.substr( MenuItem.strItemText.find( '\t' ) + 1 );

        DrawText( dc, strLeft.c_str(), (int)strLeft.length(), &rcT, DT_LEFT | DT_SINGLELINE | DT_VCENTER );
        rcT.right -= 20;

        DrawText( dc, strRight.c_str(), (int)strRight.length(), &rcT, DT_RIGHT | DT_SINGLELINE | DT_VCENTER );
      }
      else
      {
        DrawText( dc, MenuItem.strItemText.c_str(), (int)MenuItem.strItemText.length(), &rcT, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER );
      }
    }

    if ( MenuItem.dwFlags & MF_CHECKED )
    {
      RECT    rc = rcItem;
      rc.left = 2;
      rc.right = m_DisplayConstants.m_iMenuItemIndent;
      if ( MenuItem.hiconChecked )
      {
        PaintIcon( dc, rc, MenuItem.hiconChecked );
      }
      else
      {
        PaintIcon( dc, rc, m_hIconCheck );
      }
    }
    else if ( MenuItem.hiconChecked )
    {
      if ( MenuItem.dwFlags & MF_HILITE )
      {
        HICON   hIconGreyed = CWinUtils::CreateGrayscaleIcon( MenuItem.hiconChecked, true );
        PaintIcon( dc, rc, hIconGreyed );
        DestroyIcon( hIconGreyed );
        OffsetRect( &rc, -2, -2 );
        PaintIcon( dc, rc, MenuItem.hiconChecked );
        OffsetRect( &rc, 2, 2 );
      }
      else
      {
        HICON   hIconGreyed = CWinUtils::CreateGrayscaleIcon( MenuItem.hiconChecked );
        PaintIcon( dc, rc, hIconGreyed );
        DestroyIcon( hIconGreyed );
      }
    }
    else if ( MenuItem.hiconUnchecked )
    {
      RECT    rc = rcItem;
      rc.left = 2;
      rc.right = m_DisplayConstants.m_iMenuItemIndent;

      PaintIcon( dc, rc, MenuItem.hiconUnchecked );
    }

    // Popup-Menü - Pfeil nach rechts
    if ( MenuItem.dwFlags & MF_POPUP )
    {
      RECT    rc = rcItem;

      rc.left = rc.right - 24;

      PaintIcon( dc, rc, m_hIconPopupArrow );
    }
  }

  SelectObject( dc, hOldFont );

}



void CXPDisplayClass::PaintButton( HDC hdc, const RECT& rc, const char* szText, bool bMouseOver, bool bPushed, bool bPaintBackground )
{

  if ( bMouseOver )
  {
    HBRUSH    hBrush = CreateSolidBrush( RGB( 255, 0, 255 ) );
    HBRUSH    hOldBrush = (HBRUSH)SelectObject( hdc, hBrush );
    Rectangle( hdc, rc.left, rc.top, rc.right, rc.bottom );
    SelectObject( hdc, hOldBrush );
  }
  if ( bPushed )
  {
    RECT    rc2 = rc;
    InflateRect( &rc2, -2, -2 );
    Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
    InflateRect( &rc2, 2, 2 );
  }

  RECT    rcTemp = rc;

  int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  ::DrawText( hdc, szText, (int)strlen( szText ), &rcTemp, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

  SelectObject( hdc, hOldFont );

  SetBkMode( hdc, iOldBkMode );

}
