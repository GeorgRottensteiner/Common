#ifndef GR_MATH_UTIL_H
#define GR_MATH_UTIL_H

#include <Math/vector3.h>

#include <GR/GRTypes.h>

#include <vector>

#include "mathtemplates.h"



namespace math
{

  bool IntersectQuadWithLine( const math::vector3& vPick1, const math::vector3& vPick2, 
                              const math::vector3& vect1, const math::vector3& vect2,
                              const math::vector3& vect3, const math::vector3& vect4,
                              math::vector3& vResult, GR::tVector* pVectNormal = NULL );
  bool IntersectTriangleWithLine( const math::vector3& vPick1, const math::vector3& vPick2, 
                                  const math::vector3& vect1, const math::vector3& vect2,
                                  const math::vector3& vect3, math::vector3& vResult );
  bool IntersectTriangleWithLineSegment( const GR::tVector& vectTriangle1,
                                         const GR::tVector& vectTriangle2,
                                         const GR::tVector& vectTriangle3,
                                         const GR::tVector& vectLine1,
                                         const GR::tVector& vectLine2,
                                         GR::tVector& vectResult );

  bool IntersectCircleWithCircle2d( const GR::tFPoint& CirclePos1, const GR::f32 fRadius1,
                                    const GR::tFPoint& CirclePos2, const GR::f32 fRadius2 );

  bool IntersectCircleWithBox2d( const GR::tFPoint& CirclePos, const GR::f32 fRadius,
                                 const GR::tFPoint& BoxMin, const GR::tFPoint& BoxMax );

  bool IntersectCircleWithTriangle2d( const GR::tFPoint& CirclePos, const GR::f32 fRadius,
                                      const GR::tFPoint& Pos1, const GR::tFPoint& Pos2, const GR::tFPoint& Pos3 );

  int IntersectCircleWithLine2d( const GR::tFPoint& Point1, const GR::tFPoint& Point2,
                                 const GR::tFPoint& CircleCenter, const float fRadius,
                                 GR::tFPoint& Intersection1, GR::tFPoint& Intersection2 );

  bool IntersectLineWithLine2d( const GR::tFPoint& Pos1A, const GR::tFPoint& Pos1B,
                                const GR::tFPoint& Pos2A, const GR::tFPoint& Pos2B,
                                GR::tFPoint& Result );

  bool IntersectCircleWithLineSegment2d( const GR::tFPoint& Point1, const GR::tFPoint& Point2,
                                         const GR::tFPoint& CircleCenter, const float fRadius );

  int IntersectLineSegmentWithLineSegment2d( const GR::tFPoint& Segment1A, const GR::tFPoint& Segment1B,
                                             const GR::tFPoint& Segment2A, const GR::tFPoint& Segment2B,
                                             GR::tFPoint* pIntersection1 = NULL, GR::tFPoint* pIntersection2 = NULL );
  bool PointInLineSegment2d( const GR::tFPoint& Point,
                             const GR::tFPoint& SegmentA, const GR::tFPoint& SegmentB );

  bool PointInTriangle2d( const GR::tFPoint& Point,
                          const GR::tFPoint& Triangle1, 
                          const GR::tFPoint& Triangle2,
                          const GR::tFPoint& Triangle3 );

  bool PolygonsIntersect( const std::vector<GR::tFPoint>& Points1, const GR::tFPoint& Center1,
                          const std::vector<GR::tFPoint>& Points2, const GR::tFPoint& Center2 );

  float ReflectAngle( const float SurfaceNormalAngle, const float IncomingAngle );

  float RotateToAngle( const float StartAngle, const float EndAngle, const float Delta );

  GR::f32 AngleDistance( const GR::f32 Angle1, const GR::f32 Angle2 );

  int   OrientationPointToLine2d( GR::tFPoint& X, GR::tFPoint& Pos1, GR::tFPoint& Pos2 );

  int Round( const float Value );

  GR::i32 Signum( const GR::i32 iValue );

  GR::f32 DistancePointLineSegment2d( const GR::tFPoint& Point, 
                                      const GR::tFPoint& Line1,
                                      const GR::tFPoint& Line2 );
  GR::f32 DistancePointLine2d( const GR::tFPoint& Point, 
                               const GR::tFPoint& Line1,
                               const GR::tFPoint& Line2 );
  bool ClosestPointToLineSegment2d( const GR::tFPoint& Point, 
                                    const GR::tFPoint& Line1,
                                    const GR::tFPoint& Line2,
                                    GR::tFPoint& ClosestPoint );
  bool ClosestPointToLine2d( const GR::tFPoint& Point, 
                             const GR::tFPoint& Line1,
                             const GR::tFPoint& Line2,
                             GR::tFPoint& ClosestPoint );

  GR::tFPoint     MirrorPointAboutAxis( const GR::tFPoint& Point, const GR::tFPoint& LineP1, const GR::tFPoint& LineP2 );

  GR::f32         DistancePointToLine3d( const GR::tVector& Point, 
                                         const GR::tVector& Line1, 
                                         const GR::tVector& Line2 );
  GR::f32         PositionPointOnLine3d( const GR::tVector& Point, 
                                         const GR::tVector& Line1, 
                                         const GR::tVector& Line2 );
  bool            ClosestPointToLine3d( const GR::tVector& Point, 
                                        const GR::tVector& Line1, 
                                        const GR::tVector& Line2, 
                                        GR::tVector& Result );
  GR::f32         PositionPointOnLine2d( const GR::tFPoint& Point, 
                                         const GR::tFPoint& Line1, 
                                         const GR::tFPoint& Line2 );

  GR::f32         RandomFloat( GR::f32 range );

  bool            IntersectSweptSphereAndLinesegment2d( const GR::tFPoint& Pos, float Radius, const GR::tFPoint& Velocity, const GR::tFPoint& Pos1,
				                                                const GR::tFPoint& Pos2, float maxtime, float& time, GR::tFPoint& intersection );

  bool            IntersectSweptSphereAndLinesegment2d( const GR::tFPoint& SpherePos, 
                                                        const GR::tFPoint& SphereEndpos, 
                                                        float Radius, 
                                                        const GR::tFPoint& P1, const GR::tFPoint& P2, 
                                                        GR::tFPoint& PointOfIntersection );

  bool            IntersectLineWithCylinder( const GR::tVector& LineA, const GR::tVector& LineB,
                                             const GR::tVector& CylinderCenter1, const GR::tVector& CylinderCenter2, const GR::f32 CylinderRadius,
                                             GR::tVector& HitPos, GR::tVector& HitNormal );
  bool            IntersectLineSegmentWithCylinder( const GR::tVector& LineA, const GR::tVector& LineB,
                                                    const GR::tVector& CylinderCenter1, const GR::tVector& CylinderCenter2, const GR::f32 CylinderRadius,
                                                    GR::tVector& HitPos, GR::tVector& HitNormal );

}  // namespace math


#endif // GR_MATH_UTIL_H
