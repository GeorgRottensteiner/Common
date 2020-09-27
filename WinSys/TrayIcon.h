#ifndef TRAYICON_H_
#define TRAYICON_H_



#include <windows.h>

#include <string>

#include <GR/GRTypes.h>



class TrayIcon
{
  protected:

    GR::u32                       m_dwWMTaskbarCreated;

    GR::u32                       m_dwResourceID;

    GR::u32                       m_dwNotifyMessage;

    HWND                          m_hwndNotify;

    HWND                          m_hwndMySecretWindow;

    HINSTANCE                     m_hInstance;

    GR::String                   m_strDescriptionText;

    NOTIFYICONDATA                m_nidIcon;

    LRESULT CALLBACK              WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK       TrayIconProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


  public:


    TrayIcon( DWORD dwIconResourceID = 0, const GR::String strDescription = "", HWND hwndToNotify = NULL, DWORD dwNotifyMessage = WM_USER );
    virtual ~TrayIcon();


    void                          ChangeIcon( DWORD dwResourceID = 0, const char* szDescription = "" );
    void                          ChangeNotifier( HWND hwndToNotify, GR::u32 dwNotifyMessage );

};


#endif // TRAYICON_H_