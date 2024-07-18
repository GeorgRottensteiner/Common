#include "FileUtil.h"

#include <IO/FileStream.h>

#include <DateTime/DateTime.h>

#include <OS/OS.h>

#include <debug/debugclient.h>

#include <String/StringUtil.h>

#include <ctime>



#if ( OPERATING_SYSTEM == OS_ANDROID ) || ( OPERATING_SYSTEM == OS_WEB )
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#if ( ( OPERATING_SUB_SYSTEM != OS_SUB_WINDOWS_PHONE ) && ( OPERATING_SYSTEM != OS_WINDOWS ) )
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#if OPERATING_SYSTEM == OS_WINDOWS
#include <windows.h>
#include <ShlObj.h>



namespace
{
  void MakeBackSlashes( GR::String& Path )
  {
    for ( size_t i = 0; i < Path.length(); ++i )
    {
      if ( Path[i] == '/' )
      {
        Path[i] = '\\';
      }
    }
  }

  void MakeBackSlashes( GR::WString& Path )
  {
    for ( size_t i = 0; i < Path.length(); ++i )
    {
      if ( Path[i] == L'/' )
      {
        Path[i] = L'\\';
      }
    }
  }

  GR::WString MakeSafeWinFilename( const GR::String& UTF8Filename )
  {
    GR::String    checkFilename = UTF8Filename;
    if ( Path::IsRelative( checkFilename ) )
    {
      checkFilename = Path::Append( GR::IO::FileUtil::CurrentWorkingDirectory(), checkFilename );
    }
    GR::WString     result = GR::WString( L"\\\\?\\" ) + GR::Convert::ToUTF16( GR::Strings::Replace( GR::Strings::Replace( checkFilename, "\\.\\", "\\" ), "/", "\\" ) );

    MakeBackSlashes( result );
    return result;
  }


}

#endif

#if OPERATING_SYSTEM == OS_TANDEM
  #include <String/Path.h>
  #include <cextdecs.h>
  #include <tal.h>
  #ifdef TANDEM_OSS
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/param.h>
    #include <dirent.h>
  #endif
#endif


namespace GR
{
  namespace IO
  {
    namespace FileUtil
    {
      GR::String CurrentWorkingDirectory()
      {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        using namespace Windows::ApplicationModel;
        using namespace Windows::Storage;

        auto installLocation = GR::WString( Package::Current->InstalledLocation->Path->Data() );
        auto dataLocation = GR::WString( ApplicationData::Current->LocalFolder->Path->Data() );

        return GR::Convert::ToUTF8( installLocation );
#elif ( OPERATING_SYSTEM == OS_WINDOWS )
        GR::WChar*    pBuffer = NULL;
        GR::u32       requiredLength = 0;
        requiredLength = ::GetCurrentDirectoryW( requiredLength, NULL );

        pBuffer = new GR::WChar[requiredLength + 2];
        ::GetCurrentDirectoryW( requiredLength, pBuffer );

        GR::String   currentFolder = GR::Convert::ToUTF8( pBuffer );
        delete[] pBuffer;

        return currentFolder;
#else
        char            origPath[2048 + 1];

        getcwd( origPath, 2048 );

        return origPath;
#endif
      }



#if OPERATING_SYSTEM == OS_WINDOWS
      bool FileExists( const GR::String& Filename )
      {
        GR::WString     utf16Filename = MakeSafeWinFilename( Filename );

        WIN32_FILE_ATTRIBUTE_DATA   attributes;
        if ( !GetFileAttributesExW( utf16Filename.c_str(), GetFileExInfoStandard, &attributes ) )
        {
          GR::u32   Error = GetLastError();
          if ( ( Error == ERROR_FILE_NOT_FOUND )
          ||   ( Error == ERROR_PATH_NOT_FOUND )
          ||   ( Error == ERROR_INVALID_NAME )
          ||   ( Error == ERROR_INVALID_DRIVE )
          ||   ( Error == ERROR_NOT_READY )
          ||   ( Error == ERROR_INVALID_PARAMETER )
          ||   ( Error == ERROR_BAD_PATHNAME )
          ||   ( Error == ERROR_BAD_NETPATH ) )
          {
            return false;
          }
        }
        return true;

        /*
        GR::u32   Attributes = GetFileAttributesW( utf16Filename.c_str() );
        if ( Attributes == INVALID_FILE_ATTRIBUTES )
        {
          GR::u32   Error = GetLastError();
          if ( Error == ERROR_FILE_NOT_FOUND )
          {
            return false;
          }
        }
        return true;*/
      }



