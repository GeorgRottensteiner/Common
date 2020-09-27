// LightBall.cpp : implementation file
//

#include "stdafx.h"

#include <GR/GRTypes.h>

#include <MFC\LightBall.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL CLightBall::hasclass = CLightBall::RegisterMe();
DWORD CLightBall::dwLBM = RegisterWindowMessage( LIGHTBALL_MESSAGE );


/////////////////////////////////////////////////////////////////////////////
// CLightBall

CLightBall::CLightBall()
{
}

CLightBall::~CLightBall()
{
}


BEGIN_MESSAGE_MAP(CLightBall, CWnd)
	//{{AFX_MSG_MAP(CLightBall)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLightBall message handlers


/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CLightBall::RegisterMe()
{
  WNDCLASS wc;   
  wc.style = 0;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = NULL;   // we use OnSetCursor                  
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = LIGHTBALL_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



/*-OnPaint--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CLightBall::OnPaint() 
{

	CPaintDC dc( this );

  RECT    rc;

  GetClientRect( &rc );
	
	//dc.FillSolidRect( &rc, 0xffffff );

  float   fSquareRadius = (float)( rc.right - rc.left - 30 ) * (float)( rc.right - rc.left - 30 ) / 4.0f
                        + (float)( rc.bottom - rc.top - 30 ) * (float)( rc.bottom - rc.top - 30 ) / 4.0f,
          fTempSR;


  GR::tVector   vect( m_vectLightDir.x * (float)( rc.right - rc.left ) / 2.0f,
                      -m_vectLightDir.y * (float)( rc.bottom - rc.top ) / 2.0f, 
                      -( rc.bottom - rc.top ) / 2.0f ),
                vectCur;

  vect.normalize();

  for ( int i = rc.left; i < rc.right; i++ )
  {
    for ( int j = rc.top ; j < rc.bottom; j++ )
    {
      fTempSR =   ( (float)i - (float)( rc.right - rc.left ) / 2.0f ) * ( (float)i - (float)( rc.right - rc.left ) / 2.0f ) 
                + ( (float)j - (float)( rc.bottom - rc.top ) / 2.0f ) * ( (float)j - (float)( rc.bottom - rc.top ) / 2.0f );

      if ( fTempSR <= fSquareRadius )
      {
        vectCur.x = ( i - (float)( rc.right - rc.left ) / 2.0f );
        vectCur.y = ( j - (float)( rc.bottom - rc.top ) / 2.0f );

        float   fDummy = vectCur.x * vectCur.x + vectCur.y * vectCur.y;

        vectCur.z = (float)sqrt( fSquareRadius - fDummy );

        vectCur.normalize();

        float fAngle = vectCur.dot( vect );

        fAngle = acosf( fAngle );

        while ( fAngle < 0.0f )
        {
          fAngle += 2 * 3.1415926f;
        }

        fAngle = fAngle / 3.1415926f;

        int iPart = 0;

        iPart = (int)( fAngle * 255.0f );
        if ( iPart > 0xff )
        {
          iPart = 255;
        }

        DWORD dwColor = ( iPart << 16 ) + ( iPart << 8 ) + iPart;

        dc.SetPixel( i, j, dwColor );
      }
    }
  }
	
}



void CLightBall::SetLightDirection( const GR::tVector& vectLightDir )
{

  m_vectLightDir= vectLightDir;

  ValuesChanged();

  Invalidate();

}



void CLightBall::OnMouseMove( UINT nFlags, CPoint point )
{

  if ( nFlags & MK_LBUTTON )
  {
    RECT    rc;

    GetClientRect( &rc );

    float   fAngleX = 180.0f * ( point.x - (float)( rc.right - rc.left ) * 0.5f ) / ( ( rc.right - rc.left ) * 0.5f );
    float   fAngleY = 180.0f * ( point.y - (float)( rc.bottom - rc.top ) * 0.5f ) / ( ( rc.right - rc.left ) * 0.5f );

    GR::tVector   vectLightDir( 0, 0, 1 );

    vectLightDir.RotateY( fAngleX );
    vectLightDir.RotateX( fAngleY );

    SetLightDirection( vectLightDir );
  }
	
	CWnd::OnMouseMove( nFlags, point );

}



BOOL CLightBall::OnEraseBkgnd(CDC* pDC) 
{

	return TRUE;

}



void CLightBall::OnLButtonDown( UINT nFlags, CPoint point )
{

  RECT    rc;

  GetClientRect( &rc );

  float   fAngleX = 180.0f * ( point.x - (float)( rc.right - rc.left ) * 0.5f ) / ( ( rc.right - rc.left ) * 0.5f );
  float   fAngleY = 180.0f * ( point.y - (float)( rc.bottom - rc.top ) * 0.5f ) / ( ( rc.right - rc.left ) * 0.5f );

  GR::tVector   vectLightDir( 0, 0, 1 );

  vectLightDir.RotateX( fAngleX );
  vectLightDir.RotateY( fAngleY );

  SetLightDirection( vectLightDir );

	CWnd::OnLButtonDown(nFlags, point);

}



void CLightBall::ValuesChanged()
{

  CWnd  *wndParent = GetParent();

  if ( wndParent )
  {
    wndParent->SendMessage( dwLBM, (WPARAM)GetSafeHwnd(), 0 );
  }

}
