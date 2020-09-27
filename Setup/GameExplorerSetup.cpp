#include <windows.h>
#include <wbemcli.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <tchar.h>

#include <String/Path.h>

#include ".\gameexplorersetup.h"

#include <debug/debugclient.h>

#include <IO/FileUtil.h>

//#pragma comment( lib, "shlwapi.lib" )



WINOLEAPI CoSetProxyBlanket(
    IN IUnknown                 *pProxy,
    IN DWORD                     dwAuthnSvc,
    IN DWORD                     dwAuthzSvc,
    IN OLECHAR                  *pServerPrincName,
    IN DWORD                     dwAuthnLevel,
    IN DWORD                     dwImpLevel,
    IN RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    IN DWORD                     dwCapabilities );



namespace GR
{
  namespace Setup
  {
    namespace GameExplorerSetup
    {
      enum GAME_INSTALL_SCOPE
      {
        GIS_NOT_INSTALLED	= 1,
	      GIS_CURRENT_USER	= 2,
	      GIS_ALL_USERS	= 3
      };

      class DECLSPEC_UUID( "9A5EA990-3034-4D6F-9128-01F3C61022BC" ) GameExplorer;

      EXTERN_C const IID IID_IGameExplorer;

      MIDL_INTERFACE("E7B2FB72-D728-49B3-A5F2-18EBF5F1349E")
      IGameExplorer : public IUnknown
      {
        public:
          virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddGame(
                  /* [in] */ BSTR bstrGDFBinaryPath,
                  /* [in] */ BSTR bstrGameInstallDirectory,
                  /* [in] */ GAME_INSTALL_SCOPE installScope,
                  /* [out][in] */ GUID *pguidInstanceID) = 0;

          virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveGame(
                  /* [in] */ GUID guidInstanceID) = 0;

          virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UpdateGame(
                  /* [in] */ GUID guidInstanceID) = 0;

          virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE VerifyAccess(
                  /* [in] */ BSTR bstrGDFBinaryPath,
                  /* [out] */ BOOL *pfHasAccess) = 0;

      };

      BOOL RegDelnodeRecurseW( HKEY hKeyRoot, LPWSTR lpSubKey )
      {
        LPWSTR lpEnd;
        LONG lResult;
        DWORD dwSize;
        WCHAR szName[MAX_PATH];
        HKEY hKey;

        FILETIME ftWrite;
        // First, see if we can delete the key without having
        // to recurse.
        lResult = RegDeleteKeyW( hKeyRoot, lpSubKey );
        if ( lResult == ERROR_SUCCESS )
        {
          return TRUE;
        }
        lResult = RegOpenKeyExW( hKeyRoot, lpSubKey, 0, KEY_READ, &hKey );
        if ( lResult != ERROR_SUCCESS )
        {
          if ( lResult == ERROR_FILE_NOT_FOUND )
          {
            // Key not found
            return TRUE;
          }
          else
          {
            printf("Error opening key.\n");
            return FALSE;
          }
        }
        // Check for an ending slash and add one if it is missing.
        lpEnd = lpSubKey + wcslen( lpSubKey );
        if ( *( lpEnd - 1 ) != TEXT( '\\' ) )
        {
          *lpEnd =  TEXT( '\\' );
          lpEnd++;
          *lpEnd =  TEXT( '\0' );
        }
        // Enumerate the keys
        dwSize = MAX_PATH;
        lResult = RegEnumKeyExW( hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite );
        if ( lResult == ERROR_SUCCESS )
        {
          do
          {
            wcscpy( lpEnd, szName );
            if ( !RegDelnodeRecurseW( hKeyRoot, lpSubKey ) )
            {
              break;
            }
            dwSize = MAX_PATH;
            lResult = RegEnumKeyExW( hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite );
          }
          while ( lResult == ERROR_SUCCESS );
        }
        lpEnd--;
        *lpEnd = TEXT( '\0' );
        RegCloseKey( hKey );
        // Try again to delete the key.
        lResult = RegDeleteKeyW( hKeyRoot, lpSubKey );
        if ( lResult == ERROR_SUCCESS )
        {
          return TRUE;
        }
        return FALSE;
      }

