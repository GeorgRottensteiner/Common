#ifndef ISTREAM_INTERFACE_H
#define ISTREAM_INTERFACE_H

#include <GR/GRTypes.h>
#include <string>



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

    bool                  m_Opened,
                          m_ReadFailed;

    OpenType              m_OpenType;


    virtual void          DoClose()
    {
    }


  public:


    IIOStream() :
      m_Opened( false ),
      m_ReadFailed( false ),
      m_OpenType( OT_CLOSED )
    {
    }

    virtual ~IIOStream()
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


    // Lese-Operationen
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

    virtual unsigned long     ReadU32()
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

    virtual signed long     ReadI32()
    {
      static signed long    dwDummy = 0;

      if ( ReadBlock( &dwDummy, 4 ) != 4 )
      {
        return 0;
      }
      return dwDummy;
    }


    // must be overridden by child class
    virtual GR::i32           LastError() const
    {
      return -1;
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

    virtual unsigned long     ReadBlock( void* pTarget, size_t ulSize ) = 0;

    virtual bool              ReadLine( char* pTarget, unsigned long ulMaxReadLength )
    {
      char            a         = '\0';

      unsigned long   ulOffset  = 0;

      for ( ;; )
      {
        a = ReadU8();
        if ( ( m_ReadFailed )
        ||   ( ulOffset + 1 >= ulMaxReadLength )
        ||   ( a == 10 ) )
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
      char            a         = '\0';

      unsigned long   ulOffset  = 0;

      for ( ;; )
      {
        a = ReadU8();
        if ( ( m_ReadFailed )
        ||   ( ulOffset + 1 >= ulMaxReadLength )
        ||   ( a == '\0' ) )
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

    // Liest einen GR::String, zuerst ein DWORD als Länge und dann DWORD Zeichen in den String, der String wird gelöscht!
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

      unsigned long   ulOffset  = 0;

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
      char            a         = '\0';

      unsigned long   ulOffset  = 0;

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

    virtual unsigned long     WriteU32( GR::u32 ulValue )
    {
      return WriteBlock( &ulValue, 4 );
    }

    virtual unsigned long     WriteU64( GR::u64 ulValue )
    {
      return WriteBlock( &ulValue, 8 );
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
#ifdef __TANDEM
      ulSize += WriteU16( 13 * 256 + 10 );
#else
      ulSize += WriteU16( 10 * 256 + 13 );
#endif
      return ulSize;
    }

    virtual unsigned long     WriteLine( const GR::String& strLine )
    {
      unsigned long   ulSize = WriteBlock( strLine.c_str(), (GR::u32)strLine.length() );
#ifdef __TANDEM
      ulSize += WriteU16( 13 * 256 + 10 );
#else
      ulSize += WriteU16( 10 * 256 + 13 );
#endif
      return ulSize;
    }

    virtual unsigned long     WriteString( const char *lpszSource )
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

};



#endif // __ISTREAM_INTERFACE_H__



