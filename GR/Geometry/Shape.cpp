#include "./Shape.h"


namespace GR
{
  namespace Geometry
  {

    Shape::Shape( eShape aShape ) :
      m_Type( aShape )
    {
    }

    Shape::Shape( GR::f32 Width, GR::f32 Height ) :
      m_ptSize( Width, Height ),
      m_Type( SH_RECTANGLE )
    {
    }

    Shape::Shape( GR::f32 Radius ) :
      m_ptSize( Radius, Radius ),
      m_Type( SH_CIRCLE )
    {
    }

    eShape Shape::Type() const
    {
      return m_Type;
    }

    const GR::tFPoint& Shape::Size() const
    {
      return m_ptSize;
    }

    GR::f32 Shape::Radius() const
    {
      return m_ptSize.x;
    }

    const GR::tFPoint& Shape::Vertex( GR::up dwIndex ) const
    {
      if ( dwIndex >= m_vectPoints.size() )
      {
        static GR::tFPoint    vectTemp;
        return vectTemp;
      }
      return m_vectPoints[dwIndex];
    }

  };
};
