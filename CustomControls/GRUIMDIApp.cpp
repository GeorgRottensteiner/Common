// CustomControls.cpp : Definiert den Einstiegspunkt für die Anwendung.
//

#include "GRUIMDIApp.h"
#include "GRUIDockingManager.h"
#include "GRUIDockFrame.h"
#include "GRUIMDIDocument.h"
#include "GRUIMDIView.h"
#include "GRUICmdManager.h"


GRUIMDIApp*       GRUIMDIApp::m_pMDIApp = NULL;
std::list<HWND>   GRUIMDIApp::m_listSyncWindows;



GRUIMDIApp::GRUIMDIApp() :
  m_pActiveMDIChild( NULL )
{

  m_pMDIApp = this;
  m_strCaption = "GRUIMDIApp";

}



GRUIMDIApp::~GRUIMDIApp()
{
  m_pMDIApp = NULL;
}



void GRUIMDIApp::Draw( HDC hdc, const RECT& rc )
{
}



void GRUIMDIApp::DrawBackground( HDC hdc, const RECT& rc )
{
}



LRESULT GRUIMDIApp::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{

  switch ( uMsg )
  {
    case WM_NCCREATE:
      break;
    case WM_CREATE:
      RecalcLayout();
      break;
    case WM_SIZE:
      RecalcLayout();
      break;
    case WM_CLOSE:
      DestroyWindow();
      break;
    case WM_DESTROY:
      ExitInstance();
      PostQuitMessage( 0 );
      break;
    case WM_ACTIVATE:
      m_bActive = IsCaptionActive();
      SyncActivate( m_bActive );
      SendMessage( WM_NCPAINT, 1, 0 );
      return 0;
    case WM_ENABLE:
      CCustomWnd::Default();
      SyncEnable( (BOOL)wParam );
      return 0;
    case WM_NCPAINT:
      {
        RECT    rc;
        GetWindowRect( &rc );

        OffsetRect( &rc, -rc.left, -rc.top );

        HDC   hdc = BeginNCPaint();

        m_pDisplayClass->PaintWindowBorder( hdc, rc, m_dwWantedStyles, m_dwWantedExtendedStyle );
        DrawCaption( hdc, IsCaptionActive() );

        EndNCPaint();
      }
      return 0;
    case WM_NCACTIVATE:
      SendMessage( WM_NCPAINT, 1, 0 );
      return TRUE;
    case WM_ACTIVATEAPP:
      SyncActivate( !!wParam );
      break;
  }
  return CCustomWnd::WindowProc( uMsg, wParam, lParam );

}



int GRUIMDIApp::Run()
{

  RECT      rc;

  SetRect( &rc, 50, 50, 640, 480 );
  if ( !Create( m_strCaption.c_str(), WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, rc, NULL, 0 ) )
  {
    MessageBox( "Failed to create Main Window!", m_strCaption.c_str() );
    return -1;
  }

  GetClientRect( &rc );

  //dh::Log( "MDIApp ClientRect %d,%d", rc.right, rc.bottom );

  if ( !m_wndMDIClient.CreateEx( WS_EX_CLIENTEDGE, "GRMDIClient", WS_VISIBLE | WS_CHILD, rc, m_hWnd, 0 ) )
  {
    DestroyWindow();
    return -1;
  }

  GRUIDockingManager::m_pWndTopLevel = this;
  GRUIDockingManager::EnableDocking( GRUIDockFrame::DFP_ANY );

  if ( !InitInstance() )
  {
    return -1;
  }

  if ( m_hWnd )
  {
    while ( PumpMessage() );
  }

  return 0;

}


bool GRUIMDIApp::InitInstance()
{

  m_wndStatusBar.Create( "StatusBar", m_hWnd );

  if ( m_pDisplayClass->m_pCmdManager )
  {
    m_pDisplayClass->m_pCmdManager->m_pStatusBar = &m_wndStatusBar;
  }

  RecalcLayout();
  return true;

}



void GRUIMDIApp::ExitInstance()
{

  tListDocuments::iterator    itDoc( m_listDocuments.begin() );
  while ( itDoc != m_listDocuments.end() )
  {
    GRUIMDIDocument*    pDoc = *itDoc;

    if ( pDoc )
    {
      pDoc->OnExitInstance();
      delete pDoc;
    }

    ++itDoc;
  }

  m_listDocuments.clear();

  GRUIDockingManager::EnableDocking( 0 );

}



