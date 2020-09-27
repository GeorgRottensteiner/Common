#ifndef GR_UI_EDIT_H
#define GR_UI_EDIT_H

#include "CustomWnd.h"


class GRUIEdit : public CCustomWnd
{

  private:

    GR::String   m_strContent;

    int           m_iTrueCursorPos,
                  m_iFirstVisibleLine,
                  m_iCursorInLine,
                  m_iSelectionAnchor,
                  m_iFirstVisibleChar,
                  m_iCaretX;

    bool          m_bMouseDownInEdit,
                  m_bMultiLine;

    DWORD         m_dwDrawTextFlags;


  public:


    GRUIEdit() :
      m_bMouseDownInEdit( false ),
      m_iCaretX( 0 ),
      m_strContent( "" ),
      m_iTrueCursorPos( 0 ),
      m_iFirstVisibleChar( 0 ),
      m_iFirstVisibleLine( 0 ),
      m_iCursorInLine( 0 ),
      m_iSelectionAnchor( -1 ),
      m_bMultiLine( false ),
      m_dwDrawTextFlags( DT_LEFT | DT_NOPREFIX )
    {
    }

    virtual void Draw( HDC hdc, const RECT& rc )
    {
      m_pDisplayClass->PaintEdit( hdc, rc, m_strContent, m_pWndBackgroundProducer == NULL, GetFocus() == GetSafeHwnd(), m_bMultiLine, m_iFirstVisibleChar, m_iSelectionAnchor, m_iTrueCursorPos );
    }

    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {

      switch ( uMsg )
      {
        case WM_CREATE:
          if ( GetWindowLong( GWL_STYLE ) & ES_MULTILINE )
          {
            m_bMultiLine = true;
          }
          else
          {
            m_dwDrawTextFlags |= DT_SINGLELINE;
          }
          break;
        case WM_SETFOCUS:
          OnSetFocus();
          break;
        case WM_KILLFOCUS:
          OnKillFocus();
          break;
        case WM_GETDLGCODE:
          return DLGC_WANTALLKEYS;
        case WM_CHAR:
          OnChar( (UINT)wParam, LOWORD( lParam ) & 0xf, HIWORD( lParam ) );
          break;
        case WM_KEYDOWN:
          OnKeyDown( (UINT)wParam, LOWORD( lParam ) & 0xf, HIWORD( lParam ) );
          break;
        case WM_MOUSEMOVE:
          {
            POINT   pt;

            pt.x = LOWORD( lParam );
            pt.y = HIWORD( lParam );
            OnMouseMove( (UINT)wParam, pt );
          }
          break;
        case WM_LBUTTONDOWN:
          {
            POINT   pt;

            pt.x = LOWORD( lParam );
            pt.y = HIWORD( lParam );
            OnLButtonDown( (UINT)wParam, pt );
          }
          break;
        case WM_LBUTTONUP:
          {
            POINT   pt;

            pt.x = LOWORD( lParam );
            pt.y = HIWORD( lParam );
            OnLButtonUp( (UINT)wParam, pt );
          }
          break;
      }

      return CCustomWnd::WindowProc( uMsg, wParam, lParam );
    }

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
    {

      if ( nChar == VK_BACK )
      {
        if ( m_iTrueCursorPos > 0 )
        {
          GR::String     strNew = "";

          if ( m_iTrueCursorPos == 1 )
          {
            strNew = m_strContent.substr( 1 );
          }
          else
          {
            // linker Rest
            strNew = m_strContent.substr( 0, m_iTrueCursorPos - 1 );

            // rechter Rest
            if ( m_iTrueCursorPos < (int)m_strContent.length() )
            {
              strNew += m_strContent.substr( m_iTrueCursorPos );
            }
          }
          m_iTrueCursorPos--;

          m_iSelectionAnchor = m_iTrueCursorPos;
          m_strContent = strNew;
          RecalcCaretPos();
          Invalidate();
        }
      }
      else //if ( m_dwMode == 0 )
      {
        if ( ( (int)nChar >= 32 )
        ||   ( ( m_bMultiLine )
        &&     ( (int)nChar == 13 ) ) )
        {
          GR::String   strNew = "";

          if ( HasSelection() )
          {
            int   iSelBegin = m_iTrueCursorPos,
                  iSelEnd   = m_iSelectionAnchor;

            if ( iSelBegin > iSelEnd )
            {
              iSelBegin = m_iSelectionAnchor;
              iSelEnd   = m_iTrueCursorPos;
            }

            strNew = m_strContent.substr( 0, iSelBegin )
                      + (char)nChar;
            if ( nChar == 13 )
            {
              strNew += (char)10;
            }
            if ( iSelEnd < (int)m_strContent.length() )
            {
              strNew += m_strContent.substr( iSelEnd );
            }
            m_iTrueCursorPos = iSelBegin + 1;
          }
          else
          {
            if ( m_strContent.empty() )
            {
              strNew = (char)nChar;
              if ( nChar == 13 )
              {
                strNew += (char)10;
              }
            }
            else
            {
              strNew = m_strContent.substr( 0, m_iTrueCursorPos )
                      + (char)nChar;
              if ( nChar == 13 )
              {
                strNew += (char)10;
              }
              if ( m_iTrueCursorPos < (int)m_strContent.length() )
              {
                strNew += m_strContent.substr( m_iTrueCursorPos );
              }
            }
            m_iTrueCursorPos++;
          }
          m_strContent = strNew;
          m_iSelectionAnchor = m_iTrueCursorPos;
          RecalcCaretPos();
          Invalidate();
        }
      }

	    CCustomWnd::OnChar( nChar, nRepCnt, nFlags );

    }

