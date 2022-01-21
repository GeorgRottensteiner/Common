#include "JSON.h"

#include <debug/debugclient.h>

#include <String/StringUtil.h>

#include <IO/FileStream.h>
#include <IO/FileUtil.h>



namespace GR
{
  namespace Strings
  {
    namespace JSON
    {
      GR::String                   Parser::m_NewLine = "\r\n";



      Element::Element() :
        m_pParent( NULL )
      {
      }



      Element::Element( const ElementType::Value Type, const GR::String& Content ) :
        m_pParent( NULL ),
        m_Type( Type ),
        m_Content( Content )
      {
      }



      Element::~Element()
      {
        Clear();
      }



      ElementType::Value Element::Type() const
      {
        return m_Type;
      }



      void Element::Name( const GR::String& Name )
      {
        m_Name = Name;
      }



      GR::String Element::Name() const
      {
        return m_Name;
      }



      GR::String Element::GetContent() const
      {
        return m_Content;
      }



      void Element::Content( const GR::String& Content )
      {
        m_Content = Content;
      }



      void Element::Clear()
      {
        tListElements::iterator   it( m_Childs.begin() );
        while ( it != m_Childs.end() )
        {
          Element* pChild( *it );

          pChild->m_pParent = NULL;
          delete pChild;

          ++it;
        }
        m_Childs.clear();
      }



      GR::u32 Element::Level() const
      {
        GR::u32     level = 0;

        const Element*    pElement = this;

        while ( pElement->m_pParent != NULL )
        {
          ++level;
          pElement = pElement->m_pParent;
        }
        return level;
      }



      GR::String Element::ValueToJSON( const GR::u32 IndentSpaces, bool UseLineBreaks ) const
      {
        GR::String        result;
        GR::String        indent;
        GR::String        indentOneLevel = GR::Strings::Fill( ' ', IndentSpaces );

        GR::u32   level = Level();

        while ( level > 1 )
        {
          indent += indentOneLevel;
          level--;
        }

        if ( UseLineBreaks )
        {
          result += indent;
        }
        if ( m_Type == ElementType::Object )
        {
          result += "{";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }
        else if ( m_Type == ElementType::Array )
        {
          result += "[";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }

        switch ( m_Type )
        {
          case ElementType::Bool:
          case ElementType::Number:
            result += m_Content;
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::Null:
            result += "null";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::String:
            result += "\"" + Parser::JSONEncode( m_Content ) + "\"";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::Object:
            {
              tListElements::const_iterator   itE( m_Childs.begin() );
              while ( itE != m_Childs.end() )
              {
                const Element* pChild = *itE;
                ++itE;

                result += pChild->ToJSON( IndentSpaces, UseLineBreaks );
                if ( itE != m_Childs.end() )
                {
                  if ( UseLineBreaks )
                  {
                    result.Insert( result.length() - Parser::m_NewLine.length(), "," );
                  }
                  else
                  {
                    result += ",";
                  }
                }
              }
            }
            break;
          case ElementType::Array:
            {
              tListElements::const_iterator   itE( m_Childs.begin() );
              while ( itE != m_Childs.end() )
              {
                const Element* pChild = *itE;
                ++itE;

                result += pChild->ToJSON( IndentSpaces, UseLineBreaks );
                if ( itE != m_Childs.end() )
                {
                  if ( UseLineBreaks )
                  {
                    result.Insert( result.length() - Parser::m_NewLine.length(), "," );
                  }
                  else
                  {
                    result += ",";
                  }
                }
              }
            }
            break;
        }

        if ( m_Type == ElementType::Object )
        {
          if ( UseLineBreaks )
          {
            result += indent;
          }
          result += "}";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }
        else if ( m_Type == ElementType::Array )
        {
          if ( UseLineBreaks )
          {
            result += indent;
          }
          result += "]";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }

        return result;
      }



