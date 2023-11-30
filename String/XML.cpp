#include "XML.h"

#include <IO/FileStream.h>

#include <String/Path.h>
#include <String/StringUtil.h>

#include <debug/debugclient.h>

#include <GR/GRTypes.h>

#ifndef _WIN32
#ifdef UNICODE
#define _T(x)      L ## x
#else
#define _T(x)      x
#endif
#endif


namespace GR
{
  namespace Strings
  {
    XML::tMapKnownTypes           XML::m_KnownTypes;

    XML::tSetParsedFiles          XML::m_ParsedFiles;



    GR::String                   XML::m_NewLine       = "\r\n";
    //GR::String                   XML::m_XMLFileHeader = _T( "\xfeff<?xml version='1.0' encoding='UTF-16'?>" );
    GR::String                   XML::m_XMLFileHeader = "<?xml version='1.0' encoding='ISO-8859-1'?>";


    GR::String DoSubstr( const GR::String& Orig, size_t Offset, size_t Length )
    {
      if ( Offset >= Orig.length() )
      {
        dh::Log( "XMLParser: SubStr Offset zu gross!" );
      }
      if ( Length >= 1000000 )
      {
        dh::Log( "XMLParser: too long" );
      }
      if ( Offset + Length > Orig.length() )
      {
        dh::Log( "XMLParser: length is outside original string!" );
      }
      return Orig.substr( Offset, Length );
    }



    GR::String DoSubstr( const GR::String& Orig, size_t Offset )
    {
      if ( Offset >= Orig.length() )
      {
        dh::Log( "SubStr Offset zu gross!" );
      }
      return Orig.substr( Offset );
    }



    XMLElement::XMLElement( const GR::String& Type, const GR::String& ContentArg ) :
      m_IsCDataContent( false ),
      m_IsSystemTag( false ),
      m_pParent( NULL ),
      m_Type( Type )
    {
      // Sonderzeichen im Data ¸bersetzen
      Content( ContentArg );
    }



    XMLElement::XMLElement( const XMLElement& RHS )
    {
      m_Type            = RHS.m_Type;
      m_IsCDataContent  = RHS.m_IsCDataContent;
      m_Content         = RHS.m_Content;
      m_pParent         = RHS.m_pParent;
      m_Attributes      = RHS.m_Attributes;
      m_IsSystemTag     = RHS.m_IsSystemTag;

      tListElements::const_iterator   it( RHS.m_Childs.begin() );
      while ( it != RHS.m_Childs.end() )
      {
        GR::Strings::XMLElement*    pXMLChild( *it );

        m_Childs.push_back( pXMLChild->Clone() );

        ++it;
      }
    }



    XMLElement::~XMLElement()
    {
      Clear();
    }



    XMLElement& XMLElement::operator=( const XMLElement& RHS )
    {
      if ( this != &RHS )
      {
        m_Type            = RHS.m_Type;
        m_IsCDataContent  = RHS.m_IsCDataContent;
        m_Content         = RHS.m_Content;
        m_pParent         = RHS.m_pParent;
        m_Attributes      = RHS.m_Attributes;
        m_IsSystemTag     = RHS.m_IsSystemTag;

        tListElements::const_iterator   it( RHS.m_Childs.begin() );
        while ( it != RHS.m_Childs.end() )
        {
          GR::Strings::XMLElement*    pXMLChild( *it );

          m_Childs.push_back( pXMLChild->Clone() );

          ++it;
        }
      }
      return *this;
    }



    bool XMLElement::IsComment() const
    {
      if ( !m_Type.empty() )
      {
        return false;
      }
      return ( ( GR::Strings::StartsWith( m_Content, "!--" ) )
      ||       ( GR::Strings::StartsWith( m_Content, "!DOCTYPE" ) ) );
    }



    void XMLElement::SystemTag( bool IsSystemTag )
    {
      m_IsSystemTag = IsSystemTag;
    }



    bool XMLElement::IsSystemTag() const
    {
      return m_IsSystemTag;
    }



    void XMLElement::Clear()
    {
      tListElements::iterator   it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pElement = *it;

        delete pElement;

        ++it;
      }
      m_Childs.clear();
      m_Attributes.clear();
      m_IsSystemTag = false;
    }




    bool XMLElement::IsCDataContent() const
    {
      return m_IsCDataContent;
    }



    void XMLElement::CDATAContent( const GR::String& Content )
    {
      m_IsCDataContent  = true;
      m_Content         = Content;
    }



    void XMLElement::Content( const GR::String& Content, bool ParseHTMLTags )
    {
      if ( !ParseHTMLTags )
      {
        m_Content    = Content;
        m_IsCDataContent = false;
        return;
      }

      m_IsCDataContent = false;
      m_Content.erase();

      bool    isCommand = false;

      size_t  pos = 0,
              commandStartPos = 0;

      GR::String   command;

      while ( pos < Content.length() )
      {
        GR::Char    c = Content.at( pos );

        if ( c == '&' )
        {
          if ( !isCommand )
          {
            isCommand = true;
            command.erase();
            commandStartPos = pos;
          }
        }
        else if ( c == ';' )
        {
          if ( isCommand )
          {
            isCommand = false;
            GR::String     Command = DoSubstr( Content, commandStartPos, pos - commandStartPos + 1 );

            m_Content += XML::XMLDecode( Command );
          }
          else
          {
            m_Content += c;
          }
        }
        else if ( isCommand )
        {
          command += (char)toupper( c );
        }
        else
        {
          m_Content += c;
        }

        ++pos;
      }
    }



    bool XMLElement::HasAttribute( const GR::String& AttributeName ) const
    {
      XMLElement::tListAttributes::const_iterator   it( m_Attributes.begin() );
      while ( it != m_Attributes.end() )
      {
        const tXMLAttribute&    XMLAttr = *it;

        if ( XMLAttr.AttributeName == AttributeName )
        {
          return true;
        }
        ++it;
      }
      return false;
    }



    void XMLElement::AddAttribute( const GR::String& AttributeName, const GR::String& AttributeValue )
    {
      XMLElement::tListAttributes::iterator   it( m_Attributes.begin() );
      while ( it != m_Attributes.end() )
      {
        tXMLAttribute&    XMLAttr = *it;

        if ( XMLAttr.AttributeName == AttributeName )
        {
          XMLAttr.AttributeValue = XML::XMLEncode( AttributeValue );
          return;
        }
        ++it;
      }
      m_Attributes.push_back( tXMLAttribute( AttributeName, XML::XMLEncode( AttributeValue ) ) );
    }



