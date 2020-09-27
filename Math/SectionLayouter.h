#ifndef MATH_SECTION_LAYOUTER_H
#define MATH_SECTION_LAYOUTER_H

#include <GR/GRTypes.h>

#include <list>


namespace math
{

  //- Unausgegorene Rand-Klasse
  template <class T> class CSectionLayouter
  {

    private:

      struct Node
      {
        Node*       pChild[2];
        GR::tRect   Rectangle;
        T           SectionID;

        Node() :
          SectionID( NULL )
        {
          pChild[0] = NULL;
          pChild[1] = NULL;
        }

        ~Node()
        {
          delete pChild[0];
          delete pChild[1];
        }

        Node* Insert( const T& NewSectionID, const GR::tPoint& ptSize )
        {
          if ( pChild[0] != NULL )
          {
            // we're not a leaf then

            // try inserting into first child
            Node* pNewNode = pChild[0]->Insert( NewSectionID, ptSize );
            if ( pNewNode )
            {
              return pNewNode;
            }
            // no room, insert into second
            return pChild[1]->Insert( NewSectionID, ptSize );
          }
          // if we're too small, return
          if ( ( ptSize.x > Rectangle.width() )
          ||   ( ptSize.y > Rectangle.height() ) )
          {
            // img doesn't fit in pnode->rect
            return NULL;
          }

          // if there's already a lightmap here, return
          if ( SectionID != NULL )
          {
            return NULL;
          }
          if ( ( ptSize.x == Rectangle.width() )
          &&   ( ptSize.y == Rectangle.height() ) )
          {
            // if we're just right, accept
            // if img fits perfectly in pnode->rect
            SectionID = NewSectionID;
            return this;
          }
          
          // otherwise, gotta split this node and create some kids
          pChild[0] = new Node();
          pChild[1] = new Node();
          
          // decide which way to split
          int dw = Rectangle.width() - ptSize.x;
          int dh = Rectangle.height() - ptSize.y;
          
          if ( dw > dh )
          {
            pChild[0]->Rectangle.set( Rectangle.Left, Rectangle.Top, ptSize.x, Rectangle.height() );
            pChild[1]->Rectangle.set( Rectangle.Left + ptSize.x, Rectangle.Top, 
                                      Rectangle.width() - ptSize.x, Rectangle.height() );
          }
          else
          {
            pChild[0]->Rectangle.set( Rectangle.Left, Rectangle.Top, Rectangle.width(), ptSize.y );
            pChild[1]->Rectangle.set( Rectangle.Left, Rectangle.Top + ptSize.y, 
                                      Rectangle.width(), Rectangle.height() - ptSize.y );
          }
          
          // insert into first child we created
          return pChild[0]->Insert( NewSectionID, ptSize );
          /*
          pChild[0]->SectionID = NewSectionID;
          return pChild[0];
          */
        }
      };
    

      Node            m_mainNode;


    public:

      CSectionLayouter( const GR::tRect& rcFullSize )
      {

        m_mainNode.Rectangle = rcFullSize;

      }

      bool Insert( const T& SectionID, const GR::tPoint& ptSize )
      {
        return !!m_mainNode.Insert( SectionID, ptSize );
      }

      void Result( std::list<std::pair<GR::tPoint,T> >& listResult ) const
      {
        AddFromNode( &m_mainNode, listResult );
      }

      void AddFromNode( const Node* pNode, std::list<std::pair<GR::tPoint,T> >& listResult ) const
      {
        if ( pNode->SectionID )
        {
          listResult.push_back( std::make_pair( GR::tPoint( pNode->Rectangle.Left, pNode->Rectangle.Top ), pNode->SectionID ) );
        }
        if ( pNode->pChild[0] )
        {
          AddFromNode( pNode->pChild[0], listResult );
        }
        if ( pNode->pChild[1] )
        {
          AddFromNode( pNode->pChild[1], listResult );
        }
      }

  };
};


#endif // MATH_SECTION_LAYOUTER_H
