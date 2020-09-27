// TransparentStatic.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "TransparentStatic.h"


// CTransparentStatic

IMPLEMENT_DYNAMIC(CTransparentStatic, CStatic)
CTransparentStatic::CTransparentStatic() :
  m_pWndBackground( NULL )
{
}

CTransparentStatic::~CTransparentStatic()
{
}


BEGIN_MESSAGE_MAP(CTransparentStatic, CStatic)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_CREATE()
END_MESSAGE_MAP()



// CTransparentStatic-Meldungshandler

void CTransparentStatic::PreSubclassWindow()
{

  ModifyStyleEx( 0, WS_EX_TRANSPARENT, 0 );

  GetWindowText( m_strText );

  m_pWndBackground = GetParent();

}



void CTransparentStatic::SetAlternateBackgroundWnd( CWnd* pWndBackground )
{

  m_pWndBackground = pWndBackground;
  Invalidate();

}



BOOL CTransparentStatic::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void CTransparentStatic::OnPaint()
{

  /*
  RECT    rc2;

  GetClientRect( &rc2 );
  ClientToScreen( &rc2 );
  m_pWndBackground->ScreenToClient( &rc2 );

  //m_pWndBackground->InvalidateRect( &rc2 );
  //m_pWndBackground->UpdateWindow();
  m_pWndBackground->RedrawWindow( &rc2 );
  */


  CPaintDC    dc( this );

  DoPaint( &dc );

}

int CTransparentStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CStatic::OnCreate(lpCreateStruct) == -1)
    return -1;


  return 0;
}



void CTransparentStatic::SetWindowText( LPCTSTR lpszString )
{

  m_strText = lpszString;

  RECT    rc2;

  GetClientRect( &rc2 );
  ClientToScreen( &rc2 );
  m_pWndBackground->ScreenToClient( &rc2 );

  m_pWndBackground->InvalidateRect( &rc2 );
  m_pWndBackground->UpdateWindow();

}

void CTransparentStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct )
{

  CDC*    pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

  DoPaint( pDC );

}



void CTransparentStatic::DoPaint( CDC* pDC )
{

  if ( ( GetStyle() & SS_ICON ) == SS_ICON )
  {
    DrawIconEx( pDC->GetSafeHdc(), 0, 0, GetIcon(), 0, 0, 0, 0, DI_NORMAL );
    return;
  }
  CFont* pOldFont = pDC->SelectObject( GetFont() );

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
	pDC->SetBkMode( TRANSPARENT );

	// Draw the text
	pDC->DrawText( m_strText, &rc, dwText );

  pDC->SelectObject( pOldFont );

}
