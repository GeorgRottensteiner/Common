/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

#include "rangeedit.h"


#include <Misc/Misc.h>

#include <MFC/MemDC.h>

#include <math.h>




#include <debug/debugclient.h>

#ifdef UNICODE
#define TO_UTF_WINDOWS GR::Convert::ToUTF16
#else
#define TO_UTF_WINDOWS GR::Convert::ToUTF8
#endif


namespace RANGEEDIT
{
  LRESULT CALLBACK RangeEditHelperProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
  {
    if ( uMsg == WM_PAINT )
    {
      RangeEdit*   pRangeEdit = (RangeEdit*)::GetWindowLongPtr( hWnd, GWL_USERDATA );

      if ( pRangeEdit )
      {
        RECT          rc;
        PAINTSTRUCT   ps;

        ::GetClientRect( hWnd, &rc );
        HDC     hdc = ::BeginPaint( hWnd, &ps );

        pRangeEdit->DrawPopup( hdc, rc );

        ::EndPaint( hWnd, &ps );

        ::ValidateRect( hWnd, &rc );
        return TRUE;
      }
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
  }
}



RangeEdit::RangeEdit()
: CWnd(),
  m_SliderMode( SM_DEFAULT ),
  m_Minimum( 0 ),
  m_Maximum( 100 ),
  m_Position( 0 ),
  m_CursorPosInText( 1 ),
  m_FirstVisibleChar( 0 ),
  m_SelectionAnchor( -1 ),
  m_Step( 1 ),
  m_DigitsPreComma( -1 ),
  m_DigitsPostComma( 2 ),
  m_MinimumF( 0.0f ),
  m_MaximumF( 100.0f ),
  m_PositionF( 0.0f ),
  m_StepF( 1.0f ),
  m_ControlFlags( 0 ),
  m_Focus( false ),
  m_MouseDownInEdit( false ),
  m_CaretX( 0 ),
  m_Mode( MODE_INTEGER ),
  m_Content( "" ),
  m_hwndPopup( NULL )
{
  m_bgrBarColor = GetSysColor( COLOR_HIGHLIGHT );
}



RangeEdit::~RangeEdit()
{
  if ( m_hwndPopup )
  {
    ::DestroyWindow( m_hwndPopup );
    m_hwndPopup = NULL;
  }
  CWnd::~CWnd();
}



BEGIN_MESSAGE_MAP(RangeEdit, CWnd)
	//{{AFX_MSG_MAP(RangeEdit)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CHAR()
	ON_WM_GETDLGCODE()
	ON_WM_MOUSEMOVE()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
  ON_WM_KEYDOWN()
  ON_WM_SETCURSOR()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



BOOL RangeEdit::hasclass = RangeEdit::RegisterMe();



BOOL RangeEdit::RegisterMe()
{
  WNDCLASS wc;   
  wc.style          = 0;                                                 
  wc.lpfnWndProc    = ::DefWindowProc; // must be this value
  wc.cbClsExtra     = 0;                         
  wc.cbWndExtra     = 0;                               
  wc.hInstance      = (HINSTANCE)::GetModuleHandle(NULL);
  wc.hIcon          = NULL;     // child window has no icon         
  wc.hCursor        = NULL;   // we use OnSetCursor                  
  wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName   = NULL;  // no menu                             
  wc.lpszClassName  = RANGEEDIT_CLASS_NAME;
  return AfxRegisterClass(&wc);
}



void RangeEdit::OnPaint() 
{
  PAINTSTRUCT   ps;

  CDC*          pPaintDC = BeginPaint( &ps );

  RECT          rc;



  GetClientRect( &rc );

  MFCExtension::CMemDC* pDC = new MFCExtension::CMemDC( pPaintDC, rc );
 
  if ( GetWindowLong( m_hWnd, GWL_STYLE ) & WS_DISABLED )
  {
    pDC->FillSolidRect( &rc, GetSysColor( COLOR_3DFACE ) );
  }
  else
  {
    pDC->FillSolidRect( &rc, GetSysColor( COLOR_WINDOW ) );
  }

  // BAUSTELLE - nicht drüber malen
  rc.bottom = rc.top + 20;

  pDC->DrawEdge( &rc, EDGE_SUNKEN, BF_RECT );

  rc.left = rc.right - 20;
  InflateRect( &rc, -2, -2 );

  DWORD   ControlFlags = DFCS_SCROLLDOWN;

  if ( GetWindowLong( m_hWnd, GWL_STYLE ) & WS_DISABLED )
  {
    ControlFlags |= DFCS_INACTIVE;
  }

  if ( m_ControlFlags & 0x00000001 )
  {
    ControlFlags |= DFCS_FLAT;
  }
  pDC->DrawFrameControl( &rc, DFC_SCROLL, ControlFlags );

  GetClientRect( &rc );
  rc.top = 20;
  rc.bottom = 26;

  pDC->DrawEdge( &rc, BDR_SUNKENOUTER, BF_RECT );

  InflateRect( &rc, -1, -1 );

  // kleiner Balken unten
  DrawPopup( pDC->GetSafeHdc(), rc );
  //pDC->FillSolidRect( &rc, GetSysColor( COLOR_HIGHLIGHT ) );

  {
    // Inhalt
    HFONT   holdFont = (HFONT)SelectObject( pDC->GetSafeHdc(), GetStockObject( DEFAULT_GUI_FONT ) );
    GetClientRect( &rc );

    rc.bottom = rc.top + 20;
    InflateRect( &rc, -2, -2 );

    RECT    rc2 = rc;

    pDC->SetBkMode( TRANSPARENT );

    if ( GetWindowLong( m_hWnd, GWL_STYLE ) & WS_DISABLED )
    {
      pDC->SetTextColor( GetSysColor( COLOR_GRAYTEXT ) );
    }
    else
    {
      pDC->SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
    }

    rc2.right -= 20;
    rc2.left += 2;

    if ( m_Content.length() )
    {
      size_t   iLength = m_Content.length();

      rc.left += 2;
      rc.right -= 20;

      rc.top++;

      if ( ( HasSelection() )
      &&   ( m_Focus ) )
      {
        int   iSelBegin = m_SelectionAnchor,
              iSelEnd   = m_CursorPosInText;

        if ( iSelBegin > iSelEnd )
        {
          iSelEnd     = m_SelectionAnchor;
          iSelBegin   = m_CursorPosInText;
        }
        // linke, nicht selektierte Hälfte darstellen
        if ( iSelBegin > m_FirstVisibleChar )
        {
          int   iDisplayLength = iSelBegin - m_FirstVisibleChar;

          RECT    rcTemp = rc;

          pDC->SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
          pDC->SetBkColor( GetSysColor( COLOR_WINDOW ) );
          pDC->SetBkMode( TRANSPARENT );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( 0, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_LEFT );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( 0, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT );

          rc.left += rcTemp.right - rcTemp.left;
        }

        // mittleren, selektierten Anteil darstellen
        if ( iSelEnd > iSelBegin )
        {
          int   iDisplayLength = iSelEnd - iSelBegin;

          RECT    rcTemp = rc;

          pDC->SetTextColor( GetSysColor( COLOR_HIGHLIGHTTEXT ) );
          pDC->SetBkColor( GetSysColor( COLOR_HIGHLIGHT ) );
          pDC->SetBkMode( OPAQUE );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iSelBegin, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iSelBegin, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER );

          rc.left += rcTemp.right - rcTemp.left;
        }

        // rechten, nicht selektierten Anteil darstellen
        if ( (int)m_Content.length() > iSelEnd )
        {
          int   iDisplayLength = (int)m_Content.length() - iSelEnd;

          RECT    rcTemp = rc;

          pDC->SetTextColor( GetSysColor( COLOR_WINDOWTEXT ) );
          pDC->SetBkColor( GetSysColor( COLOR_WINDOW ) );
          pDC->SetBkMode( TRANSPARENT );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iSelEnd, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER );

          pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iSelEnd, iDisplayLength ) ).c_str(), iDisplayLength, &rcTemp, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER );

