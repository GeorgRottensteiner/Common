// Frustum.cpp: implementation of the CFrustum class.
//
//////////////////////////////////////////////////////////////////////

#include "Frustum.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFrustum::CFrustum()
{

}

CFrustum::~CFrustum()
{

}



/*-Create---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CFrustum::Create( D3DXMATRIX& matProjection, D3DXMATRIX& matView, D3DVIEWPORT8& viewPort, 
                       float fExtraBorder )
{

  m_vectCorners[C_LO_NEAR]  = D3DXVECTOR3( -1.0f - fExtraBorder,  1.0f + fExtraBorder, viewPort.MinZ );
  m_vectCorners[C_LO_FAR]   = D3DXVECTOR3( -1.0f - fExtraBorder,  1.0f + fExtraBorder, viewPort.MaxZ );
  m_vectCorners[C_RO_NEAR]  = D3DXVECTOR3(  1.0f + fExtraBorder,  1.0f + fExtraBorder, viewPort.MinZ );
  m_vectCorners[C_RO_FAR]   = D3DXVECTOR3(  1.0f + fExtraBorder,  1.0f + fExtraBorder, viewPort.MaxZ );
  m_vectCorners[C_LU_NEAR]  = D3DXVECTOR3( -1.0f - fExtraBorder, -1.0f - fExtraBorder, viewPort.MinZ );
  m_vectCorners[C_LU_FAR]   = D3DXVECTOR3( -1.0f - fExtraBorder, -1.0f - fExtraBorder, viewPort.MaxZ );
  m_vectCorners[C_RU_NEAR]  = D3DXVECTOR3(  1.0f + fExtraBorder, -1.0f - fExtraBorder, viewPort.MinZ );
  m_vectCorners[C_RU_FAR]   = D3DXVECTOR3(  1.0f + fExtraBorder, -1.0f - fExtraBorder, viewPort.MaxZ );

  D3DXMATRIX    matProjInv,
                matViewInv;

  for ( int i = 0; i < 16; ++i )
  {
    dh::Log( "matProjection %f", matProjection.m[i / 4][i % 4] );
  }
  for ( int i = 0; i < 16; ++i )
  {
    dh::Log( "matView %f", matView.m[i / 4][i % 4] );
  }

  D3DXMatrixInverse( &matProjInv, NULL, &matProjection );
  D3DXMatrixInverse( &matViewInv, NULL, &matView );

  for ( int i = 0; i < 16; ++i )
  {
    dh::Log( "matProjInv %f", matProjInv.m[i / 4][i % 4] );
  }
  for ( int i = 0; i < 16; ++i )
  {
    dh::Log( "matViewInv %f", matViewInv.m[i / 4][i % 4] );
  }

  matProjInv *= matViewInv;

  for ( int i = 0; i < 16; ++i )
  {
    dh::Log( "matProjInv %f", matProjInv.m[i / 4][i % 4] );
  }

  for ( int i = 0; i < C_CORNER_COUNT; ++i )
  {
    D3DXVec3TransformCoord( &m_vectCorners[i], &m_vectCorners[i], &matProjInv );
  }

  D3DXPlaneFromPoints( &planeLeft,    &m_vectCorners[C_LO_NEAR],  &m_vectCorners[C_LO_FAR],   &m_vectCorners[C_LU_NEAR] );
  D3DXPlaneFromPoints( &planeRight,   &m_vectCorners[C_RO_NEAR],  &m_vectCorners[C_RU_NEAR],  &m_vectCorners[C_RO_FAR] );
  D3DXPlaneFromPoints( &planeTop,     &m_vectCorners[C_LO_FAR],   &m_vectCorners[C_LO_NEAR],  &m_vectCorners[C_RO_NEAR] );
  D3DXPlaneFromPoints( &planeBottom,  &m_vectCorners[C_RU_NEAR],  &m_vectCorners[C_LU_NEAR],  &m_vectCorners[C_LU_FAR] );

  D3DXPlaneFromPoints( &planeZNear,   &m_vectCorners[C_RO_NEAR],  &m_vectCorners[C_LO_NEAR],  &m_vectCorners[C_LU_NEAR] );
  D3DXPlaneFromPoints( &planeZFar,    &m_vectCorners[C_LO_FAR],   &m_vectCorners[C_RO_FAR],   &m_vectCorners[C_LU_FAR] );

  /*
  theApp.DrawFreeLine( vectLONear, vectRONear, 0xffff00ff );
  theApp.DrawFreeLine( vectRONear, vectRUNear, 0xffff00ff );
  theApp.DrawFreeLine( vectRUNear, vectLUNear, 0xffff00ff );
  theApp.DrawFreeLine( vectLUNear, vectLONear, 0xffff00ff );

  theApp.DrawFreeLine( vectLONear, vectLOFar, 0xffff00ff );
  theApp.DrawFreeLine( vectRONear, vectROFar, 0xffff00ff );
  theApp.DrawFreeLine( vectRUNear, vectRUFar, 0xffff00ff );
  theApp.DrawFreeLine( vectLUNear, vectLUFar, 0xffff00ff );
  */

}



