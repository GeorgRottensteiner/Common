#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <list>
#include <vector>
#include <locale>

#include <GR/GRTypes.h>

#include <Interface/IFont.h>

#include <OS/OS.h>

#include <debug/debugclient.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <tchar.h>
#endif

#if OPERATING_SYSTEM == OS_TANDEM
#pragma nowarn (262,1506)
#endif


namespace GR
{

  namespace Strings
  {
    // forward declaration
    static GR::Char ToUpper( GR::Char Char );
    static GR::String ToUpper( const GR::String& String );
    static GR::Char ToLower( GR::Char Char );
    static GR::String ToLower( const GR::String& String );



    static bool StartsWith( const GR::String& Source, const GR::String& SearchString )
    {
      if ( SearchString.length() > Source.length() )
      {
        return false;
      }
      return Source.compare( 0, SearchString.length(), SearchString ) == 0;
    }



    static bool StartsWith( const GR::WString& Source, const GR::WString& SearchString )
    {
      if ( SearchString.length() > Source.length() )
      {
        return false;
      }
      return Source.compare( 0, SearchString.length(), SearchString ) == 0;
    }



    static bool EndsWith( const GR::String& Source, const GR::String& SearchString )
    {
      if ( SearchString.length() > Source.length() )
      {
        return false;
      }
      return Source.compare( Source.length() - SearchString.length(), SearchString.length(), SearchString ) == 0;
    }



    static bool EndsWith( const GR::WString& Source, const GR::WString& SearchString )
    {
      if ( SearchString.length() > Source.length() )
      {
        return false;
      }
      return Source.compare( Source.length() - SearchString.length(), SearchString.length(), SearchString ) == 0;
    }



    static int CompareCaseInsensitive( const GR::String& String1, const GR::String& String2 )
    {
      GR::String str1 = GR::Strings::ToLower( GR::Strings::ToUpper( String1 ) );
      GR::String str2 = GR::Strings::ToLower( GR::Strings::ToUpper( String2 ) );

      return str1.compare( str2 );
    }



    static void Split( const GR::String& Source, const char Separator, std::list<GR::String>& Parts )
    {
      size_t      pos = 0;
      size_t      startPos = 0;

      while ( ( pos = Source.find( Separator, startPos ) ) != GR::String::npos )
      {
        Parts.push_back( Source.substr( startPos, pos - startPos ) );
        startPos = pos + 1;
      }
      if ( ( Source.length() >= startPos )
      &&   ( !Source.empty() ) )
      {
        Parts.push_back( Source.substr( startPos, Source.length() - startPos ) );
      }
    }



    static void Split( const GR::String& Source, const char Separator, std::vector<GR::String>& Parts )
    {
      size_t      pos = 0;
      size_t      startPos = 0;

      while ( ( pos = Source.find( Separator, startPos ) ) != GR::String::npos )
      {
        Parts.push_back( Source.substr( startPos, pos - startPos ) );
        startPos = pos + 1;
      }
      if ( ( Source.length() >= startPos )
      &&   ( !Source.empty() ) )
      {
        Parts.push_back( Source.substr( startPos, Source.length() - startPos ) );
      }
    }



    static GR::String Join( const std::list<GR::String>& Parts, const GR::Char Separator )
    {
      GR::String    result;

      std::list<GR::String>::const_iterator   it( Parts.begin() );
      while ( it != Parts.end() )
      {
        if ( it != Parts.begin() )
        {
          result += Separator;
        }
        result += *it;
        ++it;
      }
      return result;
    }



    static GR::String Fill( const GR::Char FillChar, GR::u32 FillCount )
    {
      GR::String   result;

      while ( (GR::u32)result.length() < FillCount )
      {
        result += FillChar;
      }
      return result;
    }



    static GR::String TrimLeft( const GR::String& ToTrim, const GR::Char TrimChar = ' ' )
    {
      size_t    pos = ToTrim.find_first_not_of( TrimChar );

      if ( pos == GR::String::npos )
      {
        if ( ( !ToTrim.empty() )
        &&   ( ToTrim[0] == TrimChar ) )
        {
          // der String besteht nur aus dem Trimm-Zeichen
          return GR::String();
        }
        // der String ist sauber
        return ToTrim;
      }
      return ToTrim.substr( pos );
    }



