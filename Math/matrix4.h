#ifndef _MATRIX4_H_
#define _MATRIX4_H_

#include <GR/GRTypes.h>

#include <Math/vector3.h>

#include <math.h>

#include <debug/debugclient.h>


// Matrix ist auf DirectX gemünzt!

namespace math
{

  struct matrix4
  {
    struct tStruct
    {
      float        _11, _12, _13, _14;
      float        _21, _22, _23, _24;
      float        _31, _32, _33, _34;
      float        _41, _42, _43, _44;
    };

    union
    {
      tStruct   ms;
      float     m[4][4];
      float     M[16];
    };


    matrix4() 
    {
    };

    matrix4( const matrix4& rhs )
    {
      memcpy( &m[0][0], &rhs.m[0][0], sizeof( float ) * 16 );
    }

    matrix4( float f11, float f12, float f13, float f14,
             float f21, float f22, float f23, float f24,
             float f31, float f32, float f33, float f34,
             float f41, float f42, float f43, float f44 )
    {
      ms._11 = f11; ms._12 = f12; ms._13 = f13; ms._14 = f14;
      ms._21 = f21; ms._22 = f22; ms._23 = f23; ms._24 = f24;
      ms._31 = f31; ms._32 = f32; ms._33 = f33; ms._34 = f34;
      ms._41 = f41; ms._42 = f42; ms._43 = f43; ms._44 = f44;
    }

    matrix4& Identity()
    {
      m[0][1] = m[0][2] = m[0][3] =
      m[1][0] = m[1][2] = m[1][3] =
      m[2][0] = m[2][1] = m[2][3] =
      m[3][0] = m[3][1] = m[3][2] = 0.0f;

      m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
      return *this;
    }

    matrix4& Transpose()
    {
      std::swap( m[0][1], m[1][0] );
      std::swap( m[0][2], m[2][0] );
      std::swap( m[0][3], m[3][0] );

      std::swap( m[1][2], m[2][1] );
      std::swap( m[1][3], m[3][1] );

      std::swap( m[2][3], m[3][2] );

      return *this;
    }

    matrix4& Billboard( const matrix4& matView, const GR::tVector& vectPos )
    {

      *this = matView;

      Transpose();

      ms._14 = ms._24 = ms._34 = 0.0f;	

      //position the billboard	
      ms._41 = vectPos.x;	
      ms._42 = vectPos.y;	
      ms._43 = vectPos.z;	

      return *this;
    }

    matrix4& OrientAboutAxis( GR::tVector& vectDir, const GR::f32 fDegrees )
    {
      GR::tVector zAxis( vectDir );

      zAxis.normalize();

      // Build the Y vector of the matrix (handle the degenerate case
      // in the way that 3DS does) -- This is not the TRUE vector, only
      // a reference vector.

      GR::tVector     yAxis;

      if ( ( !zAxis.x ) 
      &&   ( !zAxis.z ) )
      {
        yAxis = GR::tVector( -zAxis.y, 0.0f, 0.0f );
      }
      else
      {
        yAxis = GR::tVector( 0.0f, 1.0f, 0.0f );
      }

      // Build the X axis vector based on the two existing vectors
      GR::tVector xAxis = yAxis.cross( zAxis );

      xAxis.normalize();

      // Correct the Y reference vector
      yAxis = xAxis.cross( zAxis );
      yAxis.normalize();
      yAxis = -yAxis;

      // Generate rotation matrix without roll included
      math::matrix4   rot;

      rot.m[0][0] = xAxis.x;
      rot.m[0][1] = xAxis.y;
      rot.m[0][2] = xAxis.z;
      rot.m[0][3] = 0.0f;

      rot.m[1][0] = yAxis.x;
      rot.m[1][1] = yAxis.y;
      rot.m[1][2] = yAxis.z;
      rot.m[1][3] = 0.0f;

      rot.m[2][0] = zAxis.x;
      rot.m[2][1] = zAxis.y;
      rot.m[2][2] = zAxis.z;
      rot.m[2][3] = 0.0f;

      rot.m[3][0] = 0.0f;
      rot.m[3][1] = 0.0f;
      rot.m[3][2] = 0.0f;
      rot.m[3][3] = 1.0f;

      // Generate the Z rotation matrix for roll (bank)
      math::matrix4   roll;

      roll.RotationZ( fDegrees );

      // Concatinate them for a complete rotation matrix that includes
      // all rotations
      *this = roll * rot;

      return *this;
    }

    matrix4& RotationYawPitchRoll( const GR::f32 fYaw, const GR::f32 fPitch, const GR::f32 fRoll )
    {
      math::matrix4     matBeginn = RotationY( fYaw );

      matBeginn *= RotationX( fPitch );
      matBeginn *= RotationZ( fRoll );

      *this = matBeginn;

      return *this;
    }

