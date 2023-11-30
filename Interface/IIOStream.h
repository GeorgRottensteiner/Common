#ifndef ISTREAM_INTERFACE_H
#define ISTREAM_INTERFACE_H

#include <GR/GRTypes.h>
#include <string>

#include <Memory/ByteBuffer.h>



struct IIOStream
{

  public:

    enum OpenType
    {
      OT_CLOSED     = 0,
      OT_READ_ONLY,
      OT_WRITE_ONLY,
      OT_READ_WRITE,
      OT_WRITE_APPEND,
      OT_READ_ONLY_SHARED,
      OT_DEFAULT    = OT_READ_ONLY
    };

    enum PositionType
    {
      PT_SET            = 0,
      PT_CURRENT        = 1,
      PT_END            = 2,
      PT_DEFAULT        = PT_SET
    };


  protected:

    virtual void DoClose() = 0;


  public:


    virtual ~IIOStream()
    {
    }

    virtual bool Open( const char*, OpenType = OT_DEFAULT ) = 0;
    virtual void Close() = 0;
    virtual bool Error() = 0;
    virtual bool Flush() = 0;
    virtual bool IsGood() = 0;

    virtual GR::u64           GetSize() = 0;
    virtual bool              DataAvailable() = 0;
    virtual GR::u8            ReadU8() = 0;
    virtual GR::u16           ReadU16() = 0;
    virtual GR::u32           ReadU32() = 0;
    virtual GR::u64           ReadU64() = 0;
    virtual GR::u16           ReadU16NetworkOrder() = 0;
    virtual GR::u32           ReadU32NetworkOrder() = 0;
    virtual GR::u64           ReadU64NetworkOrder() = 0;
    virtual GR::i32           ReadI32() = 0;
    virtual GR::i32           LastError() const = 0;
    virtual size_t            ReadSize() = 0;
    virtual float             ReadF32() = 0;
    virtual unsigned long     ReadBlock( void* pTarget, size_t ulSize ) = 0;
    virtual unsigned long     ReadInBuffer( ByteBuffer& Target, size_t NumBytes ) = 0;
    virtual bool              ReadLine( char* pTarget, unsigned long ulMaxReadLength ) = 0;
    virtual bool              ReadLine( GR::String& strResult ) = 0;
    virtual bool              ReadLine( GR::WString& strResult ) = 0;
    virtual unsigned long     ReadString( char* lpszDestination, unsigned long ulMaxReadLength ) = 0;
    virtual GR::String        ReadString() = 0;
    virtual GR::WString       ReadStringW() = 0;
    virtual unsigned long     ReadStringW( GR::WString& strString ) = 0;
    virtual unsigned long     ReadString( GR::String& strString ) = 0;
    virtual unsigned long     WriteU8( GR::u8 ucValue ) = 0;
    virtual unsigned long     WriteU16( GR::u16 wValue ) = 0;
    virtual unsigned long     WriteU32( GR::u32 ulValue ) = 0;
    virtual unsigned long     WriteU64( GR::u64 ulValue ) = 0;
    virtual unsigned long     WriteU16NetworkOrder( GR::u16 wValue ) = 0;
    virtual unsigned long     WriteU32NetworkOrder( GR::u32 ulValue ) = 0;
    virtual unsigned long     WriteU64NetworkOrder( GR::u64 ulValue ) = 0;
    virtual unsigned long     WriteI32( GR::i32 ulValue ) = 0;
    virtual unsigned long     WriteSize( size_t ulValue ) = 0;
    virtual unsigned long     WriteF32( GR::f32 fValue ) = 0;
    virtual unsigned long     WriteBlock( const void*, size_t ) = 0;
    virtual unsigned long     WriteLine( const char* szLine ) = 0;
    virtual unsigned long     WriteLine( const GR::String& strLine ) = 0;
    virtual unsigned long     WriteString( const char *lpszSource ) = 0;
    virtual unsigned long     WriteString( const GR::String& strString ) = 0;
    virtual unsigned long     WriteStringW( const GR::WString& strString ) = 0;
    virtual unsigned long     SetPosition( GR::i64, PositionType = PT_SET ) = 0;
    virtual GR::u64           GetPosition() = 0;

};



struct IIOStreamBase : IIOStream
{

  protected:

    bool                  m_Opened,
                          m_ReadFailed;

    OpenType              m_OpenType;


    virtual void DoClose()
    {
    }


  public:


    IIOStreamBase() :
      m_Opened( false ),
      m_ReadFailed( false ),
      m_OpenType( OT_CLOSED )
    {
    }



    virtual bool              Open( const char*, OpenType = OT_DEFAULT )
    {
      return false;
    }



    virtual void              Close()
    {
      m_Opened = false;
    }



    virtual bool              Error()
    {
      return m_ReadFailed;
    }



    virtual bool              Flush()
    {
      return false;
    }



    virtual bool              IsGood()
    {
      return false;
    }


    virtual GR::u64           GetSize()
    {
      return 0;
    }



    virtual unsigned char     ReadU8()
    {
      static unsigned char    ucDummy = 0;

      if ( ReadBlock( &ucDummy, 1 ) != 1 )
      {
        return 0;
      }
      return ucDummy;
    }



