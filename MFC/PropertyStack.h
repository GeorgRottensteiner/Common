#if !defined(AFX_PROPERTYSTACK_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_PROPERTYSTACK_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//

#include "stdafx.h"

#include <vector>
#include <string>

#include <GR/GRTypes.h>



class CPropertyStack : public CWnd
{

  public:

    enum    ePropertyType
    {
      E_INVALID   = 0,
      E_TEXT,
      E_STATIC_TEXT,
      E_NUMBER,
      E_COLOR,
      E_FILE,
      E_COMBO,
      E_CHECKBOX,
      E_CUSTOM,
    };


  protected:

    struct tPropertyItem
    {
      public:

        typedef std::pair<GR::String,GR::up>   tComboPair;


        ePropertyType               m_Type;

        GR::String                 m_strDescription;

        int                         m_iValue;

        std::vector<tComboPair>     m_vectItems;

        GR::up                      m_dwItemData;

        bool                        m_bLabelEditable;


        tPropertyItem( ePropertyType eType ) :
          m_strDescription( "" ),
          m_iValue( 0 ),
          m_dwItemData( 0 ),
          m_Type( eType ),
          m_bLabelEditable( false )
        {
        }
    };

    bool                              m_bHasFocus;

    bool                              m_bEditingItem,
                                      m_bEditingLabel;

    CEdit                             m_EditPopup;

    CComboBox                         m_ComboPopup;

    std::vector<tPropertyItem>        m_vectItems;

    size_t                            m_iSelectedItem,
                                      m_iFocusedItem,
                                      m_iVisibleItems,
                                      m_iEditItem,
                                      m_iOffset;

    int                               m_iItemWidth,
                                      m_iItemHeight,
                                      m_iTimerID;

    HFONT                             m_hFont;



    // Helper-Functions
    void                              DoLButtonDown( CPoint &point );
    void                              AdjustScrollBars();

    BOOL                              NotifyParent( WPARAM wParam, LPARAM lParam = 0 );


  public:

    enum eNotifyType
    {
      NM_ITEM_SELECTED,
      NM_BEGIN_EDIT,
      NM_END_EDIT,
      NM_ITEM_CHANGED,
      NM_EDITING,
      NM_BEGIN_LABEL_EDIT,
      NM_END_LABEL_EDIT,
      NM_LABEL_CHANGED,
      NM_LABEL_EDITING,
    };


    #define PROPERTYSTACK_CLASS_NAME _T( "GRPropertyStack" )

    static const char szNotifyMessage[];

    static GR::u32      m_dwNotifyMessage;

    static BOOL hasclass;
    static BOOL RegisterMe();


    CPropertyStack();
    BOOL              Create( RECT &rc, CWnd *wndParent, int iID );

    void              Initialize( int iItemHeight = -1 );
    void              SetItemHeight( int iHeight );

    void              SetCurSel( size_t iItem );
    size_t            GetCurSel( ) const;

    void              ScrollIntoView( size_t iItem = -1 );

    size_t            AddString( const GR::String& strDesc, const GR::String& strItem, GR::up dwItemData = 0 );
    size_t            AddStatic( const GR::String& strDesc, const GR::String& strItem, bool bBold, GR::up dwItemData = 0 );
    size_t            AddNumber( const GR::String& strDesc, int iValue, GR::up dwItemData = 0 );
    size_t            AddColor( const GR::String& strDesc, GR::u32 dwColor, GR::up dwItemData = 0 );
    size_t            AddFile( const GR::String& strDesc, const GR::String& strItem, const GR::String& Filter, GR::up dwItemData = 0 );
    size_t            AddCombo( const GR::String& strDesc, GR::up dwItemData = 0 );
    size_t            AddComboItem( size_t iComboIndex, const GR::String& strDesc, GR::up dwItemData = 0 );
    size_t            AddCheckBox( const GR::String& strDesc, bool bChecked, GR::up dwItemData = 0 );
    size_t            AddCustom( const GR::String& strDesc, const GR::String& strItem, GR::up dwItemData = 0 );

    size_t            DeleteString( size_t nIndex );
    size_t            GetCount() const;

    void              SetLabelEditable( size_t iIndex, bool bEditable = true );

    GR::String       GetItemDesc( size_t iIndex ) const;
    GR::String       GetItemText( size_t iIndex ) const;
    int               GetItemValue( size_t iIndex ) const;

    int               SetItemText( size_t Index, const GR::String& Text );

    void              SetItemValue( size_t iIndex, int iValue );

    int               SetItemData( size_t iIndex, GR::up dwData );
    GR::up            GetItemData( size_t iIndex ) const;

    void              SetComboSelection( size_t iIndex, size_t iComboItemIndex );
    size_t            GetComboSel( size_t iIndex ) const;
    GR::up            GetComboItemData( size_t iIndex, size_t iComboItemIndex ) const;

    int               GetItemRect( size_t nIndex, LPRECT lpRect ) const;
    int               GetLineRect( size_t nIndex, LPRECT lpRect ) const;

    size_t            ItemFromPoint( CPoint pt, BOOL& bOutside ) const;
    size_t            ItemFromPoint( CPoint pt ) const;

    void              InvalidateItem( size_t nIndex );

    virtual void      DrawItem( CDC* pDC, RECT rc, tPropertyItem& Item );

    virtual void      EditItem( size_t iItem );
    virtual void      EditLabel( size_t iItem );



    void ResetContent();

  // Überschreibungen
	  // Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	  //{{AFX_VIRTUAL(CPropertyStack)
	  //}}AFX_VIRTUAL

  // Implementierung
  public:
	  virtual ~CPropertyStack();

  protected:
    afx_msg void OnDestroy();
	  afx_msg void OnPaint();
	  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	  afx_msg UINT OnGetDlgCode();
	  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSize( UINT nType, int cx, int cy );
	  DECLARE_MESSAGE_MAP()
  public:
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  public:
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

/////////////////////////////////////////////////////////////////////////////

// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PROPERTYSTACK_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_