      //*************************************************************
      ////  RegDelnode()
      ////  Purpose:    Deletes a registry key and all it's subkeys / values.
      ////  Parameters: hKeyRoot    -   Root key
      //              lpSubKey    -   SubKey to delete
      ////  Return:     TRUE if successful.
      //              FALSE if an error occurs.
      ////*************************************************************

      BOOL RegDelnodeW( HKEY hKeyRoot, LPWSTR lpSubKey )
      {
        WCHAR szDelKey[MAX_PATH * 2];
        wcscpy( szDelKey, lpSubKey );
        return RegDelnodeRecurseW( hKeyRoot, szDelKey );
      }

      HRESULT ConvertStringToGUID( const WCHAR* strSrc, GUID* pGuidDest )
      {
        UINT aiTmp[10];

        if ( swscanf( strSrc, L"{%8X-%4X-%4X-%2X%2X-%2X%2X%2X%2X%2X%2X}",
                      &pGuidDest->Data1,
                      &aiTmp[0], &aiTmp[1],
                      &aiTmp[2], &aiTmp[3],
                      &aiTmp[4], &aiTmp[5],
                      &aiTmp[6], &aiTmp[7],
                      &aiTmp[8], &aiTmp[9] ) != 11 )
        {
          ZeroMemory( pGuidDest, sizeof( GUID ) );
          return E_FAIL;
        }
        else
        {
          pGuidDest->Data2       = (USHORT) aiTmp[0];
          pGuidDest->Data3       = (USHORT) aiTmp[1];
          pGuidDest->Data4[0]    = (BYTE) aiTmp[2];
          pGuidDest->Data4[1]    = (BYTE) aiTmp[3];
          pGuidDest->Data4[2]    = (BYTE) aiTmp[4];
          pGuidDest->Data4[3]    = (BYTE) aiTmp[5];
          pGuidDest->Data4[4]    = (BYTE) aiTmp[6];
          pGuidDest->Data4[5]    = (BYTE) aiTmp[7];
          pGuidDest->Data4[6]    = (BYTE) aiTmp[8];
          pGuidDest->Data4[7]    = (BYTE) aiTmp[9];
          return S_OK;
        }
      }

      //-----------------------------------------------------------------------------
      // Enums WinXP registry for GDF upgrade keys, and returns the GUID
      // based on the GDF binary path
      //-----------------------------------------------------------------------------
      bool RetrieveGUIDForApplicationOnWinXP( HKEY hKeyRoot, WCHAR* szPathToGDFdll, GUID* pGUID )
      {
        DWORD iKey = 0;
        WCHAR strRegKeyName[256];
        WCHAR strGDFBinPath[MAX_PATH];
        HKEY hKey = NULL;
        LONG lResult;
        DWORD dwDisposition, dwType, dwSize;
        bool bFound = false;

        for ( ;; )
        {
          lResult = RegEnumKeyW( hKeyRoot, iKey, strRegKeyName, 256 );
          if ( lResult != ERROR_SUCCESS )
          {
            break;
          }

          lResult = RegCreateKeyExW( hKeyRoot, strRegKeyName, 0, NULL, 0, KEY_READ, NULL, &hKey, &dwDisposition );
          if ( lResult == ERROR_SUCCESS )
          {
            dwSize = MAX_PATH * sizeof( WCHAR );
            lResult = RegQueryValueExW( hKey, L"GDFBinaryPath", 0, &dwType, (BYTE*)strGDFBinPath, &dwSize );
            if ( lResult == ERROR_SUCCESS )
            {
              if ( wcscmp( strGDFBinPath, szPathToGDFdll ) == 0 )
              {
                bFound = true;
                ConvertStringToGUID( strRegKeyName, pGUID );
              }
            }
            RegCloseKey( hKey );
          }

          if ( bFound )
          {
            break;
          }
          iKey++;
        }

        return bFound;
      }