    void RecalcCaretPos()
    {
      if ( m_iTrueCursorPos < m_iFirstVisibleChar )
      {
        m_iFirstVisibleChar = m_iTrueCursorPos;
        Invalidate();
      }

      RECT    rc,
              rc2;

      GetClientRect( &rc );

      rc.bottom = rc.top + 20;
      InflateRect( &rc, -2, -2 );

      HDC     hdc = GetDC();


      HFONT   holdFont = (HFONT)SelectObject( hdc, GetStockObject( DEFAULT_GUI_FONT ) );

      int     iFirstChar = m_iFirstVisibleChar,
              iLength = (int)m_strContent.substr( iFirstChar, m_iTrueCursorPos - iFirstChar ).length();

      rc2 = rc;

      DrawText( hdc, m_strContent.substr( iFirstChar, m_iTrueCursorPos - iFirstChar ).c_str(), iLength, &rc2, m_dwDrawTextFlags | DT_VCENTER | DT_CALCRECT );
      if ( iLength == 0 )
      {
        m_iCaretX = rc2.left;
      }
      else
      {
        m_iCaretX = rc2.right;
      }

      SelectObject( hdc, holdFont );

      ReleaseDC( hdc );



      POINT pt;

      pt.x = m_iCaretX;
      pt.y = 2;

      if ( GetFocus() == GetSafeHwnd() )
      {
        SetCaretPos( pt );
        ShowCaret();
      }
    }

    bool HasSelection()
    {

      if ( m_iSelectionAnchor == -1 )
      {
        return false;
      }
      if ( m_iSelectionAnchor == m_iTrueCursorPos )
      {
        return false;
      }
      return true;

    }

    void OnKillFocus()
    {
      m_bMouseDownInEdit = false;
      DestroyCaret();
      ClipCursor( NULL );

      if ( GetCapture() == GetSafeHwnd() )
      {
	      ReleaseCapture();
      }

      Invalidate();
    }

    void OnSetFocus()
    {
      CreateSolidCaret( 1, 12 );
      RecalcCaretPos();
      Invalidate();
    }

    void OnLButtonDown( UINT nFlags, POINT point )
    {
      if ( GetCapture() != GetSafeHwnd() )
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

      // Cursor setzen
      m_bMouseDownInEdit = true;
      m_iTrueCursorPos = CursorPosFromPoint( point );
      m_iSelectionAnchor = m_iTrueCursorPos;
      RecalcCaretPos();
      Invalidate();

	    Default();
    }

    void OnLButtonUp( UINT nFlags, POINT point )
    {
	    Default();

      m_bMouseDownInEdit = false;

      ClipCursor( NULL );
	    ReleaseCapture();
    }

