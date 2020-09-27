#if !defined(AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomPlacesBar.h : Header-Datei
//

#include "stdafx.h"

#include <vector>
#include <string>



/////////////////////////////////////////////////////////////////////////////
// Fenster CODListBox

class CCustomPlacesBar : public CWnd
{

  protected:

  struct tPBItem
  {
    public:

      GR::String     m_strItem;

      HICON           m_hIcon,
                      m_hIconGreyed;

      DWORD           m_dwItemData;


      tPBItem() :
        m_strItem( "" ),
        m_hIcon( NULL ),
        m_hIconGreyed( NULL ),
        m_dwItemData( 0 )
      {
      }

      ~tPBItem()
      {
        if ( m_hIconGreyed )
        {
          DeleteObject( m_hIconGreyed );
          m_hIconGreyed = NULL;
        }
      }


  };

  BOOL                              m_bHasFocus;

  std::vector<tPBItem>              m_vectItems;

  size_t                            m_iSelectedItem,
                                    m_iFocusedItem,
                                    m_iMouseOverItem,
                                    m_iOffset;

  int                               m_iItemWidth,
                                    m_iItemHeight,
                                    m_iTimerID;

  DWORD                             m_dwCPStyles;

  HFONT                             m_hFont;

  CToolTipCtrl                      m_ToolTip;



  // Helper-Functions
  void DoLButtonDown( CPoint &point );
  void AdjustScrollBars();


  public:

  enum eCustomPlacesStyles
  {
    PBS_DEFAULT     = 0,
    PBS_SMALL_ITEMS = 0x00000001,
  };

  #define CUSTOMPLACESBAR_CLASS_NAME _T( "CustomPlacesBar" )

  static BOOL hasclass;
  static BOOL RegisterMe();


  CCustomPlacesBar();
  BOOL              Create( RECT &rc, CWnd *wndParent, int iID );

  void              Initialize( DWORD dwStyles = PBS_DEFAULT, int iItemHeight = -1 );

  void              SetItemSize( int iWidth, int iHeight );

  void              SetCurSel( size_t iItem );
  size_t            GetCurSel( ) const;

  void              ScrollIntoView( size_t iItem = -1 );

  size_t            AddString( const char *szItem, HICON hIcon = NULL, DWORD dwItemData = 0 );
  size_t            DeleteString( size_t nIndex );
  size_t            GetCount() const;

  void              DeleteAllItems();

  size_t            GetText( size_t iIndex, LPTSTR lpszBuffer ) const;

  int               SetItemData( size_t iIndex, DWORD dwData );
  DWORD             GetItemData( size_t iIndex ) const;

  int               GetItemRect( size_t nIndex, LPRECT lpRect ) const;
  size_t            ItemFromPoint( CPoint pt, BOOL& bOutside ) const;
  size_t            ItemFromPoint( CPoint pt ) const;

    size_t            GetItemCount() const;

  void              InvalidateItem( size_t nIndex );



  void ResetContent();

  // Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CCustomPlacesBar)
	//}}AFX_VIRTUAL

  // Implementierung
  public:
	virtual ~CCustomPlacesBar();

  protected:
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
	DECLARE_MESSAGE_MAP()
  public:
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnEnable(BOOL bEnable);
  virtual BOOL PreTranslateMessage(MSG* pMsg);

};

/////////////////////////////////////////////////////////////////////////////

// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_
