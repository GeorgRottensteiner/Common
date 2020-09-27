// FileDialogEx.cpp : implementation file
//

#include "stdafx.h"
#include "filedialogex.h"
#include <map>

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/


namespace CFILEDIALOGEX
{

static std::map<HWND,CFileDialogEx*>   mapHWNDToFOpen;


UINT_PTR CALLBACK OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{

  if ( uiMsg == WM_NOTIFY )
  {
	  OFNOTIFY* pNotify = (OFNOTIFY*)lParam;

    if ( pNotify->hdr.code == CDN_INITDONE )
	  {
      CFileDialogEx*   pFOpen = (CFileDialogEx*)pNotify->lpOFN->lCustData;
      mapHWNDToFOpen[hdlg] = pFOpen;
    }
  }

  std::map<HWND,CFileDialogEx*>::iterator   itMap( mapHWNDToFOpen.find( hdlg ) );
  if ( itMap != mapHWNDToFOpen.end() )
  {
    CFileDialogEx*   pFOpen = itMap->second;

    return pFOpen->HookProc( hdlg, uiMsg, wParam, lParam );
  }

  return FALSE;

}

};




/////////////////////////////////////////////////////////////////////////////
// CFileDialogEx

IMPLEMENT_DYNAMIC(CFileDialogEx, CFileDialog)

CFileDialogEx::CFileDialogEx(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
    m_hwndParent( NULL )
{

  if ( pParentWnd )
  {
    m_hwndParent = pParentWnd->GetSafeHwnd();
    m_ofnEx.hwndOwner = m_hwndParent;
  }

}


BEGIN_MESSAGE_MAP(CFileDialogEx, CFileDialog)
	//{{AFX_MSG_MAP(CFileDialogEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/*-HookProc-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

UINT_PTR CFileDialogEx::HookProc( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{

  if ( uiMsg == WM_NOTIFY )
  {
	  OFNOTIFY* pNotify = (OFNOTIFY*)lParam;

	  switch( pNotify->hdr.code )
	  {
	    case CDN_SELCHANGE:
		    OnFileNameChange();
		    return TRUE;
	    case CDN_FOLDERCHANGE:
		    OnFolderChange();
		    return TRUE;
	    case CDN_INITDONE:
        m_hWnd = hWnd;
		    OnInitDone();
		    return TRUE;
      case CDN_FILEOK:
        OnFileOK();
        return TRUE;
	  }
  }
  else if ( uiMsg == WM_DESTROY )
  {
    if ( CFILEDIALOGEX::mapHWNDToFOpen.find( hWnd ) != CFILEDIALOGEX::mapHWNDToFOpen.end() )
    {
      OnDestroy();
      CFILEDIALOGEX::mapHWNDToFOpen.erase( CFILEDIALOGEX::mapHWNDToFOpen.find( hWnd ) );
    }
  }

  return 0;

}



/*-OnFileOK-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFileDialogEx::OnFileOK()
{
}



/*-OnFileNameChange-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFileDialogEx::OnFileNameChange()
{
}



/*-OnFolderChange-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFileDialogEx::OnFolderChange()
{
}


#include <debug/debugclient.h>

/*-OnInitDone-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFileDialogEx::OnInitDone()
{

  GetParent()->CenterWindow( CWnd::GetDesktopWindow() );

}



/*-DoModal--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

int CFileDialogEx::DoModal()
{

  BOOL    bResult = 0;

  ZeroMemory( &m_ofnEx, sizeof( m_ofnEx ) );
  memcpy( &m_ofnEx, &m_ofn, sizeof( m_ofn ) );
  m_ofnEx.lStructSize = sizeof( m_ofn );
  m_ofnEx.hwndOwner = m_hwndParent;

  OSVERSIONINFO   osInfo;

  osInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
  GetVersionEx( &osInfo );
  if ( ( osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
  &&   ( osInfo.dwMajorVersion >= 5 ) )
  {
    // 2000, XP, Server .NET
#if WINVER <= 0x0400
//#if _MSC_VER < 1300
    // ab Visual Studio .Net ist es repariert
    m_ofnEx.lStructSize = sizeof( OPENFILENAMEEX );
#endif
  }
  m_ofnEx.lpfnHook = CFILEDIALOGEX::OpenFileHookProc;
  m_ofnEx.Flags |= OFN_ENABLEHOOK;
  m_ofnEx.lCustData = (LPARAM)this;

  if ( m_bOpenFileDialog )
  {
    bResult = GetOpenFileName( &m_ofnEx );
  }
  else
  {
    bResult = GetSaveFileName( &m_ofnEx );
  }

  memcpy( &m_ofn, &m_ofnEx, sizeof( m_ofn ) );

  m_hWnd = NULL;

  return bResult;

}



/*-OnDestroy------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFileDialogEx::OnDestroy()
{
}
