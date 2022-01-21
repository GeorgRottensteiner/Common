#include <Math/plane.h>

#include "MathUtil.h"



namespace math
{

  bool PolygonsIntersect( const std::vector<GR::tFPoint>& Points1, const GR::tFPoint& Center1,
                          const std::vector<GR::tFPoint>& Points2, const GR::tFPoint& Center2 )
  {
    GR::tFPoint axis;

    float       tmp, 
                minA, 
                maxA, 
                minB, 
                maxB;

	  // test polygon A's sides
    for ( size_t side = 0; side < Points1.size(); ++side )
	  {
		  // get the axis that we will project onto
		  if ( side == 0 )
		  {
        axis.x = Points1[Points1.size() - 1].y - Points1[0].y;
			  axis.y = Points1[0].x - Points1[Points1.size() - 1].x;
		  }
		  else
		  {
			  axis.x = Points1[side - 1].y - Points1[side].y;
			  axis.y = Points1[side].x - Points1[side - 1].x;
		  }

		  // normalize the axis
		  tmp = sqrt( axis.x * axis.x + axis.y * axis.y );
		  axis.x /= tmp;
		  axis.y /= tmp;

		  // project polygon A onto axis to determine the min/max
		  minA = maxA = Points1[0].x * axis.x + Points1[0].y * axis.y;
		  for ( size_t i = 1; i < Points1.size(); i++ )
		  {
			  tmp = Points1[i].x * axis.x + Points1[i].y * axis.y;
			  if ( tmp > maxA )
        {
				  maxA = tmp;
        }
			  else if ( tmp < minA )
        {
				  minA = tmp;
        }
		  }
		  // correct for offset
		  tmp = Center1.x * axis.x + Center1.y * axis.y;
		  minA += tmp;
		  maxA += tmp;

		  // project polygon B onto axis to determine the min/max
		  minB = maxB = Points2[0].x * axis.x + Points2[0].y * axis.y;
		  for ( size_t i = 1; i < Points2.size(); i++ )
		  {
			  tmp = Points2[i].x * axis.x + Points2[i].y * axis.y;
			  if ( tmp > maxB )
        {
				  maxB = tmp;
        }
			  else if ( tmp < minB )
        {
				  minB = tmp;
        }
		  }
		  // correct for offset
		  tmp = Center2.x * axis.x + Center2.y * axis.y;
		  minB += tmp;
		  maxB += tmp;

		  // test if lines intersect, if not, return false
		  if ( ( maxA < minB )
      ||   ( minA > maxB ) )
      {
			  return false;
      }
	  }

	  // test polygon B's sides
	  for ( size_t side = 0; side < Points2.size(); side++ )
	  {
		  // get the axis that we will project onto
		  if ( side == 0 )
		  {
			  axis.x = Points2[Points2.size() - 1].y - Points2[0].y;
			  axis.y = Points2[0].x - Points2[Points2.size() - 1].x;
		  }
		  else
		  {
			  axis.x = Points2[side - 1].y - Points2[side].y;
			  axis.y = Points2[side].x - Points2[side - 1].x;
		  }

		  // normalize the axis
		  tmp = sqrt( axis.x * axis.x + axis.y * axis.y );
		  axis.x /= tmp;
		  axis.y /= tmp;

		  // project polygon A onto axis to determine the min/max
		  minA = maxA = Points1[0].x * axis.x + Points1[0].y * axis.y;
		  for ( size_t i = 1; i < Points1.size(); i++ )
		  {
			  tmp = Points1[i].x * axis.x + Points1[i].y * axis.y;
			  if ( tmp > maxA )
        {
				  maxA = tmp;
        }
			  else if ( tmp < minA )
        {
				  minA = tmp;
        }
		  }
		  // correct for offset
		  tmp = Center1.x * axis.x + Center1.y * axis.y;
		  minA += tmp;
		  maxA += tmp;

		  // project polygon B onto axis to determine the min/max
		  minB = maxB = Points2[0].x * axis.x + Points2[0].y * axis.y;
		  for ( size_t i = 1; i < Points2.size(); i++ )
		  {
			  tmp = Points2[i].x * axis.x + Points2[i].y * axis.y;
			  if ( tmp > maxB )
        {
				  maxB = tmp;
        }
			  else if ( tmp < minB )
        {
				  minB = tmp;
        }
		  }
		  // correct for offset
		  tmp = Center2.x * axis.x + Center2.y * axis.y;
		  minB += tmp;
		  maxB += tmp;

		  // test if lines intersect, if not, return false
		  if ( ( maxA < minB )
      ||   ( minA > maxB ) )
      {
			  return false;
      }
	  }
    return true;
  }



  bool IntersectCircleWithCircle2d( const GR::tFPoint& CirclePos1, const GR::f32 Radius1,
                                    const GR::tFPoint& CirclePos2, const GR::f32 Radius2 )
  {
    return ( ( CirclePos2 - CirclePos1 ).length() < Radius1 + Radius2 );
  }



  bool IntersectCircleWithBox2d( const GR::tFPoint& CirclePos, const GR::f32 Radius,
                                 const GR::tFPoint& BoxMin, const GR::tFPoint& BoxMax )
  {
    float dmin = 0;
    if ( CirclePos.x < BoxMin.x )
    {
      dmin += ( CirclePos.x - BoxMin.x ) * ( CirclePos.x - BoxMin.x );
    }
    else if ( CirclePos.x > BoxMax.x )
    {
      dmin += ( CirclePos.x - BoxMax.x ) * ( CirclePos.x - BoxMax.x );
    }
    if ( CirclePos.y < BoxMin.y )
    {
      dmin += ( CirclePos.y - BoxMin.y ) * ( CirclePos.y - BoxMin.y );
    }
    else if ( CirclePos.y > BoxMax.y )
    {
      dmin += ( CirclePos.y - BoxMax.y ) * ( CirclePos.y - BoxMax.y );
    }

    return ( dmin < Radius * Radius );
  }