      STDAPI RetrieveGUIDForApplicationW( WCHAR* szPathToGDFdll, GUID* pGUID )
      {
        HRESULT               hr;
        IWbemLocator*         pIWbemLocator = NULL;
        IWbemServices*        pIWbemServices = NULL;
        BSTR                  pNamespace = NULL;
        IEnumWbemClassObject* pEnum = NULL;
        bool                  bFound = false;

        hr = CoInitialize( 0 );
        bool bCleanupCOM = SUCCEEDED( hr );

        hr = CoCreateInstance( __uuidof( WbemLocator ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IWbemLocator ), (LPVOID*)&pIWbemLocator );
        if ( ( SUCCEEDED( hr ) )
        &&   ( pIWbemLocator ) )
        {
          // Using the locator, connect to WMI in the given namespace.
          pNamespace = SysAllocString( L"\\\\.\\root\\cimv2\\Applications\\Games" );

          hr = pIWbemLocator->ConnectServer( pNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices );
          if ( ( SUCCEEDED( hr ) )
          &&   ( pIWbemServices != NULL ) )
          {
            // Switch security level to IMPERSONATE.
            CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0 );

            BSTR bstrQueryType = SysAllocString( L"WQL" );

            // Double up the '\' marks for the WQL query
            WCHAR szDoubleSlash[2048];
            int iDest = 0, iSource = 0;
            for ( ;; )
            {
              if ( ( szPathToGDFdll[iSource] == 0 )
              ||   ( iDest > 2000 ) )
              {
                break;
              }
              szDoubleSlash[iDest] = szPathToGDFdll[iSource];
              if ( szPathToGDFdll[iSource] == L'\\' )
              {
                iDest++;
                szDoubleSlash[iDest] = L'\\';
              }
              iDest++;
              iSource++;
            }
            szDoubleSlash[iDest] = 0;

            WCHAR szQuery[1024];

            _snwprintf( szQuery, 1024, L"SELECT * FROM GAME WHERE GDFBinaryPath = \"%s\"", szDoubleSlash );
            BSTR bstrQuery = SysAllocString( szQuery );

            hr = pIWbemServices->ExecQuery( bstrQueryType, bstrQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum );
            if ( SUCCEEDED( hr ) )
            {
              IWbemClassObject* pGameClass = NULL;
              DWORD             uReturned = 0;
              BSTR              pPropName = NULL;

              // Get the first one in the list
              hr = pEnum->Next( 5000, 1, &pGameClass, &uReturned );
              if ( ( SUCCEEDED( hr ) )
              &&   ( uReturned != 0 )
              &&   ( pGameClass != NULL ) )
              {
                VARIANT var;

                // Get the InstanceID string
                pPropName = SysAllocString( L"InstanceID" );
                hr = pGameClass->Get( pPropName, 0L, &var, NULL, NULL );
                if ( ( SUCCEEDED( hr ) )
                &&   ( var.vt == VT_BSTR ) )
                {
                  bFound = true;
                  if ( pGUID )
                  {
                    ConvertStringToGUID( var.bstrVal, pGUID );
                  }
                }
                if ( pPropName )
                {
                  SysFreeString( pPropName );
                }
              }
              if ( pGameClass )
              {
                pGameClass->Release();
                pGameClass = NULL;
              }
            }
            if ( pEnum )
            {
              pEnum->Release();
              pEnum = NULL;
            }
          }
          if ( pNamespace )
          {
            SysFreeString( pNamespace );
          }
          if ( pIWbemServices )
          {
            pIWbemServices->Release();
            pIWbemServices = NULL;
          }
        }
        if ( pIWbemLocator )
        {
          pIWbemLocator->Release();
          pIWbemLocator = NULL;
        }

