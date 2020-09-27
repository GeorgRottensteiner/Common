#ifndef __BOUNDING_BOX_H__
#define __BOUNDING_BOX_H__
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <math/matrix4.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CBoundingBox
{
  public:

    math::vector3       m_UpperLeftCorner,
                        m_LowerRightCorner,

                        m_RotatedBoundingBox[8];

    bool                m_bTransformed;


    CBoundingBox();

    void                Clear();
    void                ErweitereUmVertex( math::vector3& vectNew );
    void                Combine( const CBoundingBox& BoundingBox );
    bool                IntersectWithRay( const math::vector3& v1, const math::vector3& v2, 
                                          math::vector3& vResultNear, math::vector3& vResultFar ) const;

    void                Transform( math::matrix4& matTransform );

    float               Width() const;
    float               Height() const;
    float               Depth() const;

    math::vector3       UpperLeftCorner() const;
    math::vector3       LowerRightCorner() const;
    math::vector3       TransformedCorner( size_t iIndex ) const;

};


#endif // __BOUNDING_BOX_H__