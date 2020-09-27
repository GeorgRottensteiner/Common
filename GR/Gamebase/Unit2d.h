#ifndef GR_GAMEBASE_UNIT2D_H
#define GR_GAMEBASE_UNIT2D_H

#include <Xtreme/XOrientation.h>

#include <GR/GRTypes.h>
#include <GR/Geometry/GeometryTypes.h>



namespace GR
{

  namespace Gamebase
  {

    class Unit2d : public XOrientation
    {

      public:

        enum eUnitEvent
        {
          UE_LIFETIME_DONE,
          UE_DIE,

          UE_LAST_ENTRY,
        };

        GR::tFPoint           m_Speed;

        bool                  m_RemoveMe;

        GR::f32               m_LifeTime;

        GR::Geometry::eShape  m_Shape;

        GR::tFPoint           m_Size;

        GR::f32               m_Radius;
        GR::f32               m_Angle;


        Unit2d();
        virtual ~Unit2d();


        void                  Update( const GR::f32 fElapsedTime );

        virtual void          UnitEvent( GR::u32 Event );

        GR::tFPoint           GetPosition2d() const;

    };

  }
}


#endif // GR_GAMEBASE_UNIT2D_H