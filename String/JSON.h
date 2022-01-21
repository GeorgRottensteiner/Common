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
    namespace JSON
    {
      namespace ElementType
      {
        enum Value
        {
          Null = 0,
          Bool,
          Number,
          String,
          Array,
          Object
        };
      }

      class Element
      {
        protected:

          GR::String                    m_Name;
          GR::String                    m_Content;
          ElementType::Value            m_Type;

          Element*                      m_pParent;

          GR::String                    ContentToString();
          

        public:

          typedef std::list<Element*>   tListElements;

          tListElements                 m_Childs;


          void                          InsertChild( Element* pChild );
          void                          DeleteChild( Element* pChild );
          void                          Clear();

          GR::String                    ToJSON( const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true ) const;
          GR::String                    ValueToJSON( const GR::u32 IndentSpaces, bool UseLineBreaks ) const;

          ElementType::Value            Type() const;
          GR::String                    GetContent() const;
          void                          Content( const GR::String& Content );

          void                          Name( const GR::String& Name );
          GR::String                    Name() const;

          GR::u32                       Level() const;

          Element*                      FirstChild() const;
          size_t                        ChildCount() const;
          Element*                      Parent() const;

          Element*                      Clone( bool DeepClone = true );    // true = alle Childs mitklonen, false = nur Attribute


          Element();
          Element( const Element& RHS );
          Element( const ElementType::Value Type, const GR::String& Content = GR::String() );
          Element& operator= ( const Element& RHS );
          virtual ~Element();

      };



      class Parser
      {
        public:

          class iterator : public std::iterator<std::bidirectional_iterator_tag,Element*>
          {

            protected:

              Element*                        m_pElement;


            public:

              iterator( Element* pElement = NULL ) :
                m_pElement( pElement )
              {
              }

              iterator( const iterator& rhs ) :
                m_pElement( rhs.m_pElement )
              {
              }

              Element* operator->() const 
              { 
                return m_pElement; 

              }
              Element* operator*()  const 
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
                Element::tListElements::iterator    it( m_pElement->Parent()->m_Childs.begin() );
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
                Element::tListElements::iterator    it( m_pElement->Parent()->m_Childs.begin() );
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
                Element*    pParent = m_pElement->Parent();
                Element*    pElement = m_pElement;

                while ( pParent )
                {
                  Element::tListElements::iterator    it( pParent->m_Childs.begin() );
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
                Element*    pParent   = m_pElement->Parent();
                Element*    pElement  = m_pElement;

                Element::tListElements::iterator    it( pParent->m_Childs.begin() );
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

              friend class Element;

          };

          static GR::String             m_NewLine;

          GR::String                    m_LoadingFromFileName;

          Element                       m_RootElement;


          Parser();
          ~Parser();

          Parser( const Parser& rhsJSON );
          Parser&       operator=( const Parser& rhsJSON );


          void                          Clear();

          bool                          Load( const GR::String& FileName );
          bool                          Load( IIOStream& ioIn );
          bool                          Parse( const GR::Char* String, size_t iLength );

          void                          InsertChild( Element* pElement );
          Element*                      FirstChild() const;

          void                          CopyFrom( const Parser& JSONRhs );

          GR::String                    ToJSON( const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );
          bool                          Save( const GR::String& FileName, const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );
          bool                          Save( IIOStream& ioOut, const GR::u32 IndentSpaces = 2, bool UseLineBreaks = true );

          iterator                      begin();
          iterator                      end();

          void                          SetLineBreak( const GR::String& LineBreak );

          static GR::String             JSONEncode( const GR::String& ToEncode );
          static GR::String             JSONDecode( const GR::String& ToEncode );


        protected:

          bool                          _ParseObject( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement );
          bool                          _ParseArray( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement );
          bool                          _ParseValue( const GR::Char* pString, size_t& CurPos, size_t Length, Element*& pElement );
          bool                          _LookAheadIs( const GR::Char* pString, size_t& CurPos, size_t Length, const GR::String& Text );

          bool                          _IsWhiteSpace( const GR::Char Char );
          void                          _SkipWhiteSpace( const GR::Char* pString, size_t& CurPos, size_t Length );
          bool                          _IsNumberChar( const GR::Char Char );

          bool                          _ParseString( const GR::Char* pString, size_t& CurPos, size_t Length, GR::String& ResultingString );
          bool                          _ParseNumber( const GR::Char* pString, size_t& CurPos, size_t Length, GR::String& ResultingString );


      };

    }

  }
}
