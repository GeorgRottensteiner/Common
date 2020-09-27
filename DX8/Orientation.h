#ifndef _ORIENTATION_H_
#define _ORIENTATION_H_
/*----------------------------------------------------------------------------+
 | Programmname       : Orientation                                           |
 |                      Helper-Klasse für nicht frei rotierbare Ansicht       |
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

class COrientation
{
  protected:

    D3DXVECTOR3   m_vectPosition,
                  m_vectLookAt,
                  m_vectLookUp,
                  m_vectDirection;

    D3DXMATRIX    m_matOrientation;


    void UpdateMatrix();

  public:

    COrientation();
    virtual ~COrientation();

    void Pitch( float fDelta );
    void Yaw( float fDelta );
    void Roll( float fDelta );

    void PitchAbsolute( float fDelta );
    void YawAbsolute( float fDelta );
    void RollAbsolute( float fDelta );

    void                  RotateAboutAxis( D3DXVECTOR3& vectAxis, float fDelta );

    void Advance( float fLength );
    void Strafe( float fLength );
    void Raise( float fLength );

    D3DXVECTOR3 CalcAdvanceVector( float fLength );
    D3DXVECTOR3 CalcStrafeVector( float fLength );
    D3DXVECTOR3 CalcRaiseVector( float fLength );

    void SetX( const float& fX );
    void SetY( const float& fY );
    void SetZ( const float& fZ );

    void                MovePosition( const D3DXVECTOR3& vectPos );
    void                MovePosition( const float fX, const float fY, const float fZ );
    void                SetPosition( const D3DXVECTOR3& vectPos );
    void                SetPosition( const float fX, const float fY, const float fZ );
    void                SetLookAtPoint( const D3DXVECTOR3& vectPos );
    void                SetLookAtPoint( const float fX, const float fY, const float fZ );
    void                SetDirection( const D3DXVECTOR3& vectDir );
    void                SetLookUp( const D3DXVECTOR3& vectLUp );

    void                SetValues( const D3DXVECTOR3& vectPos, const D3DXVECTOR3& vectLookAt, const D3DXVECTOR3& vectLookUp );

    void                SlideToLookAtPoint( const D3DXVECTOR3& vectPos );
    
    float               GetPositionX() const;
    float               GetPositionY() const;
    float               GetPositionZ() const;

    D3DXVECTOR3 GetPosition() const;
    D3DXVECTOR3 GetLookAtPoint() const;
    D3DXVECTOR3 GetDirection() const;
    D3DXVECTOR3 GetLookUp() const;

    virtual D3DXMATRIX GetMatrix() const;
    virtual D3DXMATRIX GetMatrixRH() const;
    virtual D3DXMATRIX GetViewMatrix() const;
    virtual D3DXMATRIX GetViewMatrixRH() const;

    void              ToEulerAngles( float& fYaw, float& fPitch, float& fRoll );

    void Dump();
};




#endif // _ORIENTATION_H_