  bool IntersectCircleWithTriangle2d( const GR::tFPoint& CirclePos, const GR::f32 Radius,
                                      const GR::tFPoint& Pos1, const GR::tFPoint& Pos2, const GR::tFPoint& Pos3 )
  {
    if ( PointInTriangle2d( CirclePos, Pos1, Pos2, Pos3 ) )
    {
      return true;
    }

    if ( IntersectCircleWithLineSegment2d( Pos1, Pos2, CirclePos, Radius ) )
    {
      return true;
    }
    if ( IntersectCircleWithLineSegment2d( Pos2, Pos3, CirclePos, Radius ) )
    {
      return true;
    }
    if ( IntersectCircleWithLineSegment2d( Pos3, Pos1, CirclePos, Radius ) )
    {
      return true;
    }
    return false;
  }



  bool IntersectQuadWithLine( const math::vector3& Pick1, const math::vector3& Pick2, 
                              const math::vector3& vect1, const math::vector3& vect2,
                              const math::vector3& vect3, const math::vector3& vect4,
                              math::vector3& vResult, GR::tVector* pVectNormal )
  {
    math::vector3           vecNormal1, 
                            vecNormal2;

    math::plane             plane1,
                            plane2;

    plane1.FromPoints( vect1, vect2, vect3 );
    plane2.FromPoints( vect3, vect2, vect4 );

    vecNormal1 = ( vect1 - vect2 ).cross( vect3 - vect1 );
    vecNormal2 = ( vect3 - vect4 ).cross( vect4 - vect2 );

    math::plane             planeTriangle1,
                            planeTriangle2,
                            planeTriangle3,
                            planeTriangle4,
                            planeTriangle5,
                            planeTriangle6;

    planeTriangle1.FromPoints( vect1, vect2, vect1 + vecNormal1 );
    planeTriangle2.FromPoints( vect2, vect3, vect2 + vecNormal1 );
    planeTriangle3.FromPoints( vect3, vect1, vect3 + vecNormal1 );

    planeTriangle4.FromPoints( vect2, vect4, vect2 + vecNormal2 );
    planeTriangle5.FromPoints( vect4, vect3, vect4 + vecNormal2 );
    planeTriangle6.FromPoints( vect3, vect2, vect3 + vecNormal2 );

    if ( plane1.Intersect( Pick1, Pick2, vResult ) )
    {
      if ( plane1.Distance( Pick1 ) >= 0.0f )
      {
        // prüfen, ob im Dreieck enthalten!
        if ( ( planeTriangle1.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle2.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle3.Distance( vResult ) >= 0.0f ) )
        {
          if ( pVectNormal )
          {
            *pVectNormal = -vecNormal1.normalized();
          }
          return true;
        }
      }
    }
    if ( plane2.Intersect( Pick1, Pick2, vResult ) )
    {
      if ( plane2.Distance( Pick1 ) >= 0.0f )
      {
        // prüfen, ob im Dreieck enthalten!
        if ( ( planeTriangle4.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle5.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle6.Distance( vResult ) >= 0.0f ) )
        {
          if ( pVectNormal )
          {
            *pVectNormal = -vecNormal2.normalized();
          }
          return true;
        }
      }
    }
    return false;
  }



  /*
  bool IntersectTriangleWithLineSegment( const GR::tVector& vert0,
                                         const GR::tVector& vert1,
                                         const GR::tVector& vert2,
                                         const GR::tVector& orig,
                                         const GR::tVector& vectLine2,
                                         GR::tVector& vectResult )
  {

    GR::tVector   dir = vectLine2 - orig;

    GR::f32 det,inv_det;

    // find vectors for two edges sharing vert0 
    GR::tVector     edge1 = vert1 - vert0;
    GR::tVector     edge2 = vert2 - vert0;

    // begin calculating determinant - also used to calculate U parameter 
    GR::tVector     pvec = dir.cross( edge2 );

    // if determinant is near zero, ray lies in plane of triangle 
    det = edge1.dot( pvec );

    // calculate distance from vert0 to ray origin 
    GR::tVector   tvec = orig - vert0;

    inv_det = 1.0f / det;
  
    GR::tVector   qvec = tvec.cross( edge1 );
    
    if ( det > 0.000001f )
    {
      vectResult.x = tvec.dot( pvec );
      if (vectResult.x < 0.0 || vectResult.x > det)
	  return 0;
          
      // calculate V parameter and test bounds
      vectResult.y = dir.dot( qvec );
      if (vectResult.y < 0.0 || vectResult.x + vectResult.y > det)
	  return 0;
    
    }
    else if(det < -0.000001f )
    {
      // calculate U parameter and test bounds 
      vectResult.x = tvec.dot( pvec );
      if (vectResult.x > 0.0 || vectResult.x < det)
	  return 0;
    
      // calculate V parameter and test bounds 
      vectResult.y = dir.dot( qvec );
      if (vectResult.y > 0.0 || vectResult.x + vectResult.y < det)
	  return 0;
    }
    else return 0;  // ray is parallell to the plane of the triangle

    vectResult.z = edge2.dot( qvec ) * inv_det;

    vectResult.x *= inv_det;
    vectResult.y *= inv_det;

    vectResult += vert0;

    return true;
  }
  */