        if ( bCleanupCOM )
        {
          CoUninitialize();
        }

        if ( !bFound )
        {
          // Look in WinXP regkey paths
          HKEY hKeyRoot;
          LONG lResult;
          DWORD dwDisposition;
          lResult = RegCreateKeyExW( HKEY_CURRENT_USER, L"Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 0, NULL, 0, KEY_READ, NULL, &hKeyRoot, &dwDisposition );
          if ( ERROR_SUCCESS == lResult )
          {
            bFound = RetrieveGUIDForApplicationOnWinXP( hKeyRoot, szPathToGDFdll, pGUID );
            RegCloseKey( hKeyRoot );
          }

          if ( !bFound )
          {
            lResult = RegCreateKeyExW( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade", 0, NULL, 0, KEY_READ, NULL, &hKeyRoot, &dwDisposition );
            if ( ERROR_SUCCESS == lResult )
            {
              bFound = RetrieveGUIDForApplicationOnWinXP( hKeyRoot, szPathToGDFdll, pGUID );
              RegCloseKey( hKeyRoot );
            }
          }
        }
        return ( bFound ) ? S_OK : E_FAIL;
      }

      HRESULT CreateShortcut( WCHAR* strLaunchPath, WCHAR* strCommandLineArgs, WCHAR* strShortcutFilePath )
      {
        HRESULT hr = CoInitialize( 0 );

        bool bCleanupCOM = SUCCEEDED( hr );

        IShellLinkW* psl;

        hr = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl );
        if ( SUCCEEDED( hr ) )
        {
          // Setup shortcut
          psl->SetPath( strLaunchPath );
          if ( strCommandLineArgs )
          {
            psl->SetArguments( strCommandLineArgs );
          }

          // These shortcut settings aren't needed for tasks
          // if( strIconPath ) psl->SetIconLocation( strIconPath, nIcon );
          // if( wHotkey ) psl->SetHotkey( wHotkey );
          // if( nShowCmd ) psl->SetShowCmd( nShowCmd );
          // if( strDescription ) psl->SetDescription( strDescription );

          // Set working dir to path of launch exe
          WCHAR strFullPath[512];
          WCHAR* strExePart;
          GetFullPathNameW( strLaunchPath, 512, strFullPath, &strExePart );
          if ( strExePart )
          {
            *strExePart = 0;
          }
          psl->SetWorkingDirectory( strFullPath );

          // Save shortcut to file
          IPersistFile* ppf;
          hr = psl->QueryInterface( IID_IPersistFile, (LPVOID*)&ppf );
          if ( SUCCEEDED( hr ) )
          {
            hr = ppf->Save( strShortcutFilePath, TRUE );
            ppf->Release();
          }
          psl->Release();
        }

        if ( bCleanupCOM )
        {
          CoUninitialize();
        }
        return hr;
      }

