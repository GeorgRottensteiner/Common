#include <Xtreme\XBoundingBox.h>

#include <Math\math_util.h>
#include <Math\vector3.h>



XBoundingBox::XBoundingBox() :
  m_Transformed( false )
{
  Clear();
}



void XBoundingBox::Clear()
{
  m_Transformed = false;
  
  m_UpperLeftCorner   = math::vector3( FLT_MAX, 
                                       FLT_MAX, 
                                       FLT_MAX );
  m_LowerRightCorner  = math::vector3( -FLT_MAX, 
                                       -FLT_MAX, 
                                       -FLT_MAX );
}



void XBoundingBox::AddVertex( const math::vector3& vectNew )
{
  // upper left
  if ( vectNew.x < m_UpperLeftCorner.x )
  {
    m_UpperLeftCorner.x = vectNew.x;
  }
  if ( vectNew.y < m_UpperLeftCorner.y )
  {
    m_UpperLeftCorner.y = vectNew.y;
  }
  if ( vectNew.z < m_UpperLeftCorner.z )
  {
    m_UpperLeftCorner.z = vectNew.z;
  }
  // lower right
  if ( vectNew.x > m_LowerRightCorner.x )
  {
    m_LowerRightCorner.x = vectNew.x;
  }
  if ( vectNew.y > m_LowerRightCorner.y )
  {
    m_LowerRightCorner.y = vectNew.y;
  }
  if ( vectNew.z > m_LowerRightCorner.z )
  {
    m_LowerRightCorner.z = vectNew.z;
  }
}



