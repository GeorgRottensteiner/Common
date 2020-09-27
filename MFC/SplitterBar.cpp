#include "stdafx.h"
#include "SplitterBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNAMIC(SplitterBar, CWnd)

BEGIN_MESSAGE_MAP(SplitterBar, CWnd)
	//{{AFX_MSG_MAP(SplitterBar)
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



SplitterBar::SplitterBar()
{
	m_Dragging     = false ;
	m_pwndLeftPane = m_pwndRightPane = NULL;

  m_DragImmediately = false;
}



SplitterBar::~SplitterBar()
{
}



BOOL SplitterBar::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, BOOL Horizontal )
{
	CWnd*   pWnd = this;
	m_Horizontal = !!Horizontal;

	return pWnd->CreateEx( 0, NULL, _T( "" ), dwStyle, rect, pParentWnd, nID);
}



void SplitterBar::OnPaint() 
{
	RECT rc;
  if ( !GetUpdateRect( &rc ) )
  {
    return;
  }

	PAINTSTRUCT paint;
	CDC* pDC = BeginPaint( &paint );

	CRect rect;
	GetClientRect( rect );

  pDC->FillSolidRect( rect, GetSysColor( COLOR_BTNFACE ) );
	EndPaint(&paint);
}



LRESULT SplitterBar::OnNcHitTest(CPoint point) 
{	
	return HTCLIENT;
}



int SplitterBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
  {
    return -1;
  }
	
	GetWindowRect( &m_rectSplitter );
	SetWindowPos( &CWnd::wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE );

	//Initialize left most and right most coordinator
	CRect rectParent;
	GetParent()->GetClientRect(rectParent);
	if ( m_Horizontal )
	{
		m_LeftMost  = rectParent.top;
		m_RightMost = rectParent.bottom;
	}
	else
	{
		m_LeftMost  = rectParent.left;
		m_RightMost = rectParent.right;
	}
	return 0;
}



BOOL SplitterBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor=GetMessagePos();		

	if ( IsCursorOverSplitter( ptCursor ) )
	{
		::SetCursor( AfxGetApp()->LoadStandardCursor( m_Horizontal ? IDC_SIZENS : IDC_SIZEWE ) );	
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}



BOOL SplitterBar::IsCursorOverSplitter( const CPoint& ptCursor )
{
	CRect     rectSplitter;

	GetWindowRect( rectSplitter );
	return rectSplitter.PtInRect( ptCursor );
}



void SplitterBar::OnMouseMove( UINT nFlags, CPoint point ) 
{
	if ( ( nFlags & MK_LBUTTON )
  &&   ( m_Dragging ) )
	{
    if ( !m_DragImmediately )
    {
  		DrawDraggingBar( point );
    }
    else
    {
      ClientToScreen( &point );
      DraggedTo( point );
    }
		return;
	}

	CWnd::OnMouseMove(nFlags, point);
}



