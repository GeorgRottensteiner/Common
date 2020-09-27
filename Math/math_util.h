#ifndef _math_util_h
#define _math_util_h
  
#include <limits>

namespace math
{


  /////////////////////////////////////////////////////////////////////
  //-    culling/links oder rechts
  /////////////////////////////////////////////////////////////////////
  template <class VECTOR>
  float distanceXZ2( const VECTOR& a, const VECTOR& b, const VECTOR& c )
  { 
    return (a.x - b.x) * (c.z - b.z) - (a.z - b.z) * (c.x - b.x);
  }
  
  /////////////////////////////////////////////////////////////////////
  //-    Kreise berechnen
  /////////////////////////////////////////////////////////////////////
  template <class VECTOR>
  bool circleXZ( const VECTOR& a, const VECTOR& b, const VECTOR& c, VECTOR& result ) 
  {
    float A = b.x - a.x;
    float B = b.z - a.z;
    float C = c.x - a.x;
    float D = c.z - a.z;

    float E = A * (a.x + b.x) + B * (a.z + b.z);
    float F = C * (a.x + c.x) + D * (a.z + c.z);

    float G = 2.0f * ( A * (c.z - b.z) - B * (c.x - b.x) );

    if ( G == 0.0f ) return false;

    float p_0 = ( D * E - B * F ) / G;
    float p_1 = ( A * F - C * E ) / G;

    result.x = p_0;
    result.z = p_1;
    
    return true;
  }




  template <class VECTOR>
  bool IntersectBoxWithRay( const VECTOR& ray_orig,  const VECTOR& ray_dir, 
                            const VECTOR& box_min,   const VECTOR& box_max,
                            VECTOR&       res_near,  VECTOR&       res_far ) 
  { 
    float tnear = -FLT_MAX; 
    float tfar  =  FLT_MAX; 
 
    for ( int i = 0; i < VECTOR::dimensions(); ++i ) 
    { 
      float origin    = ray_orig[i]; 
      float direction = ray_dir[i]; 
      float min       = box_min[i]; 
      float max       = box_max[i]; 

      if ( direction ) 
      { 
        // calculate intersections to the slab 
        direction = 1.0f / direction; 
        float t1 = (min - origin) * direction; 
        float t2 = (max - origin) * direction; 

        if ( t1 > t2 ) 
        { 
          if ( t2 > tnear ) tnear = t2; 
          if ( t1 < tfar ) tfar = t1; 
        } 
        else 
        { 
          if ( t1 > tnear ) tnear = t1; 
          if ( t2 < tfar ) tfar = t2; 
        } 

        // failed intersecting this axis or intersecting behind the origin 
        if ( tnear > tfar || tfar < 0 ) 
            return false; 
      } 
      else 
      { 
        // ray parallel to axis/slab being tested against 
        if ( origin < min || origin > max ) 
          return false; 
      } 
    } 
 
    // ray hits box, compute 2 points 
    res_near  = ray_orig + tnear * ray_dir;
    res_far   = ray_orig + tfar  * ray_dir;
    return true; 
  }      

  //- prüfen, ob drei Punkte im Gegenuhrzeigersinn liegen (2D)
  //- gibt -1 zurück, falls im Uhrzeigersinn
  //- gibt +1 zurück, falls im Gegenuhrzeigersinn
  template< typename P >
  int ccwXY( const P& p0, const P& p1, const P& p2 )
  {
    float dx1 = p1.x - p0.x; 
    float dy1 = p1.y - p0.y;
    float dx2 = p2.x - p0.x;
    float dy2 = p2.y - p0.y;
    if ( dx1 * dy2 > dy1 * dx2 ) return +1;
    if ( dx1 * dy2 < dy1 * dx2 ) return -1;
    if ( ( dx1 * dx2 < 0 ) || ( dy1 * dy2 < 0 ) ) return -1;
    if ( ( dx1 * dx1 + dy1 * dy1 ) < ( dx2 * dx2 + dy2 * dy2 ) ) return +1;
    return 0;
  }

  //- prüfen, ob drei Punkte im Gegenuhrzeigersinn liegen (2D)
  //- gibt -1 zurück, falls im Uhrzeigersinn
  //- gibt +1 zurück, falls im Gegenuhrzeigersinn
  template< typename P >
  int ccwXZ( const P& p0, const P& p1, const P& p2 )
  {
    float dx1 = p1.x - p0.x; 
    float dz1 = p1.z - p0.z;
    float dx2 = p2.x - p0.x;
    float dz2 = p2.z - p0.z;
    if ( dx1 * dz2 > dz1 * dx2 ) return +1;
    if ( dx1 * dz2 < dz1 * dx2 ) return -1;
    if ( ( dx1 * dx2 < 0 ) || ( dz1 * dz2 < 0 ) ) return -1;
    if ( ( dx1 * dx1 + dz1 * dz1 ) < ( dx2 * dx2 + dz2 * dz2 ) ) return +1;
    return 0;
  }

  template < typename T >
  bool is_point_in_poly( int num_edges, T *xp, T *yp, T x, T y)
  {
    int i, j;
    bool c = 0;
    for (i = 0, j = num_edges-1; i < num_edges; j = i++) {
      if ((((yp[i]<=y) && (y<yp[j])) ||
           ((yp[j]<=y) && (y<yp[i]))) &&
          (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))

        c = !c;
    }
    return c;
  }

}
#endif// _math_util_h
