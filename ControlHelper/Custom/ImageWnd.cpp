// ImageWnd.cpp : implementation file
//

#include <Grafik/GfxPage.h>
#include <Grafik/Image.h>

#include ".\imagewnd.h"



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



void CImageWnd::OnPaint() 
{

  PAINTSTRUCT   ps;

  HDC     hdc = BeginPaint( m_hWnd, &ps );

  RECT    rc;

  GetClientRect( &rc );

  if ( m_pPage == NULL )
  {
    m_pPage = new CWinGPage();
    if ( m_pImage )
    {
      m_pPage->Create( m_hWnd, rc.right - rc.left, rc.bottom - rc.top, m_pImage->GetDepth() );
    }
    else
    {
      m_pPage->Create( m_hWnd, rc.right - rc.left, rc.bottom - rc.top, 16 );
    }
    
  }
	
  if ( m_pImage )
  {
    m_pPage->Box( 0, 0, rc.right - rc.left, rc.bottom - rc.top, m_pPage->GetRGBColorRef( GetSysColor( COLOR_3DFACE ) ) );
    m_pImage->PutImage( m_pPage, 0, 0 );
  }

  BitBlt( hdc,
          0, 0, m_pPage->GetWidth(), m_pPage->GetHeight(),
          m_pPage->GetDC(), 0, 0, SRCCOPY );


  EndPaint( m_hWnd, &ps );
	
}




/*-RegisterMe-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

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
  return RegisterClass(&wc);
}



void CImageWnd::SetImage( GR::CImage* pImage )
{

  if ( m_pPage != NULL )
  {
    m_pPage->Box( 0, 0, m_pPage->GetWidth() - 1, m_pPage->GetHeight() - 1, m_pPage->GetRGBColorRef( GetSysColor( COLOR_3DFACE ) ) );
  }
  m_pImage = pImage;
  Invalidate();

}



void CImageWnd::CopyPage( CGfxPage* pPage )
{

  if ( m_pPage != NULL )
  {
    m_pPage->Box( 0, 0, m_pPage->GetWidth() - 1, m_pPage->GetHeight() - 1, m_pPage->GetRGBColorRef( GetSysColor( COLOR_3DFACE ) ) );
  }
  if ( pPage )
  {
    SafeDelete( m_pOwnedImage );
    GR::CImage*       m_pOwnedImage = new GR::CImage( pPage->GetWidth(), pPage->GetHeight(), pPage->GetDepth() );

    m_pOwnedImage->GetImage( pPage, 0, 0 );
  }
  m_pImage = m_pOwnedImage;
  Invalidate();

}



GR::CImage* CImageWnd::GetImage() const
{

  return m_pImage;

}



void CImageWnd::OnAttach()
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
    m_pPage = new CWinGPage();
    m_pPage->Create( m_hWnd, rc.right - rc.left, rc.bottom - rc.top, iDepth );
  }

  //CWnd::PreSubclassWindow();
}



LRESULT CImageWnd::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_PAINT:
      OnPaint();
      return TRUE;
    case WM_ERASEBKGND:
      return 1;
    case WM_GETDLGCODE:
      return DLGC_WANTALLKEYS;
    case WM_SIZE:
      {
        int   iW = LOWORD( lParam );
        int   iH = HIWORD( lParam );

        if ( ( iW != 0 )
        &&   ( iH != 0 ) )
        {
          if ( m_pPage == NULL )
          {
            m_pPage = new CWinGPage();
            m_pPage->Create( m_hWnd, iW, iH, 15 );
          }

          if ( ( iW != m_pPage->GetWidth() )
          ||   ( iH != m_pPage->GetHeight() ) )
          {
            int   iDepth = m_pPage->GetDepth();
            m_pPage->Destroy();
            m_pPage->Create( m_hWnd, iW, iH, iDepth );
          }
        }
      }
      break;
  }

  return CWnd::WindowProc( uMsg, wParam, lParam );

}



bool CImageWnd::Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT_PTR nID, LPVOID lpParam )
{

  return !!CWnd::CreateEx( WS_EX_CLIENTEDGE, "ImageWnd", "", dwStyle, rect, hwndParent, nID, lpParam );

}