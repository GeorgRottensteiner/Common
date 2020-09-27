#if !defined(AFX_ODLISTBOX_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_ODLISTBOX_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ODListBox.h : Header-Datei
//


#include <vector>
#include <string>

#include <GR/GRTypes.h>



class CODListBox : public CWnd
{

  protected:

    #define ODLISTBOX_CLASS_NAME _T( "GRODListBox" )

    static BOOL hasclass;
    static BOOL RegisterMe();

    class CODItem
    {
      public:

      GR::String        m_strItem;

        DWORD_PTR       m_dwItemData;
    };

    BOOL                              m_bHasFocus,
                                      m_bOwnerDraw;

    std::vector<CODItem*>             m_vectItems;

    int                               m_iOffset,
                                      m_iItemWidth,
                                      m_iItemHeight,
                                      m_iSelectedItem,
                                      m_iFocusedItem,
                                      m_iItemsPerLine;

    DWORD                             m_dwID;

    HFONT                             m_hFont;



    // Helper-Functions

    void DoLButtonDown( CPoint &point );
    void AdjustScrollBars();


  // Konstruktion
  public:
	  CODListBox();

  // Attribute
  public:

  // Operationen
  public:


    BOOL Create( RECT &rc, CWnd *wndParent, int iID );

    void SetItemSize( int iWidth, int iHeight );

    void SetCurSel( int iItem );
    int GetCurSel( ) const;

    void ScrollIntoView();

    size_t AddString( const GR::Char* Item );
    size_t DeleteString( size_t nIndex );

    size_t GetText( int iIndex, LPTSTR lpszBuffer ) const;
    GR::String GetText( int iIndex ) const;

    int SetItemData( size_t iIndex, DWORD_PTR dwData );
    DWORD_PTR GetItemData( size_t iIndex ) const;

    int GetItemRect( size_t nIndex, LPRECT lpRect ) const;
    size_t ItemFromPoint( CPoint pt, BOOL& bOutside ) const;
    size_t ItemFromPoint( CPoint pt ) const;

    void              InvalidateItem( int nIndex );

    BOOL              PreTranslateMessage( MSG* pMsg );

    size_t GetCount() const;

    void ResetContent();

    void SetOwnerDraw( BOOL bOD = TRUE );


  // Überschreibungen
	  // Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	  //{{AFX_VIRTUAL(CODListBox)
	  //}}AFX_VIRTUAL

  // Implementierung
  public:
	  virtual ~CODListBox();

	  // Generierte Nachrichtenzuordnungsfunktionen
  protected:
	  //{{AFX_MSG(CODListBox)
	  afx_msg void OnDestroy();
	  afx_msg void OnPaint();
	  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	  afx_msg UINT OnGetDlgCode();
	  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	  afx_msg void OnKillFocus(CWnd* pNewWnd);
	  afx_msg void OnSetFocus(CWnd* pOldWnd);
	  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSize( UINT nType, int cx, int cy );
	  //}}AFX_MSG
	  DECLARE_MESSAGE_MAP()
    virtual void PreSubclassWindow();
};


#endif // AFX_ODLISTBOX_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_