    void XMLElement::SetAttribute( const GR::String& AttributeName, const GR::String& AttributeValue )
    {
      XMLElement::tListAttributes::iterator   it( m_Attributes.begin() );
      while ( it != m_Attributes.end() )
      {
        tXMLAttribute&    XMLAttr = *it;

        if ( XMLAttr.AttributeName == AttributeName )
        {
          XMLAttr.AttributeValue = XML::XMLEncode( AttributeValue );
          return;
        }
        ++it;
      }
      // a new attribute
      AddAttribute( AttributeName, XML::XMLEncode( AttributeValue ) );
    }



    XMLElement* XMLElement::FirstChild() const
    {
      if ( m_Childs.empty() )
      {
        return NULL;
      }
      return m_Childs.front();
    }



    XMLElement* XMLElement::Parent() const
    {
      return m_pParent;
    }



    void XMLElement::InsertChild( XMLElement* pChild )
    {
      pChild->m_pParent = this;
      m_Childs.push_back( pChild );
    }



    XMLElement* XMLElement::AddElement( const GR::String& ElementName, const GR::String& Content )
    {
      XMLElement* pElement = new XMLElement( ElementName, Content );

      pElement->m_pParent = this;

      m_Childs.push_back( pElement );

      return pElement;
    }



    void XMLElement::MoveChilds( XMLElement* pXMLTarget )
    {
      if ( pXMLTarget == NULL )
      {
        return;
      }

      tListElements::iterator   it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pElement = *it;

        pXMLTarget->InsertChild( pElement );
        it = m_Childs.erase( it );
      }
    }



    void XMLElement::RemoveAllAttributes()
    {
      m_Attributes.clear();
    }



    size_t XMLElement::AttributeCount() const
    {
      return m_Attributes.size();
    }



    GR::String XMLElement::AttributeName( size_t Index ) const
    {
      if ( Index >= m_Attributes.size() )
      {
        return GR::String();
      }

      XMLElement::tListAttributes::const_iterator   it( m_Attributes.begin() );
      std::advance( it, Index );

      return it->AttributeName;
    }



    GR::String XMLElement::AttributeValue( size_t Index ) const
    {
      if ( Index >= m_Attributes.size() )
      {
        return GR::String();
      }

      XMLElement::tListAttributes::const_iterator   it( m_Attributes.begin() );
      std::advance( it, Index );

      return XML::XMLDecode( it->AttributeValue );
    }



    GR::String XMLElement::Attribute( const GR::String& Param ) const
    {
      XMLElement::tListAttributes::const_iterator   it( m_Attributes.begin() );
      while ( it != m_Attributes.end() )
      {
        const tXMLAttribute&    XMLAttr = *it;

        if ( XMLAttr.AttributeName == Param )
        {
          return GR::String( XML::XMLDecode( it->AttributeValue ) );
        }

        ++it;
      }
      return GR::String();
    }



    GR::String XMLElement::ToXML( const GR::u32 IndentSpaces, bool UseLineBreaks )
    {
      return ToString( IndentSpaces, UseLineBreaks );
    }



    GR::String XMLElement::ContentToString()
    {
      return XML::XMLEncode( m_Content );
    }



    GR::String XMLElement::ToString( const GR::u32 IndentSpaces, bool UseLineBreaks )
    {
      GR::String       result;

      GR::String       indent;

      GR::u32   level = Level();

      while ( level > 1 )
      {
        indent += GR::Strings::Fill( ' ', IndentSpaces );
        level--;
      }


      result += indent;

      result += '<';

      if ( IsSystemTag() )
      {
        result += '?';
      }

      if ( IsComment() )
      {
        result += m_Content + ">";
        if ( UseLineBreaks )
        {
          result += XML::m_NewLine;
        }
        return result;
      }

      result += m_Type;

      if ( !m_Attributes.empty() )
      {
        if ( !m_Type.empty() )
        {
          result += ' ';
        }
        tListAttributes::iterator   itAttr( m_Attributes.begin() );
        while ( itAttr != m_Attributes.end() )
        {
          tXMLAttribute&    Attribute = *itAttr;

          result += Attribute.AttributeName;
          // Parameter-Inhalt in Anf¸hrungszeichen
          result += "=\"";
          result += XML::XMLEncode( Attribute.AttributeValue );
          result += "\"";


          ++itAttr;
          if ( itAttr != m_Attributes.end() )
          {
            result += ' ';
          }
        }
      }

      if ( IsSystemTag() )
      {
        result += "?>";
        if ( UseLineBreaks )
        {
          result += XML::m_NewLine;
        }
        return result;
      }

      if ( ( m_Childs.empty() )
      &&   ( m_Content.empty() ) )
      {
        // Einzel-Tag, autoclosing
        result += " />";
        if ( UseLineBreaks )
        {
          result += XML::m_NewLine;
        }
        return result;
      }
      else if ( m_Content.empty() )
      {
        result += ">";
        if ( UseLineBreaks )
        {
          result += XML::m_NewLine;
        }
      }
      else
      {
        result += ">" + ContentToString();

        if ( ( !m_Childs.empty() )
        &&   ( UseLineBreaks ) )
        {
          result += XML::m_NewLine;
        }
      }

      // Childs
      tListElements::iterator   itChild( m_Childs.begin() );
      while ( itChild != m_Childs.end() )
      {
        XMLElement*    pChild = *itChild;

        result += pChild->ToString( IndentSpaces );

        ++itChild;
      }

      // Tag schlieﬂen
      if ( !m_Childs.empty() )
      {
        result += indent;
      }
      result += "</";
      result += m_Type;
      result += ">";
      if ( UseLineBreaks )
      {
        result += XML::m_NewLine;
      }

      return result;
    }



    void XMLElement::DumpElement( const GR::String& Indent )
    {
      dh::Log( Indent.c_str() );
      dh::Log( "Element %s (%x)", Type().c_str(), this );
      if ( !GetContent().empty() )
      {
        dh::Log( "   Content %s", GetContent().c_str() );
      }

      XMLElement::tListAttributes::iterator    itAttr( m_Attributes.begin() );
      while ( itAttr != m_Attributes.end() )
      {
        tXMLAttribute&    XMLAttr = *itAttr;

        dh::Log( " %s=%s", XMLAttr.AttributeName.c_str(), XMLAttr.AttributeValue.c_str() );

        ++itAttr;
      }

      dh::Log( "" );

      XMLElement::tListElements::iterator    it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        pChild->DumpElement( Indent + "  " );

        ++it;
      }

      dh::Log( Indent.c_str() );
      dh::Log( "/Element %s", Type().c_str() );

    }



    void XMLElement::Type( const GR::String& Type )
    {
      m_Type = Type;
    }



    GR::String XMLElement::Type() const
    {
      return m_Type;
    }



    GR::String XMLElement::GetContentOfSubTags( const GR::String& SubTags, bool PrepareForFile )
    {
      size_t      dotPos = SubTags.find( '.' );
      if ( dotPos == GR::String::npos )
      {
        XMLElement*   pChild = FindByType( SubTags );
        if ( pChild == NULL )
        {
          return GR::String();
        }
        return pChild->GetContent( PrepareForFile );
      }
      XMLElement*   pChild = FindByType( SubTags.substr( 0, dotPos ) );
      if ( pChild == NULL )
      {
        return GR::String();
      }
      return pChild->GetContentOfSubTags( SubTags.substr( dotPos + 1 ), PrepareForFile );
    }



    GR::String XMLElement::GetContent( bool PrepareForFile ) const
    {
      if ( !PrepareForFile )
      {
        return m_Content;
      }

      // f¸r Files m¸ssen einige Zeichen durch Codes ersetzt werden
      if ( m_IsCDataContent )
      {
        GR::String     Result = "![CDATA[";

        Result += m_Content + "]]";
        return Result;
      }
      return XML::XMLEncode( m_Content );
    }



    size_t XMLElement::ChildCount() const
    {
      return m_Childs.size();
    }



    GR::u32 XMLElement::Level() const
    {
      GR::u32   level = 0;

      const XMLElement*    pElement = this;

      while ( pElement->m_pParent )
      {
        pElement = pElement->m_pParent;
        ++level;
      }
      return level;
    }



    XMLElement* XMLElement::Clone( bool bDeepClone )
    {

      XMLElement*    pNewElement = new XMLElement();

      pNewElement->m_Attributes     = m_Attributes;
      pNewElement->m_Content        = m_Content;
      pNewElement->m_Type           = m_Type;
      pNewElement->m_IsCDataContent = m_IsCDataContent;

      if ( bDeepClone )
      {
        tListElements::iterator   it( m_Childs.begin() );
        while ( it != m_Childs.end() )
        {
          XMLElement*    pChild = *it;

          pNewElement->InsertChild( pChild->Clone() );

          ++it;
        }
      }

      return pNewElement;
    }



    XMLElement* XMLElement::FindByType( const GR::String& TypeName )
    {
      if ( Type() == TypeName )
      {
        return this;
      }

      tListElements::iterator   it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        XMLElement*    pFound = pChild->FindByType( TypeName );
        if ( pFound )
        {
          return pFound;
        }

        ++it;
      }
      return NULL;
    }



    XMLElement* XMLElement::FindByTypeRecursive( const GR::String& TypeName )
    {
      size_t      dotPos = TypeName.find( _T( '.' ) );
      if ( dotPos == GR::String::npos )
      {
        return FindByType( TypeName );
      }

      GR::String     subType = TypeName.substr( dotPos + 1 );

      tListElements::iterator   it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        XMLElement*    pFound = pChild->FindByTypeRecursive( subType );
        if ( pFound != NULL )
        {
          return pFound;
        }
        ++it;
      }
      return NULL;
    }



    void XMLElement::DeleteChild( XMLElement* pChildToDelete )
    {
      tListElements::iterator   it( m_Childs.begin() );
      while ( it != m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        if ( pChild == pChildToDelete )
        {
          delete pChild;
          m_Childs.erase( it );
          return;
        }
        ++it;
      }
    }



    XML::XML( const XML& rhsXML )
    {
      CopyFrom( rhsXML );
    }



    XML& XML::operator=( const XML& rhsXML )
    {
      CopyFrom( rhsXML );
      return *this;
    }



    XML::XML( bool bCheckTypes ) :
      m_pCurrentElement( NULL ),
      m_pLoadingRootElement( NULL ),
      m_LoadStackDepth( 0 ),
      m_CheckTypes( bCheckTypes )
    {

      if ( m_CheckTypes )
      {
        m_KnownTypes["xml"] = NULL;
        m_KnownTypes["xs:schema"] = NULL;
        m_KnownTypes["xs:include"] = NULL;
        m_KnownTypes["xs:element"] = NULL;
        m_KnownTypes["xs:attribute"] = NULL;
        m_KnownTypes["xs:attributeGroup"] = NULL;
        m_KnownTypes["xs:anyAttribute"] = NULL;
        m_KnownTypes["xs:annotation"] = NULL;
        m_KnownTypes["xs:documentation"] = NULL;
        m_KnownTypes["xs:restriction"] = NULL;
        m_KnownTypes["xs:extension"] = NULL;
        m_KnownTypes["xs:pattern"] = NULL;
        m_KnownTypes["xs:complexContent"] = NULL;
        m_KnownTypes["xs:simpleContent"] = NULL;
        m_KnownTypes["xs:complexType"] = NULL;
        m_KnownTypes["xs:simpleType"] = NULL;
        m_KnownTypes["xs:any"] = NULL;
        m_KnownTypes["xs:sequence"] = NULL;
        m_KnownTypes["xs:length"] = NULL;
        m_KnownTypes["xs:enumeration"] = NULL;
        m_KnownTypes["xs:totalDigits"] = NULL;
        m_KnownTypes["xs:fractionDigits"] = NULL;
        m_KnownTypes["xs:minInclusive"] = NULL;
        m_KnownTypes["xs:maxInclusive"] = NULL;
        m_KnownTypes["xs:minLength"] = NULL;
        m_KnownTypes["xs:maxLength"] = NULL;
      }
    }

    XML::~XML()
    {

      Clear();

    }



    void XML::CopyFrom( const XML& xmlRhs )
    {
      Clear();
      m_LoadingFromFileName  = xmlRhs.m_LoadingFromFileName;
      m_CheckTypes             = xmlRhs.m_CheckTypes;

      m_RootElement.Clear();

      m_RootElement.m_Attributes = xmlRhs.m_RootElement.m_Attributes;

      XMLElement::tListElements::iterator   itElements( m_RootElement.m_Childs.begin() );
      while ( itElements != m_RootElement.m_Childs.end() )
      {
        XMLElement*   pChild = *itElements;

        XMLElement*   pClonedChild = pChild->Clone();

        m_RootElement.InsertChild( pClonedChild );

        ++itElements;
      }
    }



    void XML::Clear()
    {
      XMLElement::tListElements::iterator   it( m_RootElement.m_Childs.begin() );
      while ( it != m_RootElement.m_Childs.end() )
      {
        delete *it;

        ++it;
      }
      m_RootElement.m_Childs.clear();
      m_SystemElement = XMLElement();
      m_DocType.erase();
    }



    void XML::AddComment( const GR::String& Comment )
    {
      m_Comments.push_back( Comment );
    }



    GR::String XML::Comment( size_t Index ) const
    {
      if ( Index >= m_Comments.size() )
      {
        return GR::String();
      }
      std::list<GR::String>::const_iterator    it( m_Comments.begin() );
      std::advance( it, Index );
      return *it;
    }



    void XML::RemoveComment( size_t Index )
    {
      if ( Index >= m_Comments.size() )
      {
        return;
      }
      std::list<GR::String>::iterator    it( m_Comments.begin() );
      std::advance( it, Index );
      m_Comments.erase( it );
    }



    size_t XML::CommentCount() const
    {
      return m_Comments.size();
    }



    bool XML::Load( IIOStream& ioIn )
    {
      Clear();
      m_pCurrentElement     = NULL;
      m_pLoadingRootElement = NULL;

      if ( !ioIn.IsGood() )
      {
        return false;
      }

      //dh::Log( "XML::Load Loading file %s", szFileName );

      // TODO - gibt es nicht mehr
      //m_LoadingFromFileName = szFileName;

      size_t      size = (size_t)ioIn.GetSize();

      //dh::Log( "XML::Load Grˆsse %d", dwSize );


      char*         pDataBlock = new char[size];

      ioIn.ReadBlock( pDataBlock, size );
      ioIn.Close();
      
      //dh::Log( "XML::Load Bytes geladen %d", dwBytesRead );

      m_LoadStackDepth++;

      /*
#ifdef UNICODE

      if ( dwSize >= 2 )
      {
        if ( ( *(GR::u16*)( pDataBlock ) == 0xFEFF )
        ||   ( IsTextUnicode( pDataBlock, (int)dwSize, NULL ) ) )
        {
          // war auch wirklich unicode
          dwSize /= 2;
        }
        else
        {
          // wir ignorieren erstmal die Mˆglichkeit des verkehrten Byte Orderings
          // ASCII
          GR::WString      Temp;

          GR::String       SingleTemp( pDataBlock, dwSize );

          Temp = GR::Convert::ToStringW( SingleTemp );

          delete[] pDataBlock;
          pDataBlock = ( char* )new GR::Char[strTemp.length() + 1];

          memcpy( pDataBlock, Temp.c_str(), ( Temp.length() + 1 ) * 2 );
        }
      }
    #else*/
      if ( size >= 2 )
      {
        if ( *(GR::u16*)( pDataBlock ) == 0xFEFF )
        {
          // Unicode drin
          GR::string        Temp;

          GR::WString       wString;

          wString.append( (wchar_t*)( pDataBlock + 2 ), ( size - 2 ) / 2 );

          Temp = GR::Convert::ToString( wString );

          delete[] pDataBlock;
          pDataBlock = ( char* )new GR::Char[Temp.length() + 1];

          memcpy( pDataBlock, Temp.c_str(), Temp.length() + 1 );

          size = ( size - 2 ) / 2;
        }
      }
    //#endif

      if ( !_Parse( (GR::Char*)pDataBlock, size ) )
      {
        m_LoadStackDepth--;
        delete m_pLoadingRootElement;
        m_pLoadingRootElement = NULL;
        delete[] pDataBlock;
        return false;
      }

      delete[] pDataBlock;

      if ( m_pLoadingRootElement )
      {
        m_RootElement.InsertChild( m_pLoadingRootElement );
        m_pLoadingRootElement = NULL;
      }

      m_LoadStackDepth--;

      return true;
    }



    bool XML::Load( const GR::Char* FileName )
    {
      GR::IO::FileStream  ioIn( FileName );

      return Load( ioIn );
    }



    bool XML::_Parse( const GR::Char* String, size_t Length )
    {
      GR::u32           pos = 0;

      GR::String    Line;

      bool              insideBrackets = false;

      bool              insideCDataTag = false;

      GR::Char*         pData = (GR::Char*)String;

      GR::Char*         pTagStart = NULL;

      m_TempTagContent.erase();

      while ( pos < Length )
      {
        if ( ( *pData == '!' )
        &&   ( insideBrackets )
        &&   ( pTagStart == pData )
        &&   ( pos + 8 <= Length )
        &&   ( pData[0] == '!' )
        &&   ( pData[1] == '[' )
        &&   ( pData[2] == 'C' )
        &&   ( pData[3] == 'D' )
        &&   ( pData[4] == 'A' )
        &&   ( pData[5] == 'T' )
        &&   ( pData[6] == 'A' )
        &&   ( pData[7] == '[' ) )
        {
          // ein CData-Tag!
          insideCDataTag = true;
        }
        else if ( ( *pData == '!' )
        &&        ( insideBrackets )
        &&        ( pTagStart == pData ) )
        {
          // ein DTD-Element (Kommentar?)
          // einfach ¸berlesen
          int   EckBracketNestLevel = 0;

          while ( pos < Length )
          {
            if ( *pData == '[' )
            {
              ++EckBracketNestLevel;
            }
            else if ( *pData == ']' )
            {
              --EckBracketNestLevel;
            }
            else if ( *pData == '>' )
            {
              if ( EckBracketNestLevel == 0 )
              {
                // Ende des Tags
                insideBrackets = false;

                GR::Char*    pSpace = pTagStart;
                while ( pSpace != pData )
                {
                  if ( *pSpace == ' ' )
                  {
                    GR::String     dtdTag;
                    dtdTag.append( pTagStart, pSpace - pTagStart );

                    if ( dtdTag == "!DOCTYPE" )
                    {
                      m_DocType.assign( pSpace + 1, pData - pSpace - 1 );
                    }
                    //goto tag_done;
                  }
                  ++pSpace;
                }

                GR::String   tagContent;

                tagContent.assign( pTagStart, pData - pTagStart );
                GR::Strings::XMLElement*    pElement = new GR::Strings::XMLElement( "", tagContent );
                InsertChild( pElement );
                goto tag_done;
              }
            }

            ++pos;
            ++pData;
          }
          /*
          GR::String       Comment;

          Comment.append( pTagStart, pData );
          AddComment( Comment );*/

        }

        if ( ( *pData == ']' )
        &&   ( insideCDataTag )
        &&   ( pos + 1 <= Length )
        &&   ( pData[1] == ']' ) )
        {
          insideCDataTag = false;
          pos++;
          ++pData;
          continue;
        }
        if ( insideCDataTag )
        {
          ++pData;
          ++pos;
          continue;
        }
        if ( *pData == '<' )
        {
          if ( insideBrackets )
          {
            // Parse Error
            GR::String   Tag;

            Tag.assign( pTagStart, pData - pTagStart );

            dh::Log( "Parse Error: Tag not closed %s", Tag.c_str() );
            return false;
          }
          insideBrackets = true;
          pTagStart = pData + 1;
        }
        else if ( *pData == '>' )
        {
          if ( insideBrackets )
          {
            insideBrackets = false;

            GR::String   Tag;

            Tag.assign( pTagStart, pData - pTagStart );
            /*
            char*         pTag = new char[pData - pTagStart + 1];

            memcpy( pTag, pTagStart, pData - pTagStart + 1 );
            pTag[pData - pTagStart] = 0;

            GR::String   Tag = pTag;
            delete[] pTag;
            */

            //dh::Log( "Parse Tag (%s)", Tag.c_str() );
            if ( !ParseTag( Tag ) )
            {
              dh::Log( "Error parsing tag (%s)", Tag.c_str() );
              return false;
            }

            m_TempTagContent.erase();
          }
          else if ( (GR::u8)*pData >= 32 )
          {
            m_TempTagContent += *pData;
          }
        }
        else if ( !insideBrackets )
        {
          if ( (GR::u8)*pData >= 32 )
          {
            m_TempTagContent += *pData;
          }
          else
          {
            m_TempTagContent += (GR::Char)*pData;
          }
        }

        tag_done:;
        ++pData;
        ++pos;
      }

      m_TempTagContent = GR::Strings::TrimRight( m_TempTagContent, "\r\n" );
      m_TempTagContent = GR::Strings::TrimRight( m_TempTagContent, 0x0a );
      m_TempTagContent = GR::Strings::TrimRight( m_TempTagContent, 0x0d );
      m_TempTagContent = GR::Strings::Trim( m_TempTagContent );

      if ( m_pCurrentElement != NULL )
      {
        dh::Log( "Missing closing tag for %s", m_pCurrentElement->Type().c_str() );
        return false;
      }
      if ( !m_TempTagContent.empty() )
      {
        dh::Log( "TempTag Content is not empty" );
        return false;
      }
      return true;
    }



    /*
    int XML::ParseDTDTag( const GR::String& XMLText, int nextContentPos )
    {
      int     iTagStart = xmlText.indexOf( "<!", nextContentPos );
      if ( iTagStart == -1 )
      {
        return -1;
      }
      // einfach ¸berlesen
      int   tagEnd = xmlText.indexOf( '>', iTagStart );
      int   bracketStart = xmlText.indexOf( '[', iTagStart );
      if ( bracketStart == -1 )
      {
        // da sind keine [] enthalten
        return tagEnd + 1;
      }
      if ( tagEnd < bracketStart )
      {
        // da ist direkt ein Ende
        return tagEnd + 1;
      }
      // verschachtelte DTD-Elemente
      int   bracketEnd = xmlText.indexOf( ']', bracketStart );
      if ( bracketEnd == -1 )
      {
        return -1;
      }
      int   tagClose = xmlText.indexOf( '>', bracketEnd );
      if ( tagClose == bracketEnd + 1 )
      {
        return tagClose + 1;
      }
      return -1;
    }
    */



    bool XML::ParseTag( const GR::String& ParamTag )
    {
      if ( ParamTag.empty() )
      {
        dh::Log( "XML::ParseTag empty tag" );
        return false;
      }

      // Content trimmen
      while ( ( !m_TempTagContent.empty() )
      &&      ( m_TempTagContent.at( 0 ) == ' ' ) )
      {
        m_TempTagContent.erase( 0, 1 );
      }
      while ( ( !m_TempTagContent.empty() )
      &&      ( m_TempTagContent.find_last_of( ' ' ) == m_TempTagContent.length() - 1 ) )
      {
        m_TempTagContent.resize( m_TempTagContent.length() - 1 );
      }
      m_TempTagContent = GR::Strings::TrimRight( m_TempTagContent, 0x0a );
      m_TempTagContent = GR::Strings::TrimRight( m_TempTagContent, 0x0d );

      GR::String   Tag = ParamTag;

      bool      systemTag = false;
      bool      closingTag = false;
      bool      autoClosingTag = false;

      if ( Tag.at( 0 ) == '?' )
      {
        // System-Tag
        systemTag = true;

        Tag = DoSubstr( Tag, 1, Tag.length() - 2 );

        if ( Tag.find( "xml" ) != 0 )
        {
          // das muﬂ ?XML... sein!
          dh::Log( "XML::ParseTag no xml signature" );
          return false;
        }
      }

      if ( GR::Strings::StartsWith( Tag, "!--" ) )
      {
        if ( !GR::Strings::EndsWith( Tag, "--" ) )
        {
          dh::Log( "XML::ParseTag Comment tag not ending as expected" );
          return false;
        }
        GR::Strings::XMLElement*    pElement = new GR::Strings::XMLElement( "", Tag );
        InsertChild( pElement );
        m_pCurrentElement = m_pCurrentElement->Parent();
        return true;
      }

      if ( Tag.at( 0 ) == '/' )
      {
        closingTag = true;
        Tag = DoSubstr( Tag, 1 );
      }
      if ( Tag.at( Tag.length() - 1 ) == '/' )
      {
        autoClosingTag = true;
        Tag = DoSubstr( Tag, 0, Tag.length() - 1 );
      }

      if ( Tag.find( "![CDATA[" ) == 0 )
      {
        // ein CData-Tag
        if ( DoSubstr( Tag, Tag.length() - 2, 2 ) != "]]" )
        {
          // CData-Abschluss
          dh::Log( "XML::ParseTag CDATA not closed correctly" );
          return false;
        }

        if ( m_pCurrentElement )
        {
          m_pCurrentElement->CDATAContent( DoSubstr( Tag, 8, Tag.length() - 10 ) );
          return true;
        }
        // CData-Abschluss
        dh::Log( "XML::ParseTag CDATA without enclosing element" );
        return false;
      }

      XMLElement*    pElement = new XMLElement();

      size_t      pos = 0;
      size_t      startPos = 0;
      bool        apostrophe = false;
      GR::Char    cStartApostrophe = 32;

      while ( pos < Tag.length() )
      {
        if ( ( Tag.at( pos ) == '"' )
        ||   ( Tag.at( pos ) == 39 ) )   // '
        {
          if ( !apostrophe )
          {
            cStartApostrophe = Tag.at( pos );
            apostrophe = true;
          }
          else if ( Tag.at( pos ) == cStartApostrophe )
          {
            // nur wenn Anfangs- und End-Apostroph ¸bereinstimmen
            apostrophe = false;
          }
        }
        else if ( ( !apostrophe )
        &&        ( Tag.at( pos ) == ' ' ) )
        {
          if ( startPos == 0 )
          {
            pElement->m_Type = DoSubstr( Tag, 0, pos );
            startPos = pos + 1;
            while ( ( startPos < Tag.length() )
            &&      ( Tag.at( startPos ) == ' ' ) )
            {
              ++startPos;
              ++pos;
            }

            //dh::Log( "Tag Type %s", pElement->m_Type.c_str() );
          }
          else
          {
            while ( ( pos < Tag.length() )
            &&      ( Tag.at( pos ) == ' ' ) )
            {
              ++pos;
            }
            pos--;
            size_t   separatorPos = Tag.find( '=', startPos );
            if ( separatorPos == GR::String::npos )
            {
              dh::Log( "no param set (kein '=') " );
              delete pElement;
              return false;
            }
            if ( separatorPos < startPos )
            {
              dh::Log( "no param set ('=' zuweit weg)" );
              delete pElement;
              return false;
            }

            GR::String     ParamName  = DoSubstr( Tag, startPos, separatorPos - startPos );
            GR::String     ParamValue = DoSubstr( Tag, separatorPos + 1, pos - separatorPos - 1 );

            //dh::Log( "Tag Param (%s) Value (%s)", ParamName.c_str(), ParamValue.c_str() );

            // Apostrophe entfernen
            if ( ( ParamName.length() >= 2 )
            &&   ( ParamName.at( 0 ) == '"' )
            &&   ( ParamName.at( ParamName.length() - 1 ) == '"' ) )
            {
              ParamName = DoSubstr( ParamName, 1, ParamName.length() - 2 );
            }
            if ( ( ParamValue.length() >= 2 )
            &&   ( ParamValue.at( 0 ) == '"' )
            &&   ( ParamValue.at( ParamValue.length() - 1 ) == '"' ) )
            {
              ParamValue = DoSubstr( ParamValue, 1, ParamValue.length() - 2 );
            }
            if ( ( ParamName.length() >= 2 )
            &&   ( ParamName.at( 0 ) == 39 )    // '
            &&   ( ParamName.at( ParamName.length() - 1 ) == 39 ) )
            {
              ParamName = DoSubstr( ParamName, 1, ParamName.length() - 2 );
            }
            if ( ( ParamValue.length() >= 2 )
            &&   ( ParamValue.at( 0 ) == 39 )
            &&   ( ParamValue.at( ParamValue.length() - 1 ) == 39 ) )
            {
              ParamValue = DoSubstr( ParamValue, 1, ParamValue.length() - 2 );
            }

            //dh::Log( "Tag Cleaned Param (%s) Value (%s)", ParamName.c_str(), ParamValue.c_str() );

            pElement->AddAttribute( ParamName, XML::XMLDecode( ParamValue ) );
            startPos = pos + 1;
          }
        }

        ++pos;
      }

      if ( apostrophe )
      {
        dh::Log( "Parse error - no closing '\"'" );
        delete pElement;
        return false;
      }

      if ( startPos == 0 )
      {
        pElement->m_Type = Tag;
      }
      else
      {
        if ( ( pos < Tag.length() )
        &&   ( Tag.at( pos ) == ' ' ) )
        {
          while ( Tag.at( pos ) == ' ' )
          {
            ++pos;
          }
          pos--;
        }
        if ( pos > startPos )
        {
          size_t   separatorPos = (int)Tag.find( '=', startPos );
          if ( separatorPos == GR::String::npos )
          {
            dh::Log( "no param set (kein '=') " );
            delete pElement;
            return false;
          }
          if ( separatorPos > pos )
          {
            dh::Log( "no param set ('=' zuweit weg)" );
            delete pElement;
            return false;
          }

          GR::String     ParamName  = DoSubstr( Tag, startPos, separatorPos - startPos );
          GR::String     ParamValue = DoSubstr( Tag, separatorPos + 1, pos - separatorPos - 1 );

          if ( ( ParamName.length() >= 2 )
          &&   ( ParamName.at( 0 ) == '"' )
          &&   ( ParamName.at( ParamName.length() - 1 ) == '"' ) )
          {
            ParamName = DoSubstr( ParamName, 1, ParamName.length() - 2 );
          }
          if ( ( ParamValue.length() >= 2 )
          &&   ( ParamValue.at( 0 ) == '"' )
          &&   ( ParamValue.at( ParamValue.length() - 1 ) == '"' ) )
          {
            ParamValue = DoSubstr( ParamValue, 1, ParamValue.length() - 2 );
          }
          if ( ( ParamName.length() >= 2 )
          &&   ( ParamName.at( 0 ) == 39 )
          &&   ( ParamName.at( ParamName.length() - 1 ) == 39 ) )
          {
            ParamName = DoSubstr( ParamName, 1, ParamName.length() - 2 );
          }
          if ( ( ParamValue.length() >= 2 )
          &&   ( ParamValue.at( 0 ) == 39 )
          &&   ( ParamValue.at( ParamValue.length() - 1 ) == 39 ) )
          {
            ParamValue = DoSubstr( ParamValue, 1, ParamValue.length() - 2 );
          }

          pElement->AddAttribute( ParamName, ParamValue );
          startPos = pos + 1;
        }
      }

      if ( !XSIsKnownType( pElement ) )
      {
        dh::Log( "Unknown element type %s", pElement->Type().c_str() );
        delete pElement;
        return false;
      }

      if ( systemTag )
      {
        // ?XML... usw. wird nicht wirklich gebraucht
        m_SystemElement = *pElement;
        m_SystemElement.SystemTag();
        delete pElement;
        return true;
      }
      else if ( m_pCurrentElement == NULL )
      {
        m_pCurrentElement = pElement;

        m_pLoadingRootElement = pElement;
        XSAddKnownType( m_pCurrentElement );

        if ( autoClosingTag )
        {
          m_pCurrentElement = NULL;
        }
      }
      else if ( autoClosingTag )
      {
        m_pCurrentElement->InsertChild( pElement );
        XSAddKnownType( pElement );

        //m_pCurrentElement = m_pCurrentElement->Parent();


        // das Abschluﬂtag wird ja nicht wirklich gebraucht
        //delete pElement;
        pElement = NULL;
      }
      else if ( closingTag )
      {
        // pr¸fen, ob opentag stimmt
        if ( m_pCurrentElement->m_Type != pElement->m_Type )
        {
          dh::Log( "mismatching closed tag! (%s != %s)", m_pCurrentElement->m_Type.c_str(), pElement->m_Type.c_str() );
          delete pElement;
          return false;
        }
        if ( !m_pCurrentElement->IsCDataContent() )
        {
          bool    bValidContent = false;
          for ( size_t i = 0; i < m_TempTagContent.length(); ++i )
          {
            if ( m_TempTagContent.at( i ) != ' ' )
            {
              bValidContent = true;
              break;
            }
          }
          if ( bValidContent )
          {
            m_pCurrentElement->Content( m_pCurrentElement->GetContent() + m_TempTagContent, true );
          }
          m_TempTagContent.erase();
        }
        m_pCurrentElement = m_pCurrentElement->Parent();


        // das Abschluﬂtag wird ja nicht wirklich gebraucht
        delete pElement;
        pElement = NULL;
      }
      else
      {
        // ein neues Sub-Element
        if ( m_pCurrentElement )
        {
          m_pCurrentElement->Content( m_pCurrentElement->GetContent() + m_TempTagContent, true );
          m_TempTagContent.erase();
        }

        XSAddKnownType( pElement );

        m_pCurrentElement->InsertChild( pElement );
        m_pCurrentElement = pElement;
      }

      /*
      // this was required for some empty tag thingies
      if ( autoClosingTag )
      {
        m_pCurrentElement = NULL;
      }*/

      if ( pElement )
      {
        if ( pElement->Type() == "xs:include" )
        {
          // XS-Schema Include
          GR::String     includeFile = pElement->Attribute( "schemaLocation" );

          if ( !includeFile.empty() )
          {
            GR::String   FileName = Path::Append( Path::RemoveFileSpec( m_LoadingFromFileName ), includeFile );

            if ( !XSInclude( FileName ) )
            {
              dh::Log( "xs::include (%s) failed", pElement->Attribute( "schemaLocation" ).c_str() );
              delete pElement;
              return false;
            }
          }
        }
      }

      return true;
    }



    bool XML::XSInclude( const GR::String& FileName )
    {
      if ( m_ParsedFiles.find( FileName ) != m_ParsedFiles.end() )
      {
        // schon geparst
        return true;
      }

      XML      secondParser;

      if ( !secondParser.Load( FileName.c_str() ) )
      {
        return false;
      }
      m_ParsedFiles.insert( FileName );

      secondParser.m_RootElement.MoveChilds( &m_RootElement );

      return true;
    }



    void XML::Dump()
    {
      dh::Log( "XML-Tree-Dump:\n============\n" );
      m_RootElement.DumpElement( "" );
    }



    void XML::XSAddKnownType( XMLElement* pElement )
    {
      if ( !m_CheckTypes )
      {
        return;
      }

      if ( pElement->Type() == "xs:element" )
      {
        GR::String   param = pElement->Attribute( "name" );
        if ( !param.empty() )
        {
          dh::Log( "AddKnownType %s", param.c_str() );
          m_KnownTypes[param] = pElement;
        }
      }
      else if ( pElement->Type() == "xs:simpleType" )
      {
        GR::String   param = pElement->Attribute( "name" );
        if ( !param.empty() )
        {
          dh::Log( "AddKnownType %s", param.c_str() );
          m_KnownTypes[param] = pElement;
        }
      }
    }



    void XML::InsertChild( XMLElement* pElement )
    {
      m_RootElement.InsertChild( pElement );
    }



    XMLElement* XML::AddElement( const GR::String& ElementName, const GR::String& Content )
    {
      XMLElement*  pElement = new XMLElement( ElementName, Content );

      InsertChild( pElement );

      return pElement;
    }



    void XML::AddElement( XMLElement* pParentElement, XMLElement* pElement )
    {
      XSAddKnownType( pElement );
      if ( pParentElement == NULL )
      {
        if ( m_pLoadingRootElement == NULL )
        {
          m_pLoadingRootElement = pElement;

          if ( m_LoadStackDepth == 0 )
          {
            m_RootElement.InsertChild( pElement );
          }
          return;
        }
        m_pLoadingRootElement->InsertChild( pElement );
        return;
      }
      pParentElement->InsertChild( pElement );
    }



    bool XML::Save( IIOStream& ioOut, const GR::u32 IndentSpaces, bool UseLineBreaks )
    {
      if ( m_RootElement.ChildCount() == 0 )
      {
        return false;
      }

      if ( !ioOut.IsGood() )
      {
        return false;
      }

      GR::String   Result = ToXML( IndentSpaces, UseLineBreaks );

      ioOut.WriteBlock( Result.c_str(), (GR::u32)GR::Strings::Size( Result ) );

      return true;
    }



    bool XML::Save( const GR::Char* FileName, const GR::u32 IndentSpaces, bool UseLineBreaks )
    {
      GR::IO::FileStream     MyFile( FileName, IIOStream::OT_WRITE_ONLY );

      return Save( MyFile, IndentSpaces, UseLineBreaks );
    }



    GR::String XML::ToXML( const GR::u32 IndentSpaces, bool UseLineBreaks )
    {
      if ( m_RootElement.ChildCount() == 0 )
      {
        return GR::String();
      }

      GR::String     Result;

      if ( !m_SystemElement.Type().empty() )
      {
        Result = XMLFileHeader();
      }

      /*
      if ( !m_DocType.empty() )
      {
        Result += _T( "<!DOCTYPE " ) + m_DocType + _T( ">" ) + m_NewLine;
      }

      std::list<GR::String>::iterator    itComments( m_Comments.begin() );
      while ( itComments != m_Comments.end() )
      {
        Result += _T( "<!-- " ) + *itComments + _T( " -->" ) + m_NewLine;

        ++itComments;
      }*/

      XMLElement::tListElements::iterator   it( m_RootElement.m_Childs.begin() );
      while ( it != m_RootElement.m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        Result += pChild->ToString( IndentSpaces, UseLineBreaks );

        ++it;
      }

      return Result;
    }



    XML::iterator XML::begin()
    {
      if ( m_RootElement.ChildCount() == 0 )
      {
        return end();
      }
      return iterator( m_RootElement.m_Childs.front() );
    }



    XML::iterator XML::end()
    {
      return iterator();
    }



    bool XML::XSIsKnownType( XMLElement* pElement )
    {
      if ( !m_CheckTypes )
      {
        return true;
      }
      if ( pElement == NULL )
      {
        return false;
      }

      tMapKnownTypes::iterator    it( m_KnownTypes.find( pElement->Type() ) );
      if ( it == m_KnownTypes.end() )
      {
        return false;
      }
      return true;
    }



    XMLElement* XML::XSFindType( const GR::String& Type )
    {
      tMapKnownTypes::iterator    it( m_KnownTypes.find( Type ) );
      if ( it == m_KnownTypes.end() )
      {
        return NULL;
      }
      return it->second;
    }



    XMLElement* XML::XSCreateTemplate( const GR::String& Type )
    {
      XMLElement*    pBaseElement = XSFindType( Type );

      if ( pBaseElement == NULL )
      {
        return NULL;
      }

      XMLElement*    pNewElement = new XMLElement();


      pNewElement->m_Type = Type;

      XSCreateTemplateElement( pNewElement, pBaseElement );

      return pNewElement;
    }



    void XML::XSCreateTemplateElement( XMLElement* pClone, XMLElement* pBaseElement )
    {
      XMLElement::tListElements::iterator    it( pBaseElement->m_Childs.begin() );
      while ( it != pBaseElement->m_Childs.end() )
      {
        XMLElement*    pChild = *it;

        if ( pChild->Type() == "xs:element" )
        {
          XMLElement*    pNewChild = new XMLElement();

          pNewChild->m_Type = pChild->Attribute( "name" );

          XSCreateTemplateElement( pNewChild, pChild );

          pClone->m_Childs.push_back( pNewChild );
        }
        else if ( pChild->Type() == "xs:attribute" )
        {
          //if ( pChild->Attribute( "use" ) == "required" )
          {
            GR::String   UsedType;


            XMLElement*    pType = XSFindType( pChild->Attribute( "type" ) );
            if ( pType )
            {
              XML::iterator    itType( pType );

              while ( itType != XML::iterator() )
              {
                XMLElement*    pElement = *itType;

                if ( ( pElement->Level() <= pType->Level() )
                &&   ( pElement != pType ) )
                {
                  break;
                }

                if ( pElement->Type() == "xs:enumeration" )
                {
                  UsedType = pElement->Attribute( "value" );
                  break;
                }

                ++itType;
              }
            }

            pClone->AddAttribute( pChild->Attribute( "name" ), UsedType );
          }
        }
        else
        {
          XSCreateTemplateElement( pClone, pChild );
        }
        ++it;
      }
    }



    bool XML::Parse( const GR::Char* String, size_t iLength )
    {
      Clear();
      if ( String == NULL )
      {
        return false;
      }

      m_LoadStackDepth++;

      if ( !_Parse( String, iLength ) )
      {
        m_LoadStackDepth--;
        delete m_pLoadingRootElement;
        m_pLoadingRootElement = NULL;
        return false;
      }

      if ( m_pLoadingRootElement )
      {
        m_RootElement.InsertChild( m_pLoadingRootElement );
        m_pLoadingRootElement = NULL;
      }

      m_LoadStackDepth--;

      return true;
    }



    void XML::SetXMLHeader( const GR::String& Header )
    {
      m_XMLFileHeader = Header;
    }



    void XML::SetLineBreak( const GR::String& LineBreak )
    {
      m_NewLine = LineBreak;
    }



    XMLElement* XML::FirstChild() const
    {
      return m_RootElement.FirstChild();
    }



    GR::String XML::XSTimeStamp()
    {
      GR::String     Result;

    #ifdef _WIN32
      SYSTEMTIME      sTime;

      TIME_ZONE_INFORMATION   tzI;

      GetLocalTime( &sTime );

      GetTimeZoneInformation( &tzI );

      Result = Misc::Format( "%01:4%-%02:2%-%03:2%T%04:2%:%05:2%:%06:2%" )
                                 << sTime.wYear
                                 << sTime.wMonth
                                 << sTime.wDay
                                 << sTime.wHour
                                 << sTime.wMinute
                                 << sTime.wSecond;

      if ( tzI.Bias < 0 )
      {
        Result += Misc::Format( "-%01:2%:%02:2%" )
                                  << (GR::i64)abs( tzI.Bias / 60 )
                                  << (GR::i64)abs( tzI.Bias ) % 60;
      }
      else
      {
        Result += Misc::Format( "+%01:2%:%02:2%" )
                                    << (GR::i64)tzI.Bias / 60
                                    << (GR::i64)tzI.Bias % 60;
      }
    #else
      // TODO !
    #endif

      return Result;
    }



    bool XML::CheckingTypes() const
    {
      return m_CheckTypes;
    }



    GR::String XML::XMLFileHeader()
    {
      return m_SystemElement.ToString();
      //return m_XMLFileHeader + XML::m_NewLine;
    }



    XMLElement* XML::FindByType( const GR::String& TypeName )
    {
      XML::iterator  it( begin() );
      while ( it != end() )
      {
        XMLElement*    pChild = *it;

        XMLElement*    pFound = pChild->FindByType( TypeName );
        if ( pFound )
        {
          return pFound;
        }
        ++it;
      }
      return NULL;
    }



    XMLElement* XML::FindByTypeRecursive( const GR::String& TypeName )
    {
      XML::iterator  it( begin() );
      while ( it != end() )
      {
        XMLElement*    pChild = *it;

        XMLElement*    pFound = pChild->FindByTypeRecursive( TypeName );
        if ( pFound )
        {
          return pFound;
        }
        ++it;
      }
      return NULL;
    }



    GR::String XML::XMLEncode( const GR::String& ToEncode )
    {
      GR::String       ReturnContent = ToEncode;

      // & muﬂ als erstes!
      ReturnContent = GR::Strings::Replace( ReturnContent, "&", "&amp;" );
      ReturnContent = GR::Strings::Replace( ReturnContent, "<", "&lt;" );
      ReturnContent = GR::Strings::Replace( ReturnContent, ">", "&gt;" );
      ReturnContent = GR::Strings::Replace( ReturnContent, "\'", "&apos;" );
      ReturnContent = GR::Strings::Replace( ReturnContent, "\"", "&quot;" );

      return ReturnContent;
    }



    GR::String XML::XMLDecode( const GR::String& ToDecode )
    {
      GR::String       ReturnContent = ToDecode;

      ReturnContent = GR::Strings::ReplaceWithUpperCaseCompare( ReturnContent, "&LT;", "<" );
      ReturnContent = GR::Strings::ReplaceWithUpperCaseCompare( ReturnContent, "&AMP;", "&" );
      ReturnContent = GR::Strings::ReplaceWithUpperCaseCompare( ReturnContent, "&GT;", ">" );
      ReturnContent = GR::Strings::ReplaceWithUpperCaseCompare( ReturnContent, "&APOS;", "\'" );
      ReturnContent = GR::Strings::ReplaceWithUpperCaseCompare( ReturnContent, "&QUOT;", "\"" );

      return ReturnContent;
    }



    void XML::DocType( const GR::String& DocType )
    {
      m_DocType = DocType;
    }



    GR::String XML::DocType() const
    {
      return m_DocType;
    }

  }
}



