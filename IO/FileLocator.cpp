#include "FileLocator.h"

#include <IO/FileStream.h>
#include <IO/FileUtil.h>

#include <debug/debugclient.h>



namespace GR
{
  namespace IO
  {
    void FileLocator::AddSource( const char* pFolder )
    {
      m_SourceFolders.push_back( pFolder );
    }



    void FileLocator::RemoveSource( const char* pFolder )
    {
      m_SourceFolders.remove( pFolder );
    }



    bool FileLocator::FileExists( const char* pFilename )
    {
      tSourceFolders::iterator    it( m_SourceFolders.begin() );
      while ( it != m_SourceFolders.end() )
      {
        if ( GR::IO::FileUtil::FileExists( Path::Append( *it, pFilename ) ) )
        {
          return true;
        }
        ++it;
      }
      return false;
    }



    bool FileLocator::OpenFile( const char* pFilename, IIOStream& Stream )
    {
      if ( m_SourceFolders.empty() )
      {
        dh::Log( "GR::IO::FileLocator::OpenFile No source folders given when trying to open %s", pFilename );
        return false;
      }
      tSourceFolders::iterator    it( m_SourceFolders.begin() );
      while ( it != m_SourceFolders.end() )
      {
        if ( GR::IO::FileUtil::FileExists( Path::Append( *it, pFilename ) ) )
        {
          Stream = GR::IO::FileStream( Path::Append( *it, pFilename ) );

          return true;
        }

        ++it;
      }
      dh::Log( "GR::IO::FileLocator::OpenFile File not found %s", pFilename );
      return false;
    }



    const char* FileLocator::LocateFile( const char* pFilename )
    {
      if ( m_SourceFolders.empty() )
      {
        dh::Log( "GR::IO::FileLocator::LocateFile No source folders given when trying to open %s", pFilename );
        return "";
      }
      
      // in case of wild cards use the first path with potential files
      tSourceFolders::iterator    it( m_SourceFolders.begin() );
      while ( it != m_SourceFolders.end() )
      {
        if ( GR::IO::FileUtil::FileExists( Path::Append( *it, pFilename ) ) )
        {
          static GR::String resultString;

          resultString = Path::Append( *it, pFilename );
          return resultString.c_str();
        }

        ++it;
      }
      dh::Log( "GR::IO::FileLocator::LocateFile File not found %s", pFilename );
      return "";
    }



    FileLocator& FileLocator::Instance()
    {
      static FileLocator   g_Instance;

      return g_Instance;
    }

  }
}