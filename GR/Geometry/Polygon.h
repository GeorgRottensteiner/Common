#ifndef GR_GEOMETRY_POLYGON2D
#define GR_GEOMETRY_POLYGON2D

#include <vector>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Geometry
  {

    class Polygon2d
    {

      public:

        std::vector<GR::tFPoint>        m_vectPoints;


        bool                            Intersects( const Polygon2d& OtherPoly );

        bool                            IsPointInside( const GR::tFPoint& ptPos ) const;

    };

  };
};


#endif // GR_GEOMETRY_POLYGON2D