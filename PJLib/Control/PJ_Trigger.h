#ifndef _PJ_TRIGGER_H_INCLUDED
#define _PJ_TRIGGER_H_INCLUDED

namespace PJ
{
  namespace Control
  {
    template < class T >
    class TCTrigger
    {
    private:
      T m_tCountedSoFar; //- bisher gezählt
      T m_tThreshold;    //- Schwellwert

      //- Standkonstruktor, verboten, Trigger muss mit Schwellwert initalisert werden
  
    public:

      //- Konstruktor, initialisiert Trigger mit Schwellwert und auch Counted
      TCTrigger( const T& threshold = 1, const T& counted = 0 )
        : m_tCountedSoFar( counted ),
          m_tThreshold( threshold )
      { 
        if ( m_tThreshold < 0 ) m_tThreshold = 0;
      }

      //- vorbildlicher Copy-Konstruktor
      TCTrigger( const TCTrigger& rhs )
        : m_tCountedSoFar(  rhs.m_tCountedSoFar ),
          m_tThreshold(     rhs.m_tThreshold )
      {}

      //- vorbildlicher Zuweisungsoperator
      TCTrigger& operator=( const TCTrigger& rhs )
      {
        if ( this == &rhs ) return *this;
        m_tCountedSoFar = rhs.m_tCountedSoFar;
        m_tThreshold    = rhs.m_tThreshold;
        return *this;
      }

      inline void threshold( const T& t ) { m_tThreshold = t; if ( m_tThreshold < 0 ) m_tThreshold = 0; }
      inline T    threshold() const       { return m_tThreshold; }
      inline void counted( const T& t ) {        m_tCountedSoFar = t; }
      inline T    counted() const       { return m_tCountedSoFar;     }



      //- Klammer-Operator, vergleicht mit Schwellwert,
      //- falls Schwelle überschritten, wird true zurückgegeben
      bool operator()()
      {
        if ( m_tCountedSoFar < m_tThreshold )
        {
          return false; 
        }
        else
        {
          m_tCountedSoFar -= m_tThreshold;
          return true;
        }
      }

      //- Klammer-Operator, zählt hoch und vergleicht mit Schwellwert,
      //- falls Schwelle überschritten, wird true zurückgegeben und Schwellwert abgezogen
      bool operator()( const T& tickamount )
      {
        m_tCountedSoFar += tickamount;
        if ( m_tCountedSoFar < m_tThreshold ) return false; 
        else                                  return true;
      }

      //- Dasselbe wie voriger operator
      bool count( const T& tickamount )
      {
        m_tCountedSoFar += tickamount;
        if ( m_tCountedSoFar < m_tThreshold ) return false; 
        else                                  return true;
      }

    };
  }
}

#endif //_PJ_TRIGGER_H_INCLUDED