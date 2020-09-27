#ifndef PARTICLE_H
#define PARTICLE_H

#include <GR/GRTypes.h>
#include <Math/vector3.h>



namespace Particle
{

  class Particle
  {

    public:

      GR::u32           m_Type;
      GR::u32           m_Color;
      float             m_LifeTime;
      math::vector3     m_Pos,
                        m_Speed;

      Particle() :
        m_Type( 0 ),
        m_Color( 0xffffffff ),
        m_LifeTime( 0.0f ),
        m_Pos( 0, 0, 0 ),
        m_Speed( 0, 0, 0 )
      {
      }



      virtual ~Particle()
      {
      }



      virtual bool Update( const float ElapsedTime )
      {
        if ( m_LifeTime > 0.0f )
        {
          if ( ElapsedTime >= m_LifeTime )
          {
            return true;
          }
          m_LifeTime -= ElapsedTime;
        }
        m_Pos += m_Speed * ElapsedTime;
        return false;
      }

  };

}

#endif // PARTICLE_H