	  matrix4& ProjectionPerspectiveFovLH( float fieldOfView, float aspectRatio, float zNear, float zFar )
	  {
      // geprüft

      float   yScale = 1 / tan( 3.1415926f * fieldOfView * 0.5f / 180.0f );// / sin( 3.1415926f * fieldOfView * 0.5f / 180.0f );
      float   xScale = yScale / aspectRatio;

      m[0][0] = xScale;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;

		  m[0][1] = 0;
		  m[1][1] = yScale;
		  m[2][1] = 0;
		  m[3][1] = 0;

		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = zFar / ( zFar - zNear );
		  m[3][2] = zNear * zFar / ( zNear - zFar );

		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = 1;
		  m[3][3] = 0;

      return *this;
	  }

	  matrix4& ProjectionPerspectiveFovRH( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar )
	  {
		  float h = (float)( cos( fieldOfViewRadians / 2.0f ) / sin( fieldOfViewRadians / 2.0f ) );
		  float w = h / aspectRatio;

      m[0][0] = 2 * zNear / w;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;

		  m[0][1] = 0;
		  m[1][1] = 2 * zNear / h;
		  m[2][1] = 0;
		  m[3][1] = 0;

		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = zFar / ( zFar - zNear );
		  m[3][2] = -1;

		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = zNear * zFar / ( zNear - zFar );
		  m[3][3] = 0;

      return *this;
	  }

	  matrix4& OrthoLH( float widthOfViewVolume, float heightOfViewVolume, float zNear, float zFar )
	  {
      // geprüft
		  m[0][0] = 2 / widthOfViewVolume;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;
      
		  m[0][1] = 0;
		  m[1][1] = 2 / heightOfViewVolume;
		  m[2][1] = 0;
		  m[3][1] = 0;
      
		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = 1 / ( zFar - zNear );
		  m[3][2] = zNear / ( zNear - zFar );
      
		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = 0;
		  m[3][3] = 1;

      return *this;
	  }

	  matrix4& OrthoRH( float widthOfViewVolume, float heightOfViewVolume, float zNear, float zFar )
	  {
		  m[0][0] = 2 / widthOfViewVolume;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;
      
		  m[0][1] = 0;
		  m[1][1] = 2 / heightOfViewVolume;
		  m[2][1] = 0;
		  m[3][1] = 0;
      
		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = 1 / ( zNear - zFar );
		  m[3][2] = -1;
      
		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = zNear / ( zNear - zFar );
		  m[3][3] = 0;

      return *this;
	  }

	  matrix4& OrthoOffCenterLH( float fMinX, float fMaxX, float fMinY, float fMaxY, float fNearZ, float fFarZ )
	  {
		  m[0][0] = 2 / ( fMaxX - fMinX );
		  m[0][1] = 0;
		  m[0][2] = 0;
		  m[0][3] = 0;
          
		  m[1][0] = 0;
		  m[1][1] = 2 / ( fMaxY - fMinY );
		  m[1][2] = 0;
		  m[1][3] = 0;
          
		  m[2][0] = 0;
		  m[2][1] = 0;
		  m[2][2] = 1 / ( fFarZ - fNearZ );
		  m[2][3] = 0;
          
		  m[3][0] = ( fMinX + fMaxX ) / ( fMinX - fMaxX );
		  m[3][1] = ( fMinY + fMaxY ) / ( fMinY - fMaxY );
		  m[3][2] = fNearZ / ( fNearZ - fFarZ );
		  m[3][3] = 1;

      return *this;
	  }

	  matrix4& OrthoOffCenterRH( float fMinX, float fMaxX, float fMinY, float fMaxY, float fNearZ, float fFarZ )
	  {
		  m[0][0] = 2 / ( fMaxX - fMinX );
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;
      
		  m[0][1] = 0;
		  m[1][1] = 2 / ( fMinY - fMaxY );
		  m[2][1] = 0;
		  m[3][1] = 0;
      
		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = 1 / ( fNearZ - fFarZ );
		  m[3][2] = 0;
      
		  m[0][3] = ( fMinX + fMaxX ) / ( fMinX - fMaxX );
		  m[1][3] = ( fMinY + fMaxY ) / ( fMaxY - fMinY );
		  m[2][3] = fNearZ / ( fNearZ - fFarZ );
		  m[3][3] = 1;

      return *this;
	  }

