#if !defined(AFX_SCRIPTEDITCTRL_H__6A93CF60_D8E1_11D4_9D7D_0001029EF128__INCLUDED_)
#define AFX_SCRIPTEDITCTRL_H__6A93CF60_D8E1_11D4_9D7D_0001029EF128__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include <vector>
#include <string>

#include <windows.h>

#include <ControlHelper/CWnd.h>


namespace SCRIPT_EDIT_CONTROL
{
  namespace SYNTAX_TYPE
  {
    const DWORD DEFAULT                 = 0;
    const DWORD KEYWORD                 = 1;
  }

  namespace FLAG
  {
    const DWORD REPLACE_TABS            = 0x0001;
  }

  namespace COLOR
  {
    const DWORD BACKGROUND              = 0;      // Hintergrund
    const DWORD TEXT                    = 1;      // Default-Text
    const DWORD SELECTED_BACKGROUND     = 2;      // selektierter Hintergrund
    const DWORD SELECTED_TEXT           = 3;      // selektierter Text
    const DWORD DISABLED_BACKGROUND     = 4;      // deaktivierter Hintergrund
    const DWORD DISABLED_TEXT           = 5;      // deaktivierter Text
    const DWORD STRING_TEXT             = 6;      // String-Text
    const DWORD COMMENT                 = 7;      // Kommentar-Farbe
  }
  const DWORD MAX_LINE_LENGTH           = 1024;
}



class CScriptEditCtrl : public WindowsWrapper::CWnd
{

  class CSyntaxColorEntry
  {
    public:

      GR::String          KeyWord;

      COLORREF            ColorKey;
  };



  friend class CSyntaxColorEntry;



  public:

    static BOOL               m_Registered;
    static BOOL               Register();


	  CScriptEditCtrl();



  protected:

    HFONT                             m_hFont;

    RECT                              m_rectRC;

    bool                              m_Focus,
                                      m_Enabled,
                                      m_Created,
                                      m_Selection,
                                      m_ShowCaret,
                                      m_SelectingByMouse,
                                      m_Modified;

    DWORD                             m_TabSpacing,
                                      m_Flags;

    int                               m_FirstLine,
                                      m_FirstCharacter,

                                      m_VisibleCharsPerLine,
                                      m_VisibleLines,

                                      m_CursorWantedX,

                                      m_SelectionAnchorX,
                                      m_SelectionAnchorY,

                                      m_CaretX,
                                      m_CursorX,
                                      m_CursorY,
                                      m_CharWidth,
                                      m_CharHeight;

    std::vector<GR::String>           m_Text;

    std::vector<CSyntaxColorEntry>    m_SyntaxEntry;



  public:

  // Überschreibungen
	  // Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	  //{{AFX_VIRTUAL(CScriptEditCtrl)
	  //}}AFX_VIRTUAL

    BOOL Create( int iX, int iY, int iWidth, int iHeight,
                HWND hwndParent,
                int iID,
                DWORD dwFlags = SCRIPT_EDIT_CONTROL::FLAG::REPLACE_TABS );

    void Reset();

    BOOL SetWindowPos( HWND hwndAfter, int x, int y, int cx, int cy, UINT nFlags );

    void Enable( BOOL bEnable = TRUE );
    BOOL IsEnabled();

    BOOL LoadFromFile( const char *szFileName );
    void SaveToFile( const char *szFileName );

    GR::String GetLine( DWORD dwLine );
    DWORD GetLineCount();

    void GetWindowText( GR::String& strText ) const;

    int GetCursorX() const;
    int GetCursorY() const;
    int GetSelectionAnchorX() const;
    int GetSelectionAnchorY() const;

    void SetPosition( int iX, int iY );
    void SetSelection( int iX1, int iY1, int iX2, int iY2 );

    void ClipCoordinatesToText( int *iX, int *iY );

    void AppendText( const GR::String& strText );
    void InsertChar( char cChar, BOOL bRedraw = TRUE );
    void InsertString( const GR::String& strNew, BOOL bRedraw = TRUE, BOOL bSelect = FALSE );

    void ReplaceSelection( GR::String& strNew, BOOL bSelect = TRUE );

    void AddKeyWord( const char *szKeyWord, COLORREF colorDummy );
    void SetTabSpacing( DWORD dwSpaceCount );

    void SetColor( DWORD dwColor, COLORREF color );

    BOOL FindString( GR::String, int*, int* );

    void AdjustScrollBars();
    BOOL ScrollIntoView();
    void ScrollToBottom();

    BOOL IsSelection()
    {
      return m_Selection;
    }
    GR::String         GetSelection();
    BOOL                IsInsideSelection( int iX, int iY );
    void                RemoveSelection();

    DWORD GetMaxLength();



    BOOL inline Modified()
    {
      return m_Modified;
    }



    void inline Modified( BOOL bMod )
    {
      m_Modified = !!bMod;
    }



	  virtual ~CScriptEditCtrl();


    virtual void Cut();
    virtual void Copy();
    virtual void Paste();


	  // Generierte Nachrichtenzuordnungsfunktionen
  protected:

    std::vector <COLORREF>            m_vectColor;

    void DrawCursor();

    void UpdateCaret();
    void UpdateSelectionAnchor( BOOL bTestShift );

    BOOL IsAbove( int iX1, int iY1, int iX2, int iY2 );
    std::vector<GR::String>::iterator GetStringIteratorAt( int iY );

    DWORD GetTrueLength( const GR::String& strLine );
    DWORD GetTextPosition( int iTrueX, int iY );
    DWORD GetVisiblePosition( int iX, int iY );

    void GetMouseCursorPos( POINT point, int *iX, int *iY );


    // Überschreibbare Funktionen
    virtual void FillColorVector( const GR::String& strLine, int iCurrentLine, std::vector<COLORREF> *vectColors );

	  void OnPaint();
	  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	  void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	  void OnLButtonDown(UINT nFlags, POINT point);
	  void OnSetFocus( HWND hwndOld );
	  void OnKillFocus( HWND hwndNew );
	  UINT OnGetDlgCode();
	  void OnVScroll(UINT nSBCode, UINT nPos );
	  void OnHScroll(UINT nSBCode, UINT nPos );
	  BOOL OnEraseBkgnd( HDC hdc );
    void OnSetCursor( HWND hWnd, UINT nHitTest, UINT message );
	  void OnMouseMove(UINT nFlags, POINT point);
	  void OnLButtonUp(UINT nFlags, POINT point);
	  void OnTimer(UINT nIDEvent);
	  void OnLButtonDblClk(UINT nFlags, POINT point);
	  void OnClose();

    void FillSolidRect( HDC hdc, int iX, int iY, int iCX, int iCY, COLORREF color );
    void FillSolidRect( HDC hdc, RECT& rc, COLORREF color );

    virtual LRESULT               WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

};


#endif // AFX_SCRIPTEDITCTRL_H__6A93CF60_D8E1_11D4_9D7D_0001029EF128__INCLUDED_
