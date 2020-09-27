// IconButton.cpp : implementation file
//

#include "stdafx.h"

#include <GR/GRTypes.h>

#include "iconbutton.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



CIconButton::CIconButton() :
  m_hIcon( NULL ),
  m_bPushed( false )
{
}

CIconButton::~CIconButton()
{
}


BEGIN_MESSAGE_MAP(CIconButton, CWnd)
	//{{AFX_MSG_MAP(CIconButton)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CIconButton message handlers

BOOL CIconButton::hasclass = CIconButton::RegisterMe();



/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

BOOL CIconButton::RegisterMe()
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
  wc.lpszClassName = ICONBUTTON_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



void CIconButton::OnPaint() 
{

	CPaintDC dc( this );

  RECT    rc;

  GetClientRect( &rc );

  if ( m_bPushed )
  {
    DrawFrameControl( dc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED );
  }
  else
  {
    DrawFrameControl( dc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH );
  }

  SetBkMode( dc, TRANSPARENT );
  SetTextColor( dc, GetSysColor( COLOR_BTNTEXT ) );

  int   iTextOffset = 0;
  if ( m_hIcon )
  {
    ICONINFO    ii;
    if ( GetIconInfo( m_hIcon, &ii ) )
    {
      BITMAP bm;
      if ( GetObject( ii.hbmColor, sizeof( BITMAP ), &bm ) )
      {
        int   iWidth = bm.bmWidth,
              iHeight = bm.bmHeight;
        if ( bm.bmHeight == bm.bmWidth * 2 )
        {
          iHeight /= 2;
        }
        iTextOffset = ( rc.bottom - rc.top - iHeight ) / 2;
        if ( m_bPushed )
        {
          DrawIconEx( dc, iTextOffset + 1, iTextOffset + 1, m_hIcon, 0, 0, 0, NULL, DI_NORMAL );
        }
        else
        {
          DrawIconEx( dc, iTextOffset, iTextOffset, m_hIcon, 0, 0, 0, NULL, DI_NORMAL );
        }
      }
      DeleteObject( ii.hbmColor );
      DeleteObject( ii.hbmMask );
    }
  }

  HFONT hOldFont = (HFONT)SelectObject( dc, GetStockObject( DEFAULT_GUI_FONT ) );

  GR::WChar    szBuffer[MAX_PATH];
  GetWindowTextW( szBuffer, MAX_PATH );
  InflateRect( &rc, 2, 2 );
  rc.left += iTextOffset;
  if ( m_bPushed )
  {
    rc.left++;
    rc.top++;
    rc.bottom++;
    rc.right++;
  }

#ifdef UNICODE
  DrawText( dc, szBuffer, (int)wcslen( szBuffer ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
#else
  DrawText( dc, szBuffer, (int)strlen( szBuffer ), &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
#endif

  SelectObject( dc, hOldFont );
	
}



BOOL CIconButton::OnEraseBkgnd( CDC* pDC ) 
{

	return TRUE;

}

void CIconButton::OnLButtonDblClk(UINT nFlags, CPoint point) 
{

	m_bPushed = true;
  SetCapture();
	
	Invalidate( FALSE );

}



void CIconButton::OnLButtonDown(UINT nFlags, CPoint point) 
{

  if ( !m_bPushed )
  {
	m_bPushed = true;
  SetCapture();
    GetOwner()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), BN_PUSHED ), (LPARAM)GetSafeHwnd() );
  }

  Invalidate( FALSE );

  CWnd::OnLButtonDown( nFlags, point );

}



void CIconButton::OnLButtonUp(UINT nFlags, CPoint point) 
{

  if ( m_bPushed )
  {
    m_bPushed = false;
    RECT    rc;
    if ( GetCapture() == this )
    {
    ReleaseCapture();
    }
    GetClientRect( &rc );
    if ( PtInRect( &rc, point ) )
    {
      // Button ist gedrückt!
      GetOwner()->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), BN_CLICKED ), (LPARAM)GetSafeHwnd() );
    }
  }
	
	Invalidate( FALSE );

}



void CIconButton::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{

	CWnd::OnActivate( nState, pWndOther, bMinimized );
	
  if ( nState == WA_INACTIVE )
  {
    if ( m_bPushed )
    {
      m_bPushed = false;
      ReleaseCapture();
      Invalidate( FALSE );
    }
  }
	
}



void CIconButton::SetIcon( HICON icon )
{

  m_hIcon  = icon;
  Invalidate( FALSE );

}