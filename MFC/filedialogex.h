#if !defined(AFX_FILEDIALOGEX_H__21A25BCB_1E39_42FC_92C1_337EA820DBEA__INCLUDED_)
#define AFX_FILEDIALOGEX_H__21A25BCB_1E39_42FC_92C1_337EA820DBEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// filedialogex.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileDialogEx dialog
namespace CFILEDIALOGEX
{
  UINT_PTR CALLBACK OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};

class CFileDialogEx : public CFileDialog
{

	DECLARE_DYNAMIC( CFileDialogEx )

  protected:


    struct OPENFILENAMEEX : public OPENFILENAME 
    { 
      void *        pvReserved;
      DWORD         dwReserved;
      DWORD         FlagsEx;
    };

    OPENFILENAMEEX   m_ofnEx;

    HWND                  m_hwndParent;


  public:


	  CFileDialogEx( BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);


    virtual void          OnFileOK();
    virtual void          OnInitDone();
    virtual void          OnFolderChange();
    virtual void          OnFileNameChange();
    virtual void          OnDestroy();

    virtual int           DoModal();


  protected:
	  //{{AFX_MSG(CFileDialogEx)
		  // NOTE - the ClassWizard will add and remove member functions here.
	  //}}AFX_MSG
	  DECLARE_MESSAGE_MAP()

    virtual UINT_PTR      HookProc( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam );


    friend UINT_PTR CALLBACK CFILEDIALOGEX::OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDIALOGEX_H__21A25BCB_1E39_42FC_92C1_337EA820DBEA__INCLUDED_)
