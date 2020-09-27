#ifndef XFRUSTUM_H
#define XFRUSTUM_H

#include <math/plane.h>

#include "XBoundingBox.h"
#include "XViewport.h"



class XFrustum  
{

  public:

    enum eCorner
    {
      C_LO_NEAR,
      C_LO_FAR,
      C_RO_NEAR,
      C_RO_FAR,
      C_LU_NEAR,
      C_LU_FAR,
      C_RU_NEAR,
      C_RU_FAR,

      C_CORNER_COUNT,
    };



	  XFrustum();
	  virtual ~XFrustum();

    math::plane     PlaneLeft,
                    PlaneRight,
                    PlaneTop,
                    PlaneBottom,
                    PlaneZNear,
                    PlaneZFar;

    GR::tVector     Corners[C_CORNER_COUNT];


    void            Create( math::matrix4& Projection, math::matrix4& View, XViewport& ViewPort, float ExtraBorder = 0.0f );

    // erwartet eine Axis-Aligned (nicht rotierte) Bounding Box
    bool            IntersectWithBoundingBox( const XBoundingBox& BBox ) const;
    bool            IntersectWithBoundingBox( GR::tVector& UpperLeft, GR::tVector& LowerRight ) const;

    // erhält die 8 Eckpunkte einer rotierten (orientierten) Bounding Box
    bool            IntersectWithBoundingBoxRotated( const XBoundingBox& BBox ) const;
    bool            IntersectWithRotatedBox( const GR::tVector& V1, const GR::tVector& V2,
                                             const GR::tVector& V3, const GR::tVector& V4,
                                             const GR::tVector& V5, const GR::tVector& V6,
                                             const GR::tVector& V7, const GR::tVector& V8 ) const;
    bool            IsPointInside( const GR::tVector& Point ) const;
};

#endif // XFRUSTUM_H