/*-IntersectWithBoundingBoxRotated--------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CFrustum::IntersectWithBoundingBoxRotated( const CBoundingBox& BBox ) const
{

  return IntersectWithRotatedBox( (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[0],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[1],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[2],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[3],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[4],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[5],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[6],
                                  (D3DXVECTOR3&)BBox.m_RotatedBoundingBox[7] );

}



/*-IntersectWithBoundingBox---------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CFrustum::IntersectWithBoundingBox( const CBoundingBox& BBox ) const
{

  if ( BBox.m_bTransformed )
  {
    return IntersectWithBoundingBoxRotated( BBox );
  }
  return IntersectWithBoundingBox( (D3DXVECTOR3&)BBox.UpperLeftCorner(), (D3DXVECTOR3&)BBox.LowerRightCorner() );

}

bool CFrustum::IntersectWithBoundingBox( D3DXVECTOR3& vect1, D3DXVECTOR3& vect2 ) const
{

  D3DXVECTOR3   v1( vect1.x, vect2.y, vect1.z ),
                v2( vect1.x, vect2.y, vect2.z ),
                v3( vect2.x, vect2.y, vect1.z ),
                v4( vect2.x, vect2.y, vect2.z ),
                v5( vect1.x, vect1.y, vect1.z ),
                v6( vect1.x, vect1.y, vect2.z ),
                v7( vect2.x, vect1.y, vect1.z ),
                v8( vect2.x, vect1.y, vect2.z );

  /*
  D3DXMATRIX    matWorld;

  D3DXMatrixIdentity( &matWorld );
  theApp.SetTransform( D3DTS_WORLD, &matWorld );
    
  theApp.DrawFreeLine( v1, v3, 0xffff00ff );
  theApp.DrawFreeLine( v3, v7, 0xffff00ff );
  theApp.DrawFreeLine( v7, v5, 0xffff00ff );
  theApp.DrawFreeLine( v5, v1, 0xffff00ff );

  theApp.DrawFreeLine( v1, v2, 0xffff00ff );
  theApp.DrawFreeLine( v3, v4, 0xffff00ff );
  theApp.DrawFreeLine( v7, v8, 0xffff00ff );
  theApp.DrawFreeLine( v5, v6, 0xffff00ff );
  */

  return IntersectWithRotatedBox( v1, v2, v3, v4, v5, v6, v7, v8 );

}



/*-IntersectWithRotatedBox----------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

bool CFrustum::IntersectWithRotatedBox( D3DXVECTOR3& vect1, D3DXVECTOR3& vect2,
                                        D3DXVECTOR3& vect3, D3DXVECTOR3& vect4,
                                        D3DXVECTOR3& vect5, D3DXVECTOR3& vect6,
                                        D3DXVECTOR3& vect7, D3DXVECTOR3& vect8 ) const
{

  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect1 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect1 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect1 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect1 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect1 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect1 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect2 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect2 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect2 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect2 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect2 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect2 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect3 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect3 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect3 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect3 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect3 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect3 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect4 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect4 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect4 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect4 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect4 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect4 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect5 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect5 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect5 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect5 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect5 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect5 ) > 0.0f ) )
  {
    return true;
  }
  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect6 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeTop,    &vect6 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeRight,  &vect6 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeBottom, &vect6 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZNear,  &vect6 ) > 0.0f )
  &&   ( D3DXPlaneDotCoord( &planeZFar,   &vect6 ) > 0.0f ) )
  {
    return true;
  }

  if ( ( D3DXPlaneDotCoord( &planeLeft,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeLeft,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der left Plane
    return false;
  }

  if ( ( D3DXPlaneDotCoord( &planeRight,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeRight,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( D3DXPlaneDotCoord( &planeZNear,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZNear,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( D3DXPlaneDotCoord( &planeZFar,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeZFar,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( D3DXPlaneDotCoord( &planeTop,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeTop,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  if ( ( D3DXPlaneDotCoord( &planeBottom,   &vect1 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect2 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect3 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect4 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect5 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect6 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect7 ) > 0.0f )
  ||   ( D3DXPlaneDotCoord( &planeBottom,   &vect8 ) > 0.0f ) )
  {
  }
  else
  {
    // alle 8 Punkte liegen hinter der right Plane
    return false;
  }

  return true;

}