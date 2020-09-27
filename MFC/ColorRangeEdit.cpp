#include "stdafx.h"
#include "ColorRangeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



ColorRangeEdit::ColorRangeEdit() :
  RangeEdit()
{
  m_Minimum  = 0;
  m_Maximum  = 1536;
  m_Step     = 1;

  HWND  hwndDesktop = ::GetDesktopWindow();
  HDC   hdcDesktop = ::GetDC( hwndDesktop );
  HDC   hdc = CreateCompatibleDC( hdcDesktop );
  m_hbmColorBar = CreateCompatibleBitmap( hdcDesktop, 1536, 1 );
  ::ReleaseDC( hwndDesktop, hdcDesktop );

  BITMAP bm;
  GetObject( m_hbmColorBar, sizeof( BITMAP ), &bm );

  SelectObject( hdc, m_hbmColorBar );

  RECT    rcSmall;

  rcSmall.top     = 0;
  rcSmall.bottom  = 1;

  for ( int i = 0; i < 1536; ++i )
  {
    ::SetBkColor( hdc, GetColor( i ) );
    rcSmall.left = i;
    rcSmall.right = i + 1;
    ::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rcSmall, NULL, 0, NULL);
  }

  DeleteDC( hdc );

}

ColorRangeEdit::~ColorRangeEdit()
{
  DeleteObject( m_hbmColorBar );
  RangeEdit::~RangeEdit();
}



BEGIN_MESSAGE_MAP(ColorRangeEdit, RangeEdit)
	//{{AFX_MSG_MAP(ColorRangeEdit)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void ColorRangeEdit::SetRange( int Min, int Max, int Step )
{
  m_Minimum  = 0;
  m_Maximum  = 1536;
  m_Step     = 2;
}



void ColorRangeEdit::SetRange( float Min, float Max, float Step, int DigitsPreComma, int DigitsPostComma )
{
  m_Minimum  = 0;
  m_Maximum  = 1536;
  m_Step     = 2;
}



void ColorRangeEdit::SetPosition( float Pos )
{
}



COLORREF ColorRangeEdit::GetColor( int Pos )
{
  int         colorWorld,
              r,
              g,
              b;


  colorWorld = Pos;
  if ( colorWorld < 0 )
  {
    colorWorld = 0;
  }
  if ( colorWorld >= 1536 )
  {
    colorWorld = 1535;
  }
  r = 0;
  g = 0;
  b = 0;
  if ( ( colorWorld < 256 )
  ||   ( colorWorld >= 1280 ) )
  {
    r = 255;
  }
  if ( ( colorWorld >= 256 )
  &&   ( colorWorld < 2 * 256 ) )
  {
    r = 2 * 256 - 1 - colorWorld;
  }
  if ( ( colorWorld >= 4 * 256 )
  &&   ( colorWorld < 5 * 256 ) )
  {
    r = colorWorld - 4 * 256;
  }
  if ( ( colorWorld >= 256 )
  &&   ( colorWorld < 3 * 256 ) )
  {
    g = 255;
  }
  if ( ( colorWorld >= 3 * 256 )
  &&   ( colorWorld < 4 * 256 ) )
  {
    g = 4 * 256 - 1 - colorWorld;
  }
  if ( ( colorWorld >= 0 )
  &&   ( colorWorld < 256 ) )
  {
    g = colorWorld;
  }

  if ( ( colorWorld >= 3 * 256 )
  &&   ( colorWorld < 5 * 256 ) )
  {
    b = 255;
  }
  if ( ( colorWorld >= 5 * 256 )
  &&   ( colorWorld < 6 * 256 ) )
  {
    b = 6 * 256 - 1 - colorWorld;
  }
  if ( ( colorWorld >= 2 * 256 )
  &&   ( colorWorld < 3 * 256 ) )
  {
    b = colorWorld - 2 * 256;
  }

  return ( b << 16 ) | ( g << 8 ) | r;
}



void ColorRangeEdit::DrawPopup( HDC hdc, const RECT& rcTarget )
{
  HDC   hdcBitmap = CreateCompatibleDC( hdc );

  SelectObject( hdcBitmap, m_hbmColorBar );
  SetStretchBltMode( hdc, COLORONCOLOR );
  StretchBlt( hdc, rcTarget.left, rcTarget.top, rcTarget.right - rcTarget.left, rcTarget.bottom - rcTarget.top,
              hdcBitmap, 0, 0, 1536, 1, SRCCOPY );
  SelectObject( hdcBitmap, NULL );

  int x = 0;
  if ( m_Mode == MODE_INTEGER )
  {
    x = rcTarget.left + ( m_Position - m_Minimum ) * ( rcTarget.right - rcTarget.left - 1 ) / ( m_Maximum - m_Minimum );
  }
  else if ( m_Mode == MODE_FLOAT )
  {
    x = int( rcTarget.left + ( m_PositionF - m_MinimumF ) * ( rcTarget.right - rcTarget.left - 1 ) / ( m_MaximumF - m_MinimumF ) );
  }
  
  RECT    rc = rcTarget;
  rc.left   = x;
  rc.right  = x + 1;
	::SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

  DeleteDC( hdcBitmap );
}
