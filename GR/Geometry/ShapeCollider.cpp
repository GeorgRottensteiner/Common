#include <debug/debugclient.h>

#include "ShapeCollider.h"



namespace GR
{

  namespace Geometry
  {

    bool Collides( const GR::tFPoint& ptPos1, const Shape& Shape1, const GR::tFPoint& ptPos2, const Shape& Shape2 )
    {
      switch ( Shape1.Type() )
      {
        case GR::Geometry::SH_CIRCLE:
          switch ( Shape2.Type() )
          {
            case GR::Geometry::SH_CIRCLE:
              return ( ( ptPos2 - ptPos1 ).length_squared() <= ( Shape1.Radius() + Shape2.Radius() ) * ( Shape1.Radius() + Shape2.Radius() ) );
            case GR::Geometry::SH_RECTANGLE:
              return math::IntersectCircleWithBox2d( ptPos1, Shape1.Radius(), ptPos2 - Shape2.Size() * 0.5f, ptPos2 + Shape2.Size() * 0.5f );
            default:
              dh::Log( "GR::Geometry::Collides Unsupported collision combination %d/%d", Shape1.Type(), Shape2.Type() );
              return false;
          }
          break;
        case GR::Geometry::SH_RECTANGLE:
          switch ( Shape2.Type() )
          {
            case GR::Geometry::SH_CIRCLE:
              return math::IntersectCircleWithBox2d( ptPos2, Shape2.Radius(), ptPos1 - Shape1.Size() * 0.5f, ptPos1 + Shape1.Size() * 0.5f );
            case GR::Geometry::SH_RECTANGLE:
              return GR::tFRect( ptPos1 - Shape1.Size() * 0.5f, ptPos1 + Shape1.Size() * 0.5f ).intersects( 
                GR::tFRect( ptPos2 - Shape2.Size() * 0.5f, ptPos2 + Shape2.Size() * 0.5f ) );
            default:
              dh::Log( "GR::Geometry::Collides Unsupported collision combination %d/%d", Shape1.Type(), Shape2.Type() );
              return false;
          }
          break;
      }
      dh::Log( "GR::Geometry::Collides Unsupported collision combination %d/%d", Shape1.Type(), Shape2.Type() );
      return false;
    }

  };
};