          rc.left += rcTemp.right - rcTemp.left;
        }
      }
      else
      {
        pDC->DrawText( TO_UTF_WINDOWS( m_Content ).c_str(), (int)iLength, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT );
      }
    }
    else
    {
      m_CaretX = 4;
    }

    SelectObject( pDC->GetSafeHdc(), holdFont );
  }


  RecalcCaretPos();

  delete pDC;

  EndPaint( &ps );
}



void RangeEdit::RecalcCaretPos()
{
  if ( m_CursorPosInText < m_FirstVisibleChar )
  {
    m_FirstVisibleChar = m_CursorPosInText;
    Invalidate();
  }

  RECT    rc,
          rc2;

  GetClientRect( &rc );

  rc.bottom = rc.top + 20;
  InflateRect( &rc, -2, -2 );

  CDC*    pDC = GetDC();


  HFONT   holdFont = (HFONT)SelectObject( pDC->GetSafeHdc(), GetStockObject( DEFAULT_GUI_FONT ) );

  int     iFirstChar = m_FirstVisibleChar,
          iLength = (int)m_Content.substr( iFirstChar, m_CursorPosInText - iFirstChar ).length();

  rc2 = rc;
  rc2.right -= 20;
  rc2.left += 2;

  pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iFirstChar, m_CursorPosInText - iFirstChar ) ).c_str(), iLength, &rc2, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT );
  if ( iLength == 0 )
  {
    m_CaretX = rc2.left;
  }
  else
  {
    m_CaretX = rc2.right;
  }

  SelectObject( pDC->GetSafeHdc(), holdFont );

  ReleaseDC( pDC );

  

  POINT pt;

  pt.x = m_CaretX;
  pt.y = 4;

  if ( m_Focus )
  {
    SetCaretPos( pt );
    ShowCaret();
  }
}



