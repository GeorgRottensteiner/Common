/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8\BoundingBox.h>

#include <Math\math_util.h>
#include <Math\vector3.h>



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CBoundingBox::CBoundingBox() :
  m_bTransformed( false )
{

  Clear();

}



/*-Clear----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBoundingBox::Clear()
{

  m_bTransformed = false;
  
  m_UpperLeftCorner   = math::vector3( FLT_MAX, 
                                       FLT_MAX, 
                                       FLT_MAX );
  m_LowerRightCorner  = math::vector3( -FLT_MAX, 
                                       -FLT_MAX, 
                                       -FLT_MAX );

}



/*-ErweitereUmVertex----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBoundingBox::ErweitereUmVertex( math::vector3& vectNew )
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



/*-IntersectWithRay-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CBoundingBox::IntersectWithRay( const math::vector3& v1, const math::vector3& v2, math::vector3& vResultNear, math::vector3& vResultFar ) const
{

  math::vector3   resultNear,
                  resultFar;

  bool    bResult = math::IntersectBoxWithRay<math::vector3>( v1, v2 - v1, 
                                    m_UpperLeftCorner,
                                    m_LowerRightCorner, resultNear, resultFar );

  vResultNear = resultNear;
  vResultFar = resultFar;

  return bResult;

}	



/*-Combine--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBoundingBox::Combine( const CBoundingBox& BoundingBox )
{

  if ( BoundingBox.m_bTransformed )
  {
    for ( size_t i = 0; i < 8; ++i )
    {
      ErweitereUmVertex( BoundingBox.TransformedCorner( i ) );
    }
  }
  else
  {
    ErweitereUmVertex( BoundingBox.UpperLeftCorner() );
    ErweitereUmVertex( BoundingBox.LowerRightCorner() );
  }

}



/*-Width----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float CBoundingBox::Width() const
{

  return m_LowerRightCorner.x - m_UpperLeftCorner.x;

}



/*-Height---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float CBoundingBox::Height() const
{

  return m_LowerRightCorner.y - m_UpperLeftCorner.y;

}



/*-Depth----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float CBoundingBox::Depth() const
{

  return m_LowerRightCorner.z - m_UpperLeftCorner.z;

}



/*-Transform------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CBoundingBox::Transform( math::matrix4& matTransform )
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

  m_bTransformed = true;

}



math::vector3 CBoundingBox::TransformedCorner( size_t iIndex ) const
{

  if ( iIndex >= 8 )
  {
    return math::vector3();
  }
  return m_RotatedBoundingBox[iIndex];

}



math::vector3 CBoundingBox::UpperLeftCorner() const
{

  return m_UpperLeftCorner;

}



math::vector3 CBoundingBox::LowerRightCorner() const
{

  return m_LowerRightCorner;

}
