#ifndef _PJ_STRINGHELPER_H_INCLUDED
#define _PJ_STRINGHELPER_H_INCLUDED

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <algorithm>
#include <sstream>
#include <string>
#include <list>

namespace PJ
{



  template <int BASE >
    //- BASE darf höchstens 62 sein!
  GR::String to_base( unsigned int d )
  {
    static const char symbols[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    GR::String res;
    do
    {
      unsigned int mod = d % BASE;
      res = GR::String( symbols + mod, symbols + mod + 1 ) + res;
      d /= BASE;
    }
    while ( d );
    return res;
  }

  class CStringHelper
  {
  public:
    /////////////////////////////////////////////////////////////////////
    //-    IsAlphaNumOrUnderscore( const char c )
    /////////////////////////////////////////////////////////////////////
    struct IsAlphaNumOrUnderscore
    {
      bool operator()( const char c ) const
      {
        static const GR::String CharSet( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
        return CharSet.find( c ) != GR::String::npos;
      }
    };

    struct IsNotAlphaNumOrUnderscore
    {
      bool operator()( const char c ) const
      {
        return !IsAlphaNumOrUnderscore()( c );
      }
    };

    /////////////////////////////////////////////////////////////////////
    //-    TrimLeft
    /////////////////////////////////////////////////////////////////////
    GR::String& TrimLeft(  GR::String& s, const GR::String& whitespace = " \t" ) const
    { s.erase( 0, s.find_first_not_of( whitespace ) ); return s; }

    /////////////////////////////////////////////////////////////////////
    //-    TrimRight
    /////////////////////////////////////////////////////////////////////
    GR::String& TrimRight( GR::String& s, const GR::String& whitespace = " \t" ) const
    { s.erase( s.find_last_not_of( whitespace ) + 1 ); return s; }

    /////////////////////////////////////////////////////////////////////
    //-    Trim
    /////////////////////////////////////////////////////////////////////
    GR::String& Trim( GR::String& s, const GR::String& whitespace = " \t" ) const
    {
      TrimLeft(  s, whitespace );
      TrimRight( s, whitespace );
      return s;
    }

    /////////////////////////////////////////////////////////////////////
    //-    ParsePath
    /////////////////////////////////////////////////////////////////////
    std::list< GR::String > ParsePath( const GR::String& path, const char cDiv = '\\' )
    {
      std::list< GR::String > ParsedPath;
      GR::String::const_iterator itCurrent = path.begin();
      GR::String::const_iterator itDivider = path.begin();

      do
      {
        itDivider = std::find( itCurrent, path.end(), cDiv );

        ParsedPath.push_back( GR::String( itCurrent, itDivider ) );

        if ( itDivider != path.end() )
          itCurrent = ++itDivider;
      }
      while ( itDivider != path.end() );

      return ParsedPath;
    }

    /////////////////////////////////////////////////////////////////////
    //-    ParsePath1
    /////////////////////////////////////////////////////////////////////
    //- Version 1
    std::list <GR::String> ParsePath1( const GR::String& strPath, const char cDiv = '\\' )
    {
      std::list <GR::String> results;
      if ( strPath.empty() ) return results;

      char c;
      GR::String strTemp;
      GR::String::const_iterator it( strPath.begin() ), itend( strPath.end() );
      for ( ; it != itend; ++it )
      {
        c = *it;
        // if ( c != '\\' && c != '/' )
        if ( c != cDiv )
        {
          strTemp += c;
        }
        else
        {
          results.push_back( strTemp );
          strTemp = "";
        }
      }
      results.push_back( strTemp );
      return results;
    }

    // /////////////////////////////////////////////////////////////////////
    // //-    RelativierePfad
    // /////////////////////////////////////////////////////////////////////
    // //- absoluten Unterpfad in relativen umwandeln
    // GR::String RelativierePfad( const char* const szMainPath, const char* const szSubPath )
    // {
      // if ( !szSubPath )  return "";
      // if ( !szMainPath ) return szSubPath;

      // GR::String strMainPath( szMainPath  );
      // GR::String strSubPath(  szSubPath   );
      // GR::String strResult(   strSubPath  );

      // if ( !strMainPath.empty() )
        // if ( strMainPath[ strMainPath.size() - 1 ] != '\\' )
          // strMainPath += '\\';

      // if ( !strSubPath.empty() )
        // if ( strSubPath[ strSubPath.size() - 1 ] != '\\' )
          // strSubPath += '\\';

      // //- "c:\projekte\daten\" und "c:\projekte\daten\" => ".\"
      // if ( strMainPath == strSubPath )
        // return GR::String( "" );

      // //- "c:\projekte\" und "c:\projekte\daten\" => ".\daten\"
      // GR::String strTemp( std::mismatch(  strSubPath.begin(), strSubPath.end(), strMainPath.begin() ).first );

      // if ( !strTemp.empty() )
        // if ( strTemp[ 1 ] != ':' )
          // return strTemp;

      // return strResult;
    // }



    //- "name" >> "name2"
    //- "name3" >> "name4" usw.
    /////////////////////////////////////////////////////////////////////
    //-    IncreaseIndex
    /////////////////////////////////////////////////////////////////////
    GR::String IncreaseIndex( const GR::String& strName )
    {
      GR::String strTemp( strName );

      int iIndexPos = strTemp.find_last_not_of( "0123456789" );
      if ( iIndexPos != GR::String::npos )
      {
        GR::String strIndex( strTemp );
        strIndex.erase( 0, iIndexPos + 1 );
        strTemp.erase( iIndexPos + 1 );

        int iIndex = atoi( strIndex.c_str() );

        std::ostringstream oss;
        oss << strTemp.c_str();
        oss.setf( std::ios::right, std::ios::right );
        oss.fill( '0' );
        oss.width( 4 );
        oss << iIndex + 1;

        strTemp = oss.str();
      }
      else
      {
        strTemp += "1";
      }

      return strTemp;
    }


    /////////////////////////////////////////////////////////////////////
    //-    IncreaseIndex
    /////////////////////////////////////////////////////////////////////
    GR::String IncreaseIndex( const GR::String& strName, const long iStartIndex )
    {
      GR::String strTemp( strName );

      int iIndexPos = strTemp.find_last_not_of( "0123456789" );
      if ( iIndexPos != GR::String::npos )
      {
        GR::String strIndex( strTemp );
        strIndex.erase( 0, iIndexPos + 1 );
        strTemp.erase( iIndexPos + 1 );

        int iIndex = atoi( strIndex.c_str() );
        if ( iIndex < iStartIndex ) iIndex = iStartIndex;

        std::ostringstream oss;
        oss << strTemp.c_str();
        // oss.setf( std::ios::right, std::ios::right );
        // oss.fill( '0' );
        // oss.width( 4 );
        oss << iIndex + 1;

        strTemp = oss.str();
      }
      else
      {
        strTemp += "1";
      }

      // else
      // {
        // std::ostringstream oss;
        // oss << strTemp.c_str() << iStartIndex;
        // strTemp = oss.str();
      // }

      return strTemp;
    }


    /////////////////////////////////////////////////////////////////////
    //-    IsAlphaNumOrUnderscore (lieber die entsprechende struct verwenden)
    /////////////////////////////////////////////////////////////////////
    bool _IsAlphaNumOrUnderscore( const char c )
    {
      static GR::String CharSet( "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );
      if ( CharSet.find( c ) != GR::String::npos )
      {
        return true;
      }
      else
      {
        return false;
      }
    }


    /////////////////////////////////////////////////////////////////////
    //-    MakeBezeichner
    /////////////////////////////////////////////////////////////////////
    GR::String& MakeBezeichner( GR::String& strBezeichner )
    {
      std::replace_if( strBezeichner.begin(), strBezeichner.end(), IsNotAlphaNumOrUnderscore(), (char)'_' );
      return strBezeichner;
    }

    /////////////////////////////////////////////////////////////////////
    //-    ESC
    /////////////////////////////////////////////////////////////////////
    //- string mit Escapes versehen
    //- \ -> \\, " -> \" usw. und umgekehrt
    GR::String ESC( const GR::String& s )
    {
      GR::String strTemp;
      GR::String::const_iterator it( s.begin() ), itend( s.end() );
      for ( ; it != itend; ++it )
      {
        switch ( *it )
        {
        case '\n': strTemp += "\\n" ; break;
        case '\r': strTemp += "\\r" ; break;
        case '\t': strTemp += "\\t" ; break;
        case '\\': strTemp += "\\\\"; break;
        case '\"': strTemp += "\\\""; break;
        default:   strTemp += *it;    break;
        }
      }
      return strTemp;
    }

    /////////////////////////////////////////////////////////////////////
    //-    ESCself
    /////////////////////////////////////////////////////////////////////
    GR::String& ESCself( GR::String& s )
    {
      s.swap( ESC( s ) );
      return s;
    }

    /////////////////////////////////////////////////////////////////////
    //-    unESC
    /////////////////////////////////////////////////////////////////////
    //- string von Escapes befreien
    GR::String unESC( const GR::String& s )
    {
      GR::String strTemp;
      GR::String::const_iterator it( s.begin() ), itend( s.end() );
      for ( ; it != itend; )
      {
        if ( *it != '\\' )
        {
          strTemp += *it;
          ++it;
        }
        else
        {
          ++it;
          if ( it == itend ) break;
          switch ( *it )
          {
          case 'n' : strTemp += '\n'; break;
          case 'r' : strTemp += '\r'; break;
          case 't' : strTemp += '\t'; break;
          case '\\': strTemp += '\\'; break;
          case '\"': strTemp += '\"'; break;
          default:   strTemp += *it ; break;
          }
          ++it;
        }
      }
      return strTemp;
    }

    /////////////////////////////////////////////////////////////////////
    //-    unESCself
    /////////////////////////////////////////////////////////////////////
    GR::String& unESCself( GR::String& s )
    {
      s.swap( unESC( s ) );
      return s;
    }

    /////////////////////////////////////////////////////////////////////
    //-    Entifizieren (html-tauglich machen)
    /////////////////////////////////////////////////////////////////////
    GR::String Entifizieren( const GR::String& s )
    {
      std::ostringstream os;
      GR::String::const_iterator it( s.begin() ), itend( s.end() );
      for ( ; it != itend; ++it )
      {
        switch ( *it )
        {
        case '\0':                break;
        // case '\n': os << "<br>" ; break;
        case '\n': os << "<p>" ; break;
        // case '\r': strTemp += "\\r" ; break;
        // case '\t': strTemp += "\\t" ; break;
        // case '\\': strTemp += "\\\\"; break;
        // case '\"': strTemp += "\\\""; break;
        case 'ä': os << "&auml;"; break;
        case 'Ä': os << "&Auml;"; break;
        case 'ö': os << "&ouml;"; break;
        case 'Ö': os << "&Ouml;"; break;
        case 'ü': os << "&uuml;"; break;
        case 'Ü': os << "&Uuml;"; break;
        // case 'ß': os << "&suml;"; break;
        case '<': os << "&lt;";   break;
        case '>': os << "&gt;";   break;
        default:  os << *it;      break;
        }
      }
      return os.str();
    }

    //- ersetzt jedes auftreten von replace_this in source durch change_to und schreibt alles nach target
    static void replace(        GR::String& target,
                                GR::String& source,
                          const GR::String& replace_this,
                          const GR::String& change_to )
    {
      GR::String::iterator it    =  source.begin();
      GR::String::iterator itend =  source.end();
      GR::String result;

      for ( ; it != itend; )
      {
        GR::String::iterator itbegin = it;
        it = std::search( it, itend, replace_this.begin(), replace_this.end() );

        result += GR::String( itbegin, it );

        if ( it != itend )
        {
          result += change_to;
          std::advance( it, replace_this.length() );
        }
      }

      target.swap( result );
    }

  };




};

#endif//_PJ_STRINGHELPER_H_INCLUDED