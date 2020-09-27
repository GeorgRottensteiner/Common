#ifndef _PJ_RANDOM_H_INCLUDED
#define _PJ_RANDOM_H_INCLUDED

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <iostream>

namespace PJ
{
  namespace Random
  {

    /////////////////////////////////////////////////////////////////////
    //-  
    //-    CRand, ein Pseudozufallszahlengenerator 
    //-  
    /////////////////////////////////////////////////////////////////////
    class CRand
    {
    public:
      //- Seed zuweisen
      inline void          seed( const unsigned long s )  { m_iSeed = s & 0x0000FFFF; }
      inline unsigned long seed() const                   { return m_iSeed; }
      

      //- Zufallszahl erzeugen
      //- Beispiel:
      //-   CRand karlheinz;
      //-   long i = karlheinz();
      //-   //- i enthält jetzt einen wert zwischen 0 und 1234567 (oder 1234566, weiss nich)
      unsigned long operator()()
      { 
        generate();
	      return m_iSeed;
      }


      //- Zufallszahl erzeugen innerhalb Bereich
      //- Beispiel:
      //-   CRand gerd;
      //-   long i = gerd( 100 );
      //-   //- i enthält jetzt einen wert zwischen 0 und 99
      unsigned long operator()( const unsigned long bereich )
      { 
	      return ( bereich * operator()() ) / c_iP();
      }


      //- Serialisierung mit streams (Seed speichern für späteren Gebrauch)
      //- abgespecktes Beipiel:
      //-   #include <fstream>
      //-   CRand myRand; 
      //-   std::ofstream o( "Samenprobe.txt" );
      //-   o << myRand; //- Zustand von myRand speichern
      //-   CRand nochnRand; 
      //-   std::ifstream i( "Samenprobe.txt" );
      //-   i >> nochnRand; //- Zustand von nochnRand laden, myRand und nochnRand sind jerz gleich
      inline friend std::ostream& operator<<( std::ostream& s, CRand& r ) { s << r.m_iSeed; return s; }
      inline friend std::istream& operator>>( std::istream& s, CRand& r ) { s >> r.m_iSeed; return s; }


      /////////////////////////////////////////////////////////////////////
      //- The Fab Four (SCD=): 
      /////////////////////////////////////////////////////////////////////
      //- Standardkonstruktor
      CRand( const unsigned long seed = 0 ) : m_iSeed( seed & 0x0000FFFF ) {} //- das undieren könnte weg
      //- Kopierkonstruktor
      CRand( const CRand& rhs ) : m_iSeed( rhs.m_iSeed ) {}
      //- (virtueller) Destruktor (hier arbeitslos)
      virtual ~CRand() {}
      //- Zuweisungsoperator
      CRand& operator=( const CRand& rhs )
      {
        if ( this != &rhs ) m_iSeed = rhs.m_iSeed;
        return *this;
      }


    protected:
      unsigned long m_iSeed; 
      inline void generate() { m_iSeed = ( c_iA() * m_iSeed + c_iA() ) % c_iP(); } 
      inline const long c_iP() const { return 1234567; }
      inline const long c_iA() const { return 1234;    }
    };
  };
};


#endif// _PJ_RANDOM_H_INCLUDED
