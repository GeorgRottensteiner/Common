#include "FileLogger.h"

#include <Misc/Misc.h>
#include <Misc/Format.h>

#include <IO/FileUtil.h>

#include <String/Path.h>

#include <DateTime/DateTime.h>



FileLogger::FileLogger() :
  m_NumFiles( 0 ),
  m_MaxSize( 0 ), 
  m_NumCallsSinceLastRotateCheck( 0 ),
  m_ActiveFileIndex( -1 ),
  m_LogLevel( LL_INFO ),
  m_Verbose( false ),
  m_IsSetup( false )
{
}



FileLogger::FileLogger( const GR::String& Filename, int NumFiles, GR::u64 MaxSize, LogLevel LogLevel ) :
  m_Filename( Filename ),
  m_NumFiles( NumFiles ),
  m_MaxSize( MaxSize ), 
  m_NumCallsSinceLastRotateCheck( 0 ),
  m_ActiveFileIndex( -1 ),
  m_LogLevel( LogLevel ),
  m_Verbose( false ),
  m_IsSetup( true )
{
  DetermineFirstFile();
}



FileLogger& FileLogger::Instance()
{
  static FileLogger    instance;
  
  return instance;
}



FileLogger::LogLevel FileLogger::LogLevelFromProperty( const GR::String& Property )
{
  LogLevel logLevel = LL_INFO;
  GR::String    upperCase( Property );
  upperCase.ToUpper();
  
  if ( upperCase == "DEBUG" ) 
  {
    logLevel = LL_DEBUG;
  } 
  else if ( upperCase == "NONE" ) 
  {
    logLevel = LL_NONE;
  } 
  else if ( upperCase == "ERROR" ) 
  {
    logLevel = LL_ERROR;
  } 
  else if ( upperCase == "WARNING" ) 
  {
    logLevel = LL_WARNING;
  }
  return logLevel;
}



void FileLogger::ResetAllFiles()
{
  if ( !m_IsSetup )
  {
    return;
  }
  for ( int i = 0; i < m_NumFiles; ++i )
  {
    GR::String    setFilename = Path::RenameFile( m_Filename,
      Path::FileNameWithoutExtension( m_Filename ) + ( Misc::Format() << ( i + 1 ) )
      + "."
      + Path::Extension( m_Filename ) );

    GR::IO::FileUtil::Delete( setFilename );
  }
  DetermineFirstFile();
}



void FileLogger::DetermineFirstFile()
{
  if ( !m_IsSetup )
  {
    return;
  }
  
  int         bestMatchIndex = -1;
  GR::u64     bestSize = m_MaxSize;
  GR::String  bestFilename;
  int         bestMatchByDate = -1;
  GR::String  bestFilenameByDate;
  GR::DateTime::DateTime    oldestDate( GR::DateTime::DateTime::Now() );
  int         missingFileIndex = -1;
  GR::String  missingFilename;

  for ( int i = 0; i < m_NumFiles; ++i )
  {
    GR::String    setFilename = Path::RenameFile( m_Filename,
      Path::FileNameWithoutExtension( m_Filename ) + ( Misc::Format() << ( i + 1 ) )
      + "."
      + Path::Extension( m_Filename ) );

    GR::u64   curSize = 0;
    if ( !GR::IO::FileUtil::FileExists( setFilename ) )
    {
      // this file is missing, use as fallback if no existing file is small enough
      if ( missingFileIndex == -1 )
      {
        missingFileIndex  = i;
        missingFilename   = setFilename;
      }
    }

    if ( GR::IO::FileUtil::FileExists( setFilename ) )
    {
      if ( GR::IO::FileUtil::GetSize( setFilename, curSize ) )
      {
        if ( ( curSize < m_MaxSize )
        &&   ( curSize < bestSize ) )
        {
          bestSize        = curSize;
          bestFilename    = setFilename;
          bestMatchIndex  = i;
        }
      }
      else
      {
        printf( "FileLogger::DetermineFirstFile failed to get size of file %s\n", setFilename.c_str() );
      }
    }    
    GR::DateTime::DateTime    modTime;
    if ( GR::IO::FileUtil::GetFileModificationTime( setFilename, modTime ) )
    {
      if ( modTime < oldestDate )
      {
        oldestDate          = modTime;
        bestMatchByDate     = i;
        bestFilenameByDate  = setFilename;
      }
    }
  }
  if ( bestMatchIndex == -1 )
  {
    // none of the existing files are small enough, fallback to a missing one
    if ( missingFileIndex != -1 )
    {
      bestFilename    = missingFilename;
      bestMatchIndex  = missingFileIndex;
    }
    else
    {
      // none of the files are small enough, delete the oldest
      if ( !GR::IO::FileUtil::Delete( bestFilenameByDate ) )
      {
        printf( "FileLogger::DetermineFirstFile failed to delete log file %s\n", bestFilenameByDate.c_str() );
      }
      
      bestMatchIndex  = bestMatchByDate;
      bestFilename    = bestFilenameByDate;
    }
  }

  m_ActiveFileIndex = bestMatchIndex;
  m_ActiveFilename  = bestFilename;
}



