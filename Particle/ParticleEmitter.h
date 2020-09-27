#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include <GR/LocalRegistry/LocalRegistry.h>

#include "Particle.h"


namespace Particle
{
  class System;

  class Emitter : public LocalRegistry
  {

    public:

      enum eEmitterType
      {
        EMIT_LINEAR = 0,
        EMIT_LINEAR_GAUSSIC,
        EMIT_TIMED_BURST,
      };

      eEmitterType                    m_Type;

      System*                         m_pSystem;

      GR::f32                         m_fLastPartTime,
                                      m_fLinearSpawnDelay,
                                      m_fLifeTime,
                                      m_fSpreadLifeTime;

      bool                            m_bActive;

      GR::i32                         m_iAngleSpread;

      GR::u32                         m_dwColor;

      GR::tVector                     m_Position;
      math::vector3                   m_vectGeneralDirection;

      Particle                        m_particleTemplate;


      Emitter( const GR::tVector& Position = GR::tVector() );
      ~Emitter();


      void                            Update( const float fDeltaT );

      void                            RefreshVars();

      Particle*                       EmitParticle();

      void                            SetActive( bool bActive = true );
      bool                            IsActive() const;


  };

}

#endif // PARTICLE_EMITTER_H