      bool DirectoryExists( const GR::String& Dirname )
      {
        GR::WString     utf16Filename = MakeSafeWinFilename( Dirname );

        WIN32_FILE_ATTRIBUTE_DATA   attributes;
        if ( !GetFileAttributesExW( utf16Filename.c_str(), GetFileExInfoStandard, &attributes ) )
        {
          GR::u32   Error = GetLastError();
          if ( ( Error == ERROR_FILE_NOT_FOUND )
          ||   ( Error == ERROR_PATH_NOT_FOUND )
          ||   ( Error == ERROR_INVALID_NAME )
          ||   ( Error == ERROR_INVALID_DRIVE )
          ||   ( Error == ERROR_NOT_READY )
          ||   ( Error == ERROR_INVALID_PARAMETER )
          ||   ( Error == ERROR_BAD_PATHNAME )
          ||   ( Error == ERROR_BAD_NETPATH ) )
          {
            return false;
          }
        }
        /*
        GR::u32   Attributes = GetFileAttributesW( utf16Filename.c_str() );
        if ( Attributes == INVALID_FILE_ATTRIBUTES )
        {
          GR::u32   Error = GetLastError();
          if ( Error == ERROR_FILE_NOT_FOUND )
          {
            return false;
          }
        }*/
        if ( attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
          return true;
        }
        return false;
      }
#elif OPERATING_SYSTEM == OS_TANDEM
#ifdef TANDEM_OSS
      bool ExistsGuardian( const GR::Char* Filename )
      {
        _cc_status Status = FILE_GETINFOBYNAME_( Filename,
                                                 (short)strlen( Filename ) );
        if ( Status == 11 )
        {
          // file does not exist
          return false;
        }
        return true;
      }

      bool ExistsOSS( const GR::Char* Filename )
      {
        struct stat st;

        int   result = stat( Filename, &st );
        return ( result  == 0 );
      }



      bool FileExists( const GR::String& Filename )
      {
        struct stat sb;

        if ( ( stat( Filename.c_str(), &sb ) == 0 )
        &&   ( !S_ISDIR( sb.st_mode ) ) )
        {
          return true;
        }
        return false;
      }



      bool DirectoryExists( const GR::String& Dirname )
      {
        struct stat sb;

        if ( ( stat( Dirname.c_str(), &sb ) == 0 )
        &&   ( S_ISDIR( sb.st_mode ) ) )
        {
          return true;
        }
        return false;
      }



#else
      bool FileExists( const GR::String& Filename )
      {
        _cc_status Status = FILE_GETINFOBYNAME_( Filename.c_str(),
                                                 (short)Filename.length() );
        if ( Status == 11 )
        {
          // file does not exist
          return false;
        }
        return true;
      }
#endif // TANDEM_OSS
#endif // OS_TANDEM



#if ( OPERATING_SYSTEM == OS_ANDROID ) || ( OPERATING_SYSTEM == OS_WEB ) || ( OPERATING_SYSTEM == OS_LINUX )
      bool FileExists( const GR::String& Filename )
      {
        struct stat sb;

        if ( ( stat( Filename.c_str(), &sb ) == 0 )
        &&   ( !S_ISDIR( sb.st_mode ) ) )
        {
          return true;
        }
        return false;
      }



      bool DirectoryExists( const GR::String& Dirname )
      {
        struct stat sb;

        if ( ( stat( Dirname.c_str(), &sb ) == 0 )
        &&   ( S_ISDIR( sb.st_mode ) ) )
        {
          return true;
        }
        return false;
      }
#endif



      bool Delete( const GR::String& Filename )
      {
#if OPERATING_SYSTEM == OS_WINDOWS
        GR::WString     fullFilename = MakeSafeWinFilename( Filename );

        return !!DeleteFileW( fullFilename.c_str() );
#elif OPERATING_SYSTEM == OS_TANDEM
#ifdef TANDEM_OSS
        return !remove( Filename.c_str() );
#else
        short Error = FILE_PURGE_( Filename.c_str(), (short)Filename.length() );
        return ( Error == 0 );
#endif // TANDEM_OSS
#else // OPERATING_SYSTEM == OS_TANDEM
        return !remove( Filename.c_str() );
#endif
      }



#if ( ( ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_OSS ) ) || ( OPERATING_SYSTEM == OS_ANDROID ) || ( OPERATING_SYSTEM == OS_WEB ) )
      bool CreateSubDir( const GR::String& SubDir )
      {
        int             i = 0;

        GR::String      Dest,
                        TempPath;

        char            origPath[2048 + 1];


        getcwd( origPath, 2048 );

        Dest = SubDir;
        while ( Dest[i] )
        {
          TempPath += Dest[i];

          if ( ( Dest[i + 1] == '/' )
          ||   ( Dest[i + 1] == 0 ) )
          {
            // Wir haben ein weiteres Unterverzeichnis entdeckt
            if ( chdir( TempPath.c_str() ) )
            {
              // Verzeichnis existiert also noch nicht
              chdir( origPath );
              unsigned long origUMask = umask( 0 );
              // octal constant! (WTF!)
              if ( mkdir( TempPath.c_str(), 0777 ) )
              {
                // Konnte Verzeichnis nicht anlegen, Hurra
                umask( origUMask );
                return false;
              }
              else
              {
                // Mitloggen, falls wir abbrechen müssen
                umask( origUMask );
              }
            }
            else
            {
              // Mitloggen, falls wir abbrechen müssen
            }
          }
          i++;
        }
        chdir( origPath );
        return true;
      }



