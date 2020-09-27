#include "stdafx.h"

#include "GradientCtrl.h"

#include <MFC/memdc.h>

#include <debug/debugclient.h>
#include ".\gradientctrl.h"


// CGradientCtrl

BOOL CGradientCtrl::hasclass = CGradientCtrl::RegisterMe();
DWORD CGradientCtrl::m_dwNotifyMessage = RegisterWindowMessage( _T( "GR_GRADIENT_CHANGE" ) );



BOOL CGradientCtrl::RegisterMe()
{

  WNDCLASS wc;   
  wc.style = CS_DBLCLKS;                                                 
  wc.lpfnWndProc = ::DefWindowProc; // must be this value
  wc.cbClsExtra = 0;                         
  wc.cbWndExtra = 0;                               
  wc.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon = NULL;     // child window has no icon         
  wc.hCursor = LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;  // no menu                             
  wc.lpszClassName = GRGRADIENTCTRL_CLASS_NAME;

  return AfxRegisterClass(&wc);
}



IMPLEMENT_DYNAMIC(CGradientCtrl, CWnd)
CGradientCtrl::CGradientCtrl() :
  m_dwStyle( GCS_DEFAULT ),
  m_pSelectedPeg( NULL ),
  m_Dragging( false )
{
}

CGradientCtrl::~CGradientCtrl()
{
}


BEGIN_MESSAGE_MAP(CGradientCtrl, CWnd)
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_WM_KILLFOCUS()
  ON_WM_MOUSEMOVE()
  ON_WM_SETFOCUS()
  ON_WM_KEYDOWN()
  ON_WM_GETDLGCODE()
  ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



// CGradientCtrl-Meldungshandler
void CGradientCtrl::OnPaint()
{

  CPaintDC pdc(this);

  RECT    rc;

  GetClientRect( &rc );

  MFCExtension::CMemDC    dc( &pdc, rc );

  // Verlauf
  int   iVerlaufWidth = rc.right;
  int   iVerlaufX1    = 0;

  if ( m_dwStyle & GCS_PEGS_LEFT )
  {
    iVerlaufX1    = 10;
    iVerlaufWidth -= 10;

    // links ausgrauen
    dc.FillSolidRect( 0, 0, 10, rc.bottom, GetSysColor( COLOR_BTNFACE ) );
  }
  if ( m_dwStyle & GCS_PEGS_RIGHT )
  {
    iVerlaufWidth -= 10;

    // rechts ausgrauen
    dc.FillSolidRect( iVerlaufX1 + iVerlaufWidth, 0, 10, rc.bottom, GetSysColor( COLOR_BTNFACE ) );
  }

  for ( int i = 0; i < rc.bottom; ++i )
  {
    DWORD   dwColor = m_Gradient.GetColorAt( i * m_Gradient.Length() / rc.bottom );
    dc.FillSolidRect( iVerlaufX1, i, iVerlaufWidth, 1, RGB( ( dwColor & 0xff0000 ) >> 16, ( dwColor & 0xff00 ) >> 8, ( dwColor & 0xff ) ) );
  }

  // Schwarzer Rand
  {
    RECT    rc2;

    SetRect( &rc2, iVerlaufX1, 0, iVerlaufX1 + iVerlaufWidth, rc.bottom );

    dc.FrameRect( &rc2, &CBrush( (COLORREF)0 ) );
  }

  GR::Graphic::Gradient::tPegs::iterator    it( m_Gradient.m_Pegs.begin() );
  while ( it != m_Gradient.m_Pegs.end() )
  {
    GR::Graphic::Gradient::tPeg&   Peg = *it;

    if ( m_dwStyle & GCS_PEGS_LEFT )
    {
      DrawLeftPeg( dc, Peg );
    }
    if ( m_dwStyle & GCS_PEGS_RIGHT )
    {
      DrawRightPeg( dc, Peg );
    }

    ++it;
  }

  if ( ( GetFocus() == this )
  &&   ( m_pSelectedPeg == NULL ) )
  {
    InflateRect( &rc, -1, -1 );

    dc.DrawFocusRect( &rc );
  }
}



