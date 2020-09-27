// e:\projekte\common\mfc\ColoredButton.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "ColoredButton.h"


// CColoredButton

IMPLEMENT_DYNAMIC(CColoredButton, CButton)
CColoredButton::CColoredButton()
{

  m_rgbTextColor      = GetSysColor( COLOR_BTNTEXT );

}

CColoredButton::~CColoredButton()
{
}


BEGIN_MESSAGE_MAP(CColoredButton, CButton)
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CColoredButton-Meldungshandler


BOOL CColoredButton::OnEraseBkgnd( CDC* pDC )
{

  RECT    rc;

  GetClientRect( &rc );
  pDC->FillSolidRect( &rc, m_rgbBackground );
  return TRUE;

}

void CColoredButton::SetBackground( COLORREF rgbColor, COLORREF rgbDisabledColor )
{

  m_rgbBackground         = rgbColor;
  m_rgbDisabledBackground = rgbDisabledColor;
  Invalidate();

}


void CColoredButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{


  CString       cstrCaption;
  GetWindowText( cstrCaption );

  RECT          rcItem,
                rcFocus,
                rcText,
                rcOffsetText;

  GetClientRect( &rcItem );

	CDC		*dc;
	UINT	state;
	
	dc    = CDC::FromHandle( lpDrawItemStruct->hDC );
	state = lpDrawItemStruct->itemState;

  rcFocus       = rcItem;
  rcText        = rcItem;
  rcOffsetText  = rcItem;

  OffsetRect( &rcText, -1, -1 );

  InflateRect( &rcFocus, -4, -4 );
      
	if ( state & ODS_DISABLED )
  {
    dc->FillSolidRect( &rcItem, m_rgbDisabledBackground );
	}
	else
	{
		dc->FillSolidRect( &rcItem, m_rgbBackground );
	}

	if ( state & ODS_SELECTED )
	{ 
    dc->DrawEdge( &rcItem, EDGE_SUNKEN, BF_RECT );
	}
	else
	{
    dc->DrawEdge( &rcItem, EDGE_RAISED, BF_RECT );
	}

	if ( state & ODS_DISABLED )
	{
    dc->SetBkMode( TRANSPARENT );
    dc->SetTextColor( GetSysColor( COLOR_3DHILIGHT ) );
    dc->DrawText( cstrCaption, &rcOffsetText, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
    dc->SetTextColor( GetSysColor( COLOR_3DSHADOW ) );
    dc->DrawText( cstrCaption, &rcText, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
  }
	else
	{
		if ( state & ODS_SELECTED )
		{
      dc->DrawText( cstrCaption, &rcOffsetText, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
		}
		else
		{
      dc->DrawText( cstrCaption, &rcText, DT_SINGLELINE | DT_VCENTER | DT_CENTER );
		}
	}

	if ( state & ODS_FOCUS )
	{
		DrawFocusRect( lpDrawItemStruct->hDC, &rcFocus );
	}

} 