void GRUIMDIApp::RecalcLayout()
{

  RECT      rcMDI;

  GetClientRect( &rcMDI );

  // Childs prüfen
  HWND    hwndChild = GetWindow( GW_CHILD );
  while ( hwndChild )
  {
    CCustomWnd*  pWndChild = (CCustomWnd*)FromHandle( hwndChild );
    if ( pWndChild )
    {
      pWndChild->OnParentRecalcClientSize( rcMDI );
    }
    hwndChild = ::GetNextWindow( hwndChild, GW_HWNDNEXT );
  }

  for ( int i = 0; i < 4; ++i )
  {
    if ( GRUIDockingManager::m_pDockBars[i] )
    {
      GRUIDockingManager::m_pDockBars[i]->OnSizeParent( rcMDI );
    }
  }

  RECT    rcOld;

  m_wndMDIClient.GetWindowRect( &rcOld );

  ScreenToClient( &rcOld );
  

  if ( ( rcOld.left != rcMDI.left )
  ||   ( rcOld.top != rcMDI.top )
  ||   ( rcOld.right != rcMDI.right )
  ||   ( rcOld.bottom != rcMDI.bottom ) )
  {
    m_wndMDIClient.SetWindowPos( 0, rcMDI.left, rcMDI.top, rcMDI.right - rcMDI.left, rcMDI.bottom - rcMDI.top,
                                 SWP_NOZORDER );
  }

}



void GRUIMDIApp::GetPopupList( HWND hwndMain, std::list<HWND>& listWindows, BOOL fIncMain )
{

  listWindows = m_listSyncWindows;

  if ( fIncMain )
  {
    listWindows.push_back( hwndMain );
  }

}



void GRUIMDIApp::SyncActivate( BOOL bActivate )
{

  std::list<HWND>   listHwnd;

  GetPopupList( m_hWnd, listHwnd, TRUE);

  // UNDOCUMENTED FEATURE: 
  // If the other window being activated/deactivated 
  // (i.e. NOT this one) is one of our popups, then go (or stay) active.
  std::list<HWND>::iterator   it( listHwnd.begin() );
  while ( it != listHwnd.end() )
  {
    ::SendMessage( *it, WM_NCACTIVATE, bActivate, 0 );

    ++it;
  }

}



bool GRUIMDIApp::IsCaptionActive()
{

  if ( !IsWindowEnabled() )
  {
    return false;
  }

  std::list<HWND>   listHwnd;

  GRUIMDIApp::GetPopupList( m_hWnd, listHwnd, TRUE);

  std::list<HWND>::iterator   it( listHwnd.begin() );
  while ( it != listHwnd.end() )
  {
    if ( GetActiveWindow() == *it )
    {
      // das andere Window ist ja auch eins von unseren
      return true;
    }

    ++it;
  }

  return false;

}



bool GRUIMDIApp::NewDocument( GRUIMDIDocument* pDoc )
{

  if ( pDoc == NULL )
  {
    return false;
  }

  tListDocuments::iterator    it( m_listDocuments.begin() );
  while ( it != m_listDocuments.end() )
  {
    if ( *it == pDoc )
    {
      // Doc is already in list
      return false;
    }

    ++it;
  }

  m_listDocuments.push_back( pDoc );

  pDoc->OnInitInstance();

  return true;

}



void GRUIMDIApp::SetActiveView( GRUIMDIView* pView )
{

  if ( m_pActiveMDIChild == pView )
  {
    return;
  }

  if ( m_pActiveMDIChild )
  {
    // das alte
    m_pActiveMDIChild->OnMDIDeactivate();
  }

  m_pActiveMDIChild = pView;
  if ( m_pActiveMDIChild )
  {
    m_pActiveMDIChild->SetForegroundWindow();
    m_pActiveMDIChild->Invalidate();

    m_pActiveMDIChild->OnMDIActivate();
  }

}



void GRUIMDIApp::SyncEnable( BOOL bEnable )
{

  std::list<HWND>   listHwnd;

  GetPopupList( m_hWnd, listHwnd, TRUE);

  std::list<HWND>::iterator   it( listHwnd.begin() );
  while ( it != listHwnd.end() )
  {
    if ( *it != m_hWnd )
    {
      ::EnableWindow( *it, bEnable );
    }

    ++it;
  }

  SyncActivate( bEnable );

}