void SplitterBar::OnLButtonDown(UINT nFlags, CPoint point) 
{	
	ClientToScreen(&point);
	if ( IsCursorOverSplitter( point ) )
	{
		SetCapture();
		m_Dragging = TRUE;
		GetWindowRect( &m_rectSplitter );		
		ScreenToClient( &point );
    if ( !m_DragImmediately )
    {
		  DrawDraggingBar(point,DRAG_ENTER);
    }
		return;
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}



void SplitterBar::OnLButtonUp( UINT nFlags, CPoint point ) 
{
	if ( m_Dragging )
	{
    if ( !m_DragImmediately )
    {
  		DrawDraggingBar( point, DRAG_EXIT );
    }
		//Move the splitter here
		ClientToScreen( &point );

    DraggedTo( point );

    if ( GetCapture() == this )
    {
	    ReleaseCapture();
    }
	  m_Dragging=FALSE;
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}



void SplitterBar::DrawDraggingBar(CPoint point,DRAGFLAG df)
{
	ClientToScreen(&point);
	m_rectDragCurt=m_rectSplitter;
	if ( m_Horizontal )
	{
		m_rectDragCurt.top=point.y;
		m_rectDragCurt.bottom=point.y+m_rectSplitter.Height();
	}
	else
	{
		m_rectDragCurt.left=point.x;
		m_rectDragCurt.right=point.x+m_rectSplitter.Width();
	}

	CSize size(m_rectDragCurt.Width(),m_rectDragCurt.Height());

	CWnd *pParentWnd=GetParent();
	ASSERT(pParentWnd);
	CDC *pDC=pParentWnd->GetDC();	
	pParentWnd->ScreenToClient(m_rectDragCurt);
	switch(df)
	{
	case DRAG_ENTER:
		 pDC->DrawDragRect(m_rectDragCurt,size,NULL,size);
		 break;
	case DRAG_EXIT:	//fall through
	default:
		 pDC->DrawDragRect(m_rectDragCurt,size,m_rectDragPrev,size);
		 break;
	}

	pParentWnd->ReleaseDC(pDC);
	m_rectDragPrev=m_rectDragCurt;
}


void SplitterBar::DraggedTo( POINT point )
{
	if ( m_Horizontal )
	{
		CPoint pointLeftMost;
		pointLeftMost.y=m_LeftMost;
		GetParent()->ClientToScreen(&pointLeftMost);
		CPoint pointRightMost;
		pointRightMost.y=m_RightMost;
		GetParent()->ClientToScreen(&pointRightMost);

		if(point.y < pointLeftMost.y)
			point.y=pointLeftMost.y;
		if(point.y > pointRightMost.y)
			point.y=pointRightMost.y;

		m_rectDragCurt=m_rectSplitter;
		m_rectDragCurt.top=point.y;
		m_rectDragCurt.bottom=point.y+m_rectSplitter.Height();
	}
	else
	{
		CPoint pointLeftMost;
		pointLeftMost.x=m_LeftMost;
		GetParent()->ClientToScreen(&pointLeftMost);
		CPoint pointRightMost;
		pointRightMost.x=m_RightMost;
		GetParent()->ClientToScreen(&pointRightMost);

		if(point.x < pointLeftMost.x)
			point.x=pointLeftMost.x;
		if(point.x > pointRightMost.x)
			point.x=pointRightMost.x;

		m_rectDragCurt=m_rectSplitter;
		m_rectDragCurt.left=point.x;
		m_rectDragCurt.right=point.x+m_rectSplitter.Width();
	}
	GetParent()->ScreenToClient(m_rectDragCurt);
	MoveWindow(m_rectDragCurt,TRUE);
	OnPaint();

	MovePanes();
	GetParent()->SendMessage(WM_SPLITTER_MOVED,0,0L);

  SetPanes( m_pwndLeftPane, m_pwndRightPane );
}


void SplitterBar::SetPanes(CWnd *pwndLeftPane,CWnd *pwndRightPane)
{
	ASSERT(pwndLeftPane);
	ASSERT(pwndRightPane);

	m_pwndLeftPane=pwndLeftPane;
	m_pwndRightPane=pwndRightPane;

	if( m_Horizontal )
	{
		//Initialize splitter bar position & size
		CRect rectBar;
		pwndLeftPane->GetWindowRect(rectBar);
		GetParent()->ScreenToClient(rectBar);
		rectBar.top=rectBar.bottom;
		int nBarWidth=GetSystemMetrics(SM_CXFRAME);
		rectBar.top-=nBarWidth/2;
		rectBar.bottom+=nBarWidth/2;
		MoveWindow(rectBar);
		
		//repostion top & bottom panes
		MovePanes();
	}
	else
	{
		//Initialize splitter bar position & size
		CRect rectBar;
		pwndLeftPane->GetWindowRect(rectBar);
		GetParent()->ScreenToClient(rectBar);
		rectBar.left=rectBar.right;
		int nBarWidth=GetSystemMetrics(SM_CYFRAME);
		rectBar.left-=nBarWidth/2;
		rectBar.right+=nBarWidth/2;
		MoveWindow(rectBar);
		
		//repostion left & rigth panes
		MovePanes();
	}
  Resize();
}



void SplitterBar::MovePanes()
{
	ASSERT(m_pwndLeftPane);
	ASSERT(m_pwndRightPane);

	if( m_Horizontal )
	{
		//Get position of the splitter bar
		CRect rectBar;
		GetWindowRect(rectBar);
		GetParent()->ScreenToClient(rectBar);

		//reposition top pane
		CRect rectLeft;
		m_pwndLeftPane->GetWindowRect(rectLeft);
		GetParent()->ScreenToClient(rectLeft);
		rectLeft.bottom=rectBar.top+GetSystemMetrics(SM_CXBORDER);
		m_pwndLeftPane->MoveWindow(rectLeft);

		//reposition bottom pane
		CRect rectRight;
		m_pwndRightPane->GetWindowRect(rectRight);
		GetParent()->ScreenToClient(rectRight);
		rectRight.top=rectBar.bottom-GetSystemMetrics( SM_CXBORDER );;
		m_pwndRightPane->MoveWindow(rectRight);
	}
	else
	{
		//Get position of the splitter bar
		CRect rectBar;
		GetWindowRect(rectBar);
		GetParent()->ScreenToClient(rectBar);

		//reposition left pane
		CRect rectLeft;
		m_pwndLeftPane->GetWindowRect(rectLeft);
		GetParent()->ScreenToClient(rectLeft);
		rectLeft.right=rectBar.left+GetSystemMetrics(SM_CYBORDER);
		m_pwndLeftPane->MoveWindow(rectLeft);

		//reposition right pane
		CRect rectRight;
		m_pwndRightPane->GetWindowRect(rectRight);
		GetParent()->ScreenToClient(rectRight);
		rectRight.left=rectBar.right-GetSystemMetrics(SM_CYBORDER);;
		m_pwndRightPane->MoveWindow(rectRight);
	}

	//repaint client area
	GetParent()->Invalidate();
}



void SplitterBar::Resize()
{
	if ( m_Horizontal )
	{
		//calculate top most and bottom most coordinator
		CRect rectLeft;
		m_pwndLeftPane->GetWindowRect(rectLeft);
		GetParent()->ScreenToClient(rectLeft);
		m_LeftMost=rectLeft.top;

		CRect rectRight;
		m_pwndRightPane->GetWindowRect(rectRight);
		GetParent()->ScreenToClient(rectRight);
		m_RightMost=rectRight.bottom;
	}
	else
	{
		//calculate left most and right most coordinator
		CRect rectLeft;
		m_pwndLeftPane->GetWindowRect(rectLeft);
		GetParent()->ScreenToClient(rectLeft);
		m_LeftMost=rectLeft.left;

		CRect rectRight;
		m_pwndRightPane->GetWindowRect(rectRight);
		GetParent()->ScreenToClient(rectRight);
		m_RightMost=rectRight.right;
	}
}