	  matrix4& ProjectionPerspectiveLH( float widthOfViewVolume, float heightOfViewVolume, float zNear, float zFar )
	  {
      // geprüft
		  m[0][0] = 2 * zNear / widthOfViewVolume;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;
      
		  m[0][1] = 0;
		  m[1][1] = 2 * zNear / heightOfViewVolume;
		  m[2][1] = 0;
		  m[3][1] = 0;
      
		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = zFar / ( zFar - zNear );
		  m[3][2] = 1;
      
		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = zNear * zFar / ( zNear - zFar );
		  m[3][3] = 0;

      return *this;
	  }

	  matrix4& ProjectionPerspectiveRH( float widthOfViewVolume, float heightOfViewVolume, float zNear, float zFar )
	  {
		  m[0][0] = 2 * zNear / widthOfViewVolume;
		  m[1][0] = 0;
		  m[2][0] = 0;
		  m[3][0] = 0;
      
		  m[0][1] = 0;
		  m[1][1] = 2 * zNear / heightOfViewVolume;
		  m[2][1] = 0;
		  m[3][1] = 0;
      
		  m[0][2] = 0;
		  m[1][2] = 0;
		  m[2][2] = zFar / ( zNear - zFar );
		  m[3][2] = -1;
      
		  m[0][3] = 0;
		  m[1][3] = 0;
		  m[2][3] = zNear * zFar / ( zNear - zFar );
		  m[3][3] = 0;

      return *this;
	  }

	  matrix4& LookAtLH( const vector3& position,
											 const vector3& target,
											 const vector3& upVector )
	  {
      // geprüft
		  vector3 zaxis = target - position;
		  zaxis.normalize();

		  vector3 xaxis = upVector.cross( zaxis );
		  xaxis.normalize();

		  vector3 yaxis = zaxis.cross( xaxis );

		  m[0][0] = xaxis.x;
		  m[0][1] = yaxis.x;
		  m[0][2] = zaxis.x;
		  m[0][3] = 0;
      
		  m[1][0] = xaxis.y;
		  m[1][1] = yaxis.y;
		  m[1][2] = zaxis.y;
		  m[1][3] = 0;
      
		  m[2][0] = xaxis.z;
		  m[2][1] = yaxis.z;
		  m[2][2] = zaxis.z;
		  m[2][3] = 0;
      
		  m[3][0] = -xaxis.dot( position );
		  m[3][1] = -yaxis.dot( position );
		  m[3][2] = -zaxis.dot( position );
		  m[3][3] = 1.0f;

      return *this;
	  }

	  matrix4& LookAtRH( const vector3& position,
											 const vector3& target,
											 const vector3& upVector )
	  {
		  vector3 zaxis = position - target;
		  zaxis.normalize();

		  vector3 xaxis = upVector.cross( zaxis );
		  xaxis.normalize();

		  vector3 yaxis = zaxis.cross( xaxis );

		  m[0][0] = xaxis.x;
		  m[1][0] = yaxis.x;
		  m[2][0] = zaxis.x;
		  m[3][0] = 0;
      
		  m[0][1] = xaxis.y;
		  m[1][1] = yaxis.y;
		  m[2][1] = zaxis.y;
		  m[3][1] = 0;
      
		  m[0][2] = xaxis.z;
		  m[1][2] = yaxis.z;
		  m[2][2] = zaxis.z;
		  m[3][2] = 0;
      
		  m[0][3] = -xaxis.dot( position );
		  m[1][3] = -yaxis.dot( position );
		  m[2][3] = -zaxis.dot( position );
		  m[3][3] = 1.0f;

      return *this;
	  }

    matrix4& Translation( const math::vector3& vectOffset )
    {
      return Translation( vectOffset.x, vectOffset.y, vectOffset.z );
    }

    matrix4& Translation( float fX, float fY, float fZ )
    {
      ms._11 = 1.0f;    ms._12 = 0.0f;    ms._13 = 0.0f;    ms._14 = 0.0f;
      ms._21 = 0.0f;    ms._22 = 1.0f;    ms._23 = 0.0f;    ms._24 = 0.0f;
      ms._31 = 0.0f;    ms._32 = 0.0f;    ms._33 = 1.0f;    ms._34 = 0.0f;
      ms._41 = fX;      ms._42 = fY;      ms._43 = fZ;      ms._44 = 1.0f;

      return *this;
    }

    matrix4& Scaling( float sx,float sy, float sz )
    {
      ms._11 = sx;      ms._12 =  0.0f;   ms._13 = 0.0f;    ms._14 = 0.0f;
      ms._21 = 0.0f;    ms._22 =  sy;     ms._23 = 0.0f;    ms._24 = 0.0f;
      ms._31 = 0.0f;    ms._32 =  0.0f;   ms._33 = sz;      ms._34 = 0.0f;
      ms._41 = 0.0f;    ms._42 =  0.0f;   ms._43 = 0.0f;    ms._44 = 1.0f;

      return *this;
    }

