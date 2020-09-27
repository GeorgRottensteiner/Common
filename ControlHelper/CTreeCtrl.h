#ifndef INCLUDE_CTREECTRL_H
#define INCLUDE_CTREECTRL_H



#include <ControlHelper\CWnd.h>

#include <commctrl.h>

#pragma comment( lib, "comctl32.lib" )



namespace WindowsWrapper
{

  class CCommonCtrlDummy
  {
    public:

    CCommonCtrlDummy()
    {
      InitCommonControls();
    }
  };

  static CCommonCtrlDummy    commoncontroldummy;

  class CTreeCtrl : public CWnd
  {

    // Constructors
    public:


    CTreeCtrl();
    virtual ~CTreeCtrl();

    BOOL              CreateEx( DWORD dwExStyle, DWORD dwStyle, const RECT& rc, HWND hwndParent, UINT nID );

    HTREEITEM         InsertItem( LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST );
    HTREEITEM         InsertItem( GR::String& strItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST );
    HTREEITEM         InsertItemAndData( LPCTSTR lpszItem, DWORD dwData = 0, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST );
    HTREEITEM         InsertItemAndData( GR::String& strItem, DWORD dwData = 0, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST );

    UINT              GetCount() const;

    BOOL              SelectItem( HTREEITEM hItem );
    HTREEITEM         GetSelectedItem();
    HTREEITEM         GetRootItem() const;
    HTREEITEM         GetParentItem( HTREEITEM hItem );
    HTREEITEM         GetChildItem( HTREEITEM hItem );
    HTREEITEM         GetNextItem( HTREEITEM hItem );
    HTREEITEM         GetNextSiblingItem( HTREEITEM hItem );

    BOOL              Expand( HTREEITEM hItem, UINT nCode );

    BOOL              ItemHasChildren( HTREEITEM hItem ) const;

    BOOL              SetItem( HTREEITEM hItem, UINT nMask, LPCTSTR lpszItem, int nImage,
                               int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam );
    BOOL              SetItemText( HTREEITEM hItem, const GR::String& strText );
    BOOL              SetItemData( HTREEITEM hItem, DWORD_PTR dwData );

    int GetItemLevel( HTREEITEM hItem );
    DWORD_PTR         GetItemData( HTREEITEM hItem );
    GR::String    GetItemText( HTREEITEM hItem ) const;

    BOOL              DeleteItem( HTREEITEM hItem );
    BOOL DeleteAllItems();

    // Image-List
    HIMAGELIST SetImageList( HIMAGELIST hImageList, int nImageListType );
    BOOL SetItemImage( HTREEITEM hItem, int nImage, int nSelectedImage );

    BOOL              GetItemRect( HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly );

    HTREEITEM         HitTest( POINT& pt, UINT* pFlags = NULL ) const;

    HTREEITEM         FindItem( const GR::String& strText, HTREEITEM hItemSearchRoot = TVI_ROOT );

    // divers
    BOOL Attach( HWND hwndNew );


  };

}

#endif //__INCLUDE_CTREECTRL_H__



