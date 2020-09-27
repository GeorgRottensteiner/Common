#include "ClassicDisplayClass.h"

#include "GRUIMenu.h"
#include "GRUICmdManager.h"

#pragma comment( lib, "comctl32.lib" )



CClassicDisplayClass::CClassicDisplayClass() :
  m_hIconCheck( NULL ),
  m_hIconPopupArrow( NULL ),
  m_pCmdManager( NULL )
{

  m_DisplayConstants.m_iMenuItemIndent        = 20;
  m_DisplayConstants.m_iMenuItemHeight        = 16;
  m_DisplayConstants.m_iMenuSeparatorHeight   = 4;
  m_DisplayConstants.m_iMenuBorderWidth       = GetSystemMetrics( SM_CXDLGFRAME );
  m_DisplayConstants.m_iMenuBorderHeight      = GetSystemMetrics( SM_CYDLGFRAME );
  m_DisplayConstants.m_iMenuPopupArrowWidth   = 0;
  m_DisplayConstants.m_iMenuPopupArrowHeight  = 0;
  m_DisplayConstants.m_dwMenuWindowStyles     = WS_THICKFRAME | WS_POPUP;

  m_DisplayConstants.m_iCheckWidth            = 0;
  m_DisplayConstants.m_iCheckHeight           = 0;

  m_DisplayConstants.m_iSliderSize            = 20;

  m_DisplayConstants.m_iSizeBorderWidth       = 3;
  m_DisplayConstants.m_iSizeBorderHeight      = 3;
  m_DisplayConstants.m_iCaptionHeight         = 18;
  m_DisplayConstants.m_iSmallCaptionHeight    = 15;
  m_DisplayConstants.m_iClientEdgeWidth       = 2;
  m_DisplayConstants.m_iClientEdgeHeight      = 2;

  m_DisplayConstants.m_iToolBarGripperWidth   = 4;
  m_DisplayConstants.m_iToolBarGripperHeight  = 4;


  HBITMAP   hbmCheck = LoadBitmap( NULL, MAKEINTRESOURCE( 32760 ) );  // OEM_CHECKBOX
  if ( hbmCheck )
  {
		BITMAP    bmInfo;

		GetObject( hbmCheck, sizeof( bmInfo ), &bmInfo );

    m_DisplayConstants.m_iCheckWidth  = bmInfo.bmWidth;
    m_DisplayConstants.m_iCheckHeight = bmInfo.bmHeight;

    m_hIconCheck = CWinUtils::BitmapToIconEx( 0, hbmCheck, RGB( 255, 255, 255 ) );
    DeleteObject( hbmCheck );

  }

  HBITMAP   hbmArrow = LoadBitmap( NULL, MAKEINTRESOURCE( 32739 ) );  // OBM_MNARROW
  if ( hbmArrow )
  {
		BITMAP    bmInfo;

		GetObject( hbmArrow, sizeof( bmInfo ), &bmInfo );

    m_DisplayConstants.m_iMenuPopupArrowWidth  = bmInfo.bmWidth;
    m_DisplayConstants.m_iMenuPopupArrowHeight = bmInfo.bmHeight;

    m_hIconPopupArrow = CWinUtils::BitmapToIconEx( 0, hbmArrow, RGB( 255, 255, 255 ) );
    DeleteObject( hbmArrow );
  }

}



CClassicDisplayClass::~CClassicDisplayClass()
{

  DestroyIcon( m_hIconCheck );
  DestroyIcon( m_hIconPopupArrow );

}



void CClassicDisplayClass::FillSolidRect( HDC hdc, const RECT* pRC, COLORREF clr )
{
	::SetBkColor( hdc, clr );
	RECT    rect;

  SetRect( &rect, pRC->left, pRC->top, pRC->right, pRC->bottom );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}



void CClassicDisplayClass::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
{
	::SetBkColor( hdc, clr );
	RECT    rect;

  SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}



