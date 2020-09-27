#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include "ParticleSystem.h"


namespace Particle
{
  class Manager
  {

    public:

      typedef std::list<System*>  tListSystems;

      tListSystems                m_listSystems;



      Manager();
      ~Manager();

      static Manager&             Instance();

      void                        Clear();
      void                        ClearParticles();

      void                        AddSystem( System* pSystem );

      void                        Display( const GR::tPoint& Offset = GR::tPoint() );
      void                        Update( const float fElapsedTime );

  };

}

#endif // PARTICLE_MANAGER_H