    int CursorPosFromPoint( POINT& pt )
    {

      RECT  rc;

      GetWindowRect( &rc );
      ScreenToClient( &rc );

      InflateRect( &rc, -2, 0 );

      if ( !PtInRect( &rc, pt ) )
      {
        if ( pt.x <= rc.left )
        {
          return 0;
        }
        if ( pt.x >= rc.right )
        {
          return (int)m_strContent.length();
        }
        return m_iTrueCursorPos;
      }

      InflateRect( &rc, 2, 0 );

      int   iNewCursorPos = -1;

      int   iX1 = 4,
            iX2 = 4;

      HDC   hdc = GetDC();


      HFONT   holdFont = (HFONT)SelectObject( hdc, GetStockObject( DEFAULT_GUI_FONT ) );

      if ( m_strContent.empty() )
      {
        return 0;
      }

      int     iFirstChar = m_iFirstVisibleChar,
              iLength = 1;

      while ( true )
      {
        DrawText( hdc, m_strContent.substr( iFirstChar, iLength ).c_str(), iLength, &rc, m_dwDrawTextFlags | DT_VCENTER | DT_CALCRECT );

        iX2 = rc.right;

        if ( pt.x < iX1 )
        {
          iNewCursorPos = iLength - 1;
          break;
        }
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
        if ( iLength >= (int)m_strContent.length() )
        {
          iNewCursorPos = (int)m_strContent.length();
          break;
        }
      }
      SelectObject( hdc, holdFont );

      ReleaseDC( hdc );

      return iNewCursorPos;
    }

    void OnMouseMove( UINT nFlags, POINT point )
    {
      if ( nFlags & MK_LBUTTON )
      {
        if ( m_bMouseDownInEdit )
        {
          // der User draggt in der Edit-Control rum
          m_iTrueCursorPos = CursorPosFromPoint( point );
          Invalidate();
        }
      }
    }

    void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
    {
      if ( nChar == VK_LEFT )
      {
        if ( ( HasSelection() )
        &&   ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) )
        {
          if ( m_iTrueCursorPos > m_iSelectionAnchor )
          {
            m_iTrueCursorPos = m_iSelectionAnchor;
          }
          m_iSelectionAnchor = m_iTrueCursorPos;
          Invalidate();
          RecalcCaretPos();
        }
        else if ( m_iTrueCursorPos > 0 )
        {
          m_iTrueCursorPos--;
          RecalcCaretPos();
          if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
          {
            m_iSelectionAnchor = m_iTrueCursorPos;
          }
          Invalidate();
        }
      }
      else if ( nChar == VK_RIGHT )
      {
        if ( ( HasSelection() )
        &&   ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) )
        {
          if ( m_iSelectionAnchor > m_iTrueCursorPos )
          {
            m_iTrueCursorPos = m_iSelectionAnchor;
          }
          m_iSelectionAnchor = m_iTrueCursorPos;
          Invalidate();
          RecalcCaretPos();
        }
        else if ( m_iTrueCursorPos < GetWindowTextLength() )
        {
          m_iTrueCursorPos++;
          RecalcCaretPos();
          if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
          {
            m_iSelectionAnchor = m_iTrueCursorPos;
          }
          Invalidate();
        }
      }
      else if ( nChar == VK_HOME )
      {
        m_iTrueCursorPos = 0;
        RecalcCaretPos();
        if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
        {
          m_iSelectionAnchor = m_iTrueCursorPos;
        }
        Invalidate();
      }
      else if ( nChar == VK_END )
      {
        m_iTrueCursorPos = (int)m_strContent.length();
        RecalcCaretPos();
        if ( ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) == 0 )
        {
          m_iSelectionAnchor = m_iTrueCursorPos;
        }
        Invalidate();
      }
      else if ( nChar == VK_DELETE )
      {
        if ( m_iTrueCursorPos < (int)m_strContent.length() )
        {
          GR::String     strNew = "";

          if ( m_iTrueCursorPos == 0 )
          {
            strNew = m_strContent.substr( 1 );
          }
          else
          {
            // linker Rest
            strNew = m_strContent.substr( 0, m_iTrueCursorPos );

            // rechter Rest
            if ( m_iTrueCursorPos + 1 < (int)m_strContent.length() )
            {
              strNew += m_strContent.substr( m_iTrueCursorPos + 1 );
            }
          }
          m_strContent = strNew;
          RecalcCaretPos();
          Invalidate();
        }
      }
    }


};


#endif // GR_UI_EDIT_H