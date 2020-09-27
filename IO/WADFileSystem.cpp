/*--------------------+-------------------------------------------------------+
 | Programmname       : Resource-File                                         |
 +--------------------+-------------------------------------------------------+
 | Autor              : Georg Rottensteiner                                   |
 | Datum              : 01.09.2000                                            |
 | Version            : 1.0                                                   |
 +--------------------+-------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <IO\WADFileSystem.h>
#include <IO\FileStream.h>
#include <debug/debugclient.h>



/*-Structures-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Variablen------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CWADFileSystem::CWADFileSystem()
{

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CWADFileSystem::~CWADFileSystem()
{

  m_listSearchDir.clear();
  m_listWADFiles.clear();

}



/*-AddSearchDir---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CWADFileSystem::AddSearchDir( const char *szFileName )
{

  m_listSearchDir.push_back( szFileName );

}



/*-AddWADFileToList-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CWADFileSystem::AddWADFileToList( const char* szFileName )
{

  tListWADFiles::iterator    it( m_listWADFiles.begin() );
  while ( it != m_listWADFiles.end() )
  {
    tWADFileEntry&    Entry = *it;

    if ( Entry.strFileName.compare( szFileName ) == 0 )
    {
      return true;
    }

    ++it;
  }

  m_listWADFiles.push_back( tWADFileEntry() );

  tWADFileEntry&    Entry = m_listWADFiles.back();

  if ( !Entry.WadFile.Open( szFileName ) )
  {
    m_listWADFiles.pop_back();
    return false;
  }
  Entry.strFileName = szFileName;
  return true;

}



/*-RemoveWADFileFromList------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CWADFileSystem::RemoveWADFileFromList( const char *szFileName )
{

  tListWADFiles::iterator    it( m_listWADFiles.begin() );
  while ( it != m_listWADFiles.end() )
  {
    tWADFileEntry&    Entry = *it;

    if ( Entry.strFileName.compare( szFileName ) == 0 )
    {
      m_listWADFiles.erase( it );
      return;
    }
    ++it;
  }

}



GR::IO::FileStream CWADFileSystem::OpenFile( const char* szFileName )
{

  if ( ( m_listSearchDir.empty() )
  &&   ( m_listWADFiles.empty() ) )
  {
    dh::Log( "CWADFileSystem::OpenFile  No Source given when trying to open %s\n", szFileName );
    return GR::IO::FileStream();
  }
  tListSearchDirs::iterator   it( m_listSearchDir.begin() );
  while ( it != m_listSearchDir.end() )
  {
    GR::String   strPath = *it;

    if ( ( !strPath.empty() )
    &&   ( strPath[strPath.length() - 1] != '\\' ) )
    {
      strPath += '\\';
    }
    strPath += szFileName;

    GR::IO::FileStream   Stream;
    if ( Stream.Open( strPath.c_str() ) )
    {
      return Stream;
    }

    ++it;
  }

  // jetzt die WADFiles durchsuchen
  tListWADFiles::iterator   itWAD( m_listWADFiles.begin() );
  while ( itWAD != m_listWADFiles.end() )
  {
    tWADFileEntry&    WadFile = *itWAD;

    int   iIndex = WadFile.WadFile.GetEntryIndex( szFileName );
    if ( iIndex != -1 )
    {
      GR::IO::FileStream   Stream( WadFile.strFileName.c_str() );
      Stream.SetPosition( WadFile.WadFile.GetEntryOffset( iIndex ) );

      return Stream;
    }

    ++itWAD;
  }

  dh::Log( "CWADFileSystem::OpenFile  File not found %s\n", szFileName );
  return GR::IO::FileStream();

}



CWADFileSystem& CWADFileSystem::Instance()
{

  static CWADFileSystem   g_Instance;

  return g_Instance;

}