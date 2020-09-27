#include "ParticleEmitter.h"
#include "ParticleSystem.h"

#include <math/random.h>


namespace Particle
{

  Emitter::Emitter( const GR::tVector& Position ) :
    m_fLastPartTime( 0.0f ),
    m_pSystem( NULL ),
    m_bActive( true ),
    m_Position( Position )
  {
    SetVarI( "Type", EMIT_LINEAR_GAUSSIC );
    SetVarF( "GenDirX", 0.0f );
    SetVarF( "GenDirY", -40.0f );
    SetVarF( "GenDirZ", 0.0f );
    SetVarI( "SpreadAngle", 20 );
    SetVarF( "SpawnDelay", 0.25f );
    SetVarF( "LifeTime", 2.5f );
    SetVarF( "SpreadLifeTime", 0.5f );
    SetVarI( "Color", 0xffffffff );

    SetVarF( "PosX", m_Position.x );
    SetVarF( "PosY", m_Position.y );
    SetVarF( "PosZ", m_Position.z );

    RefreshVars();
  }



  Emitter::~Emitter()
  {
  }



  void Emitter::Update( const float fDeltaT )
  {

    if ( !m_bActive )
    {
      return;
    }

    switch ( m_Type )
    {
      case EMIT_LINEAR:
        if ( m_pSystem )
        {
          float   fTempDelta = m_fLastPartTime + fDeltaT;

          while ( fTempDelta >= m_fLinearSpawnDelay )
          {
            EmitParticle();
            fTempDelta -= m_fLinearSpawnDelay;
          }
          m_fLastPartTime = fTempDelta;
        }
        break;
      case EMIT_LINEAR_GAUSSIC:
        if ( ( m_pSystem )
        &&   ( m_fLinearSpawnDelay > 0.0f ) )
        {
          float   fTempDelta = m_fLastPartTime + fDeltaT;

          while ( fTempDelta >= m_fLinearSpawnDelay )
          {
            EmitParticle();

            fTempDelta -= m_fLinearSpawnDelay;
          }
          m_fLastPartTime = fTempDelta;
        }
        break;
    }

  }



  void Emitter::RefreshVars()
  {

    m_Type = (eEmitterType)GetVarI( "Type" );

    m_Position.x = GetVarF( "PosX" );
    m_Position.y = GetVarF( "PosY" );
    m_Position.z = GetVarF( "PosZ" );

    m_vectGeneralDirection.x = GetVarF( "GenDirX" );
    m_vectGeneralDirection.y = GetVarF( "GenDirY" );
    m_vectGeneralDirection.z = GetVarF( "GenDirZ" );

    m_iAngleSpread = (GR::i32)GetVarI( "SpreadAngle" );

    m_fLinearSpawnDelay = GetVarF( "SpawnDelay" );

    m_fLifeTime       = GetVarF( "LifeTime" );
    m_fSpreadLifeTime = GetVarF( "SpreadLifeTime" );

    m_dwColor = (GR::u32)GetVarI( "Color" );

  }



  Particle* Emitter::EmitParticle()
  {
    Particle*    pParticle = new Particle( m_particleTemplate );

    math::vector3   vectDir = m_vectGeneralDirection;

    float   fAngle = atan2f( vectDir.y, vectDir.x ) * 180.0f / 3.1415926f;
    float   fLength = vectDir.length();

    int     iDelta = 0;

    pParticle->m_Pos = m_Position;

    if ( m_iAngleSpread )
    {
      /*
      for ( int i = 0; i < 8; ++i )
      {
        iDelta += rand() % m_iAngleSpread;
      }
      iDelta /= 8;
      */

      iDelta = rand() % m_iAngleSpread;
    }

    fAngle += m_iAngleSpread / 2 - iDelta;

    fAngle *= 3.1415926f / 180.0f;

    vectDir.x = fLength * cosf( fAngle );
    vectDir.y = fLength * sinf( fAngle );

    pParticle->m_Speed    = vectDir;
    pParticle->m_Color    = m_dwColor;
    pParticle->m_LifeTime = m_fLifeTime + m_fSpreadLifeTime * ( rand() % 100 ) * 0.01f;

    m_pSystem->AddParticle( pParticle );

    return pParticle;
  }



  void Emitter::SetActive( bool bActive )
  {
    m_bActive = bActive;
  }



  bool Emitter::IsActive() const
  {
    return m_bActive;
  }

}
