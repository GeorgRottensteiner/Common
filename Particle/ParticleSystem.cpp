#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleModifier.h"



namespace Particle
{
  System::System()
  {
  }



  System::~System()
  {
    Clear();
  }



  void System::Clear()
  {

    tListParticles::iterator    it( m_ListParticles.begin() );
    while ( it != m_ListParticles.end() )
    {
      Particle* pParticle = *it;

      delete pParticle;

      ++it;
    }
    m_ListParticles.clear();

    tListEmitter::iterator    it2( m_ListEmitter.begin() );
    while ( it2 != m_ListEmitter.end() )
    {
      delete *it2;

      ++it2;
    }
    m_ListEmitter.clear();

    tListModifier::iterator    itModifier( m_ListModifier.begin() );
    while ( itModifier != m_ListModifier.end() )
    {
      delete *itModifier;

      ++itModifier;
    }
    m_ListModifier.clear();

  }



  void System::ClearParticles()
  {
    tListParticles::iterator    it( m_ListParticles.begin() );
    while ( it != m_ListParticles.end() )
    {
      delete *it;

      ++it;
    }
    m_ListParticles.clear();
  }



  void System::Update( const float fDeltaT )
  {
    tListEmitter::iterator    itEmitter( m_ListEmitter.begin() );
    while ( itEmitter != m_ListEmitter.end() )
    {
      Emitter*   pEmitter = *itEmitter;

      pEmitter->Update( fDeltaT );

      ++itEmitter;
    }

    tListModifier::iterator   itModifier( m_ListModifier.begin() );
    while ( itModifier != m_ListModifier.end() )
    {
      Modifier*    pModifier = *itModifier;

      pModifier->Update( fDeltaT );

      tListParticles::iterator    itParticle( m_ListParticles.begin() );
      while ( itParticle != m_ListParticles.end() )
      {
        Particle*   pParticle = *itParticle;

        pModifier->Update( pParticle );
        ++itParticle;
      }

      ++itModifier;
    }

    tListParticles::iterator    itParticle( m_ListParticles.begin() );
    while ( itParticle != m_ListParticles.end() )
    {
      Particle*   pParticle = *itParticle;

      if ( pParticle->Update( fDeltaT ) )
      {
        itParticle = m_ListParticles.erase( itParticle );
        delete pParticle;
      }
      else
      {
        ++itParticle;
      }
    }
  }



  void System::AddEmitter( Emitter* pEmitter )
  {
    m_ListEmitter.push_back( pEmitter );
    pEmitter->m_pSystem = this;
  }



  void System::RemoveEmitter( Emitter* pEmitter )
  {
    pEmitter->m_pSystem = NULL;
    m_ListEmitter.remove( pEmitter );
  }



  void System::AddModifier( Modifier* pModifier )
  {
    m_ListModifier.push_back( pModifier );
  }



  void System::AddParticle( Particle* pParticle )
  {
    m_ListParticles.push_back( pParticle );
  }

}
