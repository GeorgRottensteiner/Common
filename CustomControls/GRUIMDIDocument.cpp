#include ".\gruimdidocument.h"
#include ".\gruimdiview.h"
#include ".\gruimdiapp.h"

#include <Misc/Misc.h>



GRUIMDIDocument::GRUIMDIDocument( HWND hwndParent ) :
  m_hwndMDIParent( hwndParent ),
  m_bChanged( false )
{
}

GRUIMDIDocument::~GRUIMDIDocument(void)
{
}



void GRUIMDIDocument::OnInitInstance()
{

  AddView( new GRUIMDIView( this ) );
  
}



void GRUIMDIDocument::OnExitInstance()
{

  tListViews::iterator    it( m_listViews.begin() );
  while ( it != m_listViews.end() )
  {
    delete *it;

    ++it;
  }
  m_listViews.clear();

}




void GRUIMDIDocument::AddView( GRUIMDIView* pView )
{

  if ( pView == NULL )
  {
    return;
  }

  RECT    rc;

  SetRect( &rc, 0, 0, 320, 240 );

  pView->Create( "View", WS_SYSMENU | WS_VISIBLE | WS_CHILD | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, rc, m_hwndMDIParent, 0 );
  pView->SetOwner( GRUIMDIApp::m_pMDIApp->GetSafeHwnd() );

  static char   cName = 'A';

  pView->SetWindowText( CMisc::printf( "View %c", cName ) );
  ++cName;


  tListViews::iterator    it( m_listViews.begin() );
  while ( it != m_listViews.end() )
  {
    if ( *it == pView )
    {
      // View is already in list
      return;
    }

    ++it;
  }

  m_listViews.push_back( pView );

  GRUIMDIApp::m_pMDIApp->SetActiveView( pView );

}



size_t GRUIMDIDocument::ViewCount() const
{

  return m_listViews.size();

}



bool GRUIMDIDocument::CanClose()
{

  if ( GRUIMDIApp::m_pMDIApp->MessageBox( "Zumachen?", "wss?", MB_YESNO ) == IDYES )
  {
    return true;
  }
  return false;
  //return !m_bChanged;

}