#ifndef XCAMERA_H_
#define XCAMERA_H_



#include "XRenderer.h"



class XCamera
{
  public:

    enum ProjectionType
    {
      PerspectiveFOVLH = 0,
      OrthoLH,
      PerspectiveFOVRH,
      OrthoRH
    };

    XCamera();
    virtual ~XCamera();

    void                    Pitch( float Delta );
    void                    Yaw( float Delta );
    void                    Roll( float Delta );

    void                    RotateAboutAxis( const GR::tVector& Axis, const GR::f32 Delta );

    /*
    inline float Pitch() { return m_fPitch; }
    inline float Yaw() { return m_fYaw; }
    inline float Roll() { return m_fRoll; }
    void RetrieveYPR();
    */

    void                    SetX( const float X );
    void                    SetY( const float Y );
    void                    SetZ( const float Z );
    void                    MovePosition( const GR::tVector& Pos );
    void                    SetPosition( const GR::tVector& Pos );
    void                    SetPosition( const float X, const float Y, const float Z );

    void                    Advance( float Length );
    void                    Strafe( float Length );
    void                    Raise( float Length );

    void                    SetLookAtPoint( const GR::tVector& Pos );
    void                    SetLookAtPoint( float X, float Y, float Z );

    void                    SetLookUp( const GR::tVector& Pos );
    void                    SetLookUp( float X, float Y, float Z );

    void                    SetDirection( const GR::tVector& Pos );
    void                    SetLookAtAndDirection( const GR::tVector& At, const GR::tVector& Dir );
    void                    SetValues( const GR::tVector& Pos, const GR::tVector& LookAt, const GR::tVector& LookUp );

    GR::tVector             GetPosition() const;
    GR::tVector             GetDirection() const;
    GR::tVector             GetLookUp() const;
    GR::tVector             GetCurrentPosition() const;   // für lerpende Kameras
    GR::tVector             GetCurrentDirection() const;  // für lerpende Kameras
    GR::tVector             GetCurrentLookUp() const;     // für lerpende Kameras


    GR::tVector             GetLookAt() const;

    GR::tVector             CalcAdvanceVector( GR::f32 Length ) const;
    GR::tVector             CalcStrafeVector( GR::f32 Length ) const;
    GR::tVector             CalcRaiseVector( GR::f32 Length ) const;

    void                    SetScaling( float X, float Y, float Z );
    void                    SetScaling( float Factor );
    void                    SetScalingX( float ScaleFactor );
    void                    SetScalingY( float ScaleFactor );
    void                    SetScalingZ( float ScaleFactor );
    inline float            GetScalingX();
    inline float            GetScalingY();
    inline float            GetScalingZ();

    void                    SetProjectionType( ProjectionType ProjectionType );
    ProjectionType          GetProjectionType();



    void                    ViewFOV( float OV );
    void                    ViewPos( int X, int Y );
    void                    ViewOrthoLH( float Width, float Height, float ZMin, float ZMax );
    void                    ViewWidth( int Width );
    void                    ViewHeight( int Height );
    void                    ViewZMin( float ZNear );
    void                    ViewZMax( float ZFar );

    void                    ZNear( float ZNear );
    void                    ZFar( float ZFar );

    math::matrix4           GetViewMatrix();
    math::matrix4           GetProjectionMatrix();
    XViewport*              GetViewport();

    void                    Dump();


    bool                    Lerping() const;
    void                    Lerping( float LerpFaktor, bool Lerp = true );

    math::matrix4           UpdateLerp( float ElapsedSeconds );   // in Sekunden

    void                    ForceDistance( bool Force, float Distance );    // fürs Lerpen, Abstand erzwingen
    void                    ForceYPos( bool Force, float Y );    // fürs Lerpen, Y-Position erzwingen


  protected:

    XViewport               m_ViewPort;

    GR::tVector             m_Position,
                            m_LookAt,
                            m_LookUp,
                            m_Direction,

                            m_CurrentPosition,
                            m_CurrentDirection,
                            m_CurrentLookUp;

    math::matrix4           m_Orientation,
                            m_ViewMatrix,
                            m_ProjMatrix,
                            m_ScaleMatrix;

    bool                    m_Lerping,
                            m_ForceDistance,
                            m_ForceYPos;

    float                   m_ScaleX,
                            m_ScaleY,
                            m_ScaleZ,
                            m_FOV,

                            m_ViewWidth,
                            m_ViewHeight,
                            m_ViewZNear,
                            m_ViewZFar,

                            m_LerpFaktor,
                            m_ForcedDistance,
                            m_ForcedYPos;

    ProjectionType          m_ProjType;


    void                    NormalizeAngle( float& Angle );
    void                    UpdateMatrix();


};




#endif // XCAMERA_H_



