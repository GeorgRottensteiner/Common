// AngleWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AngleWnd.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL CAngleWnd::hasclass = CAngleWnd::RegisterMe();



/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CAngleWnd::RegisterMe()
{
  WNDCLASS wc;   
  wc.style = 0;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = ANGLEWND_CLASS_NAME;

  return AfxRegisterClass(&wc);
}

/////////////////////////////////////////////////////////////////////////////
// CAngleWnd

CAngleWnd::CAngleWnd()
{

  m_fAngle = 0.0f;

}

CAngleWnd::~CAngleWnd()
{
}


BEGIN_MESSAGE_MAP(CAngleWnd, CWnd)
	//{{AFX_MSG_MAP(CAngleWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
  ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAngleWnd 

void CAngleWnd::OnPaint() 
{

	CPaintDC olddc( this ); // device context for painting

  // flimmerfreies Zeichnen
  CDC     *dc = new CDC();
  CRect   rectMem;
  CBitmap   m_bitmapMem;
  CBitmap * m_oldBitmap;

  dc->CreateCompatibleDC( &olddc );
	olddc.GetClipBox( &rectMem );
	m_bitmapMem.CreateCompatibleBitmap( &olddc, rectMem.Width(), rectMem.Height() );
	m_oldBitmap = dc->SelectObject( &m_bitmapMem );
	dc->SetWindowOrg( rectMem.left, rectMem.top );

  
  RECT    rc;
  GetClientRect( &rc );
  dc->FillSolidRect( &rc, GetSysColor( COLOR_3DFACE ) );

  InflateRect( &rc, -4, -4 );

  CPen    *penOld,
          *penWhite = new CPen( PS_SOLID, 1, 0xffffff ),
          *penBlack = new CPen( PS_SOLID, 1, (COLORREF)0 );

  // Kreis-Form
  POINT   ptOben;

  ptOben.x = rc.right - 4;
  ptOben.y = rc.top - 4;
  penOld = dc->SelectObject( penWhite );
  dc->Arc( &rc, 
           ptOben,
           (POINT)CPoint( rc.left + 4, 
                   rc.bottom - 4 ) );
  dc->SelectObject( penBlack );
  dc->Arc( &rc, 
          CPoint( rc.left + 4, rc.bottom - 4 ),
          CPoint( rc.right - 4, rc.top - 4 ) );

  // Zeiger
  InflateRect( &rc, 4, 4 );
  dc->SelectObject( penOld );
  delete penBlack;

  LOGBRUSH    lg;

  lg.lbColor = 0;
  lg.lbHatch = 0;
  lg.lbStyle = BS_SOLID;
  penBlack = new CPen( PS_GEOMETRIC | PS_ENDCAP_ROUND, 4, &lg, 0, NULL );
  penOld = dc->SelectObject( penBlack );
  dc->MoveTo( ( rc.right - rc.left ) / 2, ( rc.bottom - rc.top ) / 2 );
  dc->LineTo( ( rc.right - rc.left ) / 2 + (int)( ( ( rc.right - rc.left ) / 2 - 8 ) * cos( m_fAngle * 3.1415926 / 180.0f ) ),
             ( rc.bottom - rc.top ) / 2 - (int)( ( ( rc.bottom - rc.top ) / 2 - 8 ) * sin( m_fAngle * 3.1415926 / 180.0f ) ) );

  dc->SelectObject( penOld );
  delete penBlack;
  delete penWhite;


  // Doppelpuffer zurückkopieren
	olddc.BitBlt( rectMem.left, rectMem.top, rectMem.Width(), rectMem.Height(),
		           dc, rectMem.left, rectMem.top, SRCCOPY );
	dc->SelectObject( m_oldBitmap );
  delete dc;

}

void CAngleWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{

  RECT    rc;
  GetClientRect( &rc );

  if ( ( point.x == ( rc.right - rc.left ) / 2 )
  &&   ( point.y == ( rc.bottom - rc.top ) / 2 ) )
  {
    m_fAngle = 0.0f;
  }
  else
  {
    m_fAngle = (float)( atan2f( (float)( ( rc.bottom - rc.top ) / 2 - point.y ), 
                                (float)( point.x - ( rc.right - rc.left ) / 2 ) ) * 180.0f ) / 3.1415926f;
  }
  while ( m_fAngle < 0.0f )
  {
    m_fAngle += 360.0f;
  }
  while ( m_fAngle > 360.0f )
  {
    m_fAngle -= 360.0f;
  }
  Invalidate( FALSE );

  SetCapture();
	CWnd::OnLButtonDown( nFlags, point );

  if ( GetParent() )
  {
    NMHDR   nmHdr;

    nmHdr.code      = EN_CHANGE;
    nmHdr.hwndFrom  = GetSafeHwnd();
    nmHdr.idFrom    = GetDlgCtrlID();

    GetParent()->SendMessage( WM_NOTIFY, (WPARAM)nmHdr.idFrom, (LPARAM)&nmHdr );
  }

}

void CAngleWnd::OnMouseMove(UINT nFlags, CPoint point) 
{

  if ( nFlags & MK_LBUTTON )
  {
    OnLButtonDown( nFlags, point );
  }
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CWnd::OnMouseMove(nFlags, point);
}

void CAngleWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{

	ReleaseCapture();
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CAngleWnd::OnKillFocus(CWnd* pNewWnd)
{
  CWnd::OnKillFocus(pNewWnd);

  if ( GetCapture() == this )
  {
    ReleaseCapture();
  }

}



float CAngleWnd::GetAngle() const
{

  return m_fAngle;

}


  
void CAngleWnd::SetAngle( float fAngle )
{

  m_fAngle = fAngle;
  while ( m_fAngle < 0.0f )
  {
    m_fAngle += 360.0f;
  }
  while ( m_fAngle > 360.0f )
  {
    m_fAngle -= 360.0f;
  }

  Invalidate();

  if ( GetParent() )
  {
    NMHDR   nmHdr;

    nmHdr.code      = EN_CHANGE;
    nmHdr.hwndFrom  = GetSafeHwnd();
    nmHdr.idFrom    = GetDlgCtrlID();

    GetParent()->SendMessage( WM_NOTIFY, (WPARAM)nmHdr.idFrom, (LPARAM)&nmHdr );
  }

}