    static GR::String TrimRight( const GR::String& ToTrim, const GR::Char TrimChar = ' ' )
    {
      size_t    pos = ToTrim.find_last_not_of( TrimChar );

      if ( pos == GR::String::npos )
      {
        if ( ( !ToTrim.empty() )
        &&   ( ToTrim[0] == TrimChar ) )
        {
          // der String besteht nur aus dem Trimm-Zeichen
          return GR::String();
        }
        // der String ist sauber
        return ToTrim;
      }
      return ToTrim.substr( 0, pos + 1 );
    }



    static GR::String TrimRight( const GR::String& ToTrim, const GR::String TrimString )
    {
      GR::String     result = ToTrim;

      while ( result.length() >= TrimString.length() )
      {
        if ( result.compare( result.length() - TrimString.length(), TrimString.length(), TrimString ) == 0 )
        {
          result = result.substr( 0, result.length() - TrimString.length() );
        }
        else
        {
          return result;
        }
      }
      return result;
    }



    static GR::String Trim( const GR::String& ToTrim, const GR::Char TrimChar = ' ' )
    {
      size_t    startPos  = ToTrim.find_first_not_of( TrimChar );
      size_t    pos       = ToTrim.find_last_not_of( TrimChar );

      if ( ( startPos == GR::String::npos )
      &&   ( pos == GR::String::npos ) )
      {
        if ( ( !ToTrim.empty() )
        &&   ( ToTrim[0] == TrimChar ) )
        {
          // der String besteht nur aus dem Trimm-Zeichen
          return GR::String();
        }
        // der String ist sauber
        return ToTrim;
      }
      return ToTrim.substr( startPos, pos - startPos + 1 );
    }




    static GR::String Remove( const GR::String& ToTrim, const GR::Char TrimChar = ' ' )
    {
      GR::String   result;

      result.reserve( ToTrim.length() );

      for ( size_t i = 0; i < ToTrim.length(); ++i )
      {
        if ( ToTrim[i] != TrimChar )
        {
          result += ToTrim[i];
        }
      }
      return result;
    }



    static GR::String Remove( const GR::String& ToTrim, const GR::String& CharsToRemove )
    {
      GR::String   result;

      result.reserve( ToTrim.length() );

      for ( size_t i = 0; i < ToTrim.length(); ++i )
      {
        if ( CharsToRemove.find( ToTrim[i] ) == GR::String::npos )
        {
          result += ToTrim[i];
        }
      }
      return result;
    }



    static GR::String Replace( const GR::String& ToTrim, const GR::Char ToRemove, const GR::Char ReplaceWith )
    {
      GR::String   result( ToTrim );

      for ( size_t i = 0; i < result.length(); ++i )
      {
        if ( result[i] == ToRemove )
        {
          result[i] = ReplaceWith;
        }
      }
      return result;
    }



    static GR::String Replace( const GR::String& ToTrim, const GR::String& ToRemove, const GR::String& ReplaceWith )
    {
      GR::String  result;
      size_t          lastFindPos = 0;
      size_t          findPos = GR::String::npos;


      while ( ( findPos = ToTrim.find( ToRemove, lastFindPos ) ) != GR::String::npos )
      {
        if ( findPos > lastFindPos )
        {
          // Ein Schnipsel vor dem Fundort
          result += ToTrim.substr( lastFindPos, findPos - lastFindPos );
        }
        result += ReplaceWith;
        lastFindPos = findPos + ToRemove.length();
      }
      if ( lastFindPos < ToTrim.length() )
      {
        result += ToTrim.substr( lastFindPos );
      }
      return result;
    }



    static size_t FindUpperCaseCompare( const GR::String& Text, const GR::String& Find, size_t Offset = 0 )
    {
      if ( Find.length() > Offset + Text.length() )
      {
        // cannot fit
        return (size_t)-1;
      }

      size_t    curPos = Offset;

      while ( curPos <= Text.length() - Find.length() )
      {
        size_t    FindCount = 0;
        while ( ToUpper( Text[curPos + FindCount] ) == ToUpper( Find[FindCount] ) )
        {
          ++FindCount;
          if ( FindCount == Find.length() )
          {
            return curPos;
          }
        }
        ++curPos;
      }
      return (size_t)-1;
    }