void RangeEdit::SetBarColor( COLORREF bgrColor )
{
  m_bgrBarColor = bgrColor;
  Invalidate();
}



void RangeEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if ( GetCapture() != this )
  {
    SetFocus();
  }
  else
  {
    RECT    rc;
    GetWindowRect( &rc );
    if ( !PtInRect( &rc, point ) )
    {
      //KillFocus();
    }
  }

  RECT    rc;

  GetButtonRect( rc );

  if ( PtInRect( &rc, point ) )
  {
    if ( !( m_ControlFlags & 0x00000001 ) )
    {
      m_ControlFlags |= 0x00000001;
      SetCapture();

      InvalidateRect( &rc, FALSE );

      ClientToScreen( &rc );

      int iWidth;

      if ( m_Mode == 0 )
      {
        iWidth = ( m_Maximum - m_Minimum + 1 ) / m_Step + 6;
        if ( ( m_Maximum - m_Minimum + 1 ) % m_Step )
        {
          iWidth++;
        }
        if ( iWidth >= 400 )
        {
          iWidth = 400;
        }
      }
      else if ( m_Mode == 1 )
      {
        iWidth = (int)( ( m_MaximumF - m_MinimumF + 1 ) / m_StepF + 6 );
        if ( iWidth >= 400 )
        {
          iWidth = 400;
        }
      }

      RECT    rcPopup;

      rcPopup.left  = rc.left + ( rc.right - rc.left ) / 2 - iWidth / 2;
      rcPopup.top   = rc.top + 20;
      rcPopup.right = rcPopup.left + iWidth;
      rcPopup.bottom = rcPopup.top + 20;

      m_hwndPopup = CreateWindow( _T( "STATIC" ),
                                  _T( "RangeEditHelper" ),
                                  WS_POPUP | WS_DLGFRAME,
                                  rcPopup.left,
                                  rcPopup.top,
                                  rcPopup.right - rcPopup.left,
                                  rcPopup.bottom - rcPopup.top,
                                  /*
                                  10, 10,
                                  200, 100,
                                  */
                                  m_hWnd,
                                  NULL,
                                  AfxGetInstanceHandle(),
                                  0 );

      ::SetWindowLongPtr( m_hwndPopup, GWL_WNDPROC, (LONG_PTR)RANGEEDIT::RangeEditHelperProc );
      ::SetWindowLongPtr( m_hwndPopup, GWL_USERDATA, (LONG_PTR)this );
      ::ShowWindow( m_hwndPopup, SW_SHOWNA );
    }
  }

  GetSliderRect( rc );

  if ( PtInRect( &rc, point ) )
  {
    if ( !( m_ControlFlags & 0x00000002 ) )
    {
      m_ControlFlags |= 0x00000002;
      InvalidateRect( &rc, FALSE );

      ClientToScreen( &rc );
      ClipCursor( &rc );
      SetCapture();
    }
  }
  GetEditRect( rc );
  if ( PtInRect( &rc, point ) )
  {
    // Cursor setzen
    m_MouseDownInEdit = true;
    m_CursorPosInText = CursorPosFromPoint( point );
    m_SelectionAnchor = m_CursorPosInText;
    RecalcCaretPos();
    Invalidate();
  }

	CWnd::OnLButtonDown(nFlags, point);
}



void RangeEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonUp(nFlags, point);

  StopButton();
  if ( m_ControlFlags & 0x00000002 )
  {
    m_ControlFlags &= ~0x00000002;
  }

  m_MouseDownInEdit = false;

  ClipCursor( NULL );
	ReleaseCapture();
}



void RangeEdit::GetEditRect( RECT& rc ) 
{
  GetClientRect( &rc );
  rc.top = 0;
  rc.bottom = 20;

  InflateRect( &rc, -2, -2 );

  rc.right -= 16;
  rc.left += 2;
}



void RangeEdit::GetSliderRect( RECT& rc ) 
{
  GetClientRect( &rc );
  rc.top = 20;
  rc.bottom = 26;

  InflateRect( &rc, -1, -1 );
}



void RangeEdit::GetButtonRect( RECT& rc ) 
{
  GetClientRect( &rc );

  rc.left = rc.right - 20;
  rc.bottom = rc.top + 20;
  InflateRect( &rc, -2, -2 );
}



int RangeEdit::CursorPosFromPoint( POINT& pt )
{
  RECT  rc;

  GetEditRect( rc );

  if ( !PtInRect( &rc, pt ) )
  {
    if ( pt.x < rc.left )
    {
      return 0;
    }
    return (int)m_Content.length();
  }

  int   iNewCursorPos = -1;

  int   iX1 = 4,
        iX2 = 4;

  CDC*    pDC = GetDC();


  HFONT   holdFont = (HFONT)SelectObject( pDC->GetSafeHdc(), GetStockObject( DEFAULT_GUI_FONT ) );

  if ( m_Content.empty() )
  {
    return 0;
  }

  int     iFirstChar = m_FirstVisibleChar,
          iLength = 1;

  while ( true )
  {
    pDC->DrawText( TO_UTF_WINDOWS( m_Content.substr( iFirstChar, iLength ) ).c_str(), iLength, &rc, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_CALCRECT );

    iX2 = rc.right;

    if ( ( pt.x >= iX1 )
    &&   ( pt.x < iX2 ) )
    {
      if ( pt.x < ( iX2 + iX1 ) / 2 )
      {
        // nach links
        iNewCursorPos = iLength - 1;
        break;
      }
      iNewCursorPos = iLength;
      break;
    }

    iX1 = iX2;
    iLength++;
    if ( iLength >= (int)m_Content.length() )
    {
      iNewCursorPos = (int)m_Content.length();
      break;
    }
  }
  SelectObject( pDC->GetSafeHdc(), holdFont );

  ReleaseDC( pDC );

  return iNewCursorPos;
}



void RangeEdit::StopButton()
{
  if ( m_ControlFlags & 0x00000001 )
  {
    RECT    rc;

    GetButtonRect( rc );
    m_ControlFlags &= ~0x00000001;
    InvalidateRect( &rc, FALSE );

    if ( m_hwndPopup )
    {
      ::DestroyWindow( m_hwndPopup );
      m_hwndPopup = NULL;
    }
  }
}



void RangeEdit::OnKillFocus( CWnd* pNewWnd ) 
{
	CWnd::OnKillFocus(pNewWnd);
	
  m_Focus           = FALSE;
  m_MouseDownInEdit = false;
  DestroyCaret();
  ClipCursor( NULL );

  if ( CWnd::GetCapture() == this )
  {
	  ReleaseCapture();
  }

  StopButton();
  if ( m_ControlFlags & 0x00000002 )
  {
    m_ControlFlags &= ~0x00000002;
  }
  Invalidate();
}



void RangeEdit::OnSetFocus( CWnd* pOldWnd ) 
{
	CWnd::OnSetFocus( pOldWnd );
	
  m_Focus = TRUE;
  CreateSolidCaret( 1, 12 );

  RecalcCaretPos();
  Invalidate();
}



void RangeEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if ( nChar == VK_BACK )
  {
    if ( m_CursorPosInText > 0 )
    {
      GR::String     newString;

      if ( m_CursorPosInText == 1 )
      {
        newString = m_Content.substr( 1 );
      }
      else
      {
        // linker Rest
        newString = m_Content.substr( 0, m_CursorPosInText - 1 );

        // rechter Rest
        if ( m_CursorPosInText < (int)m_Content.length() )
        {
          newString += m_Content.substr( m_CursorPosInText );
        }
      }
      --m_CursorPosInText;
      m_SelectionAnchor = m_CursorPosInText;
      m_Content         = newString;
      RecalcCaretPos();
      UpdatePositionFromString();
      Invalidate( FALSE );
    }
  }
  else if ( m_Mode == 0 )
  {
    if ( m_Content.length() < 250 )
    {
      if ( ( ( nChar >= '0' )
      &&     ( nChar <= '9' ) )
      ||   ( nChar == '-' ) )
      {
        GR::String   newString;

        if ( HasSelection() )
        {
          int   iSelBegin = m_CursorPosInText,
                iSelEnd   = m_SelectionAnchor;

          if ( iSelBegin > iSelEnd )
          {
            iSelBegin = m_SelectionAnchor;
            iSelEnd   = m_CursorPosInText;
          }

          newString = m_Content.substr( 0, iSelBegin )
                    + (GR::Char)nChar;
          if ( iSelEnd < (int)m_Content.length() )
          {
            newString += m_Content.substr( iSelEnd );
          }
          m_CursorPosInText = iSelBegin + 1;
        }
        else
        {
          if ( m_Content.empty() )
          {
            newString = (GR::Char)nChar;
          }
          else
          {
            newString = m_Content.substr( 0, m_CursorPosInText )
                    + (GR::Char)nChar;
            if ( m_CursorPosInText < (int)m_Content.length() )
            {
              newString += m_Content.substr( m_CursorPosInText );
            }
          }
          m_CursorPosInText++;
        }
        m_Content         = newString;
        m_SelectionAnchor = m_CursorPosInText;
        UpdatePositionFromString();
        Invalidate( FALSE );
      }
    }
  }
  else if ( m_Mode == 1 )
  {
    if ( m_Content.length() < 250 )
    {
      if ( ( ( nChar >= '0' )
      &&     ( nChar <= '9' ) )
      ||   ( nChar == '.' )
      ||   ( nChar == '-' ) )
      {
        GR::String   newString;

        if ( HasSelection() )
        {
          int   iSelBegin = m_CursorPosInText,
                iSelEnd   = m_SelectionAnchor;

          if ( iSelBegin > iSelEnd )
          {
            iSelBegin = m_SelectionAnchor;
            iSelEnd   = m_CursorPosInText;
          }

          newString = m_Content.substr( 0, iSelBegin )
                    + (GR::Char)nChar;
          if ( iSelEnd < (int)m_Content.length() )
          {
            newString += m_Content.substr( iSelEnd );
          }
          m_CursorPosInText = iSelBegin + 1;
        }
        else
        {
          if ( m_Content.empty() )
          {
            newString = (GR::Char)nChar;
          }
          else
          {
            newString = m_Content.substr( 0, m_CursorPosInText )
                    + (GR::Char)nChar;
            if ( m_CursorPosInText < (int)m_Content.length() )
            {
              newString += m_Content.substr( m_CursorPosInText );
            }
          }
          m_CursorPosInText++;
        }
        m_Content         = newString;
        m_SelectionAnchor = m_CursorPosInText;
        UpdatePositionFromString();
        Invalidate( FALSE );
      }
    }
  }
	CWnd::OnChar( nChar, nRepCnt, nFlags );
}



