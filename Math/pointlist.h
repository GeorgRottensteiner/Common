#ifndef _PJ_math_pointlist_h
#define _PJ_math_pointlist_h

//#include <utility.h>


//- template-routinen zur manipulation von punktlisten, auch polygonen
//- verschiedene container werden durch verwendung von iteratoren unterstützt
//- viele funktionen setzen voraus, dass math::vector3t<> (in vector3t.h) 
//- verwendet wird! Peter fragen!



namespace math
{
  //- streckenlänge berechnen (auch kantenlänge eines (offenen) polygons)
  template < typename IN_IT >
  double calculate_strokelength( IN_IT first, IN_IT last )
  {
    double d   = 0.0;
    if ( first == last ) return d;
    IN_IT  it1 = first;
    for( ; first != last; ++first )
    {
      d += ( *first - *it1 ).length();
      it1 = first;
    }
    return d;
  }

  template < typename IN_IT, typename OUT_IT >
  OUT_IT normalize_points_sub( OUT_IT target, IN_IT first, IN_IT last, double distance_between_points )
  {
    if ( first == last ) return target;
    
    *target++ = *first;
    IN_IT it1 = first;
    double d1 = 0.0;
    
    for ( ; first != last; ++first )
    {
      double d2 = ( *first - *it1 ).length();
      for(d1 += d2; d1 >= distance_between_points;)
      {
        d1 -= distance_between_points;
        *target++ = first->interpolated( *it1, d1 / d2);

        IN_IT   itNext( first );
        itNext++;
        if( itNext==last ) break;
      }
      IN_IT   itNext( first );
      itNext++;
      if( itNext==last ) break;
      it1 = first;
    }

    ++first;

    for ( ; first != last; ++first )
      *target++ = *first;

    return target;
  }

  //- punkte neu verteilen mit gewünschtem abstand
  //- beispiel:   
  //- std::list<CVector> ziel;
  //- math::normalize_points( back_insert_iterator<std::list<CVector> >( ziel ), quelle.begin(), quelle.end(), abstand ); 

  template < typename IN_IT, typename OUT_IT >
  OUT_IT normalize_points( OUT_IT target, IN_IT first, IN_IT last, int distance )
  {
    double d = calculate_strokelength<IN_IT>( first, last );
    int j = (int)(d / (double)distance);
    if( j < 8 ) j = 8;
    double d1 = d / (double)j;
    return normalize_points_sub( target, first, last, d1 );
  }

  template < typename IN_IT, typename OUT_IT >
  OUT_IT normalize_points_number( OUT_IT target, IN_IT first, IN_IT last, int number )
  {
    double d = calculate_strokelength<IN_IT>( first, last );
    double d1 = (int)(d / (double)number);
    // if( j < 2 ) j = 2;
    // double d1 = d / (double)j;
    return normalize_points_sub( target, first, last, d1 );
  }


  /////////////////////////////////////////////////////////////////////
  //-    Baustelle - Verwendung z.Zt. auf eigene Gefahr
  /////////////////////////////////////////////////////////////////////
  
  //- Entfernung eines Punktes zu einem Liniensegment finden
  template < typename VEC3 >
  float point_to_segment_distance( const VEC3& point, const VEC3& s1, const VEC3& s2 )
  {
    VEC3 local              = point - s1;
    VEC3 segment            = s2 - s1;
    float segmentProjection = segment.dot( local ) / local.length();
    if      ( segmentProjection < 0.0f             ) return point.distance( s1 );
    else if ( segmentProjection > segment.length() ) return point.distance( s2 );
    else                                             return point.distance( s1 + segmentProjection * segment.unit() );
  }


  //- Die geringste Entfernung eines Punktes von einem Pfad finden
  template < typename IN_IT, typename VEC3 >
  float point_to_path_distance( const VEC3& point, IN_IT first, IN_IT last )
  {
    float minDistance         = 3.402823E+038f;
    // float pathDistance        = 0.0f;
    // float segmentLength       = 0.0f;
    // float segmentLengthTotal  = 0.0f;
    // VEC3 segmentNormal, local;    
    // float segmentProjection;
    float d;

    if ( first == last ) return -1; //BAUSTELLE

    IN_IT it1 = first++;
    
    for ( ; first != last; ++first ) 
    {
      // VEC3 segment = ( *first - *it1 );
      
      // segmentNormal = segment.cross( VEC3(0,0,-1) );
      // segmentLength = first->distance( *it1 );
      // local         = point - *it1;
      
      d = point_to_segment_distance( point, *it1, *first );
      if( d < minDistance )
      {
        minDistance = d;
        // segmentProjection = ( *first - *it1 ).dot(local) / ( *first - *it1 ).length();
        // pathDistance = segmentLengthTotal + segmentProjection;
      }
      // segmentLengthTotal += segmentLength;
    }

    // return pathDistance;
    return minDistance;
  }



}

#endif//_PJ_math_pointlist_h