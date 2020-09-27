// NonModalDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "fred.h"
#include "nonmodaldialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CNonModalDialog 


// CNonModalDialog::CNonModalDialog(CWnd* pParent /*=NULL*/)
// 	: CDialog(CNonModalDialog::IDD, pParent)
// {
// 	//{{AFX_DATA_INIT(CNonModalDialog)
// 		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
// 	//}}AFX_DATA_INIT
// }


CNonModalDialog::CNonModalDialog( FRED::CNamedObject& subject, const CString& title, const CString& message, CWnd* pParent /*=NULL*/)
	: CDialog(CNonModalDialog::IDD, pParent), 
    CObserver(),
    m_pSubject( &subject ),
    m_bChanging( false )
{
	//{{AFX_DATA_INIT(CNonModalDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
  if ( CDialog::Create(IDD))
  {
    AddSubject( subject );
    ShowWindow( SW_SHOW );
    OnUpdate();
  }
}


void CNonModalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNonModalDialog)
	DDX_Control(pDX, IDC_EDIT1, m_ctlEdit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNonModalDialog, CDialog)
	//{{AFX_MSG_MAP(CNonModalDialog)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CNonModalDialog 

void CNonModalDialog::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
  TRACE( "CNonModalDialog::PostNcDestroy, delete mich nun\n" );
  delete this;
}

void CNonModalDialog::OnCancel()
{
  DestroyWindow();
}


void CNonModalDialog::OnUpdate()	
{
  if ( !IsObserver() )
  {
    m_pSubject = NULL;
    PostMessage( WM_COMMAND, IDCANCEL );
    return;
  }
  
  if ( !m_pSubject ) return;
  
  if ( m_bChanging ) return;

  CString cstrMessage( m_pSubject->name().c_str() ); 
  SetWindowText( m_pSubject->name().c_str() );
  m_ctlEdit1.SetWindowText( cstrMessage );
}

void CNonModalDialog::OnChangeEdit1() 
{
  if ( !IsObserver() )
  {
    return;
  }

  if ( !m_pSubject ) return;
  
  m_bChanging = true;

  CString cstrName;
  m_ctlEdit1.GetWindowText( cstrName );
  SetWindowText( cstrName );
  m_pSubject->name( LPCTSTR( cstrName ) );

  m_bChanging = false;
}


void CNonModalDialog::Invalidate(CObserver *from )
// Invalidates the object and notifies all its observers.
{
  if ( !IsObserver() )
  {
    m_pSubject = NULL;
    PostMessage( WM_COMMAND, IDCANCEL );
  }

  CObserver::Invalidate( from );
}