    virtual unsigned short    ReadU16()
    {
      static unsigned short    wDummy = 0;

      if ( ReadBlock( &wDummy, 2 ) != 2 )
      {
        return 0;
      }
      return wDummy;
    }



    virtual unsigned short    ReadU16NetworkOrder()
    {
      return ReadU8() * 256 + ReadU8();
    }



    virtual unsigned long ReadInBuffer( ByteBuffer& Target, size_t NumBytes )
    {
      if ( Target.Size() < NumBytes )
      {
        Target.Resize( NumBytes );
      }
      return ReadBlock( Target.Data(), NumBytes );
    }



    virtual GR::u32 ReadU32NetworkOrder()
    {
      return ( ReadU8() << 24 ) + ( ReadU8() << 16 ) + ( ReadU8() << 8 ) + ReadU8();
    }



    virtual GR::u64 ReadU64NetworkOrder()
    {
      return ( (GR::u64)ReadU8() << 56 ) + ( (GR::u64)ReadU8() << 48 ) + ( (GR::u64)ReadU8() << 40 ) + ( (GR::u64)ReadU8() << 32 )
           + ( (GR::u64)ReadU8() << 24 ) + ( (GR::u64)ReadU8() << 16 ) + ( (GR::u64)ReadU8() << 8 ) + (GR::u64)ReadU8();
    }



    virtual GR::u32 ReadU32()
    {
      static unsigned long    dwDummy = 0;

      if ( ReadBlock( &dwDummy, 4 ) != 4 )
      {
        return 0;
      }
      return dwDummy;
    }



    virtual GR::u64  ReadU64()
    {
      static GR::u64    dummy = 0;

      if ( ReadBlock( &dummy, 8 ) != 8 )
      {
        return 0;
      }
      return dummy;
    }



    virtual GR::i32 ReadI32()
    {
      static GR::i32     dwDummy = 0;

      if ( ReadBlock( &dwDummy, 4 ) != 4 )
      {
        return 0;
      }
      return dwDummy;
    }



    virtual size_t            ReadSize()
    {
      static size_t   dwDummy = 0;

      if ( ReadBlock( &dwDummy, sizeof( size_t ) ) != sizeof( size_t ) )
      {
        return 0;
      }
      return dwDummy;
    }



    virtual float             ReadF32()
    {
      static float            fDummy = 0;

      if ( ReadBlock( &fDummy, 4 ) != 4 )
      {
        return 0.0f;
      }
      return fDummy;
    }



    virtual bool              ReadLine( char* pTarget, unsigned long ulMaxReadLength )
    {
      char            a = '\0';

      unsigned long   ulOffset = 0;

      for ( ;; )
      {
        a = ReadU8();
        if ( ( m_ReadFailed )
             || ( ulOffset + 1 >= ulMaxReadLength )
             || ( a == 10 ) )
        {
          *( pTarget + ulOffset++ ) = '\0';
          return true;
        }
        else if ( a != 13 )
        {
          *( pTarget + ulOffset++ ) = a;
        }
      }
    }



    virtual bool              ReadLine( GR::String& strResult )
    {
      char            cRead;
      bool            wasCR = false;

      strResult.erase();
      for ( ;; )
      {
        cRead = ReadU8();
        if ( m_ReadFailed )
        {
          return !strResult.empty();
        }
        if ( cRead == 10 )
        {
          return true;
        }
        if ( wasCR )
        {
          // a CR without followup LF
          char      CR = 13;
          strResult.append( &CR, 1 );
          wasCR = false;
        }
        if ( cRead != 13 )
        {
          strResult.append( &cRead, 1 );
        }
        else
        {
          wasCR = true;
        }
      }
    }



    virtual bool              ReadLine( GR::WString& strResult )
    {
      GR::u16       cRead;

      strResult.erase();
      for ( ;; )
      {
        cRead = ReadU16();
        if ( m_ReadFailed )
        {
          return !strResult.empty();
        }
        if ( cRead == 10 )
        {
          return true;
        }
        else if ( cRead != 13 )
        {
          strResult.append( 1, (GR::WChar)cRead );
        }
      }
    }



    virtual unsigned long     ReadString( char* lpszDestination, unsigned long ulMaxReadLength )
    {
      char            a = '\0';

      unsigned long   ulOffset = 0;

      for ( ;; )
      {
        a = ReadU8();
        if ( ( m_ReadFailed )
             || ( ulOffset + 1 >= ulMaxReadLength )
             || ( a == '\0' ) )
        {
          *( lpszDestination + ulOffset++ ) = '\0';
          break;
        }
        else
        {
          *( lpszDestination + ulOffset++ ) = a;
        }
      }

      return ulOffset;
    }



    virtual GR::String ReadString()
    {
      GR::String strResult;

      ReadString( strResult );

      return strResult;
    }



    virtual GR::WString ReadStringW()
    {
      GR::WString strResult;

      ReadStringW( strResult );

      return strResult;
    }



