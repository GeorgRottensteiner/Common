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

#include <D3D8.h>
#include <D3DX8.h>

#include <DX8\DX8Camera.h>

#include <Misc/Misc.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Camera::CDX8Camera()
{

  m_bLerping        = false;
  m_fLerpFaktor     = 0.3f;

  m_bForceDistance  = false;
  m_fForcedDistance = 10.0f;

  m_bForceYPos      = false;
  m_fForcedYPos     = 0.0f;

  m_vectPosition.x = 0;
  m_vectPosition.y = 0;
  m_vectPosition.z = 0;

  m_vectLookAt.x = 0;
  m_vectLookAt.y = 1.0f;
  m_vectLookAt.z = 0;

  m_vectLookUp.x = 0;
  m_vectLookUp.y = 1.0;
  m_vectLookUp.z = 0;

  m_fScaleX       = 1.0f;
  m_fScaleY       = 1.0f;
  m_fScaleZ       = 1.0f;

  m_fViewZNear    = 1.0f;
  m_fViewZFar     = 100.0f;

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  SetScaling( 1.0f );

  m_fFOV          = D3DX_PI / 4.0f;

  m_Viewport.Width    = 640;
  m_Viewport.Height   = 480;
  m_Viewport.MinZ     = 0.0f;
  m_Viewport.MaxZ     = 1.0f;
  m_Viewport.X        = 0;
  m_Viewport.Y        = 0;
      
  SetProjectionType( CDX8Camera::PerspectiveFOVLH );

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Camera::~CDX8Camera()
{

}



/*-UpdateMatrix---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::UpdateMatrix()
{

  D3DXMatrixLookAtLH( &m_matOrientation, &m_vectPosition, &m_vectLookAt, &m_vectLookUp );

  m_viewMatrix = m_scaleMatrix * m_matOrientation;

}



/*-SetScaling-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetScaling( float fX, float fY, float fZ )
{

  m_fScaleX = fX;
  m_fScaleY = fY;
  m_fScaleZ = fZ;
  D3DXMatrixScaling( &m_scaleMatrix, m_fScaleX, m_fScaleY, m_fScaleZ );

  UpdateMatrix();

}



/*-SetScalingX----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetScalingX( float fX )
{

  m_fScaleX = fX;
  D3DXMatrixScaling( &m_scaleMatrix, m_fScaleX, m_fScaleY, m_fScaleZ );

  UpdateMatrix();

}



/*-SetScalingY----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetScalingY( float fY )
{

  m_fScaleY = fY;
  D3DXMatrixScaling( &m_scaleMatrix, m_fScaleX, m_fScaleY, m_fScaleZ );

  UpdateMatrix();

}



/*-SetScalingZ----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetScalingZ( float fZ )
{

  m_fScaleZ = fZ;
  D3DXMatrixScaling( &m_scaleMatrix, m_fScaleX, m_fScaleY, m_fScaleZ );

  UpdateMatrix();

}



/*-SetScaling-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetScaling( float fFaktor )
{

  SetScaling( fFaktor, fFaktor, fFaktor );

}



/*-NormalizeAngle-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::NormalizeAngle( float& fAngle )
{

  while ( fAngle < -180.0f )
  {
    fAngle += 360.0f;
  }
  while ( fAngle >= 180.0f )
  {
    fAngle -= 360.0f;
  }

}



/*-RetrieveYPR----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

/*
void CDX8Camera::RetrieveYPR()
{

  float   C,
          atrx,
          atry;


  m_fYaw = (float)asin( m_viewMatrix._13 );        // Calculate Y-axis angle
  C           =  (float)cos( m_fYaw );
  m_fYaw    *= -D3DX_PI / 180.0f;

  if ( fabs( C ) > 0.005 )             // Gimball lock?
  {
    atrx      =  m_viewMatrix._33 / C;           // No, so get X-axis angle
    atry      = -m_viewMatrix._23  / C;

    m_fPitch  = -(float)atan2( atry, atrx ) * 180.0f / D3DX_PI;

    atrx      =  m_viewMatrix._11 / C;           // Get Z-axis angle
    atry      = -m_viewMatrix._12 / C;

    m_fRoll  = -(float)atan2( atry, atrx ) * 180.0f / D3DX_PI;
  }
  else                                 //  Gimball lock has occurred
  {
    m_fPitch  = 0;                      // Set X-axis angle to zero

    atrx      = m_viewMatrix._22;                 //  And calculate Z-axis angle
    atry      = m_viewMatrix._21;

    m_fRoll  = -(float)atan2( atry, atrx ) * 180.0f / D3DX_PI;
  }

  NormalizeAngle( m_fPitch );
  NormalizeAngle( m_fYaw );
  NormalizeAngle( m_fRoll );

  OutputDebugString( CMisc::printf( "Pitch %d Yaw %d Roll %d\n", 
                  (int)( m_fPitch * 1000 ),
                  (int)( m_fYaw * 1000 ),
                  (int)( m_fRoll * 1000 ) ) );
}
*/



/*-Pitch----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::Pitch( float fDelta )
{

  {
    D3DXVECTOR3 right;

    D3DXVec3Cross( &right, &m_vectLookUp, &m_vectDirection );

    D3DXMATRIX    rotMat;


    D3DXMatrixRotationAxis( &rotMat, &right, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;

    UpdateMatrix();
  }

}



/*-Yaw------------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::Yaw( float fDelta )
{

  {
    D3DXVECTOR3 rotVect( m_vectLookUp );


    D3DXMATRIX    rotMat;


    D3DXMatrixRotationAxis( &rotMat, &rotVect, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-Roll-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::Roll( float fDelta )
{

  {
    D3DXVECTOR3 rotVect( m_vectDirection );

    D3DXMATRIX    rotMat;


    D3DXMatrixRotationAxis( &rotMat, &rotVect, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-Dump-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::Dump()
{

  OutputDebugString( CMisc::printf( "m_vectDirection %d/%d/%d\n", 
            (int)( m_vectDirection.x * 1000 ),
            (int)( m_vectDirection.y * 1000 ),
            (int)( m_vectDirection.z * 1000 ) ) );
  OutputDebugString( CMisc::printf( "m_vectLookUp %d/%d/%d\n", 
            (int)( m_vectLookUp.x * 1000 ),
            (int)( m_vectLookUp.y * 1000 ),
            (int)( m_vectLookUp.z * 1000 ) ) );

}



/*-SetX-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetX( const float fX )
{

  m_vectPosition.x = fX;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-SetY-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetY( const float fY )
{

  m_vectPosition.y = fY;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-SetZ-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetZ( const float fZ )
{

  m_vectPosition.z = fZ;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-SetPosition----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetPosition( const D3DXVECTOR3& vectPos )
{

  m_vectPosition = vectPos;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}

void CDX8Camera::SetPosition( const float fX, const float fY, const float fZ )
{

  SetPosition( D3DXVECTOR3( fX, fY, fZ ) );

}



/*-GetPosition----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetPosition() const
{

  return m_vectPosition;

}



/*-GetDirection---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetDirection() const
{

  return m_vectDirection;

}



/*-GetLookUp------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetLookUp() const
{

  return m_vectLookUp;

}



/*-GetLookAt------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetLookAt() const
{

  return m_vectLookAt;

}



/*-GetCurrentPosition---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetCurrentPosition() const
{

  return m_vectCurrentPosition;

}



/*-GetCurrentDirection--------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetCurrentDirection() const
{

  return m_vectCurrentDirection;

}



/*-GetCurrentLookUp-----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 CDX8Camera::GetCurrentLookUp() const
{

  return m_vectCurrentLookUp;

}



/*-MovePosition---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::MovePosition( const D3DXVECTOR3& vectPos )
{

  m_vectPosition += vectPos;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-SetDirection---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetDirection( const D3DXVECTOR3& vectPos )
{

  float   fLength = D3DXVec3Length( &m_vectDirection );
  D3DXVec3Normalize( &m_vectDirection, &vectPos );

  m_vectLookAt = m_vectPosition + m_vectDirection * fLength;

  UpdateMatrix();

}



/*-SetLookAtPoint-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetLookAtPoint( const D3DXVECTOR3& vectPos )
{

  m_vectLookAt = vectPos;

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  UpdateMatrix();

}

void CDX8Camera::SetLookAtPoint( float fX, float fY, float fZ )
{
  
  SetLookAtPoint( D3DXVECTOR3( fX, fY, fZ ) );

}



/*-SetLookAtAndDirection------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetLookAtAndDirection( const D3DXVECTOR3& vectAt, const D3DXVECTOR3& vectDir )
{

  float   fLength = D3DXVec3Length( &m_vectDirection );
  D3DXVec3Normalize( &m_vectDirection, &vectDir );

  m_vectLookAt = vectAt;

  m_vectPosition = m_vectLookAt - m_vectDirection * fLength;

  UpdateMatrix();

}



/*-SetValues------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetValues( const D3DXVECTOR3& vectPos, const D3DXVECTOR3& vectLookAt, const D3DXVECTOR3& vectLookUp )
{

  m_vectPosition  = vectPos;
  m_vectLookAt    = vectLookAt;

  D3DXVec3Normalize( &m_vectLookUp, &vectLookUp );

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );

  UpdateMatrix();

}



/*-SetLookUp------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetLookUp( const D3DXVECTOR3& vectPos )
{

  m_vectLookUp = vectPos;

  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  UpdateMatrix();

}

void CDX8Camera::SetLookUp( float fX, float fY, float fZ )
{
  
  SetLookUp( D3DXVECTOR3( fX, fY, fZ ) );

}



/*-GetViewMatrix--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX CDX8Camera::GetViewMatrix()
{

  return m_viewMatrix; 

}




/*-GetProjectionMatrix--------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX CDX8Camera::GetProjectionMatrix()
{

  return m_projMatrix; 

}



/*-GetViewport----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DVIEWPORT8* CDX8Camera::GetViewport()
{

  return &m_Viewport;

}



/*-GetProjectionType----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Camera::ProjectionType CDX8Camera::GetProjectionType()
{

  return m_projType;
  
}



/*-SetProjectionType----------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::SetProjectionType( CDX8Camera::ProjectionType dwProjectionType )
{

  m_projType = dwProjectionType;

  switch ( m_projType )
  {
    case CDX8Camera::PerspectiveFOVLH:
      D3DXMatrixPerspectiveFovLH( &m_projMatrix,
                                  m_fFOV,
                                  (float)m_Viewport.Width / (float)m_Viewport.Height,
                                  m_fViewZNear,
                                  m_fViewZFar );
      break;
    case CDX8Camera::PerspectiveFOVRH:
      D3DXMatrixPerspectiveFovRH( &m_projMatrix,
                                  m_fFOV,
                                  (float)m_Viewport.Width / (float)m_Viewport.Height,
                                  m_fViewZNear,
                                  m_fViewZFar );
      break;
    case CDX8Camera::OrthoLH:
      D3DXMatrixOrthoLH( &m_projMatrix,
                         m_fViewWidth,
                         m_fViewHeight,
                         m_fViewZNear,
                         m_fViewZFar );
      break;
    case CDX8Camera::OrthoRH:
      D3DXMatrixOrthoRH( &m_projMatrix,
                         m_fViewWidth,
                         m_fViewHeight,
                         m_fViewZNear,
                         m_fViewZFar );
      break;
  }

}



/*-UpdateLerp-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <debug\debugclient.h>
D3DXMATRIX CDX8Camera::UpdateLerp( float fZeitFaktor )
{

  D3DXMATRIX    matCurrent;

  float   fFaktor = 1.0f - powf( m_fLerpFaktor, fZeitFaktor );

  D3DXVec3Lerp( &m_vectCurrentPosition,   &m_vectCurrentPosition,   &m_vectPosition,  fFaktor );
  D3DXVec3Lerp( &m_vectCurrentDirection,  &m_vectCurrentDirection,  &m_vectDirection, fFaktor );
  D3DXVec3Lerp( &m_vectCurrentLookUp,     &m_vectCurrentLookUp,     &m_vectLookUp,    fFaktor );

  if ( m_bForceDistance )
  {
    //m_vectCurrentDirection = m_vectDirection;
    D3DXVec3Normalize( &m_vectCurrentDirection, &m_vectCurrentDirection );
    m_vectCurrentPosition = m_vectLookAt - m_vectCurrentDirection * m_fForcedDistance;

    m_vectCurrentDirection = m_vectLookAt - m_vectCurrentPosition;

    D3DXVec3Normalize( &m_vectCurrentDirection, &m_vectCurrentDirection );

    m_vectCurrentLookUp = m_vectLookUp;

    if ( m_bForceYPos )
    {
      m_vectCurrentPosition.y = m_fForcedYPos;

      m_vectCurrentDirection = m_vectLookAt - m_vectCurrentPosition;
      D3DXVec3Normalize( &m_vectCurrentDirection, &m_vectCurrentDirection );
    }
  }

  D3DXMatrixLookAtLH( &matCurrent,
                      &m_vectCurrentPosition,
                      //&( m_vectCurrentPosition + m_vectCurrentDirection ),
                      &m_vectLookAt,
                      &m_vectCurrentLookUp );

  return matCurrent;

}



/*-ForceDistance--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::ForceDistance( bool bForce, float fDistance )
{

  m_bForceDistance = bForce;
  m_fForcedDistance = fDistance;

}



/*-ForceYPos------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CDX8Camera::ForceYPos( bool bForce, float fPos )
{

  m_bForceYPos      = bForce;
  m_fForcedYPos     = fPos;

}