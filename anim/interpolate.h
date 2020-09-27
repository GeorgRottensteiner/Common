#ifndef _interpolate_h
#define _interpolate_h

// ================================================
//@{                             
// Nützliche funktionstemplates zur linearen interpolation und zum morphing.
//
// der delta-parameter ("mischungsverhältnis") sollte im Intervall [0,1] liegen, aber
// unter umständen sind auch andere werte sinnvoll (experimentieren!)
// verwendet wird folgende Formel:
//
// a * ( 1.0f - delta ) + b * delta
//
// interessant ist es auch, das delta erst mit einem bender (bender.h)
// zu modifzieren und statt delta z.B. slowdown_bender( delta ) zu übergeben
// für nichtlineare interpolationen
// @author Peter Jerz     
// @version 1.0                  
//@}                             
// ================================================
namespace anim
{
  //@{                             
  // mischen zweier einzelwerte a und b.
  // <pre>
  //@@ // Beipiel:
  //@@ #include <anim/interpolate.h>
  //@@  
  //@@ float test()
  //@@ {
  //@@    float a = 10.0f;
  //@@    float b = 50.0f;
  //@@    float r = anim::interpolate( a, b, 0.25f ); 
  //@@    return r; 
  //@@ }
  // </pre>
  // @author Peter Jerz     
  // @version 1.0                  
  //@}                             
  // @param a Kanal a
  // @param b Kanal b
  // @param delta Verhältnis
  // @returns interpolierter Wert 
  template< typename T >
  T interpolate( const T& a, const T& b, float delta )
  {
    return a * ( 1.0f - delta ) + b * delta;
  }

  //@{                             
  // mischen zweier einzelwerte a und b in das resultat r.
  // <pre>
  //@@ // Beipiel:
  //@@ #include <anim/interpolate.h>
  //@@  
  //@@ void test()
  //@@ {
  //@@    float a = 10.0f;
  //@@    float b = 50.0f;
  //@@    float r = 0.0f;
  //@@    anim::interpolate( r, a, b, 0.25f ); 
  //@@    // r enthält nun das resultat 
  //@@ }
  // </pre>
  // @param r Kanal r, Resultat
  // @param a Kanal a
  // @param b Kanal b
  // @param delta Verhältnis
  // @author Peter Jerz     
  // @version 1.0                  
  //@}                             
  template< typename T >
  void interpolate( T& r, const T& a, const T& b, float delta )
  {
    r = a * ( 1.0f - delta ) + b * delta;
  }

  //@{                             
  // mischen zweier ranges a und b in den range r.
  // ranges interpolieren (z.b. zwei std::vectoren und resultat in dritten schreiben).
  // die länge der a-sequenz entscheidet die anzahl der operationen.
  // <pre>
  //@@ // Beipiel:
  //@@ #include <anim/interpolate.h>
  //@@  
  //@@ void test()
  //@@ {
  //@@    float a[5]  = { 1.2f, 4.5f, -0.34f, 0.0f, 2.0f }
  //@@    float b[10] = { 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f }
  //@@    float r[7]  = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }
  //@@    anim::interpolate( r, a, a + 5, b, 0.25f ); 
  //@@    // r[0] bis r[4] enthalten nun die 5 resultate 
  //@@ }
  // </pre>
  // @param it_r ausgabeiterator für Kanal r, Resultat
  // @param it_a iterator für Kanal a
  // @param it_a_end enditerator für Kanal a
  // @param it_b iterator für Kanal b
  // @param delta Verhältnis
  // @author Peter Jerz     
  // @version 1.0                  
  //@}                             
  template< typename IT1, typename IT2, typename IT3 >
  void interpolate( IT1 it_r,                 //- ausgabeiterator
                    IT2 it_a, IT2 it_a_end,   //- eingabeiterator a und ende
                    IT3 it_b,                 //- eingabeiterator b
                    float delta )
  {
    float one_minus_delta = 1.0f - delta;
    while( it_a != it_a_end ) 
      *it_r++ = *it_a++ * one_minus_delta + *it_b++ * delta; 
  }

  //@{                             
  // mischen zweier ranges a und b in den range r.
  // ranges interpolieren (z.b. zwei std::vectoren und resultat in dritten schreiben).
  // operationen enden, sobald ein iterator == dem zugehörigen end ist
  // <pre>
  //@@ // Beipiel:
  //@@ #include <anim/interpolate.h>
  //@@  
  //@@ void test()
  //@@ {
  //@@    float a[5]  = { 1.2f, 4.5f, -0.34f, 0.0f, 2.0f }
  //@@    float b[10] = { 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f, 1.2f }
  //@@    float r[7]  = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f }
  //@@    anim::interpolate( r, r + 7, a, a + 5, b, b + 10, 0.25f ); 
  //@@    // r[0] bis r[4] enthalten nun die 5 resultate 
  //@@ }
  // </pre>
  // @param it_r ausgabeiterator für Kanal r, Resultat
  // @param it_r_end enditerator für Kanal r, Resultat
  // @param it_a iterator für Kanal a
  // @param it_a_end enditerator für Kanal a
  // @param it_b iterator für Kanal b
  // @param it_b_end enditerator für Kanal b
  // @param delta Verhältnis
  // @author Peter Jerz     
  // @version 1.0                  
  //@}                             
  template< typename IT1, typename IT2, typename IT3 >
  void interpolate( IT1 it_r, IT1 it_r_end, //- ausgabeiteratoren kanal r
                    IT2 it_a, IT2 it_a_end, //- eingabeiteratoren kanal a
                    IT3 it_b, IT3 it_b_end, //- eingabeiteratoren kanal b
                    float delta )
  {
    float one_minus_delta = 1.0f - delta;
    while( it_a != it_a_end && it_b != it_b_end && it_r != it_r_end ) 
      *it_r++ = *it_a++ * one_minus_delta + *it_b++ * delta; 
  }

}; // namespace anim

  // void interpolate( const CWF& a, const CWF& b, float morph )
  // {
    // //- dies verwendet nicht die effizienteste version von interpolate, ist aber sicherer
    // anim::interpolate( v_.begin(),  v_.end(),  a.v_.begin(),  a.v_.end(),  b.v_.begin(),  b.v_.end(),  morph );
    // anim::interpolate( vn_.begin(), vn_.end(), a.vn_.begin(), a.vn_.end(), b.vn_.begin(), b.vn_.end(), morph );
  // }

#endif// _interpolate_h
