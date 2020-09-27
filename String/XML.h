#pragma once

#include <string>
#include <list>
#include <map>
#include <set>

#include <Interface/IIOStream.h>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Strings
  {

    struct tXMLAttribute
    {
      GR::String     AttributeName;
      GR::String     AttributeValue;

      tXMLAttribute( const GR::String& AttrName, const GR::String& AttrValue ) :
        AttributeName( AttrName ),
        AttributeValue( AttrValue )
      {
      }
    };

    class XMLElement
    {

      protected:

        GR::String                m_Content;

        bool                          m_IsCDataContent;
        bool                          m_IsSystemTag;

        XMLElement*                   m_pParent;

        GR::String                ContentToString();


      public:

        typedef std::list<tXMLAttribute>    tListAttributes;

        typedef std::list<XMLElement*>     tListElements;


        tListAttributes               m_Attributes;  

        tListElements                 m_Childs;

        GR::String                m_Type;


        void                          AddAttribute( const GR::String& AttributeName, const GR::String& AttributeValue );
        void                          SetAttribute( const GR::String& AttributeName, const GR::String& AttributeValue );
        void                          RemoveAllAttributes();

        size_t                        AttributeCount() const;
        GR::String                AttributeName( size_t Index ) const;
        GR::String                AttributeValue( size_t Index ) const;

        void                          InsertChild( XMLElement* pChild );
        XMLElement*                   AddElement( const GR::String& ElementName, const GR::String& Content = GR::String() );

        void                          DeleteChild( XMLElement* pChild );
        void                          MoveChilds( XMLElement* pXMLTarget );
        void                          Clear();

        GR::String                ToString( const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );
        GR::String                ToXML( const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );

        bool                          IsComment() const;
        bool                          IsSystemTag() const;
        void                          SystemTag( bool IsSystemTag = true );
        GR::String                Type() const;
        void                          Type( const GR::String& Type );
        GR::String                GetContent( bool PrepareForFile = false ) const;
        GR::String                GetContentOfSubTags( const GR::String& SubTags, bool PrepareForFile = false );
        void                          Content( const GR::String& Content, bool ParseHTMLTags = false );
        void                          CDATAContent( const GR::String& Content );
        bool                          IsCDataContent() const;

        void                          DumpElement( const GR::String& Indent = GR::String() );

        GR::String                Attribute( const GR::String& AttributeName ) const;
        bool                          HasAttribute( const GR::String& AttributeName ) const;

        GR::u32                       Level() const;

        XMLElement*                   FirstChild() const;
        size_t                        ChildCount() const;
        XMLElement*                   Parent() const;

        XMLElement*                   Clone( bool DeepClone = true );    // true = alle Childs mitklonen, false = nur Attribute

        XMLElement*                   FindByType( const GR::String& TypeName );

        // searches recursive, tags separated by dots
        XMLElement*                   FindByTypeRecursive( const GR::String& TypeName );


        XMLElement( const XMLElement& RHS );
        XMLElement( const GR::String& Type = GR::String(), const GR::String& Content = GR::String() );
        XMLElement& operator= ( const XMLElement& RHS );
        virtual ~XMLElement();

    };

    class XML
    {

      public:

        class iterator : public std::iterator<std::bidirectional_iterator_tag,XMLElement*>
        {

          protected:

            XMLElement*                        m_pElement;


          public:

            iterator( XMLElement* pElement = NULL ) :
              m_pElement( pElement )
            {
            }

            iterator( const iterator& rhs ) :
              m_pElement( rhs.m_pElement )
            {
            }

            XMLElement* operator->() const 
            { 
              return m_pElement; 

            }
            XMLElement* operator*()  const 
            { 
              return m_pElement; 
            }

            iterator& operator ++() //- pre-inc
            {
              *this = next();
              return *this;
            }
            iterator operator ++(int) //- post-inc
            {
              iterator tmp( *this );
              *this = next();
              return tmp;
            }
            iterator& operator --() //- pre-dec
            {
              *this = previous();
              return *this;
            }
            iterator operator --(int) //- post-dec
            {
              iterator tmp( *this );
              *this = previous();
              return tmp;
            }

            bool operator==( const iterator& rhs ) const 
            { 
              return ( m_pElement == rhs.m_pElement );
            }

            bool operator!=( const iterator& rhs ) const 
            { 
              return ( m_pElement != rhs.m_pElement ); 
            }

            iterator end()
            {
              return iterator();
            }

            iterator first_child()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( m_pElement->m_Childs.empty() )
              {
                return end();
              }
              return iterator( m_pElement->m_Childs.front() );
            }

            iterator last_child()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( m_pElement->m_Childs.empty() )
              {
                return end();
              }
              return iterator( m_pElement->m_Childs.back() );
            }

            iterator next_sibling()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( m_pElement->Parent() == NULL )
              {
                return end();
              }
              XMLElement::tListElements::iterator    it( m_pElement->Parent()->m_Childs.begin() );
              while ( it != m_pElement->Parent()->m_Childs.end() )
              {
                if ( *it == m_pElement )
                {
                  ++it;
                  if ( it == m_pElement->Parent()->m_Childs.end() )
                  {
                    return end();
                  }
                  return iterator( *it );
                }
                ++it;
              }
              return end();
            }

            iterator previous_sibling()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( m_pElement->Parent() == NULL )
              {
                return end();
              }
              XMLElement::tListElements::iterator    it( m_pElement->Parent()->m_Childs.begin() );
              while ( it != m_pElement->Parent()->m_Childs.end() )
              {
                if ( *it == m_pElement )
                {
                  --it;
                  if ( it == m_pElement->Parent()->m_Childs.end() )
                  {
                    return end();
                  }
                  return iterator( *it );
                }
                ++it;
              }
              return end();
            }

            iterator next()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( !m_pElement->m_Childs.empty() )
              {
                return iterator( m_pElement->m_Childs.front() );
              }
              if ( m_pElement->Parent() == NULL )
              {
                return end();
              }
              XMLElement*    pParent = m_pElement->Parent();
              XMLElement*    pElement = m_pElement;

              while ( pParent )
              {
                XMLElement::tListElements::iterator    it( pParent->m_Childs.begin() );
                while ( it != pParent->m_Childs.end() )
                {
                  if ( *it == pElement )
                  {
                    ++it;
                    if ( it == pParent->m_Childs.end() )
                    {
                      // jetzt über die Parents weiter nach oben
                      pElement = pParent;
                      pParent = pParent->Parent();
                      if ( pParent == NULL )
                      {
                        return end();
                      }
                      it = pParent->m_Childs.begin();
                      continue;
                    }
                    return iterator( *it );
                  }
                  ++it;
                }
              }
              return end();
            }

            iterator previous()
            {
              if ( m_pElement == NULL )
              {
                return end();
              }
              if ( m_pElement->Parent() == NULL )
              {
                return end();
              }
              XMLElement*    pParent   = m_pElement->Parent();
              XMLElement*    pElement  = m_pElement;

              XMLElement::tListElements::iterator    it( pParent->m_Childs.begin() );
              while ( it != pParent->m_Childs.end() )
              {
                if ( *it == pElement )
                {
                  --it;
                  if ( it == pParent->m_Childs.end() )
                  {
                    pElement = pParent;
                    pParent = pParent->Parent();
                    it = pParent->m_Childs.begin();
                    continue;
                  }
                  pElement = *it;
                  while ( pElement )
                  {
                    if ( pElement->m_Childs.empty() )
                    {
                      return iterator( *it );
                    }
                    pElement = pElement->m_Childs.back();
                  }
                }

                ++it;
              }
              return end();
            }

            friend class XMLElement;

        };

        typedef std::map<GR::String,XMLElement*>   tMapKnownTypes;
        typedef std::set<GR::String> tSetParsedFiles;

        static tMapKnownTypes         m_KnownTypes;

        static tSetParsedFiles        m_ParsedFiles;

        static GR::String         m_NewLine;

        GR::String                m_LoadingFromFileName;

        GR::String                m_DocType;

        XMLElement                    m_RootElement;

        XMLElement*                   m_pCurrentElement;

        XMLElement*                   m_pLoadingRootElement;

        XMLElement                    m_SystemElement;

        int                           m_LoadStackDepth;


        XML( bool bCheckTypes = false );
        ~XML();

        XML( const XML& rhsXML );
        XML&       operator=( const XML& rhsXML );


        void                          Clear();

        bool                          Load( const GR::Char* FileName );
        bool                          Load( IIOStream& ioIn );
        bool                          Parse( const GR::Char* String, size_t iLength );

        bool                          ParseTag( const GR::String& Tag );
        bool                          XSInclude( const GR::String& FileName );

        void                          InsertChild( XMLElement* pElement );
        void                          AddElement( XMLElement* pParentElement, XMLElement* pElement );
        XMLElement*                   AddElement( const GR::String& ElementName, const GR::String& Content = GR::String() );
        XMLElement*                   FirstChild() const;

        bool                          XSIsKnownType( XMLElement* pElement );
        void                          XSAddKnownType( XMLElement* pElement );
        XMLElement*                   XSFindType( const GR::String& Type );
        XMLElement*                   FindByType( const GR::String& TypeName );
        // searches recursive, tags separated by dots
        XMLElement*                   FindByTypeRecursive( const GR::String& TypeName );

        XMLElement*                   XSCreateTemplate( const GR::String& Type );

        void                          CopyFrom( const XML& xmlRhs );

        void                          Dump();

        GR::String                ToXML( const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );
        bool                          Save( const GR::Char* FileName, const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );
        bool                          Save( IIOStream& ioOut, const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );

        iterator                      begin();
        iterator                      end();

        static GR::String         XSTimeStamp();

        bool                          CheckingTypes() const;

        GR::String                XMLFileHeader();

        void                          AddComment( const GR::String& Comment );
        GR::String                Comment( size_t Index ) const;
        void                          RemoveComment( size_t Index );
        size_t                        CommentCount() const;

        void                          DocType( const GR::String& DocType );
        GR::String                DocType() const;

        void                          SetXMLHeader( const GR::String& Header );
        void                          SetLineBreak( const GR::String& LineBreak );

        static GR::String         XMLEncode( const GR::String& ToEncode );
        static GR::String         XMLDecode( const GR::String& ToEncode );


      protected:

        bool                          m_CheckTypes;     // sollen Typen nur nach Schema erlaubt werden oder generell alle (Tree)

        GR::String                m_TempTagContent;

        std::list<GR::String>     m_Comments;

        static GR::String         m_XMLFileHeader;

        void                          XSCreateTemplateElement( XMLElement* pClone, XMLElement* pBaseElement );
        bool                          _Parse( const GR::Char* String, size_t Length );


    };


  }
}
