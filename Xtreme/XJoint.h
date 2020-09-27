#ifndef XTREME_JOINT_H
#define XTREME_JOINT_H

#include <GR/GRTypes.h>

#include <math/matrix4.h>



class XJoint
{

  public:

    GR::tVector     m_vectOffset;
    GR::tVector     m_vectRotation;


    void            Set( const GR::tVector& vectOffset, const GR::tVector& vectRotation = GR::tVector() )
    {
      m_vectOffset    = vectOffset;
      m_vectRotation  = vectRotation;
    }

    math::matrix4   Matrix() const
    {
      math::matrix4   matResult;

      matResult.RotationYawPitchRoll( m_vectRotation.x, m_vectRotation.y, m_vectRotation.z );
      matResult = matResult * math::matrix4().Translation( m_vectOffset );
      
      return matResult;
    }

};


#endif // XTREME_JOINT_H