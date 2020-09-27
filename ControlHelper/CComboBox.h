#ifndef INCLUDE_CCOMBOBOX_H
#define INCLUDE_CCOMBOBOX_H



#include <ControlHelper\CWnd.h>



namespace WindowsWrapper
{

  class CComboBox : public CWnd
  {

    // Constructors
    public:


    CComboBox();
    virtual ~CComboBox();


    // Items
    int           AddString( LPCSTR lpszString );
    int           AddString( LPCSTR lpszString, DWORD dwItemData );
    int           AddString( const GR::String& strText );
    int           SetItemData( int nIndex, DWORD dwItemData );
    DWORD_PTR     GetItemData( int nIndex ) const;

    LRESULT       GetCount() const;

    // Selektion
    int           SelectItem( int nSelect );
    void          SetCurSelByItemData( DWORD_PTR dwFindItemData );
    int           GetCurSel() const;
    DWORD_PTR     GetCurSelItemData() const;

    BOOL          SetEditSel( int nStartChar, int nEndChar );

    LRESULT       FindString( int nIndexStart, LPCTSTR lpszFind );
    LRESULT       FindStringExact( int nIndexStart, LPCTSTR lpszFind );
    GR::String    GetLBText( int nIndex );

    void          ResetContent();

    BOOL          GetDroppedState();

    void          ShowDropDown( BOOL bShowIt = 1 );

    HWND          GetEditHandle();

  };

}

#endif // INCLUDE_CCOMBOBOX_H



