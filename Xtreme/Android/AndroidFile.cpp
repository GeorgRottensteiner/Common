#include <Xtreme/Android/AndroidFile.h>
#include <Xtreme/Environment/AndroidMain.h>
#include <IO\FileUtil.h>

#include <String/Convert.h>
#include <String/StringUtil.h>
#include <String/Path.h>



#include <debug/debugclient.h>



namespace GR
{
  namespace IO
  {
    AndroidFile::AndroidFile() :
      IIOStreamBase(),
      m_Impl( new AndroidFileImpl( NULL ) ),
      m_LastError( 0 ),
      m_CacheSize( 256 )
    {

    }



    AndroidFile::AndroidFile( const GR::Char* pFileName, IIOStream::OpenType oType ) :
      IIOStreamBase(),
      m_Impl( new AndroidFileImpl( NULL ) ),
      m_LastError( 0 ),
      m_CacheSize( 256 )
    {

      Open( pFileName, oType );

    }



    AndroidFile::AndroidFile( const GR::String& FileName, IIOStream::OpenType oType ) :
      IIOStreamBase(),
      m_Impl( new AndroidFileImpl( NULL ) ),
      m_LastError( 0 ),
      m_CacheSize( 256 )
    {
      Open( FileName, oType );
    }



    AndroidFile::~AndroidFile()
    {
    }



    void AndroidFile::Close()
    {
      if ( m_Impl )
      {
        m_Impl->Close();
      }

      IIOStreamBase::Close();

      m_OpenType  = OT_CLOSED;
    }



    bool AndroidFile::Open( const GR::Char* FileName, OpenType Type )
    {
      return Open( GR::String( FileName ), Type );
    }



    bool AndroidFile::Open( const GR::String& FileName, OpenType Type )
    {
      AAssetManager* pManager = AndroidMediator::s_pActivity->assetManager;

      if ( ( m_Impl )
      &&   ( m_Impl->m_Handle != NULL ) )
      {
        return false;
      }

      m_OpenType  = Type;

      m_LastError = 0;
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


      if ( hHandle != NULL )
      {
        m_Impl = new AndroidFileImpl( hHandle );
        m_Opened = true;

      }
      return ( hHandle != NULL );
    }



    AAsset* AndroidFile::GetHandle()
    {
      return m_Impl->m_Handle;
    }



    GR::u64 AndroidFile::GetSize()
    {
      return 0;
    }



    bool AndroidFile::ReadToBuffer( void* pTarget, unsigned long BytesToRead, unsigned long& BytesRead )
    {
      int bytesRead = AAsset_read( m_Impl->m_Handle, pTarget, BytesToRead );
      if ( bytesRead < 0 )
      {
        return false;
      }
      BytesRead = bytesRead;
      return ( BytesRead > 0 );
    }



    unsigned long AndroidFile::ReadBlock( void* pDestination, size_t CountBytes )
    {
      if ( ( m_Impl->m_Handle == NULL )
      ||   ( !m_Opened )
      ||   ( m_OpenType == OT_WRITE_ONLY )
      ||   ( m_OpenType == OT_WRITE_APPEND ) )
      {
        m_ReadFailed = true;
        return 0;
      }

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



    unsigned long AndroidFile::WriteBlock( const void* pSource, size_t CountBytes )
    {
      if ( ( !m_Impl.IsObjectValid() )
      ||   ( m_Impl->m_Handle == NULL )
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

      // NO writing?
      dh::Log( "writing to file currently not supported!" );
      //WriteFile( m_Impl->m_Handle, pSource, ( GR::u32 )CountBytes, &ulBytesWritten, NULL );
      //m_LastError = GetLastError();
      return 0;
      /*
      m_Impl->m_FilePosAfterCache += ulBytesWritten;
      m_Impl->m_PseudoFilePos     += ulBytesWritten;

      return ulBytesWritten;*/
    }



    unsigned long AndroidFile::SetPosition( GR::i64 Offset, PositionType Position )
    {
      if ( m_Impl->m_Handle == NULL )
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
      /*
#if OPERATING_SYSTEM == OS_WINDOWS
  LARGE_INTEGER     offset;

  offset.QuadPart = Offset;
  return SetFilePointerEx( m_Impl->m_Handle, offset, NULL, Position );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      //return 0;
      FILE_SETPOSITION_( m_Impl->m_Handle, Offset );
      return Offset;
    #else
      // TODO - Assume seek-types match pType
      fseek( m_Impl->m_Handle, (long)Offset, (int)Position );
      return ftell( m_Impl->m_Handle );
    #endif
      */
    }



    bool AndroidFile::DataAvailable()
    {
      if ( m_Impl->m_Handle == NULL )
      {
        return false;
      }
      // TODO - not really correct, innit?
      return true;
    }



    GR::u64 AndroidFile::GetPosition()
    {
      if ( m_Impl->m_Handle == NULL )
      {
        return (GR::u64)-1;
      }
      return m_Impl->m_PseudoFilePos;
    }



    bool AndroidFile::Flush()
    {
      if ( m_Impl->m_Handle == NULL )
      {
        return false;
      }
      /*
    #if OPERATING_SYSTEM == OS_WINDOWS
      return ( FlushFileBuffers( m_Impl->m_Handle ) == TRUE );
    #elif OPERATING_SUB_SYSTEM == OS_SUB_GUARDIAN
      return true;
    #else
      return ( fflush( m_Impl->m_Handle ) == 0 );
    #endif
      */
    }



    bool AndroidFile::IsGood()
    {
      if ( m_Impl.ptr() == NULL )
      {
        return false;
      }
      return ( m_Impl->m_Handle != NULL );
    }



    void AndroidFile::SetCacheSize( size_t Size )
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



    GR::i32 AndroidFile::LastError() const
    {
      return m_LastError;
    }

  }

}
