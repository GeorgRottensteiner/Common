#ifndef INCLUDE_CLISTBOX_H
#define INCLUDE_CLISTBOX_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CListBox : public CWnd
  {

    // Constructors
    public:


    CListBox();
    virtual ~CListBox();


    // Items
    int AddString( LPCSTR Text );
    int AddString( const GR::String& Text );
    int AddString( LPCSTR Text, DWORD_PTR ItemData );
    int AddString( const GR::String& Text, DWORD_PTR ItemData );
    int InsertItem( int InsertAt, const GR::String& Text, DWORD_PTR ItemData );
    int SetItemData( int nIndex, DWORD_PTR ItemData );
    DWORD_PTR GetItemData( int nIndex ) const;
    DWORD_PTR GetCurSelItemData() const;
    void SetItemText( int ItemIndex, const GR::String& Text );

    int GetText( int nIndex, LPSTR lpszBuffer, int nMaxLength ) const;
    GR::String GetText( int nIndex ) const;

    int DeleteString( UINT nIndex );

    int GetCount() const;
    void ResetContent();

    // Selektion
    int SelectItem( int nSelect );
    int GetCurSel() const;

  };

}

#endif // INCLUDE_CLISTBOX_H



