#ifndef INCLUDE_CFILEDIALOG_H
#define INCLUDE_CFILEDIALOG_H



#include <ControlHelper\CDialog.h>

#pragma warning( disable:4786 )
#include <string>
#include <list>



namespace WindowsWrapper
{

  namespace CFILEDIALOG
  {
    UINT_PTR CALLBACK OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
  };

  class CFileDialog : public CDialog
  {

    public:

    OPENFILENAME    m_ofn;


    CFileDialog( bool bOpenFileDialog,
                 const char* szDefaultExtension = NULL,
                 const char* szStartingFileName = NULL,
                 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
                 const char* szFilter = NULL,
                 HWND hwndParent = NULL );

    // Operations
    virtual int                   DoModal();

    virtual const GR::String&     GetFileName() const;

    virtual int                   GetFileList( std::list<GR::String>& listFiles ) const;

    virtual void                  OnFileOK();
    virtual void                  OnInitDone();
    virtual void                  OnFolderChange();
    virtual void                  OnFileNameChange();
    virtual void                  OnDestroy();


    protected:

    virtual UINT_PTR              HookProc( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam );

#if (_WIN32_WINNT >= 0x0500)
    struct OPENFILENAMEEX : public OPENFILENAME
    {
    };
#else
    struct OPENFILENAMEEX : public OPENFILENAME
    {
      void *        pvReserved;
      DWORD         dwReserved;
      DWORD         FlagsEx;
    };
#endif

    OPENFILENAMEEX   m_ofnEx;

    bool          m_bOpenFileDialog;

    GR::WChar     szDummyBuffer[70000];

    GR::WString   m_Filter;

    GR::String   m_strFileName;

    friend UINT_PTR CALLBACK CFILEDIALOG::OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );

  };

}

#endif // INCLUDE_CDIALOG_H