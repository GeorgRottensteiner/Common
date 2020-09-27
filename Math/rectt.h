#ifndef _PJ_rectt_h
#define _PJ_rectt_h

#pragma warning ( disable : 4786 )
#include <math.h>
#include <utility>
#include <iostream>
#include <list>
#include <GR/GRTypes.h>

namespace math
{
  ////////////////////////////////////////////////
  //-    tCRect
  ////////////////////////////////////////////////
  template < class V >
    //- V = triple<T> oder tupel<T>
  class rectt
  {
  public:
    typedef V vector_type;
    typedef typename vector_type::scalar_type scalar_type;
  protected:
    vector_type m_vPos, m_vSize;
  public:
    rectt() {}
    rectt( const rectt& rhs ) : m_vPos( rhs.m_vPos ), m_vSize( rhs.m_vSize ) {}
    rectt( const vector_type& pos, const vector_type& size ) : m_vPos( pos ), m_vSize( size ) {}
    rectt& operator=( const rectt& rhs ) 
    {
      if ( this == &rhs ) return *this;
      m_vPos  = rhs.m_vPos;
      m_vSize = rhs.m_vSize; 
      return *this;
    }

    //- getter, setter
    vector_type&   position_ref()          { return m_vPos; }
    vector_type    position()    const     { return m_vPos; }
    void position( const vector_type& p )  { m_vPos = p; }
    vector_type    size()        const     { return m_vSize; }
    void size( const V& s )      { m_vSize = s; }
    
    //- mittelpunkt
    vector_type    center()    const       { return m_vPos + m_vSize / 2; }
    
    scalar_type signed_volume() const
    {
      scalar_type v = 1;
      for ( int d = 0; d < vector_type::dimensions(); ++d )
        v *= m_vSize[ d ];
      return v;
    }

    scalar_type volume() const
    {
      scalar_type v = signed_volume();
      return ( v < 0 )? -v : v;
    }

    ////////////////////////////////////////////////
    //-    Operationen
    ////////////////////////////////////////////////
    //- Verschieben
    rectt& offset( const vector_type& delta ) 
    { m_vPos += delta; return *this; }

    rectt& offset( const scalar_type& iX, const scalar_type& iY ) 
    { 
      m_vPos.x += iX;
      m_vPos.y += iY;
      return *this; 
    }
    //- normalisieren (alles anpassen, falls size negativ) 
    rectt& normalize()
    { 
      for ( int d = 0; d < vector_type::dimensions(); ++d )
        if ( m_vSize[ d ] < 0 ) { m_vPos[ d ] += m_vSize[ d ]; m_vSize[ d ] = -m_vSize[ d ]; }
      return *this;
    }

    //- 'aufblasen'
    rectt& inflate( const vector_type& delta ) 
    { 
      normalize();
      m_vPos  -= delta; 
      m_vSize += delta * 2;
      return *this;
    }

    rectt& combine( const vector_type& rhs )
    { 
      normalize();
      for ( int d = 0; d < vector_type::dimensions(); ++d )
      {
        if ( m_vPos[ d ] > rhs[d] )
        {
          m_vSize[d] = m_vSize[d] + m_vPos[d] - rhs[d];
          m_vPos[ d] = rhs[d];
        }
        else if  ( m_vPos[ d ] + m_vSize[d] < rhs[d] )
        {
          m_vSize[d] = rhs[d] - m_vPos[d];

        }
      }
      return *this;
    }

    rectt& combine( const rectt& rhs )
    { 
      combine( rhs.position() );
      combine( rhs.position() + rhs.size() );
      return *this;
    }

    ////////////////////////////////////////////////
    //-    const-Funktionen
    ////////////////////////////////////////////////
    // inline tCRect normalized() const { return normalized( tCRect( *this ) ); }
    rectt normalized() const 
    { rectt rect( *this ); return rect.normalize(); }


    ////////////////////////////////////////////////
    //-    Tests
    //-  (Rects sollten dazu vorher normalisiert 
    //- sein, also keine negative size haben
    ////////////////////////////////////////////////
    //- ist Punkt enthalten?
    bool contains( const vector_type& t ) const
    { 
      return t >= m_vPos && t <= m_vPos + m_vSize; 
    }
    //- ist anderes Rect enthalten?
    bool contains( const rectt& r ) const
    { return contains( r.position() ) && contains( r.position() + r.size() ); }