void CClassicDisplayClass::Draw3dRect( HDC dc, int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
{
	FillSolidRect( dc, x, y, cx - 1, 1, clrTopLeft);
	FillSolidRect( dc, x, y, 1, cy - 1, clrTopLeft);
	FillSolidRect( dc, x + cx, y, -1, cy, clrBottomRight);
	FillSolidRect( dc, x, y + cy, cx, -1, clrBottomRight);
}

void CClassicDisplayClass::Draw3dRect( HDC dc, const RECT& rc, COLORREF clrTopLeft, COLORREF clrBottomRight)
{
  Draw3dRect( dc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, clrTopLeft, clrBottomRight );
}



void CClassicDisplayClass::DrawBackground( HDC hdc, const RECT& rcRedraw )
{

  FillRect( hdc, &rcRedraw, GetSysColorBrush( COLOR_APPWORKSPACE ) );

}



void CClassicDisplayClass::PaintIcon( HDC hdc, const RECT& rc, HICON hIcon )
{

  ICONINFO    ii;

  GetIconInfo( hIcon, &ii );

  BITMAP    bm;

  GetObject( ii.hbmColor, sizeof( BITMAP ), &bm );



  DeleteObject( ii.hbmColor );
  DeleteObject( ii.hbmMask );

  ::DrawIconEx( hdc,
          rc.left + ( rc.right - rc.left - bm.bmWidth ) / 2,
          rc.top + ( rc.bottom - rc.top - bm.bmHeight ) / 2,
                hIcon,
                bm.bmWidth,
                bm.bmHeight,
                0,
                0,
                DI_NORMAL );

}



void CClassicDisplayClass::PaintIconSized( HDC hdc, const RECT& rc, HICON hIcon, int iWidth, int iHeight )
{

  ::DrawIconEx( hdc,
          rc.left + ( rc.right - rc.left - iWidth ) / 2,
          rc.top + ( rc.bottom - rc.top - iHeight ) / 2,
                hIcon,
                iWidth,
                iHeight,
                0,
                0,
                DI_NORMAL );

}



void CClassicDisplayClass::PaintBitmap( HDC hdc, const RECT& rc, HBITMAP hbm )
{

  BITMAP      bm;

  GetObject( hbm, sizeof( BITMAP ), &bm );

  HDC         hdc2 = CreateCompatibleDC( hdc );
  SelectObject( hdc2, hbm );

  ::BitBlt( hdc,
            rc.left + ( rc.right - rc.left - bm.bmWidth ) / 2,
            rc.top + ( rc.bottom - rc.top - bm.bmHeight ) / 2,
            bm.bmWidth,
            bm.bmHeight,
            hdc2,
            0, 0,
            SRCCOPY );

  SelectObject( hdc2, NULL );
  DeleteDC( hdc2 );

}



void CClassicDisplayClass::PaintMenuItem( HDC dc, const RECT& rcItem, int iMenuIndention, tGRUIMenuItem& MenuItem, bool bPaintBackground )
{

  if ( bPaintBackground )
  {
    FillSolidRect( dc, &rcItem, GetSysColor( COLOR_BTNFACE ) );
  }

  bool    bSelected = false;

  HFONT   hOldFont = (HFONT)SelectObject( dc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  if ( ( MenuItem.dwFlags & MF_HILITE )
  &&   ( !( MenuItem.dwFlags & MF_SEPARATOR ) ) )
  {
    // selektierte Menü-items
    bSelected = true;
    FillSolidRect( dc, &rcItem, GetSysColor( COLOR_HIGHLIGHT ) );
    SetTextColor( dc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
  }
  else
  {
    FillSolidRect( dc, &rcItem, GetSysColor( COLOR_BTNFACE ) );
    SetTextColor( dc, GetSysColor( COLOR_WINDOWTEXT ) );
  }

  if ( MenuItem.dwFlags & MF_SEPARATOR )
  {
    Draw3dRect( dc, 2, rcItem.top + 1, rcItem.right - rcItem.left - 4, 2, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
  }
  else
  {
    RECT    rc = rcItem;

    rc.left = iMenuIndention;

    int   iTimes = 1;

    if ( MenuItem.dwFlags & MF_DISABLED )
    {
      if ( MenuItem.dwFlags & MF_HILITE )
      {
        SetTextColor( dc, GetSysColor( COLOR_BTNSHADOW ) );
      }
      else
      {
        SetTextColor( dc, GetSysColor( COLOR_WINDOW ) );
        iTimes = 2;
        rc.left++;
        rc.top++;
      }
    }

    if ( MenuItem.dwFlags & MF_DEFAULT )
    {
      iTimes = 2;
    }

    GR::String     strMenuItemText = MenuItem.strItemText;

    if ( ( m_pCmdManager )
    &&   ( m_pCmdManager->IsKnownCommand( MenuItem.dwID ) )
    &&   ( m_pCmdManager->HasShortCut( MenuItem.dwID ) ) )
    {
      strMenuItemText += "\t";
      strMenuItemText += m_pCmdManager->GetShortCutDesc( MenuItem.dwID );
    }

    for ( int iT = 0; iT < iTimes; ++iT )
    {
      RECT rcT = rc;
      rcT.left += iT;
      if ( ( iT == 1 )
      &&   ( MenuItem.dwFlags & MF_DISABLED ) )
      {
        rcT.top--;
        rcT.left -= 1 + iT;
        SetTextColor( dc, GetSysColor( COLOR_BTNSHADOW ) );
      }

      if ( strMenuItemText.find( '\t' ) != GR::String::npos )
      {
        GR::String   strLeft   = strMenuItemText.substr( 0, strMenuItemText.find( '\t' ) );
        GR::String   strRight  = strMenuItemText.substr( strMenuItemText.find( '\t' ) + 1 );

        DrawText( dc, strLeft.c_str(), (int)strLeft.length(), &rcT, DT_LEFT | DT_SINGLELINE | DT_VCENTER );
        rcT.right -= 20;

        DrawText( dc, strRight.c_str(), (int)strRight.length(), &rcT, DT_RIGHT | DT_SINGLELINE | DT_VCENTER );
      }
      else
      {
        DrawText( dc, strMenuItemText.c_str(), (int)strMenuItemText.length(), &rcT, DT_EXPANDTABS | DT_LEFT | DT_SINGLELINE | DT_VCENTER );
      }
    }

    // TODO - Ein Icon, versch. Zustände?
    HICON     hDefIcon = MenuItem.hiconUnchecked;

    if ( ( m_pCmdManager )
    &&   ( m_pCmdManager->IsKnownCommand( MenuItem.dwID ) ) )
    {
      hDefIcon = m_pCmdManager->GetIcon( MenuItem.dwID );
    }


    if ( MenuItem.dwFlags & MF_CHECKED )
    {
      RECT    rc = rcItem;
      rc.left = 2;
      rc.right = iMenuIndention;
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
      RECT    rc = rcItem;
      rc.left = 2;
      rc.right = iMenuIndention;

      PaintIcon( dc, rc, MenuItem.hiconChecked );
    }
    else if ( hDefIcon )
    {
      RECT    rc = rcItem;
      rc.left = 2;
      rc.right = iMenuIndention;

      PaintIcon( dc, rc, hDefIcon );
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



void CClassicDisplayClass::PaintButton( HDC hdc, const RECT& rc, const char* szText, bool bMouseOver, bool bPushed, bool bPaintBackground, bool bFocused )
{

  if ( bMouseOver )
  {
    Rectangle( hdc, rc.left, rc.top, rc.right, rc.bottom );
  }
  if ( bPushed )
  {
    RECT    rc2 = rc;
    DrawEdge( hdc, &rc2, EDGE_SUNKEN, BF_RECT );
  }
  else
  {
    RECT    rc2 = rc;
    DrawEdge( hdc, &rc2, EDGE_RAISED, BF_RECT );
  }

  RECT    rcTemp = rc;

  int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  if ( bPushed )
  {
    OffsetRect( &rcTemp, 1, 1 );
  }

  ::DrawText( hdc, szText, (int)strlen( szText ), &rcTemp, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

  SelectObject( hdc, hOldFont );

  if ( bPushed )
  {
    OffsetRect( &rcTemp, -1, -1 );
  }

  if ( bFocused )
  {
    InflateRect( &rcTemp, -4, -4 );

    DrawFocusRect( hdc, &rcTemp );
  }

  SetBkMode( hdc, iOldBkMode );

}



void CClassicDisplayClass::PaintCheckBox( HDC hdc, const RECT& rc, const char* szText, bool bMouseOver, bool bPushed, bool bChecked, bool bPaintBackground )
{

  /*
  if ( bMouseOver )
  {
    Rectangle( hdc, rc.left, rc.top, rc.right, rc.bottom );
  }
  if ( bPushed )
  {
    RECT    rc2 = rc;
    InflateRect( &rc2, -2, -2 );
    Rectangle( hdc, rc2.left, rc2.top, rc2.right, rc2.bottom );
    InflateRect( &rc2, 2, 2 );
  }
  */

  RECT    rcTemp = rc;

  int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  ::DrawText( hdc, szText, (int)strlen( szText ), &rcTemp, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

  SelectObject( hdc, hOldFont );

  SetBkMode( hdc, iOldBkMode );


  RECT    rcCheck;

  int     iMargin = ( rcTemp.bottom - rcTemp.top - m_DisplayConstants.m_iCheckWidth ) / 2;

  SetRect( &rcCheck,
           iMargin,
           ( rcTemp.bottom - rcTemp.top - m_DisplayConstants.m_iCheckHeight ) / 2,
           iMargin + m_DisplayConstants.m_iCheckWidth,
           iMargin + m_DisplayConstants.m_iCheckHeight );

  if ( bPushed )
  {
    FillSolidRect( hdc, &rcCheck, GetSysColor( COLOR_BTNFACE ) );
  }
  else
  {
    FillSolidRect( hdc, &rcCheck, GetSysColor( COLOR_WINDOW ) );
  }
  DrawEdge( hdc, &rcCheck, EDGE_SUNKEN, BF_RECT );
  if ( bChecked )
  {
    OffsetRect( &rcCheck, 1, -1 );
    PaintIcon( hdc, rcCheck, m_hIconCheck );
  }

}



void CClassicDisplayClass::PaintStatic( HDC hdc, const RECT& rc, const char* szText, bool bPaintBackground )
{

  if ( bPaintBackground )
  {
    FillSolidRect( hdc, &rc, GetSysColor( COLOR_BTNFACE ) );
  }

  RECT    rcTemp = rc;

  int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  ::DrawText( hdc, szText, (int)strlen( szText ), &rcTemp, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

  SelectObject( hdc, hOldFont );

  SetBkMode( hdc, iOldBkMode );

}



void CClassicDisplayClass::PaintSlider( HDC hdc, const RECT& rc, const RECT& rcSlider, bool bMouseOver, bool bMouseOverSlider, bool bDragging, bool bPaintBackground )
{

  if ( bPaintBackground )
  {
    FillSolidRect( hdc, &rc, GetSysColor( COLOR_SCROLLBAR ) );
  }

  if ( bMouseOverSlider )
  {
    FillSolidRect( hdc, &rcSlider, GetSysColor( COLOR_WINDOW ) );
  }
  else
  {
    FillSolidRect( hdc, &rcSlider, GetSysColor( COLOR_BTNFACE ) );
  }
  RECT    rcTemp = rcSlider;
  DrawEdge( hdc, &rcTemp, EDGE_RAISED, BF_RECT );

}



void CClassicDisplayClass::PaintEdit( HDC hdc, const RECT& rcOrig, const GR::String& strText, bool bPaintBackground, bool bFocused, bool bMultiLine, int iFirstVisibleChar, int iSelBegin, int iSelEnd )
{

  if ( bPaintBackground )
  {
    FillSolidRect( hdc, &rcOrig, GetSysColor( COLOR_WINDOW ) );
  }

  DWORD     dwTextFlags = DT_LEFT | DT_NOPREFIX;
  if ( !bMultiLine )
  {
    dwTextFlags |= DT_SINGLELINE;
  }

  RECT    rcTemp = rcOrig;
  DrawEdge( hdc, &rcTemp, EDGE_SUNKEN, BF_RECT );

  InflateRect( &rcTemp, -2, -2 );

  int   iOldBkMode = SetBkMode( hdc, TRANSPARENT );

  HFONT   hOldFont = (HFONT)SelectObject( hdc, (HFONT)GetStockObject( DEFAULT_GUI_FONT ) );

  if ( strText.length() )
  {
    RECT      rc = rcOrig;

    size_t    iLength = strText.length();


    rc.left += 2;
    rc.right -= 2;

    rc.top++;

    int   iTextOffsetInChars = iFirstVisibleChar;

    if ( ( iSelBegin != iSelEnd )
    &&   ( bFocused ) )
    {
      if ( iSelBegin > iSelEnd )
      {
        std::swap( iSelBegin, iSelEnd );
      }

      // linke, nicht selektierte Hälfte darstellen
      if ( iSelBegin > iTextOffsetInChars )
      {
        int   iDisplayLength = iSelBegin - iTextOffsetInChars;

        RECT    rcTemp = rc;

        SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
        SetBkMode( hdc, TRANSPARENT );

        DrawText( hdc, strText.substr( 0, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | dwTextFlags );

        DrawText( hdc, strText.substr( 0, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, dwTextFlags );

        rc.left += rcTemp.right - rcTemp.left;
      }

      // mittleren, selektierten Anteil darstellen
      if ( iSelEnd > iSelBegin )
      {
        int   iDisplayLength = iSelEnd - iSelBegin;

        RECT    rcTemp = rc;

        SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
        SetBkMode( hdc, OPAQUE );

        DrawText( hdc, strText.substr( iSelBegin, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | dwTextFlags );

        DrawText( hdc, strText.substr( iSelBegin, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, DT_NOPREFIX | dwTextFlags );

        rc.left += rcTemp.right - rcTemp.left;
      }

      // rechten, nicht selektierten Anteil darstellen
      if ( (int)strText.length() > iSelEnd )
      {
        int   iDisplayLength = (int)strText.length() - iSelEnd;

        RECT    rcTemp = rc;

        SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
        SetBkMode( hdc, TRANSPARENT );

        DrawText( hdc, strText.substr( iSelEnd, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | dwTextFlags );

        DrawText( hdc, strText.substr( iSelEnd, iDisplayLength ).c_str(), iDisplayLength, &rcTemp, dwTextFlags );

        rc.left += rcTemp.right - rcTemp.left;
      }
    }
    else
    {
      DrawText( hdc, strText.c_str(), (int)iLength, &rc, dwTextFlags );
    }
  }

  SelectObject( hdc, hOldFont );

  SetBkMode( hdc, iOldBkMode );

}



void CClassicDisplayClass::PaintToolBarBack( HDC hdc, const RECT& rc, bool bHorizontal )
{

  FillSolidRect( hdc, &rc, GetSysColor( COLOR_BTNFACE ) );

}



void CClassicDisplayClass::PaintToolBarGripper( HDC hdc, const RECT& rc, bool bHorizontal )
{

  if ( bHorizontal )
  {
    RECT    rc2 = rc;
    rc2.right = 2;
    Draw3dRect( hdc, rc2, GetSysColor( COLOR_3DDKSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
    rc2 = rc;
    rc2.left = 2;
    Draw3dRect( hdc, rc2, GetSysColor( COLOR_3DDKSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
  }
  else
  {
    RECT    rc2 = rc;
    rc2.bottom = 2;
    Draw3dRect( hdc, rc2, GetSysColor( COLOR_3DDKSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
    rc2 = rc;
    rc2.top = 2;
    Draw3dRect( hdc, rc2, GetSysColor( COLOR_3DDKSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
  }

}



void CClassicDisplayClass::PaintToolBarButton( HDC hdc, int iX, int iY, const tToolBarButtonInfo& BtnInfo, HIMAGELIST hImageList, int iButtonIndex )
{

  int     iDX = 0,
          iDY = 0;

  if ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_PUSHED )
  {
    iDX = 1;
    iDY = 1;
  }

  if ( BtnInfo.dwCommandID )
  {
    if ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_DISABLED )
    {
      HICON hIcon = ImageList_ExtractIcon( AfxGetInstanceHandle(), hImageList, iButtonIndex );
      DrawState( hdc,
                  0, 0,
                  (LPARAM)hIcon,
                  0,
                  iX + iDX, iY + iDY,
                  16,
                  15,
                  DST_ICON | DSS_DISABLED );
      DestroyIcon( hIcon );
    }
    else
    {
      ImageList_DrawEx( hImageList, iButtonIndex, hdc, iX + iDX, iY + iDY, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL );
    }
    iButtonIndex++;
  }
  else
  {
    // Separator
    Draw3dRect( hdc, iX + 1, iY, 2, BtnInfo.rcButton.bottom - BtnInfo.rcButton.top, GetSysColor( COLOR_3DDKSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
  }


  if ( BtnInfo.dwCommandID )
  {
    if ( ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_PUSHED )
    ||   ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_CHECKED ) )
    {
      RECT    rc = BtnInfo.rcButton;

      Draw3dRect( hdc, rc, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
    }
    else if ( BtnInfo.dwButtonStyle & tToolBarButtonInfo::BTN_HILIGHTED )
    {
      RECT    rc = BtnInfo.rcButton;

      Draw3dRect( hdc, rc, GetSysColor( COLOR_3DHIGHLIGHT ), GetSysColor( COLOR_3DSHADOW ) );
    }
  }

}



void CClassicDisplayClass::PaintWindowBorder( HDC hdc, const RECT& rc, DWORD dwStyle, DWORD dwExStyle )
{

  if ( ( dwStyle & WS_THICKFRAME )
  ||   ( dwStyle & WS_DLGFRAME ) )
  {
    RECT      rc2 = rc;
    DrawEdge( hdc, &rc2, EDGE_RAISED, BF_RECT );

    InflateRect( &rc2, -2, -2 );
    Draw3dRect( hdc, rc2, GetSysColor( COLOR_BTNFACE ), GetSysColor( COLOR_BTNFACE ) );
  }
  else if ( dwExStyle & WS_EX_CLIENTEDGE )
  {
    RECT      rc2 = rc;
    DrawEdge( hdc, &rc2, EDGE_SUNKEN, BF_RECT );
  }

}



void CClassicDisplayClass::PaintWindowCaption( HDC hdc, const RECT& rc, HWND hWnd, bool bActive )
{

  DWORD     dwFlags = DC_GRADIENT | DC_TEXT | DC_ICON;
  if ( bActive )
  {
    dwFlags |= DC_ACTIVE;
  }
  if ( GetWindowLong( hWnd, GWL_EXSTYLE ) & WS_EX_TOOLWINDOW )
  {
    dwFlags |= DC_SMALLCAP;
  }
  DrawCaption( hWnd, hdc, &rc, dwFlags );

}



void CClassicDisplayClass::PaintWindowCaptionCloseButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive )
{

  RECT    rcTemp = rc;
  DrawFrameControl( hdc, &rcTemp, DFC_CAPTION, DFCS_CAPTIONCLOSE | ( bActive ? 0 : DFCS_INACTIVE ) | ( bPushed ? DFCS_PUSHED : 0 ) );

}



void CClassicDisplayClass::PaintWindowCaptionMaximizeButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive, bool bIsMaximized )
{

  RECT    rcTemp = rc;
  DrawFrameControl( hdc, &rcTemp, DFC_CAPTION, ( bIsMaximized ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMAX ) | ( bPushed ? DFCS_PUSHED : 0 ) | ( bActive ? 0 : DFCS_INACTIVE ) );

}



void CClassicDisplayClass::PaintWindowCaptionMinimizeButton( HDC hdc, const RECT& rc, DWORD dwStyle, bool bPushed, bool bActive, bool bIsMinimized )
{

  RECT    rcTemp = rc;
  DrawFrameControl( hdc, &rcTemp, DFC_CAPTION, ( bIsMinimized ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMIN ) | ( bActive ? 0 : DFCS_INACTIVE ) | ( bPushed ? DFCS_PUSHED : 0 ) );

}



void CClassicDisplayClass::PaintDockBarBackground( HDC hdc, const RECT& rc )
{

  FillSolidRect( hdc, &rc, GetSysColor( COLOR_BTNFACE ) );

}



void CClassicDisplayClass::PaintMenuBarItem( HDC hdc, bool bHorizontal, const tItemInfo& ItemInfo )
{

  int   iX = 0;
  int   iY = 0;

  int   iCX = ( ItemInfo.rcButton.right - ItemInfo.rcButton.left - 16 ) / 2;
  int   iCY = ( ItemInfo.rcButton.bottom - ItemInfo.rcButton.top - 15 ) / 2;

  if ( ( ItemInfo.dwButtonStyle & tItemInfo::IF_PUSHED )
  ||   ( ItemInfo.dwButtonStyle & tItemInfo::IF_CHECKED ) )
  {
    iCX++;
    iCY++;
  }

  SetBkMode( hdc, TRANSPARENT );

  if ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUBTN )
  {
    DWORD   dwExtraStyle = 0;
    if ( ItemInfo.dwButtonStyle & tItemInfo::IF_PUSHED )
    {
      dwExtraStyle = DFCS_PUSHED;
    }
    RECT    rc = ItemInfo.rcButton;

    HFONT   hFontTemp = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
    HFONT   hOldFont = (HFONT)SelectObject( hdc, hFontTemp );

    SetTextAlign( hdc, TA_TOP );

    switch ( ItemInfo.dwCommandID )
    {
      case SC_MINIMIZE:
        DrawFrameControl( hdc, &rc, DFC_CAPTION, dwExtraStyle | DFCS_CAPTIONMIN );
        break;
      case SC_RESTORE:
        DrawFrameControl( hdc, &rc, DFC_CAPTION, dwExtraStyle | DFCS_CAPTIONRESTORE );
        break;
      case SC_CLOSE:
        DrawFrameControl( hdc, &rc, DFC_CAPTION, dwExtraStyle | DFCS_CAPTIONCLOSE );
        break;
    }

    SelectObject( hdc, hOldFont );

    if ( bHorizontal )
    {
      iX += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
    }
    else
    {
      iY += ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
    }
    return;
  }
  else if ( ItemInfo.dwButtonStyle & tItemInfo::IF_SYSMENUICON )
  {
    /*
    if ( m_hwndMaximizedMDIChild )
    {
      HICON hIcon = (HICON)(DWORD)GetClassLongPtr( m_hwndMaximizedMDIChild, GCL_HICONSM );

      int   iWidth  = GetSystemMetrics( SM_CXSMICON );
      int   iHeight = GetSystemMetrics( SM_CYSMICON );

      if ( hIcon )
      {
        DrawIconEx( hdc, ItemInfo.rcButton.left + ( 23 - iWidth ) / 2, ItemInfo.rcButton.top + ( 23 - iHeight ) / 2,
                    hIcon, 0, 0, 0, 0, DI_NORMAL );
      }
    }
    */
    if ( bHorizontal )
    {
      iX += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
    }
    else
    {
      iY += ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
    }
    return;
  }

  if ( bHorizontal )
  {
    if ( ItemInfo.hIcon )
    {
      if ( ItemInfo.dwButtonStyle & tItemInfo::IF_DISABLED )
      {
        DrawState( hdc,
                    0, 0,
                    (LPARAM)ItemInfo.hIcon,
                    0,
                    iX + iCX, iY + iCY,
                    16,
                    15,
                    DST_ICON | DSS_DISABLED );
      }
      else
      {
        DrawState( hdc,
                    0, 0,
                    (LPARAM)ItemInfo.hIcon,
                    0,
                    iX + iCX, iY + iCY,
                    16,
                    15,
                    DST_ICON );
      }
    }

    RECT    rcText = ItemInfo.rcButton;

    OffsetRect( &rcText, 8, 4 );

    if ( ItemInfo.dwButtonStyle & tItemInfo::IF_DISABLED )
    {
      SetTextColor( hdc, GetSysColor( COLOR_WINDOW ) );
      OffsetRect( &rcText, 1, 1 );
      DrawText( hdc, ItemInfo.strText.c_str(), (int)ItemInfo.strText.length(), &rcText, DT_SINGLELINE );
      SetTextColor( hdc, GetSysColor( COLOR_3DSHADOW ) );
      OffsetRect( &rcText, -1, -1 );
      DrawText( hdc, ItemInfo.strText.c_str(), (int)ItemInfo.strText.length(), &rcText, DT_SINGLELINE );
    }
    else
    {
      if ( ItemInfo.dwButtonStyle & tItemInfo::IF_PUSHED )
      {
        OffsetRect( &rcText, 1, 1 );
      }
      DrawText( hdc, ItemInfo.strText.c_str(), (int)ItemInfo.strText.length(), &rcText, DT_SINGLELINE );
    }

    iX += ItemInfo.rcButton.right - ItemInfo.rcButton.left;
  }
  else
  {
    if ( ItemInfo.hIcon )
    {
      if ( ItemInfo.dwButtonStyle & tItemInfo::IF_DISABLED )
      {
        DrawState( hdc,
                    0, 0,
                    (LPARAM)ItemInfo.hIcon,
                    0,
                    iX + iCX, iY + iCY,
                    16,
                    15,
                    DST_ICON | DSS_DISABLED );
      }
      else
      {
        DrawState( hdc,
                    0, 0,
                    (LPARAM)ItemInfo.hIcon,
                    0,
                    iX + iCX, iY + iCY,
                    16,
                    15,
                    DST_ICON );
      }
    }
    RECT    rcText = ItemInfo.rcButton;

    SetTextAlign( hdc, TA_BASELINE );

    OffsetRect( &rcText, 4, 8 );

    GR::String     strVertText = "";

    for ( size_t j = 0; j < ItemInfo.strText.length(); ++j )
    {
      if ( ItemInfo.strText[j] == '&' )
      {
        strVertText += ItemInfo.strText[j + 1];
        ++j;
        continue;
      }
      strVertText += ItemInfo.strText[j];
    }
    if ( ItemInfo.dwButtonStyle & tItemInfo::IF_DISABLED )
    {
      SetTextColor( hdc, GetSysColor( COLOR_WINDOW ) );
      OffsetRect( &rcText, 1, 1 );
      DrawText( hdc, strVertText.c_str(), (int)strVertText.length(), &rcText, DT_SINGLELINE );
      SetTextColor( hdc, GetSysColor( COLOR_3DSHADOW ) );
      OffsetRect( &rcText, -1, -1 );
      DrawText( hdc, strVertText.c_str(), (int)strVertText.length(), &rcText, DT_SINGLELINE );
    }
    else
    {
      if ( ItemInfo.dwButtonStyle & tItemInfo::IF_PUSHED )
      {
        OffsetRect( &rcText, 1, 1 );
      }
      DrawText( hdc, strVertText.c_str(), (int)strVertText.length(), &rcText, DT_SINGLELINE | DT_NOPREFIX );
    }

    iY += ItemInfo.rcButton.bottom - ItemInfo.rcButton.top;
  }

  if ( ( ItemInfo.dwButtonStyle & tItemInfo::IF_PUSHED )
  ||   ( ItemInfo.dwButtonStyle & tItemInfo::IF_CHECKED ) )
  {
    RECT    rc = ItemInfo.rcButton;

    Draw3dRect( hdc, rc, GetSysColor( COLOR_3DSHADOW ), GetSysColor( COLOR_3DHIGHLIGHT ) );
  }
  else if ( ItemInfo.dwButtonStyle & tItemInfo::IF_HILIGHTED )
  {
    RECT    rc = ItemInfo.rcButton;

    Draw3dRect( hdc, rc, GetSysColor( COLOR_3DHIGHLIGHT ), GetSysColor( COLOR_3DSHADOW ) );
  }

}



void CClassicDisplayClass::SetCommandManager( GRUICmdManager* pCmdManager )
{

  m_pCmdManager = pCmdManager;

}