    matrix4& RotationX( float angle )
    {
      float my_sin, my_cos;

      angle = 3.1415926f * angle / 180.0f;

      my_sin = sinf( angle );
      my_cos = cosf( angle );

      ms._11 = 1.0f;   ms._12 =  0.0f;     ms._13 = 0.0f;     ms._14 = 0.0f;
      ms._21 = 0.0f;   ms._22 =  my_cos;   ms._23 = my_sin;   ms._24 = 0.0f;
      ms._31 = 0.0f;   ms._32 = -my_sin;   ms._33 = my_cos;   ms._34 = 0.0f;
      ms._41 = 0.0f;   ms._42 =  0.0f;     ms._43 = 0.0f;     ms._44 = 1.0f;

      return *this;
    }

    matrix4& RotationY( float angle )
    {
      float my_sin, my_cos;

      angle = 3.1415926f * angle / 180.0f;

      my_sin = sinf( angle );
      my_cos = cosf( angle );

      ms._11 = my_cos;  ms._12 =  0.0f;   ms._13 = -my_sin; ms._14 = 0.0f;
      ms._21 = 0.0f;    ms._22 =  1.0f;   ms._23 = 0.0f;    ms._24 = 0.0f;
      ms._31 = my_sin;  ms._32 =  0.0f;   ms._33 = my_cos;  ms._34 = 0.0f;
      ms._41 = 0.0f;    ms._42 =  0.0f;   ms._43 = 0.0f;    ms._44 = 1.0f;

      return *this;
    }

    matrix4& RotationZ( float angle )
    {
      float my_sin, my_cos;

      angle = 3.1415926f * angle / 180.0f;

      my_sin = sinf( angle );
      my_cos = cosf( angle );

      ms._11 = my_cos;  ms._12 = my_sin;  ms._13 = 0.0f;    ms._14 = 0.0f;
      ms._21 = -my_sin; ms._22 = my_cos;  ms._23 = 0.0f;    ms._24 = 0.0f;
      ms._31 = 0.0f;    ms._32 = 0.0f;    ms._33 = 1.0f;    ms._34 = 0.0f;
      ms._41 = 0.0f;    ms._42 = 0.0f;    ms._43 = 0.0f;    ms._44 = 1.0f;

      return *this;
    }

    void ToEulerAngles( float& fAngleX, float& fAngleY, float& fAngleZ )
    {

      static float  fRadiansToDegree = 180.0f / 3.1415926f;

      fAngleY     = asinf( M[8] );        // Calculate Y-axis angle
      float fCY   =  cosf( fAngleY );
      fAngleY     *= fRadiansToDegree;

      if ( fabsf( fCY ) > 0.005f )        
      {
        float ftrx      =  M[10] / fCY;    // No, so get X-axis angle

        float ftry      = -M[9]  / fCY;

        fAngleX  = atan2f( ftry, ftrx ) * fRadiansToDegree;

        ftrx      =  M[0] / fCY;            // Get Z-axis angle
        ftry      = -M[4] / fCY;

        fAngleZ  = atan2f( ftry, ftrx ) * fRadiansToDegree;
      }
      else
      {
        // Gimbal lock?
        fAngleX  = 0;                      // Set X-axis angle to zero

        float ftrx      =  M[5];           // And calculate Z-axis angle
        float ftry      =  M[1];

        fAngleZ  = atan2f( ftry, ftrx ) * fRadiansToDegree;
      }

      if ( fAngleX < 0 )
      {
        fAngleX += 360;
      }
      if ( fAngleY < 0 )
      {
        fAngleY += 360;
      }
      if ( fAngleZ < 0 )
      {
        fAngleZ += 360;
      }
    }

    matrix4& RotateAboutAxis( const vector3& vectAxis, float fAngle )
    {
      float rcos = cosf( fAngle * 3.1415926f / 180.0f );
      float rsin = sinf( fAngle * 3.1415926f / 180.0f );

      memset( &m, 0, sizeof( m ) );

	    m[0][0] =               rcos + vectAxis.x * vectAxis.x * ( 1 - rcos );
	    m[0][1] =  vectAxis.z * rsin + vectAxis.y * vectAxis.x * ( 1 - rcos );
	    m[0][2] = -vectAxis.y * rsin + vectAxis.z * vectAxis.x * ( 1 - rcos );
	    m[1][0] = -vectAxis.z * rsin + vectAxis.x * vectAxis.y * ( 1 - rcos );
	    m[1][1] =               rcos + vectAxis.y * vectAxis.y * ( 1 - rcos );
	    m[1][2] =  vectAxis.x * rsin + vectAxis.z * vectAxis.y * ( 1 - rcos );
	    m[2][0] =  vectAxis.y * rsin + vectAxis.x * vectAxis.z * ( 1 - rcos );
	    m[2][1] = -vectAxis.x * rsin + vectAxis.y * vectAxis.z * ( 1 - rcos );
	    m[2][2] =               rcos + vectAxis.z * vectAxis.z * ( 1 - rcos );
      m[3][3] = 1.0f;

      return *this;
    }