    static GR::String ReplaceWithUpperCaseCompare( const GR::String& ToTrim, const GR::String& ToRemove, const GR::String& ToReplaceWith )
    {
      GR::String   result;

      size_t    lastFindPos = 0;
      size_t    findPos = GR::String::npos;

      while ( ( findPos = FindUpperCaseCompare( ToTrim, ToRemove, lastFindPos ) ) != GR::String::npos )
      {
        if ( findPos > lastFindPos )
        {
          // Ein Schnipsel vor dem Fundort
          result += ToTrim.substr( lastFindPos, findPos - lastFindPos );
        }
        result += ToReplaceWith;

        lastFindPos = findPos + ToRemove.length();
      }
      if ( lastFindPos < ToTrim.length() )
      {
        result += ToTrim.substr( lastFindPos );
      }
      return result;
    }



    static void WrapText( Interface::IFont* pFont, const GR::String& Text, GR::tRect& Area, std::vector<GR::String>& Lines, int AlternativeTextHeight = 0 )
    {
      Lines.clear();
      if ( pFont == NULL )
      {
        return;
      }

      // jetzt Text schnippeln
      GR::String      workText = Text;

      GR::String      newLine;

      size_t          pos = (size_t)-1;

      do
      {
        newLine.erase();

        ++pos;
        if ( pos >= workText.length() )
        {
          Lines.push_back( workText );
          break;
        }
        if ( ( workText[pos] == '\\' )   // erzwungener Umbruch
        &&   ( pos < workText.length() )
        &&   ( workText[pos + 1] == 'n' ) )
        {
          Lines.push_back( workText.substr( 0, pos ) );
          workText = workText.substr( pos + 2 );
          pos = (size_t)-1;
        }
        else if ( workText[pos] == '\n' )   // erzwungener Umbruch
        {
          Lines.push_back( workText.substr( 0, pos ) );
          workText = workText.substr( pos + 1 );
          pos = (size_t)-1;
        }
        else if ( pFont->TextLength( workText.substr( 0, pos ) ) >= Area.width() )
        {
          if ( workText[pos] != '\n' )
          {
            // hier muﬂ noch eine passende Stelle (Leerzeichen) gefunden werden
            size_t   newPos = pos;
            do
            {
              if ( newPos == 0 )
              {
                newPos = pos;
                break;
              }
              newPos--;
              if ( newPos <= 0 )
              {
                newPos = pos;
                break;
              }
            }
            while ( workText[newPos] != ' ' );
            pos = newPos;
          }
          Lines.push_back( workText.substr( 0, pos ) );
          workText = workText.substr( pos + 1 );
          pos = (size_t)-1;
        }
      }
      while ( !workText.empty() );

      GR::i32   maxWidth = 0;

      for ( size_t i = 0; i < Lines.size(); ++i )
      {
        int     textLength = pFont->TextLength( Lines[i] );
        if ( textLength > maxWidth )
        {
          maxWidth = textLength;
        }
      }

      Area.width( maxWidth );
      if ( AlternativeTextHeight == 0 )
      {
        Area.height( pFont->TextHeight() * (int)Lines.size() );
      }
      else
      {
        Area.height( AlternativeTextHeight * (int)Lines.size() );
      }
    }



    static GR::Char ToUpper( GR::Char Char )
    {
      return std::toupper( Char, std::locale::classic() );
    }



    static GR::String ToUpper( const GR::String& String )
    {
      GR::String     Result( String );

      for ( size_t i = 0; i < Result.length(); ++i )
      {
        Result[i] = std::toupper( Result[i], std::locale::classic() );
      }
      return Result;
    }



    static GR::Char ToLower( GR::Char Char )
    {
      return std::tolower( Char, std::locale::classic() );
    }



    static GR::String ToLower( const GR::String& String )
    {
      GR::String     Result( String );

      for ( size_t i = 0; i < Result.length(); ++i )
      {
        Result[i] = std::tolower( Result[i], std::locale::classic() );
      }
      return Result;
    }



