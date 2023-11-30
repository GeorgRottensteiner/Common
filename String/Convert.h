#ifndef CONVERT_H
#define CONVERT_H

#include <string>
#include <locale>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>



namespace GR
{

  namespace Convert
  {
    GR::String        ISO8895ToUTF8( const GR::String& ISO );
    GR::String        UTF8ToISO8895( const GR::String& Input );

    GR::WString       ISO8895ToUTF16( const GR::String& ISO );

    GR::WString       ToUTF16( const GR::String& Text );
    GR::WString       ToUTF16( const GR::WString& Text );

    GR::String        ToUTF8( const GR::WString& Text );
    GR::String        ToUTF8( const GR::String& Text );

    GR::String        ToStringA( const GR::Char* Text );
    GR::String        ToStringA( const GR::String& Text );

    GR::WString       ToStringW( const GR::Char* Text );
    GR::WString       ToStringW( const GR::WString& strText );

    GR::String        ToString( const GR::String& Text );
    GR::String        ToString( const GR::WString& Text );

    GR::WString       ToStringW( const GR::String& strText );
    GR::String        ToStringA( const GR::WString& strText );
    
    GR::String        ToStringA( GR::i64 iValue );
    GR::WString       ToStringW( GR::i64 iValue );
    GR::String        ToString( GR::i64 iValue );

    GR::String        F64ToStringA( GR::f64 fValue );
    GR::WString       F64ToStringW( GR::f64 fValue );
    GR::String        F64ToString( GR::f64 iValue );

    GR::String        ToHexStringA( const void* pData, size_t dwDataSize );
    GR::WString       ToHexStringW( const void* pData, size_t dwDataSize );
    GR::String        ToHexString( const void* pData, size_t dwDataSize );

    GR::String        ToHex( GR::u64 dwValue, const GR::u32 Stellen = 0 );
    GR::WString       ToHexW( GR::u64 dwValue, const GR::u32 Stellen = 0 );

    GR::f32           ToF32( const GR::String& strValue );
    GR::f32           ToF32( const GR::WString& strValue );

    GR::f64           ToF64( const GR::String& strValue );
    GR::f64           ToF64( const GR::WString& strValue );

    GR::u8            ToU8( const GR::String& strValue, GR::ip iBase = 10 );

    GR::u8            ToU8( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::u16           ToU16( const GR::String& strValue, GR::ip iBase = 10 );
    GR::u16           ToU16( const GR::WString& strValue, GR::ip iBase = 10 );
    GR::i16           ToI16( const GR::String& strValue, GR::ip iBase = 10 );
    GR::i16           ToI16( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::u32           ToU32( const GR::String& strValue, GR::ip iBase = 10 );
    GR::u32           ToU32( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::u64           ToU64( const GR::String& strValue, GR::ip iBase = 10 );
    GR::u64           ToU64( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::up            ToUP( const GR::String& strValue, GR::ip iBase = 10 );
    GR::up            ToUP( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::ip            ToIP( const GR::String& strValue, GR::ip iBase = 10 );
    GR::ip            ToIP( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::i32           ToI32( const GR::String& strValue, GR::ip iBase = 10 );
    GR::i32           ToI32( const GR::WString& strValue, GR::ip iBase = 10 );

    GR::i64           ToI64( const GR::String& strValue, GR::ip iBase = 10 );
    GR::i64           ToI64( const GR::WString& strValue, GR::ip iBase = 10 );

    bool              ToBool( const GR::String& Value );
    bool              ToBool( const GR::WString& Value );

    ByteBuffer        ToBCD( GR::i64 Value, size_t NumDigits, bool FixedLength = true );
    ByteBuffer        ToBCD( const GR::String& Value, size_t NumDigits, bool FixedLength = false );
    ByteBuffer        ToBCD( const GR::String& Value, size_t NumDigits, bool FixedLength, bool& HadError );

    GR::String        BCDToString( const ByteBuffer& Data );
    GR::String        BCDToString( const GR::u8* pData, size_t Length );
    GR::i64           BCDToValue( const ByteBuffer& Data, bool NegativeAllowed = false );
    GR::i64           BCDToValue( const GR::u8* pData, size_t Length, bool NegativeAllowed = false );

  }

}

#endif // CONVERT_H