      HRESULT AddToGameExplorerW( WCHAR* strGDFBinPath, WCHAR* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, GUID* pInstanceGUID )
      {
        HRESULT hr                      = E_FAIL;
        bool    bCleanupCOM             = false;
        BOOL    bHasAccess              = FALSE;
        BSTR    bstrGDFBinPath          = NULL;
        BSTR    bstrGameInstallPath     = NULL;
        IGameExplorer* pFwGameExplorer  = NULL;

        if ( ( strGDFBinPath == NULL )
        ||   ( strGameInstallPath == NULL )
        ||   ( pInstanceGUID == NULL ) )
        {
          dh::Log( "AddToGameExplorerW invalid arg" );
          return E_INVALIDARG;
        }

        bstrGDFBinPath      = SysAllocString( strGDFBinPath );
        bstrGameInstallPath = SysAllocString( strGameInstallPath );
        if ( ( bstrGDFBinPath == NULL )
        ||   ( bstrGameInstallPath == NULL ) )
        {
          hr = E_OUTOFMEMORY;
          dh::Log( "AddToGameExplorerW E_OUTOFMEMORY" );
          goto LCleanup;
        }

        hr = CoInitialize( 0 );
        bCleanupCOM = SUCCEEDED( hr );

        // Create an instance of the Game Explorer Interface
        hr = CoCreateInstance( __uuidof( GameExplorer ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IGameExplorer ), (void**)&pFwGameExplorer );
        if ( ( FAILED( hr ) )
        ||   ( pFwGameExplorer == NULL ) )
        {
          dh::Log( "AddToGameExplorerW no game explorer found" );
          // On Windows XP or eariler, write registry keys to known location
          // so that if the machine is upgraded to Windows Vista or later, these games will
          // be automatically found.
          //
          // Depending on GAME_INSTALL_SCOPE, write to:
          //      HKLM\Software\Microsoft\Windows\CurrentVersion\GameUX\GamesToFindOnWindowsUpgrade\{GUID}\
          // or
          //      HKCU\Software\Classes\Software\Microsoft\Windows\CurrentVersion\GameUX\GamesToFindOnWindowsUpgrade\{GUID}\
          // and write there these 2 string values: GDFBinaryPath and GameInstallPath
          //
          HKEY  hKeyGamesToFind = NULL,
                hKeyGame = NULL;
          LONG  lResult;
          DWORD dwDisposition;

          if ( InstallScope == GIS_CURRENT_USER )
          {
            lResult = RegCreateKeyEx( HKEY_CURRENT_USER, _T( "Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade" ),
                                      0, NULL, 0, KEY_WRITE, NULL, &hKeyGamesToFind, &dwDisposition );
          }
          else
          {
            lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade" ),
                                      0, NULL, 0, KEY_WRITE, NULL, &hKeyGamesToFind, &dwDisposition );
          }
          if ( ERROR_SUCCESS == lResult )
          {
            WCHAR   strGameInstanceGUID[128] = { 0 };

            if ( *pInstanceGUID == GUID_NULL )
            {
              CoCreateGuid( pInstanceGUID );
            }
            hr = StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );

            if ( SUCCEEDED( hr ) )
            {
              lResult = RegCreateKeyExW( hKeyGamesToFind, strGameInstanceGUID, 0, NULL, 0, KEY_WRITE, NULL, &hKeyGame, &dwDisposition );
              if ( ERROR_SUCCESS == lResult )
              {
                size_t        nGDFBinPath = 0,
                              nGameInstallPath = 0;

                nGDFBinPath       = wcslen( strGDFBinPath );
                nGameInstallPath  = wcslen( strGameInstallPath );
                RegSetValueExW( hKeyGame, L"GDFBinaryPath", 0, REG_SZ, (BYTE*)strGDFBinPath, (DWORD)( ( nGDFBinPath + 1 ) * sizeof( WCHAR ) ) );
                RegSetValueExW( hKeyGame, L"GameInstallPath", 0, REG_SZ, (BYTE*)strGameInstallPath, (DWORD)( ( nGameInstallPath + 1 ) * sizeof( WCHAR ) ) );
              }
              if ( hKeyGame )
              {
                RegCloseKey( hKeyGame );
              }
            }
            else
            {
              //dh::Log( "StringFromGUID2 failed" );
            }
          }
          if ( hKeyGamesToFind )
          {
            RegCloseKey( hKeyGamesToFind );
          }
        }
        else
        {
          hr = pFwGameExplorer->VerifyAccess( bstrGDFBinPath, &bHasAccess );
          if ( ( FAILED( hr ) )
          ||   ( !bHasAccess ) )
          {
            dh::Log( "AddToGameExplorerW have no access" );
            goto LCleanup;
          }

          hr = pFwGameExplorer->AddGame( bstrGDFBinPath, bstrGameInstallPath, InstallScope, pInstanceGUID );

          if ( FAILED( hr ) )
          {
            //dh::Log( "AddGame failed" );
              // If AddGame() failed, then the game may have been already added.
              // So try to retrieve existing game instance GUID
            dh::Log( "addgame failed (%X)", hr );
            RetrieveGUIDForApplicationW( bstrGDFBinPath, pInstanceGUID );
          }
        }

