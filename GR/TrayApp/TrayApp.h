#ifndef _TRAYAPP_H_
#define _TRAYAPP_H_



#pragma warning( disable : 4786 )

#include <windows.h>

#include <ControlHelper/CWnd.h>

#include <string>
#include <map>



#define WM_NOTIFY_SIZER             WM_USER + 101



class CTrayApp
{
  protected:

    GR::u32                       m_dwWMTaskbarCreated;

    GR::u32                       m_dwResourceID;

    WindowsWrapper::CWnd          m_WndMain;

    HINSTANCE                     m_hInstance;

    GR::String                   m_strAppPath,
                                  m_strDescriptionText;

    NOTIFYICONDATA                m_nidIcon;

    virtual LRESULT CALLBACK      MainWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    GR::String                   AppPath( const char* szPath );

    void                          ChangeIcon( DWORD dwResourceID = 0, const char* szDescription = "" );


  public:


    virtual bool                  InitInstance();
    virtual void                  ExitInstance();

    virtual int                   Run( DWORD dwIconResource = 0, const char* szDescription = "" );
    void                          ShutDown();

    virtual void                  OnNotify( WPARAM wParam, LPARAM lParam );


    CTrayApp();
    virtual ~CTrayApp();


    friend BOOL CALLBACK          MainWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

};


#endif // _TRAYAPP_H_