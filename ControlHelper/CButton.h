#ifndef INCLUDE_CBUTTON_H
#define INCLUDE_CBUTTON_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CButton : public CWnd
  {

    // Constructors
    private:

    DWORD     m_dwItemData;


    public:


    CButton();
    virtual ~CButton();


    BOOL      Create( LPCSTR lpszCaption, DWORD dwStyle, const RECT& rect, HWND hwndParent, UINT nID );

    // Check-Buttons
    virtual void      SetCheck( int nCheck );
    virtual int       GetCheck() const;

    HBITMAP           SetBitmap( HBITMAP hbmImage );
    HICON             SetIcon( HICON hIcon );

    void              SetItemData( DWORD dwItemData );
    DWORD             GetItemData() const;

    UINT              GetButtonStyle() const;

  };

}

#endif // INCLUDE_CBUTTON_H