      void EnumFilesInDirectory( const GR::Char* FindMask, std::list<GR::String>& listFiles, bool AllowDirectories, bool ClearList )
      {
        if ( ClearList )
        {
          listFiles.clear();
        }
        struct dirent* de = NULL;
        DIR* d = NULL;

        GR::String     searchPath = Path::RemoveFileSpec( FindMask );
        GR::String     fileMask = Path::StripPath( FindMask );

        d = opendir( searchPath.c_str() );
        if ( d == NULL )
        {
          // Couldn't open directory
          return;
        }

        // Loop while not NULL
        while ( ( de = readdir( d ) ) )
        {
          if ( MatchFileMask( fileMask.c_str(), de->d_name ) )
          {
            if ( !AllowDirectories )
            {
              struct stat entrystat;
              if ( ( !stat( Path::Append( searchPath, de->d_name ).c_str(), &entrystat ) )
              &&   ( ( entrystat.st_mode & S_IFMT ) == S_IFDIR ) )
              {
                // entry is a directory
              }
              else
              {
                listFiles.push_back( Path::Append( searchPath, de->d_name ) );
              }
            }
            else
            {
              listFiles.push_back( Path::Append( searchPath, de->d_name ) );
            }
          }
        }
        closedir( d );
      }

      bool Rename( const GR::Char* OldFilename, const GR::Char* NewFilename )
      {
        return !rename( OldFilename, NewFilename );
      }


#elif ( OPERATING_SYSTEM == OS_TANDEM )
      void EnumFilesInDirectory( const GR::Char* FindMask, std::list<GR::String>& listFiles, bool AllowDirectories, bool ClearList )
      {
        if ( ClearList )
        {
          listFiles.clear();
        }
        short   searchID = -1;

        short   result = FILENAME_FINDSTART_ ( &searchID,
                                               FindMask,
                                               (short)strlen( FindMask ) );
        if ( _status_lt( result ) )
        {
          return;
        }

        char    name[128];

        result = FILENAME_FINDNEXT_( searchID, name, 128 );
        while ( result == 0 )
        {
          listFiles.push_back( name );

          result = FILENAME_FINDNEXT_( searchID, name, 128 );
        }
        result = FILENAME_FINDFINISH_( searchID );
      }



      bool Rename( const GR::Char* OldFilename, const GR::Char* NewFilename )
      {
        short fileNum = -1;
        short result = FILE_OPEN_( OldFilename,
                                   (short)strlen( OldFilename ),
                                   &fileNum,
                                   0,     // read-write
                                   1 );   // exclusive
        if ( result != 0 )
        {
          return false;
        }
        result = FILE_RENAME_( fileNum, NewFilename, (short)strlen( NewFilename ) );
        if ( result != 0 )
        {
          FILE_CLOSE_( fileNum );
          return false;
        }
        FILE_CLOSE_( fileNum );
        return true;
      }


#endif

#if OPERATING_SYSTEM == OS_WINDOWS
      bool DeleteSubDir( const GR::String& FolderName )
      {
        GR::WString     fullFolderName = MakeSafeWinFilename( FolderName );

        return !!RemoveDirectoryW( fullFolderName.c_str() );
      }



      void EnumFilesInDirectory( const GR::String& FindMask, std::list<GR::String>& listFiles, bool bAllowDirectories, bool bClearList )
      {
        WIN32_FIND_DATAW   wFindData;

        if ( bClearList )
        {
          listFiles.clear();
        }

        GR::String  workDirUTF8 = Path::AddSeparator( Path::RemoveFileSpec( FindMask ) );

        GR::WString     workDir = GR::Convert::ToUTF16( workDirUTF8 );
        if ( workDir.empty() )
        {
          workDir = GR::Convert::ToUTF16( GR::IO::FileUtil::CurrentWorkingDirectory() );
          if ( ( !workDir.empty() )
          &&   ( workDir[workDir.length() - 1] != L'\\' ) )
          {
            workDir += L'\\';
          }
        }


        if ( workDir.find_last_of( L'\\' ) != GR::String::npos )
        {
          // mit Backslash!
          workDir = workDir.substr( 0, workDir.find_last_of( L'\\' ) + 1 );
        }
        // FindFirstFile fails with forward slashes
        for ( size_t i = 0; i < workDir.length(); ++i )
        {
          if ( workDir[i] == L'/' )
          {
            workDir[i] = L'\\';
          }
        }
        GR::WString longFindMask = MakeSafeWinFilename( FindMask );

        HANDLE hFind = FindFirstFileExW( longFindMask.c_str(), FindExInfoStandard, &wFindData, FindExSearchNameMatch, NULL, 0 );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
          //GR::u32   lastError = GetLastError();
          //dh::Log( "FindFirstFileExW returned code %d", lastError );
          return;
        }
        while ( hFind != INVALID_HANDLE_VALUE )
        {
          if ( wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
          {
            if ( ( bAllowDirectories )
            &&   ( wcscmp( wFindData.cFileName, L"." ) )
            &&   ( wcscmp( wFindData.cFileName, L".." ) ) )
            {
              listFiles.push_back( GR::Convert::ToUTF8( workDir + wFindData.cFileName ) );
            }
          }
          else
          {
            listFiles.push_back( GR::Convert::ToUTF8( workDir + wFindData.cFileName ) );
          }
          if ( !FindNextFileW( hFind, &wFindData ) )
          {
            FindClose( hFind );
            return;
          }
        }
      }



