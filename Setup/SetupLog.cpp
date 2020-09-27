/*--------------------+-------------------------------------------------------+
 | Programmname       : Setup-Log - Uninstaller                               |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 01.09.2000                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "SetupLog.h"



/*-AddLine--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CSetupLog::AddLine( const char* szLine )
{
}



/*-ProcessList----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CSetupLog::ProcessList( const char* szDestinationPath )
{

  GR::String     strWindowsPath;


  char    szDummy[MAX_PATH];
  i = GetWindowsDirectory( szDummy, MAX_PATH );
  if ( ( i == 0 )
  ||   ( i > MAX_PATH ) )
  {
    // Der konnte den Windows-Pfad nicht reinstopfen!!
    dh::Log( "GetWindowsDirectory failed\n" );
    return false;
  }

  strWindowsPath = szDummy;
  if ( *strWindowsPath.rend() != 92 )
  {
    strWindowsPath += '\\';
  }

  std::list<GR::String>::iterator    it( listLogFile.begin() );
  while ( it != listLogFile.end() )
  {
    GR::String&    strLine = *it;

    GR::String     strParams = "";

    if ( strLine.length() < 2 )
    {
      // zu kurz, weitermachen
      ++it;
      continue;
    }
    if ( strLine.length() > 5 )
    {
      strParams = strLine.substr( 3 );
    }

    if ( strLine.substr( 0, 2 ) == "RK" )
    {
      // Registry-Key (Uninstall-Anzeige)
      GR::String   strKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

      strKey += strParams;

      CRegistryHelper::DeleteBranch( HKEY_LOCAL_MACHINE, strKey.c_str() );
    }
    else if ( strLine.substr( 0, 2 ) == "NC" )
    {
      // Normal Copy
      if ( ( strParams[1] == ':' )
      &&   ( toupper( strParams[0] ) >= 'A' )
      &&   ( toupper( strParams[0] ) <= 'Z' ) )
      {
        // ein absoluter Pfad, vorsicht!
        DeleteFile( strParams.c_str() );
      }
      else
      {
        DeleteFile( ( szDestinationPath + strParams ).c_str() );
      }
    }
    else if ( strLine.substr( 0, 2 ) == "WC" )
    {
      // Windows Copy -> Ins Windows-Hauptverzeichnis kopiert
      DeleteFile( ( strWindowsPath + strParams ).c_str() );
    }
    else if ( strLine.substr( 0, 2 ) == "SD" )
    {
      // Sub-Directory angelegt
      if ( DecreaseReferenceForDirectory( strParams.c_str() ) )
      {
        if ( !RemoveDirectory( strParams.c_str() ) )
        {
          dh::Log( "Directory not removed %x\n", GetLastError() );
          //m_wndMain.MessageBox( "NOT REMOVED", strPart.c_str(), MB_TASKMODAL | MB_SYSTEMMODAL );
        }
      }
    }
    else if ( strLine.substr( 0, 2 ) == "CI" )
    {
      // Create-Item angelegt
      RemoveDirectory( strParams.c_str() );
    }
    else if ( strLine.substr( 0, 2 ) == "CL" )
    {
      // Create Link -> LNK-File löschen
      DeleteFile( ( strDestPath + strParams ).c_str() );
    }
    else if ( strLine.substr( 0, 2 ) == "IS" )
    {
      // Saver eingetragen
      UnInstallSaver();
    }
    else if ( strLine.substr( 0, 2 ) == "IN" )
    {
      // INI-Eintrag
      if ( !RemoveIniEntry( strParams.c_str() ) )
      {
        //return FALSE;
      }
    }
    else if ( strLine.substr( 0, 2 ) == "CK" )
    {
      if ( !RemoveRegistryKey( strParams.c_str() ) )
      {
        //return FALSE;
      }
    }


    ++it;
  }

}



