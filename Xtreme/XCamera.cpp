#include <Misc/Misc.h>

#include "XRenderer.h"

#include "XCamera.h"



XCamera::XCamera()
{
  m_Lerping        = false;
  m_LerpFaktor     = 0.3f;

  m_ForceDistance  = false;
  m_ForcedDistance = 10.0f;

  m_ForceYPos      = false;
  m_ForcedYPos     = 0.0f;

  m_Position.x = 0;
  m_Position.y = 0;
  m_Position.z = 0;

  m_LookAt.x = 0;
  m_LookAt.y = 1.0f;
  m_LookAt.z = 0;

  m_LookUp.x = 0;
  m_LookUp.y = 1.0;
  m_LookUp.z = 0;

  m_ScaleX       = 1.0f;
  m_ScaleY       = 1.0f;
  m_ScaleZ       = 1.0f;

  m_ViewZNear    = 1.0f;
  m_ViewZFar     = 100.0f;

  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();
  m_LookUp.normalize();

  SetScaling( 1.0f );

  m_FOV                = 45.0f;

  m_ViewPort.Width   = 640;
  m_ViewPort.Height  = 480;
  m_ViewPort.MinZ    = 0.0f;
  m_ViewPort.MaxZ    = 1.0f;
  m_ViewPort.X       = 0;
  m_ViewPort.Y       = 0;
      
  SetProjectionType( XCamera::PerspectiveFOVLH );
}



XCamera::~XCamera()
{
}



void XCamera::UpdateMatrix()
{
  m_Orientation.LookAtLH( m_Position, m_LookAt, m_LookUp );

  m_ViewMatrix = m_ScaleMatrix * m_Orientation;
}



void XCamera::SetScaling( float X, float Y, float Z )
{
  m_ScaleX = X;
  m_ScaleY = Y;
  m_ScaleZ = Z;

  m_ScaleMatrix.Scaling( m_ScaleX, m_ScaleY, m_ScaleZ );

  UpdateMatrix();
}



void XCamera::SetScalingX( float X )
{
  m_ScaleX = X;

  m_ScaleMatrix.Scaling( m_ScaleX, m_ScaleY, m_ScaleZ );

  UpdateMatrix();
}



void XCamera::SetScalingY( float Y )
{
  m_ScaleY = Y;
  m_ScaleMatrix.Scaling( m_ScaleX, m_ScaleY, m_ScaleZ );

  UpdateMatrix();
}



void XCamera::SetScalingZ( float Z )
{
  m_ScaleZ = Z;
  m_ScaleMatrix.Scaling( m_ScaleX, m_ScaleY, m_ScaleZ );

  UpdateMatrix();
}



void XCamera::SetScaling( float Factor )
{
  SetScaling( Factor, Factor, Factor );
}



void XCamera::NormalizeAngle( float& Angle )
{
  while ( Angle < -180.0f )
  {
    Angle += 360.0f;
  }
  while ( Angle >= 180.0f )
  {
    Angle -= 360.0f;
  }
}



/*
void XCamera::RetrieveYPR()
{

  float   C,
          atrx,
          atry;


  m_Yaw = (float)asin( m_viewMatrix._13 );        // Calculate Y-axis angle
  C           =  (float)cos( m_Yaw );
  m_Yaw    *= -3.1415926f / 180.0f;

  if ( fabs( C ) > 0.005 )             // Gimball lock?
  {
    atrx      =  m_viewMatrix._33 / C;           // No, so get X-axis angle
    atry      = -m_viewMatrix._23  / C;

    m_Pitch  = -(float)atan2( atry, atrx ) * 180.0f / 1415926f;

    atrx      =  m_viewMatrix._11 / C;           // Get Z-axis angle
    atry      = -m_viewMatrix._12 / C;

    m_Roll  = -(float)atan2( atry, atrx ) * 180.0f / 1415926f;
  }
  else                                 //  Gimball lock has occurred
  {
    m_Pitch  = 0;                      // Set X-axis angle to zero

    atrx      = m_viewMatrix._22;                 //  And calculate Z-axis angle
    atry      = m_viewMatrix._21;

    m_Roll  = -(float)atan2( atry, atrx ) * 180.0f / 1415926f;
  }

  NormalizeAngle( m_Pitch );
  NormalizeAngle( m_Yaw );
  NormalizeAngle( m_Roll );

  OutputDebugString( CMisc::printf( "Pitch %d Yaw %d Roll %d\n", 
                  (int)( m_Pitch * 1000 ),
                  (int)( m_Yaw * 1000 ),
                  (int)( m_Roll * 1000 ) ) );
}
*/