    /*
    const GR::f32 SMALL_NUM = 0.00000001f;

    // get triangle edge vectors and plane normal
    GR::tVector u = vectTriangle2 - vectTriangle1;
    GR::tVector v = vectTriangle3 - vectTriangle1;

    GR::tVector n = u.cross( v );

    if ( n.length_squared() == 0.0f )
    {
      // triangle is degenerate
      return false;
    }

    GR::tVector   dir = vectLine2 - vectLine1;
    GR::tVector   w0  = vectTriangle1 - vectLine1;
    GR::f32       a = -n.dot( w0 );
    GR::f32       b = n.dot( dir );

    if ( fabs( b ) < SMALL_NUM )
    {
      // ray is parallel to triangle plane
      if ( a == 0.0f )
      {
        // ray lies in triangle plane
        return true;
      }
      else
      {
        return false;
      }
      // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    GR::f32 r = a / b;
    if ( r < 0.0f )
    {
      // ray goes away from triangle
      return false;
    }

    // for a segment, also test if (r > 1.0) => no intersect
    vectResult = vectLine1 + dir * r;  // intersect point of ray and plane

    // is I inside T?
    GR::f32 uu = u.dot( u );
    GR::f32 uv = u.dot( v );
    GR::f32 vv = v.dot( v );
    GR::tVector w = vectResult - vectTriangle1;
    GR::f32 wu = w.dot( u );
    GR::f32 wv = w.dot( v );
    GR::f32 D = uv * uv - uu * vv;

    // get and test parametric coords
    GR::f32 s = ( uv * wv - vv * wu ) / D;
    if ( ( s < 0.0f )
    ||   ( s > 1.0f ) )
    {
      // I is outside T
      return false;
    }
    GR::f32 t = ( uv * wu - uu * wv ) / D;
    if ( ( t < 0.0f )
    ||   ( s + t > 1.0f ) )
    {
      // I is outside T
      return false;
    }
    return true;
    */




  bool IntersectTriangleWithLineSegment( const GR::tVector& vPick1, const GR::tVector& vPick2,
                                         const GR::tVector& vect1, const GR::tVector& vect2,
                                         const GR::tVector& vect3, GR::tVector& vResult )
  {
    math::vector3           vecNormal1;

    math::plane             plane1;

    plane1.FromPoints( vect1, vect2, vect3 );

    vecNormal1 = ( vect1 - vect2 ).cross( vect3 - vect1 );

    math::plane             planeTriangle1,
                            planeTriangle2,
                            planeTriangle3;

    planeTriangle1.FromPoints( vect1, vect2, vect1 + vecNormal1 );
    planeTriangle2.FromPoints( vect2, vect3, vect2 + vecNormal1 );
    planeTriangle3.FromPoints( vect3, vect1, vect3 + vecNormal1 );

    if ( plane1.Intersect( vPick1, vPick2, vResult ) )
    {
      if ( plane1.Distance( vPick1 ) >= 0.0f )
      {
        GR::f32     fPosOnSegment = ( vResult.x - vPick1.x ) / ( vPick2.x - vPick1.x );
        //vResult = ( 1 - t ) * vPick2 + t * vPick1;

        if ( ( fPosOnSegment < 0.0f )
        ||   ( fPosOnSegment >= 1.0f ) )
        {
          return false;
        }

        // prüfen, ob im Dreieck enthalten!
        if ( ( planeTriangle1.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle2.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle3.Distance( vResult ) >= 0.0f ) )
        {
          return true;
        }
      }
    }
    return false;
  }



  bool IntersectTriangleWithLine( const math::vector3& vPick1, const math::vector3& vPick2, 
                                  const math::vector3& vect1, const math::vector3& vect2,
                                  const math::vector3& vect3, math::vector3& vResult )
  {
    math::vector3           vecNormal1;

    math::plane             plane1;

    plane1.FromPoints( vect1, vect2, vect3 );

    vecNormal1 = ( vect1 - vect2 ).cross( vect3 - vect1 );

    math::plane             planeTriangle1,
                            planeTriangle2,
                            planeTriangle3;

    planeTriangle1.FromPoints( vect1, vect2, vect1 + vecNormal1 );
    planeTriangle2.FromPoints( vect2, vect3, vect2 + vecNormal1 );
    planeTriangle3.FromPoints( vect3, vect1, vect3 + vecNormal1 );

    if ( plane1.Intersect( vPick1, vPick2, vResult ) )
    {
      if ( plane1.Distance( vPick1 ) >= 0.0f )
      {
        // prüfen, ob im Dreieck enthalten!
        if ( ( planeTriangle1.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle2.Distance( vResult ) >= 0.0f )
        &&   ( planeTriangle3.Distance( vResult ) >= 0.0f ) )
        {
          return true;
        }
      }
    }
    return false;
  }