    virtual unsigned long     ReadStringW( GR::WString& strString )
    {
      GR::u16       a = 0;

      unsigned long   ulOffset = 0;

      strString.erase();
      unsigned long   dwLength = ReadU32();
      strString.reserve( dwLength );
      while ( dwLength-- )
      {
        a = ReadU16();
        if ( m_ReadFailed )
        {
          break;
        }
        ulOffset++;
        strString += a;
      }
      return ulOffset;
    }



    virtual unsigned long     ReadString( GR::String& strString )
    {
      char            a = '\0';

      unsigned long   ulOffset = 0;

      strString.erase();
      unsigned long   dwLength = ReadU32();
      strString.reserve( dwLength );
      while ( dwLength-- )
      {
        a = ReadU8();
        if ( m_ReadFailed )
        {
          break;
        }
        ulOffset++;
        strString += a;
      }
      return ulOffset;
    }



    virtual unsigned long     WriteU8( GR::u8 ucValue )
    {
      return WriteBlock( &ucValue, 1 );
    }



    virtual unsigned long     WriteU16( GR::u16 wValue )
    {
      return WriteBlock( &wValue, 2 );
    }



    virtual unsigned long     WriteU16NetworkOrder( GR::u16 Value )
    {
      return WriteU8( (GR::u8)( Value >> 8 ) ) + WriteU8( (GR::u8)( Value & 0xff ) );
    }



    virtual unsigned long     WriteU32( GR::u32 ulValue )
    {
      return WriteBlock( &ulValue, 4 );
    }



    virtual unsigned long     WriteU32NetworkOrder( GR::u32 Value )
    {
      return WriteU8( (GR::u8)( Value >> 24 ) )
           + WriteU8( (GR::u8)( ( Value >> 16 ) & 0xff ) )
           + WriteU8( (GR::u8)( ( Value >> 8 ) & 0xff ) )
           + WriteU8( (GR::u8)( Value & 0xff ) );
    }



    virtual unsigned long     WriteU64( GR::u64 ulValue )
    {
      return WriteBlock( &ulValue, 8 );
    }



    virtual unsigned long     WriteU64NetworkOrder( GR::u64 Value )
    {
      return WriteU8( (GR::u8)( Value >> 56 ) ) + WriteU8( (GR::u8)( ( Value >> 48 ) & 0xff ) ) + WriteU8( (GR::u8)( ( Value >> 40 ) & 0xff ) ) + WriteU8( (GR::u8)( ( Value >> 32 ) & 0xff ) )
           + WriteU8( (GR::u8)( ( Value >> 24 ) & 0xff ) ) + WriteU8( (GR::u8)( ( Value >> 16 ) & 0xff ) ) + WriteU8( (GR::u8)( ( Value >> 8 ) & 0xff ) ) + WriteU8( (GR::u8)( Value & 0xff ) );
    }



    virtual unsigned long     WriteI32( GR::i32 ulValue )
    {
      return WriteBlock( &ulValue, 4 );
    }



    virtual unsigned long     WriteSize( size_t ulValue )
    {
      return WriteBlock( &ulValue, sizeof( size_t ) );
    }



    virtual unsigned long     WriteF32( GR::f32 fValue )
    {
      return WriteBlock( &fValue, 4 );
    }



    virtual unsigned long     WriteBlock( const void*, size_t )
    {
      return 0;
    }



    virtual unsigned long     WriteLine( const char* szLine )
    {
      unsigned long   ulSize = WriteBlock( szLine, (GR::u32)strlen( szLine ) );
      ulSize += WriteU8( 13 );
      ulSize += WriteU8( 10 );
      return ulSize;
    }



    virtual unsigned long     WriteLine( const GR::String& strLine )
    {
      unsigned long   ulSize = WriteBlock( strLine.c_str(), (GR::u32)strLine.length() );
      ulSize += WriteU8( 13 );
      ulSize += WriteU8( 10 );
      return ulSize;
    }



    virtual unsigned long     WriteString( const char* lpszSource )
    {
      unsigned long ulSize = (unsigned long)strlen( lpszSource );
      if ( lpszSource != NULL )
      {
        if ( ulSize )
        {
          WriteBlock( lpszSource, ulSize );
        }
      }
      ulSize += WriteU8( 0 );
      return ulSize;
    }



    virtual unsigned long     WriteString( const GR::String& strString )
    {
      unsigned long   ulSize = WriteU32( (unsigned long)strString.length() );
      ulSize += WriteBlock( strString.c_str(), (unsigned long)strString.length() );
      return ulSize;
    }



    virtual unsigned long     WriteStringW( const GR::WString& strString )
    {
      unsigned long   ulSize = WriteU32( (unsigned long)strString.length() );
      ulSize += WriteBlock( strString.c_str(), 2 * (unsigned long)strString.length() );
      return ulSize;
    }



    virtual unsigned long     SetPosition( GR::i64, PositionType = PT_SET )
    {
      return 0;
    }



    virtual GR::u64           GetPosition()
    {
      return 0;
    }



    virtual GR::i32           LastError() const
    {
      return 0;
    }


};



#endif // __ISTREAM_INTERFACE_H__



