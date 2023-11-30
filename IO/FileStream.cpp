#include <IO/FileStream.h>
#include <IO/FileUtil.h>

#include <String/Convert.h>
#include <String/StringUtil.h>
#include <String/Path.h>



#if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
#include <tal.h>

#pragma nowarn (262)
#endif

#if OPERATING_SYSTEM == OS_ANDROID
#include <Xtreme/Environment/AndroidMain.h>
#endif

#include <debug/debugclient.h>



#if OPERATING_SYSTEM == OS_WINDOWS
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
    GR::String    pathToSet = UTF8Filename;
    if ( Path::IsRelative( pathToSet ) )
    {
      // prepend full path
      GR::String   currentFolder = GR::IO::FileUtil::CurrentWorkingDirectory();
      pathToSet = Path::Append( currentFolder, UTF8Filename );
    }
    GR::WString     result = GR::WString( L"\\\\?\\" ) + GR::Convert::ToUTF16( GR::Strings::Replace( pathToSet, "\\.\\", "\\" ) );

    MakeBackSlashes( result );
    return result;
  }

}
#endif



namespace GR
{
  namespace IO
  {

    #if OPERATING_SUB_SYSTEM == OS_SUB_SDL
    FILE* NULL_HANDLE = NULL;
    #elif OPERATING_SYSTEM == OS_WINDOWS
    HANDLE NULL_HANDLE = INVALID_HANDLE_VALUE;
    #elif OPERATING_SYSTEM == OS_ANDROID
    AAsset*  NULL_HANDLE = NULL;
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    short NULL_HANDLE = -1;
    #else
    FILE* NULL_HANDLE = NULL;
    #endif



    FileStream::FileStream() :
      IIOStreamBase(),
      m_LastError( 0 ),
    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      m_CacheSize( 256 ),
    #else
      m_CacheSize( 256 ),
    #endif
      m_Impl( new FileStreamImpl( NULL_HANDLE ) )
    {

    }



    FileStream::FileStream( const GR::Char* pFileName, IIOStream::OpenType oType ) :
      IIOStreamBase(),
      m_LastError( 0 ),
    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      m_CacheSize( 0 ),
    #else
      m_CacheSize( 256 ),
    #endif
      m_Impl( new FileStreamImpl( NULL_HANDLE ) )
    {
      Open( pFileName, oType );
    }



    FileStream::FileStream( const GR::String& FileName, IIOStream::OpenType oType ) :
      IIOStreamBase(),
      m_LastError( 0 ),
    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      m_CacheSize( 0 ),
    #else
      m_CacheSize( 256 ),
    #endif
      m_Impl( new FileStreamImpl( NULL_HANDLE ) )
    {
      Open( FileName, oType );
    }



