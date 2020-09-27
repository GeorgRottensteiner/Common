#ifndef _BENDER_H
#define _BENDER_H
#include <math.h>

namespace anim
{
  //- Diverse bender-Funktoren z.B. zur Verwendung mit exclusive_bending_animator
  
  //- bending animatoren "biegen" die Zeit mithilfe von Bender-funktionen
  //- oder Funktoren (siehe auch bender.h), um beschleunigte Bewegungen u. ä.
  //- zu ermöglichen

  //- ein Bender erwartet einen Wert im Intervall [0.0f, 1.0f]
  //- und liefert einen Wert aus dem Intervall [0.0f, 1.0f] zurück
  //- (*keine* Bereichsprüfung!)

  //- sind auch nützlich, um zum beispiel aus linearen
  //- interpolationen und morphs nichtlineare zu machen.

  //- verlangsamt, parabel
  struct slowdown_bender
  {
    float operator()( const float& f ) const
    {
      return 1.0f - ( f - 1.0f ) * ( f - 1.0f );
    }
  };

  //- verlangsamt, sinus 
  struct sin_slowdown_bender
  {
    float operator()( const float& f ) const
    {
      return (float)sin( f  * 3.1415926f / 2.0f );
    }
  };

  //- beschleunigt, cosinus
  struct cos_speedup_bender
  {
    float operator()( const float& f ) const
    {
      return 1.0f - (float)cos( f  * 3.1415926f / 2.0f );
    }
  };

  //- beschleunigt erst und bremst dann, cosinus
  struct cos_speedup_slowdown_bender
  {
    float operator()( const float& f ) const
    {
      return 0.5f - (float)cos( f  * 3.1415926f ) / 2.0f;
    }
  };

}; // namespace anim

#endif// _BENDER_H