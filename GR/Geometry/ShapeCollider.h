#ifndef GR_GEOMETRY_SHAPE_COLLIDER_H
#define GR_GEOMETRY_SHAPE_COLLIDER_H


#include <math/MathUtil.h>

#include <GR/GRTypes.h>

#include "GeometryTypes.h"
#include "Shape.h"



namespace GR
{

  namespace Geometry
  {

    bool Collides( const GR::tFPoint& ptPos1, const Shape& Shape1, const GR::tFPoint& ptPos2, const Shape& Shape2 );

  };
};

#endif // GR_GEOMETRY_SHAPE_COLLIDER_H