      GR::String Element::ToJSON( const GR::u32 IndentSpaces, bool UseLineBreaks ) const
      {
        GR::String        result;
        GR::String        indent;
        GR::String        indentOneLevel = GR::Strings::Fill( ' ', IndentSpaces );

        GR::u32   level = Level();

        while ( level > 1 )
        {
          indent += indentOneLevel;
          level--;
        }

        if ( m_Type == ElementType::Object )
        {
          if ( !m_Name.empty() )
          {
            result += indent + '"' + Parser::JSONEncode( m_Name ) + "\":";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
          }
          result += indent + "{";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }
        else if ( m_Type == ElementType::Array )
        {
          if ( !m_Name.empty() )
          {
            result += indent + '"' + Parser::JSONEncode( m_Name ) + "\":";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
          }
          if ( UseLineBreaks )
          {
            result += indent + "[" + Parser::m_NewLine;
          }
          else
          {
            result += " [";
          }
        }

        switch ( m_Type )
        {
          case ElementType::Bool:
          case ElementType::Number:
            result += indent + '"' + Parser::JSONEncode( m_Name ) + "\": " + m_Content;
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::Null:
            result += indent + '"' + Parser::JSONEncode( m_Name ) + "\": null";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::String:
            result += indent + '"' + Parser::JSONEncode( m_Name ) + "\": \"" + Parser::JSONEncode( m_Content ) + "\"";
            if ( UseLineBreaks )
            {
              result += Parser::m_NewLine;
            }
            break;
          case ElementType::Object:
            {
              tListElements::const_iterator   itE( m_Childs.begin() );
              while ( itE != m_Childs.end() )
              {
                const Element* pChild = *itE;
                ++itE;

                result += pChild->ToJSON( IndentSpaces, UseLineBreaks );
                if ( ( UseLineBreaks )
                &&   ( itE != m_Childs.end() ) )
                {
                  result.Insert( result.length() - Parser::m_NewLine.length(), "," );
                }
              }
            }
            break;
          case ElementType::Array:
            {
              tListElements::const_iterator   itE( m_Childs.begin() );
              while ( itE != m_Childs.end() )
              {
                const Element* pChild = *itE;
                ++itE;

                result += pChild->ValueToJSON( IndentSpaces, UseLineBreaks );
                if ( itE != m_Childs.end() )
                {
                  if ( UseLineBreaks )
                  {
                    result.Insert( result.length() - Parser::m_NewLine.length(), "," );
                  }
                  else
                  {
                    result += ", ";
                  }
                }
              }
            }
            break;
        }

        if ( m_Type == ElementType::Object )
        {
          if ( UseLineBreaks )
          {
            result += indent;
          }
          result += "}";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }
        else if ( m_Type == ElementType::Array )
        {
          if ( UseLineBreaks )
          {
            result += indent;
          }
          result += "]";
          if ( UseLineBreaks )
          {
            result += Parser::m_NewLine;
          }
        }
        return result;
      }



      size_t Element::ChildCount() const
      {
        return m_Childs.size();
      }



      Element* Element::FirstChild() const
      {
        if ( m_Childs.empty() )
        {
          return NULL;
        }
        return m_Childs.front();
      }



      Element* Element::Parent() const
      {
        return m_pParent;
      }



      void Element::InsertChild( Element* pChild )
      {
        m_Childs.push_back( pChild );
        pChild->m_pParent = this;
      }



      Parser::Parser()
      {
      }



      Parser::~Parser()
      {
        Clear();
      }



      bool Parser::Parse( const GR::Char* pString, size_t Length )
      {
        Clear();

        Element*    pCurrentElement = &m_RootElement;
        if ( pString == NULL )
        {
          return false;
        }

        size_t  curPos = 0;

        while ( curPos < Length )
        {
          _SkipWhiteSpace( pString, curPos, Length );
          if ( curPos >= Length )
          {
            return true;
          }

          GR::Char    curChar = pString[curPos];

          Element* pElement = NULL;

          if ( curChar == '{' )
          {
            // object
            if ( !_ParseObject( pString, curPos, Length, pElement ) )
            {
              return false;
            }
          }
          else if ( curChar == '[' )
          {
            // array
            if ( !_ParseArray( pString, curPos, Length, pElement ) )
            {
              return false;
            }
          }
          else if ( !_ParseValue( pString, curPos, Length, pElement ) )
          {
            return false;
          }
          pCurrentElement->InsertChild( pElement );
        }
        return true;
      }



      bool Parser::_IsWhiteSpace( const GR::Char Char )
      {
        if ( ( Char == ' ' )
        ||   ( Char == '\n' )
        ||   ( Char == '\r' )
        ||   ( Char == '\t' ) )
        {
          return true;
        }
        return false;
      }



