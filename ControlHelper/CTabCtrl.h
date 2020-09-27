#ifndef INCLUDE_CTABCTRL_H
#define INCLUDE_CTABCTRL_H



#include <commctrl.h>

#include <ControlHelper\CWnd.h>



namespace WindowsWrapper
{

  class CTabCtrl : public CWnd
  {

    // Constructors
    public:


    CTabCtrl();
    virtual ~CTabCtrl();

    BOOL          Create( DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID );

    BOOL          InsertItem( int nItem, const GR::String& Item, LPARAM lItemData = 0 );

    int           GetCurSel() const;

    void          AdjustRect( BOOL bLarger, LPRECT lpRect );

    int           GetItemCount() const;
    BOOL          GetItem( int nItem, TCITEM* pTabCtrlItem ) const;
    BOOL          GetItemRect( int nItem, LPRECT lpRect );

    int           HitTest( TCHITTESTINFO* pHitTestInfo ) const;

    int           SetCurSel( int nItem );

    LPARAM        GetItemData( int nItem ) const;

    BOOL          SetItemText( int nItem, const GR::String& Text );


  };

}

#endif //INCLUDE_CTABCTRL_H



