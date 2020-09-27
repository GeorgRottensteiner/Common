#ifndef CCHECKLISTBOX_H
#define CCHECKLISTBOX_H



#include <vector>
#include <string>

#include "CWnd.h"



namespace WindowsWrapper
{

  class CCheckListBox : public CWnd
  {

    protected:

    class CCLItem
    {
      public:

      GR::String      m_Item;

      DWORD_PTR       m_ItemData;

      bool            m_Checked,
                      m_CheckAble;

      CCLItem()
        : m_Checked( false ),
          m_CheckAble( true )
      {
      }
    };

    bool                              m_HasFocus,
                                      m_OwnerDraw;

    std::vector<CCLItem*>             m_Items;

    size_t                            m_SelectedItem,
                                      m_FocusedItem,
                                      m_Offset;

    int                               m_ItemWidth,
                                      m_ItemHeight,
                                      m_ItemsPerLine;

    DWORD                             m_ID;

    HFONT                             m_hFont;

    HBITMAP                           m_hbmCheckBox;


    // Helper-Functions
    void DoLButtonDown( POINT &point );
    void AdjustScrollBars();

    public:


    CCheckListBox();
    virtual ~CCheckListBox();

    // Überladung
    virtual BOOL Attach( HWND hwndNew );
    virtual HWND CCheckListBox::Detach();

    // Items
    BOOL            Create( DWORD dwStyle, RECT &rc, HWND hwndParent, int iID );
    BOOL            CreateEx( DWORD dwExStyle, DWORD dwStyle, RECT& rc, HWND hwndParent, int iID );

    void SetItemSize( int iWidth, int iHeight );

    void SetCurSel( size_t iItem );
    size_t GetCurSel() const;

    void SetCheck( size_t nIndex, int nCheck );
    int GetCheck( size_t nIndex );

    void ScrollIntoView();

    int AddString( const char *szItem, DWORD_PTR dwItemData = 0, bool bCheckAble = true );
    int AddString( const GR::String& strItem, DWORD_PTR dwItemData = 0, bool bCheckAble = true );
    int DeleteString( UINT nIndex );

    void            ResetContent();

    int GetText( size_t iIndex, LPSTR lpszBuffer ) const;
    int GetText( size_t iIndex, GR::String& strResult ) const;

    int SetItemData( size_t iIndex, DWORD dwData );
    DWORD_PTR GetItemData( size_t iIndex ) const;

    int GetItemRect( size_t nIndex, LPRECT lpRect ) const;
    UINT ItemFromPoint( POINT pt, BOOL& bOutside ) const;
    UINT ItemFromPoint( POINT pt ) const;

    size_t GetCount() const;

    void SetOwnerDraw( BOOL bOD = TRUE );


    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    void OnDestroy();
    void OnPaint();
    BOOL OnEraseBkgnd( HDC hdc );
    void OnLButtonDown( UINT nFlags, POINT pt );
    void OnLButtonDblClk( UINT nFlags, POINT pt );
    void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );

    UINT OnGetDlgCode();

    void OnSetFocus( HWND hwndOldWindow );
    void OnKillFocus( HWND hwndNewWindow );

    void OnVScroll( UINT nSBCode, UINT nPos, HWND hwndScrollBar );

  };

}

#endif // CCHECKLISTBOX_H



