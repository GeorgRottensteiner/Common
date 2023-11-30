#if OPERATING_SYSTEM == OS_TANDEM

#include "GuardianFile.h"

#include <String/Convert.h>

#include <tal.h>
#include <cextdecs.h>

#pragma nowarn (262)



namespace GR
{
  namespace IO
  {

static short NULL_HANDLE = -1;



GuardianFile::GuardianFile() :
  IIOStreamBase(),
  m_Impl( new GuardianFileImpl( NULL_HANDLE ) ),
  m_CacheSize( 0 ),
  m_LastError( 0 )
{

}



GuardianFile::GuardianFile( const GR::Char* szFileName, IIOStream::OpenType oType ) :
  IIOStreamBase(),
  m_Impl( new GuardianFileImpl( NULL_HANDLE ) ),
  m_CacheSize( 0 ),
  m_LastError( 0 )
{

  Open( szFileName, oType );

}



GuardianFile::~GuardianFile()
{

}



void GuardianFile::Close()
{
  if ( m_Impl )
  {
    m_Impl->Close();
  }

  IIOStreamBase::Close();

  m_OpenType  = OT_CLOSED;
}



bool GuardianFile::OpenTandem( IIOStream::OpenType OType,
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
                             bool Buffered,
                             bool OddUnstructured )
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