  int IntersectCircleWithLine2d( const GR::tFPoint& Point1, const GR::tFPoint& Point2,
                                 const GR::tFPoint& CircleCenter, const float Radius,
                                 GR::tFPoint& Intersection1, GR::tFPoint& Intersection2 )
  {
    // x1,y1,z1  P1 coordinates (point of line)
    // x2,y2,z2  P2 coordinates (point of line)
    // x3,y3,z3, r  P3 coordinates and radius (sphere)
    // x,y,z   intersection coordinates
    //
    // This function returns a pointer array which first index indicates
    // the number of intersection point, followed by coordinate pairs.

    float a, b, c, mu, i ;

    a = ( Point2.x - Point1.x ) * ( Point2.x - Point1.x )
      + ( Point2.y - Point1.y ) * ( Point2.y - Point1.y );
    b =  2 * ( ( Point2.x - Point1.x ) * ( Point1.x - CircleCenter.x )
             + ( Point2.y - Point1.y ) * ( Point1.y - CircleCenter.y ) );
    c = CircleCenter.x * CircleCenter.x 
      + CircleCenter.y * CircleCenter.y
      + Point1.x * Point1.x
      + Point1.y * Point1.y
      - 2 * ( CircleCenter.x * Point1.x + CircleCenter.y * Point1.y ) - Radius * Radius;

    i = b * b - 4 * a * c;

    if ( i < 0.0f )
    {
      // no intersection
      return 0;
    }
    if ( i == 0.0f )
    {
      // one intersection
      mu = -b / ( 2 * a );

      Intersection1 = Point1 + mu * ( Point2 - Point1 );
      return 1;
    }
    // two intersections

    // first intersection
    mu = ( -b + sqrt( b * b - 4 * a * c ) ) / ( 2 * a );
    Intersection1 = Point1 + mu * ( Point2 - Point1 );

    // second intersection
    mu = (- b - sqrt( b * b - 4 * a * c ) ) / ( 2 * a );

    Intersection2 = Point1 + mu * ( Point2 - Point1 );

    return 2;
  }



  bool IntersectCircleWithLineSegment2d( const GR::tFPoint& Point1, const GR::tFPoint& Point2,
                                         const GR::tFPoint& CircleCenter, const float Radius )
  {
    if ( Point1 == Point2 )
    {
      return ( Point1.distance( CircleCenter ) <= Radius );
    }

    float     fU = ( ( CircleCenter.x - Point1.x ) * ( Point2.x - Point1.x ) + ( CircleCenter.y - Point1.y ) * ( Point2.y - Point1.y ) )
                 / ( ( Point2.x - Point1.x ) * ( Point2.x - Point1.x ) + ( Point2.y - Point1.y ) * ( Point2.y - Point1.y ) );

    if ( ( fU < 0 )
    ||   ( fU > 1 ) )
    {
      // der beste Punkt ist nicht innerhalb des Segments, aber die Segment-Endpunkte könnten innerhalb des Kreises sein
      if ( Point1.distance( CircleCenter ) <= Radius )
      {
        return true;
      }
      if ( Point2.distance( CircleCenter ) <= Radius )
      {
        return true;
      }
      return false;
    }
    GR::tFPoint   ClosestPoint = Point1 + ( Point2 - Point1 ) * fU;

    return ( ClosestPoint.distance( CircleCenter ) <= Radius );
  }



  bool IntersectLineWithLine2d( const GR::tFPoint& Pos1A, const GR::tFPoint& Pos1B,
                                const GR::tFPoint& Pos2A, const GR::tFPoint& Pos2B,
                                GR::tFPoint& Result )
  {
    float     divisor = ( ( Pos2B.x - Pos2A.x ) * ( Pos1A.y - Pos1B.y ) - ( Pos2B.y - Pos2A.y ) * ( Pos1A.x - Pos1B.x ) );
    if ( divisor == 0.0f )
    {
      // no intersection
      return false;
    }
    float counter = ( Pos1A.x - Pos1B.x ) * Pos2A.y - ( Pos1A.x - Pos1B.x ) * Pos1A.y - Pos2A.x * ( Pos1A.y - Pos1B.y ) + Pos1A.x * ( Pos1A.y - Pos1B.y );
    float l = counter / divisor;
    Result = Pos2A + l * ( Pos2B - Pos2A );
    return true;
  }



  int IntersectLineSegmentWithLineSegment2d( const GR::tFPoint& Segment1A, const GR::tFPoint& Segment1B,
                                             const GR::tFPoint& Segment2A, const GR::tFPoint& Segment2B,
                                             GR::tFPoint* Intersection1, GR::tFPoint* Intersection2 )
  {
    GR::tFPoint   u = Segment1B - Segment1A;
    GR::tFPoint   v = Segment2B - Segment2A;
    GR::tFPoint   w = Segment1A - Segment2A;

    float         D = u.perpendicular( v );

    // test if they are parallel (includes either being a point)
    if ( fabs( D ) < 0.000001f ) 
    {          
      // S1 and S2 are parallel
      if ( ( u.perpendicular( w ) != 0 )
      ||   ( v.perpendicular( w ) != 0 ) ) 
      {
        // they are NOT collinear
        return 0;                   
      }

      // they are collinear or degenerate
      // check if they are degenerate points
      float du = u.dot( u );
      float dv = v.dot( v );

      if ( ( du == 0 )
      &&   ( dv == 0 ) ) 
      {
        // both segments are points

        if ( Segment1A != Segment2A )
        {
          // they are distinct points
          return 0;
        }
        // they are the same point
        if ( Intersection1 )
        {
          *Intersection1 = Segment1A;                
        }
        return 1;
      }
      if ( du == 0 ) 
      {
        // S1 is a single point
        if ( PointInLineSegment2d( Segment1A, Segment2A, Segment2B ) == 0 )  
        {
          // but is not in S2
          return 0;
        }
        if ( Intersection1 )
        {
          *Intersection1 = Segment1A;                
        }
        return 1;
      }
      if ( dv == 0 ) 
      {
        // S2 a single point
        if ( PointInLineSegment2d( Segment2A, Segment1A, Segment1B ) == 0 )
        {
          // but is not in S1
          return 0;
        }
        if ( Intersection1 )
        {
          *Intersection1 = Segment2A;                
        }
        return 1;
      }

      // they are collinear segments - get overlap (or not)
      float t0, t1;                   // endpoints of S1 in eqn for S2

      GR::tFPoint w2 = Segment1B - Segment2A;

      if ( v.x != 0 ) 
      {
        t0 = w.x / v.x;
        t1 = w2.x / v.x;
      }
      else 
      {
        t0 = w.y / v.y;
        t1 = w2.y / v.y;
      }
      if ( t0 > t1 ) 
      {
        // must have t0 smaller than t1
        float t = t0; 
        t0 = t1; 
        t1 = t;    // swap if not
      }
      if ( ( t0 > 1 )
      ||   ( t1 < 0 ) ) 
      {
        return 0;     // NO overlap
      }
      t0 = t0 < 0 ? 0 : t0;              // clip to min 0
      t1 = t1 > 1 ? 1 : t1;              // clip to max 1
      if ( t0 == t1 ) 
      {                 
        // intersect is a point
        if ( Intersection1 )
        {
          *Intersection1 = Segment2A + t0 * v;
        }
        return 1;
      }

      // they overlap in a valid subsegment
      if ( Intersection1 )
      {
        *Intersection1 = Segment2A + t0 * v;
      }
      if ( Intersection2 )
      {
        *Intersection2 = Segment2A + t1 * v;
      }
      return 2;
    }

    // the segments are skew and may intersect in a point
    // get the intersect parameter for S1
    float     sI = v.perpendicular( w ) / D;

    if ( ( sI < 0 )
    ||   ( sI > 1 ) )
    {
      // no intersect with S1
      return 0;
    }

    // get the intersect parameter for S2
    float     tI = u.perpendicular( w ) / D;

    if ( ( tI < 0 )
    ||   ( tI > 1 ) )
    {
      // no intersect with S2
      return 0;
    }

    if ( Intersection1 )
    {
      *Intersection1 = Segment1A + sI * u;               // compute S1 intersect point
    }
    return 1;
  }



