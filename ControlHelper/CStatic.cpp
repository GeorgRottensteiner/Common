#include <windows.h>

#include "CStatic.h"



namespace WindowsWrapper
{

  CStatic::CStatic() :
    m_bSelfDrawn( false ),
    m_rgbTextColor( GetSysColor( COLOR_WINDOWTEXT ) ),
    m_rgbBackground( GetSysColor( COLOR_3DFACE ) )
  {
  }



  CStatic::~CStatic()
  {
  }



  HICON CStatic::SetIcon( HICON hIcon )
  {
    return (HICON)SendMessage( STM_SETICON, (WPARAM)hIcon, 0 );
  }



  HBITMAP CStatic::SetBitmap( HBITMAP hBitmap )
  {
    return (HBITMAP)SendMessage( STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap );
  }



  BOOL CStatic::PreTranslateMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {

    if ( ( uMsg == WM_ERASEBKGND )
         && ( hWnd == m_hWnd ) )
    {
      return TRUE;
    }
    if ( ( uMsg == WM_PAINT )
         && ( hWnd == m_hWnd ) )
    {
      if ( m_bSelfDrawn )
      {
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
      }
    }

    return FALSE;
  }



  void CStatic::OnPaint( HDC hdc )
  {
    RECT    rc;

    GetClientRect( &rc );

    GR::String     strCaption = GetWindowText();

    DWORD   dwDrawTextStyle = DT_WORDBREAK;

    DWORD   dwStyle = GetStyle();

    if ( dwStyle & SS_CENTER )
    {
      dwDrawTextStyle |= DT_CENTER;
    }

    HFONT   hFontOld = ( HFONT )::SelectObject( hdc, GetFont() );
    ::SetTextColor( hdc, m_rgbTextColor );
    ::SetBkMode( hdc, OPAQUE );
    ::SetBkColor( hdc, m_rgbBackground );

    GR::WString   text = GR::Convert::ToUTF16( strCaption );
    DrawTextW( hdc, text.c_str(), (int)text.length(), &rc, dwDrawTextStyle );

    ::SelectObject( hdc, hFontOld );
  }



  void CStatic::SetSelfDraw( bool bDraw )
  {
    // darf überhaupt self draw sein?
    DWORD   dwStyle = GetStyle();

    dwStyle &= ~0x0f;

    if ( ( dwStyle & SS_BITMAP )
         || ( dwStyle & SS_BLACKFRAME )
         || ( dwStyle & SS_BLACKRECT )
         || ( dwStyle & SS_ENHMETAFILE )
         || ( dwStyle & SS_GRAYFRAME )
         || ( dwStyle & SS_GRAYRECT )
         || ( dwStyle & SS_ICON ) )
    {
      return;
    }

    if ( m_bSelfDrawn != bDraw )
    {
      m_bSelfDrawn = bDraw;
      Invalidate();
    }
  }



  void CStatic::SetTextColor( COLORREF rgbColor )
  {
    m_rgbTextColor = rgbColor;
  }



  HICON CStatic::GetIcon() const
  {
    return ( HICON )::SendMessageW( m_hWnd, STM_GETICON, 0, 0 );
  }

}