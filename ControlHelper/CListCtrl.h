#ifndef INCLUDE_CLISTCTRL_H
#define INCLUDE_CLISTCTRL_H



#include "CWnd.h"

#include <commctrl.h>



namespace WindowsWrapper
{

  struct __POSITION
  {
  };
  typedef __POSITION* POSITION;

  class CListCtrl : public CWnd
  {

    // Constructors
    public:


    CListCtrl();

    BOOL                  CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent, UINT nID );
    //BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );

  // Attributes
    COLORREF GetBkColor() const;
    BOOL SetBkColor( COLORREF cr );
    int                   GetItemCount() const;
    int                   GetColumnCount() const;
    int                   GetItemImage( int nItem ) const;

    BOOL GetItem( LVITEM* pItem ) const;
    BOOL SetItem( const LVITEM* pItem );
    BOOL SetItem( int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
                  int nImage, UINT nState, UINT nStateMask, LPARAM lParam );
    UINT GetCallbackMask() const;
    BOOL SetCallbackMask( UINT nMask );
    /*
    POSITION GetFirstSelectedItemPosition() const;
    int GetNextSelectedItem(POSITION& pos) const;
    */
    BOOL GetItemRect( int nItem, LPRECT lpRect, UINT nCode ) const;
    BOOL SetItemPosition( int nItem, POINT pt );
    BOOL GetItemPosition( int nItem, LPPOINT lpPoint ) const;
    int GetStringWidth( LPCTSTR lpsz ) const;
    //CEdit* GetEditControl() const;
    BOOL          GetColumn( int nCol, LVCOLUMN* pColumn ) const;
    /*

    BOOL SetColumn(int nCol, const LVCOLUMN* pColumn);
    */
    int GetColumnWidth( int nCol ) const;
    BOOL SetColumnWidth( int nCol, int cx );
    BOOL GetViewRect( LPRECT lpRect ) const;
    COLORREF GetTextColor() const;
    BOOL SetTextColor( COLORREF cr );
    COLORREF GetTextBkColor() const;
    BOOL SetTextBkColor( COLORREF cr );
    int GetTopIndex() const;
    int GetCountPerPage() const;
    BOOL GetOrigin( LPPOINT lpPoint ) const;
    //BOOL SetItemState(int nItem, LVITEM* pItem);
    BOOL SetItemState( int nItem, UINT nState, UINT nMask );
    UINT GetItemState( int nItem, UINT nMask ) const;
    //CString GetItemText(int nItem, int nSubItem) const;
    GR::String GetItemText( int nItem, int nSubItem ) const;
    int GetItemText( int nItem, int nSubItem, LPTSTR lpszText, int nLen ) const;

    BOOL            SetItemText( int nItem, int nSubItem, LPCSTR lpszText );
    BOOL            SetItemText( int nItem, int nSubItem, const GR::String& Text );

    BOOL            SetItemImage( int iItem, int nImage );

    void SetItemCount( int nItems );
    BOOL SetItemData( int nItem, DWORD_PTR dwData );
    DWORD_PTR GetItemData( int nItem ) const;
    UINT GetSelectedCount() const;
    BOOL SetColumnOrderArray( int iCount, LPINT piArray );
    BOOL GetColumnOrderArray( LPINT piArray, int iCount = -1 );

    HCURSOR GetHotCursor();
    HCURSOR SetHotCursor( HCURSOR hc );
    BOOL GetSubItemRect( int iItem, int iSubItem, RECT& rcTarget );
    int GetHotItem();
    int SetHotItem( int iIndex );
    int GetSelectionMark();
    int SetSelectionMark( int iIndex );
    DWORD GetExtendedStyle();
    DWORD SetExtendedStyle( DWORD dwNewStyle );
    //int SubItemHitTest(LPLVHITTESTINFO pInfo);
    void SetWorkAreas( int nWorkAreas, LPRECT lpRect );
    BOOL SetItemCountEx( int iCount, DWORD dwFlags = LVSICF_NOINVALIDATEALL );
    //CSize ApproximateViewRect(CSize sz = CSize(-1, -1), int iCount = -1) const;
    //BOOL GetBkImage(LVBKIMAGE* plvbkImage) const;
    DWORD GetHoverTime() const;
    void GetWorkAreas( int nWorkAreas, LPRECT prc ) const;
    BOOL SetBkImage( HBITMAP hbm, BOOL fTile = TRUE, int xOffsetPercent = 0, int yOffsetPercent = 0 );
    BOOL SetBkImage( LPTSTR pszUrl, BOOL fTile = TRUE, int xOffsetPercent = 0, int yOffsetPercent = 0 );
    //BOOL SetBkImage(LVBKIMAGE* plvbkImage);
    DWORD SetHoverTime( DWORD dwHoverTime = (DWORD)-1 );
    UINT GetNumberOfWorkAreas() const;
    BOOL GetCheck( int nItem ) const;
    BOOL SetCheck( int nItem, BOOL fCheck = TRUE );

    // Operations
    int AppendItem( const GR::String& strItem, int iImage = 0, GR::up dwItemData = 0 );
    int InsertItem( const LVITEM* pItem );
    int InsertItem( int nItem, const GR::String& strItem );
    int InsertItem( int nItem, const GR::String& strItem, int nImage, GR::up dwItemData = 0 );
    BOOL DeleteItem( int nItem );
    BOOL DeleteAllItems();
    //int FindItem(LVFINDINFO* pFindInfo, int nStart = -1) const;
    //int HitTest(LVHITTESTINFO* pHitTestInfo) const;
    int HitTest( POINT& pt, UINT* pFlags = NULL ) const;
    BOOL EnsureVisible( int nItem, BOOL bPartialOK );
    //BOOL Scroll(CSize size);
    BOOL RedrawItems( int nFirst, int nLast );
    BOOL Arrange( UINT nCode );
    //CEdit* EditLabel(int nItem);
    //int InsertColumn(int nCol, const LVCOLUMN* pColumn);
    int InsertColumn( int nCol, LPCSTR lpszColumnHeading,
                      int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1 );
    BOOL DeleteColumn( int nCol );
    //CImageList* CreateDragImage(int nItem, LPPOINT lpPoint);
    BOOL Update( int nItem );
    //BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD dwData);

    bool                GetHeaderControlRect( RECT& rcHeader ) const;

    // Overridables
    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
    {
    }

    // Implementation
    public:
    int InsertItem( UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState,
                    UINT nStateMask, int nImage, LPARAM lParam );
    virtual ~CListCtrl();

    HIMAGELIST            SetImageList( HIMAGELIST hImageList, int nImageListType );

    POSITION              GetFirstSelectedItemPosition() const;
    int                   GetNextSelectedItem( POSITION& pos ) const;

    int                   GetNextItem( int nItem, int nFlags ) const;

    protected:
    void RemoveImageList( int nImageList );
    virtual BOOL OnChildNotify( UINT, WPARAM, LPARAM, LRESULT* )
    {
      return TRUE;
    }
  };

}

#endif // INCLUDE_CLISTCTRL_H