bool CGradientCtrl::GetLeftPegRect( GR::Graphic::Gradient::tPeg& Peg, RECT& rc ) const
{
  GetClientRect( &rc );

  SetRect( &rc, 2, (int)( Peg.Position * rc.bottom / m_Gradient.Length() ) - 3, 6, 6 );
  rc.right = rc.left + 6;
  rc.bottom = rc.top + 6;

  return true;
}



bool CGradientCtrl::GetRightPegRect( GR::Graphic::Gradient::tPeg& Peg, RECT& rc ) const
{
  GetClientRect( &rc );

  SetRect( &rc, rc.right - 8, (int)( Peg.Position * rc.bottom / m_Gradient.Length() ) - 3, 0, 0 );
  rc.right = rc.left + 6;
  rc.bottom = rc.top + 6;

  return true;
}



void CGradientCtrl::DrawLeftPeg( CDC& dc, GR::Graphic::Gradient::tPeg& Peg )
{
  RECT    rc;

  GetLeftPegRect( Peg, rc );

  dc.FillSolidRect( &rc, RGB( ( Peg.Color & 0xff0000 ) >> 16, ( Peg.Color & 0xff00 ) >> 8, Peg.Color & 0xff ) );


  if ( ( m_pSelectedPeg == &Peg )
  &&   ( GetFocus() == this ) )
  {
    dc.DrawFocusRect( &rc );
  }

  InflateRect( &rc, 1, 1 );

  dc.FrameRect( &rc, &CBrush( (COLORREF)0 ) );
}



void CGradientCtrl::DrawRightPeg( CDC& dc, GR::Graphic::Gradient::tPeg& Peg )
{
  RECT    rc;

  GetRightPegRect( Peg, rc );

  dc.FillSolidRect( &rc, RGB( ( Peg.Color & 0xff0000 ) >> 16, ( Peg.Color & 0xff00 ) >> 8, Peg.Color & 0xff ) );

  if ( ( m_pSelectedPeg == &Peg )
  &&   ( GetFocus() == this ) )
  {
    dc.DrawFocusRect( &rc );
  }

  InflateRect( &rc, 1, 1 );

  dc.FrameRect( &rc, &CBrush( (COLORREF)0 ) );
}



void CGradientCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
  GR::Graphic::Gradient::tPeg*    pPeg = PegFromPoint( point );


  m_pSelectedPeg = pPeg;
  if ( m_pSelectedPeg )
  {
    if ( m_pSelectedPeg->Type == GR::Graphic::Gradient::tPeg::PEG_CUSTOM )
    {
      m_Dragging = true;
      SetCapture();
    }
  }

  SetFocus();

  Invalidate();

  CWnd::OnLButtonDown(nFlags, point);
}



BOOL CGradientCtrl::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}



void CGradientCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
  StopDragging();

  CWnd::OnLButtonUp(nFlags, point);
}



void CGradientCtrl::OnKillFocus(CWnd* pNewWnd)
{
  CWnd::OnKillFocus(pNewWnd);

  StopDragging();

  Invalidate();
}



void CGradientCtrl::StopDragging()
{
  m_Dragging = false;
  if ( GetCapture() == this )
  {
    ReleaseCapture();
  }
}



void CGradientCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
  if ( ( m_Dragging )
  &&   ( !( nFlags & MK_LBUTTON ) ) )
  {
    StopDragging();
  }
  if ( m_Dragging )
  {
    RECT    rc;

    GetClientRect( &rc );

    int   iY = point.y;

    if ( iY < rc.top )
    {
      iY = rc.top;
    }
    if ( iY > rc.bottom )
    {
      iY = rc.bottom;
    }

    float   fPos = iY * m_Gradient.Length() / rc.bottom;

    const GR::Graphic::Gradient::tPeg* pPrevPeg = m_Gradient.GetPreviousPeg( m_pSelectedPeg );
    const GR::Graphic::Gradient::tPeg* pNextPeg = m_Gradient.GetNextPeg( m_pSelectedPeg );

    if ( pPrevPeg->Position > fPos )
    {
      fPos = pPrevPeg->Position;
    }
    if ( pNextPeg->Position < fPos )
    {
      fPos = pNextPeg->Position;
    }

    m_pSelectedPeg->Position = fPos;

    if ( GetOwner() )
    {
      GetOwner()->PostMessage( m_dwNotifyMessage );
    }

    Invalidate();
  }

  CWnd::OnMouseMove(nFlags, point);
}



void CGradientCtrl::AddPeg( DWORD dwColor, float fPosition )
{
  // die ersten Pegs sind immer 0 und 100
  if ( m_Gradient.PegCount() == 0 )
  {
    fPosition = 0.0f;
  }
  else if ( m_Gradient.PegCount() == 1 )
  {
    fPosition = 100.0f;
  }
  m_pSelectedPeg = m_Gradient.AddPeg( dwColor, fPosition );

  if ( GetOwner() )
  {
    GetOwner()->PostMessage( m_dwNotifyMessage );
  }

  Invalidate();
}



void CGradientCtrl::OnSetFocus(CWnd* pOldWnd)
{
  CWnd::OnSetFocus(pOldWnd);

  if ( ( m_pSelectedPeg == NULL )
  &&   ( m_Gradient.m_Pegs.size() > 2 ) )
  {

    m_pSelectedPeg = &m_Gradient.m_Pegs[1];
  }
  Invalidate();
}



void CGradientCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

  if ( nChar == VK_TAB )
  {
    if ( m_pSelectedPeg == NULL )
    {
      CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
      return;
    }
  }
  if ( m_pSelectedPeg )
  {
    if ( ( nChar == VK_LEFT )
    ||   ( nChar == VK_UP )
    ||   ( nChar == VK_PRIOR )
    ||   ( nChar == VK_HOME ) )
    {
      if ( m_pSelectedPeg->Type == GR::Graphic::Gradient::tPeg::PEG_CUSTOM )
      {
        RECT    rc;

        GetClientRect( &rc );

        // 1 Pixel als float-Wert
        float   fDelta = 1 * 100.0f / rc.bottom;

        if ( nChar == VK_PRIOR )
        {
          fDelta = 10 * 100.0f / rc.bottom;
        }

        const GR::Graphic::Gradient::tPeg*    pPrevPeg = m_Gradient.GetPreviousPeg( m_pSelectedPeg );

        if ( ( m_pSelectedPeg->Position - fDelta < pPrevPeg->Position )
        ||   ( nChar == VK_HOME ) )
        {
          m_pSelectedPeg->Position = pPrevPeg->Position;
        }
        else
        {
          m_pSelectedPeg->Position -= fDelta;
        }

        if ( GetOwner() )
        {
          GetOwner()->PostMessage( m_dwNotifyMessage );
        }
        Invalidate();
      }
    }
    else if ( ( nChar == VK_RIGHT )
    ||        ( nChar == VK_DOWN )
    ||        ( nChar == VK_NEXT )
    ||        ( nChar == VK_END ) )
    {
      if ( m_pSelectedPeg->Type == GR::Graphic::Gradient::tPeg::PEG_CUSTOM )
      {
        RECT    rc;

        GetClientRect( &rc );

        // 1 Pixel als float-Wert
        float   fDelta = 1 * 100.0f / rc.bottom;

        if ( nChar == VK_NEXT )
        {
          fDelta = 10 * 100.0f / rc.bottom;
        }

        const GR::Graphic::Gradient::tPeg*    pNextPeg = m_Gradient.GetNextPeg( m_pSelectedPeg );

        if ( ( m_pSelectedPeg->Position + fDelta > pNextPeg->Position )
        ||   ( nChar == VK_END ) )
        {
          m_pSelectedPeg->Position = pNextPeg->Position;
        }
        else
        {
          m_pSelectedPeg->Position += fDelta;
        }
        if ( GetOwner() )
        {
          GetOwner()->PostMessage( m_dwNotifyMessage );
        }
        Invalidate();
      }
    }
    else if ( ( nChar == VK_DELETE )
    ||        ( nChar == VK_BACK ) )
    {
      if ( m_pSelectedPeg->Type == GR::Graphic::Gradient::tPeg::PEG_CUSTOM )
      {
        m_Gradient.RemovePeg( m_pSelectedPeg );
        m_pSelectedPeg = NULL;
        if ( GetOwner() )
        {
          GetOwner()->PostMessage( m_dwNotifyMessage );
        }
        Invalidate();
      }
    }
    else if ( nChar == VK_TAB )
    {
      if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
      {
        GR::Graphic::Gradient::tPeg*    pPrevPeg = m_Gradient.GetPreviousPeg( m_pSelectedPeg );
        if ( pPrevPeg->Type == GR::Graphic::Gradient::tPeg::PEG_START )
        {
          GetParent()->GetNextDlgTabItem( this, TRUE )->SetFocus();
          return;
        }
        else
        {
          m_pSelectedPeg = pPrevPeg;
          Invalidate();
          return;
        }
      }
      else
      {
        GR::Graphic::Gradient::tPeg*    pNextPeg = m_Gradient.GetNextPeg( m_pSelectedPeg );
        if ( pNextPeg->Type == GR::Graphic::Gradient::tPeg::PEG_END )
        {
          GetParent()->GetNextDlgTabItem( this )->SetFocus();
          return;
        }
        else
        {
          m_pSelectedPeg = pNextPeg;
          Invalidate();
          return;
        }
      }
    }
  }

  CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}