  bool PointInLineSegment2d( const GR::tFPoint& Point,
                             const GR::tFPoint& SegmentA, const GR::tFPoint& SegmentB )
  {

    // TODO - das kann ja nicht wirklich richtig sein??
    // nur richtig, wenn der Punkt bereits sicher auf der Linie liegt, prüft nur auf Segmentränder!!
    if ( SegmentA.x != SegmentB.x ) 
    {
      // S is not vertical
      if ( ( SegmentA.x <= Point.x )
      &&   ( Point.x <= SegmentB.x ) )
      {
        return true;
      }
      if ( ( SegmentA.x >= Point.x )
      &&   ( Point.x >= SegmentB.x ) )
      {
        return true;
      }
    }
    else 
    {
      // S is vertical, so test y coordinate
      if ( ( SegmentA.y <= Point.y )
      &&   ( Point.y <= SegmentB.y ) )
      {
        return true;
      }
      if ( ( SegmentA.y >= Point.y )
      &&   ( Point.y >= SegmentB.y ) )
      {
        return true;
      }
    }
    return false;

  }



  float ReflectAngle( const float SurfaceNormalAngle, const float IncomingAngle )
  {
    return 2 * SurfaceNormalAngle - IncomingAngle + 180.0f;
  }



  bool PointInTriangle2d( const GR::tFPoint& Point,
                          const GR::tFPoint& Triangle1, 
                          const GR::tFPoint& Triangle2,
                          const GR::tFPoint& Triangle3 )
  {
    float     fAB = ( Point.y - Triangle1.y ) * ( Triangle2.x - Triangle1.x ) 
                  - ( Point.x - Triangle1.x ) * ( Triangle2.y - Triangle1.y );
    float     fCA = ( Point.y - Triangle3.y ) * ( Triangle1.x - Triangle3.x ) 
                  - ( Point.x - Triangle3.x ) * ( Triangle1.y - Triangle3.y );
    float     fBC = ( Point.y - Triangle2.y ) * ( Triangle3.x - Triangle2.x ) 
                  - ( Point.x - Triangle2.x ) * ( Triangle3.y - Triangle2.y );

    if ( ( fAB * fBC > 0.0f )
    &&   ( fBC * fCA > 0.0f ) )
    {
      return true;
    }
    return false;
  }



  int OrientationPointToLine2d( GR::tFPoint& X, GR::tFPoint& Pos1, GR::tFPoint& Pos2 )
  {
    // Linear determinant of the 3 points
    float Orin = ( Pos2.x - Pos1.x ) * ( X.y - Pos1.y ) - ( X.x - Pos1.x ) * ( Pos2.y - Pos1.y );

    if ( Orin > 0 )
    {
      return 1;
    }
    else if ( Orin < 0 )
    {
      return -1;
    }
    return 0;
  }



  float RotateToAngle( const float StartAngle, const float EndAngle, const float Delta )
  {
    float   fResult = StartAngle;

    // Überlauf ausgleichen
    if ( fabsf( fResult - EndAngle ) > 180.0f )
    {
      if ( fResult < EndAngle )
      {
        fResult += 360.0f;
      }
      else
      {
        fResult -= 360.0f;
      }
    }

    if ( fResult < EndAngle )
    {
      // gegen Uhrzeigersinn drehen
      fResult += Delta;
      if ( fResult > EndAngle )
      {
        fResult = EndAngle;
      }
    }
    else if ( fResult > EndAngle )
    {
      // gegen Uhrzeigersinn drehen
      fResult -= Delta;
      if ( fResult < EndAngle )
      {
        fResult = EndAngle;
      }
    }

    return fResult;
  }



  int Round( const float Value )
  {
    int     value = (int)( fabsf( Value ) + 0.5f );

    if ( Value < 0.0f )
    {
      return -value;
    }
    return value;
  }



