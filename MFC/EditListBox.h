#if !defined(AFX_EditListBox_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_EditListBox_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditListBox.h : Header-Datei
//


#pragma warning(disable:4786)

#include <vector>
#include <string>

#include <MFC\PopupEdit.h>


/////////////////////////////////////////////////////////////////////////////
// Fenster CEditListBox

class CEditListBox : public CWnd
{

  #define EDITLISTBOX_CLASS_NAME _T( "EditListBox" )
  #define EDITLISTBOX_MESSAGE _T( "EditListBoxItemChanged" )

protected:

  class CELItem
  {
    public:

      GR::String     m_strItem;

      DWORD           m_dwItemData;
  };

  BOOL                              m_bHasFocus,
                                    m_bOwnerDraw;

  bool                              m_bEditLabel;

  CPopupEdit                        *m_pPopupEdit;

  std::vector<CELItem*>             m_vectItems;

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
  void BeginLabelEdit();
  void EndLabelEdit();


// Konstruktion
public:

    static BOOL hasclass;
    static BOOL RegisterMe();

    static DWORD     dwELBM;

	  CEditListBox();


    BOOL Create( RECT &rc, CWnd *wndParent, int iID );

    void SetItemSize( int iWidth, int iHeight );

    void SetCurSel( int iItem );
    int GetCurSel( ) const;

    void ScrollIntoView();

    void ResetContent();
    int AddString( const char *szItem );
    int DeleteString( UINT nIndex );

    void SetItemText( int iIndex, const char *szBuffer );
    int GetText( int iIndex, LPTSTR lpszBuffer ) const;

    int SetItemData( int iIndex, DWORD dwData );
    DWORD GetItemData( int iIndex ) const;

    int GetItemRect( int nIndex, LPRECT lpRect ) const;
    UINT ItemFromPoint( CPoint pt, BOOL& bOutside ) const;
    UINT ItemFromPoint( CPoint pt ) const;

    int GetCount() const;

    void SetOwnerDraw( BOOL bOD = TRUE );


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CEditListBox)
	//}}AFX_VIRTUAL

// Implementierung
public:
	virtual ~CEditListBox();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CEditListBox)
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
  afx_msg LRESULT OnPopupEditChange( WPARAM wParam, LPARAM lParam );
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_EditListBox_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_
