#include <String/Convert.h>
#include <String/Path.h>
#include <WinSys/RegistryHelper.h>
#include <IO/FileUtil.h>

#include <tchar.h>

#include "Groups.h"

#include <windows.h>
#include <shlobj.h>



GR::String CProgramGroups::GetShellFolder( const GR::Char *szFolderName, bool bCurrentUser )
{
  HKEY            hKey;

  GR::String  strFolderName = szFolderName;

  if ( bCurrentUser )
  {
    hKey = HKEY_CURRENT_USER;
  }
  else
  {
    hKey = HKEY_LOCAL_MACHINE;
    strFolderName = "Common " + strFolderName;
  }

  GR::String      shellFolder;

  if ( !Registry::GetKey( hKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", strFolderName, shellFolder ) )
  {
    return GR::String();
  }
  return shellFolder;
}



GR::String CProgramGroups::GetSystemFolder( const GR::Char* FolderName )
{
  GR::String      systemFolder;

  if ( !Registry::GetKey( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", FolderName, systemFolder ) )
  {
    return GR::String();
  }
  return Path::AddSeparator( systemFolder );
}



GR::String CProgramGroups::GetStartMenuProgramPath( bool bCurrentUser )
{
  return GetShellFolder( "Programs", bCurrentUser );
}



bool CProgramGroups::CreateProgramGroup( const GR::Char* Description, bool bCurrentUser )
{
  GR::String  strStartMenuPath;


  strStartMenuPath = GetStartMenuProgramPath( bCurrentUser );

  strStartMenuPath += "\\";
  strStartMenuPath += Description;

  if ( GR::IO::FileUtil::DirectoryExists( strStartMenuPath ) )
  {
    // das Verzeichnis ist schon da
    return true;
  }
  if ( !GR::IO::FileUtil::CreateSubDir( strStartMenuPath ) )
  {
    return false;
  }
  return true;
}



bool CProgramGroups::CreateLink( const GR::Char* Path, 
                                 const GR::Char* Description, 
                                 const GR::Char* File, 
                                 const GR::Char* IconPath, 
                                 GR::u32 IconIndex, 
                                 const GR::Char* Parameter )
{
  HRESULT       hRes;

  IShellLinkW*    pISL;

  IPersistFile*   pIPF;

  bool            bResult;


  if ( !SUCCEEDED( CoInitialize( NULL ) ) )
  {
    return false;
  }
  hRes = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&pISL );
  if ( !SUCCEEDED( hRes ) )
  {
    CoUninitialize();
    return false;
  }
  hRes = pISL->QueryInterface( IID_IPersistFile, (void **)&pIPF );
  if ( !SUCCEEDED( hRes ) )
  {
    //MessageBox( NULL, "failed to query isl", "error", MB_OK | MB_APPLMODAL );
    CoUninitialize();
    return false;
  }

  GR::WString     fileUC = GR::Convert::ToUTF16( File );
  GR::String      path = Path::ParentDirectory( File );
  GR::WString     pathUC = GR::Convert::ToUTF16( path );
  GR::WString     paramsUC = GR::Convert::ToUTF16( Parameter );
  GR::WString     iconUC = GR::Convert::ToUTF16( IconPath );

  pISL->SetPath( fileUC.c_str() );

  
  pISL->SetWorkingDirectory( pathUC.c_str() );

  if ( Parameter )
  {
    pISL->SetArguments( paramsUC.c_str() );
  }
  if ( IconPath != NULL )
  {
    pISL->SetIconLocation( iconUC.c_str(), IconIndex );
  }

  GR::String       strDummy;


  strDummy = Path;
  if ( strDummy.length() > 0 )
  {
    if ( strDummy[strDummy.length() - 1] != 92 )
    {
      strDummy += '\\';
    }
  }
  strDummy += Description;
  strDummy += ".lnk";

  // falls der Shortcut schon existiert, löschen, sonst klappt Save nicht!
  GR::IO::FileUtil::Delete( strDummy );

  hRes = pIPF->Save( GR::Convert::ToUTF16( strDummy ).c_str(), STGM_READ );
  
  if ( SUCCEEDED( hRes ) )
  {
    bResult = true;
  }
  else
  {
    bResult = false;
  }
  pIPF->Release();
  pISL->Release();

  CoUninitialize();
  return bResult;
}



bool CProgramGroups::CreateProgramLink( const GR::Char* Description, 
                                        const GR::Char* File, 
                                        const GR::Char* Parameter, 
                                        const GR::Char* IconPath, 
                                        GR::u32 IconIndex, 
                                        bool bCurrentUser )
{
  HKEY            hKey;

  GR::String  strShellFolder = "Programs";
  GR::String  startMenuPath;



  if ( bCurrentUser )
  {
    hKey = HKEY_CURRENT_USER;
  }
  else
  {
    hKey = HKEY_LOCAL_MACHINE;
    strShellFolder = "Common Programs";
  }

  if ( !Registry::SetKey( hKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", strShellFolder.c_str(), startMenuPath.c_str() ) )
  {
    return false;
  }
  return CreateLink( startMenuPath.c_str(), Description, File, IconPath, IconIndex, Parameter );
}



bool CProgramGroups::CreateDesktopLink( const GR::Char* Description, 
                                        const GR::Char* File, 
                                        const GR::Char* IconPath, 
                                        GR::u32 IconIndex, 
                                        bool bCurrentUser )
{
  GR::String    desktopPath;

  HKEY          hKey;


  if ( bCurrentUser )
  {
    hKey = HKEY_CURRENT_USER;
  }
  else
  {
    hKey = HKEY_LOCAL_MACHINE;
  }

  if ( !Registry::GetKey( hKey, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Desktop", desktopPath ) )
  {
    return false;
  }
  return CreateLink( desktopPath.c_str(), Description, File, IconPath, IconIndex );
}