  GR::f32 DistancePointLineSegment2d( const GR::tFPoint& Point, 
                                      const GR::tFPoint& Line1,
                                      const GR::tFPoint& Line2 )
  {
    GR::f32 fLineLength = ( Line2 - Line1 ).length();

    GR::f32 fU = ( ( ( Point.x - Line1.x ) * ( Line2.x - Line1.x ) ) +
        ( ( Point.y - Line1.y ) * ( Line2.y - Line1.y ) ) ) 
        / ( fLineLength * fLineLength );


    if ( fU < 0.0f )
    {
      return ( Line1 - Point ).length();
    }
    if ( fU > 1.0f )
    {
      return ( Line2 - Point ).length();
    }

    GR::tFPoint   Closest = Line1 + fU * ( Line2 - Line1 );

    return ( Closest - Point ).length();

  }



  GR::f32 DistancePointLine2d( const GR::tFPoint& Point, 
                               const GR::tFPoint& Line1,
                               const GR::tFPoint& Line2 )
  {
    GR::f32 fLineLength = ( Line2 - Line1 ).length();

    GR::f32 fU = ( ( ( Point.x - Line1.x ) * ( Line2.x - Line1.x ) ) +
        ( ( Point.y - Line1.y ) * ( Line2.y - Line1.y ) ) ) 
        / ( fLineLength * fLineLength );


    GR::tFPoint   Closest = Line1 + fU * ( Line2 - Line1 );

    return ( Closest - Point ).length();

  }

  bool ClosestPointToLine2d( const GR::tFPoint& Point, 
                             const GR::tFPoint& Line1,
                             const GR::tFPoint& Line2,
                             GR::tFPoint& ClosestPoint )
  {

    GR::f32 fLineLength = ( Line2 - Line1 ).length();

    GR::f32 fU = ( ( ( Point.x - Line1.x ) * ( Line2.x - Line1.x ) ) +
        ( ( Point.y - Line1.y ) * ( Line2.y - Line1.y ) ) ) 
        / ( fLineLength * fLineLength );


    ClosestPoint = Line1 + fU * ( Line2 - Line1 );

    return true;

  }

  bool ClosestPointToLineSegment2d( const GR::tFPoint& Point, 
                                    const GR::tFPoint& Line1,
                                    const GR::tFPoint& Line2,
                                    GR::tFPoint& ClosestPoint )
  {

    GR::f32 fLineLength = ( Line2 - Line1 ).length();

    GR::f32 fU = ( ( ( Point.x - Line1.x ) * ( Line2.x - Line1.x ) ) +
        ( ( Point.y - Line1.y ) * ( Line2.y - Line1.y ) ) ) 
        / ( fLineLength * fLineLength );


    if ( fU < 0.0f )
    {
      return false;
    }
    if ( fU > 1.0f )
    {
      return false;
    }

    ClosestPoint = Line1 + fU * ( Line2 - Line1 );

    return true;

  }



  GR::f32 DistancePointToLine3d( const GR::tVector& vectPoint, const GR::tVector& vectLine1, const GR::tVector& vectLine2 )
  {
    GR::tVector   vDir = vectLine2 - vectLine1;
    GR::tVector   w    = vectPoint - vectLine1;

    GR::f32       c1 = w.dot( vDir );
    GR::f32       c2 = vDir.dot( vDir );

    GR::f32       b = c1 / c2;

    GR::tVector   vectResult = vectLine1 + b * vDir;

    return ( vectPoint - vectResult ).length();
  }



  bool ClosestPointToLine3d( const GR::tVector& vectPoint, const GR::tVector& vectLine1, const GR::tVector& vectLine2, GR::tVector& vectResult )
  {
    GR::tVector   vDir = vectLine2 - vectLine1;
    GR::tVector   w    = vectPoint - vectLine1;

    GR::f32       c1 = w.dot( vDir );
    GR::f32       c2 = vDir.dot( vDir );

    GR::f32       b = c1 / c2;

    vectResult = vectLine1 + b * vDir;
    return true;
  }

  GR::f32 PositionPointOnLine3d( const GR::tVector& vectPoint, const GR::tVector& vectLine1, const GR::tVector& vectLine2 )
  {
    GR::tVector   vDir = vectLine2 - vectLine1;
    GR::tVector   w    = vectPoint - vectLine1;

    GR::f32       c1 = w.dot( vDir );
    GR::f32       c2 = vDir.dot( vDir );

    return c1 / c2;
  }

  GR::f32 PositionPointOnLine2d( const GR::tFPoint& Point, const GR::tFPoint& Line1, const GR::tFPoint& Line2 )
  {
    GR::tFPoint   vDir = Line2 - Line1;
    GR::tFPoint   w    = Point - Line1;

    GR::f32       c1 = w.dot( vDir );
    GR::f32       c2 = vDir.dot( vDir );

    return c1 / c2;
  }

  GR::f32 AngleDistance( const GR::f32 fAngle1, const GR::f32 fAngle2 )
  {

    GR::f32     fA1 = fAngle1;
    GR::f32     fA2 = fAngle2;

    while ( fA1 < 0.0f )
    {
      fA1 += 360.0f;
    }
    fA1 = fmodf( fA1, 360.0f );

    while ( fA2 < 0.0f )
    {
      fA2 += 360.0f;
    }
    fA2 = fmodf( fA2, 360.0f );

    GR::f32     fDist1 = fabsf( fA1 - fA2 );
    GR::f32     fDist2 = fabsf( fA1 - fA2 + 360.0f );
    GR::f32     fDist3 = fabsf( fA1 - fA2 - 360.0f );

    if ( fDist3 < fDist1 )
    {
      return fDist3 < fDist2 ? fDist3 : fDist2;
    }
    return fDist1 < fDist2 ? fDist1 : fDist2;

  }