      void EnumDirsInDirectory( const GR::String& FindMask, std::list<GR::String>& listDirs )
      {
        WIN32_FIND_DATAW   wFindData;

        listDirs.clear();

        GR::WString     workDir = GR::Convert::ToUTF16( FindMask );

        size_t    iPos = workDir.find_last_of( L'\\' );

        if ( iPos == GR::String::npos )
        {
          workDir.clear();
        }
        else
        {
          workDir = workDir.substr( 0, iPos + 1 );
        }

        if ( workDir.empty() )
        {
          workDir = GR::Convert::ToUTF16( GR::IO::FileUtil::CurrentWorkingDirectory() );

          if ( ( !workDir.empty() )
          &&   ( workDir[workDir.length() - 1] != L'\\' ) )
          {
            workDir += L'\\';
          }
        }


        if ( workDir.find_last_of( L'\\' ) != GR::String::npos )
        {
          // mit Backslash!
          workDir = workDir.substr( 0, workDir.find_last_of( L'\\' ) + 1 );
        }
        // FindFirstFile fails with forward slashes
        for ( size_t i = 0; i < workDir.length(); ++i )
        {
          if ( workDir[i] == L'/' )
          {
            workDir[i] = L'\\';
          }
        }
        GR::WString longFindMask = MakeSafeWinFilename( FindMask );

        HANDLE hFind = FindFirstFileExW( longFindMask.c_str(), FindExInfoBasic, &wFindData, FindExSearchNameMatch, NULL, 0 );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
          return;
        }
        while ( hFind != INVALID_HANDLE_VALUE )
        {
          if ( wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
          {
            if ( ( wcscmp( wFindData.cFileName, L"." ) )
            &&   ( wcscmp( wFindData.cFileName, L".." ) ) )
            {
              listDirs.push_back( GR::Convert::ToUTF8( workDir + wFindData.cFileName ) );
            }
          }
          if ( !FindNextFileW( hFind, &wFindData ) )
          {
            FindClose( hFind );
            return;
          }
        }
      }



      bool Rename( const GR::String& OldFilename, const GR::String& NewFilename )
      {
        GR::WString     oldName = MakeSafeWinFilename( OldFilename );
        GR::WString     newName = MakeSafeWinFilename( NewFilename );
        return !!MoveFileExW( oldName.c_str(), newName.c_str(), MOVEFILE_WRITE_THROUGH );
      }



#undef CopyFile
      bool CopyFile( const GR::String& OldFilename, const GR::String& NewFilename, bool OverwriteIfExists )
      {
        GR::WString     oldName = MakeSafeWinFilename( OldFilename );
        GR::WString     newName = MakeSafeWinFilename( NewFilename );

#if ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL )
        return !!::CopyFileW( oldName.c_str(), newName.c_str(), !OverwriteIfExists );
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        COPYFILE2_EXTENDED_PARAMETERS   params;

        params.dwSize       = sizeof( COPYFILE2_EXTENDED_PARAMETERS );
        params.dwCopyFlags  = COPY_FILE_FAIL_IF_EXISTS;
        return !!::CopyFile2( oldName.c_str(), newName.c_str(), &params );
#endif
      }



#undef MoveFile
      bool MoveFile( const GR::String& OldFilename, const GR::String& NewFilename, bool OverwriteIfExists )
      {
        GR::WString     oldName = MakeSafeWinFilename( OldFilename );
        GR::WString     newName = MakeSafeWinFilename( NewFilename );

        GR::u32   Flags = MOVEFILE_COPY_ALLOWED;
        if ( OverwriteIfExists )
        {
          Flags |= MOVEFILE_REPLACE_EXISTING;
        }
        return !!::MoveFileExW( oldName.c_str(), newName.c_str(), Flags );
      }



      bool CreateSubDir( const GR::String& SubDir, const GR::String& Separator )
      {
        int             i;

        GR::String     dest = "\\\\?\\" + SubDir;
        GR::String     sepChars = Separator;

        GR::String     tempPath;

        i = 0;
        while ( dest[i] )
        {
          tempPath += dest[i];

          if ( ( i >= 4 + 3 )       // Sonst gibt´s Probleme mit dem Laufwerk
          &&   ( ( Path::IsSeparator( dest[i + 1], sepChars ) )
          ||     ( dest[i + 1] == '\"' )
          ||     ( i + 1 == dest.length() ) ) )
          {
            // found another sub dir
            if ( ( tempPath.length() )
            &&   ( ( tempPath[0] == '\"' )
            ||     ( Path::IsSeparator( tempPath[4], sepChars ) ) ) )
            {
              tempPath = tempPath.substr( 1 );
            }
            if ( ( tempPath.length() )
            &&   ( ( tempPath[tempPath.length() - 1] == '\"' )
            ||     ( Path::IsSeparator( tempPath[tempPath.length() - 1], sepChars ) ) ) )
            {
              tempPath = tempPath.substr( 0, tempPath.length() - 1 );
            }

            // substr 4 since temppath already has the extended name
            if ( !DirectoryExists( tempPath.substr( 4 ) ) )
            {
              // Verzeichnis existiert also noch nicht
              if ( !CreateDirectoryW( MakeSafeWinFilename( tempPath.substr( 4 ) ).c_str(), NULL ) )
              {
                // Konnte Verzeichnis nicht anlegen, Hurra
                GR::u32     error = GetLastError();
                if ( error != ERROR_ALREADY_EXISTS )
                {
                  dh::Log( "Win::Util::CreateSubDir  Failed to create directory %s (%x)", tempPath.c_str(), GetLastError() );

                  // BAUSTELLE
                  // Angelegte Verzeichnisse wieder killen
                  return false;
                }
              }
            }
            else
            {
              // Mitloggen, falls wir abbrechen müssen
            }
          }
          i++;
        }
        return true;
      }



