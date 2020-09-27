#ifndef _PJ_MARKOFFCHAIN_H_INCLUDED
#define _PJ_MARKOFFCHAIN_H_INCLUDED

#include <PJLib\Random\PJ_Random.h>
#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <vector>
#include <map>
#include <algorithm>


namespace PJ
{
  namespace Random
  {
    /////////////////////////////////////////////////////////////////////
    //-  
    //-    CMarkoffChain
    //-  
    /////////////////////////////////////////////////////////////////////
    template < class Signal, class Generator = CRand >
    class CMarkoffChain
    {
      typedef std::vector<Signal> signalsequence;

    public:
      CMarkoffChain();
      CMarkoffChain( const CMarkoffChain& rhs );
      CMarkoffChain& operator=( const CMarkoffChain& rhs );
      virtual ~CMarkoffChain();

      template< class I > void source(  I first, I last ) { read( m_seqSource,  first, last ); }
      const signalsequence&    source()  const            { return m_seqSource;  }
      template< class I > void pattern( I first, I last ) { read( m_seqPattern, first, last ); }
      const signalsequence&    pattern() const            { return m_seqPattern; }
      Generator& generator() { return m_Generator; }

      inline friend CMarkoffChain& operator>>( CMarkoffChain& chain, Signal& s );

    protected:
      //-    nächstes Signal erzeugen
      Signal next();

      //-    Datenmember
      signalsequence m_seqSource;  //- Quelldaten
      signalsequence m_seqPattern; //- Ausgangssequenz (Länge legt zugleich Ordnung fest)
      Generator      m_Generator;  //- Zufallszahlengenerator

      //-    Hilfsfunktionen
      template< class I >
      void read( signalsequence& v, I first, I last )
      {
        v.clear();
        v.resize( std::distance( first, last ) );
        signalsequence::iterator it( v.begin() );
        while ( first != last ) *it++ = *first++;
      }
    };

    /////////////////////////////////////////////////////////////////////
    //-  
    //-    Definitionen
    //-  
    /////////////////////////////////////////////////////////////////////
    template < class Signal, class Generator >
      CMarkoffChain<Signal, Generator>::CMarkoffChain() {}

    template < class Signal, class Generator >
      CMarkoffChain<Signal, Generator>::~CMarkoffChain() {}

    template < class Signal, class Generator >
      CMarkoffChain<Signal, Generator>::CMarkoffChain( const CMarkoffChain& rhs )
      : m_seqSource(  rhs.m_seqSource ),
        m_seqPattern( rhs.m_seqPattern ) {}

    template < class Signal, class Generator >
      CMarkoffChain<Signal, Generator>& CMarkoffChain<Signal, Generator>::operator=( const CMarkoffChain<Signal, Generator>& rhs )
      {
        if ( this == &rhs ) return *this;
        m_seqSource  = rhs.m_seqSource;
        m_seqPattern = rhs.m_seqPattern;
        return *this;
      }

    template < class Signal, class Generator >
      inline CMarkoffChain<Signal, Generator>& operator>>( CMarkoffChain<Signal, Generator>& chain, Signal& s )
      {
        s = chain.next();
        return chain;
      }

    /////////////////////////////////////////////////////////////////////
    //-    next()
    //-    nächstes Signal erzeugen
    /////////////////////////////////////////////////////////////////////
    template < class Signal, class Generator >
      Signal CMarkoffChain<Signal, Generator>::next()
      {
        Signal signal = Signal();
        if ( m_seqSource.size() <=  m_seqPattern.size() )  return signal;
        if ( m_seqPattern.empty() )                        return signal;

        //- Statistik erstellen
        typedef std::map< Signal, unsigned long >  signalstatsmap;
        signalstatsmap Stats;
        unsigned long ulFound = 0;
        unsigned long ulLength = m_seqPattern.size();
        signalsequence::iterator it(  m_seqSource.begin()  ), itend(  m_seqSource.end()  );
        signalsequence::iterator pit( m_seqPattern.begin() ), pitend( m_seqPattern.end() );
        while ( it != itend )
        {
          it = search( it, itend, pit, pitend );
          if ( it == itend )            break;
          if ( it + ulLength == itend ) break;
          ++Stats[ *(it + ulLength) ];
          ++ulFound;
          ++it;
        }

        if ( ulFound == 0 ) return signal;

        //- Resultat wählen
        signed long ulRandomNumber = m_Generator() % ulFound;
        signalstatsmap::iterator cmi( Stats.begin() ), cmiend( Stats.end() );
        for ( ; cmi != cmiend; ++cmi )
        {
          ulRandomNumber -= cmi->second;
          if ( ulRandomNumber >= 0 ) continue;
          signal = cmi->first;
          m_seqPattern.erase( m_seqPattern.begin() );
          m_seqPattern.push_back( signal );
          break;
        }

        return signal;
      }
  };
};


#endif// _PJ_MARKOFFCHAIN_H_INCLUDED