        LCleanup:
        if ( bstrGDFBinPath )
        {
          SysFreeString( bstrGDFBinPath );
        }
        if ( bstrGameInstallPath )
        {
          SysFreeString( bstrGameInstallPath );
        }
        if ( pFwGameExplorer )
        {
          pFwGameExplorer->Release();
        }
        if ( bCleanupCOM )
        {
          CoUninitialize();
        }
        return hr;
      }

      HRESULT AddToGameExplorerA( const char* strGDFBinPath, const char* strGameInstallPath, GAME_INSTALL_SCOPE InstallScope, GUID* pInstanceGUID )
      {
        WCHAR wstrBinPath[MAX_PATH]     = { 0 };
        WCHAR wstrInstallPath[MAX_PATH] = { 0 };

        MultiByteToWideChar( CP_ACP, 0, strGDFBinPath, MAX_PATH, wstrBinPath, MAX_PATH );
        MultiByteToWideChar( CP_ACP, 0, strGameInstallPath, MAX_PATH, wstrInstallPath, MAX_PATH );

        return AddToGameExplorerW( wstrBinPath, wstrInstallPath, InstallScope, pInstanceGUID );
      }

      GR::WString SHGetFolderPathReplacementW( int folder_id )
      {
        // Microsoft recommend the use of SHGetFolderPath to do
        // what we're attempting here, but SHGetFolderPath won't work
        // out of the box on older (Win98) flavours of Windows.
        // Either we distribute shfolder.dll (fully-redistributable part
        // of IE 4) and ensure that we link against it before we link
        // shell32.dll, or we use SHGetSpecialFolderLocation instead.
        LPITEMIDLIST id_list;
        if ( SHGetSpecialFolderLocation( 0, folder_id, &id_list ) != S_OK )
        {
          return GR::WString();
        }

        WCHAR         folder_path[MAX_PATH];
        DWORD const   result = SHGetPathFromIDListW( id_list, folder_path );

        LPMALLOC allocator;
        if ( SHGetMalloc( &allocator ) == S_OK )
        {
          allocator->Free( id_list );
          allocator->Release();
        }
        return ( result == TRUE ) ? folder_path : GR::WString();
      }


      STDAPI CreateTaskW( GAME_INSTALL_SCOPE InstallScope,   // Either GIS_CURRENT_USER or GIS_ALL_USERS
        GUID* pGameInstanceGUID,           // valid GameInstance GUID that was passed to AddGame()
        BOOL bSupportTask,                 // if TRUE, this is a support task otherwise it is a play task
        int nTaskID,                       // ID of task
        WCHAR* strTaskName,                // Name of task.  Ex "Play"
        WCHAR* strLaunchPath,              // Path to exe.  Example: "C:\Program Files\Microsoft\MyGame.exe"
        WCHAR* strCommandLineArgs )        // Can be NULL.  Example: "-windowed"
      {
        HRESULT hr;
        WCHAR strPath[512];
        WCHAR strGUID[256];
        WCHAR strCommonFolder[MAX_PATH];
        WCHAR strShortcutFilePath[512];

        // Get base path based on install scope
        if ( InstallScope == GIS_CURRENT_USER )
        {
          GR::WString    CommonFolder = SHGetFolderPathReplacementW( CSIDL_LOCAL_APPDATA );
          memcpy( strPath, CommonFolder.c_str(), CommonFolder.length() + 1 );
          //SHGetFolderPathW( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonFolder );
        }
        else
        {
          GR::WString    CommonFolder = SHGetFolderPathReplacementW( CSIDL_COMMON_APPDATA );
          memcpy( strPath, CommonFolder.c_str(), CommonFolder.length() + 1 );
          //SHGetFolderPathW( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonFolder );
        }

        // Convert GUID to string
        hr = StringFromGUID2( *pGameInstanceGUID, strGUID, 256 );
        if ( FAILED( hr ) )
        {
          return hr;
        }

        // Create dir path for shortcut
        _snwprintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s\\%s\\%d", strCommonFolder, strGUID, (bSupportTask) ? L"SupportTasks" : L"PlayTasks", nTaskID );

        // Create the directory and all intermediate directories
        GR::IO::FileUtil::CreateSubDir( GR::Convert::ToUTF8( strPath ) );
        //SHCreateDirectoryExW( NULL, strPath, NULL );

        // Create full file path to shortcut
        _snwprintf( strShortcutFilePath, 512, L"%s\\%s.lnk", strPath, strTaskName );

        // Create shortcut
        CreateShortcut( strLaunchPath, strCommandLineArgs, strShortcutFilePath );

        return S_OK;
      }

