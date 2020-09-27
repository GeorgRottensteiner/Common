#include "ParticleManager.h"



namespace Particle
{

  Manager::Manager()
  {
  }



  Manager::~Manager()
  {
    Clear();
  }



  Manager& Manager::Instance()
  {
    static Manager   g_Instance;

    return g_Instance;
  }



  void Manager::Clear()
  {
    tListSystems::iterator    it( m_listSystems.begin() );
    while ( it != m_listSystems.end() )
    {
      System*    pSystem = *it;

      delete pSystem;

      ++it;
    }
    m_listSystems.clear();
  }



  void Manager::ClearParticles()
  {
    tListSystems::iterator    it( m_listSystems.begin() );
    while ( it != m_listSystems.end() )
    {
      System*    pSystem = *it;

      pSystem->ClearParticles();
      ++it;
    }
  }



  void Manager::AddSystem( System* pSystem )
  {
    if ( pSystem )
    {
      m_listSystems.push_back( pSystem );
    }
  }



  void Manager::Update( const float fElapsedTime )
  {
    tListSystems::iterator    it( m_listSystems.begin() );
    while ( it != m_listSystems.end() )
    {
      System*   pSystem = *it;

      pSystem->Update( fElapsedTime );

      ++it;
    }
  }



  void Manager::Display( const GR::tPoint& Offset )
  {
    tListSystems::iterator    it( m_listSystems.begin() );
    while ( it != m_listSystems.end() )
    {
      System*   pSystem = *it;

      pSystem->Display( Offset );

      ++it;
    }
  }

}