void XCamera::Pitch( float Delta )
{
  GR::tVector right = m_LookUp.cross( m_Direction );

  math::matrix4    rotMat;

  rotMat.RotateAboutAxis( right, Delta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::Yaw( float Delta )
{
  GR::tVector       rotVect( m_LookUp );

  math::matrix4     rotMat;

  rotMat.RotateAboutAxis( rotVect, Delta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XCamera::Roll( float Delta )
{
  GR::tVector rotVect( m_Direction );

  math::matrix4    rotMat;

  rotMat.RotateAboutAxis( rotVect, Delta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XCamera::RotateAboutAxis( const GR::tVector& Axis, const GR::f32 Delta )
{
  math::matrix4    rotMat;

  rotMat.RotateAboutAxis( Axis, Delta );

  rotMat.TransformCoord( m_LookUp );
  rotMat.TransformCoord( m_Direction );

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;
  UpdateMatrix();
}



void XCamera::Dump()
{
  /*
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  OutputDebugString( CMisc::printf( "m_Direction %d/%d/%d\n", 
            (int)( m_Direction.x * 1000 ),
            (int)( m_Direction.y * 1000 ),
            (int)( m_Direction.z * 1000 ) ) );
  OutputDebugString( CMisc::printf( "m_LookUp %d/%d/%d\n", 
            (int)( m_LookUp.x * 1000 ),
            (int)( m_LookUp.y * 1000 ),
            (int)( m_LookUp.z * 1000 ) ) );
#endif
  */
}



void XCamera::SetX( const float X )
{
  m_Position.x = X;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::SetY( const float Y )
{
  m_Position.y = Y;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::SetZ( const float Z )
{
  m_Position.z = Z;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::SetPosition( const GR::tVector& Pos )
{
  m_Position = Pos;

  m_LookAt = m_Position + m_Direction;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::SetPosition( const float X, const float Y, const float Z )
{
  SetPosition( GR::tVector( X, Y, Z ) );
}



GR::tVector XCamera::GetPosition() const
{
  return m_Position;
}



GR::tVector XCamera::GetDirection() const
{
  return m_Direction;
}



GR::tVector XCamera::GetLookUp() const
{
  return m_LookUp;
}



GR::tVector XCamera::GetLookAt() const
{
  return m_LookAt;
}



GR::tVector XCamera::GetCurrentPosition() const
{
  return m_CurrentPosition;
}



GR::tVector XCamera::GetCurrentDirection() const
{
  return m_CurrentDirection;
}



GR::tVector XCamera::GetCurrentLookUp() const
{
  return m_CurrentLookUp;
}



void XCamera::MovePosition( const GR::tVector& Delta )
{
  m_Position += Delta;

  m_Direction.normalize();
  m_LookUp.normalize();

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::SetDirection( const GR::tVector& Direction )
{
  float   length = m_Direction.length();

  m_Direction = Direction.normalized();

  m_LookAt = m_Position + m_Direction * length;

  UpdateMatrix();
}



void XCamera::SetLookAtPoint( const GR::tVector& LookAtPos )
{
  m_LookAt    = LookAtPos;
  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();
  m_LookUp.normalize();

  UpdateMatrix();
}



void XCamera::SetLookAtPoint( float X, float Y, float Z )
{
  SetLookAtPoint( GR::tVector( X, Y, Z ) );
}



void XCamera::SetLookAtAndDirection( const GR::tVector& LookAt, const GR::tVector& Direction )
{
  float   length = m_Direction.length();

  m_Direction = Direction.normalized();

  m_LookAt = LookAt;

  m_Position = m_LookAt - m_Direction * length;

  UpdateMatrix();
}



void XCamera::SetValues( const GR::tVector& Pos, const GR::tVector& LookAt, const GR::tVector& LookUp )
{
  m_Position  = Pos;
  m_LookAt    = LookAt;
  m_LookUp    = LookUp.normalized();

  m_Direction = m_LookAt - m_Position;

  m_Direction.normalize();

  UpdateMatrix();
}



void XCamera::SetLookUp( const GR::tVector& LookUp )
{
  m_LookUp = LookUp;

  m_LookUp.normalize();

  UpdateMatrix();
}



void XCamera::SetLookUp( float X, float Y, float Z )
{
  SetLookUp( GR::tVector( X, Y, Z ) );
}



math::matrix4 XCamera::GetViewMatrix()
{
  return m_ViewMatrix; 
}



math::matrix4 XCamera::GetProjectionMatrix()
{
  return m_ProjMatrix; 
}



XViewport* XCamera::GetViewport()
{
  return &m_ViewPort;
}



XCamera::ProjectionType XCamera::GetProjectionType()
{
  return m_ProjType;
}



void XCamera::SetProjectionType( XCamera::ProjectionType ProjectionType )
{
  m_ProjType = ProjectionType;

  switch ( m_ProjType )
  {
    case XCamera::PerspectiveFOVLH:
      m_ProjMatrix.ProjectionPerspectiveFovLH( m_FOV,
                                               (float)m_ViewPort.Width / (float)m_ViewPort.Height,
                                               m_ViewZNear,
                                               m_ViewZFar );
      break;
    case XCamera::PerspectiveFOVRH:
      m_ProjMatrix.ProjectionPerspectiveFovRH( m_FOV,
                                               (float)m_ViewPort.Width / (float)m_ViewPort.Height,
                                               m_ViewZNear,
                                               m_ViewZFar );
      break;
    case XCamera::OrthoLH:
      m_ProjMatrix.OrthoLH( m_ViewWidth,
                            m_ViewHeight,
                            m_ViewZNear,
                            m_ViewZFar );
      break;
    case XCamera::OrthoRH:
      m_ProjMatrix.OrthoRH( m_ViewWidth,
                            m_ViewHeight,
                            m_ViewZNear,
                            m_ViewZFar );
      break;
  }
}



math::matrix4 XCamera::UpdateLerp( float ElapsedSeconds )
{
  math::matrix4    matCurrent;

  float   fFaktor = 1.0f - powf( m_LerpFaktor, ElapsedSeconds );

  m_CurrentPosition.interpolate( m_CurrentPosition,   m_Position, fFaktor );
  m_CurrentPosition.interpolate( m_CurrentDirection,  m_Direction, fFaktor );
  m_CurrentPosition.interpolate( m_CurrentLookUp,     m_LookUp, fFaktor );

  if ( m_ForceDistance )
  {
    //m_CurrentDirection = m_Direction;
    m_CurrentDirection.normalize();

    m_CurrentPosition = m_LookAt - m_CurrentDirection * m_ForcedDistance;

    m_CurrentDirection = m_LookAt - m_CurrentPosition;

    m_CurrentDirection.normalize();

    m_CurrentLookUp = m_LookUp;

    if ( m_ForceYPos )
    {
      m_CurrentPosition.y = m_ForcedYPos;

      m_CurrentDirection = m_LookAt - m_CurrentPosition;

      m_CurrentDirection.normalize();
    }
  }

  matCurrent.LookAtLH( m_CurrentPosition,
                       m_LookAt,
                       m_CurrentLookUp );

  return matCurrent;
}



void XCamera::ForceDistance( bool Force, float Distance )
{
  m_ForceDistance   = Force;
  m_ForcedDistance  = Distance;
}



void XCamera::ForceYPos( bool Force, float Pos )
{
  m_ForceYPos      = Force;
  m_ForcedYPos     = Pos;
}



void XCamera::Advance( float Length )
{
  m_Position += m_Direction * Length;

  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::Strafe( float Length )
{
  GR::tVector   strafe;

  m_Direction.normalize();
  m_LookUp.normalize();

  strafe = m_LookUp.cross( m_Direction );
  strafe.normalize();

  m_Position += strafe * Length;
  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



void XCamera::Raise( float Length )
{
  m_LookUp.normalize();

  m_Position += m_LookUp * Length;
  m_LookAt = m_Position + m_Direction;

  UpdateMatrix();
}



GR::tVector XCamera::CalcAdvanceVector( GR::f32 Length ) const
{
  return m_Direction * Length;
}



GR::tVector XCamera::CalcStrafeVector( GR::f32 Length ) const
{
  GR::tVector   strafe;

  strafe = m_LookUp.cross( m_Direction );
  strafe.normalize();

  return strafe * Length;
}



GR::tVector XCamera::CalcRaiseVector( GR::f32 Length ) const
{
  return m_LookUp * Length;
}



float XCamera::GetScalingX() 
{ 
  return m_ScaleX; 
}



float XCamera::GetScalingY() 
{ 
  return m_ScaleY; 
}



float XCamera::GetScalingZ() 
{ 
  return m_ScaleZ; 
}



void XCamera::ViewFOV( float FOV )
{
  m_FOV = FOV;
}



void XCamera::ViewPos( int X, int Y )
{
  m_ViewPort.X = X;
  m_ViewPort.Y = Y;
}



void XCamera::ViewOrthoLH( float Width, float Height, float ZMin, float ZMax )
{
  m_ViewWidth  = Width;
  m_ViewHeight = Height;
  m_ViewZNear  = ZMin;
  m_ViewZFar   = ZMax;
  SetProjectionType( OrthoLH );
}



void XCamera::ViewWidth( int Width )
{
  m_ViewPort.Width  = Width;
  m_ViewWidth       = (float)Width;
  SetProjectionType( m_ProjType );
}



void XCamera::ViewHeight( int Height )
{
  m_ViewPort.Height = Height;
  m_ViewHeight      = (float)Height;
  SetProjectionType( m_ProjType );
}



void XCamera::ViewZMin( float ZNear )
{
  m_ViewPort.MinZ = ZNear;
  SetProjectionType( m_ProjType );
}



void XCamera::ViewZMax( float ZFar )
{
  m_ViewPort.MaxZ = ZFar;
  SetProjectionType( m_ProjType );
}



void XCamera::ZNear( float ZNear )
{
  m_ViewZNear = ZNear;
  SetProjectionType( m_ProjType );
}



void XCamera::ZFar( float ZFar )
{
  m_ViewZFar = ZFar;
  SetProjectionType( m_ProjType );
}



bool XCamera::Lerping() const
{
  return m_Lerping;
}



void XCamera::Lerping( float LerpFaktor, bool Lerp )
{
  m_Lerping     = Lerp;
  m_LerpFaktor  = LerpFaktor;

  m_CurrentPosition   = m_Position;
  m_CurrentDirection  = m_Direction;
  m_CurrentLookUp     = m_LookUp;
}

