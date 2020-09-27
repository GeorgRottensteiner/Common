// TransparentStatic.cpp : Implementierungsdatei
//

#include "TransparentStatic.h"


// CTransparentStatic

CTransparentStatic::CTransparentStatic() :
  m_hwndBackground( NULL )
{
}

CTransparentStatic::~CTransparentStatic()
{
}


// CTransparentStatic-Meldungshandler

void CTransparentStatic::OnAttach()
{

  ModifyStyleEx( 0, WS_EX_TRANSPARENT, 0 );

  m_strText = GetWindowText();

  m_hwndBackground = GetParent();

}



void CTransparentStatic::SetAlternateBackgroundWnd( HWND hWnd )
{

  m_hwndBackground = hWnd;
  Invalidate();

}



void CTransparentStatic::OnPaint()
{

  PAINTSTRUCT   ps;

  HDC hdc = BeginPaint( m_hWnd, &ps );

  DoPaint( hdc );

  EndPaint( m_hWnd, &ps );

}



void CTransparentStatic::SetWindowText( LPCTSTR lpszString )
{

  m_strText = lpszString;

  RECT    rc2;

  GetWindowRect( &rc2 );

  POINT   pt;

  pt.x = rc2.left;
  pt.y = rc2.top;
  ::ScreenToClient( m_hwndBackground, &pt );
  OffsetRect( &rc2, -pt.x, -pt.y );

  ::RedrawWindow( m_hwndBackground, &rc2, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );

  RedrawWindow( m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );

}

void CTransparentStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct )
{

  DoPaint( lpDrawItemStruct->hDC );

}



void CTransparentStatic::DoPaint( HDC hdc )
{

  if ( ( GetStyle() & SS_ICON ) == SS_ICON )
  {
    DrawIconEx( hdc, 0, 0, GetIcon(), 0, 0, 0, 0, DI_NORMAL );
    return;
  }
  HFONT   hOldFont = (HFONT)SelectObject( hdc, GetFont() );

  RECT      rc;
  GetClientRect( &rc );

#define MAP_STYLE(src, dest) if(dwStyle & (src)) dwText |= (dest)
#define NMAP_STYLE(src, dest) if(!(dwStyle & (src))) dwText |= (dest)

	DWORD dwStyle = GetStyle(), dwText = 0;

  MAP_STYLE(	SS_LEFT,			DT_LEFT );
	MAP_STYLE(	SS_RIGHT,			DT_RIGHT					);
	MAP_STYLE(	SS_CENTER,			DT_CENTER					);
	MAP_STYLE(	SS_CENTERIMAGE,		DT_VCENTER | DT_SINGLELINE	);
	MAP_STYLE(	SS_NOPREFIX,		DT_NOPREFIX					);
	MAP_STYLE(	SS_WORDELLIPSIS,	DT_WORD_ELLIPSIS			);
	MAP_STYLE(	SS_ENDELLIPSIS,		DT_END_ELLIPSIS				);
	MAP_STYLE(	SS_PATHELLIPSIS,	DT_PATH_ELLIPSIS			);

	NMAP_STYLE(	SS_LEFTNOWORDWRAP |
				SS_CENTERIMAGE |
				SS_WORDELLIPSIS |
				SS_ENDELLIPSIS |
				SS_PATHELLIPSIS,	DT_WORDBREAK				);

	// Set transparent background
	SetBkMode( hdc, TRANSPARENT );

	// Draw the text
  DrawText( hdc, m_strText.c_str(), (int)m_strText.length(), &rc, dwText );

  SelectObject( hdc, hOldFont );

}



LRESULT CTransparentStatic::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
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