      bool GetFileCreationTime( const GR::String& Name, GR::DateTime::DateTime& CreationTimeUTC )
      {
        SYSTEMTIME                  sysCreationTime;

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        WIN32_FILE_ATTRIBUTE_DATA   attributes;
        GR::WString                 utf16Filename = MakeSafeWinFilename( Name );
        if ( !GetFileAttributesExW( utf16Filename.c_str(), GetFileExInfoStandard, &attributes ) )
        {
          return false;
        }

        if ( !FileTimeToSystemTime( &attributes.ftCreationTime, &sysCreationTime ) )
        {
          return false;
        }
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SYSTEM == OS_WINDOWS )
        GR::IO::FileStream    ioFile;

        if ( !ioFile.Open( Name.c_str(), IIOStream::OT_READ_ONLY_SHARED ) )
        {
          return false;
        }
        FILETIME    creationTime;

        if ( !::GetFileTime( ioFile.GetHandle(), &creationTime, NULL, NULL ) )
        {
          return false;
        }
        ioFile.Close();
        if ( !FileTimeToSystemTime( &creationTime, &sysCreationTime ) )
        {
          return false;
        }
#elif ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN )
        short fileHandle = 0;
        short lastError = FILE_OPEN_( Name.c_str(), (short)Name.length(), &fileHandle, 1, 0 );
        if (_status_lt( lastError ))
        {
          return false;
        }


        ByteBuffer    ItemList;
        ByteBuffer    ValueList;

        // Creation time
        ItemList.AppendU16NetworkOrder( 54 );
        ValueList.Resize( 8 );

        GR::u16   returnedBytes = 0;
        short     errorItem = -1;

        _cc_status status = FILE_GETINFOLIST_( fileHandle,
                                               (short*)ItemList.Data(), (short)(ItemList.Size() / 2),
                                               (short*)ValueList.Data(), (short)ValueList.Size(),
                                               &returnedBytes,
                                               &errorItem );
        FILE_CLOSE_( fileHandle );
        if (_status_lt( status ))
        {
          return false;
        }
        GR::u64     julianTimestamp = ValueList.U64NetworkOrderAt( 0 );
        ByteBuffer  gregorian( 8 * sizeof( short ) );

        // 8 16bit values
        // DATE ^ AND ^ TIME[0] !Gregorian year such as 1990 (1 - 4000)
        // DATE ^ AND ^ TIME[1] !Gregorian month( 1 - 12 )
        // DATE ^ AND ^ TIME[2] !Gregorian day of the month( 1 - 31 )
        // DATE ^ AND ^ TIME[3] !Hour of the day( 0 - 23 )
        // DATE ^ AND ^ TIME[4] !Minute of the hour( 0 - 59 )
        // DATE ^ AND ^ TIME[5] !Second of the minute( 0 - 59 )
        // DATE ^ AND ^ TIME[6] !Millisecond of the second( 0 - 999 )
        // DATE ^ AND ^ TIME[7] !Microsecond of the millisecond( 0 - 999 )

        status = INTERPRETTIMESTAMP( julianTimestamp, (short*)gregorian.Data() );
        if (gregorian.U16At( 0 ) == (GR::u16)-1)
        {
          return false;
        }

        std::tm   timeStamp;

        timeStamp.tm_mday = gregorian.U16NetworkOrderAt( 4 );
        //timeStamp.tm_wday   = DayO sysTime.wDayOfWeek;
        timeStamp.tm_hour = gregorian.U16NetworkOrderAt( 6 );
        timeStamp.tm_min = gregorian.U16NetworkOrderAt( 8 );
        timeStamp.tm_mon = gregorian.U16NetworkOrderAt( 2 ) - 1;
        timeStamp.tm_sec = gregorian.U16NetworkOrderAt( 10 );
        timeStamp.tm_year = gregorian.U16NetworkOrderAt( 0 ) - 1900;
        timeStamp.tm_isdst = -1; // "not known"

        // fill in day of year
        mktime( &timeStamp );
        ModificationTimeUTC.SetTime( timeStamp, gregorian.U16NetworkOrderAt( 12 ) * 1000 + gregorian.U16NetworkOrderAt( 14 ) );

        return true;
#endif

        // SYSTEMTIME to tm
        if ( sysCreationTime.wYear < 1900 )
        {
          // can't represent as struct tm
          return false;
        }

        std::tm   timeStamp;

        timeStamp.tm_mday   = sysCreationTime.wDay;
        timeStamp.tm_wday   = sysCreationTime.wDayOfWeek;
        timeStamp.tm_hour   = sysCreationTime.wHour;
        timeStamp.tm_min    = sysCreationTime.wMinute;
        timeStamp.tm_mon    = sysCreationTime.wMonth - 1;
        timeStamp.tm_sec    = sysCreationTime.wSecond;
        timeStamp.tm_year   = sysCreationTime.wYear - 1900;
        timeStamp.tm_isdst  = -1; // "not known"

