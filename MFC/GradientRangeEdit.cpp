#include "stdafx.h"
#include "GradientRangeEdit.h"

#include <Grafik/GFXHelper.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CGradientRangeEdit::CGradientRangeEdit() :
  RangeEdit()
{
  m_Minimum  = 0;
  m_Maximum  = 255;
  m_Step     = 1;

  SetColors( 0xff000000, 0xffffffff );
}



CGradientRangeEdit::~CGradientRangeEdit()
{
  DeleteObject( m_hbmColorBar );
}


BEGIN_MESSAGE_MAP(CGradientRangeEdit, RangeEdit)
	//{{AFX_MSG_MAP(CGradientRangeEdit)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CGradientRangeEdit::SetRange( int Min, int Max, int Step )
{
  m_Minimum  = 0;
  m_Maximum  = 255;
  m_Step     = 1;
}



void CGradientRangeEdit::SetRange( float Min, float Max, float Step, int DigitsPreComma, int DigitsPostComma )
{
  m_Minimum  = 0;
  m_Maximum  = 255;
  m_Step     = 1;
}



void CGradientRangeEdit::SetPosition( float Pos )
{
}



COLORREF CGradientRangeEdit::GetColor( int Pos )
{
  DWORD   color   = GFX::ColorGradient( m_ColorGradient1, m_ColorGradient2, Pos * 100 / 255 );

  return RGB( ( color & 0xff0000 ) >> 16,
              ( color & 0x00ff00 ) >> 8,
              ( color & 0x0000ff ) );
}



void CGradientRangeEdit::DrawPopup( HDC hdc, const RECT& rcTarget )
{
  HDC   hdcBitmap = CreateCompatibleDC( hdc );

  SelectObject( hdcBitmap, m_hbmColorBar );
  SetStretchBltMode( hdc, COLORONCOLOR );
  StretchBlt( hdc, rcTarget.left, rcTarget.top, rcTarget.right - rcTarget.left, rcTarget.bottom - rcTarget.top,
              hdcBitmap, 0, 0, 256, 1, SRCCOPY );
  SelectObject( hdcBitmap, NULL );

  int x = 0;
  if ( m_Mode == MODE_INTEGER )
  {
    x = rcTarget.left + ( m_Position - m_Minimum ) * ( rcTarget.right - rcTarget.left - 1 ) / ( m_Maximum - m_Minimum );
  }
  else if ( m_Mode == MODE_FLOAT )
  {
    x = int( rcTarget.left + ( m_Position - m_Minimum ) * ( rcTarget.right - rcTarget.left - 1 ) / ( m_Maximum - m_Minimum ) );
  }
  
  RECT    rc = rcTarget;
  rc.left   = x;
  rc.right  = x + 1;
	::SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

  DeleteDC( hdcBitmap );
}



void CGradientRangeEdit::SetColors( GR::u32 Color1, GR::u32 Color2 )
{
  m_ColorGradient1 = Color1;
  m_ColorGradient2 = Color2;

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

  for ( int i = 0; i < 256; ++i )
  {
    ::SetBkColor( hdc, GetColor( i ) );
    rcSmall.left = i;
    rcSmall.right = i + 1;
    ::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rcSmall, NULL, 0, NULL);
  }

  DeleteDC( hdc );
}