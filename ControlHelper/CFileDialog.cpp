#include <windows.h>

#include <Debug\debugclient.h>

#include <ControlHelper\CFileDialog.h>

#pragma warning ( disable : 4786 )
#include <map>



namespace WindowsWrapper
{

  namespace CFILEDIALOG
  {

    static std::map<HWND, CFileDialog*>   mapHWNDToFOpen;


    UINT_PTR CALLBACK OpenFileHookProc( HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
    {

      if ( uiMsg == WM_NOTIFY )
      {
        OFNOTIFY* pNotify = (OFNOTIFY*)lParam;

        if ( pNotify->hdr.code == CDN_INITDONE )
        {
          CFileDialog*   pFOpen = (CFileDialog*)pNotify->lpOFN->lCustData;
          mapHWNDToFOpen[hdlg] = pFOpen;
        }
      }

      std::map<HWND, CFileDialog*>::iterator   itMap( mapHWNDToFOpen.find( hdlg ) );
      if ( itMap != mapHWNDToFOpen.end() )
      {
        CFileDialog*   pFOpen = itMap->second;

        return pFOpen->HookProc( hdlg, uiMsg, wParam, lParam );
      }

      return FALSE;

    }

  };




  CFileDialog::CFileDialog( bool bOpenFile,
                            const char* strDefaultExtension,
                            const char* strStartingFileName,
                            DWORD dwFlags,
                            const char* strFilter,
                            HWND hwndParent ) :
    CDialog(),
    m_bOpenFileDialog( bOpenFile )
  {

    szDummyBuffer[0] = 0;
    if ( strStartingFileName )
    {
      wsprintfW( szDummyBuffer, GR::Convert::ToUTF16( strStartingFileName ).c_str() );
    }

    m_Filter = GR::Convert::ToUTF16( strFilter );

    ZeroMemory( &m_ofn, sizeof( m_ofn ) );
    m_ofn.lStructSize = sizeof( OPENFILENAME );
    m_ofn.hwndOwner = hwndParent;
    m_ofn.hInstance = GetModuleHandle( NULL );
    if ( !m_Filter.empty() )
    {
      m_ofn.lpstrFilter = m_Filter.c_str();
    }
    m_ofn.lpstrCustomFilter = NULL;
    m_ofn.nMaxCustFilter = 0;
    m_ofn.nFilterIndex = 1;
    m_ofn.lpstrFile = szDummyBuffer;
    m_ofn.nMaxFile = sizeof( szDummyBuffer );
    m_ofn.lpstrInitialDir = NULL;
    m_ofn.lpstrTitle = NULL;
    m_ofn.Flags = dwFlags;
  }



  UINT_PTR CFileDialog::HookProc( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam )
  {
    if ( uiMsg == WM_NOTIFY )
    {
      OFNOTIFY* pNotify = (OFNOTIFY*)lParam;

      switch ( pNotify->hdr.code )
      {
        case CDN_SELCHANGE:
          OnFileNameChange();
          return TRUE;
        case CDN_FOLDERCHANGE:
          OnFolderChange();
          return TRUE;
        case CDN_INITDONE:
          Attach( ::GetParent( hWnd ) );
          OnInitDone();
          return TRUE;
        case CDN_FILEOK:
          OnFileOK();
          return TRUE;
      }
    }
    else if ( uiMsg == WM_DESTROY )
    {
      if ( CFILEDIALOG::mapHWNDToFOpen.find( hWnd ) != CFILEDIALOG::mapHWNDToFOpen.end() )
      {
        Detach();
        OnDestroy();
        CFILEDIALOG::mapHWNDToFOpen.erase( CFILEDIALOG::mapHWNDToFOpen.find( hWnd ) );
      }
    }

    return 0;

  }



