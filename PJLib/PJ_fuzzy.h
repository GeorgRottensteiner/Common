#ifndef _PJ_FUZZY_H_INCLUDED
#define _PJ_FUZZY_H_INCLUDED
 
namespace PJ
{
  typedef float fvalue;

  ////////////////////////////////////////////////
  //- eine unscharfe bool-Version
  ////////////////////////////////////////////////
  class fuzzy
  {
    //- Member-Variablen
  private:
    fvalue value;

  public:
    //- Konstruktoren
    fuzzy( const fuzzy& rhs )               : value( rhs.value  ) {}
    fuzzy( const fvalue rhs = fvalue(0.0) ) : value( rhs        ) {}
  
    //- nicht const

    fuzzy& operator=( const fuzzy& rhs ) { if ( this != &rhs )  value = rhs.value;  return *this; } 
    fuzzy& operator=( const fvalue rhs ) {                      value = rhs;        return *this; }

    //- const

    //- binäre Operatoren
    fuzzy operator&&( const fuzzy& rhs ) const     { return value > rhs.value ? rhs.value : value; }
    fuzzy operator||( const fuzzy& rhs ) const     { return value < rhs.value ? rhs.value : value; }
    //- knüfflig - Gleichheit/Ungleichheit ist natürlich auch fuzzy - too heavy?     
    fuzzy operator==( const fuzzy& rhs ) const     { fvalue v( value - rhs.value ); return v >= 0 ? v : -v; } 
    fuzzy operator!=( const fuzzy& rhs ) const     { return !( *this == rhs ); } 
  
    //- unäre Operatoren
    fuzzy operator!() const   { return fvalue(1.0) - value; }

    //- Casting
    operator fvalue() const    { return value; }
    // operator float() const    { return float( value );      }
    // operator double() const   { return double( value );     }
    // operator bool() const      { return value >= fvalue( 0.5 ); } 
  };


  //- einige Modifier
  fuzzy very(       const fuzzy& rhs )  { return fvalue( rhs ) * fvalue( rhs ); } 
  fuzzy extremely(  const fuzzy& rhs )  { return fvalue( rhs ) * fvalue( rhs ) * fvalue( rhs ); } 
  fuzzy average(    const fuzzy& rhs )  { return rhs == fuzzy( fvalue( 0.5 ) ); } 
  fuzzy not(        const fuzzy& rhs )  { return !rhs; } 
  // fuzzy not(        const fuzzy& rhs )  { return fvalue(1.0) - fvalue( rhs ); } 
};
#endif//_PJ_FUZZY_H_INCLUDED

/*
  Notes:
  Unschärfe existiert nur in Bezug auf einen Kontext,
  Werte selbst sind absolut.
*/