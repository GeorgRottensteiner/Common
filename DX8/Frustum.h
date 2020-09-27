// Frustum.h: interface for the CFrustum class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRUSTUM_H__634612D0_46DA_4092_BE72_646923985E24__INCLUDED_)
#define AFX_FRUSTUM_H__634612D0_46DA_4092_BE72_646923985E24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <d3dx8.h>

#include <DX8\BoundingBox.h>

class CFrustum  
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

	  CFrustum();
	  virtual ~CFrustum();

    D3DXPLANE       planeLeft,
                    planeRight,
                    planeTop,
                    planeBottom,
                    planeZNear,
                    planeZFar;

    D3DXVECTOR3     m_vectCorners[C_CORNER_COUNT];


    void            Create( D3DXMATRIX& matProjection, D3DXMATRIX& matView, D3DVIEWPORT8& viewPort,
                            float fExtraBorder = 0.0f );

    // erwartet eine Axis-Aligned (nicht rotierte) Bounding Box
    bool            IntersectWithBoundingBox( const CBoundingBox& BBox ) const;
    bool            IntersectWithBoundingBox( D3DXVECTOR3& vectLO, D3DXVECTOR3& vectRU ) const;
    // erhält die 8 Eckpunkte einer rotierten (orientierten) Bounding Box
    bool            IntersectWithBoundingBoxRotated( const CBoundingBox& BBox ) const;
    bool            IntersectWithRotatedBox( D3DXVECTOR3& vect1, D3DXVECTOR3& vect2,
                                             D3DXVECTOR3& vect3, D3DXVECTOR3& vect4,
                                             D3DXVECTOR3& vect5, D3DXVECTOR3& vect6,
                                             D3DXVECTOR3& vect7, D3DXVECTOR3& vect8 ) const;
};

#endif // !defined(AFX_FRUSTUM_H__634612D0_46DA_4092_BE72_646923985E24__INCLUDED_)
