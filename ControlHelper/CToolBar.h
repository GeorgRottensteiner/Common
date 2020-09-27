#ifndef INCLUDE_CTOOLBAR_H
#define INCLUDE_CTOOLBAR_H



#include <ControlHelper\CWnd.h>


#include <commctrl.h>

#pragma comment( lib, "comctl32.lib" )



namespace WindowsWrapper
{

  class CCommonCtrlDummyToolBar
  {
    public:

    CCommonCtrlDummyToolBar()
    {
      InitCommonControls();
    }
  };

  static CCommonCtrlDummyToolBar    commoncontroldummytoolbar;

  class CToolBar : public CWnd
  {

    // Constructors
    public:

    HBITMAP               m_hbmImageWell;

    SIZE                  m_sizeImage,
      m_sizeButton;


    CToolBar();
    virtual ~CToolBar();


    virtual BOOL          Create( HWND hwndParent,
                                  DWORD dwStyle = WS_CHILD | WS_VISIBLE,
                                  DWORD_PTR dwID = 0 );
    BOOL                  CreateEx( HWND hwndParent, DWORD dwCtrlStyle = TBSTYLE_FLAT, DWORD dwStyle = WS_CHILD | WS_VISIBLE, DWORD_PTR dwID = 0 );

    virtual BOOL          LoadToolBar( LPCTSTR lpszResourceName );
    virtual BOOL          LoadToolBar( DWORD dwResourceId );
    virtual BOOL          LoadBitmap( LPCTSTR lpszResourceName );

    virtual void          AdjustParentClientRect( HWND hwndParent );

    virtual BOOL          SetButtons( const UINT* lpIDArray, int nIDCount );
    virtual BOOL          AddReplaceBitmap( HBITMAP hbmImageWell );

    virtual void          EnableButton( DWORD dwButtonId, BOOL bEnable = TRUE );

    protected:



  };

}



#endif // INCLUDE_CTOOLBAR_H