      bool Parser::_IsNumberChar( const GR::Char Char )
      {
        if ( ( ( Char >= '0' )
        &&     ( Char <= '9' ) )
        ||   ( Char == 'e' )
        ||   ( Char == 'E' )
        ||   ( Char == '.' )
        ||   ( Char == '-' )
        ||   ( Char == '+' ) )
        {
          return true;
        }
        return false;
      }



      void Parser::_SkipWhiteSpace( const GR::Char* pString, size_t& CurPos, size_t Length )
      {
        if ( pString == NULL )
        {
          return;
        }

        while ( ( CurPos < Length )
        &&      ( _IsWhiteSpace( pString[CurPos] ) ) )
        {
          ++CurPos;
        }
      }



      GR::String Parser::JSONEncode( const GR::String& Source )
      {
        GR::String    result;

        size_t    pos = 0;

        while ( pos < Source.length() )
        {
          GR::Char    curChar( Source[pos] );

          if ( curChar == '"' )
          {
            result += "\\\"";
          }
          else if ( curChar == '\\' )
          {
            result += "\\\\";
          }
          else if ( curChar == '\t' )
          {
            result += "\\t";
          }
          else if ( curChar == '\n' )
          {
            result += "\\n";
          }
          else if ( curChar == '\r' )
          {
            result += "\\r";
          }
          else
          {
            result += curChar;
          }

          ++pos;
        }
        return result;
      }



      bool Parser::_ParseString( const GR::Char* pString, size_t& CurPos, size_t Length, GR::String& ResultingString )
      {
        if ( ( pString == NULL )
        ||   ( CurPos >= Length ) )
        {
          return false;
        }

        GR::Char    curChar = pString[CurPos];
        if ( curChar != '"' )
        {
          dh::Log( "JSON::ParseString expected \", found %c instead", curChar );
          return false;
        }
        ++CurPos;

        ResultingString.clear();

        bool      controlCharacterEncountered = false;

        while ( CurPos < Length )
        {
          curChar = pString[CurPos];

          if ( controlCharacterEncountered )
          {
            switch ( curChar )
            {
              case '"':
              case '\\':
              case '/':
                ResultingString += curChar;
                break;
              case 'b':
                // backspace???
                break;
              case 'f':
                // form feed?
                break;
              case 'n':
                ResultingString += '\n';
                break;
              case 'r':
                ResultingString += '\r';
                break;
              case 't':
                ResultingString += '\t';
                break;
              case 'u':
                {
                  if ( CurPos + 4 >= Length )
                  {
                    dh::Log( "JSON::ParseString malformed \\u constant" );
                    return false;
                  }
                  if ( !GR::Strings::IsValidHex( pString + 1, 4 ) )
                  {
                    dh::Log( "JSON::ParseString malformed \\u constant" );
                    return false;
                  }
                  GR::u32     hexValue = GR::Convert::ToU32( GR::String( pString + 1, 4 ), 16 );

                  // TODO
                }
                break;
              default:
                dh::Log( "JSON::ParseString malformed control character %c", curChar );
                return false;
            }
            controlCharacterEncountered = false;
          }
          else if ( curChar == '"' )
          {
            // end of string
            ++CurPos;
            return true;
          }
          else if ( curChar == '\\' )
          {
            controlCharacterEncountered = true;
          }
          else
          {
            ResultingString.append( curChar );
          }
          ++CurPos;
        }
        dh::Log( "JSON::ParseString unexpected end" );
        return false;
      }



      bool Parser::_ParseNumber( const GR::Char* pString, size_t& CurPos, size_t Length, GR::String& ResultingString )
      {
        if ( ( pString == NULL )
        ||   ( CurPos >= Length ) )
        {
          return false;
        }

        ResultingString.clear();

        while ( CurPos < Length )
        {
          GR::Char curChar = pString[CurPos];

          if ( !_IsNumberChar( curChar ) )
          {
            // end of number
            return true;
          }
          ResultingString.append( curChar );
          ++CurPos;
          if ( CurPos >= Length )
          {
            // end of number
            return true;
          }
        }
        dh::Log( "JSON::ParseString unexpected end" );
        return false;
      }