void RangeEdit::UpdatePositionFromString()
{
  if ( m_Mode == 0 )
  {
    int iValue = 0;
    if ( m_Content.length() )
    {
      iValue = GR::Convert::ToI32( m_Content );
    }
    if ( iValue >= m_Maximum )
    {
      m_Position = m_Maximum;
      m_Content = GR::Convert::ToString( m_Maximum );
    }
    else if ( iValue < m_Minimum )
    {
      m_Position = m_Minimum;
      m_Content = GR::Convert::ToString( m_Minimum );
    }
    else
    {
      m_Position = iValue;
    }
  }
  else if ( m_Mode == 1 )
  {
    float fValue = 0.0f;

    fValue = GR::Convert::ToF32( m_Content );

    if ( fValue < m_MinimumF )
    {
      fValue = m_MinimumF;
    }
    if ( fValue > m_MaximumF )
    {
      fValue = m_MaximumF;
    }
    m_PositionF = fValue;
  }

  if ( m_CursorPosInText > (int)m_Content.length() )
  {
    m_CursorPosInText = (int)m_Content.length();
  }
  if ( m_SelectionAnchor > (int)m_Content.length() )
  {
    m_SelectionAnchor = (int)m_Content.length();
  }
  PositionChanged();
}



void RangeEdit::UpdatePositionFromValue()
{
  if ( m_Mode == 0 )
  {
    if ( m_Position < m_Minimum )
    {
      m_Position = m_Minimum;
    }
    else if ( m_Position > m_Maximum )
    {
      m_Position = m_Maximum;
    }
    m_Content = GR::Convert::ToString( m_Position );
  }
  else if ( m_Mode == 1 )
  {
    if ( m_PositionF < m_MinimumF )
    {
      m_PositionF = m_MinimumF;
    }
    else if ( m_PositionF > m_MaximumF )
    {
      m_PositionF = m_MaximumF;
    }

    int   iMax = (int)m_PositionF,
          iMin = abs( (int)( m_PositionF * 10000 ) % 10000 );

    GR::String     strTemp = GR::Convert::ToString( abs( iMax ) );

    if ( m_DigitsPreComma != -1 )
    {
      while ( (int)strTemp.length() < m_DigitsPreComma )
      {
        strTemp = "0" + strTemp;
      }
    }
    if ( iMax < 0.0f )
    {
      m_Content = '-';
      m_Content += strTemp;
    }
    else
    {
      m_Content = strTemp;
    }

    strTemp = GR::Convert::ToString( iMin );

    // auf 4 Stellen, da ein 10000tel
    while ( strTemp.length() < 4 )
    {
      strTemp = "0" + strTemp;
    }
    
    if ( m_DigitsPostComma != -1 )
    {
      while ( (int)strTemp.length() < m_DigitsPostComma )
      {
        strTemp = "0" + strTemp;
      }
      if ( (int)strTemp.length() > m_DigitsPostComma )
      {
        strTemp = strTemp.substr( 0, m_DigitsPostComma );
      }
    }

    m_Content += '.';
    m_Content += strTemp;
  }

  if ( m_CursorPosInText > (int)m_Content.length() )
  {
    m_CursorPosInText = (int)m_Content.length();
  }
  if ( m_SelectionAnchor > (int)m_Content.length() )
  {
    m_SelectionAnchor = (int)m_Content.length();
  }
  PositionChanged();
}



void RangeEdit::PositionChanged()
{
  if ( m_hwndPopup )
  {
    ::InvalidateRect( m_hwndPopup, NULL, FALSE );
  }
  CWnd *pParent = GetParent();

  SetWindowText( TO_UTF_WINDOWS( m_Content ).c_str() );

  RecalcCaretPos();

  if ( pParent )
  {
    if ( ( m_TempContent.empty() )
    ||   ( m_TempContent.compare( m_Content ) ) )
      {
        m_TempContent = m_Content;
        pParent->SendMessage( WM_COMMAND, MAKEWPARAM( GetDlgCtrlID(), EN_CHANGE ), (LPARAM)GetSafeHwnd() );
    }
  }
}



UINT RangeEdit::OnGetDlgCode() 
{
  return DLGC_WANTALLKEYS;
}



