#include <windows.h>
#include <commctrl.h>

#include "CDialog.h"

#include <map>



namespace WindowsWrapper
{

  CDialog::CDialog( DWORD dwResourceId, HWND hwndParent ) :
    CWnd(),
    m_bCreated( false ),
    m_dwResourceID( dwResourceId ),
    m_hwndParent( hwndParent )
  {


  }



  BOOL CDialog::Create( UINT dwResource, HWND hwndParent )
  {

    return Create( MAKEINTRESOURCEA( dwResource ), hwndParent );

  }


  BOOL CDialog::Create( HWND hwndParent )
  {
    return Create( MAKEINTRESOURCEA( m_dwResourceID ), hwndParent );
  }


  BOOL CDialog::Create( LPCSTR lpszTemplateName, HWND hwndParent )
  {
    HINSTANCE   hInst = CWnd::FindResourceHandle( lpszTemplateName, RT_DIALOG );
    if ( hInst == NULL )
    {
      return FALSE;
    }
    HRSRC hResource = NULL;

    if ( IS_INTRESOURCE( lpszTemplateName ) )
    {
      hResource = ::FindResourceW( hInst, (LPCWSTR)lpszTemplateName, MAKEINTRESOURCEW(5) ); // IS RT_DIALOG
    }
    else
    {
      hResource = ::FindResourceW( hInst, GR::Convert::ToUTF16( lpszTemplateName ).c_str(), MAKEINTRESOURCEW( 5 ) ); // IS RT_DIALOG
    }
    if ( hResource == NULL )
    {
      return FALSE;
    }
    HGLOBAL hTemplate = LoadResource( hInst, hResource );
    if ( hTemplate == NULL )
    {
      return FALSE;
    }

    if ( hwndParent )
    {
      m_hwndOwner = hwndParent;
    }

    BOOL bResult = CreateIndirect( hTemplate, hwndParent, hInst );

    FreeResource( hTemplate );

    if ( bResult )
    {
      m_bCreated = true;
    }

    return bResult;

  }



  BOOL CDialog::Create( HINSTANCE hInstance, LPCSTR lpszTemplateName, HWND hwndParent )
  {
    HRSRC hResource = NULL;

    if ( IS_INTRESOURCE( lpszTemplateName ) )
    {
      hResource = ::FindResourceW( hInstance, (LPCWSTR)lpszTemplateName, MAKEINTRESOURCEW( 5 ) ); // IS RT_DIALOG
    }
    else
    {
      hResource = ::FindResourceW( hInstance, GR::Convert::ToUTF16( lpszTemplateName ).c_str(), MAKEINTRESOURCEW( 5 ) ); // IS RT_DIALOG
    }
    if ( hResource == NULL )
    {
      return FALSE;
    }
    HGLOBAL hTemplate = LoadResource( hInstance, hResource );
    if ( hTemplate == NULL )
    {
      return FALSE;
    }

    if ( hwndParent )
    {
      m_hwndOwner = hwndParent;
    }

    BOOL bResult = CreateIndirect( hTemplate, hwndParent, hInstance );

    FreeResource( hTemplate );

    if ( bResult )
    {
      m_bCreated = true;
    }

    return bResult;
  }



  BOOL CDialog::Create( HINSTANCE hInstance, UINT dwResource, HWND hwndParent )
  {
    return Create( hInstance, MAKEINTRESOURCEA( dwResource ), hwndParent );
  }



  BOOL CDialog::CreateIndirect( HGLOBAL hDialogTemplate, HWND hwndParent, HINSTANCE hInst )
  {
    LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource( hDialogTemplate );

    BOOL bResult = CreateIndirect( lpDialogTemplate, hwndParent, NULL, hInst );
    UnlockResource( hDialogTemplate );

    return bResult;
  }



  BOOL CDialog::CreateIndirect( LPCDLGTEMPLATE lpDialogTemplate, HWND hwndParent, void* lpDialogInit, HINSTANCE hInst )
  {

    return CreateDlgIndirect( lpDialogTemplate, hwndParent, hInst );

  }



  BOOL CDialog::PreTranslateMessage( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
  {

    if ( CWnd::PreTranslateMessage( hwnd, uMsg, wParam, lParam, lResult ) )
    {
      return TRUE;
    }

    MSG   msg;

    msg.hwnd = hwnd;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.message = uMsg;

    if ( ( msg.message < WM_KEYFIRST || msg.message > WM_KEYLAST ) &&
      ( msg.message < WM_MOUSEFIRST || msg.message > 0x0209 ) )//AFX_WM_MOUSELAST))
    {
      return FALSE;
    }
    if ( ::IsDialogMessage( m_hWnd, &msg ) )
    {
      return TRUE;
    }
    return FALSE;

  }



  BOOL CDialog::Attach( HWND hWndNew )
  {

    BOOL  bResult = CWnd::Attach( hWndNew );

    m_hwndOwner = ::GetParent( m_hWnd );

    return bResult;

  }



  int CDialog::DoModal( HINSTANCE hInstance )
  {

    if ( !IsWindow() )
    {
      if ( m_dwResourceID == 0 )
      {
        return 0;
      }
      if ( hInstance == NULL )
      {
        hInstance = CWnd::FindResourceHandle( MAKEINTRESOURCEA( m_dwResourceID ), RT_DIALOG );
      }
      if ( !Create( hInstance, MAKEINTRESOURCEA( m_dwResourceID ), m_hwndParent ) )
      {
        return 0;
      }
    }

    if ( m_bModal )
    {
      return 0;
    }

    if ( m_hwndOwner )
    {
      ::EnableWindow( m_hwndOwner, FALSE );
    }

    ShowWindow( SW_SHOW );
    //SetFocus();

    m_bModal = true;

    // Modal-Loop
    //MSG     dlgMsg;

    do
    {
      if ( !CWnd::PumpMessage() )
      {
        break;
      }
      /*
      if ( !::GetMessage( &dlgMsg, NULL, NULL, NULL ) )
      {
        // WM_QUIT
        break;
      }
      //if ( !::IsDialogMessage( m_hWnd, &dlgMsg ) )
      {
        if ( dlgMsg.message == WM_DESTROY )
        {
          ::TranslateMessage( &dlgMsg );
          ::DispatchMessage( &dlgMsg );
          m_bModal = false;
          break;
        }
        ::TranslateMessage( &dlgMsg );
        ::DispatchMessage( &dlgMsg );
      }
      */
    }
    while ( ( m_bModal )
            && ( m_hWnd ) );

    if ( m_hwndOwner )
    {
      ::EnableWindow( m_hwndOwner, TRUE );
    }

    DestroyWindow();

    return m_iModalResult;

  }



  void CDialog::EndDialog( int nResult )
  {

    if ( m_bModal )
    {
      EndModalLoop( nResult );
    }

    ::EndDialog( m_hWnd, nResult );

  }



  BOOL CDialog::OnInitDialog()
  {

    return TRUE;

  }



  void CDialog::OnOK()
  {

    if ( m_bModal )
    {
      EndDialog( IDOK );
    }

  }



  void CDialog::OnCancel()
  {

    if ( m_bModal )
    {
      EndDialog( IDCANCEL );
    }

  }



  LRESULT CDialog::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
  {

    switch ( uMsg )
    {
      case WM_COMMAND:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
          if ( LOWORD( wParam ) == IDOK )
          {
            OnOK();
          }
          else if ( LOWORD( wParam ) == IDCANCEL )
          {
            OnCancel();
          }
        }
        break;
    }
    return CWnd::WindowProc( uMsg, wParam, lParam );

  }


}