#include "./HierarchicalShape.h"


namespace GR
{
  namespace Geometry
  {

    HierarchicalShape::HierarchicalShape( eShape aHierarchicalShape ) :
      Shape( aHierarchicalShape )
    {
    }

    HierarchicalShape::HierarchicalShape( GR::f32 Width, GR::f32 Height ) :
      Shape( Width, Height )
    {
    }

    HierarchicalShape::HierarchicalShape( GR::f32 Radius ) :
      Shape( Radius )
    {
    }
  };
};