  if ( ( FileCode != 101 )
  ||   ( FileType != 0 ) )
  {
    // not an EDIT file

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
    if ( OddUnstructured )
    {
      if ( FileType == 0 )
      {
        // odd filebyte, only allowed for unstructured files
        ItemList.AppendU16NetworkOrder( 65 );
        ValueList.AppendU16NetworkOrder( 1 );
      }
    }
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

  short hHandle = NULL_HANDLE;
  m_LastError = 0;

  if ( ( FileCode != 101 )
  ||   ( FileType != 0 ) )
  {
    _cc_status Status = FILE_CREATELIST_( NewFilename, FilenameLen, &FilenameLen,
                                          (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                          (short*)ValueList.Data(), (short)ValueList.Size() );
    //if ( _status_lt( Status ) )
    if ( Status != 0 )
    {
      m_LastError = (short)Status;
      //printf( "FILE_CREATELIST_ failed %d\n", m_LastError );
      return false;
    }

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
  }
  else
  {
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
  }

  if ( _status_lt( m_LastError ) )
  {
    Close();
    return false;
  }

  if ( hHandle != NULL_HANDLE )
  {
    m_Impl = new GuardianFileImpl( hHandle );

    // special behaviour for guardian text files
    if ( ( FileCode == 101 )
    &&   ( FileType == 0 ) )
    {
      m_Impl->m_EditFileOpened = true;
    }

    m_Opened = true;
    if ( m_OpenType == OT_WRITE_APPEND )
    {
      SetPosition( 0, IIOStream::PT_END );
    }
  }
  return ( hHandle != NULL_HANDLE );
}



GR::i32 GuardianFile::LastError() const
{
  return m_LastError;
}



short GuardianFile::GetFileError() const
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



bool GuardianFile::Open( const GR::Char* FileName, OpenType Type )
{
  if ( ( m_Impl )
  &&   ( m_Impl->m_Handle != NULL_HANDLE ) )
  {
    return false;
  }

  m_OpenType  = Type;

  short hHandle = NULL_HANDLE;
  m_LastError = 0;
  switch ( m_OpenType )
  {
    case OT_READ_ONLY:
      m_LastError = FILE_OPEN_( FileName, (short)strlen( FileName ), &hHandle, 1, 1 );
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
      m_LastError    = FILE_OPEN_( FileName, (short)strlen( FileName ), &hHandle, 1, 0 );
      break;
    case OT_WRITE_ONLY:
      return OpenTandem( OT_WRITE_ONLY,
                         FileName,
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
      m_LastError = FILE_OPEN_( FileName, (short)strlen( FileName ), &hHandle, 2 );
      m_LastError = FILE_SETPOSITION_( hHandle, -1 );
      break;
    case OT_READ_WRITE:
      m_LastError = FILE_OPEN_( FileName, (short)strlen( FileName ), &hHandle, 0 );
      break;
  }
  if ( _status_lt( m_LastError ) )
  {
    Close();
    return false;
  }

  if ( hHandle != NULL_HANDLE )
  {
    m_Impl = new GuardianFileImpl( hHandle );
    m_Opened = true;

    if ( m_OpenType == OT_WRITE_APPEND )
    {
      SetPosition( 0, IIOStream::PT_END );
    }
  }

  return ( hHandle != NULL_HANDLE );
}




short GuardianFile::GetHandle()
{
  return m_Impl->m_Handle;
}



GR::u64 GuardianFile::GetSize()
{
  ByteBuffer    ItemList;
  ByteBuffer    ValueList;

  // EOF location
  ItemList.AppendU16NetworkOrder( 191 );
  ValueList.Resize( 8 );

  GR::u16   returnedBytes = 0;
  short     errorItem = 0;

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
  return ValueList.U64At( 0 );
}



GR::u64 GuardianFile::GetNumberOfRecords()
{
  ByteBuffer    ItemList;
  ByteBuffer    ValueList;

  // File type
  ItemList.AppendU16NetworkOrder( 196 );
  ValueList.Resize( 4 );
  ItemList.AppendU16NetworkOrder( 197 );
  ValueList.Resize( 4 );

  GR::u16 returnedBytes = 0;
  short   errorItem = 0;

  _cc_status Status = FILE_GETINFOLIST_( m_Impl->m_Handle,
                                        (short*)ItemList.Data(), (short)( ItemList.Size() / 2 ),
                                        (short*)ValueList.Data(), (short)ValueList.Size(),
                                        &returnedBytes,
                                        &errorItem );
  if ( _status_lt( Status ) )
  {
    m_LastError = (short)Status;
    printf( "FILE_GETINFOLIST_ failed %d\n", m_LastError );
    return false;
  }
  printf( "record size 196: %d / %04X\n", ValueList.U32NetworkOrderAt( 0 ), ValueList.U32NetworkOrderAt( 0 ) );
  printf( "197: %d / %04X\n", ValueList.U32NetworkOrderAt( 4 ), ValueList.U32NetworkOrderAt( 4 ) );
  return 0;

}



bool GuardianFile::ReadToBuffer( void* pTarget, unsigned long BytesToRead, unsigned long& BytesRead )
{

  unsigned long  ulNewBytesRead = 0;
  unsigned long  BytesToReadTemp = BytesToRead;
  unsigned short BytesToReadNow = 0;
  bool           Fehler = false;
  unsigned short NewBytesRead = 0;
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
    if ( ( _status_lt( Status ) )
    &&   ( Status != 1 ) )  // 1 indicates EOF
    {
      // Fehler
      Fehler = true;
      m_LastError = (short)Status;
      //printf( "-failed (%d bytes gotten)\n", NewBytesRead );
      printf( "Failed to read, received error %d\n", Status );
      break;
    }
    //printf( "-read %d bytes\n", NewBytesRead );
    ulNewBytesRead += NewBytesRead;
    BytesRead += NewBytesRead;
    // Guardian can read more(!) than requested, make sure the counter does not wrap around!
    if ( BytesToReadTemp >= NewBytesRead )
    {
      BytesToReadTemp -= NewBytesRead;
    }
    else
    {
      // we've got more than we wanted!
      BytesToReadTemp = 0;
    }
    if ( Status == 1 )
    {
      if ( BytesRead > 0 )
      {
        return true;
      }
      // EOF, and no bytes found
      m_LastError = (short)Status;
      return false;
    }
    if ( NewBytesRead < BytesToReadNow )
    {
      // Fehler
      //printf( "Read less than expected (%d < %d)\n", NewBytesRead, BytesToReadNow );

      Fehler = true;
      m_LastError = (short)Status;
      break;
    }
  }
  if ( Fehler )
  {
    return false;
  }
  //Log( Misc::Format( "Bytes wanted %1% Bytes got %2%" ) << BytesToRead
  return ( ulNewBytesRead == BytesToRead );

}



bool GuardianFile::HadReadError()
{

  return m_ReadFailed;

}



unsigned long GuardianFile::ReadBlock( void* pDestination, GR::up CountBytes )
{

  if ( ( GetHandle() == -1 )
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
  GR::up            ulBytesToRead = CountBytes;

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
      //printf( "ReadToBuffer failed\n" );
      m_ReadFailed = true;
    }
    m_Impl->m_PseudoFilePos     += ulNewBytesRead;
    m_Impl->m_FilePosAfterCache += ulNewBytesRead;
    ulBytesRead += ulNewBytesRead;
    /*
    if ( ulBytesRead < CountBytes )
    {
      m_ReadFailed = true;
    }*/
    //dh::Log( "%08x-direct read (want %d got %d)", m_Impl, ulBytesToRead, ulNewBytesRead );
    //printf( "%08x-direct read (want %d got %d)\n", m_Impl, ulBytesToRead, ulNewBytesRead );
    //dh::Hex( lpucDestination, ulBytesRead );
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




bool GuardianFile::ReadLine( char* pTarget, unsigned long MaxReadLength )
{
  return IIOStreamBase::ReadLine( pTarget, MaxReadLength );
}



bool GuardianFile::ReadLine( GR::WString& Result )
{
  return IIOStreamBase::ReadLine( Result );
}



bool GuardianFile::ReadLine( GR::String& Result )
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



unsigned long GuardianFile::WriteBlock( const void* pSource, GR::up CountBytes )
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

  short BytesToWriteNow = 0;
  unsigned short ulNewBytesWritten = 0;

  if ( m_Impl->m_EditFileOpened )
  {
    while ( CountBytes > 0 )
    {
      BytesToWriteNow = (short)CountBytes;
      if ( BytesToWriteNow > 255 )
      {
        BytesToWriteNow = 255;
      }
      _cc_status Status = WRITEEDIT( m_Impl->m_Handle, -2, ( (char*)pSource ) + ulNewBytesWritten, BytesToWriteNow, 1 );
      if ( _status_lt( Status ) )
      {
        return ulNewBytesWritten;
      }
      ulNewBytesWritten += BytesToWriteNow;
      CountBytes -= BytesToWriteNow;
    }
    return ulNewBytesWritten;
  }


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
    _cc_status Status = WRITEX( m_Impl->m_Handle, ( (char*)pSource ) + ulBytesWritten, BytesToWriteNow, &ulNewBytesWritten );
    if ( ( _status_lt( Status ) )
    ||   ( ulNewBytesWritten < BytesToWriteNow ) )
    {
      // Fehler
      //printf( "Error calling writeX (%d)\n", GetFileError() );
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

  m_Impl->m_FilePosAfterCache += ulBytesWritten;
  m_Impl->m_PseudoFilePos     += ulBytesWritten;

  return ulBytesWritten;

}



unsigned long GuardianFile::SetPosition( GR::i64 Offset, PositionType Position )
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
  return 0;

}



GR::u64 GuardianFile::GetPosition()
{

  if ( m_Impl->m_Handle == NULL_HANDLE )
  {
    return (GR::u64)-1;
  }

  return m_Impl->m_PseudoFilePos;

}



bool GuardianFile::Flush()
{

  if ( m_Impl->m_Handle == NULL_HANDLE )
  {
    return false;
  }
  return true;

}



bool GuardianFile::IsGood()
{

  if ( m_Impl.ptr() == NULL )
  {
    return false;
  }
  return ( m_Impl->m_Handle != NULL_HANDLE );

}



void GuardianFile::SetCacheSize( size_t Size )
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



bool GuardianFile::DataAvailable()
{
  return GetPosition() < GetSize();
}



}

}


#endif // OS_TANDEM