bool XBoundingBox::IntersectBoxWithRay( const math::vector3& ray_orig,  const math::vector3& ray_dir, 
                                        const math::vector3& box_min,   const math::vector3& box_max,
                                        math::vector3&       res_near,  eIntersectPlane& ipNear,
                                        math::vector3&       res_far,   eIntersectPlane& ipFar ) const
{ 
  float tnear = -FLT_MAX; 
  float tfar  =  FLT_MAX; 

  ipNear  = IP_NONE;
  ipFar   = IP_NONE;

  for ( int i = 0; i < 3; ++i ) 
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
        if ( t2 > tnear ) 
        {
          tnear = t2; 

          ipNear = (eIntersectPlane)( IP_XMAX + 2 * i );
        }
        if ( t1 < tfar ) 
        {
          tfar = t1; 

          ipFar = (eIntersectPlane)( IP_XMIN + 2 * i );
        }
      } 
      else 
      { 
        if ( t1 > tnear ) 
        {
          tnear = t1; 

          ipNear = (eIntersectPlane)( IP_XMIN + 2 * i );
        }
        if ( t2 < tfar ) 
        {
          tfar = t2; 

          ipFar = (eIntersectPlane)( IP_XMAX + 2 * i );
        }
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



bool XBoundingBox::IntersectWithRay( const math::vector3& v1, const math::vector3& v2, 
                                     math::vector3& vResultNear, eIntersectPlane& ipNear,
                                     math::vector3& vResultFar, eIntersectPlane& ipFar ) const
{
  math::vector3   resultNear,
                  resultFar;



  bool    result = IntersectBoxWithRay( v1, v2 - v1,
                                         m_UpperLeftCorner,
                                         m_LowerRightCorner, 
                                         resultNear, ipNear,
                                         resultFar, ipFar );

  vResultNear = resultNear;
  vResultFar = resultFar;

  return result;
}	



bool XBoundingBox::IntersectWithRay( const math::vector3& v1, const math::vector3& v2, 
                                     math::vector3& vResultNear, math::vector3& vResultFar ) const
{
  math::vector3   resultNear,
                  resultFar;

  XBoundingBox::eIntersectPlane     ipNear,
                                    ipFar;

  bool    result = IntersectBoxWithRay( v1, v2 - v1, 
                                         m_UpperLeftCorner,
                                         m_LowerRightCorner, 
                                         resultNear, ipNear,
                                         resultFar, ipFar );

  vResultNear = resultNear;
  vResultFar = resultFar;

  return result;
}	



void XBoundingBox::Combine( const XBoundingBox& BoundingBox )
{
  if ( BoundingBox.m_Transformed )
  {
    for ( size_t i = 0; i < 8; ++i )
    {
      AddVertex( BoundingBox.TransformedCorner( i ) );
    }
  }
  else
  {
    AddVertex( BoundingBox.UpperLeftCorner() );
    AddVertex( BoundingBox.LowerRightCorner() );
  }
}



float XBoundingBox::Width() const
{
  return m_LowerRightCorner.x - m_UpperLeftCorner.x;
}



float XBoundingBox::Height() const
{
  return m_LowerRightCorner.y - m_UpperLeftCorner.y;
}



float XBoundingBox::Depth() const
{
  return m_LowerRightCorner.z - m_UpperLeftCorner.z;
}



void XBoundingBox::Transform( const math::matrix4& matTransform )
{
  math::vector3&  vectLO = m_UpperLeftCorner,
                  vectRU = m_LowerRightCorner;

  math::vector3   vect1( vectLO.x, 
                         vectLO.y, 
                         vectLO.z ),
                  vect2( vectLO.x, 
                         vectLO.y, 
                         vectRU.z ),
                  vect3( vectLO.x, 
                         vectRU.y, 
                         vectRU.z ),
                  vect4( vectLO.x, 
                         vectRU.y, 
                         vectLO.z ),
                  vect5( vectRU.x, 
                         vectLO.y, 
                         vectLO.z ),
                  vect6( vectRU.x, 
                         vectLO.y, 
                         vectRU.z ),
                  vect7( vectRU.x, 
                         vectRU.y, 
                         vectRU.z ),
                  vect8( vectRU.x, 
                         vectRU.y, 
                         vectLO.z );

  matTransform.TransformCoord( vect1, m_RotatedBoundingBox[0] );
  matTransform.TransformCoord( vect2, m_RotatedBoundingBox[1] );
  matTransform.TransformCoord( vect3, m_RotatedBoundingBox[2] );
  matTransform.TransformCoord( vect4, m_RotatedBoundingBox[3] );
  matTransform.TransformCoord( vect5, m_RotatedBoundingBox[4] );
  matTransform.TransformCoord( vect6, m_RotatedBoundingBox[5] );
  matTransform.TransformCoord( vect7, m_RotatedBoundingBox[6] );
  matTransform.TransformCoord( vect8, m_RotatedBoundingBox[7] );

  m_Transformed = true;
}



math::vector3 XBoundingBox::TransformedCorner( size_t CornerIndex ) const
{
  if ( CornerIndex >= 8 )
  {
    return math::vector3();
  }
  return m_RotatedBoundingBox[CornerIndex];
}



math::vector3 XBoundingBox::UpperLeftCorner() const
{
  return m_UpperLeftCorner;
}



math::vector3 XBoundingBox::LowerRightCorner() const
{
  return m_LowerRightCorner;
}



bool XBoundingBox::Intersect( const XBoundingBox& BoundingBox ) const
{

  if ( ( m_UpperLeftCorner.x <= BoundingBox.m_LowerRightCorner.x )
  &&   ( m_LowerRightCorner.x >= BoundingBox.m_UpperLeftCorner.x )
  &&   ( m_UpperLeftCorner.y <= BoundingBox.m_LowerRightCorner.y )
  &&   ( m_LowerRightCorner.y >= BoundingBox.m_UpperLeftCorner.y )
  &&   ( m_UpperLeftCorner.z <= BoundingBox.m_LowerRightCorner.z )
  &&   ( m_LowerRightCorner.z >= BoundingBox.m_UpperLeftCorner.z ) )
  {
    return true;
  }
  return false;

}



GR::tVector XBoundingBox::Size() const
{
  return GR::tVector( Width(), Height(), Depth() );
}



void XBoundingBox::Offset( const GR::tVector& vectOffset )
{
  m_LowerRightCorner += vectOffset;
  m_UpperLeftCorner  += vectOffset;
}



bool XBoundingBox::Contains( const GR::tVector& vectPos ) const
{
  if ( ( vectPos.x >= m_UpperLeftCorner.x )
  &&   ( vectPos.x < m_LowerRightCorner.x )
  &&   ( vectPos.y >= m_UpperLeftCorner.y )
  &&   ( vectPos.y < m_LowerRightCorner.y )
  &&   ( vectPos.z >= m_UpperLeftCorner.z )
  &&   ( vectPos.z < m_LowerRightCorner.z ) )
  {
    return true;
  }
  return false;
}



GR::tVector XBoundingBox::Center() const
{
  return ( m_UpperLeftCorner + m_LowerRightCorner ) * 0.5f;
}



void XBoundingBox::Inflate( const GR::f32 fDX, const GR::f32 fDY, const GR::f32 fDZ )
{
  m_UpperLeftCorner.x -= fDX * 0.5f;
  m_UpperLeftCorner.y -= fDY * 0.5f;
  m_UpperLeftCorner.z -= fDZ * 0.5f;

  m_LowerRightCorner.x += fDX * 0.5f;
  m_LowerRightCorner.y += fDY * 0.5f;
  m_LowerRightCorner.z += fDZ * 0.5f;
}



void XBoundingBox::Set( const GR::f32 X, const GR::f32 Y, const GR::f32 Z, const GR::f32 Width, const GR::f32 Height, const GR::f32 Depth )
{
  m_UpperLeftCorner.set( X, Y, Z );
  m_LowerRightCorner.set( X + Width, Y + Height, Z + Depth );
}



void XBoundingBox::Set( const GR::tBounds& Bounds )
{
  m_UpperLeftCorner.set( (GR::f32)Bounds.Position.x, (GR::f32)Bounds.Position.y, (GR::f32)Bounds.Position.z );
  m_LowerRightCorner = m_UpperLeftCorner + GR::tVector( (GR::f32)Bounds.Size.x, (GR::f32)Bounds.Size.y, (GR::f32)Bounds.Size.z );
}