        // fill in day of year
        mktime( &timeStamp );
        CreationTimeUTC.SetTime( timeStamp, sysCreationTime.wMilliseconds * 1000 );
        return true;
      }



#endif

      bool GetFileModificationTime( const GR::String& Name, GR::DateTime::DateTime& ModificationTimeUTC )
      {
#if OPERATING_SYSTEM == OS_WINDOWS
#if OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE
        SYSTEMTIME                  sysTime;
        WIN32_FILE_ATTRIBUTE_DATA   attributes;
        GR::WString                 utf16Filename = MakeSafeWinFilename( Name );
        if ( !GetFileAttributesExW( utf16Filename.c_str(), GetFileExInfoStandard, &attributes ) )
        {
          return false;
        }

        if ( !FileTimeToSystemTime( &attributes.ftLastWriteTime, &sysTime ) )
        {
          return false;
        }
#else
        GR::IO::FileStream    ioFile;

        if ( !ioFile.Open( Name.c_str(), IIOStream::OT_READ_ONLY_SHARED ) )
        {
          return false;
        }
        FILETIME    lastAccessTime;

        if ( !GetFileTime( ioFile.GetHandle(), NULL, NULL, &lastAccessTime ) )
        {
          return false;
        }
        ioFile.Close();
        SYSTEMTIME    sysTime;

        if ( !FileTimeToSystemTime( &lastAccessTime, &sysTime ) )
        {
          return false;
        }
#endif
        // SYSTEMTIME to tm
        if ( sysTime.wYear < 1900 )
        {
          // can't represent as struct tm
          return false;
        }

        std::tm   timeStamp;

        timeStamp.tm_mday   = sysTime.wDay;
        timeStamp.tm_wday   = sysTime.wDayOfWeek;
        timeStamp.tm_hour   = sysTime.wHour;
        timeStamp.tm_min    = sysTime.wMinute;
        timeStamp.tm_mon    = sysTime.wMonth - 1;
        timeStamp.tm_sec    = sysTime.wSecond;
        timeStamp.tm_year   = sysTime.wYear - 1900;
        timeStamp.tm_isdst  = -1; // "not known"

        // fill in day of year
        mktime( &timeStamp );
        ModificationTimeUTC.SetTime( timeStamp, sysTime.wMilliseconds * 1000 );
        return true;
#elif ( OPERATING_SYSTEM == OS_TANDEM ) && ( OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN )
        short fileHandle = 0;
        short lastError = FILE_OPEN_( Name.c_str(), (short)Name.length(), &fileHandle, 1, 0 );
        if ( _status_lt( lastError ) )
        {
          return false;
        }


        ByteBuffer    ItemList;
        ByteBuffer    ValueList;

        // Modify time
        ItemList.AppendU16NetworkOrder( 144 );
        ValueList.Resize( 8 );

        GR::u16   returnedBytes = 0;
        short     errorItem = -1;

        _cc_status status = FILE_GETINFOLIST_( fileHandle,
                                               (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                               (short*)ValueList.Data(), (short)ValueList.Size(),
                                               &returnedBytes,
                                               &errorItem );
        FILE_CLOSE_( fileHandle );
        if ( _status_lt( status ) )
        {
          return false;
        }
        GR::u64     julianTimestamp = ValueList.U64NetworkOrderAt( 0 );
        ByteBuffer  gregorian( 8 * sizeof( short ) );

        // 8 16bit values
        // DATE ^ AND ^ TIME[0] !Gregorian year such as 1990 (1 - 4000)
        // DATE ^ AND ^ TIME[1] !Gregorian month( 1 - 12 )
        // DATE ^ AND ^ TIME[2] !Gregorian day of the month( 1 - 31 )
        // DATE ^ AND ^ TIME[3] !Hour of the day( 0 - 23 )
        // DATE ^ AND ^ TIME[4] !Minute of the hour( 0 - 59 )
        // DATE ^ AND ^ TIME[5] !Second of the minute( 0 - 59 )
        // DATE ^ AND ^ TIME[6] !Millisecond of the second( 0 - 999 )
        // DATE ^ AND ^ TIME[7] !Microsecond of the millisecond( 0 - 999 )

        status = INTERPRETTIMESTAMP( julianTimestamp, (short*)gregorian.Data() );
        if ( gregorian.U16At( 0 ) == (GR::u16)-1 )
        {
          return false;
        }

        std::tm   timeStamp;

        timeStamp.tm_mday   = gregorian.U16NetworkOrderAt( 4 );
        //timeStamp.tm_wday   = DayO sysTime.wDayOfWeek;
        timeStamp.tm_hour   = gregorian.U16NetworkOrderAt( 6 );
        timeStamp.tm_min    = gregorian.U16NetworkOrderAt( 8 );
        timeStamp.tm_mon    = gregorian.U16NetworkOrderAt( 2 ) - 1;
        timeStamp.tm_sec    = gregorian.U16NetworkOrderAt( 10 );
        timeStamp.tm_year   = gregorian.U16NetworkOrderAt( 0 ) - 1900;
        timeStamp.tm_isdst  = -1; // "not known"

        // fill in day of year
        mktime( &timeStamp );
        ModificationTimeUTC.SetTime( timeStamp, gregorian.U16NetworkOrderAt( 12 ) * 1000 + gregorian.U16NetworkOrderAt( 14 ) );

        return true;
#else
        // OSS or other unix like
        FILE* fileHandle  = fopen( Name.c_str(), "rb" );
        if ( fileHandle == NULL )
        {
          return false;
        }

        struct stat fStat;

        fstat( fileno( fileHandle ), &fStat );

        return ModificationTimeUTC.FromTime( fStat.st_mtime );
#endif
      }



      GR::String ReadFileAsString( const GR::String& Filename )
      {
        ByteBuffer      bbTemp = ReadFileAsBuffer( Filename );

        GR::String result;

        result.append( (GR::Char*)bbTemp.Data(), bbTemp.Size() );
        return result;
      }



      ByteBuffer ReadFileAsBuffer( const GR::String& Filename )
      {
        GR::IO::FileStream      ioIn;

        if ( !ioIn.Open( Filename, IIOStream::OT_READ_ONLY_SHARED ) )
        {
          return ByteBuffer();
        }

		    // yes, this truncates GR::u64 to GR::u32!
        GR::u32                 FileLength = (GR::u32)ioIn.GetSize();

        ByteBuffer              BBTemp( FileLength );

        ioIn.ReadBlock( BBTemp.Data(), (GR::u32)BBTemp.Size() );
        ioIn.Close();

        return BBTemp;
      }



      bool WriteFileFromString( const GR::String& Filename, const GR::String& Text )
      {
        GR::IO::FileStream      ioOut;

        if ( !ioOut.Open( Filename, IIOStream::OT_WRITE_ONLY ) )
        {
          return false;
        }

        size_t BytesWritten = ioOut.WriteBlock( Text.data(), (GR::up)Text.length() );
        ioOut.Close();

        return BytesWritten == Text.length();
      }



      bool WriteFileFromBuffer( const GR::String& Filename, const ByteBuffer& Buffer )
      {
        GR::IO::FileStream      ioOut;

        if ( !ioOut.Open( Filename, IIOStream::OT_WRITE_ONLY ) )
        {
          return false;
        }

        size_t BytesWritten = ioOut.WriteBlock( Buffer.Data(), (GR::up)Buffer.Size() );
        ioOut.Close();

        return BytesWritten == Buffer.Size();
      }



      bool AppendFileFromString( const GR::String& Filename, const GR::String& Text )
      {
        GR::IO::FileStream      ioOut;

        if ( !ioOut.Open( Filename, IIOStream::OT_WRITE_APPEND ) )
        {
          return false;
        }

        size_t BytesWritten = ioOut.WriteBlock( Text.data(), (GR::up)Text.length() );
        ioOut.Flush();
        ioOut.Close();

        return BytesWritten == Text.length();
      }



      bool AppendFileFromBuffer( const GR::String& Filename, const ByteBuffer& Buffer )
      {
        GR::IO::FileStream      ioOut;

        if ( !ioOut.Open( Filename, IIOStream::OT_WRITE_APPEND ) )
        {
          return false;
        }

        size_t BytesWritten = ioOut.WriteBlock( Buffer.Data(), (GR::up)Buffer.Size() );
        ioOut.Flush();
        ioOut.Close();

        return BytesWritten == Buffer.Size();
      }



      bool MatchFileMask( const char* Mask, const char* Filename )
      {
        const char* pGlob = Mask;
        const char* pPath = Filename;

        int pGlobInc = 0;
        int pPathInc = 0;

        int mp = 0;
        int cp = 0;

        while ( ( *( pPath + pPathInc ) != 0 ) && ( *( pGlob + pGlobInc ) != '*' ) )
        {
          if ( ( *( pGlob + pGlobInc ) != *( pPath + pPathInc ) ) && ( *( pGlob + pGlobInc ) != '?' ) )
          {
            return false;
          }
          pGlobInc++;
          pPathInc++;
        }

        while ( *( pPath + pPathInc ) != 0 )
        {
          if ( *( pGlob + pGlobInc ) == '*' )
          {
            if ( 0 == *( pGlob + ++pGlobInc ) )
            {
              return true;
            }
            mp = pGlobInc;
            cp = pPathInc + 1;
          }
          else if ( ( *( pGlob + pGlobInc ) == *( pPath + pPathInc ) ) || ( *( pGlob + pGlobInc ) == '?' ) )
          {
            pGlobInc++;
            pPathInc++;
          }
          else
          {
            pGlobInc = mp;
            pPathInc = cp++;
          }
        }

        while ( *( pGlob + pGlobInc ) == '*' )
        {
          pGlobInc++;
        }
        return ( 0 == *( pGlob + pGlobInc ) );
      }



      bool GetSize( const GR::String& Filename, GR::u64& FileSize )
      {
        GR::IO::FileStream      ioTemp;

        FileSize = 0;
        if ( !ioTemp.Open( Filename.c_str(), IIOStream::OT_READ_ONLY_SHARED ) )
        {
          return false;
        }
#if OPERATING_SYSTEM == OS_WINDOWS
#if ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        FILE_STANDARD_INFO      fileInfo;

        if ( !GetFileInformationByHandleEx( ioTemp.GetHandle(), FileStandardInfo, &fileInfo, sizeof( fileInfo ) ) )
        {
          return false;
        }
        FileSize = fileInfo.EndOfFile.QuadPart;
#else
        LARGE_INTEGER     largeInt = { 0 };
        if ( !GetFileSizeEx( ioTemp.GetHandle(), &largeInt ) )
        {
          ioTemp.Close();
          return false;
        }
        FileSize = (GR::u64)largeInt.QuadPart;
#endif
#elif OPERATING_SYSTEM == OS_ANDROID
        dh::Log( "FileUtil::GetSize not supported yet" );
        return 0;
#elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
        ByteBuffer    ItemList;
        ByteBuffer    ValueList;

        // File type
        ItemList.AppendU16NetworkOrder( 191 );
        ValueList.Resize( 8 );

        unsigned short    returnedBytes = 0;
        short             errorItem = -1;

        _cc_status Status = FILE_GETINFOLIST_( ioTemp.GetHandle(),
                                              (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                              (short*)ValueList.Data(), (short)ValueList.Size(),
                                              &returnedBytes,
                                              &errorItem );
        if ( _status_lt( Status ) )
        {
          ioTemp.Close();
          return false;
        }
        FileSize = ValueList.U64NetworkOrderAt( 0 );
#else
        ftell( ioTemp.GetHandle() );
        if ( fseek( ioTemp.GetHandle(), 0, SEEK_END ) == -1 )
        {
          ioTemp.Close();
          return false;
        }
        FileSize = ftell( ioTemp.GetHandle() );
#endif
        ioTemp.Close();

        return true;
      }



      GR::String AppPath()
      {
#if ( ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) ) && ( OPERATING_SYSTEM == OS_WINDOWS )
        GR::WChar            tempBuffer[65536];
        ::GetModuleFileNameW( NULL, tempBuffer, 65536 );

        GR::String    appPath = Path::RemoveFileSpec( GR::Convert::ToUTF8( tempBuffer ) );

#ifndef GR_APP_PATH_UNTOUCHED
        GR::String  upPath( appPath );
        upPath.ToUpper();

        if ( ( upPath.EndsWith( "RELEASE" ) )
        ||   ( upPath.EndsWith( "DEBUG" ) ) )
        {
          appPath = Path::TraverseUp( appPath );
        }
#endif // GR_APP_PATH_UNTOUCHED
        return appPath;
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        using namespace Windows::Storage;

        StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

        Platform::String^   appPathMgd = appFolder->Path;

        return GR::Convert::ToUTF8( appPathMgd->Data() );
#elif OPERATING_SYSTEM == OS_ANDROID
        // TODO ?
        GR::String    appPath;// = "/data/data/" + AppPath;

        return appPath;
#elif ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_WEB )
        // TODO!
        GR::String    appPath;

        return appPath;
#elif ( OPERATING_SYSTEM == OS_LINUX )
        // TODO!
        GR::String    appPath;

        return appPath;
#else
        return NOT SUPPORTED
#endif
      }



      GR::String AppDataPath()
      {
#if ( ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) ) && ( OPERATING_SYSTEM == OS_WINDOWS )
        TCHAR szPath[32770];

        if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath ) ) )
        {
          return GR::Convert::ToUTF8( szPath );
        }