  GR::i32 Signum( const GR::i32 Value )
  {
    if ( Value > 0 )
    {
      return 1;
    }
    else if ( Value < 0 )
    {
      return -1;
    }
    return 0;
  }



  GR::f32 RandomFloat( GR::f32 Range )
  {
    return (GR::f32)rand() / 10000 * Range;
  }



  bool FindRoot( float A, float B, float C, float& R )
  {
	  float d = B * B - 4.0f * A * C;
    if ( d < 0.0f )
    {
      return false;
    }
	  float inv2a = 0.5f / A;
    if ( d == 0.0f )
    {
      R = -B * inv2a;
    }
	  else
	  {
		  float sqrtd = sqrtf( d );
      R = math::minValue( ( -B - sqrtd ) * inv2a, ( -B + sqrtd ) * inv2a );
	  }
	  return true;
  }



  bool IntersectSweptSphereAndLinesegment2d( const GR::tFPoint& pos, float radius, 
                                             const GR::tFPoint& vel, 
                                             const GR::tFPoint& p1, const GR::tFPoint& p2, 
                                             float maxtime, 
                                             float& time, GR::tFPoint& intersection )
  {	
	  GR::tFPoint edge = p2 - p1;
	  GR::tFPoint diff = p1 - pos;	
			
	  float edge2 = edge.dot(edge);
	  float vel2 = vel.dot(vel);
	  float edgedotvel = edge.dot(vel);
	  float edgedotdiff = edge.dot(diff);

	  float a = edge2 * -vel2 + edgedotvel * edgedotvel;
	  float b = edge2 * 2.0f * vel.dot(diff) - 2.0f * edgedotvel * edgedotdiff;
	  float c = edge2 * (radius * radius - diff.dot(diff)) + edgedotdiff * edgedotdiff;

	  float t;
    if ( FindRoot( a, b, c, t ) )
    {
      if ( ( t >= 0.0f )
      &&   ( t < maxtime ) )
      {
		    float u = (edgedotvel * t - edgedotdiff) / edge2;

        if ( ( u >= 0.0f )
        &&   ( u < 1.0f ) )
		    {
			    time = t;
			    intersection = p1 + u * edge;
			    return true;
		    }
      }
	  }
	  return false;
  }



  GR::tFPoint MirrorPointAboutAxis( const GR::tFPoint& point, const GR::tFPoint& lineP1, const GR::tFPoint& lineP2 )
  {
    //vector y (the point)
    GR::f32 y1 = point.x - lineP1.x;
    GR::f32 y2 = point.y - lineP1.y;

    //vector u (the line)
    GR::f32 u1 = lineP2.x - lineP1.x;
    GR::f32 u2 = lineP2.y - lineP1.y;

    //orthogonal projection of y onto u
    GR::f32 scale = ( y1 * u1 + y2 * u2 ) / ( u1 * u1 + u2 * u2 );
    GR::f32 projX = scale * u1 + lineP1.x;
    GR::f32 projY = scale * u2 + lineP1.y;

    return GR::tFPoint( 2 * projX - point.x, 2 * projY - point.y );
  }



  bool IntersectLineWithCylinder( const GR::tVector& LineA, const GR::tVector& LineB, 
                                  const GR::tVector& CylinderCenter1, const GR::tVector& CylinderCenter2, const GR::f32 CylinderRadius,
                                  GR::tVector& HitPos, GR::tVector& HitNormal )
  {
    GR::tVector   dir = LineB - LineA;

    // Solution : http://www.gamedev.net/community/forums/topic.asp?topic_id=467789
    double cxmin, cymin, czmin, cxmax, cymax, czmax;
    if ( CylinderCenter1.z < CylinderCenter2.z ) 
    { 
      czmin = CylinderCenter1.z - CylinderRadius; 
      czmax = CylinderCenter2.z + CylinderRadius; 
    }
    else 
    { 
      czmin = CylinderCenter2.z - CylinderRadius; 
      czmax = CylinderCenter1.z + CylinderRadius; 
    }
    if ( CylinderCenter1.y < CylinderCenter2.y ) 
    { 
      cymin = CylinderCenter1.y - CylinderRadius; 
      cymax = CylinderCenter2.y + CylinderRadius; 
    }
    else 
    { 
      cymin = CylinderCenter2.y - CylinderRadius; 
      cymax = CylinderCenter1.y + CylinderRadius; 
    }
    if ( CylinderCenter1.x < CylinderCenter2.x ) 
    { 
      cxmin = CylinderCenter1.x - CylinderRadius; 
      cxmax = CylinderCenter2.x + CylinderRadius; 
    }
    else 
    { 
      cxmin = CylinderCenter2.x - CylinderRadius; 
      cxmax = CylinderCenter1.x + CylinderRadius; 
    }

    GR::tVector AB = CylinderCenter2 - CylinderCenter1;
    GR::tVector AO = LineA - CylinderCenter1;
    GR::tVector AOxAB = AO.cross( AB );
    GR::tVector VxAB = dir.cross( AB );
    double ab2 = AB.dot( AB );
    double a = VxAB.dot( VxAB );
    double b = 2 * VxAB.dot( AOxAB );
    double c = AOxAB.dot( AOxAB ) - ( CylinderRadius * CylinderRadius * ab2 );
    double d = b * b - 4 * a * c;
    if ( d < 0 )
    {
      return false;
    }
    double time = ( -b - sqrt( d ) ) / ( 2 * a );
    if ( time < 0 )
    {
      return false;
    }

    // intersection point
    GR::tVector intersection = LineA + dir * (GR::f32)time;        

    // intersection projected onto cylinder axis
    GR::tVector projection = CylinderCenter1 + ( AB.dot( intersection - CylinderCenter1 ) / ( GR::f32 )ab2 ) * AB;
    if ( ( projection - CylinderCenter1 ).length() + ( CylinderCenter2 - projection ).length() > AB.length() )
    {
      return false;
    }

    HitNormal = ( intersection - projection ).normalize();
    HitPos = LineA + dir * ( GR::f32 )time;
    return true;
  }



