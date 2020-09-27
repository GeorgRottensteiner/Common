#ifndef GR_GEOMETRY_HierarchicalShape_H
#define GR_GEOMETRY_HierarchicalShape_H



#include <GR/GRTypes.h>

#include "GeometryTypes.h"
#include "Shape.h"

#include <vector>
#include <list>



namespace GR
{

  namespace Geometry
  {

    class HierarchicalShape : public Shape
    {

      private:

        std::list<HierarchicalShape>    m_ChildShapes;


      public:


        HierarchicalShape( eShape AShape = SH_UNKNOWN );
        HierarchicalShape( GR::f32 Width, GR::f32 Height );
        HierarchicalShape( GR::f32 Radius );

    };

  };
};

#endif // GR_GEOMETRY_HierarchicalShape_H