    static GR::WChar ToUpper( GR::WChar Char )
    {
      return std::toupper( Char, std::locale::classic() );
    }



    static GR::WString ToUpper( const GR::WString& String )
    {
      GR::WString     Result( String );

      for ( size_t i = 0; i < Result.length(); ++i )
      {
        Result[i] = std::toupper( Result[i], std::locale::classic() );
      }
      return Result;
    }



    static GR::WChar ToLower( GR::WChar Char )
    {
      return std::tolower( Char, std::locale::classic() );
    }



    static GR::WString ToLower( const GR::WString& String )
    {
      GR::WString     Result( String );

      for ( size_t i = 0; i < Result.length(); ++i )
      {
        Result[i] = std::tolower( Result[i], std::locale::classic() );
      }
      return Result;
    }



    static int CompareUpperCase( const GR::String& String1, const GR::String& String2 )
    {
      size_t    pos = 0;

      while ( ( pos < String1.length() )
      &&      ( pos < String2.length() ) )
      {
        GR::Char     char1 = std::toupper( String1[pos], std::locale::classic() );
        GR::Char     char2 = std::toupper( String2[pos], std::locale::classic() );

        if ( char1 < char2 )
        {
          return -1;
        }
        else if ( char1 > char2 )
        {
          return 1;
        }

        ++pos;
      }

      if ( String1.length() < String2.length() )
      {
        return -1;
      }
      else if ( String1.length() > String2.length() )
      {
        return 1;
      }
      return 0;
    }



    static size_t Length( const GR::Char* String )
    {
      // TODO - UTF8 length vs chars
      return strlen( String );
    }



    static size_t Size( const GR::String Text )
    {
      return Text.length();
    }



    static GR::String PadLeft( const GR::String& Text, size_t Digits, GR::Char FillChar )
    {
      GR::String  result = Text;
      while ( result.length() < Digits )
      {
        result = FillChar + result;
      }
      return result;
    }



    static GR::String PadRight( const GR::String& Text, size_t Digits, GR::Char FillChar )
    {
      GR::String  result = Text;
      while ( result.length() < Digits )
      {
        result += FillChar;
      }
      return result;
    }



    static bool ContainsOneNotOf( const GR::String& Text, const GR::String& SearchChars )
    {
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        if ( SearchChars.find( Text[i] ) == GR::String::npos )
        {
          return true;
        }
      }
      return false;
    }



    static bool IsNumeric( const GR::String& Text, size_t Offset = 0, size_t Length = (size_t)-1 )
    {
      if ( Offset >= Text.length() )
      {
        return false;
      }
      if ( Length == (size_t)-1 )
      {
        Length = Text.length() - Offset;
      }
      if ( Offset + Length > Text.length() )
      {
        return false;
      }
      for ( size_t i = Offset; i < Length; ++i )
      {
        char    digit( Text[i] );

        if ( ( digit < '0' )
        ||   ( digit > '9' ) )
        {
          return false;
        }
      }
      return true;
    }



    static bool IsNumericWithSign( const GR::String& Text )
    {
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        char    digit( Text[i] );

        if ( ( digit < '0' )
        ||   ( digit > '9' ) )
        {
          if ( ( i == 0 )
          &&   ( digit != '-' )
          &&   ( digit != '+' ) )
          {
            return false;
          }
        }
      }
      return true;
    }



    static bool IsValidASCII( const GR::String& Text )
    {
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        char    digit( Text[i] );

        if ( (GR::u8)digit < 32 )
        {
          return false;
        }
      }
      return true;
    }



    static bool IsAlphaNumeric( const GR::String& Text )
    {
      #ifndef __TANDEM
      std::locale     loc( std::locale::classic() );
      #endif
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        char    digit( Text[i] );

        #ifdef __TANDEM
        if ( !std::isalnum( digit ) )
        #else
        if ( !std::isalnum( digit, loc ) )
        #endif
        {
          return false;
        }
      }
      return true;
    }



    static bool IsAlpha( const GR::String& Text )
    {
      #ifndef __TANDEM
      std::locale     loc( std::locale::classic() );
      #endif
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        char    digit( Text[i] );

        #ifdef __TANDEM
        if ( !std::isalpha( digit ) )
        #else
        if ( !std::isalpha( digit, loc ) )
        #endif
        {
          return false;
        }
      }
      return true;
    }



    static bool IsValidHex( const char Character )
    {
      if ( ( ( Character >= 'A' )
      &&     ( Character <= 'F' ) )
      ||   ( ( Character >= 'a' )
      &&     ( Character <= 'f' ) )
      ||   ( ( Character >= '0' )
      &&     ( Character <= '9' ) ) )
      {
        return true;
      }
      return false;
    }



    static bool IsValidHex( const GR::String& Text )
    {
      for ( size_t i = 0; i < Text.length(); ++i )
      {
        if ( !IsValidHex( Text[i] ) )
        {
          return false;
        }
      }
      return true;
    }