    matrix4& Inverse() 
    {
		  matrix4 temp;

		  if (getInverse(temp))
		  {
			  *this = temp;
		  }

      return *this;
    }

    inline float
        MINOR( const matrix4& m, const int r0, const int r1, const int r2, const int c0, const int c1, const int c2) const
    {
        return m.m[c0][r0] * ( m.m[c1][r1] * m.m[c2][r2] - m.m[c1][r2] * m.m[c2][r1] ) -
               m.m[c1][r0] * ( m.m[c0][r1] * m.m[c2][r2] - m.m[c0][r2] * m.m[c2][r1] ) +
               m.m[c2][r0] * ( m.m[c0][r1] * m.m[c1][r2] - m.m[c0][r2] * m.m[c1][r1] );
    }


    matrix4 adjoint() const
    {
        return matrix4( MINOR(*this, 1, 2, 3, 1, 2, 3),
            -MINOR(*this, 0, 2, 3, 1, 2, 3),
            MINOR(*this, 0, 1, 3, 1, 2, 3),
            -MINOR(*this, 0, 1, 2, 1, 2, 3),

            -MINOR(*this, 1, 2, 3, 0, 2, 3),
            MINOR(*this, 0, 2, 3, 0, 2, 3),
            -MINOR(*this, 0, 1, 3, 0, 2, 3),
            MINOR(*this, 0, 1, 2, 0, 2, 3),

            MINOR(*this, 1, 2, 3, 0, 1, 3),
            -MINOR(*this, 0, 2, 3, 0, 1, 3),
            MINOR(*this, 0, 1, 3, 0, 1, 3),
            -MINOR(*this, 0, 1, 2, 0, 1, 3),

            -MINOR(*this, 1, 2, 3, 0, 1, 2),
            MINOR(*this, 0, 2, 3, 0, 1, 2),
            -MINOR(*this, 0, 1, 3, 0, 1, 2),
            MINOR(*this, 0, 1, 2, 0, 1, 2));
    }


    float determinant() const
    {
        return m[0][0] * MINOR(*this, 1, 2, 3, 1, 2, 3) -
            m[0][1] * MINOR(*this, 1, 2, 3, 0, 2, 3) +
            m[0][2] * MINOR(*this, 1, 2, 3, 0, 1, 3) -
            m[0][3] * MINOR(*this, 1, 2, 3, 0, 1, 2);
    }

    matrix4 inverse() const
    {
		  matrix4 temp;

		  if ( getInverse( temp ) )
		  {
        return temp;
		  }

      return *this;
        //return adjoint() * (1.0f / determinant());
    }

