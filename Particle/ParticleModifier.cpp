#include "ParticleModifier.h"
#include "ParticleSystem.h"


namespace Particle
{
  Modifier::Modifier() :
    m_pSystem( NULL ),
    m_fElapsedWindTime( 0.0f )
  {
    SetVarF( "GravityX", 0.0f );
    SetVarF( "GravityY", 20.0f );
    SetVarF( "GravityZ", 0.0f );
    SetVarI( "Type", PMT_GRAVITIY );

    SetVarF( "WindPeakDelay", 5.0f );
    SetVarF( "WindStrength", 15.0f );
    SetVarF( "WindMinStrength", 0.0f );
    SetVarF( "WindX", 0.0f );
    SetVarF( "WindY", 0.0f );
    SetVarF( "WindZ", 0.0f );

    RefreshVars();
  }



  Modifier::~Modifier()
  {
  }



  void Modifier::Update( const float fDeltaT )
  {

    m_fElapsedWindTime += fDeltaT;
    m_fElapsedTime = fDeltaT;

    switch ( m_Type )
    {
      case PMT_WIND:
        {
          if ( m_fElapsedWindTime >= 2 * 3.1415926f )
          {
            m_fElapsedWindTime -= 2 * 3.1415926f;
          }

          float fCurrentWindStrength = m_fWindMinStrength + ( m_fWindStrength - m_fWindMinStrength ) + ( m_fWindStrength - m_fWindMinStrength ) * cos( m_fElapsedWindTime * m_fWindPeakDelay );

          m_vectCurrentWindAcc = m_vectWindDirection * fCurrentWindStrength;
          break;
        }
    }

  }



  void Modifier::Update( Particle* pParticle )
  {
    pParticle->m_Speed += m_vectGravityAcceleration * m_fElapsedTime;

    switch ( m_Type )
    {
      case PMT_WIND:
        pParticle->m_Pos += m_vectCurrentWindAcc;
        break;
      case PMT_FRICTION:
        pParticle->m_Speed *= ( 1.0f - 0.9999f * m_fElapsedTime );
        break;
    }
  }



  void Modifier::RefreshVars()
  {

    m_Type = (eParticleModifierType)GetVarI( "Type" );

    m_vectGravityAcceleration.x = GetVarF( "GravityX" );
    m_vectGravityAcceleration.y = GetVarF( "GravityY" );
    m_vectGravityAcceleration.z = GetVarF( "GravityZ" );

    m_vectWindDirection.x     = GetVarF( "WindX" );
    m_vectWindDirection.y     = GetVarF( "WindY" );
    m_vectWindDirection.z     = GetVarF( "WindZ" );
    m_fWindPeakDelay          = GetVarF( "WindPeakDelay" );
    m_fWindStrength           = GetVarF( "WindStrength" );
    m_fWindMinStrength        = GetVarF( "WindMinStrength" );

  }

}