    void FileStream::Close()
    {
      if ( m_Impl )
      {
        m_Impl->Close();
      }

      IIOStreamBase::Close();

      m_OpenType  = OT_CLOSED;
    }



    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    bool FileStream::OpenTandem( IIOStream::OpenType OType,
                                 const char* Filename,
                                 short FileCode,
                                 short PrimaryExtentSize,
                                 short SecondaryExtentSize,
                                 short MaximumExtents,
                                 short FileType,
                                 short Options,
                                 short RecordLen,
                                 short BlockLen,
                                 short KeyLen,
                                 short KeyOffset,
                                 bool Buffered )
    {
      m_LastError = 0;

      if ( ( m_Impl )
      &&   ( m_Impl->m_Handle != NULL_HANDLE ) )
      {
        return false;
      }

      m_OpenType = OType;

      if ( m_OpenType == OT_WRITE_ONLY )
      {
        // delete file if it exists, this doesn't seem to happen automatically with Tandem
        m_LastError = FILE_PURGE_( Filename, (short)strlen( Filename ) );
      }

      char    NewFilename[1024];
      strcpy( NewFilename, Filename );
      short   FilenameLen = (short)strlen( Filename );
      memcpy( NewFilename, Filename, FilenameLen + 1 );

      ByteBuffer    ItemList;
      ByteBuffer    ValueList;

      // File type
      ItemList.AppendU16NetworkOrder( 41 );
      ValueList.AppendU16NetworkOrder( FileType );
      // File Code
      ItemList.AppendU16NetworkOrder( 42 );
      ValueList.AppendU16NetworkOrder( FileCode );
      // Record length
      ItemList.AppendU16NetworkOrder( 43 );
      ValueList.AppendU16NetworkOrder( RecordLen );
      // Block length
      ItemList.AppendU16NetworkOrder( 44 );
      ValueList.AppendU16NetworkOrder( BlockLen );
      // key offset
      ItemList.AppendU16NetworkOrder( 45 );
      ValueList.AppendU16NetworkOrder( KeyOffset );
      // key length
      ItemList.AppendU16NetworkOrder( 46 );
      ValueList.AppendU16NetworkOrder( KeyLen );
      // primary extent size
      ItemList.AppendU16NetworkOrder( 50 );
      ValueList.AppendU16NetworkOrder( PrimaryExtentSize );
      // secondary extent size
      ItemList.AppendU16NetworkOrder( 51 );
      ValueList.AppendU16NetworkOrder( SecondaryExtentSize );
      // maximum extents
      ItemList.AppendU16NetworkOrder( 52 );
      ValueList.AppendU16NetworkOrder( MaximumExtents );
      if ( FileType == 0 )
      {
        // odd filebyte, only allowed for unstructured files
        ItemList.AppendU16NetworkOrder( 65 );
        ValueList.AppendU16NetworkOrder( 1 );
      }
      // create options
      if ( Options != 0 )
      {
        ItemList.AppendU16NetworkOrder( 71 );
        ValueList.AppendU16NetworkOrder( Options );
      }
      if ( Buffered )
      {
        // write through
        ItemList.AppendU16NetworkOrder( 72 );
        ValueList.AppendU16NetworkOrder( 0 );
      }

      _cc_status Status = FILE_CREATELIST_( NewFilename, FilenameLen, &FilenameLen,
                                            (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                            (short*)ValueList.Data(), (short)ValueList.Size() );
      if ( _status_lt( Status ) )
      {
        m_LastError = (short)Status;
        //printf( "FILE_CREATELIST_ failed %d\n", m_LastError );
        return false;
      }

      short hHandle = NULL_HANDLE;
      m_LastError = 0;
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          m_LastError = FILE_OPEN_( Filename,
                  (short)strlen( Filename ),
                  &hHandle,
                  1,
                  1 );
          /*
              { const char *filename |
              const char *pathname }
              ,short length
              ,short *filenum
              ,[ short access ]
              ,[ short exclusion ]
              ,[ short nowait-depth ]
              ,[ short sync-or-receive-depth ]
              ,[ short options ]
              ,[ short seq-block-buffer-id ]
              ,[ short seq-block-buffer-len ]
              ,[ short *primary-processhandle ]
              ,[ __int32_t elections ] );
              "rb" );
              */
          break;
        case OT_READ_ONLY_SHARED:
          m_LastError    = FILE_OPEN_( Filename,
                               (short)strlen( Filename ),
                               &hHandle,
                               1,
                               0 );
          break;
        case OT_WRITE_ONLY:
          m_LastError    = FILE_OPEN_( Filename,
                                   (short)strlen( Filename ),
                                   &hHandle,
                                   2 );
          break;
        case OT_WRITE_APPEND:
          m_LastError    = FILE_OPEN_( Filename,
                                   (short)strlen( Filename ),
                                   &hHandle,
                                   2 );
          break;
        case OT_READ_WRITE:
          m_LastError    = FILE_OPEN_( Filename,
                               (short)strlen( Filename ),
                               &hHandle,
                               0 );
          break;
      }

      if ( _status_lt( m_LastError ) )
      {
        Close();
        return false;
      }

      if ( hHandle != NULL_HANDLE )
      {
        m_Impl = new FileStreamImpl( hHandle );
        m_Opened = true;

        if ( ( GetFileCode() == 101 )
        &&   ( GetFileType() == 0 ) )
        {
          // reopen as edit file (by passing the previous unclosed handle into OPENEDIT!)
          switch ( m_OpenType )
          {
            case OT_READ_ONLY:
              m_LastError    = OPENEDIT_( Filename,
                                          (short)strlen( Filename ),
                                          &hHandle,
                                          1,
                                          1 );
              break;
            case OT_READ_ONLY_SHARED:
              m_LastError    = OPENEDIT_( Filename,
                                          (short)strlen( Filename ),
                                          &hHandle,
                                          1,
                                          0 );
              break;
            case OT_WRITE_ONLY:
            case OT_WRITE_APPEND:
              m_LastError    = OPENEDIT_( Filename,
                                          (short)strlen( Filename ),
                                          &hHandle,
                                          2 );
              break;
          }
          if ( _status_lt( m_LastError ) )
          {
            Close();
            return false;
          }
          m_Impl->m_EditFileOpened  = true;
          m_OpenType                = OType;
          m_Opened                  = true;
        }

        if ( m_OpenType == OT_WRITE_APPEND )
        {
          m_LastError = FILE_SETPOSITION_( hHandle, -1 );
        }
      }
      return ( hHandle != NULL_HANDLE );
    }
    #endif



    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    short FileStream::GetFileError() const
    {

      if ( ( m_Impl )
      &&   ( m_Impl->m_Handle == NULL_HANDLE ) )
      {
        return -1;
      }

      short   LastError = 0;

      _cc_status Status = FILE_GETINFO_( m_Impl->m_Handle, &LastError );
      if ( _status_lt( Status ) )
      {
        //m_LastError = (short)Status;
        return -2;
      }
      return LastError;

    }
    #endif



    bool FileStream::Open( const GR::Char* FileName, OpenType Type )
    {
      return Open( GR::String( FileName ), Type );
    }



