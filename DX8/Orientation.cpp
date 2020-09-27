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

#include <DX8\Orientation.h>

#include <Misc/Misc.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Constructor----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

COrientation::COrientation()
{

  m_vectPosition.x = 0;
  m_vectPosition.y = 0;
  m_vectPosition.z = 0;

  m_vectLookAt.x = 0;
  m_vectLookAt.y = 0;
  m_vectLookAt.z = 1.0f;

  m_vectLookUp.x = 0;
  m_vectLookUp.y = 1.0;
  m_vectLookUp.z = 0;

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  UpdateMatrix();

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

COrientation::~COrientation()
{

}



/*-UpdateMatrix---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::UpdateMatrix()
{

  D3DXMatrixLookAtLH( &m_matOrientation, &m_vectPosition, &m_vectLookAt, &m_vectLookUp );

}



/*-Pitch----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::Pitch( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 1.0f, 0.0f, 0.0f );

    D3DXMATRIX    rotMat;


    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

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

void COrientation::Yaw( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 0.0f, 1.0f, 0.0f );


    D3DXMATRIX    rotMat;


    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

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

void COrientation::Roll( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 0.0f, 0.0f, 1.0f );

    D3DXMATRIX    rotMat;


    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-PitchAbsolute--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::PitchAbsolute( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 1.0f, 0.0f, 0.0f );

    D3DXMATRIX    rotMat;

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXVec3Cross( &vectRot, &m_vectDirection, &m_vectLookUp );

    D3DXVec3Normalize( &vectRot, &vectRot );

    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;

    UpdateMatrix();
  }

}



/*-YawAbsolute----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::YawAbsolute( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 0.0f, 1.0f, 0.0f );


    D3DXMATRIX    rotMat;


    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-RollAbsolute---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::RollAbsolute( float fDelta )
{

  {
    D3DXVECTOR3 vectRot( 0.0f, 0.0f, 1.0f );

    D3DXMATRIX    rotMat;


    D3DXMatrixRotationAxis( &rotMat, &vectRot, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-RotateAboutAxis------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::RotateAboutAxis( D3DXVECTOR3& vectAxis, float fDelta )
{

  {

    D3DXMATRIX    rotMat;


    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    D3DXMatrixRotationAxis( &rotMat, &vectAxis, fDelta * D3DX_PI / 180.0f );

    D3DXVec3TransformCoord( &m_vectLookUp, &m_vectLookUp, &rotMat );
    D3DXVec3TransformCoord( &m_vectDirection, &m_vectDirection, &rotMat );

    D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
    D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

    m_vectLookAt = m_vectPosition + m_vectDirection;
    UpdateMatrix();
  }


}



/*-ToEulerAngles--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::ToEulerAngles( float& fYaw, float& fPitch, float& fRoll )
{

	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	fPitch = asinf( m_matOrientation._31 );
	if ( fPitch < D3DX_PI / 2 )
	{
		if ( fPitch > -D3DX_PI / 2 )
		{
			fYaw = atan2f( -m_matOrientation._32, m_matOrientation._33 );
			fRoll = atan2f( -m_matOrientation._21, m_matOrientation._11 );
		}
		else
		{
			// WARNING.  Not a unique solution.
			float fRmY = atan2f( m_matOrientation._12, m_matOrientation._22 );
			fRoll = 0.0;  // any angle works
			fYaw = fRoll - fRmY;
		}
	}
	else
	{
		// WARNING.  Not a unique solution.
		float fRpY = atan2f( m_matOrientation._12, m_matOrientation._22 );//Math::ATan2(m[1][0],m[1][1]);
		fRoll = 0.0;  // any angle works
		fYaw = fRpY - fRoll;
	}

  fYaw    = D3DXToDegree( fYaw );
  fPitch  = D3DXToDegree( fPitch );
  fRoll   = D3DXToDegree( fRoll );
  
}



/*-Dump-----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::Dump()
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

void COrientation::SetX( const float& fX )
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

void COrientation::SetY( const float& fY )
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

void COrientation::SetZ( const float& fZ )
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

void COrientation::SetPosition( const D3DXVECTOR3& vectPos )
{

  m_vectPosition = vectPos;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



void COrientation::SetPosition( const float fX, const float fY, const float fZ )
{

  SetPosition( D3DXVECTOR3( fX, fY, fZ ) );

}



/*-GetPosition----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::GetPosition() const
{

  return m_vectPosition;

}



/*-GetPositionX---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float COrientation::GetPositionX() const
{

  return m_vectPosition.x;

}



/*-GetPositionY---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float COrientation::GetPositionY() const
{

  return m_vectPosition.y;

}



/*-GetPositionZ---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

float COrientation::GetPositionZ() const
{

  return m_vectPosition.z;

}



/*-GetLookAtPoint-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::GetLookAtPoint() const
{

  return m_vectLookAt;

}



/*-GetDirection---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::GetDirection() const
{

  return m_vectDirection;

}



/*-GetLookUp------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::GetLookUp() const
{

  return m_vectLookUp;

}



/*-MovePosition---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::MovePosition( const D3DXVECTOR3& vectPos )
{

  m_vectPosition += vectPos;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



void COrientation::MovePosition( const float fDX, const float fDY, const float fDZ )
{

  MovePosition( D3DXVECTOR3( fDX, fDY, fDZ ) );

}



/*-SetLookAtPoint-------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::SetLookAtPoint( const D3DXVECTOR3& vectPos )
{

  m_vectLookAt = vectPos;

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}

void COrientation::SetLookAtPoint( const float fX, const float fY, const float fZ )
{

  SetLookAtPoint( D3DXVECTOR3( fX, fY, fZ ) );

}



/*-SetDirection---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::SetDirection( const D3DXVECTOR3& vectDir )
{

  m_vectDirection = vectDir;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-SetLookUp------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::SetLookUp( const D3DXVECTOR3& vectLUp )
{

  m_vectLookUp = vectLUp;

  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  UpdateMatrix();

}



/*-SlideToLookAtPoint---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::SlideToLookAtPoint( const D3DXVECTOR3& vectPos )
{

  D3DXVECTOR3   vectOldDir = m_vectLookAt - m_vectPosition;

  m_vectLookAt = vectPos;

  m_vectPosition = m_vectLookAt - vectOldDir;

  UpdateMatrix();

}



/*-GetViewMatrix--------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX COrientation::GetViewMatrix() const
{

  return m_matOrientation; 

}



/*-GetViewMatrixRH------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX COrientation::GetViewMatrixRH() const
{

  D3DXMATRIX    matViewRH;

  D3DXMatrixLookAtLH( &matViewRH, &m_vectPosition, &m_vectLookAt, &m_vectLookUp );

  return matViewRH; 

}



/*-GetMatrix------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX COrientation::GetMatrix() const
{

  D3DXMATRIX    mat;

  D3DXMatrixLookAtLH( &mat, &m_vectPosition, &( m_vectPosition + m_vectDirection ), &m_vectLookUp );

  D3DXMatrixInverse( &mat, NULL, &mat );

  return mat;

}



/*-GetMatrixRH----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXMATRIX COrientation::GetMatrixRH() const
{

  D3DXMATRIX    mat;

  D3DXVECTOR3   vectNull( 0, 0, 0 );

  D3DXMatrixLookAtRH( &mat, &m_vectPosition, &( m_vectPosition + m_vectDirection ), &m_vectLookUp );

  D3DXMatrixInverse( &mat, NULL, &mat );
  
  return mat;

}



/*-Advance--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::Advance( float fLength )
{

  m_vectPosition += m_vectDirection * fLength;

  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-Strafe---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::Strafe( float fLength )
{

  D3DXVECTOR3   vectStrafe;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  D3DXVec3Cross( &vectStrafe, &m_vectLookUp, &m_vectDirection );

  D3DXVec3Normalize( &vectStrafe, &vectStrafe );

  m_vectPosition += vectStrafe * fLength;
  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-Raise----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::Raise( float fLength )
{

  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  m_vectPosition += m_vectLookUp * fLength;
  m_vectLookAt = m_vectPosition + m_vectDirection;

  UpdateMatrix();

}



/*-Advance--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::CalcAdvanceVector( float fLength )
{

  return m_vectDirection * fLength;

}



/*-Strafe---------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::CalcStrafeVector( float fLength )
{

  D3DXVECTOR3   vectStrafe;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  D3DXVec3Cross( &vectStrafe, &m_vectLookUp, &m_vectDirection );

  D3DXVec3Normalize( &vectStrafe, &vectStrafe );

  return vectStrafe * fLength;

}



/*-Raise----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

D3DXVECTOR3 COrientation::CalcRaiseVector( float fLength )
{

  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  return m_vectLookUp * fLength;

}



/*-SetValues------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void COrientation::SetValues( const D3DXVECTOR3& vectPos, const D3DXVECTOR3& vectLookAt, const D3DXVECTOR3& vectLookUp )
{

  m_vectPosition  = vectPos;
  m_vectLookAt    = vectLookAt;
  m_vectLookUp    = vectLookUp;

  m_vectDirection = m_vectLookAt - m_vectPosition;

  D3DXVec3Normalize( &m_vectDirection, &m_vectDirection );
  D3DXVec3Normalize( &m_vectLookUp, &m_vectLookUp );

  UpdateMatrix();

}