      STDAPI CreateTaskA( GAME_INSTALL_SCOPE InstallScope, GUID* pGameInstanceGUID, BOOL bSupportTask,
                          int nTaskID, const char* strTaskName, const char* strLaunchPath, const char* strCommandLineArgs )
      {
        WCHAR wstrTaskName[MAX_PATH] = {0};
        WCHAR wstrLaunchPath[MAX_PATH] = {0};
        WCHAR wstrCommandLineArgs[MAX_PATH] = {0};

        MultiByteToWideChar( CP_ACP, 0, strTaskName, MAX_PATH, wstrTaskName, MAX_PATH );
        MultiByteToWideChar( CP_ACP, 0, strLaunchPath, MAX_PATH, wstrLaunchPath, MAX_PATH );
        MultiByteToWideChar( CP_ACP, 0, strCommandLineArgs, MAX_PATH, wstrCommandLineArgs, MAX_PATH );

        return CreateTaskW( InstallScope, pGameInstanceGUID, bSupportTask, nTaskID, wstrTaskName, wstrLaunchPath, wstrCommandLineArgs );
      }

      // valid GameInstance GUID that was passed to AddGame()
      STDAPI RemoveTasks( GUID* pGUID )
      {
        HRESULT hr;
        WCHAR strPath[512] = {0};
        WCHAR strGUID[256];
        WCHAR strLocalAppData[MAX_PATH];
        WCHAR strCommonAppData[MAX_PATH];

        // Get base path based on install scope
        GR::WString    CommonFolder = SHGetFolderPathReplacementW( CSIDL_LOCAL_APPDATA );
        memcpy( strLocalAppData, CommonFolder.c_str(), CommonFolder.length() + 1 );

        CommonFolder = SHGetFolderPathReplacementW( CSIDL_COMMON_APPDATA );
        memcpy( strCommonAppData, CommonFolder.c_str(), CommonFolder.length() + 1 );

        /*
        if ( FAILED( hr = SHGetFolderPathW( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, strLocalAppData ) ) )
        {
          return hr;
        }

        if ( FAILED( hr = SHGetFolderPathW( NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, strCommonAppData ) ) )
        {
          return hr;
        }*/

        // Convert GUID to string
        if ( FAILED( hr = StringFromGUID2( *pGUID, strGUID, 256 ) ) )
        {
          return hr;
        }

        _snwprintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s", strLocalAppData, strGUID );

        SHFILEOPSTRUCTW fileOp;
        ZeroMemory( &fileOp, sizeof( SHFILEOPSTRUCTW ) );
        fileOp.wFunc = FO_DELETE;
        fileOp.pFrom = strPath;
        fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        SHFileOperationW( &fileOp );

        _snwprintf( strPath, 512, L"%s\\Microsoft\\Windows\\GameExplorer\\%s", strCommonAppData, strGUID );

        ZeroMemory( &fileOp, sizeof( SHFILEOPSTRUCTW ) );
        fileOp.wFunc = FO_DELETE;
        fileOp.pFrom = strPath;
        fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
        SHFileOperationW( &fileOp );

        return S_OK;
      }