void FileLogger::Log( const GR::String& Message )
{
  if ( m_ActiveFilename.empty() )
  {
    printf( "%s%s\n", CMisc::TimeStamp(), Message.c_str() );
    return;
  }
  
  if ( m_Verbose )
  {
    printf( "%s\n", Message.c_str() );
  }
  
  GR::IO::FileUtil::AppendFileFromString( m_ActiveFilename, Misc::Format( "%1%%2%\n" ) << CMisc::TimeStamp() << Message );
  
  // 2000 is an arbitrary value, just to avoid reading the file size all the time
  ++m_NumCallsSinceLastRotateCheck;
  if ( m_NumCallsSinceLastRotateCheck >= 2000 )
  {
    RotateFiles();
  }
}



void FileLogger::LogRaw( const GR::String& Message )
{
  if ( m_ActiveFilename.empty() )
  {
    printf( "%s%s\n", CMisc::TimeStamp(), Message.c_str() );
    return;
  }
  
  GR::IO::FileUtil::AppendFileFromString( m_ActiveFilename, Misc::Format( "%1%\n" ) << Message );
  
  // 2000 is an arbitrary value, just to avoid reading the file size all the time
  ++m_NumCallsSinceLastRotateCheck;
  if ( m_NumCallsSinceLastRotateCheck >= 2000 )
  {
    RotateFiles();
  }
}



void FileLogger::LogDebug( const GR::String& Message )
{
  if ( m_LogLevel < LL_DEBUG )
  {
    return;
  }
  Log( "[dbg]: " + Message );
}



void FileLogger::LogInfo( const GR::String& Message )
{
  Log( "[inf]: " + Message );
}



void FileLogger::LogError( const GR::String& Message )
{
  if ( m_LogLevel < LL_ERROR )
  {
    return;
  }
  Log( "[err]: " + Message );
}



void FileLogger::LogWarning( const GR::String& Message )
{
  if ( m_LogLevel < LL_WARNING )
  {
    return;
  }
  Log( "[wrn]: " + Message );
}



void FileLogger::RotateFiles()
{
  GR::u64     currentLogSize = 0;
  
  if ( !GR::IO::FileUtil::GetSize( m_ActiveFilename, currentLogSize ) )
  {
    printf( "FileLogger::RotateFiles failed to get size of file %s\n", m_ActiveFilename.c_str() );
    return;
  }
  if ( currentLogSize < m_MaxSize )
  {
    // nothing to do
    return;
  }

  DetermineFirstFile();
}



void FileLogger::SetVerbose( bool Verbose )
{
  m_Verbose = Verbose;
}



void FileLogger::Setup( const GR::String& Filename, int NumFiles, GR::u64 MaxSize, LogLevel LogLevel )
{
  m_ActiveFileIndex               = -1;
  m_NumFiles                      = NumFiles;
  m_MaxSize                       =  MaxSize;
  m_Filename                      = Filename;
  m_LogLevel                      = LogLevel;
  m_Verbose                       = false;
  m_IsSetup                       = true;
  m_NumCallsSinceLastRotateCheck  = 0;
  m_ActiveFilename.clear();
  DetermineFirstFile();
}

    