  int CFileDialog::DoModal()
  {
    int nResult;

    m_strFileName = GR::Convert::ToUTF8( m_ofn.lpstrFile );

    ZeroMemory( &m_ofnEx, sizeof( m_ofnEx ) );
    memcpy( &m_ofnEx, &m_ofn, sizeof( m_ofn ) );
    m_ofnEx.lStructSize = sizeof( m_ofn );

    OSVERSIONINFO   osInfo;

    osInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    GetVersionExW( &osInfo );
    if ( ( osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
         && ( osInfo.dwMajorVersion >= 5 ) )
    {
      // 2000, XP, Server .NET
      m_ofnEx.lStructSize = sizeof( OPENFILENAMEEX );
    }

    m_ofnEx.lpfnHook = CFILEDIALOG::OpenFileHookProc;
    m_ofnEx.Flags |= OFN_ENABLEHOOK;
    m_ofnEx.lCustData = ( LPARAM )this;

    if ( m_bOpenFileDialog )
    {
      nResult = ::GetOpenFileName( &m_ofnEx );
    }
    else
    {
      nResult = ::GetSaveFileName( &m_ofnEx );
    }
    if ( nResult == 0 )
    {
      DWORD error = CommDlgExtendedError();
      dh::Log( "CFileDialog Error %x", error );
    }

    memcpy( &m_ofn, &m_ofnEx, sizeof( m_ofn ) );

    if ( nResult )
    {
      m_strFileName = GR::Convert::ToUTF8( szDummyBuffer );
    }

    return nResult;
  }



  const GR::String& CFileDialog::GetFileName() const
  {
    return m_strFileName;
  }



  int CFileDialog::GetFileList( std::list<GR::String>& listFiles ) const
  {

    listFiles.clear();

    if ( !( m_ofn.Flags & OFN_ALLOWMULTISELECT ) )
    {
      // nur ein File drin
      listFiles.push_back( m_strFileName );
      return 1;
    }

    // mehrere Files!
    char    cTrenner = 0;
    if ( !( m_ofn.Flags & OFN_EXPLORER ) )
    {
      cTrenner = 32;
    }

    int           iPos = 0,
      iOldStyleLength = (int)wcslen( szDummyBuffer );

    GR::String  pathToCheck = GR::Convert::ToUTF8( szDummyBuffer );

    GR::String   strFileName = "",
      strPath = "";

    bool          bPath = true;

    while ( true )
    {
      if ( pathToCheck[iPos] != cTrenner )
      {
        strFileName += pathToCheck[iPos];
      }
      if ( pathToCheck[iPos] == cTrenner )
      {
        if ( pathToCheck[iPos + 1] == cTrenner )
        {
          // der letzte Eintrag
          if ( bPath )
          {
            listFiles.push_back( strFileName );
            break;
          }
          listFiles.push_back( strPath + strFileName );
          break;
        }
        else
        {
          if ( bPath )
          {
            strPath = strFileName;
            if ( ( !( strPath.empty() ) )
                 && ( strPath[strPath.length() - 1] != 92 ) )
            {
              strPath += '\\';
            }

            bPath = false;
          }
          else
          {
            listFiles.push_back( strPath + strFileName );
          }
        }
        strFileName = "";
      }
      ++iPos;
      if ( iPos >= MAX_PATH * 20 )
      {
        break;
      }
      if ( !( m_ofn.Flags & OFN_EXPLORER ) )
      {
        if ( iPos >= iOldStyleLength )
        {
          if ( bPath )
          {
            listFiles.push_back( strFileName );
            break;
          }
          listFiles.push_back( strPath + strFileName );
          break;
        }
      }
    }
    return (int)listFiles.size();
  }



  void CFileDialog::OnFileOK()
  {
  }



  void CFileDialog::OnFileNameChange()
  {
  }



  void CFileDialog::OnFolderChange()
  {
  }



  void CFileDialog::OnInitDone()
  {
    CenterWindow();
  }



  void CFileDialog::OnDestroy()
  {
  }

}