#if OPERATING_SYSTEM == OS_WINDOWS
    static GR::WString UTF8ToUTF16( const GR::String& utf8 )
    {
      // Special case of empty input string
      if ( utf8.empty() )
      {
        return GR::WString();
      }

      // Get length (in wchar_t's) of resulting UTF-16 string
      const int utf16_length = ::MultiByteToWideChar( CP_UTF8,            // convert from UTF-8
                                                      0,                  // default flags
                                                      utf8.data(),        // source UTF-8 string
                                                      (int)utf8.length(),      // length (in chars) of source UTF-8 string
                                                      NULL,               // unused - no conversion done in this step
                                                      0 );                // request size of destination buffer, in wchar_t's
      if ( utf16_length == 0 )
      {
        // Error
        DWORD error = ::GetLastError();
        dh::Error( "Can't get length of UTF-16 string (MultiByteToWideChar set last error to %lu).", error );
        return L"";
      }

      // Allocate destination buffer for UTF-16 string
      GR::WString  utf16;

      utf16.resize( utf16_length );

      // Do the conversion from UTF-8 to UTF-16
      if ( !::MultiByteToWideChar( CP_UTF8,            // convert from UTF-8
                                   0,                  // default flags
                                   utf8.data(),        // source UTF-8 string
                                   (int)utf8.length(),      // length (in chars) of source UTF-8 string
                                   &utf16[0],          // destination buffer
                                   (int)utf16.length() ) )  // size of destination buffer, in wchar_t's
      {
        // Error
        DWORD error = ::GetLastError();
        dh::Error( "Can't convert string from UTF-8 to UTF-16 (MultiByteToWideChar set last error to %lu).", error );
        return L"";
      }
      return utf16;
    }



    static GR::String UTF16ToUTF8( const GR::WString& utf16 )
    {
      // Special case of empty input string
      if ( utf16.empty() )
      {
        return GR::String();
      }

      // Get length (in chars) of resulting UTF-8 string
      const int utf8_length = ::WideCharToMultiByte( CP_UTF8,            // convert to UTF-8
                                                     0,                  // default flags
                                                     utf16.data(),       // source UTF-16 string
                                                     (int)utf16.length(),     // source string length, in wchar_t's,
                                                     NULL,               // unused - no conversion required in this step
                                                     0,                  // request buffer size
                                                     NULL, NULL );       // unused
      if ( utf8_length == 0 )
      {
        // Error
        DWORD error = ::GetLastError();
        dh::Error( "Can't get length of UTF-8 string (WideCharToMultiByte set last error to %lu).", error );
        return GR::String();
      }

      // Allocate destination buffer for UTF-8 string
      GR::String utf8;
      utf8.resize( utf8_length );

      // Do the conversion from UTF-16 to UTF-8
      if ( !::WideCharToMultiByte( CP_UTF8,                // convert to UTF-8
                                   0,                      // default flags
                                   utf16.data(),           // source UTF-16 string
                                   (int)utf16.length(),         // source string length, in wchar_t's,
                                   &utf8[0],               // destination buffer
                                   (int)utf8.length(),          // destination buffer size, in chars
                                   NULL, NULL ) )          // unused
      {
        // Error
        DWORD error = ::GetLastError();
        dh::Error( "Can't convert string from UTF-16 to UTF-8 (WideCharToMultiByte set last error to %lu).", error );
        return GR::String();
      }
      return utf8;
    }
#endif

  }

}

#endif // STRING_UTIL_H
