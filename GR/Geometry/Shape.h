#ifndef GR_GEOMETRY_SHAPE_H
#define GR_GEOMETRY_SHAPE_H


#include <vector>

#include <GR/GRTypes.h>

#include "GeometryTypes.h"



namespace GR
{

  namespace Geometry
  {

    class Shape
    {

      private:

        eShape                      m_Type;

        GR::tFPoint                 m_ptSize;
        std::vector<GR::tFPoint>    m_vectPoints;


      public:


        Shape( eShape aShape = SH_UNKNOWN );
        Shape( GR::f32 Width, GR::f32 Height );
        Shape( GR::f32 Radius );

        eShape                      Type() const;

        const GR::tFPoint&          Size() const;
        GR::f32                     Radius() const;

        const GR::tFPoint&          Vertex( GR::up dwIndex ) const;

    };

  };
};

#endif // GR_GEOMETRY_SHAPE_H