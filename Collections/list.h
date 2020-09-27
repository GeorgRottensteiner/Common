#ifndef GR_LIST_H
#define GR_LIST_H

#include <list>


namespace GR
{

  //- match (nicht-full-match) in assoc umbenennen, interface an find anpassen (iterator zurückgeben!)
  //- überlegen, ob siblings grundsätzlich alphabetisch geordnet sein sollen oder frei gruppierbar
  //- (wie liste - macht das Sinn?)
  //- const_iteration usw. einbauen!!!!
  //- --iterator einbauen

  //- auch einzel-keys suchen: iterator find( iterator first, iterator last, key ) wie std::find

  template <typename V> class List : public std::list<V>
  {

    public:


      typedef V value_type;



      // Append other container to back without sorting
      void Add( const GR::List<V>& OtherContainer )
      {
        GR::List<V>::const_iterator   it( OtherContainer.begin() );
        while ( it != OtherContainer.end() )
        {
          push_back( *it );

          ++it;
        }
      }



      const V& operator[]( size_t Index ) const
      {
        if ( Index >= size() )
        {
          dh::Log( "GR::List - Index %d out of bounds (size is %d)", Index, size() );
          return V();
        }
        GR::List<V>::const_iterator   it( begin() );
        std::advance( it, Index );
        return *it;
      }



      V& operator[]( size_t Index )
      {
        if ( Index >= size() )
        {
          static V   v;
          dh::Log( "GR::List - Index %d out of bounds (size is %d)", Index, size() );
          return v;
        }
        GR::List<V>::iterator   it( begin() );
        std::advance( it, Index );
        return *it;
      }


  };


}

#endif // GR_LIST_H