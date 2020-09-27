#ifndef XBOUNDING_BOX_H__
#define XBOUNDING_BOX_H__



#include <Math/matrix4.h>



class XBoundingBox
{

  public:

    enum eIntersectPlane
    {
      IP_NONE = 0,
      IP_XMIN,
      IP_XMAX,
      IP_YMIN,
      IP_YMAX,
      IP_ZMIN,
      IP_ZMAX,
    };


    math::vector3       m_UpperLeftCorner,
                        m_LowerRightCorner,

                        m_RotatedBoundingBox[8];

    bool                m_Transformed;


    XBoundingBox();

    void                Clear();
    void                AddVertex( const math::vector3& vectNew );
    void                Combine( const XBoundingBox& BoundingBox );
    bool                Intersect( const XBoundingBox& BoundingBox ) const;
    bool                IntersectWithRay( const math::vector3& v1, const math::vector3& v2, 
                                          math::vector3& vResultNear, eIntersectPlane& ipNear,
                                          math::vector3& vResultFar,  eIntersectPlane& ipFar ) const;
    bool                IntersectWithRay( const math::vector3& v1, const math::vector3& v2, 
                                          math::vector3& vResultNear, math::vector3& vResultFar ) const;
    bool                IntersectBoxWithRay( const math::vector3& ray_orig,  const math::vector3& ray_dir, 
                                             const math::vector3& box_min,   const math::vector3& box_max,
                                             math::vector3&       res_near,  eIntersectPlane& ipNear,
                                             math::vector3&       res_far,   eIntersectPlane& ipFar ) const;

    void                Transform( const math::matrix4& matTransform );
    void                Offset( const GR::tVector& vectOffset );

    bool                Contains( const GR::tVector& vectPos ) const;

    GR::tVector         Center() const;

    GR::tVector         Size() const;
    float               Width() const;
    float               Height() const;
    float               Depth() const;

    math::vector3       UpperLeftCorner() const;
    math::vector3       LowerRightCorner() const;
    math::vector3       TransformedCorner( size_t iIndex ) const;

    void                Inflate( const GR::f32 fDX, const GR::f32 fDY, const GR::f32 fDZ );

    void                Set( const GR::f32 X, const GR::f32 Y, const GR::f32 Z, const GR::f32 Width, const GR::f32 Height, const GR::f32 Depth );
    void                Set( const GR::tBounds& Bounds );

};


#endif // XBOUNDING_BOX_H__