      bool Parser::_ParseObject( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement )
      {
        pElement = NULL;
        if ( ( pString == NULL )
        ||   ( CurPos >= Length ) )
        {
          return false;
        }

        GR::Char    curChar = pString[CurPos];

        if ( curChar != '{' )
        {
          dh::Log( "JSON::ParseObject expected {, found %c instead", curChar );
          return false;
        }
        ++CurPos;

        pElement = new Element( GR::Strings::JSON::ElementType::Object );

        while ( CurPos < Length )
        {
          _SkipWhiteSpace( pString, CurPos, Length );
          if ( CurPos >= Length )
          {
            dh::Log( "JSON::ParseObject unexpected end" );
            delete pElement;
            pElement = NULL;
            return false;
          }
          curChar = pString[CurPos];

          if ( curChar == '}' )
          {
            ++CurPos;
            return true;
          }
          else if ( curChar == '"' )
          {
            // string -> ws -> : -> value
            GR::String      resultingString;
            if ( !_ParseString( pString, CurPos, Length, resultingString ) )
            {
              delete pElement;
              pElement = NULL;
              return false;
            }
            _SkipWhiteSpace( pString, CurPos, Length );
            if ( CurPos >= Length )
            {
              dh::Log( "JSON::ParseObject unexpected end" );
              delete pElement;
              pElement = NULL;
              return false;
            }
            curChar = pString[CurPos];
            if ( curChar != ':' )
            {
              dh::Log( "JSON::ParseObject expected :, found %c instead", curChar );
              delete pElement;
              pElement = NULL;
              return false;
            }
            ++CurPos;

            Element* pChildElement = NULL;
            if ( !_ParseValue( pString, CurPos, Length, pChildElement ) )
            {
              delete pElement;
              pElement = NULL;
              return false;
            }
            pChildElement->Name( resultingString );
            pElement->InsertChild( pChildElement );
            curChar = pString[CurPos];
            if ( curChar == '}' )
            {
              ++CurPos;
              return true;
            }
            if ( curChar != ',' )
            {
              dh::Log( "JSON::ParseObject expected ',', found %c instead", curChar );
              delete pElement;
              pElement = NULL;
              return false;
            }
            ++CurPos;
          }
          else
          {
            dh::Log( "JSON::ParseObject malformed, unexpected character %c found", curChar );
            delete pElement;
            pElement = NULL;
            return false;
          }
        }

        dh::Log( "JSON::ParseObject unexpected end" );
        delete pElement;
        pElement = NULL;
        return false;
      }



      bool Parser::_ParseArray( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement )
      {
        if ( ( pString == NULL )
        ||   ( CurPos >= Length ) )
        {
          return false;
        }

        GR::Char    curChar = pString[CurPos];
        if ( curChar != '[' )
        {
          dh::Log( "JSON::_ParseArray expected [, found %c instead", curChar );
          return false;
        }
        ++CurPos;

        pElement = new Element( GR::Strings::JSON::ElementType::Array );

        bool        hadElement = false;

        while ( CurPos < Length )
        {
          _SkipWhiteSpace( pString, CurPos, Length );
          if ( CurPos >= Length )
          {
            dh::Log( "JSON::ParseObject unexpected end" );
            delete pElement;
            pElement = NULL;
            return false;
          }
          curChar = pString[CurPos];

          if ( curChar == ']' )
          {
            ++CurPos;
            return true;
          }
          else if ( curChar == ',' )
          {
            if ( !hadElement )
            {
              dh::Log( "JSON::ParseArray unexpected , found" );
              delete pElement;
              pElement = NULL;
              return false;
            }
            ++CurPos;
            continue;
          }

          _SkipWhiteSpace( pString, CurPos, Length );

          Element*  pChildElement = NULL;
          if ( !_ParseValue( pString, CurPos, Length, pChildElement ) )
          {
            delete pElement;
            pElement = NULL;
            return false;
          }
          pElement->InsertChild( pChildElement );
          _SkipWhiteSpace( pString, CurPos, Length );
          hadElement = true;
        }

        dh::Log( "JSON::ParseArray unexpected end" );
        delete pElement;
        pElement = NULL;
        return false;
      }