	  inline bool getInverse( matrix4& out ) const
	  {
		  /// Calculates the inverse of this Matrix 
		  /// The inverse is calculated using Cramers rule.
		  /// If no inverse exists then 'false' is returned.

		  const matrix4 &m = *this;

		  float d = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3))	- (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3))
				  + (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3))	+ (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3))
				  - (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3))	+ (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3));
  		
		  if ( d == 0.f )
      {
        dh::Log( "matrix4::getInverse failed" );
			  return false;
      }

		  d = 1.f / d;

		  out(0, 0) = d * (m(1, 1) * (m(2, 2) * m(3, 3) - m(3, 2) * m(2, 3)) + m(2, 1) * (m(3, 2) * m(1, 3) - m(1, 2) * m(3, 3)) + m(3, 1) * (m(1, 2) * m(2, 3) - m(2, 2) * m(1, 3)));
		  out(1, 0) = d * (m(1, 2) * (m(2, 0) * m(3, 3) - m(3, 0) * m(2, 3)) + m(2, 2) * (m(3, 0) * m(1, 3) - m(1, 0) * m(3, 3)) + m(3, 2) * (m(1, 0) * m(2, 3) - m(2, 0) * m(1, 3)));
		  out(2, 0) = d * (m(1, 3) * (m(2, 0) * m(3, 1) - m(3, 0) * m(2, 1)) + m(2, 3) * (m(3, 0) * m(1, 1) - m(1, 0) * m(3, 1)) + m(3, 3) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)));
		  out(3, 0) = d * (m(1, 0) * (m(3, 1) * m(2, 2) - m(2, 1) * m(3, 2)) + m(2, 0) * (m(1, 1) * m(3, 2) - m(3, 1) * m(1, 2)) + m(3, 0) * (m(2, 1) * m(1, 2) - m(1, 1) * m(2, 2)));
		  out(0, 1) = d * (m(2, 1) * (m(0, 2) * m(3, 3) - m(3, 2) * m(0, 3)) + m(3, 1) * (m(2, 2) * m(0, 3) - m(0, 2) * m(2, 3)) + m(0, 1) * (m(3, 2) * m(2, 3) - m(2, 2) * m(3, 3)));
		  out(1, 1) = d * (m(2, 2) * (m(0, 0) * m(3, 3) - m(3, 0) * m(0, 3)) + m(3, 2) * (m(2, 0) * m(0, 3) - m(0, 0) * m(2, 3)) + m(0, 2) * (m(3, 0) * m(2, 3) - m(2, 0) * m(3, 3)));
		  out(2, 1) = d * (m(2, 3) * (m(0, 0) * m(3, 1) - m(3, 0) * m(0, 1)) + m(3, 3) * (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) + m(0, 3) * (m(3, 0) * m(2, 1) - m(2, 0) * m(3, 1)));
		  out(3, 1) = d * (m(2, 0) * (m(3, 1) * m(0, 2) - m(0, 1) * m(3, 2)) + m(3, 0) * (m(0, 1) * m(2, 2) - m(2, 1) * m(0, 2)) + m(0, 0) * (m(2, 1) * m(3, 2) - m(3, 1) * m(2, 2)));
		  out(0, 2) = d * (m(3, 1) * (m(0, 2) * m(1, 3) - m(1, 2) * m(0, 3)) + m(0, 1) * (m(1, 2) * m(3, 3) - m(3, 2) * m(1, 3)) + m(1, 1) * (m(3, 2) * m(0, 3) - m(0, 2) * m(3, 3)));
		  out(1, 2) = d * (m(3, 2) * (m(0, 0) * m(1, 3) - m(1, 0) * m(0, 3)) + m(0, 2) * (m(1, 0) * m(3, 3) - m(3, 0) * m(1, 3)) + m(1, 2) * (m(3, 0) * m(0, 3) - m(0, 0) * m(3, 3)));
		  out(2, 2) = d * (m(3, 3) * (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) + m(0, 3) * (m(1, 0) * m(3, 1) - m(3, 0) * m(1, 1)) + m(1, 3) * (m(3, 0) * m(0, 1) - m(0, 0) * m(3, 1)));
		  out(3, 2) = d * (m(3, 0) * (m(1, 1) * m(0, 2) - m(0, 1) * m(1, 2)) + m(0, 0) * (m(3, 1) * m(1, 2) - m(1, 1) * m(3, 2)) + m(1, 0) * (m(0, 1) * m(3, 2) - m(3, 1) * m(0, 2)));
		  out(0, 3) = d * (m(0, 1) * (m(2, 2) * m(1, 3) - m(1, 2) * m(2, 3)) + m(1, 1) * (m(0, 2) * m(2, 3) - m(2, 2) * m(0, 3)) + m(2, 1) * (m(1, 2) * m(0, 3) - m(0, 2) * m(1, 3)));
		  out(1, 3) = d * (m(0, 2) * (m(2, 0) * m(1, 3) - m(1, 0) * m(2, 3)) + m(1, 2) * (m(0, 0) * m(2, 3) - m(2, 0) * m(0, 3)) + m(2, 2) * (m(1, 0) * m(0, 3) - m(0, 0) * m(1, 3)));
		  out(2, 3) = d * (m(0, 3) * (m(2, 0) * m(1, 1) - m(1, 0) * m(2, 1)) + m(1, 3) * (m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1)) + m(2, 3) * (m(1, 0) * m(0, 1) - m(0, 0) * m(1, 1)));
		  out(3, 3) = d * (m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) + m(1, 0) * (m(2, 1) * m(0, 2) - m(0, 1) * m(2, 2)) + m(2, 0) * (m(0, 1) * m(1, 2) - m(1, 1) * m(0, 2)));

		  return true;
	  }


	  inline bool makeInverse()
	  {
		  matrix4 temp;

		  if (getInverse(temp))
		  {
			  *this = temp;
			  return true;
		  }

		  return false;
	  }


    // access grants
    float& operator () ( unsigned int iRow, unsigned int iCol )
    {
      return m[iRow][iCol];
    }

    float  operator () ( unsigned int iRow, unsigned int iCol ) const
    {
      return m[iRow][iCol];
    }


    // assignment operators
    matrix4& operator *= ( const matrix4& mat )
    {
		  float   newMatrix[16];

		  const float* m2 = M;
      const float* m1 = mat.M;


		  newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
		  newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
		  newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
		  newMatrix[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
  		
		  newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
		  newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
		  newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
		  newMatrix[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
  		
		  newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
		  newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
      newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
		  newMatrix[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];
  		
		  newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
		  newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
		  newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
		  newMatrix[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
  		
		  for ( int i = 0; i < 16; ++i )
      {
			  M[i] = newMatrix[i];
      }
		  return *this;
    }

    matrix4& operator += ( const matrix4& mat )
    {
      ms._11 += mat.ms._11; ms._12 += mat.ms._12; ms._13 += mat.ms._13; ms._14 += mat.ms._14;
      ms._21 += mat.ms._21; ms._22 += mat.ms._22; ms._23 += mat.ms._23; ms._24 += mat.ms._24;
      ms._31 += mat.ms._31; ms._32 += mat.ms._32; ms._33 += mat.ms._33; ms._34 += mat.ms._34;
      ms._41 += mat.ms._41; ms._42 += mat.ms._42; ms._43 += mat.ms._43; ms._44 += mat.ms._44;
      return *this;
    }

    matrix4& operator -= ( const matrix4& mat )
    {
      ms._11 -= mat.ms._11; ms._12 -= mat.ms._12; ms._13 -= mat.ms._13; ms._14 -= mat.ms._14;
      ms._21 -= mat.ms._21; ms._22 -= mat.ms._22; ms._23 -= mat.ms._23; ms._24 -= mat.ms._24;
      ms._31 -= mat.ms._31; ms._32 -= mat.ms._32; ms._33 -= mat.ms._33; ms._34 -= mat.ms._34;
      ms._41 -= mat.ms._41; ms._42 -= mat.ms._42; ms._43 -= mat.ms._43; ms._44 -= mat.ms._44;
      return *this;
    }

    matrix4& operator *= ( float f )
    {
      ms._11 *= f; ms._12 *= f; ms._13 *= f; ms._14 *= f;
      ms._21 *= f; ms._22 *= f; ms._23 *= f; ms._24 *= f;
      ms._31 *= f; ms._32 *= f; ms._33 *= f; ms._34 *= f;
      ms._41 *= f; ms._42 *= f; ms._43 *= f; ms._44 *= f;
      return *this;
    }

    matrix4& operator /= ( float f )
    {
      float fInv = 1.0f / f;

      ms._11 *= fInv; ms._12 *= fInv; ms._13 *= fInv; ms._14 *= fInv;
      ms._21 *= fInv; ms._22 *= fInv; ms._23 *= fInv; ms._24 *= fInv;
      ms._31 *= fInv; ms._32 *= fInv; ms._33 *= fInv; ms._34 *= fInv;
      ms._41 *= fInv; ms._42 *= fInv; ms._43 *= fInv; ms._44 *= fInv;
      return *this;
    }

    // unary operators
    matrix4 operator + () const
    {
      return *this;
    }

    matrix4 operator - () const
    {
      return matrix4( -ms._11, -ms._12, -ms._13, -ms._14,
                      -ms._21, -ms._22, -ms._23, -ms._24,
                      -ms._31, -ms._32, -ms._33, -ms._34,
                      -ms._41, -ms._42, -ms._43, -ms._44 );
    }

    // binary operators
    matrix4 operator * ( const matrix4& mat ) const
    {
		  matrix4     tmtrx;
		  const float* m2 = M;
      const float* m1 = mat.M;

		  float*  m3 = tmtrx.M;

		  m3[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
		  m3[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
		  m3[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
		  m3[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];
  		
		  m3[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
		  m3[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
		  m3[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
		  m3[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];
  		
		  m3[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
		  m3[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
		  m3[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
		  m3[11] = m1[3]*m2[8] + m1[7]*m2[10] + m1[11]*m2[10] + m1[15]*m2[11];
  		
		  m3[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
		  m3[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
		  m3[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
		  m3[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

		  return tmtrx;
    }

    matrix4 operator + ( const matrix4& mat ) const
    {
      return matrix4( ms._11 + mat.ms._11, ms._12 + mat.ms._12, ms._13 + mat.ms._13, ms._14 + mat.ms._14,
                      ms._21 + mat.ms._21, ms._22 + mat.ms._22, ms._23 + mat.ms._23, ms._24 + mat.ms._24,
                      ms._31 + mat.ms._31, ms._32 + mat.ms._32, ms._33 + mat.ms._33, ms._34 + mat.ms._34,
                      ms._41 + mat.ms._41, ms._42 + mat.ms._42, ms._43 + mat.ms._43, ms._44 + mat.ms._44 );
    }

    matrix4 operator - ( const matrix4& mat ) const
    {
      return matrix4( ms._11 - mat.ms._11, ms._12 - mat.ms._12, ms._13 - mat.ms._13, ms._14 - mat.ms._14,
                      ms._21 - mat.ms._21, ms._22 - mat.ms._22, ms._23 - mat.ms._23, ms._24 - mat.ms._24,
                      ms._31 - mat.ms._31, ms._32 - mat.ms._32, ms._33 - mat.ms._33, ms._34 - mat.ms._34,
                      ms._41 - mat.ms._41, ms._42 - mat.ms._42, ms._43 - mat.ms._43, ms._44 - mat.ms._44 );
    }

    matrix4 operator * ( float f ) const
    {
      return matrix4( ms._11 * f, ms._12 * f, ms._13 * f, ms._14 * f,
                      ms._21 * f, ms._22 * f, ms._23 * f, ms._24 * f,
                      ms._31 * f, ms._32 * f, ms._33 * f, ms._34 * f,
                      ms._41 * f, ms._42 * f, ms._43 * f, ms._44 * f );
    }

    matrix4 operator / ( float f ) const
    {
      float fInv = 1.0f / f;

      return matrix4( ms._11 * fInv, ms._12 * fInv, ms._13 * fInv, ms._14 * fInv,
                      ms._21 * fInv, ms._22 * fInv, ms._23 * fInv, ms._24 * fInv,
                      ms._31 * fInv, ms._32 * fInv, ms._33 * fInv, ms._34 * fInv,
                      ms._41 * fInv, ms._42 * fInv, ms._43 * fInv, ms._44 * fInv );
    }

    friend matrix4 operator * ( float, const matrix4& mat );

    bool operator == ( const matrix4& mat ) const
    {
      return ( 0 == memcmp( &m[0][0], &mat.m[0][0], sizeof( matrix4 ) ) );
    }

    bool operator != ( const matrix4& mat ) const
    {
      return ( 0 != memcmp( &m[0][0], &mat.m[0][0], sizeof( matrix4 ) ) );
    }

	  matrix4& operator=( const matrix4 &other )
	  {
		  for ( int i = 0; i < 16; ++i )
      {
			  M[i] = other.M[i];
      }

		  return *this;
	  }

    math::vector3 TransformCoord( math::vector3& vect ) const
	  {
		  float   vector[3];

      float fInvW = 1.0f / ( m[0][3] * vect.x + m[1][3] * vect.y + m[2][3] * vect.z + m[3][3] );

      vector[0] = ( m[0][0] * vect.x + m[1][0] * vect.y + m[2][0] * vect.z + m[3][0] ) * fInvW;
      vector[1] = ( m[0][1] * vect.x + m[1][1] * vect.y + m[2][1] * vect.z + m[3][1] ) * fInvW;
      vector[2] = ( m[0][2] * vect.x + m[1][2] * vect.y + m[2][2] * vect.z + m[3][2] ) * fInvW;

		  vect.x = vector[0];
		  vect.y = vector[1];
		  vect.z = vector[2];

      return vect;
	  }

    void TransformCoord( const math::vector3& vect, math::vector3& vectOut ) const
	  {
      //float fInvW = 1.0f / ( m[3][0] * vect.x + m[3][1] * vect.y + m[3][2] * vect.z + m[3][3] );

      float fInvW = 1.0f / ( m[0][3] * vect.x + m[1][3] * vect.y + m[2][3] * vect.z + m[3][3] );

      vectOut.x = ( m[0][0] * vect.x + m[1][0] * vect.y + m[2][0] * vect.z + m[3][0] ) * fInvW;
      vectOut.y = ( m[0][1] * vect.x + m[1][1] * vect.y + m[2][1] * vect.z + m[3][1] ) * fInvW;
      vectOut.z = ( m[0][2] * vect.x + m[1][2] * vect.y + m[2][2] * vect.z + m[3][2] ) * fInvW;
      /*
      vectOut.x = ( m[0][0] * vect.x + m[0][1] * vect.y + m[0][2] * vect.z + m[0][3] ) * fInvW;
      vectOut.y = ( m[1][0] * vect.x + m[1][1] * vect.y + m[1][2] * vect.z + m[1][3] ) * fInvW;
      vectOut.z = ( m[2][0] * vect.x + m[2][1] * vect.y + m[2][2] * vect.z + m[2][3] ) * fInvW;
      */
	  }

    math::vector3 TransformNormal( math::vector3& vect ) const
	  {
      float   vector[3];

      vector[0] = m[0][0] * vect.x + m[1][0] * vect.y + m[2][0] * vect.z;
      vector[1] = m[0][1] * vect.x + m[1][1] * vect.y + m[2][1] * vect.z;
      vector[2] = m[0][2] * vect.x + m[1][2] * vect.y + m[2][2] * vect.z;

		  vect.x = vector[0];
		  vect.y = vector[1];
		  vect.z = vector[2];

      return vect;
	  }

  };

};  // namespace math




#endif //_MATRIX4_H_