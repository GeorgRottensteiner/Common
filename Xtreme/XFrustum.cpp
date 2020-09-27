#include "XFrustum.h"



XFrustum::XFrustum()
{

}



XFrustum::~XFrustum()
{

}



void XFrustum::Create( math::matrix4& Projection, math::matrix4& View, XViewport& ViewPort, float ExtraBorder )
{
  Corners[C_LO_NEAR]  = GR::tVector( -1.0f - ExtraBorder,  1.0f + ExtraBorder, ViewPort.MinZ );
  Corners[C_LO_FAR]   = GR::tVector( -1.0f - ExtraBorder,  1.0f + ExtraBorder, ViewPort.MaxZ );
  Corners[C_RO_NEAR]  = GR::tVector(  1.0f + ExtraBorder,  1.0f + ExtraBorder, ViewPort.MinZ );
  Corners[C_RO_FAR]   = GR::tVector(  1.0f + ExtraBorder,  1.0f + ExtraBorder, ViewPort.MaxZ );
  Corners[C_LU_NEAR]  = GR::tVector( -1.0f - ExtraBorder, -1.0f - ExtraBorder, ViewPort.MinZ );
  Corners[C_LU_FAR]   = GR::tVector( -1.0f - ExtraBorder, -1.0f - ExtraBorder, ViewPort.MaxZ );
  Corners[C_RU_NEAR]  = GR::tVector(  1.0f + ExtraBorder, -1.0f - ExtraBorder, ViewPort.MinZ );
  Corners[C_RU_FAR]   = GR::tVector(  1.0f + ExtraBorder, -1.0f - ExtraBorder, ViewPort.MaxZ );

  math::matrix4     matProjInv,
                    matViewInv;

  matProjInv = Projection.inverse();

  matViewInv = View.inverse();

  matProjInv *= matViewInv;

  for ( int i = 0; i < C_CORNER_COUNT; ++i )
  {
    matProjInv.TransformCoord( Corners[i] );
  }

  PlaneLeft.FromPoints( Corners[C_LO_NEAR], Corners[C_LO_FAR], Corners[C_LU_NEAR] );
  PlaneRight.FromPoints( Corners[C_RO_NEAR], Corners[C_RU_NEAR], Corners[C_RO_FAR] );
  PlaneTop.FromPoints( Corners[C_LO_FAR], Corners[C_LO_NEAR], Corners[C_RO_NEAR] );
  PlaneBottom.FromPoints( Corners[C_RU_NEAR], Corners[C_LU_NEAR], Corners[C_LU_FAR] );

  PlaneZNear.FromPoints( Corners[C_RO_NEAR], Corners[C_LO_NEAR], Corners[C_LU_NEAR] );
  PlaneZFar.FromPoints( Corners[C_LO_FAR], Corners[C_RO_FAR], Corners[C_LU_FAR] );
}



bool XFrustum::IntersectWithBoundingBoxRotated( const XBoundingBox& BBox ) const
{
  return IntersectWithRotatedBox( BBox.m_RotatedBoundingBox[0],
                                  BBox.m_RotatedBoundingBox[1],
                                  BBox.m_RotatedBoundingBox[2],
                                  BBox.m_RotatedBoundingBox[3],
                                  BBox.m_RotatedBoundingBox[4],
                                  BBox.m_RotatedBoundingBox[5],
                                  BBox.m_RotatedBoundingBox[6],
                                  BBox.m_RotatedBoundingBox[7] );
}



bool XFrustum::IntersectWithBoundingBox( const XBoundingBox& BBox ) const
{
  if ( BBox.m_Transformed )
  {
    return IntersectWithBoundingBoxRotated( BBox );
  }
  return IntersectWithBoundingBox( BBox.UpperLeftCorner(), BBox.LowerRightCorner() );
}



bool XFrustum::IntersectWithBoundingBox( GR::tVector& V1, GR::tVector& V2 ) const
{
  GR::tVector   v1( V1.x, V2.y, V1.z ),
                v2( V1.x, V2.y, V2.z ),
                v3( V2.x, V2.y, V1.z ),
                v4( V2.x, V2.y, V2.z ),
                v5( V1.x, V1.y, V1.z ),
                v6( V1.x, V1.y, V2.z ),
                v7( V2.x, V1.y, V1.z ),
                v8( V2.x, V1.y, V2.z );

  return IntersectWithRotatedBox( v1, v2, v3, v4, v5, v6, v7, v8 );
}