      STDAPI RemoveFromGameExplorer( GUID *pInstanceGUID )
      {
        HRESULT hr = E_FAIL;
        IGameExplorer* pFwGameExplorer = NULL;

        hr = CoInitialize( 0 );
        bool bCleanupCOM = SUCCEEDED(hr);
        bool bVistaPath = false;

        // Create an instance of the Game Explorer Interface
        hr = CoCreateInstance( __uuidof( GameExplorer ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IGameExplorer ), (void**)&pFwGameExplorer );
        if ( SUCCEEDED( hr ) )
        {
          bVistaPath = true;
          // Remove the game from the Game Explorer
          hr = pFwGameExplorer->RemoveGame( *pInstanceGUID );
        }
        else
        {
          // On Windows XP remove reg keys
          if ( pInstanceGUID == NULL )
          {
            goto LCleanup;
          }

          WCHAR strGameInstanceGUID[128] = {0};
          hr = StringFromGUID2( *pInstanceGUID, strGameInstanceGUID, 128 );
          if ( FAILED( hr ) )
          {
            goto LCleanup;
          }

          WCHAR szKeyPath[1024];
          _snwprintf( szKeyPath, 1024, L"Software\\Classes\\Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade\\%s", strGameInstanceGUID );
          RegDelnodeW( HKEY_CURRENT_USER, szKeyPath );
          //SHDeleteKeyW( HKEY_CURRENT_USER, szKeyPath );

          _snwprintf( szKeyPath, 1024, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade\\%s", strGameInstanceGUID );
          RegDelnodeW( HKEY_LOCAL_MACHINE, szKeyPath );
          //SHDeleteKeyW( HKEY_LOCAL_MACHINE, szKeyPath );

          hr = S_OK;
          goto LCleanup;
        }

        LCleanup:
        if ( pFwGameExplorer )
        {
          pFwGameExplorer->Release();
        }
        if ( bCleanupCOM )
        {
          CoUninitialize();
        }
        return hr;
      }

      bool RegisterGameWithGameExplorer( const GR::String& PathToExecutable,
                                         const GR::String& StartParameters,
                                         bool CurrentUserOnly,
                                         GUID& guidGame )
      {
        HRESULT hRes = AddToGameExplorerA( PathToExecutable.c_str(),
                                           Path::RemoveFileSpec( PathToExecutable ).c_str(),
                                           CurrentUserOnly ? GIS_CURRENT_USER : GIS_ALL_USERS,
                                           &guidGame );
        if ( FAILED( hRes ) )
        {
          dh::Log( "AddToGameExplorerA failed" );
          return false;
        }
        hRes = CreateTaskA( CurrentUserOnly ? GIS_CURRENT_USER : GIS_ALL_USERS,
                            &guidGame,
                            FALSE,
                            0,
                            "Play Game",
                            PathToExecutable.c_str(),
                            StartParameters.c_str() );
        if ( FAILED( hRes ) )
        {
          dh::Log( "CreateTaskA failed" );
          return false;
        }
        return true;
      }

      bool UnregisterGameWithGameExplorer( GUID& guidGame )
      {
        bool      Success = true;

        HRESULT hRes = RemoveTasks( &guidGame );
        if ( FAILED( hRes ) )
        {
          Success = false;
        }
        hRes = RemoveFromGameExplorer( &guidGame );
        if ( FAILED( hRes ) )
        {
          Success = false;
        }
        return Success;
      }


    };
  };
};