    bool FileStream::Open( const GR::String& FileName, OpenType Type )
    {
      if ( ( m_Impl )
      &&   ( m_Impl->m_Handle != NULL_HANDLE ) )
      {
        return false;
      }

      m_OpenType  = Type;

#if ( ( OPERATING_SYSTEM == OS_WINDOWS ) && ( ( OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_SDL ) ) )

      HANDLE    hHandle = NULL_HANDLE;
      GR::WString     utf16FileName = MakeSafeWinFilename( FileName );
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          hHandle = CreateFileW( utf16FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
          break;
        case OT_READ_ONLY_SHARED:
          hHandle = CreateFileW( utf16FileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
          break;
        case OT_WRITE_ONLY:
          hHandle = CreateFileW( utf16FileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
          break;
        case OT_WRITE_APPEND:
          hHandle = CreateFileW( utf16FileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
          break;
        case OT_READ_WRITE:
          hHandle = CreateFileW( utf16FileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
          break;
      }
#elif ( OPERATING_SYSTEM == OS_ANDROID )
      AAssetManager* pManager = AndroidMediator::s_pActivity->assetManager;
      AAsset*   hHandle = NULL;
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          hHandle = AAssetManager_open( pManager, FileName.c_str(), AASSET_MODE_BUFFER );
          //hHandle    = fopen( FileName.c_str(), "rb" );
          break;
        case OT_READ_ONLY_SHARED:
          hHandle = AAssetManager_open( pManager, FileName.c_str(), AASSET_MODE_BUFFER );
          //hHandle    = fopen( FileName.c_str(), "rb" );
          break;
        case OT_WRITE_ONLY:
          //hHandle    = fopen( FileName.c_str(), "wb" );
          break;
        case OT_WRITE_APPEND:
          //hHandle    = fopen( FileName.c_str(), "ab" );
          break;
        case OT_READ_WRITE:
          //hHandle    = fopen( FileName.c_str(), "r+b" );
          break;
      }
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      HANDLE    hHandle = NULL_HANDLE;
      GR::WString     utf16FileName = MakeSafeWinFilename( FileName );
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          hHandle = CreateFile2( utf16FileName.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, NULL );
          break;
        case OT_READ_ONLY_SHARED:
          hHandle = CreateFile2( utf16FileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, NULL );
          break;
        case OT_WRITE_ONLY:
          hHandle = CreateFile2( utf16FileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, NULL );
          break;
        case OT_WRITE_APPEND:
          hHandle = CreateFile2( utf16FileName.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_ALWAYS, NULL );
          break;
        case OT_READ_WRITE:
          hHandle = CreateFile2( utf16FileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING, NULL );
          break;
      }

#elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      short hHandle = NULL_HANDLE;
      m_LastError = 0;
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          m_LastError = FILE_OPEN_( FileName.c_str(), (short)FileName.length(), &hHandle, 1, 1 );
          break;
        case OT_READ_ONLY_SHARED:
          m_LastError    = FILE_OPEN_( FileName.c_str(), (short)FileName.length(), &hHandle, 1, 0 );
          break;
        case OT_WRITE_ONLY:
          return OpenTandem( OT_WRITE_ONLY,
                             FileName.c_str(),
                             0,
                             2000,
                             2000,
                             500,
                             0,
                             0,   // options
                             0,
                             0,
                             0,
                             0,
                             false );
        case OT_WRITE_APPEND:
          m_LastError = FILE_OPEN_( FileName.c_str(), (short)FileName.length(), &hHandle, 2 );
          if ( m_LastError == 11 )
          {
            // file not found, create new
            return OpenTandem( OT_WRITE_ONLY,
                               FileName.c_str(),
                               0,
                               2000,
                               2000,
                               500,
                               0,
                               0,   // options
                               0,
                               0,
                               0,
                               0,
                               false );
          }
          m_LastError = FILE_SETPOSITION_( hHandle, -1 );
          break;
        case OT_READ_WRITE:
          m_LastError = FILE_OPEN_( FileName.c_str(), (short)FileName.length(), &hHandle, 0 );
          break;
      }
      if ( hHandle == NULL_HANDLE )
      {
        Close();
        return false;
      }
    #else
      m_LastError = 0;
      FILE*   hHandle = NULL_HANDLE;
      switch ( m_OpenType )
      {
        case OT_READ_ONLY:
          hHandle    = fopen( FileName.c_str(), "rb" );
          break;
        case OT_READ_ONLY_SHARED:
          hHandle    = fopen( FileName.c_str(), "rb" );
          break;
        case OT_WRITE_ONLY:
          hHandle    = fopen( FileName.c_str(), "wb" );
          break;
        case OT_WRITE_APPEND:
          hHandle    = fopen( FileName.c_str(), "ab" );
          break;
        case OT_READ_WRITE:
          hHandle    = fopen( FileName.c_str(), "r+b" );
          break;
        case OT_CLOSED:
          break;
      }
    #endif


      if ( hHandle != NULL_HANDLE )
      {
        m_Impl = new FileStreamImpl( hHandle );
        m_Opened = true;

#if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
        if ( ( GetFileCode() == 101 )
        &&   ( GetFileType() == 0 ) )
        {
          // reopen as edit file (by passing the previous unclosed handle into OPENEDIT!)
          switch ( Type )
          {
            case OT_READ_ONLY:
              m_LastError    = OPENEDIT_( FileName.c_str(),
                                          (short)FileName.length(),
                                          &hHandle,
                                          1,
                                          1 );
              break;
            case OT_READ_ONLY_SHARED:
              m_LastError    = OPENEDIT_( FileName.c_str(),
                                          (short)FileName.length(),
                                          &hHandle,
                                          1,
                                          0 );
              break;
            case OT_WRITE_ONLY:
            case OT_WRITE_APPEND:
              m_LastError    = OPENEDIT_( FileName.c_str(),
                                          (short)FileName.length(),
                                          &hHandle,
                                          2 );
              break;
          }
          if ( _status_lt( m_LastError ) )
          {
            Close();
            return false;
          }
          m_Impl->m_EditFileOpened  = true;
          m_OpenType                = Type;
          m_Opened                  = true;
        }
        if ( m_OpenType == OT_WRITE_APPEND )
        {
          m_LastError = FILE_SETPOSITION_( hHandle, -1 );
        }
#endif
      }
    #if OPERATING_SYSTEM == OS_WINDOWS
      else
      {
        m_LastError = GetLastError();
      }
    #else 
      else
      {
        m_LastError = errno;
        //dh::Log( "File Open failed (%d,%s) for (%s)", m_LastError, strerror( m_LastError ), FileName.c_str() );
      }
    #endif

      return ( hHandle != NULL_HANDLE );
    }



    #if OPERATING_SYSTEM == OS_WINDOWS
    HANDLE FileStream::GetHandle()
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    short FileStream::GetHandle()
    #elif OPERATING_SYSTEM == OS_ANDROID
    AAsset* FileStream::GetHandle()
    #else
    FILE* FileStream::GetHandle()
    #endif
    {

      return m_Impl->m_Handle;

    }



    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    short FileStream::GetFileCode()
    {

      ByteBuffer    ItemList;
      ByteBuffer    ValueList;

      // File Code
      ItemList.AppendU16NetworkOrder( 42 );
      ValueList.Resize( 2 );

      unsigned short    returnedBytes = 0;
      short             errorItem = -1;

      _cc_status Status = FILE_GETINFOLIST_( m_Impl->m_Handle,
                                            (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                            (short*)ValueList.Data(), (short)ValueList.Size(),
                                            &returnedBytes,
                                            &errorItem );
      if ( _status_lt( Status ) )
      {
        m_LastError = (short)Status;
        //printf( "FILE_GETINFOLIST_ failed %d\n", m_LastError );
        return false;
      }
      return (GR::i16)ValueList.U16NetworkOrderAt( 0 );

    }



    short FileStream::GetFileType()
    {

      ByteBuffer    ItemList;
      ByteBuffer    ValueList;

      // File Type
      ItemList.AppendU16NetworkOrder( 41 );
      ValueList.Resize( 2 );

      unsigned short    returnedBytes = 0;
      short             errorItem = -1;

      _cc_status Status = FILE_GETINFOLIST_( m_Impl->m_Handle,
                                            (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                            (short*)ValueList.Data(), (short)ValueList.Size(),
                                            &returnedBytes,
                                            &errorItem );
      if ( _status_lt( Status ) )
      {
        m_LastError = (short)Status;
        //printf( "FILE_GETINFOLIST_ failed %d\n", m_LastError );
        return false;
      }
      return (GR::i16)ValueList.U16At( 0 );

    }
    #endif



    GR::u64 FileStream::GetSize()
    {
#if OPERATING_SYSTEM == OS_WINDOWS

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
      FILE_STANDARD_INFO      fileInfo;

      if ( !GetFileInformationByHandleEx( m_Impl->m_Handle, FileStandardInfo, &fileInfo, sizeof( fileInfo ) ) )
      {
        m_LastError = GetLastError();
        return 0;
      }
      m_Impl->m_CachedFileSize = fileInfo.EndOfFile.QuadPart;
#else 
      m_Impl->m_CachedFileSize = GetFileSize( m_Impl->m_Handle, NULL );
#endif
    #elif OPERATING_SYSTEM == OS_ANDROID
      m_Impl->m_CachedFileSize = AAsset_getLength64( m_Impl->m_Handle );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN

      ByteBuffer    ItemList;
      ByteBuffer    ValueList;

      // File type
      ItemList.AppendU16NetworkOrder( 191 );
      ValueList.Resize( 8 );

      unsigned short    returnedBytes = 0;
      short             errorItem = -1;

      _cc_status Status = FILE_GETINFOLIST_( m_Impl->m_Handle,
                                            (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                            (short*)ValueList.Data(), (short)ValueList.Size(),
                                            &returnedBytes,
                                            &errorItem );
      if ( _status_lt( Status ) )
      {
        m_LastError = (short)Status;
        //printf( "FILE_GETINFOLIST_ failed %d\n", m_LastError );
        return false;
      }
      m_Impl->m_CachedFileSize = ValueList.U64NetworkOrderAt( 0 );
    #else
      int   CurPos = ftell( m_Impl->m_Handle );
      if ( fseek( m_Impl->m_Handle, 0, SEEK_END ) == -1 )
      {
        return (GR::u64)-1;
      }
      GR::u64     Size = ftell( m_Impl->m_Handle );
      if ( fseek( m_Impl->m_Handle, CurPos, SEEK_SET ) == -1 )
      {
        return (GR::u64)-2;
      }
      m_Impl->m_CachedFileSize = Size;
    #endif

      return m_Impl->m_CachedFileSize;
    }



    bool FileStream::ReadToBuffer( void* pTarget, unsigned long BytesToRead, unsigned long& BytesRead )
    {
    #if OPERATING_SYSTEM == OS_WINDOWS
      if ( !ReadFile( m_Impl->m_Handle, pTarget, (GR::u32)BytesToRead, &BytesRead, NULL ) )
      {
        //dh::Log( "ReadFile failed (Wanted % bytes, got %d bytes", BytesToRead, BytesRead );
        m_LastError = GetLastError();
        return false;
      }
      return BytesToRead == BytesRead;
    #elif OPERATING_SYSTEM == OS_ANDROID
      int bytesRead = AAsset_read( m_Impl->m_Handle, pTarget, BytesToRead );
      if ( bytesRead < 0 )
      {
        return false;
      }
      BytesRead = bytesRead;
      return ( BytesRead > 0 );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      if ( m_Impl->m_EditFileOpened )
      {
        short NewBytesRead = 0;
        unsigned short BytesAlreadyRead = 0;
        unsigned short BytesToReadNow = 0;

        while ( BytesAlreadyRead < BytesToRead )
        {
          if ( m_Impl->m_LFNeedsToBeAdded )
          {
            m_Impl->m_LFNeedsToBeAdded = false;

            *( ( (char*)pTarget ) + BytesAlreadyRead ) = 10;
            BytesAlreadyRead++;
            BytesRead = BytesAlreadyRead;
          }
          NewBytesRead = 0;
          _cc_status Status = READEDIT( m_Impl->m_Handle, , ( (char*)pTarget ) + BytesAlreadyRead, (short)( BytesToRead - BytesAlreadyRead ), &NewBytesRead );
          if ( _status_lt( Status ) )
          {
            // Fehler
            return false;
          }
          if ( NewBytesRead < 0 )
          {
            return true;
          }
          if ( Status == 1 )
          {
            // EOF reached
            return true;
          }
          /*
          if ( NewBytesRead == 0 )
          {
            printf( "-failed, no bytes read\n" );
            return false;
          }*/
          BytesAlreadyRead += NewBytesRead;
          if ( BytesAlreadyRead < BytesToRead )
          {
            *( ( (char*)pTarget ) + BytesAlreadyRead ) = 10;
            BytesAlreadyRead++;
          }
          else
          {
            m_Impl->m_LFNeedsToBeAdded = true;
          }
          BytesRead = BytesAlreadyRead;
        }
        return true;
      }
      unsigned long  BytesToReadTemp = BytesToRead;
      unsigned short BytesToReadNow = 0;
      bool           Fehler = false;
      unsigned short NewBytesRead = 0;
      unsigned long  ulNewBytesRead = 0;
      while ( BytesToReadTemp > 0 )
      {
        if ( BytesToReadTemp > 57344 )
        {
          BytesToReadNow = 57344;
        }
        else
        {
          BytesToReadNow = (short)BytesToReadTemp;
        }
        //printf( "Read %d Bytes now\n", BytesToReadNow );
        _cc_status Status = READX( m_Impl->m_Handle, ( (char*)pTarget ) + ulNewBytesRead, BytesToReadNow, &NewBytesRead );
        if ( _status_lt( Status ) )
        {
          // Fehler
          Fehler = true;
          //printf( "-failed (%d bytes gotten)\n", NewBytesRead );
          break;
        }
        //printf( "-read %d bytes\n", NewBytesRead );

        ulNewBytesRead += NewBytesRead;
        BytesRead += NewBytesRead;
        BytesToReadTemp -= NewBytesRead;
        if ( NewBytesRead < BytesToReadNow )
        {
          // Fehler
          Fehler = true;
          break;
        }
      }
      if ( Fehler )
      {
        return false;
      }
      //Log( Misc::Format( "Bytes wanted %1% Bytes got %2%" ) << BytesToRead
      return ( ulNewBytesRead == BytesToRead );
    #else
      BytesRead = (unsigned long)fread( pTarget, 1, BytesToRead, m_Impl->m_Handle );
      return ( BytesRead == BytesToRead );
    #endif

    }



    #if OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
    bool FileStream::ReadLine( char* pTarget, unsigned long MaxReadLength )
    {
      return IIOStreamBase::ReadLine( pTarget, MaxReadLength );
    }



    bool FileStream::ReadLine( GR::WString& Result )
    {
      return IIOStreamBase::ReadLine( Result );
    }



    bool FileStream::ReadLine( GR::String& Result )
    {
      if ( m_Impl->m_EditFileOpened )
      {
        static ByteBuffer    temp( 256 );

        GR::i16       bytesRead = 0;

        _cc_status Status = READEDIT( m_Impl->m_Handle, , (char*)temp.Data(), (short)temp.Size(), &bytesRead );
        if ( _status_lt( Status ) )
        {
          // Fehler
          Result.erase();
          return false;
        }
        if ( Status == 1 )
        {
          // EOF reached
          Result.erase();
          return false;
        }
        Result.assign( (char*)temp.Data(), bytesRead );
        return true;
      }
      return IIOStreamBase::ReadLine( Result );
    }
    #endif



    unsigned long FileStream::ReadBlock( void* pDestination, size_t CountBytes )
    {
    #if OPERATING_SYSTEM == OS_WINDOWS
      if ( ( m_Impl == INVALID_HANDLE_VALUE )
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      //if ( ( m_Impl == -1 )
      if ( ( GetHandle() == -1 )
    #else
      if ( ( m_Impl == NULL )
    #endif
      ||   ( !m_Opened )
      ||   ( m_OpenType == OT_WRITE_ONLY )
      ||   ( m_OpenType == OT_WRITE_APPEND ) )
      {
        m_ReadFailed = true;
        return 0;
      }

      /*
      // without cache
      unsigned long     ulBytesRead = 0;

      if ( !ReadFile( m_Impl->m_Handle, pDestination, (GR::u32)ulSize, &ulBytesRead, NULL ) )
      {
        m_ReadFailed = true;
      }
      if ( ulBytesRead < ulSize )
      {
        m_ReadFailed = true;
      }

      return ulBytesRead;
      */

      unsigned long     ulBytesRead = 0;
      size_t            ulBytesToRead = CountBytes;

      m_ReadFailed = false;

      //dh::Log( "%08x>Read %d Bytes (Cached %d Bytes)", m_Impl, ulSize, m_Impl->m_Cache.Size() );
      //printf( "%08x>Read %d Bytes (Cached %d Bytes)\n", m_Impl, ulSize, m_Impl->m_Cache.Size() );

      // read from cache if enough data left
      if ( m_Impl->m_Cache.Size() - m_Impl->m_CacheBytesUsed >= ulBytesToRead )
      {
        memcpy( pDestination, m_Impl->m_Cache.DataAt( (size_t)m_Impl->m_CacheBytesUsed ), ulBytesToRead );
        m_Impl->m_CacheBytesUsed += ulBytesToRead;
        //m_Impl->m_Cache.TruncateFront( ulBytesToRead );

        m_Impl->m_PseudoFilePos += ulBytesToRead;

        //dh::Log( "%08x-from cache (%d bytes in cache left)", m_Impl, m_Impl->m_Cache.Size() );
        //printf( "%08x-from cache (%d bytes in cache left)\n", m_Impl, m_Impl->m_Cache.Size() );
        //dh::Hex( lpucDestination, ulBytesToRead );
        return (unsigned long)ulBytesToRead;
      }

      // read rest of cache
      if ( m_Impl->m_Cache.Size() - m_Impl->m_CacheBytesUsed > 0 )
      {
        // im Cache ist noch was drin
        memcpy( pDestination, m_Impl->m_Cache.DataAt( (size_t)m_Impl->m_CacheBytesUsed ), (size_t)( m_Impl->m_Cache.Size() - m_Impl->m_CacheBytesUsed ) );
        ulBytesRead = (unsigned long)( m_Impl->m_Cache.Size() - m_Impl->m_CacheBytesUsed );
        m_Impl->m_Cache.Clear();
        m_Impl->m_CacheBytesUsed = 0;
        ulBytesToRead -= ulBytesRead;

        m_Impl->m_PseudoFilePos += ulBytesRead;

        //dh::Log( "%08x-from cache2 (%d bytes in cache left)", m_Impl, m_Impl->m_Cache.Size() );
        //printf( "%08x-from cache2 (%d bytes in cache left)\n", m_Impl, m_Impl->m_Cache.Size() );
      }

      // direct read if bigger than cache size
      if ( ulBytesToRead > m_CacheSize )
      {
        // direkt rauslesen
        unsigned long   ulNewBytesRead = 0;

        if ( !ReadToBuffer( ( (char*)pDestination ) + ulBytesRead, (GR::u32)ulBytesToRead, ulNewBytesRead ) )
        {
          //dh::Log( "Read failed" );
          m_ReadFailed = true;
        }
        m_Impl->m_PseudoFilePos     += ulNewBytesRead;
        m_Impl->m_FilePosAfterCache += ulNewBytesRead;
        ulBytesRead += ulNewBytesRead;
        if ( ulBytesRead < CountBytes )
        {
          m_ReadFailed = true;
        }
        return ulBytesRead;
      }

      // refill cache
      m_Impl->m_Cache.Resize( m_CacheSize );
      m_Impl->m_CacheBytesUsed = 0;

      unsigned long   ulCacheBytesRead = 0;

      if ( !ReadToBuffer( m_Impl->m_Cache.Data(), (GR::u32)m_CacheSize, ulCacheBytesRead ) )
      {
        if ( ulCacheBytesRead == 0 )
        {
          //dh::Log( "Read failed 2" );
          m_ReadFailed = true;
        }
      }
      m_Impl->m_FilePosAfterCache += ulCacheBytesRead;
      //dh::Log( "%08x-into cache (wanted %d, got %d)", m_Impl, m_CacheSize, ulCacheBytesRead );
      //printf( "%08x-into cache (wanted %d, got %d)\n", m_Impl, m_CacheSize, ulCacheBytesRead );
      m_Impl->m_Cache.Resize( ulCacheBytesRead );
      if ( ulBytesRead + ulCacheBytesRead < CountBytes )
      {
        m_ReadFailed = true;
      }
      if ( ulBytesToRead > ulCacheBytesRead )
      {
        // wir waren am Ende; der volle Cache wird kopiert
        memcpy( ( (char*)pDestination ) + ulBytesRead, m_Impl->m_Cache.Data(), ulCacheBytesRead );
        m_Impl->m_Cache.Clear();
        m_Impl->m_CacheBytesUsed = 0;

        m_Impl->m_PseudoFilePos += ulCacheBytesRead;

        //dh::Log( "%08x-from cache3 (%d bytes in cache left)", m_Impl, m_Impl->m_Cache.Size() );
        //printf( "%08x-from cache3 (%d bytes in cache left)\n", m_Impl, m_Impl->m_Cache.Size() );
        //dh::Hex( lpucDestination, ulCacheBytesRead + ulBytesRead );
        return ulCacheBytesRead + ulBytesRead;
      }
      memcpy( ( (char*)pDestination ) + ulBytesRead, m_Impl->m_Cache.DataAt( (size_t)m_Impl->m_CacheBytesUsed ), ulBytesToRead );
      m_Impl->m_CacheBytesUsed += ulBytesToRead;
      //m_Impl->m_Cache.TruncateFront( ulBytesToRead );

      m_Impl->m_PseudoFilePos += ulBytesToRead;

      //dh::Log( "%08x-from cache4 (%d bytes in cache left)", m_Impl, m_Impl->m_Cache.Size() );
      //printf( "%08x-from cache4 (%d bytes in cache left)\n", m_Impl, m_Impl->m_Cache.Size() );
      //dh::Hex( lpucDestination, ulBytesToRead + ulBytesRead );
      return (unsigned long)ulBytesToRead + ulBytesRead;
    }



    unsigned long FileStream::WriteBlock( const void* pSource, size_t CountBytes )
    {
      if ( ( !m_Impl.IsObjectValid() )
      ||   ( m_Impl->m_Handle == NULL_HANDLE )
      ||   ( m_OpenType == OT_READ_ONLY ) )
      {
        //printf( "WriteBlock Invalid\n" );
        return 0;
      }

      unsigned long ulBytesWritten = 0;

      if ( !m_Impl->m_Cache.Empty() )
      {
        m_Impl->m_Cache.Clear();
        m_Impl->m_CacheBytesUsed    = 0;
        m_Impl->m_FilePosAfterCache = m_Impl->m_PseudoFilePos;

        SetPosition( m_Impl->m_PseudoFilePos, PT_SET );
      }

    #if OPERATING_SYSTEM == OS_WINDOWS
      WriteFile( m_Impl->m_Handle, pSource, (GR::u32)CountBytes, &ulBytesWritten, NULL );
      m_LastError = GetLastError();
    #elif OPERATING_SYSTEM == OS_ANDROID
      dh::Log( "FileStream::WriteBlock not supported yet" );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      unsigned short BytesToWriteNow = 0;
      unsigned short ulNewBytesWritten = 0;

      //printf( "Want to write %lu bytes\n", ulSize );

      while ( CountBytes > 0 )
      {
        if ( CountBytes > (short)16384 )
        {
          BytesToWriteNow = (short)16384;
        }
        else
        {
          BytesToWriteNow = (short)CountBytes;
        }
        //printf( "Write %d bytes\n", BytesToWriteNow );
        _cc_status Status = WRITEX( m_Impl->m_Handle, (const char*)pSource + ulBytesWritten, BytesToWriteNow, &ulNewBytesWritten );
        if ( ( _status_lt( Status ) )
        ||   ( ulNewBytesWritten < BytesToWriteNow ) )
        {
          // Fehler
          //printf( "Error calling writeX (%d)\n", GetFileError() );
          m_LastError = GetFileError();
          break;
        }
        // darf das so laufen?????
        if ( ulNewBytesWritten > BytesToWriteNow )
        {
          // TANDEM kann ein Byte mehr schreiben!?!
          //printf( "Tandem wrote %u bytes, but we only provided %u!\n", ulNewBytesWritten, BytesToWriteNow );
          ulNewBytesWritten = BytesToWriteNow;

        }
        //printf( "-Bytes written %d\n", ulNewBytesWritten );
        ulBytesWritten  += ulNewBytesWritten;
        CountBytes      -= ulNewBytesWritten;
      }
    #else
      ulBytesWritten = (unsigned long)fwrite( pSource, 1, CountBytes, m_Impl->m_Handle );
    #endif

      m_Impl->m_FilePosAfterCache += ulBytesWritten;
      m_Impl->m_PseudoFilePos     += ulBytesWritten;

      return ulBytesWritten;
    }



    unsigned long FileStream::SetPosition( GR::i64 Offset, PositionType Position )
    {
      if ( m_Impl->m_Handle == NULL_HANDLE )
      {
        return (unsigned long)-1;
      }

      switch ( Position )
      {
        case PT_CURRENT:
          // uhoh, Cache berücksichtigen
          if ( Offset == 0 )
          {
            return (unsigned long)( m_Impl->m_PseudoFilePos );
          }
          /*
          if ( iOffset < 0 )
          {
            m_Impl->m_iFilePos += iOffset;
            iOffset = m_Impl->m_iFilePos;
            m_Impl->m_Cache.Clear();
            pType = PT_SET;
          }
          if ( ( iOffset <= (GR::i64)m_Impl->m_Cache.Size() )
          &&   ( iOffset >= 0 ) )
          {
            // Offset passt in Cache
            m_Impl->m_iFilePos += iOffset - (GR::i64)m_Impl->m_Cache.Size();
            m_Impl->m_Cache.TruncateFront( iOffset );
            return GetPosition();
          }
          else
          {
          }
          */

          /*
          //iOffset -= (GR::i64)m_Impl->m_iFilePos - (GR::i64)m_CacheSize + (GR::i64)m_Impl->m_Cache.Size();
          m_Impl->m_iFilePos += iOffset - (GR::i64)m_Impl->m_Cache.Size();
          //iOffset -= (GR::i64)m_Impl->m_iFilePos - (GR::i64)m_Impl->m_Cache.Size();
          //m_Impl->m_iFilePos += iOffset;
          */
          /*
          m_Impl->m_iFilePos += iOffset - m_Impl->m_Cache.Size();
          iOffset = m_Impl->m_iFilePos;
          */
          m_Impl->m_PseudoFilePos     += Offset;
          m_Impl->m_FilePosAfterCache = m_Impl->m_PseudoFilePos;
          Offset                      = m_Impl->m_PseudoFilePos;
          Position                    = PT_SET;
          break;
        case PT_SET:
          m_Impl->m_PseudoFilePos     = Offset;
          m_Impl->m_FilePosAfterCache = Offset;
          break;
        case PT_END:
          m_Impl->m_PseudoFilePos     = (GR::i64)GetSize();
          m_Impl->m_FilePosAfterCache = m_Impl->m_PseudoFilePos;
          break;
      }
      m_Impl->m_Cache.Clear();
      m_Impl->m_CacheBytesUsed = 0;
#if OPERATING_SYSTEM == OS_WINDOWS
  LARGE_INTEGER     offset;

  offset.QuadPart = Offset;
  return SetFilePointerEx( m_Impl->m_Handle, offset, NULL, Position );
    #elif OPERATING_SYSTEM == OS_ANDROID
      dh::Log( "FileStream::SetPosition not supported yet" );
      return 0;
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      //return 0;
      FILE_SETPOSITION_( m_Impl->m_Handle, Offset );
      return Offset;
    #else
      // TODO - Assume seek-types match pType
      fseek( m_Impl->m_Handle, (long)Offset, (int)Position );
      return ftell( m_Impl->m_Handle );
    #endif
    }



    GR::u64 FileStream::GetPosition()
    {
      if ( m_Impl->m_Handle == NULL_HANDLE )
      {
        return (GR::u64)-1;
      }
      return m_Impl->m_PseudoFilePos;
    }



    bool FileStream::Flush()
    {
      if ( m_Impl->m_Handle == NULL_HANDLE )
      {
        return false;
      }
    #if OPERATING_SYSTEM == OS_WINDOWS
      return ( FlushFileBuffers( m_Impl->m_Handle ) == TRUE );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      return true;
    #elif OPERATING_SYSTEM == OS_ANDROID
      dh::Log( "FileStream::Flush not supported yet" );
      return false;
    #else
      return ( fflush( m_Impl->m_Handle ) == 0 );
    #endif
    }



    bool FileStream::IsGood()
    {
      if ( m_Impl.ptr() == NULL )
      {
        return false;
      }
      return ( m_Impl->m_Handle != NULL_HANDLE );
    }



    void FileStream::SetCacheSize( size_t Size )
    {
      m_CacheSize = Size;
      if ( m_Impl.ptr() == NULL )
      {
        return;
      }
      // force reload of cache
      m_Impl->m_Cache.Clear();
      m_Impl->m_CacheBytesUsed    = 0;
      m_Impl->m_FilePosAfterCache = m_Impl->m_PseudoFilePos;
    }



    GR::i32 FileStream::LastError() const
    {
      return m_LastError;
    }



    bool FileStream::DataAvailable()
    {
      return m_Impl->m_PseudoFilePos < m_Impl->m_CachedFileSize;
    }



  }

}