bool XFrustum::IntersectWithRotatedBox( const GR::tVector& V1, const GR::tVector& V2,
                                        const GR::tVector& V3, const GR::tVector& V4,
                                        const GR::tVector& V5, const GR::tVector& V6,
                                        const GR::tVector& V7, const GR::tVector& V8 ) const
{

  if ( ( PlaneLeft.Distance( V1 ) > 0.0f )
  &&   ( PlaneRight.Distance( V1 ) > 0.0f )
  &&   ( PlaneTop.Distance( V1 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V1 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V1 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V1 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( PlaneLeft.Distance( V2 ) > 0.0f )
  &&   ( PlaneTop.Distance( V2 ) > 0.0f )
  &&   ( PlaneRight.Distance( V2 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V2 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V2 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V2 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( PlaneLeft.Distance( V3 ) > 0.0f )
  &&   ( PlaneTop.Distance( V3 ) > 0.0f )
  &&   ( PlaneRight.Distance( V3 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V3 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V3 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V3 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( PlaneLeft.Distance( V4 ) > 0.0f )
  &&   ( PlaneTop.Distance( V4 ) > 0.0f )
  &&   ( PlaneRight.Distance( V4 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V4 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V4 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V4 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( PlaneLeft.Distance( V5 ) > 0.0f )
  &&   ( PlaneTop.Distance( V5 ) > 0.0f )
  &&   ( PlaneRight.Distance( V5 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V5 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V5 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V5 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( PlaneLeft.Distance( V6 ) > 0.0f )
  &&   ( PlaneTop.Distance( V6 ) > 0.0f )
  &&   ( PlaneRight.Distance( V6 ) > 0.0f )
  &&   ( PlaneBottom.Distance( V6 ) > 0.0f )
  &&   ( PlaneZNear.Distance( V6 ) > 0.0f )
  &&   ( PlaneZFar.Distance( V6 ) > 0.0f ) )
  {
    return true;
  }

  if ( ( PlaneLeft.Distance( V1 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V2 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V3 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V4 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V5 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V6 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V7 ) > 0.0f )
  ||   ( PlaneLeft.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der left Plane
    return false;
  }

  if ( ( PlaneRight.Distance( V1 ) > 0.0f )
  ||   ( PlaneRight.Distance( V2 ) > 0.0f )
  ||   ( PlaneRight.Distance( V3 ) > 0.0f )
  ||   ( PlaneRight.Distance( V4 ) > 0.0f )
  ||   ( PlaneRight.Distance( V5 ) > 0.0f )
  ||   ( PlaneRight.Distance( V6 ) > 0.0f )
  ||   ( PlaneRight.Distance( V7 ) > 0.0f )
  ||   ( PlaneRight.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( PlaneZNear.Distance( V1 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V2 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V3 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V4 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V5 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V6 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V7 ) > 0.0f )
  ||   ( PlaneZNear.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( PlaneZFar.Distance( V1 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V2 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V3 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V4 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V5 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V6 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V7 ) > 0.0f )
  ||   ( PlaneZFar.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( PlaneTop.Distance( V1 ) > 0.0f )
  ||   ( PlaneTop.Distance( V2 ) > 0.0f )
  ||   ( PlaneTop.Distance( V3 ) > 0.0f )
  ||   ( PlaneTop.Distance( V4 ) > 0.0f )
  ||   ( PlaneTop.Distance( V5 ) > 0.0f )
  ||   ( PlaneTop.Distance( V6 ) > 0.0f )
  ||   ( PlaneTop.Distance( V7 ) > 0.0f )
  ||   ( PlaneTop.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( PlaneBottom.Distance( V1 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V2 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V3 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V4 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V5 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V6 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V7 ) > 0.0f )
  ||   ( PlaneBottom.Distance( V8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }
  return true;
}



bool XFrustum::IsPointInside( const GR::tVector& Point ) const
{
  if ( ( PlaneLeft.Distance( Point ) > 0.0f )
  &&   ( PlaneRight.Distance( Point ) > 0.0f )
  &&   ( PlaneTop.Distance( Point ) > 0.0f )
  &&   ( PlaneBottom.Distance( Point ) > 0.0f )
  &&   ( PlaneZNear.Distance( Point ) > 0.0f )
  &&   ( PlaneZFar.Distance( Point ) > 0.0f ) )
  {
    return true;
  }
  return false;
}