  bool IntersectLineSegmentWithCylinder( const GR::tVector& LineA, const GR::tVector& LineB,
                                         const GR::tVector& CylinderCenter1, const GR::tVector& CylinderCenter2, const GR::f32 CylinderRadius,
                                         GR::tVector& HitPos, GR::tVector& HitNormal )
  {
    GR::tVector   dir = LineB - LineA;

    // Solution : http://www.gamedev.net/community/forums/topic.asp?topic_id=467789
    double cxmin, cymin, czmin, cxmax, cymax, czmax;
    if ( CylinderCenter1.z < CylinderCenter2.z )
    {
      czmin = CylinderCenter1.z - CylinderRadius;
      czmax = CylinderCenter2.z + CylinderRadius;
    }
    else
    {
      czmin = CylinderCenter2.z - CylinderRadius;
      czmax = CylinderCenter1.z + CylinderRadius;
    }
    if ( CylinderCenter1.y < CylinderCenter2.y )
    {
      cymin = CylinderCenter1.y - CylinderRadius;
      cymax = CylinderCenter2.y + CylinderRadius;
    }
    else
    {
      cymin = CylinderCenter2.y - CylinderRadius;
      cymax = CylinderCenter1.y + CylinderRadius;
    }
    if ( CylinderCenter1.x < CylinderCenter2.x )
    {
      cxmin = CylinderCenter1.x - CylinderRadius;
      cxmax = CylinderCenter2.x + CylinderRadius;
    }
    else
    {
      cxmin = CylinderCenter2.x - CylinderRadius;
      cxmax = CylinderCenter1.x + CylinderRadius;
    }

    GR::tVector AB = CylinderCenter2 - CylinderCenter1;
    GR::tVector AO = LineA - CylinderCenter1;
    GR::tVector AOxAB = AO.cross( AB );
    GR::tVector VxAB = dir.cross( AB );
    double ab2 = AB.dot( AB );
    double a = VxAB.dot( VxAB );
    double b = 2 * VxAB.dot( AOxAB );
    double c = AOxAB.dot( AOxAB ) - ( CylinderRadius * CylinderRadius * ab2 );
    double d = b * b - 4 * a * c;
    if ( d < 0 )
    {
      return false;
    }
    double time = ( -b - sqrt( d ) ) / ( 2 * a );
    if ( ( time < 0 )
    ||   ( time >= 1.0f ) )
    {
      return false;
    }

    // intersection point
    GR::tVector intersection = LineA + dir * ( GR::f32 )time;

    // intersection projected onto cylinder axis
    GR::tVector projection = CylinderCenter1 + ( AB.dot( intersection - CylinderCenter1 ) / ( GR::f32 )ab2 ) * AB;
    if ( ( projection - CylinderCenter1 ).length() + ( CylinderCenter2 - projection ).length() > AB.length() )
    {
      return false;
    }

    HitNormal = ( intersection - projection ).normalize();
    HitPos = LineA + dir * ( GR::f32 )time;
    return true;
  }



  bool IntersectSweptSphereAndLinesegment2d( const GR::tFPoint& SpherePos,
                                             const GR::tFPoint& SphereEndpos,
                                             float Radius,
                                             const GR::tFPoint& P1, const GR::tFPoint& P2,
                                             GR::tFPoint& PointOfIntersection )
  {
    GR::tFPoint edge = P2 - P1;
    GR::tFPoint diff = P1 - SpherePos;

    float edge2       = edge.dot( edge );
    GR::tFPoint vel   = SphereEndpos - SpherePos;
    float vel2        = vel.dot( vel );
    float edgedotvel  = edge.dot( vel );
    float edgedotdiff = edge.dot( diff );

    float a = edge2 * -vel2 + edgedotvel * edgedotvel;
    float b = edge2 * 2.0f * vel.dot( diff ) - 2.0f * edgedotvel * edgedotdiff;
    float c = edge2 * ( Radius * Radius - diff.dot( diff ) ) + edgedotdiff * edgedotdiff;

    float t;
    if ( FindRoot( a, b, c, t ) )
    {
      /*
      if ( ( t >= 0.0f )
      &&   ( t < maxtime ) )
      */
      {
        float u = ( edgedotvel * t - edgedotdiff ) / edge2;

        PointOfIntersection = P1 + u * edge;
        if ( ( u >= 0.0f )
        &&   ( u < 1.0f ) )
        {
          // der Punkt ist auf dem Line-Segment!
          //time = t;
          return true;
        }
        else if ( u < 0.0f )
        {
          // im Radius-Bereich links?
          if ( ( PointOfIntersection - P1 ).length_squared() <= Radius * Radius )
          {
            return true;
          }
        }
        else if ( u >= 1.0f )
        {
          // im Radius-Bereich rechts?
          if ( ( PointOfIntersection - P2 ).length_squared() <= Radius * Radius )
          {
            return true;
          }
        }
      }
    }
    return false;
  }

}

