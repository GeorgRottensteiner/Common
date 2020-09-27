#include <Misc/Misc.h>

#include "XOrientation.h"



XOrientation::XOrientation() :
  m_Scale( 1.0f, 1.0f, 1.0f ),
  m_Position(),
  m_LookAt( 0, 0, 1.0f ),
  m_LookUp( 0, 1.0f, 0 )
{

  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();
  m_LookUp.normalize();

  UpdateMatrix();

}



XOrientation::~XOrientation()
{
}



void XOrientation::UpdateMatrix()
{
  m_Orientation.LookAtLH( m_Position, m_LookAt, m_LookUp );
}



void XOrientation::Pitch( float fDelta )
{
  GR::tVector vectRot( 1.0f, 0.0f, 0.0f );

  math::matrix4    rotMat;


  m_Direction.normalize();
  m_LookUp.normalize();

  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XOrientation::Yaw( float fDelta )
{
  GR::tVector vectRot( 0.0f, 1.0f, 0.0f );


  math::matrix4    rotMat;


  m_Direction.normalize();
  m_LookUp.normalize();

  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XOrientation::Roll( float fDelta )
{
  GR::tVector vectRot( 0.0f, 0.0f, 1.0f );

  math::matrix4    rotMat;


  m_Direction.normalize();
  m_LookUp.normalize();

  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XOrientation::PitchAbsolute( float fDelta )
{
  GR::tVector vectRot( 1.0f, 0.0f, 0.0f );

  math::matrix4    rotMat;

  m_Direction.normalize();
  m_LookUp.normalize();

  vectRot = m_Direction.cross( m_LookUp );
  vectRot.normalize();

  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XOrientation::YawAbsolute( float fDelta )
{
  GR::tVector vectRot( 0.0f, 1.0f, 0.0f );


  math::matrix4    rotMat;


  m_Direction.normalize();
  m_LookUp.normalize();

  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XOrientation::RollAbsolute( float fDelta )
{
  GR::tVector vectRot( 0.0f, 0.0f, 1.0f );

  math::matrix4    rotMat;


  rotMat.RotateAboutAxis( vectRot, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XOrientation::RotateAboutAxis( GR::tVector& vectAxis, float fDelta )
{
  math::matrix4    rotMat;


  m_Direction.normalize();
  m_LookUp.normalize();

  rotMat.RotateAboutAxis( vectAxis, fDelta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XOrientation::ToEulerAngles( float& fYaw, float& fPitch, float& fRoll )
{

	// rot =  cy*cz          -cy*sz           sy
	//        cz*sx*sy+cx*sz  cx*cz-sx*sy*sz -cy*sx
	//       -cx*cz*sy+sx*sz  cz*sx+cx*sy*sz  cx*cy

	fPitch = asinf( m_Orientation.ms._31 );
	if ( fPitch < 3.1415926f / 2 )
	{
		if ( fPitch > -3.1415926f / 2 )
		{
			fYaw = atan2f( -m_Orientation.ms._32, m_Orientation.ms._33 );
			fRoll = atan2f( -m_Orientation.ms._21, m_Orientation.ms._11 );
		}
		else
		{
			// WARNING.  Not a unique solution.
			float fRmY = atan2f( m_Orientation.ms._12, m_Orientation.ms._22 );
			fRoll = 0.0;  // any angle works
			fYaw = fRoll - fRmY;
		}
	}
	else
	{
		// WARNING.  Not a unique solution.
		float fRpY = atan2f( m_Orientation.ms._12, m_Orientation.ms._22 );//Math::ATan2(m[1][0],m[1][1]);
		fRoll = 0.0;  // any angle works
		fYaw = fRpY - fRoll;
  }

  fYaw    = ( ( fYaw ) * ( 180.0f / 3.1415926f ) );
  fPitch  = ( ( fPitch ) * ( 180.0f / 3.1415926f ) );
  fRoll   = ( ( fRoll ) * ( 180.0f / 3.1415926f ) );
  
}



void XOrientation::Dump()
{

  /*
  OutputDebugString( CMisc::printf( "m_Direction %d/%d/%d\n", 
            (int)( m_Direction.x * 1000 ),
            (int)( m_Direction.y * 1000 ),
            (int)( m_Direction.z * 1000 ) ) );
  OutputDebugString( CMisc::printf( "m_LookUp %d/%d/%d\n", 
            (int)( m_LookUp.x * 1000 ),
            (int)( m_LookUp.y * 1000 ),
            (int)( m_LookUp.z * 1000 ) ) );
            */

}



void XOrientation::SetX( const float& fX )
{

  m_Position.x = fX;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::SetY( const float& fY )
{

  m_Position.y = fY;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::SetZ( const float& fZ )
{

  m_Position.z = fZ;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::SetPosition( const GR::tVector& vectPos )
{
  m_Position = vectPos;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XOrientation::SetPosition( const float fX, const float fY, const float fZ )
{
  SetPosition( GR::tVector( fX, fY, fZ ) );
}



GR::tVector XOrientation::GetPosition() const
{
  return m_Position;
}



float XOrientation::GetPositionX() const
{
  return m_Position.x;
}



float XOrientation::GetPositionY() const
{
  return m_Position.y;
}



float XOrientation::GetPositionZ() const
{
  return m_Position.z;
}



GR::tVector XOrientation::GetLookAtPoint() const
{
  return m_LookAt;
}



GR::tVector XOrientation::GetDirection() const
{
  return m_Direction;
}



GR::tVector XOrientation::GetLookUp() const
{

  return m_LookUp;

}



void XOrientation::MovePosition( const GR::tVector& vectPos )
{
  m_Position += vectPos;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XOrientation::MovePosition( const float fDX, const float fDY, const float fDZ )
{

  MovePosition( GR::tVector( fDX, fDY, fDZ ) );

}



void XOrientation::SetLookAtPoint( const GR::tVector& vectPos )
{

  m_LookAt = vectPos;

  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::SetLookAtPoint( const float fX, const float fY, const float fZ )
{
  SetLookAtPoint( GR::tVector( fX, fY, fZ ) );
}



void XOrientation::SetDirection( const GR::tVector& vectDir )
{
  m_Direction = vectDir;

  m_Direction.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XOrientation::SetLookUp( const GR::tVector& vectLUp )
{

  m_LookUp = vectLUp;

  m_LookUp.normalize();

  UpdateMatrix();

}



void XOrientation::SlideToLookAtPoint( const GR::tVector& vectPos )
{

  GR::tVector   vectOldDir = m_LookAt - m_Position;

  m_LookAt = vectPos;

  m_Position = m_LookAt - vectOldDir;

  UpdateMatrix();

}



math::matrix4 XOrientation::GetViewMatrix() const
{

  return m_Orientation * math::matrix4().Scaling( m_Scale.x, m_Scale.y, m_Scale.z ); 

}



math::matrix4 XOrientation::GetViewMatrixRH() const
{

  math::matrix4    matViewRH;

  matViewRH.LookAtRH( m_Position, m_LookAt, m_LookUp );

  return matViewRH * math::matrix4().Scaling( m_Scale.x, m_Scale.y, m_Scale.z ); 

}



math::matrix4 XOrientation::GetMatrix() const
{

  math::matrix4    mat;

  mat.LookAtLH( m_Position, m_Position + m_Direction, m_LookUp );

  mat.Inverse();

  return math::matrix4().Scaling( m_Scale.x, m_Scale.y, m_Scale.z ) * mat;

}



math::matrix4 XOrientation::GetMatrixRH() const
{

  math::matrix4    mat;

  GR::tVector   vectNull( 0, 0, 0 );

  mat.LookAtRH( m_Position, m_Position + m_Direction, m_LookUp );

  mat.Inverse();
  
  return math::matrix4().Scaling( m_Scale.x, m_Scale.y, m_Scale.z ) * mat;

}



void XOrientation::Advance( float fLength )
{

  m_Position += m_Direction * fLength;

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::Strafe( float fLength )
{

  GR::tVector   vectStrafe;

  m_Direction.normalize();
  m_LookUp.normalize();

  vectStrafe = m_LookUp.cross( m_Direction );

  vectStrafe.normalize();

  m_Position += vectStrafe * fLength;
  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



void XOrientation::Raise( float fLength )
{

  m_LookUp.normalize();

  m_Position += m_LookUp * fLength;
  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();

}



GR::tVector XOrientation::CalcAdvanceVector( float fLength ) const
{

  return m_Direction * fLength;

}



GR::tVector XOrientation::CalcStrafeVector( float fLength ) const
{

  GR::tVector   vectStrafe;

  /*
  m_Direction.normalize();
  m_LookUp.normalize();
  */

  vectStrafe = m_LookUp.cross( m_Direction );

  vectStrafe.normalize();

  return vectStrafe * fLength;

}



GR::tVector XOrientation::CalcRaiseVector( float fLength ) const
{

  //m_LookUp.normalize();

  return m_LookUp * fLength;

}



void XOrientation::SetValues( const GR::tVector& vectPos, const GR::tVector& vectLookAt, const GR::tVector& vectLookUp )
{

  m_Position  = vectPos;
  m_LookAt    = vectLookAt;
  m_LookUp    = vectLookUp;

  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();
  m_LookUp.normalize();

  UpdateMatrix();

}



void XOrientation::RotateX( float fDelta )
{

  Pitch( fDelta );

}


void XOrientation::RotateY( float fDelta )
{

  Yaw( fDelta );

}


void XOrientation::RotateZ( float fDelta )
{

  Roll( fDelta );

}



void XOrientation::SetScale( const GR::tVector& vectScale )
{

  m_Scale = vectScale;
  UpdateMatrix();

}



void XOrientation::SetScale( const float fX, const float fY, const float fZ )
{

  m_Scale.set( fX, fY, fZ );
  UpdateMatrix();

}




void XOrientation::ResetToDefault()
{

  m_Scale.set( 1.0f, 1.0f, 1.0f );
  m_Position.clear();
  m_LookAt.set( 0, 0, 1.0f );
  m_LookUp.set( 0, 1.0f, 0 );

  UpdateMatrix();

}



void XOrientation::TransformCoordToWorld( GR::tVector& vectPos ) const
{

  GetMatrix().TransformCoord( vectPos );

}



void XOrientation::TransformCoord( const XOrientation& Orientation, GR::tVector& vectPos ) const
{

  GR::tVector   vectTemp( vectPos );

  vectTemp -= Orientation.GetPosition();
  vectTemp += GetPosition();

  GR::tVector   vectNewPos;

  GR::tVector   vectStrafe  = CalcStrafeVector( 1.0f );
  GR::tVector   vectRaise   = CalcRaiseVector( 1.0f );
  GR::tVector   vectAdvance = CalcAdvanceVector( 1.0f );

  vectNewPos.x = vectStrafe.dot( vectTemp );
  vectNewPos.y = vectRaise.dot( vectTemp );
  vectNewPos.z = vectAdvance.dot( vectTemp );

  vectPos = vectNewPos;

}