void RangeEdit::OnMouseMove( UINT nFlags, CPoint point ) 
{
  if ( nFlags & MK_LBUTTON )
  {
    if ( m_ControlFlags & 0x00000001 )
    {
      // der User draggt im Popup rum
      ClientToScreen( &point );
      RECT    rc;

      ::GetWindowRect( m_hwndPopup, &rc );

      if ( m_Mode == 0 )
      {
        float   fRelPos = (float)( point.x - rc.left - 3 ) / ( rc.right - rc.left - 6 );

        if ( m_SliderMode & SM_EXPONENTIELL )
        {
          fRelPos = fRelPos * fRelPos * fRelPos * fRelPos;
        }

        m_Position = m_Minimum + (int)( ( m_Maximum - m_Minimum ) * fRelPos );
        if ( m_Position < m_Minimum )
        {
          m_Position = m_Minimum;
        }
        if ( m_Position > m_Maximum )
        {
          m_Position = m_Maximum;
        }
      }
      else if ( m_Mode == 1 )
      {
        point.x++;
        m_PositionF = m_MinimumF + ( m_MaximumF - m_MinimumF ) * (float)( point.x - rc.left - 3 ) / (float)( rc.right - rc.left - 6 );

        if ( m_PositionF < m_MinimumF )
        {
          m_PositionF = m_MinimumF;
        }
        if ( m_PositionF > m_MaximumF )
        {
          m_PositionF = m_MaximumF;
        }
      }
      UpdatePositionFromValue();
      Invalidate( FALSE );
    }
    else if ( m_ControlFlags & 0x00000002 )
    {
      // der User draggt im Slider rum
      RECT    rc;

      GetSliderRect( rc );

      point.x -= rc.left;
      rc.right -= rc.left;

      if ( m_Mode == 0 )
      {
        m_Position = m_Minimum + ( m_Maximum - m_Minimum ) * point.x / ( rc.right - rc.left );
        m_Position -= ( m_Position % m_Step );
      }
      else if ( m_Mode == 1 )
      {
        m_PositionF = m_MinimumF + ( m_MaximumF - m_MinimumF ) * (float)point.x / (float)( rc.right - rc.left );

        //m_PositionF -= ( m_Position % m_Step );
      }

      UpdatePositionFromValue();
      Invalidate( FALSE );
    }
    else if ( m_MouseDownInEdit )
    {
      // der User draggt in der Edit-Control rum
      m_CursorPosInText = CursorPosFromPoint( point );
      Invalidate();
    }
  }
	
	CWnd::OnMouseMove(nFlags, point);
}



    
void RangeEdit::DrawPopup( HDC hdc, const RECT& rcTarget )
{
	::SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rcTarget, NULL, 0, NULL);

  RECT    rcCaret = rcTarget;
  if ( m_Mode == 0 )
  {
    float   fRelPos = (float)( m_Position - m_Minimum ) / ( m_Maximum - m_Minimum );

    if ( m_SliderMode & SM_EXPONENTIELL )
    {
      fRelPos = powf( fRelPos, 0.25f );
    }

    rcCaret.right = rcTarget.left + (int)( ( rcTarget.right - rcTarget.left ) * fRelPos );
  }
  else if ( m_Mode == 1 )
  {
    rcCaret.right = (int)( rcTarget.left + ( rcTarget.right - rcTarget.left ) * ( m_PositionF - m_MinimumF ) / ( m_MaximumF - m_MinimumF ) );
  }
  
	::SetBkColor( hdc, m_bgrBarColor );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rcCaret, NULL, 0, NULL);
}



void RangeEdit::SetRange( int Min, int Max, int Step )
{
  m_Mode      = MODE_INTEGER;
  m_Minimum   = Min;
  m_Maximum   = Max;
  m_Step      = Step;

  UpdatePositionFromValue();
  Invalidate( FALSE );
}



void RangeEdit::SetRange( float Min, float Max, float Step, int DigitsPreComma, int DigitsPostComma )
{
  m_Mode              = MODE_FLOAT;
  m_MinimumF          = Min;
  m_MaximumF          = Max;
  m_StepF             = Step;
  m_DigitsPreComma    = DigitsPreComma;
  m_DigitsPostComma   = DigitsPostComma;

  UpdatePositionFromValue();
  Invalidate( FALSE );
}



void RangeEdit::SetPosition( int Pos )
{
  m_Mode = MODE_INTEGER;

  if ( Pos < m_Minimum )
  {
    Pos = m_Minimum;
  }
  if ( Pos > m_Maximum )
  {
    Pos = m_Maximum;
  }
  m_Position = Pos;

  UpdatePositionFromValue();
  Invalidate( FALSE );
}



void RangeEdit::SetPosition( float Pos )
{
  m_Mode = MODE_FLOAT;

  if ( Pos < m_MinimumF )
  {
    Pos = m_MinimumF;
  }
  if ( Pos > m_MaximumF )
  {
    Pos = m_MaximumF;
  }
  m_PositionF = Pos;

  UpdatePositionFromValue();
  Invalidate( FALSE );
}



