#ifndef INCLUDE_CDIALOG_H
#define INCLUDE_CDIALOG_H



#include "CWnd.h"



namespace WindowsWrapper
{

  class CDialog : public CWnd
  {

    public:

    CDialog( DWORD dwResourceID = 0, HWND hwndParent = NULL );

    BOOL            Create( HWND hwndParent = NULL );
    BOOL            Create( LPCSTR lpszTemplateName, HWND hwndParent = NULL );
    BOOL            Create( UINT dwResource, HWND hwndParent = NULL );
    BOOL            Create( HINSTANCE hInstance, UINT dwResource, HWND hwndParent = NULL );
    BOOL            Create( HINSTANCE hInstance, LPCSTR lpszTemplateName, HWND hwndParent = NULL );
    BOOL            CreateIndirect( HGLOBAL hDialogTemplate, HWND hwndParent, HINSTANCE hInst );
    BOOL            CreateIndirect( LPCDLGTEMPLATE lpDialogTemplate, HWND hwndParent, void* lpDialogInit, HINSTANCE hInst );

    virtual BOOL    PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );

    virtual BOOL    Attach( HWND hWndNew );

    virtual int     DoModal( HINSTANCE hInstance = NULL );

    virtual BOOL    OnInitDialog();

    virtual void    OnOK();
    virtual void    OnCancel();

    void            EndDialog( int nResult );

    protected:

    bool            m_bCreated;

    DWORD           m_dwResourceID;

    HWND            m_hwndParent;

    virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    friend LRESULT CALLBACK MYHELPERWINDOWPROC( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

  };

}

#endif // INCLUDE_CDIALOG_H