#include "Convert.h"

#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (262)
#endif

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#endif


#include <String/StringUtil.h>



namespace GR
{

  namespace Convert
  {
    GR::WString ISO8895ToUTF16( const GR::String& ISO )
    {
      return ToUTF16( ISO8895ToUTF8( ISO ) );
    }



    GR::String ISO8895ToUTF8( const GR::String& ISO )
    {
      GR::String     result;

      for ( size_t i = 0; i < ISO.length(); ++i )
      {
        char c = ISO[i];
        if ( ( GR::u8 )c < 0x80 )
        {
          result += c;
        }
        else if ( ( GR::u8 )c == 0x80 )
        {
          // special case Euro (ISO 8895-1 vs. -15)
          result += (char)164;
        }
        else
        {
          // first byte, simplified since our range is only 8-bits
          result += (char)( 0xc0 | ( c & 0xc0 ) >> 6 );
          result += (char)( 0x80 | ( c & 0x3f ) );
        }
      }
      return result;
    }



    GR::String UTF8ToISO8895( const GR::String& Input )
    {
      GR::String out;

      unsigned int codepoint = 0;

      for ( size_t i = 0; i < Input.length(); )
      {
        unsigned char ch = static_cast<unsigned char>( Input[i] );
        if ( ch <= 0x7f )
        {
          codepoint = ch;
        }
        else if ( ch == 0x80 )
        {
          // special case Euro (ISO8895-1 vs. -15)
          codepoint = ch;
        }
        else if ( ch <= 0xbf )
        {
          codepoint = ( codepoint << 6 ) | ( ch & 0x3f );
        }
        else if ( ch <= 0xdf )
        {
          codepoint = ch & 0x1f;
        }
        else if ( ch <= 0xef )
        {
          codepoint = ch & 0x0f;
        }
        else
        {
          codepoint = ch & 0x07;
        }
        ++i;
        if ( ( i < Input.length() )
        &&   ( ( Input[i] & 0xc0 ) != 0x80 )
        &&   ( codepoint <= 0x10ffff ) )
        {
          if ( codepoint <= 255 )
          {
            out.append( 1, static_cast<char>( codepoint ) );
          }
          else
          {
            // do whatever you want for out-of-bounds characters
          }
        }
        else if ( ( codepoint <= 255 )
        &&        ( i == Input.length() ) )
        {
          out.append( 1, static_cast<char>( codepoint ) );
        }
      }
      return out;
    }




    GR::String ToUTF8( const GR::String& Text )
    {
      return Text;
    }



    GR::WString ToUTF16( const GR::WString& Text )
    {
      return Text;
    }



    GR::WString ToUTF16( const GR::String& Text )
    {
      GR::WString      out;
      if ( Text.empty() )
      {
        return out;
      }

      unsigned int codepoint = 0;

      for ( size_t i = 0; i < Text.length(); )
      {
        char    in = Text[i];

        unsigned char ch = static_cast<unsigned char>( in );
        if ( ch <= 0x7f )
        {
          codepoint = ch;
        }
        else if ( ch <= 0xbf )
        {
          codepoint = ( codepoint << 6 ) | ( ch & 0x3f );
        }
        else if ( ch <= 0xdf )
        {
          codepoint = ch & 0x1f;
        }
        else if ( ch <= 0xef )
        {
          codepoint = ch & 0x0f;
        }
        else
        {
          codepoint = ch & 0x07;
        }
        ++i;
        if ( ( ( ( i < Text.length() )
        &&       ( ( Text[i] & 0xc0 ) != 0x80 ) )
        ||     ( i >= Text.length() ) )
        &&   ( codepoint <= 0x10ffff ) )
        {
          if ( codepoint > 0xffff )
          {
            out.append( 1, static_cast<wchar_t>( 0xd800 + ( codepoint >> 10 ) ) );
            out.append( 1, static_cast<wchar_t>( 0xdc00 + ( codepoint & 0x03ff ) ) );
          }
          else if ( ( codepoint < 0xd800 )
          ||        ( codepoint >= 0xe000 ) )
          {
            out.append( 1, static_cast<wchar_t>( codepoint ) );
          }
        }
      }
      return out;
    }



