#ifndef _PJ_ALGORITHM_H_INCLUDED
#define _PJ_ALGORITHM_H_INCLUDED


#include <algorithm>
// #include <PJLib/PJ_algorithm.h>


namespace PJ
{
  //- Füllt einen eine Sequenz 
  //- mit aufeinander folgenden Zahlen, 
  //- beginnend ab value.
  //-
  //- Beispiel: 
  //- int array[5];
  //- iota( array, array + 5, 7 );
  //- array enthält nun folgende Werte:
  //- { 7, 8, 9, 10, 11 }
  template<class ForwardIterator, class T> inline
    void iota(ForwardIterator first, ForwardIterator last, T value)
    {
      while ( first != last )
	      *first++ = value++;
    }

  //- Füllt einen eine Sequenz 
  //- mit aufeinander folgenden Zahlen, 
  //- beginnend ab value mit einstellbarer
  //- Schrittweite.
  //-
  //- Beispiel: 
  //- int array[5];
  //- iota( array, array + 5, 7, 3 );
  //- array enthält nun folgende Werte:
  //- { 7, 10, 13, 16, 19 }
  template<class ForwardIterator, class T> inline
    void iota(ForwardIterator first, ForwardIterator last, T value, T step)
    {
      while ( first != last )
      {
        *first++ = value;
        value += step;
      }
    }


  //- findet in einer Sequenz z.B. die entsprechende schliessende Klammer
  template < class Tokentype, class Iterator >
    Iterator FindClosingToken( Iterator first, 
                               Iterator last,
                               const Tokentype& tokenOpen,
                               const Tokentype& tokenClose )
    {
      if ( first == last ) return first;

      first = std::find( first, last, tokenOpen );
      if ( first == last ) return first;
    
      ++first;
      long level = 1;
      while ( first != last )
      {
        if      ( *first == tokenOpen  ) ++level;
        else if ( *first == tokenClose ) --level;

        if ( level < 1 ) break;
      
        ++first;
      }

      return first;
    }


  //- findet in einer Sequenz z.B. die entsprechende schliessende Klammer
  template < class Tokentype, class Iterator >
    std::pair< Iterator, Iterator > FindInnerBlock(  Iterator first, 
                                                 Iterator last,
                                                 const Tokentype& tokenOpen,
                                                 const Tokentype& tokenClose )
    {
      std::pair< Iterator, Iterator > result = std::make_pair( last, last );
      
      if ( first == last ) return result;

      first = std::find( first, last, tokenOpen );
      if ( first == last ) return result;
    
      ++first;
      result.first = first;

      long level = 1;
      while ( first != last )
      {
        if      ( *first == tokenOpen  ) ++level;
        else if ( *first == tokenClose ) --level;

        if ( level < 1 ) break;
      
        ++first;
      }

      result.second = first;

      return result;
    }


};
#endif //_PJ_ALGORITHM_H_INCLUDED