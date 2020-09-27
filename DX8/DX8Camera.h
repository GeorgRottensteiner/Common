#ifndef _DX8_CAMERA_H_
#define _DX8_CAMERA_H_
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

#include <D3DX8.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CDX8Camera
{
  public:

    enum ProjectionType
    {
      PerspectiveFOVLH = 0,
      OrthoLH,
      PerspectiveFOVRH,
      OrthoRH
    };

    CDX8Camera();
    virtual ~CDX8Camera();

    void Pitch( float fDelta );
    void Yaw( float fDelta );
    void Roll( float fDelta );

    /*
    inline float Pitch() { return m_fPitch; }
    inline float Yaw() { return m_fYaw; }
    inline float Roll() { return m_fRoll; }
    void RetrieveYPR();
    */

    void SetX( const float fX );
    void SetY( const float fY );
    void SetZ( const float fZ );
    void MovePosition( const D3DXVECTOR3& vectPos );
    void SetPosition( const D3DXVECTOR3& vectPos );
    void SetPosition( const float fX, const float fY, const float fZ );

    void SetLookAtPoint( const D3DXVECTOR3& vectPos );
    void SetLookAtPoint( float fX, float fY, float fZ );

    void SetLookUp( const D3DXVECTOR3& vectPos );
    void SetLookUp( float fX, float fY, float fZ );

    void SetDirection( const D3DXVECTOR3& vectPos );
    void SetLookAtAndDirection( const D3DXVECTOR3& vectAt, const D3DXVECTOR3& vectDir );
    void SetValues( const D3DXVECTOR3& vectPos, const D3DXVECTOR3& vectLookAt, const D3DXVECTOR3& vectLookUp );

    D3DXVECTOR3 GetPosition() const;
    D3DXVECTOR3 GetDirection() const;
    D3DXVECTOR3 GetLookUp() const;
    D3DXVECTOR3 GetCurrentPosition() const;   // für lerpende Kameras
    D3DXVECTOR3 GetCurrentDirection() const;  // für lerpende Kameras
    D3DXVECTOR3 GetCurrentLookUp() const;     // für lerpende Kameras

    D3DXVECTOR3             GetLookAt() const;

    void SetScaling( float fX, float fY, float fZ );
    void SetScaling( float fFaktor );
    void SetScalingX( float fX );
    void SetScalingY( float fX );
    void SetScalingZ( float fX );
    inline float GetScalingX() { return m_fScaleX; }
    inline float GetScalingY() { return m_fScaleY; }
    inline float GetScalingZ() { return m_fScaleZ; }

    void SetProjectionType( ProjectionType dwProjectionType );
    ProjectionType        GetProjectionType();

    inline void ViewFOV( float fFOV ) { m_fFOV = fFOV; };

    inline void ViewPos( int iX, int iY ) { m_Viewport.X = iX; m_Viewport.Y = iY; };

    void          ViewOrthoLH( float fWidth, float fHeight, float fZMin, float fZMax )
    {
      m_fViewWidth    = fWidth;
      m_fViewHeight   = fHeight;
      m_fViewZNear    = fZMin;
      m_fViewZFar     = fZMax;
      SetProjectionType( OrthoLH );
    }
    inline void ViewWidth( int iWidth ) 
    { 
      m_Viewport.Width  = iWidth; 
      m_fViewWidth      = (float)iWidth;
      SetProjectionType( m_projType );
    };
    inline void ViewHeight( int iHeight ) 
    { 
      m_Viewport.Height = iHeight; 
      m_fViewHeight     = (float)iHeight;
      SetProjectionType( m_projType );
    };
    inline void ViewZMin( float fZNear ) 
    { 
      m_Viewport.MinZ = fZNear; 
      SetProjectionType( m_projType );
    };
    inline void ViewZMax( float fZFar ) 
    { 
      m_Viewport.MaxZ = fZFar; 
      SetProjectionType( m_projType );
    };
    inline void ZNear( float fZNear ) 
    { 
      m_fViewZNear = fZNear;
      SetProjectionType( m_projType );
    };
    inline void ZFar( float fZFar ) 
    { 
      m_fViewZFar = fZFar;
      SetProjectionType( m_projType );
    };


    D3DXMATRIX GetViewMatrix();
    D3DXMATRIX GetProjectionMatrix();
    D3DVIEWPORT8* GetViewport();

    void Dump();


    bool              Lerping() const
    {
      return m_bLerping;
    }
    void              Lerping( float fLerpFaktor, bool bLerp = true )
    {
      m_bLerping    = bLerp;
      m_fLerpFaktor = fLerpFaktor;

      m_vectCurrentPosition   = m_vectPosition;
      m_vectCurrentDirection  = m_vectDirection;
      m_vectCurrentLookUp     = m_vectLookUp;
    }

    D3DXMATRIX        UpdateLerp( float fZeitFaktor );   // in Sekunden

    void              ForceDistance( bool bForce, float fDistance );    // fürs Lerpen, Abstand erzwingen
    void              ForceYPos( bool bForce, float fY );    // fürs Lerpen, Y-Position erzwingen


  protected:

    D3DVIEWPORT8      m_Viewport;

    D3DXVECTOR3       m_vectPosition,
                      m_vectLookAt,
                      m_vectLookUp,
                      m_vectDirection,

                      m_vectCurrentPosition,
                      m_vectCurrentDirection,
                      m_vectCurrentLookUp;

    D3DXMATRIX        m_matOrientation,
                      m_viewMatrix,
                      m_projMatrix,
                      m_scaleMatrix;

    bool              m_bLerping,
                      m_bForceDistance,
                      m_bForceYPos;

    float             m_fScaleX,
                      m_fScaleY,
                      m_fScaleZ,
                      m_fFOV,

                      m_fViewWidth,
                      m_fViewHeight,
                      m_fViewZNear,
                      m_fViewZFar,

                      m_fLerpFaktor,
                      m_fForcedDistance,
                      m_fForcedYPos;

    ProjectionType    m_projType;


    void NormalizeAngle( float& fAngle );
    void UpdateMatrix();


};




#endif // _DX8_CAMERA_H_