void RangeEdit::OnEnable( BOOL Enable ) 
{
	CWnd::OnEnable( Enable );
	
  Invalidate();
}



void RangeEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if ( nChar == VK_TAB )
  {
    if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
    {
      CWnd* pWndNext = GetNextWindow( GW_HWNDPREV );
      if ( pWndNext )
      {
        pWndNext->SetFocus();
      }
    }
    else
    {
      CWnd* pWndNext = GetNextWindow();
      if ( pWndNext )
      {
        pWndNext->SetFocus();
      }
    }
  }
  else if ( nChar == VK_LEFT )
  {
    if ( ( HasSelection() )
    &&   ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) )
    {
      if ( m_CursorPosInText > m_SelectionAnchor )
      {
        m_CursorPosInText = m_SelectionAnchor;
      }
      m_SelectionAnchor = m_CursorPosInText;
      Invalidate();
      RecalcCaretPos();
    }
    else if ( m_CursorPosInText > 0 )
    {
      m_CursorPosInText--;
      RecalcCaretPos();
      if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
      {
        m_SelectionAnchor = m_CursorPosInText;
      }
      Invalidate();
    }
  }
  else if ( nChar == VK_RIGHT )
  {
    if ( ( HasSelection() )
    &&   ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) )
    {
      if ( m_SelectionAnchor > m_CursorPosInText )
      {
        m_CursorPosInText = m_SelectionAnchor;
      }
      m_SelectionAnchor = m_CursorPosInText;
      Invalidate();
      RecalcCaretPos();
    }
    else if ( m_CursorPosInText < GetWindowTextLength() )
    {
      m_CursorPosInText++;
      RecalcCaretPos();
      if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
      {
        m_SelectionAnchor = m_CursorPosInText;
      }
      Invalidate();
    }
  }
  else if ( nChar == VK_HOME )
  {
    m_CursorPosInText = 0;
    RecalcCaretPos();
    if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
    {
      m_SelectionAnchor = m_CursorPosInText;
    }
    Invalidate();
  }
  else if ( nChar == VK_END )
  {
    m_CursorPosInText = (int)m_Content.length();
    RecalcCaretPos();
    if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
    {
      m_SelectionAnchor = m_CursorPosInText;
    }
    Invalidate();
  }
  else if ( nChar == VK_DELETE )
  {
    if ( m_CursorPosInText < (int)m_Content.length() )
    {
      GR::String     newString;

      if ( m_CursorPosInText == 0 )
      {
        newString = m_Content.substr( 1 );
      }
      else
      {
        // linker Rest
        newString = m_Content.substr( 0, m_CursorPosInText );

        // rechter Rest
        if ( m_CursorPosInText + 1 < (int)m_Content.length() )
        {
          newString += m_Content.substr( m_CursorPosInText + 1 );
        }
      }
      m_Content = newString;
      RecalcCaretPos();
      UpdatePositionFromString();
      Invalidate( FALSE );
    }
  }

  CWnd::OnKeyDown( nChar, nRepCnt, nFlags );
}



bool RangeEdit::HasSelection()
{
  if ( m_SelectionAnchor == -1 )
  {
    return false;
  }
  if ( m_SelectionAnchor == m_CursorPosInText )
  {
    return false;
  }
  return true;
}



void RangeEdit::SetMode( eMode Mode )
{
  m_Mode = Mode;
}



BOOL RangeEdit::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  RECT    rc;

  GetEditRect( rc );

  POINT   pt;
  GetCursorPos( &pt );
  ScreenToClient( &pt );

  if ( PtInRect( &rc, pt ) )
  {
    SetCursor( LoadCursor( NULL, IDC_IBEAM ) );
    return TRUE;
  }
  SetCursor( LoadCursor( NULL, IDC_ARROW ) );
  return TRUE;
}



BOOL RangeEdit::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= WS_TABSTOP;

  return CWnd::PreCreateWindow(cs);
}



BOOL RangeEdit::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;

  return CWnd::OnEraseBkgnd(pDC);
}



int RangeEdit::GetPosition() const
{
  return m_Position;
}



float RangeEdit::GetPositionF() const
{
  return m_PositionF;
}