      bool Parser::_LookAheadIs( const GR::Char* pString, size_t& CurPos, size_t Length, const GR::String& Text )
      {
        if ( ( pString == NULL )
        ||   ( CurPos + Text.length() > Length ) )
        {
          return false;
        }
        GR::String    snippet( pString + CurPos, Text.length() );

        return snippet == Text;
      }



      bool Parser::_ParseValue( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement )
      {
        pElement = NULL;
        _SkipWhiteSpace( pString, CurPos, Length );

        if ( CurPos >= Length )
        {
          return false;
        }
        GR::Char curChar = pString[CurPos];

        if ( curChar == '{' )
        {
          if ( !_ParseObject( pString, CurPos, Length, pElement ) )
          {
            return false;
          }
        } 
        else if ( curChar == '[' )
        {
          if ( !_ParseArray( pString, CurPos, Length, pElement ) )
          {
            return false;
          }
        }
        else if ( curChar == '"' )
        {
          GR::String    resultingString;

          if ( !_ParseString( pString, CurPos, Length, resultingString ) )
          {
            return false;
          }
          pElement = new Element( ElementType::String, resultingString );
        }
        else if ( ( GR::Strings::IsNumeric( curChar ) )
        ||        ( curChar == '-' ) )
        {
          GR::String    resultingString;

          if ( !_ParseNumber( pString, CurPos, Length, resultingString ) )
          {
            return false;
          }
          pElement = new Element( ElementType::Number, resultingString );
        }
        else if ( _LookAheadIs( pString, CurPos, Length, "true" ) )
        {
          CurPos += 4;
          pElement = new Element( ElementType::Bool, "true" );
        }
        else if ( _LookAheadIs( pString, CurPos, Length, "false" ) )
        {
          CurPos += 5;
          pElement = new Element( ElementType::Bool, "false" );
        }
        else if ( _LookAheadIs( pString, CurPos, Length, "null" ) )
        {
          CurPos += 4;
          pElement = new Element( ElementType::Null );
        }
        else
        {
          dh::Log( "ParseValue: Unexpected char %c found", curChar );
          return false;
        }

        _SkipWhiteSpace( pString, CurPos, Length );
        return true;
      }



      GR::String Parser::ToJSON( const GR::u32 IndentSpaces, bool UseLineBreaks )
      {
        if ( m_RootElement.ChildCount() == 0 )
        {
          return GR::String();
        }
        return m_RootElement.m_Childs.front()->ValueToJSON( IndentSpaces, UseLineBreaks );
      }



      void Parser::Clear()
      {
        m_RootElement.Clear();
      }



      void Parser::InsertChild( Element* pChild )
      {
        m_RootElement.InsertChild( pChild );
      }



      Element* Parser::FirstChild() const
      {
        return m_RootElement.FirstChild();
      }



      void Parser::SetLineBreak( const GR::String& LineBreak )
      {
        m_NewLine = LineBreak;
      }



      bool Parser::Load( const GR::String& FileName )
      {
        GR::String    content = GR::IO::FileUtil::ReadFileAsString( FileName );

        return Parse( content.c_str(), content.length() );
      }



      bool Parser::Load( IIOStream& IOIn )
      {
        if ( !IOIn.IsGood() )
        {
          return false;
        }

        GR::String    content;

        ByteBuffer    tempData( 65536 );

        GR::u32       bytesRead = 0;

        while ( ( bytesRead = IOIn.ReadBlock( tempData.Data(), tempData.Size() ) ) > 0 )
        {
          content.append( (GR::Char*)tempData.Data(), bytesRead );
        }

        return Parse( content.c_str(), content.length() );
      }



      bool Parser::Save( const GR::String& FileName, const GR::u32 IndentSpaces, bool UseLineBreaks )
      {
        GR::String    content = ToJSON( IndentSpaces, UseLineBreaks );

        return GR::IO::FileUtil::WriteFileFromString( FileName, content );
      }



      bool Parser::Save( IIOStream& ioOut, const GR::u32 IndentSpaces, bool UseLineBreaks )
      {
        GR::String    content = ToJSON( IndentSpaces, UseLineBreaks );

        return ioOut.WriteBlock( content.c_str(), content.length() ) == content.length();
      }

    }

  }
}
