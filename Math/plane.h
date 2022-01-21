#ifndef MATH_PLANE_H
#define MATH_PLANE_H

#include <Math/vector3.h>



namespace math
{
  class plane
  {

    public:

      math::vector3                 Normal;
      float                         D;


      plane() : 
        Normal( math::vector3( 0,0,1 ) ), 
        D( 0 )        
      {
      }

      plane( const plane& rhs ) : 
        Normal( rhs.Normal ),
        D( rhs.D ) 
      {
      }

      plane( const math::vector3& Normal, const float& D ) :
        Normal( Normal ), 
        D( D ) 
      {
      }

      plane( const math::vector3& P1, const math::vector3& P2, const math::vector3& P3 )
      {
        FromPoints( P1, P2, P3 );
      }


      plane& FromPoints( const math::vector3& P1, const math::vector3& P2, const math::vector3& P3 )
      {
        Normal = normal( P1, P2, P3 );
        D      = -Normal.dot( P1 );

        return *this;
      }



      float Distance( const math::vector3& Point ) const
      {
        return ( Point.x * Normal.x + Point.y * Normal.y + Point.z * Normal.z + D );
      }



      bool Intersect( const math::vector3& LinePos1, const math::vector3& LinePos2, math::vector3& HitPos ) const
      {
        math::vector3   dir( LinePos2 - LinePos1 );

        float z = Normal.dot( LinePos1 ) + D;
        float n = Normal.dot( dir );

        if ( n == 0 )
        {
          return false;
        }
        float t = -z / n;

        // Schnittpunkt ist nun: pos + t * dir
        HitPos = LinePos1 + dir * t;
        return true;
      }

  };

}

#endif //MATH_PLANE_H