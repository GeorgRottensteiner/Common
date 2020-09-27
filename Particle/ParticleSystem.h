#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <GR/GRTypes.h>
#include <Math/vector3.h>

#include "Particle.h"

#include <list>


namespace Particle
{
  class Emitter;
  class Modifier;

  class System
  {

    public:

      typedef std::list<Particle*>    tListParticles;

      typedef std::list<Emitter*>     tListEmitter;

      typedef std::list<Modifier*>    tListModifier;

      tListParticles                  m_ListParticles;

      tListEmitter                    m_ListEmitter;

      tListModifier                   m_ListModifier;


      System();
      virtual ~System();

      void                            Clear();
      void                            ClearParticles();

      virtual void                    Display( const GR::tPoint& Offset = GR::tPoint() ) = 0;
      void                            Update( const float fDeltaT );

      void                            AddEmitter( Emitter* pEmitter );
      void                            RemoveEmitter( Emitter* pEmitter );

      void                            AddModifier( Modifier* pModifier );
      void                            AddParticle( Particle* pParticle );

  };


}


#endif // PARTICLE_SYSTEM_H