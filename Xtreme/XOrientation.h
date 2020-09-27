#ifndef XORIENTATION_H_
#define XORIENTATION_H_



#include <GR/GRTypes.h>

#include <math/matrix4.h>



class XOrientation
{
  protected:

    GR::tVector     m_Position,
                    m_LookAt,
                    m_LookUp,
                    m_Direction,
                    m_Scale;

    math::matrix4   m_Orientation;


    void UpdateMatrix();


  public:

    XOrientation();
    virtual ~XOrientation();

    void Pitch( float fDelta );
    void Yaw( float fDelta );
    void Roll( float fDelta );

    void                  RotateX( float fDelta );
    void                  RotateY( float fDelta );
    void                  RotateZ( float fDelta );

    void PitchAbsolute( float fDelta );
    void YawAbsolute( float fDelta );
    void RollAbsolute( float fDelta );

    void                  RotateAboutAxis( GR::tVector& vectAxis, float fDelta );

    void Advance( float fLength );
    void Strafe( float fLength );
    void Raise( float fLength );

    GR::tVector         CalcAdvanceVector( float fLength ) const;
    GR::tVector         CalcStrafeVector( float fLength ) const;
    GR::tVector         CalcRaiseVector( float fLength ) const;

    void SetX( const float& fX );
    void SetY( const float& fY );
    void SetZ( const float& fZ );

    void                MovePosition( const GR::tVector& vectPos );
    void                MovePosition( const float fX, const float fY, const float fZ );
    void                SetPosition( const GR::tVector& vectPos );
    void                SetPosition( const float fX, const float fY, const float fZ );
    void                SetLookAtPoint( const GR::tVector& vectPos );
    void                SetLookAtPoint( const float fX, const float fY, const float fZ );
    void                SetDirection( const GR::tVector& vectDir );
    void                SetLookUp( const GR::tVector& vectLUp );
    void                SetScale( const GR::tVector& vectScale );
    void                SetScale( const float fX, const float fY, const float fZ );

    void                SetValues( const GR::tVector& vectPos, const GR::tVector& vectLookAt, const GR::tVector& vectLookUp );

    void                SlideToLookAtPoint( const GR::tVector& vectPos );
    
    float               GetPositionX() const;
    float               GetPositionY() const;
    float               GetPositionZ() const;

    GR::tVector GetPosition() const;
    GR::tVector GetLookAtPoint() const;
    GR::tVector GetDirection() const;
    GR::tVector GetLookUp() const;

    virtual math::matrix4 GetMatrix() const;
    virtual math::matrix4 GetMatrixRH() const;
    virtual math::matrix4 GetViewMatrix() const;
    virtual math::matrix4 GetViewMatrixRH() const;

    void              ToEulerAngles( float& fYaw, float& fPitch, float& fRoll );

    void              ResetToDefault();

    void              TransformCoordToWorld( GR::tVector& vectPos ) const;
    void              TransformCoord( const XOrientation& Orientation, GR::tVector& vectPos ) const;

    void Dump();
};




#endif // XORIENTATION_H_