UINT CGradientCtrl::OnGetDlgCode()
{
  return DLGC_WANTALLKEYS;
}



void CGradientCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  CWnd::OnLButtonDblClk(nFlags, point);
}



GR::Graphic::Gradient::tPeg* CGradientCtrl::PegFromPoint( POINT& point, bool bIncludeStartAndEndPegs )
{
  GR::Graphic::Gradient::tPeg*    pLowestPeg = NULL;
  GR::Graphic::Gradient::tPeg*    pHighestPeg = NULL;

  GR::Graphic::Gradient::tPegs::iterator    it( m_Gradient.m_Pegs.begin() );
  while ( it != m_Gradient.m_Pegs.end() )
  {
    GR::Graphic::Gradient::tPeg&   Peg = *it;

    if ( ( !bIncludeStartAndEndPegs )
    &&   ( Peg.Type != GR::Graphic::Gradient::tPeg::PEG_CUSTOM ) )
    {
      ++it;
      continue;
    }

    if ( m_dwStyle & GCS_PEGS_LEFT )
    {
      RECT    rc;

      GetLeftPegRect( Peg, rc );

      if ( PtInRect( &rc, point ) )
      {
        if ( pLowestPeg == NULL )
        {
          pLowestPeg = &Peg;
        }
        pHighestPeg = &Peg;
      }
    }
    if ( m_dwStyle & GCS_PEGS_RIGHT )
    {
      RECT    rc;

      GetRightPegRect( Peg, rc );

      if ( PtInRect( &rc, point ) )
      {
        if ( pLowestPeg == NULL )
        {
          pLowestPeg = &Peg;
        }
        pHighestPeg = &Peg;
      }
    }

    ++it;
  }

  if ( ( pLowestPeg == NULL )
  &&   ( pHighestPeg == NULL ) )
  {
    return NULL;
  }
  // etwas erwischt
  RECT    rc;

  GetClientRect( &rc );

  // vermeiden, dass alle Pegs ganz oben oder unten festhängen
  if ( point.y < rc.bottom / 2 )
  {
    return pHighestPeg;
  }
  return pLowestPeg;
}