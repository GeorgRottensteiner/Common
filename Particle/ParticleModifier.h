#ifndef PARTICLE_MODIFIER_H
#define PARTICLE_MODIFIER_H

#include <GR/LocalRegistry/LocalRegistry.h>

#include "Particle.h"



namespace Particle
{
  class System;

  class Modifier : public LocalRegistry
  {

    public:

      enum eParticleModifierType
      {
        PMT_GRAVITIY = 0,
        PMT_WIND,
        PMT_FRICTION,
      };

      eParticleModifierType           m_Type;

      System*                         m_pSystem;

      float                           m_fElapsedTime,
                                      m_fElapsedWindTime,
                                      m_fWindPeakDelay,
                                      m_fWindStrength,
                                      m_fWindMinStrength;

      math::vector3                   m_vectGravityAcceleration,
                                      m_vectWindDirection,
                                      m_vectCurrentWindAcc;


      Modifier();
      virtual ~Modifier();

      virtual void                    Update( const float fDeltaT );
      virtual void                    Update( Particle* pParticle );

      void                            RefreshVars();

  };
}

#endif // PARTICLE_MODIFIER_H