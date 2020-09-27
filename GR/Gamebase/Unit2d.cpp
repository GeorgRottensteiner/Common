#include ".\unit2d.h"



namespace GR
{

  namespace Gamebase
  {



    Unit2d::Unit2d() :
      m_RemoveMe( false ),
      m_LifeTime( 0.0f ),
      m_Radius( 1.0f ),
      m_Angle( 0.0f ),
      m_Size( 20, 20 )
    {
    }



    Unit2d::~Unit2d()
    {
    }



    void Unit2d::Update( const GR::f32 ElapsedTime )
    {
      if ( m_LifeTime > 0.0f )
      {
        m_LifeTime -= ElapsedTime;
        if ( m_LifeTime <= 0.0f )
        {
          // TODO - Event?
          m_RemoveMe = true;
        }
      }
    }



    void Unit2d::UnitEvent( GR::u32 Event )
    {
      switch ( Event )
      {
        case UE_LIFETIME_DONE:
          UnitEvent( UE_DIE );
          break;
        case UE_DIE:
          m_RemoveMe = true;
          break;
      }
    }



    GR::tFPoint Unit2d::GetPosition2d() const
    {
      return GR::tFPoint( m_Position.x, m_Position.y );
    }

  }

}