    //- eckpunkte aufzählen
    void enumerate_corners( std::list<vector_type>& l ) const
    {
      //- 2 hoch dim ecken werden zurückgegeben
      int corners   = 1 << vector_type::dimensions();
      int dimension = 0;

      vector_type v;

      //- nun permutieren:  
      //- alles mit allem usw
      for ( int corner = 0; corner < corners; ++corner )
      {
        v.clear();
        for ( dimension = 0; dimension < vector_type::dimensions(); ++dimension )
        {
          if ( corner & ( 1 << dimension ) )
            v[ dimension ] = m_vPos[ dimension ] + m_vSize[ dimension ];
          else
            v[ dimension ] = m_vPos[ dimension ];  
        }
        l.push_back( v );
      }  
    }
    
    //- minima und maxima jeder dimension in bereich ablegen, der
    //- scalarpaare enthält
    //- die iteratoren müssen auf std::pair< scalar, scalar > zeigen
    template< typename IT >
    void get_extents( IT first, IT last ) const
    { 
      for ( int d = 0; first != last && d < vector_type::dimensions(); ++d, ++first )
      {
        if ( m_vSize[ d ] < 0 )
        {
          //- size ist negative => pos + size ist kleiner
          first->first  = m_vPos[ d ] + m_vSize[ d ];
          first->second = m_vPos[ d ];
        }
        else
        {
          //- size ist positiv => pos + size ist größer
          first->first  = m_vPos[ d ];
          first->second = m_vPos[ d ] + m_vSize[ d ];
        }
      }
    }

    //- prüfen, ob sich zwei rects schneiden
    bool intersects( const rectt& rhs ) const
    { 
      int intersections = 0; 
      scalar_type mymin, mymax, rhsmin, rhsmax;
      //- in jeder dimension muss eine überschneidung der extents gegeben sein!
      for ( int d = 0; d < vector_type::dimensions(); ++d  )
      {
        //- meine minimaxima usw für diese dimension holen
        if ( m_vSize[ d ] < 0 )
        {
          mymin = m_vPos[d] + m_vSize[d];
          mymax = m_vPos[d];
        }
        else
        {
          mymin = m_vPos[d];
          mymax = m_vPos[d] + m_vSize[d];
        }

        //- rhs' minimaxima usw für diese dimension holen
        if ( rhs.m_vSize[ d ] < 0 )
        {
          rhsmin = rhs.m_vPos[d] + rhs.m_vSize[d];
          rhsmax = rhs.m_vPos[d];
        }
        else
        {
          rhsmin = rhs.m_vPos[d];
          rhsmax = rhs.m_vPos[d] + rhs.m_vSize[d];
        }
        
        //- überschneidung prüfen
        if (     ( ( mymin  <= rhsmin ) && ( rhsmin < mymax  ) )
              || ( ( mymin  < rhsmax ) && ( rhsmax <= mymax  ) )
              || ( ( rhsmin <= mymin  ) && ( mymin  < rhsmax ) )
              || ( ( rhsmin < mymax  ) && ( mymax  <= rhsmax ) ) )
              // sind das nicht zuviele vergleiche? 
        {
          ++intersections;
        }
      }      

      //- in jeder dimension eine überschneidung? - treffer!
      return intersections == vector_type::dimensions();
    }

    ////////////////////////////////////////////////
    //-    I/O
    ////////////////////////////////////////////////
    //- Ausgabe
    friend inline std::ostream& operator <<( std::ostream& o, const rectt& v ) 
    { o << v.m_vPos << " "; o << v.m_vSize; return o; }
    //- Eingabe
    friend inline std::istream& operator >>( std::istream& i, rectt& v ) 
    { i >> v.m_vPos >> v.m_vSize; return i; }
  };

  //- oft benötigt: 3D-Boxen mit floats
  typedef rectt< vector3t< float > > rect3;

};

#endif //_PJ_rectt_h