#ifndef IO_FILEUTIL_H
#define IO_FILEUTIL_H

#include <GR/GRTypes.h>

#include <list>

#include <Memory/ByteBuffer.h>

#include <String/Path.h>

#include <OS/OS.h>



namespace GR
{
  namespace DateTime
  {
    class DateTime;
  }

  namespace IO
  {
    namespace FileUtil
    {
#if OPERATING_SYSTEM == OS_WINDOWS
      bool FileExists( const GR::String& Filename );
      bool DirectoryExists( const GR::String& Dirname );

      bool Rename( const GR::WChar* OldFilename, const GR::WChar* NewFilename );
#elif OPERATING_SYSTEM == OS_TANDEM
#ifdef TANDEM_OSS
      bool ExistsOSS( const GR::Char* Filename );
      bool ExistsGuardian( const GR::Char* Filename );

      bool FileExists( const GR::String& Filename );
      bool DirectoryExists( const GR::String& Dirname );
#else
      bool FileExists( const GR::String& Filename );
#endif // TANDEM_OSS
#elif ( OPERATING_SYSTEM == OS_ANDROID ) || ( OPERATING_SYSTEM == OS_WEB ) || ( OPERATING_SYSTEM == OS_LINUX )
      bool FileExists( const GR::String& Filename );
      bool DirectoryExists( const GR::String& Dirname );
#endif // OS_ANDROID

      GR::String        ReadFileAsString( const GR::String& Filename );
      ByteBuffer        ReadFileAsBuffer( const GR::String& Filename );

      bool              WriteFileFromString( const GR::String& Filename, const GR::String& Text );
      bool              WriteFileFromBuffer( const GR::String& Filename, const ByteBuffer& Buffer );
      
      bool              AppendFileFromString( const GR::String& Filename, const GR::String& Text );
      bool              AppendFileFromBuffer( const GR::String& Filename, const ByteBuffer& Buffer );
      
      bool              Delete( const GR::String& Filename );
      bool              Rename( const GR::String& OldFilename, const GR::String& NewFilename );

#if OPERATING_SYSTEM == OS_TANDEM
  #ifdef TANDEM_OSS      
      bool              CreateSubDir( const GR::String& SubDir );
  #endif
      void              EnumFilesInDirectory( const GR::Char* FindMask, std::list<GR::String>& listFiles, bool bAllowDirectories = false, bool bClearList = true );
      bool              Rename( const GR::Char* OldFilename, const GR::Char* NewFilename );
#endif      

#if OPERATING_SYSTEM == OS_ANDROID
      bool              CreateSubDir( const GR::String& SubDir );

      void              EnumFilesInDirectory( const GR::Char* FindMask, std::list<GR::String>& listFiles, bool bAllowDirectories = false, bool bClearList = true );
      bool              Rename( const GR::Char* OldFilename, const GR::Char* NewFilename );
#endif
      
#if OPERATING_SYSTEM == OS_WEB
      bool              CreateSubDir( const GR::String& SubDir );

      void              EnumFilesInDirectory( const GR::Char* FindMask, std::list<GR::String>& listFiles, bool bAllowDirectories = false, bool bClearList = true );
      bool              Rename( const GR::Char* OldFilename, const GR::Char* NewFilename );
#endif

#if OPERATING_SYSTEM == OS_WINDOWS
      bool              DeleteSubDir( const GR::String& FolderName );

      void              EnumFilesInDirectory( const GR::String& FindMask, std::list<GR::String>& listFiles, bool bAllowDirectories = false, bool bClearList = true );

      void              EnumDirsInDirectory( const GR::String& FindMask, std::list<GR::String>& listDirs );

#undef CopyFile
      bool              CopyFile( const GR::String& OldName, const GR::String& NewName, bool OverwriteIfExists = false );

#undef MoveFile
      bool              MoveFile( const GR::String& OldName, const GR::String& NewName, bool OverwriteIfExists = false );

      bool              CreateSubDir( const GR::String& Name, const GR::String& Separator = OS_PATH_SEPARATORS );

      bool              GetFileCreationTime( const GR::String& Name, GR::DateTime::DateTime& CreationTimeUTC );
#endif
      
      GR::String        CurrentWorkingDirectory();
      GR::String        AppDataPath();
      GR::String        UserAppDataPath();
      GR::String        AppPath();
      
      bool              GetFileModificationTime( const GR::String& Name, GR::DateTime::DateTime& ModificationTimeUTC );


      bool              MatchFileMask( const char* Mask, const char* Filename );

      bool              GetSize( const GR::String& Filename, GR::u64& FileSize );
    }
  }
}

#endif // IO_FILEUTIL_H