    GR::String ToUTF8( const GR::WString& Text )
    {
      GR::String    out;
      unsigned int      codepoint = 0;

      for ( size_t i = 0; i < Text.length(); ++i )
      {
        GR::WChar    in( Text[i] );

        if ( ( in >= 0xd800 )
        &&   ( in <= 0xdbff ) )
        {
          codepoint = ( ( in - 0xd800 ) << 10 ) + 0x10000;
        }
        else
        {
          if ( ( in >= 0xdc00 )
          &&   ( in <= 0xdfff ) )
          {
            codepoint |= in - 0xdc00;
          }
          else
          {
            codepoint = in;
          }

          if ( codepoint <= 0x7f )
          {
            out.append( 1, static_cast<char>( codepoint ) );
          }
          else if ( codepoint <= 0x7ff )
          {
            out.append( 1, static_cast<char>( 0xc0 | ( ( codepoint >> 6 ) & 0x1f ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( codepoint & 0x3f ) ) );
          }
          else if ( codepoint <= 0xffff )
          {
            out.append( 1, static_cast<char>( 0xe0 | ( ( codepoint >> 12 ) & 0x0f ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( ( codepoint >> 6 ) & 0x3f ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( codepoint & 0x3f ) ) );
          }
          else
          {
            out.append( 1, static_cast<char>( 0xf0 | ( ( codepoint >> 18 ) & 0x07 ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( ( codepoint >> 12 ) & 0x3f ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( ( codepoint >> 6 ) & 0x3f ) ) );
            out.append( 1, static_cast<char>( 0x80 | ( codepoint & 0x3f ) ) );
          }
          codepoint = 0;
        }
      }
      return out;
    }



    GR::WString      ToStringW( const GR::String& strText )
    {
      GR::WString      wstrResult;

      wstrResult.resize( strText.length() );

      typedef std::codecvt<wchar_t, char, mbstate_t> widecvt;

      std::locale     locGlob;

      std::locale::global( locGlob );

      const widecvt& cvt( std::use_facet<widecvt>( locGlob ) );

      mbstate_t   State;

      const char*       cTemp;
      wchar_t*    wTemp;

      cvt.in( State,
              &strText[0], &strText[0] + strText.length(), cTemp,
              (wchar_t*)&wstrResult[0], &wstrResult[0] + wstrResult.length(), wTemp );

      return wstrResult;
    }



    GR::String ToStringA( const GR::Char* Text )
    {
      return GR::String( Text );
    }



    GR::String ToStringA( const GR::String& Text )
    {
      return Text;
    }



    GR::WString ToStringW( const GR::Char* Text )
    {
      GR::String    Temp( Text );
      return ToStringW( Temp );
    }



    GR::WString ToStringW( const GR::WString& strText )
    {
      return strText;
    }



    GR::String ToString( const GR::String& Text )
    {
      return Text;
    }



    GR::String ToStringA( const GR::WString& strText )
    {
      return ToUTF8( strText );
    }



    GR::String ToString( const GR::WString& Text )
    {
      return ToUTF8( Text );
    }



    GR::String      ToStringA( GR::i64 iValue )
    {
      GR::String     strResult;

      if ( iValue == 0 )
      {
        return GR::String( "0" );
      }

      bool            bNegative = false;

      if ( iValue < 0 )
      {
        iValue = -iValue;
        bNegative = true;
      }

      GR::i64         iTemp( iValue );

      while ( iTemp )
      {
        strResult = (GR::Char)( ( iTemp % 10 ) + '0' ) + strResult;
        iTemp /= 10;
      }
      if ( bNegative )
      {
        strResult = "-" + strResult;
      }
      return strResult;
    }

    GR::WString      ToStringW( GR::i64 iValue )
    {
      GR::WString     strResult;

      if ( iValue == 0 )
      {
        return GR::WString( L"0" );
      }

      bool            bNegative = false;

      if ( iValue < 0 )
      {
        iValue = -iValue;
        bNegative = true;
      }

      GR::i64         iTemp( iValue );

      while ( iTemp )
      {
        strResult = (GR::WChar)( ( iTemp % 10 ) + '0' ) + strResult;
        iTemp /= 10;
      }
      if ( bNegative )
      {
        strResult = L"-" + strResult;
      }
      return strResult;
    }



    GR::String ToString( GR::i64 iValue )
    {
      return ToStringA( iValue );
    }



    GR::String      F64ToStringA( GR::f64 fValue )
    {
      GR::String   strResult;

      GR::f64       dblValue = fValue;

      GR::i64       iValue = (GR::i64)dblValue;

      if ( iValue < 0 )
      {
        iValue = -iValue;
        dblValue = -dblValue;
      }
      if ( iValue == 0 )
      {
        strResult = "0";
      }
      while ( iValue > 0 )
      {
        strResult = (GR::Char)( ( iValue % 10 ) + '0' ) + strResult;
        iValue /= 10;
      }
      if ( fValue < 0 )
      {
        strResult = '-' + strResult;
      }

      strResult += ".";

      if ( dblValue < 0.0 )
      {
        dblValue = -dblValue;
      }

      while ( ( dblValue < 1.0 )
      &&      ( dblValue > 0.0 ) )
      {
        dblValue *= 10;

        strResult += (char)( (int)fmod( dblValue, 10 ) + '0' );
      }

      return strResult;
    }

    GR::WString      F64ToStringW( GR::f64 fValue )
    {
      GR::WString   strResult;

      GR::f64       dblValue = fValue;

      GR::i64       iValue = (GR::i64)dblValue;

      if ( iValue < 0 )
      {
        iValue = -iValue;
        dblValue = -dblValue;
      }
      if ( iValue == 0 )
      {
        strResult = L"0";
      }
      while ( iValue > 0 )
      {
        strResult = (GR::WChar)( ( iValue % 10 ) + '0' ) + strResult;
        iValue /= 10;
      }
      if ( fValue < 0 )
      {
        strResult = L'-' + strResult;
      }

      strResult += L".";

      if ( dblValue < 0.0 )
      {
        dblValue = -dblValue;
      }

      while ( ( dblValue < 1.0 )
      &&      ( dblValue > 0.0 ) )
      {
        dblValue *= 10;

        strResult += (char)( (int)fmod( dblValue, 10 ) + '0' );
      }

      return strResult;
    }



    GR::String F64ToString( GR::f64 iValue )
    {
      return F64ToStringA( iValue );
    }



    GR::String      ToHexStringA( const void* pData, size_t dwDataSize )
    {
      GR::String   strResult;

      GR::u8*   pBData = (GR::u8*)pData;

      for ( size_t i = 0; i < dwDataSize; ++i )
      {
        GR::u8    ucNibble = (GR::u8)( ( *pBData & 0xf0 ) >> 4 );

        if ( ucNibble >= 10 )
        {
          strResult += (GR::Char)( 'A' + ucNibble - 10 );
        }
        else
        {
          strResult += (GR::Char)( '0' + ucNibble );
        }

        ucNibble = (GR::u8)( *pBData & 0x0f );
        if ( ucNibble >= 10 )
        {
          strResult += (GR::Char)( 'A' + ucNibble - 10 );
        }
        else
        {
          strResult += (GR::Char)( '0' + ucNibble );
        }
        pBData++;
      }
      return strResult;
    }

    GR::WString      ToHexStringW( const void* pData, size_t dwDataSize )
    {
      GR::WString   strResult;

      GR::u8*   pBData = (GR::u8*)pData;

      for ( size_t i = 0; i < dwDataSize; ++i )
      {
        GR::u8    ucNibble = (GR::u8)( ( *pBData & 0xf0 ) >> 4 );

        if ( ucNibble >= 10 )
        {
          strResult += (GR::WChar)( L'A' + ucNibble - 10 );
        }
        else
        {
          strResult += (GR::WChar)( L'0' + ucNibble );
        }

        ucNibble = (GR::u8)( ( *pBData & 0x0f ) );
        if ( ucNibble >= 10 )
        {
          strResult += (GR::WChar)( L'A' + ucNibble - 10 );
        }
        else
        {
          strResult += (GR::WChar)( L'0' + ucNibble );
        }
        pBData++;
      }
      return strResult;
    }



    GR::String ToHexString( const void* pData, size_t dwDataSize )
    {
      return ToHexStringA( pData, dwDataSize );
    }



    GR::String ToHex( GR::u64 Value, const GR::u32 Digits )
    {
      GR::String    result;

      do
      {
        GR::u8    nibble = (GR::u8)( Value & 0x0f );
        if ( nibble >= 10 )
        {
          result = (GR::Char)( 'A' + nibble - 10 ) + result;
        }
        else
        {
          result = (GR::Char)( '0' + nibble ) + result;
        }

        nibble = (GR::u8)( ( Value >> 4 ) & 0x0f );
        if ( nibble >= 10 )
        {
          result = (GR::Char)( 'A' + nibble - 10 ) + result;
        }
        else
        {
          result = (GR::Char)( '0' + nibble ) + result;
        }
        Value >>= 8;
      }
      while ( Value != 0 );

      if ( Digits > 0 )
      {
        while ( result.length() < Digits )
        {
          result = "0" + result;
        }
      }
      else
      {
        while ( ( !result.empty() )
        &&      ( result[0] == '0' ) )
        {
          result = result.substr( 1 );
        }
      }
      if ( result.empty() )
      {
        return "0";
      }
      return result;
    }



    GR::WString ToHexW( GR::u64 Value, const GR::u32 Digits )
    {
      GR::WString    result;

      do
      {
        GR::u8    nibble = (GR::u8)( Value & 0x0f );
        if ( nibble >= 10 )
        {
          result = (GR::WChar)( L'A' + nibble - 10 ) + result;
        }
        else
        {
          result = (GR::WChar)( L'0' + nibble ) + result;
        }

        nibble = (GR::u8)( ( Value >> 4 ) & 0x0f );
        if ( nibble >= 10 )
        {
          result = (GR::WChar)( L'A' + nibble - 10 ) + result;
        }
        else
        {
          result = (GR::WChar)( L'0' + nibble ) + result;
        }
        Value >>= 8;
      }
      while ( Value != 0 );

      if ( Digits > 0 )
      {
        while ( result.length() < Digits )
        {
          result = L"0" + result;
        }
      }
      else
      {
        while ( ( !result.empty() )
        &&      ( result[0] == L'0' ) )
        {
          result = result.substr( 1 );
        }
      }
      if ( result.empty() )
      {
        return L"0";
      }
      return result;
    }



    GR::f64 ToF64( const GR::String& strValue )
    {
      GR::f64     fResult   = 0.0;
      GR::f64     fPower    = 0.1;
      GR::f64     fExponent = 0.0;

      bool      bBeforeDot = true;
      bool      bExponent = false;

      bool      bNegative = false;

      size_t    iPos = 0;

      while ( iPos < strValue.length() )
      {
        GR::Char cChar = strValue[iPos];

        if ( cChar == '-' )
        {
          bNegative = !bNegative;
        }
        else if ( ( cChar == 'e' )
        ||        ( cChar == 'E' ) )
        {
          bExponent = true;
        }
        else if ( ( cChar >= '0' )
        &&        ( cChar <= '9' ) )
        {
          if ( bExponent )
          {
            fExponent *= 10.0;
            fExponent += (float)( cChar - '0' );
          }
          else if ( bBeforeDot )
          {
            fResult *= 10.0;
            fResult += (float)( cChar - '0' );
          }
          else
          {
            fResult += (float)( cChar - '0' ) * fPower;
            fPower *= 0.1;
          }
        }
        else if ( cChar == '.' )
        {
          bBeforeDot = false;
        }
        else if ( cChar != ' ' )
        {
          // Fehler, Abbruch
          break;
        }
        ++iPos;
      }
      if ( bExponent )
      {
        fResult *= fExponent;
      }
      if ( bNegative )
      {
        fResult = -fResult;
      }
      return fResult;
    }

    GR::f64 ToF64( const GR::WString& strValue )
    {
      GR::f64     fResult   = 0.0;
      GR::f64     fPower    = 0.1;
      GR::f64     fExponent = 0.0;

      bool      bBeforeDot = true;
      bool      bExponent = false;

      bool      bNegative = false;

      size_t    iPos = 0;

      while ( iPos < strValue.length() )
      {
        GR::WChar cChar = strValue[iPos];

        if ( cChar == L'-' )
        {
          bNegative = !bNegative;
        }
        else if ( ( cChar == L'e' )
        ||        ( cChar == L'E' ) )
        {
          bExponent = true;
        }
        else if ( ( cChar >= L'0' )
        &&        ( cChar <= L'9' ) )
        {
          if ( bExponent )
          {
            fExponent *= 10.0;
            fExponent += (float)( cChar - L'0' );
          }
          else if ( bBeforeDot )
          {
            fResult *= 10.0;
            fResult += (float)( cChar - L'0' );
          }
          else
          {
            fResult += (float)( cChar - L'0' ) * fPower;
            fPower *= 0.1;
          }
        }
        else if ( cChar == L'.' )
        {
          bBeforeDot = false;
        }
        else if ( cChar != L' ' )
        {
          // Fehler, Abbruch
          break;
        }
        ++iPos;
      }
      if ( bExponent )
      {
        fResult *= fExponent;
      }
      if ( bNegative )
      {
        fResult = -fResult;
      }
      return fResult;
    }

    GR::f32 ToF32( const GR::String& strValue )
    {
      return (GR::f32)ToF64( strValue );
    }

    GR::f32 ToF32( const GR::WString& strValue )
    {
      return (GR::f32)ToF64( strValue );
    }

    GR::u64          ToU64( const GR::String& strValue, GR::ip iBase )
    {
      GR::u64   dwValue = 0;

      size_t  iPos = 0;

      if ( ( strValue.length() >= 2 )
      &&   ( strValue.substr( 0, 2 ) == "0x" ) )
      {
        iPos = 2;
        iBase = 16;
      }
      if ( ( strValue.length() >= 1 )
      &&   ( strValue[0] == '#' ) )
      {
        iPos = 1;
        iBase = 16;
      }

      // Space darf am Anfang sein
      while ( ( iPos < strValue.length() )
      &&      ( strValue[iPos] == ' ' ) )
      {
        ++iPos;
      }

      if ( iPos < strValue.length() )
      {
        GR::Char   cChar = strValue[iPos];

        if ( ( ( cChar < '0' )
        ||     ( cChar > '9' ) )
        &&   ( cChar != '-' ) )
        {
          // ungültiges Zeichen zum Anfang
          if ( ( iBase == 16 )
          &&   ( ( ( cChar >= 'A' )
          &&       ( cChar <= 'F' ) )
          ||     ( ( cChar >= 'a' )
          &&       ( cChar <= 'f' ) ) ) )
          {
          }
          else
          {
            return 0;
          }
        }
      }

      if ( iBase == 16 )
      {
        // hexadezimal
        while ( iPos < strValue.length() )
        {
          dwValue <<= 4;
          GR::Char   cChar = strValue[iPos];

          // avoided use of toupper

          if ( ( cChar >= '0' )
          &&   ( cChar <= '9' ) )
          {
            dwValue += cChar - '0';
          }
          else if ( ( cChar >= 'a' )
          &&        ( cChar <= 'f' ) )
          {
            dwValue += cChar + 10 - 'a';
          }
          else if ( ( cChar >= 'A' )
          &&        ( cChar <= 'F' ) )
          {
            dwValue += cChar + 10 - 'A';
          }
          else
          {
            // ungültiges Zeichen
            dwValue >>= 4;
            break;
          }
          ++iPos;
        }
        return dwValue;
      }

      bool    bNegative = false;

      while ( iPos < strValue.length() )
      {
        dwValue *= 10;
        GR::Char   cChar = strValue[iPos];

        // avoided use of toupper
        if ( ( iPos == 0 )
        &&   ( cChar == '-' ) )
        {
          bNegative = true;
        }

        if ( ( cChar >= '0' )
        &&   ( cChar <= '9' ) )
        {
          dwValue += cChar - '0';
        }
        else if ( cChar == '-' )
        {
        }
        else
        {
          // ungültiges Zeichen
          dwValue /= 10;
          break;
        }
        ++iPos;
      }
      return bNegative ? ( -(GR::i64)dwValue ) : dwValue;
    }



    GR::u64          ToU64( const GR::WString& strValue, GR::ip iBase )
    {
      GR::u64   dwValue = 0;

      size_t  iPos = 0;

      if ( ( strValue.length() >= 2 )
      &&   ( strValue.substr( 0, 2 ) == L"0x" ) )
      {
        iPos = 2;
        iBase = 16;
      }
      if ( ( strValue.length() >= 1 )
      &&   ( strValue[0] == L'#' ) )
      {
        iPos = 1;
        iBase = 16;
      }

      // Space darf am Anfang sein
      while ( ( iPos < strValue.length() )
      &&      ( strValue[iPos] == L' ' ) )
      {
        ++iPos;
      }

      if ( iPos < strValue.length() )
      {
        GR::WChar   cChar = strValue[iPos];

        if ( ( ( cChar < L'0' )
        ||     ( cChar > L'9' ) )
        &&   ( cChar != L'-' ) )
        {
          // ungültiges Zeichen zum Anfang
          if ( ( iBase == 16 )
          &&   ( ( ( cChar >= L'A' )
          &&       ( cChar <= L'F' ) )
          ||     ( ( cChar >= L'a' )
          &&       ( cChar <= L'f' ) ) ) )
          {
          }
          else
          {
            return 0;
          }
        }
      }

      if ( iBase == 16 )
      {
        // hexadezimal
        while ( iPos < strValue.length() )
        {
          dwValue <<= 4;
          GR::WChar   cChar = strValue[iPos];

          // avoided use of toupper

          if ( ( cChar >= L'0' )
          &&   ( cChar <= L'9' ) )
          {
            dwValue += cChar - L'0';
          }
          else if ( ( cChar >= L'a' )
          &&        ( cChar <= L'f' ) )
          {
            dwValue += cChar + 10 - L'a';
          }
          else if ( ( cChar >= L'A' )
          &&        ( cChar <= L'F' ) )
          {
            dwValue += cChar + 10 - L'A';
          }
          else
          {
            // ungültiges Zeichen
            dwValue >>= 4;
            break;
          }
          ++iPos;
        }
        return dwValue;
      }

      bool    bNegative = false;

      while ( iPos < strValue.length() )
      {
        dwValue *= 10;
        GR::WChar   cChar = strValue[iPos];

        // avoided use of toupper
        if ( ( iPos == 0 )
        &&   ( cChar == L'-' ) )
        {
          bNegative = true;
        }

        if ( ( cChar >= L'0' )
        &&   ( cChar <= L'9' ) )
        {
          dwValue += cChar - L'0';
        }
        else if ( cChar == '-' )
        {
        }
        else
        {
          // ungültiges Zeichen
          dwValue /= 10;
          break;
        }
        ++iPos;
      }
      return bNegative ? ( -(GR::i64)dwValue ) : dwValue;
    }

    GR::u8          ToU8( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::u8)ToU64( strValue, iBase );
    }

    GR::u8          ToU8( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::u8)ToU64( strValue, iBase );
    }

    GR::u16          ToU16( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::u16)ToU64( strValue, iBase );
    }

    GR::u16          ToU16( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::u16)ToU64( strValue, iBase );
    }

    GR::i16          ToI16( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::i16)ToI64( strValue, iBase );
    }