        return GR::String();
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        using namespace Windows::Storage;

        StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
        StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

        Platform::String^   fullPath = localFolder->Path;
        Platform::String^   appPathMgd = appFolder->Path;

        GR::String    appDataPath = GR::Convert::ToUTF8( appPathMgd->Data() );

        return appDataPath;
#elif OPERATING_SYSTEM == OS_ANDROID
        // TODO ?
        GR::String    appPath;// = "/data/data/" + AppPath;

        return appPath;
#elif ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_WEB )
        // TODO!
        GR::String    appPath;

        return appPath;
#elif ( OPERATING_SYSTEM == OS_LINUX )
        // TODO!
        GR::String    appPath;

        return appPath;
#else
        return NOT SUPPORTED
#endif
      }



      GR::String UserAppDataPath()
      {
#if ( ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) ) && ( OPERATING_SYSTEM == OS_WINDOWS )
        TCHAR szPath[32770];

        if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, szPath ) ) )
        {
          return GR::Convert::ToUTF8( szPath );
        }

        return GR::String();
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        using namespace Windows::Storage;

        StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
        StorageFolder^ appFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;

        Platform::String^   fullPath = localFolder->Path;
        Platform::String^   appPathMgd = appFolder->Path;

        GR::String    localFolderPath = GR::Convert::ToUTF8( fullPath->Data() );

        return localFolderPath;
#elif OPERATING_SYSTEM == OS_ANDROID
        // TODO ?
        GR::String    localFolderPath;// = "/data/data/" + AppPath;
        GR::String    appPath;// = "/data/data/" + AppPath;

        return localFolderPath;
#elif ( OPERATING_SYSTEM == OS_TANDEM ) || ( OPERATING_SYSTEM == OS_WEB )
        // TODO!
        GR::String    appPath;

        return appPath;
#elif ( OPERATING_SYSTEM == OS_LINUX )
        // TODO!
        GR::String    appPath;

        return appPath;
#else
        return NOT SUPPORTED
#endif
      }


    }
  }
}
