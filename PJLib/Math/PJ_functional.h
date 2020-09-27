#ifndef _PJ_FUNCTIONAL_H_INCLUDED
#define _PJ_FUNCTIONAL_H_INCLUDED

#pragma warning ( disable : 4786 )
#pragma once


#include <algorithm> //- für swap


namespace math
{
  /////////////////////////////////////////////////////////////////////
  //-
  //-    Test, ob Werte innerhalb Intervall liegen
  //-
  /////////////////////////////////////////////////////////////////////





  //- Innerhalb geschlossenem Intervall? (Intervall enthält Grenzen border1 u. border2)
  template <class T>
  inline bool Inside_CC( T border1, const T& testvalue, T border2 )
  {
    if ( border2 < border1 ) //- warum prüfen? könnte der user machen - oder?
      std::swap( border1, border2 );

    if ( testvalue < border1 ) return false;
    if ( testvalue > border2 ) return false;
    return true;
  }

  //- Innerhalb offenem Intervall? (Intervall enthält nicht Grenzen border1 u. border2)
  template <class T>
  inline bool Inside_OO( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue <= border1 ) return false;
    if ( testvalue >= border2 ) return false;
    return true;
  }

  //- Innerhalb links geschlossenem Intervall? (Intervall enthält border1, aber nicht border2)
  template <class T>
  inline bool Inside_CO( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue < border1 ) return false;
    if ( testvalue >= border2 ) return false;
    return true;
  }

  //- Innerhalb rechts geschlossenem Intervall? (Intervall enthält border2, aber nicht border1)
  template <class T>
  inline bool Inside_OC( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue <= border1 ) return false;
    if ( testvalue > border2 ) return false;
    return true;
  }





  /////////////////////////////////////////////////////////////////////
  //-
  //-    Werte auf Intervall begrenzen
  //-
  /////////////////////////////////////////////////////////////////////

  //- Innerhalb geschlossenem Intervall? (Intervall enthält Grenzen border1 u. border2)
  template <class T>
  inline T Limit_CC( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue < border1 ) return border1;
    if ( testvalue > border2 ) return border2;
    return testvalue;
  }

  //- Innerhalb offenem Intervall? (Intervall enthält nicht Grenzen border1 u. border2)
  template <class T>
  inline T Limit_OO( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue <= border1 ) return border1;
    if ( testvalue >= border2 ) return border2;
    return testvalue;
  }

  //- Innerhalb links geschlossenem Intervall? (Intervall enthält border1, aber nicht border2)
  template <class T>
  inline T Limit_CO( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue < border1 ) return border1;
    if ( testvalue >= border2 ) return border2;
    return testvalue;
  }

  //- Innerhalb rechts geschlossenem Intervall? (Intervall enthält border2, aber nicht border1)
  template <class T>
  inline T Limit_OC( const T& border1, const T& testvalue, const T& border2 )
  {
    if ( testvalue <= border1 ) return border1;
    if ( testvalue > border2 ) return border2;
    return testvalue;
  }



};

#endif//_PJ_FUNCTIONAL_H_INCLUDED