    GR::i16          ToI16( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::i16)ToI64( strValue, iBase );
    }

    GR::u32          ToU32( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::u32)ToU64( strValue, iBase );
    }

    GR::u32          ToU32( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::u32)ToU64( strValue, iBase );
    }

    GR::up          ToUP( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::up)ToU64( strValue, iBase );
    }
  
    GR::up          ToUP( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::up)ToU64( strValue, iBase );
    }
  
    GR::i64          ToI64( const GR::String& strValue, GR::ip iBase )
    {
      GR::i64   dwValue = 0;

      size_t  iPos = 0;

      if ( ( strValue.length() >= 2 )
      &&   ( strValue.substr( 0, 2 ) == "0x" ) )
      {
        iPos = 2;
        iBase = 16;
      }
      if ( ( strValue.length() >= 1 )
      &&   ( strValue[0] == '#' ) )
      {
        iPos = 1;
        iBase = 16;
      }

      // Space darf am Anfang sein
      while ( ( iPos < strValue.length() )
      &&      ( strValue[iPos] == ' ' ) )
      {
        ++iPos;
      }


      if ( iBase == 16 )
      {
        // hexadezimal
        while ( iPos < strValue.length() )
        {
          dwValue <<= 4;
          GR::Char cChar = strValue[iPos];

          // avoided use of toupper

          if ( ( cChar >= '0' )
          &&   ( cChar <= '9' ) )
          {
            dwValue += cChar - '0';
          }
          else if ( ( cChar >= 'a' )
          &&        ( cChar <= 'f' ) )
          {
            dwValue += cChar + 10 - 'a';
          }
          else if ( ( cChar >= 'A' )
          &&        ( cChar <= 'F' ) )
          {
            dwValue += cChar + 10 - 'A';
          }
          else
          {
            dwValue >>= 4;
            break;
          }
          ++iPos;
        }
        return dwValue;
      }

      bool    bNegative = false;

      while ( iPos < strValue.length() )
      {
        GR::Char cChar = strValue[iPos];

        // avoided use of toupper
        if ( ( iPos == 0 )
        &&   ( cChar == '-' ) )
        {
          bNegative = true;
        }
        else if ( ( iPos == 0 )
        &&        ( cChar == '+' ) )
        {
        }
        else if ( ( cChar >= '0' )
        &&        ( cChar <= '9' ) )
        {
          dwValue *= 10;
          dwValue += cChar - '0';
        }
        else
        {
          break;
        }
        ++iPos;
      }
      return bNegative ? ( -(GR::i64)dwValue ) : dwValue;
    }

    GR::i64          ToI64( const GR::WString& strValue, GR::ip iBase )
    {
      GR::i64   dwValue = 0;

      size_t  iPos = 0;

      if ( ( strValue.length() >= 2 )
      &&   ( strValue.substr( 0, 2 ) == L"0x" ) )
      {
        iPos = 2;
        iBase = 16;
      }
      if ( ( strValue.length() >= 1 )
      &&   ( strValue[0] == L'#' ) )
      {
        iPos = 1;
        iBase = 16;
      }

      // Space darf am Anfang sein
      while ( ( iPos < strValue.length() )
      &&      ( strValue[iPos] == L' ' ) )
      {
        ++iPos;
      }


      if ( iBase == 16 )
      {
        // hexadezimal
        while ( iPos < strValue.length() )
        {
          dwValue <<= 4;
          GR::WChar cChar = strValue[iPos];

          // avoided use of toupper

          if ( ( cChar >= L'0' )
          &&   ( cChar <= L'9' ) )
          {
            dwValue += cChar - L'0';
          }
          else if ( ( cChar >= L'a' )
          &&        ( cChar <= L'f' ) )
          {
            dwValue += cChar + 10 - L'a';
          }
          else if ( ( cChar >= L'A' )
          &&        ( cChar <= L'F' ) )
          {
            dwValue += cChar + 10 - L'A';
          }
          else
          {
            dwValue >>= 4;
            break;
          }
          ++iPos;
        }
        return dwValue;
      }

      bool    bNegative = false;

      while ( iPos < strValue.length() )
      {
        dwValue *= 10;
        GR::WChar cChar = strValue[iPos];

        // avoided use of toupper
        if ( ( iPos == 0 )
        &&   ( cChar == L'-' ) )
        {
          bNegative = true;
        }
        else if ( ( cChar >= L'0' )
        &&        ( cChar <= L'9' ) )
        {
          dwValue += cChar - L'0';
        }
        else
        {
          dwValue /= 10;
          break;
        }
        ++iPos;
      }
      return bNegative ? ( -(GR::i64)dwValue ) : dwValue;
    }

    GR::ip          ToIP( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::ip)ToI64( strValue, iBase );
    }

    GR::ip          ToIP( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::ip)ToI64( strValue, iBase );
    }

    GR::i32          ToI32( const GR::String& strValue, GR::ip iBase )
    {
      return (GR::i32)ToI64( strValue, iBase );
    }

    GR::i32          ToI32( const GR::WString& strValue, GR::ip iBase )
    {
      return (GR::i32)ToI64( strValue, iBase );
    }



    bool ToBool( const GR::String& Value )
    {
      GR::String    upper( Value );

      upper = upper.ToUpper();

      if ( ( upper == "Y" )
      ||   ( upper == "YES" )
      ||   ( upper == "TRUE" )
      ||   ( upper == "1" )
      ||   ( upper == "J" ) )
      {
        return true;
      }
      return false;
    }



    bool ToBool( const GR::WString& Value )
    {
      return ToBool( ToUTF8( Value ) );
    }



    ByteBuffer ToBCD( const GR::String& Value, size_t NumDigits, bool FixedLength )
    {
      bool  swallowError = false;
      return ToBCD( Value, NumDigits, FixedLength, swallowError );
    }



    ByteBuffer ToBCD( const GR::String& Value, size_t NumDigits, bool FixedLength, bool& HadError )
    {
      size_t          trueLength = Value.length();

      if ( FixedLength )
      {
        trueLength = NumDigits;
      }
      if ( trueLength > NumDigits )
      {
        trueLength = NumDigits;
      }
      if ( trueLength & 1 )
      {
        ++trueLength;
      }

      GR::String   temp = Value;

      if ( FixedLength )
      {
        // bei FixedLength wird links mit Nullen aufgef?llt
        while ( temp.length() < NumDigits )
        {
          temp = "0" + temp;
        }
      }

      ByteBuffer     bbBCD;

      bbBCD.Reserve( trueLength / 2 );

      for ( size_t i = 0; i < trueLength / 2; ++i )
      {
        GR::u8      ucValue = 0;

        char    nibble = temp[i * 2];

        if ( !GR::Strings::IsValidHex( nibble ) )
        {
          HadError = true;
          return bbBCD;
        }
        ucValue = GR::Convert::ToU8( temp.substr( i * 2, 1 ), 16 ) << 4;
        if ( i * 2 + 1 >= temp.length() )
        {
          ucValue += 0x0f;
        }
        else
        {
          nibble = temp[i * 2 + 1];
          if ( !GR::Strings::IsValidHex( nibble ) )
          {
            HadError = true;
            return bbBCD;
          }
          ucValue += GR::Convert::ToU8( temp.substr( i * 2 + 1, 1 ), 16 );
        }
        bbBCD.AppendU8( ucValue );
      }
      return bbBCD;
    }



    ByteBuffer ToBCD( GR::i64 Value, size_t Length, bool FixedLength )
    {
      GR::String     value = GR::Convert::ToStringA( Value );

      return ToBCD( value, Length, FixedLength );
    }



    GR::String BCDToString( const ByteBuffer& Data )
    {
      return BCDToString( ( GR::u8* )Data.Data(), Data.Size() );
    }



    GR::String BCDToString( const GR::u8* pData, size_t Length )
    {
      GR::String     result;

      if ( pData == NULL )
      {
        return result;
      }

      for ( size_t i = 0; i < Length; ++i )
      {
        char    upperNibble = (char)( '0' + ( ( pData[i] & 0xf0 ) >> 4 ) );
        char    lowerNibble = (char)( '0' + ( pData[i] & 0x0f ) );

        result += upperNibble;
        result += lowerNibble;
      }

      return result;
    }



    GR::i64 BCDToValue( const ByteBuffer& Data, bool NegativeAllowed )
    {
      return BCDToValue( ( GR::u8* )Data.Data(), Data.Size(), NegativeAllowed );
    }



    GR::i64 BCDToValue( const GR::u8* pData, size_t Length, bool NegativeAllowed )
    {
      if ( pData == NULL )
      {
        return 0;
      }

      GR::i64     result = 0;
      for ( size_t i = 0; i < Length; ++i )
      {
        if ( ( i == 0 )
        &&   ( ( pData[0] & 0xf0 ) == 0xD0 ) )
        {
          // allow negative sign
        }
        else
        {
          result *= 10;
          result += ( ( pData[i] & 0xf0 ) >> 4 );
        }
        result *= 10;
        result += ( pData[i] & 0xf );
      }
      if ( ( NegativeAllowed )
      &&   ( ( pData[0] & 0xf0 ) == 0xD0 ) )
      {
        result = -result;
      }
      return result;
    }

  }

}
