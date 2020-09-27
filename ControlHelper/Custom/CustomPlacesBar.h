#if !defined(AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_)
#define AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomPlacesBar.h : Header-Datei
//
#include <tchar.h>


#include <ControlHelper/CWnd.h>
#include <ControlHelper/CToolTipCtrl.h>

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
    void              DoLButtonDown( POINT& pt );
    void AdjustScrollBars();


  public:

    enum eCustomPlacesStyles
    {
      PBS_DEFAULT     = 0,
      PBS_SMALL_ITEMS = 0x00000001,
    };

    #define CUSTOMPLACESBAR_CLASS_NAME __T( "CustomPlacesBar" )

    static BOOL hasclass;
    static BOOL RegisterMe();


    CCustomPlacesBar();
    BOOL              Create( RECT &rc, HWND hwndParent, int iID );

    void              Initialize( DWORD dwStyles = PBS_DEFAULT, int iItemHeight = -1 );

    void              SetItemSize( int iWidth, int iHeight );

    void              SetCurSel( size_t iItem );
    size_t            GetCurSel( ) const;

    void              ScrollIntoView( size_t iItem = -1 );

    size_t            AddString( const char *szItem, HICON hIcon = NULL, DWORD dwItemData = 0 );
    size_t            DeleteString( size_t nIndex );
    size_t            GetCount() const;

    size_t            GetText( size_t iIndex, LPTSTR lpszBuffer ) const;

    int               SetItemData( size_t iIndex, DWORD dwData );
    DWORD             GetItemData( size_t iIndex ) const;

    int               GetItemRect( size_t nIndex, LPRECT lpRect ) const;
    size_t            ItemFromPoint( const POINT& pt, BOOL& bOutside ) const;
    size_t            ItemFromPoint( const POINT& pt ) const;

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
     void OnDestroy();
	   void OnPaint();
	   UINT OnGetDlgCode();
     void OnSize( UINT nType, int cx, int cy );
  public:
     void OnMouseMove(UINT nFlags, POINT point);
     void OnTimer(UINT nIDEvent);
    virtual BOOL PreTranslateMessage( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    virtual LRESULT     CCustomPlacesBar::WindowProc( UINT message, WPARAM wParam, LPARAM lParam );

};

/////////////////////////////////////////////////////////////////////////////

// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CUSTOMPLACESBAR_H__FDC5A824_04FB_11D6_AC02_00104B6795C0__INCLUDED_
