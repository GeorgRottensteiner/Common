#include "stdafx.h"

#include <Grafik/GDIPage.h>
#include <Grafik/Image.h>

#include <Misc/Misc.h>

#include <MFC\ImageWnd.h>
#include ".\imagewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL CImageWnd::hasclass = CImageWnd::RegisterMe();



CImageWnd::CImageWnd()
{
  m_pPage       = NULL;
  m_pImage      = NULL;
  m_pOwnedImage = NULL;
}



CImageWnd::~CImageWnd()
{
  SafeDelete( m_pPage );
  SafeDelete( m_pOwnedImage );
}



BEGIN_MESSAGE_MAP(CImageWnd, CWnd)
	//{{AFX_MSG_MAP(CImageWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CImageWnd::OnPaint() 
{
	CPaintDC dc( this );

  RECT    rc;

  GetClientRect( &rc );

  if ( m_pPage == NULL )
  {
    m_pPage = new GR::Graphic::GDIPage();
    m_pPage->Create( m_hWnd, rc.right - rc.left, rc.bottom - rc.top, m_pImage->GetDepth() );
  }
	
  if ( m_pImage )
  {
    m_pPage->Box( 0, 0, rc.right - rc.left, rc.bottom - rc.top, m_pPage->GetRGB256( GetSysColor( COLOR_3DFACE ) ) );
    m_pImage->PutImage( m_pPage, 0, 0 );
  }

  BitBlt( dc.GetSafeHdc(),
          0, 0, m_pPage->GetWidth(), m_pPage->GetHeight(),
          m_pPage->GetDC(), 0, 0, SRCCOPY );
}




BOOL CImageWnd::RegisterMe()
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
  wc.lpszClassName = IMAGEWND_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



void CImageWnd::SetImage( GR::Graphic::Image* pImage )
{
  if ( m_pPage != NULL )
  {
    m_pPage->Box( 0, 0, m_pPage->GetWidth() - 1, m_pPage->GetHeight() - 1, m_pPage->GetRGB256( GetSysColor( COLOR_3DFACE ) ) );
  }
  m_pImage = pImage;
  Invalidate();
}



void CImageWnd::CopyPage( GR::Graphic::GFXPage* pPage )
{
  if ( m_pPage != NULL )
  {
    m_pPage->Box( 0, 0, m_pPage->GetWidth() - 1, m_pPage->GetHeight() - 1, m_pPage->GetRGB256( GetSysColor( COLOR_3DFACE ) ) );
  }
  if ( pPage )
  {
    SafeDelete( m_pOwnedImage );
    GR::Graphic::Image*       m_pOwnedImage = new GR::Graphic::Image( pPage->GetWidth(), pPage->GetHeight(), pPage->GetDepth() );

    m_pOwnedImage->GetImage( pPage, 0, 0 );
  }
  m_pImage = m_pOwnedImage;
  Invalidate();
}



GR::Graphic::Image* CImageWnd::GetImage() const
{
  return m_pImage;
}



BOOL CImageWnd::OnEraseBkgnd(CDC* pDC) 
{
  return 1;
}



void CImageWnd::PreSubclassWindow()
{
  RECT    rc;

  GetClientRect( &rc );

  int   iDepth = 15;

  if ( m_pImage )
  {
    iDepth = m_pImage->GetDepth();
  }
  if ( m_pPage == NULL )
  {
    m_pPage = new GR::Graphic::GDIPage();
    m_pPage->Create( m_hWnd, rc.right - rc.left, rc.bottom - rc.top, iDepth );
  }

  CWnd::PreSubclassWindow();
}
