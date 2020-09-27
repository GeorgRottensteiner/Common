#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H

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

#include <vector>

#include <GR/GRTypes.h>

#include <DX8\VertexBuffer.h>
#include <DX8\BoundingBox.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CMesh;

struct tVertex
{
  math::vector3   position;
  math::vector3   normal;
  GR::u32         colorDiffuse;    // The color
  GR::f32         fU,
                  fV;
};

class CVertex
{
  public:

    math::vector3   vectPos;

    GR::u32         m_dwReferenzen;


    CVertex() :
      vectPos( 0, 0, 0 ),
      m_dwReferenzen( 0 )
    {
    }

    CVertex( GR::f32 fX, GR::f32 fY, GR::f32 fZ )
    {
      m_dwReferenzen = 0;
      vectPos.x = fX;
      vectPos.y = fY;
      vectPos.z = fZ;
    }

    CVertex( int iX, int iY, int iZ )
    {
      m_dwReferenzen = 0;
      vectPos.x = (GR::f32)iX;
      vectPos.y = (GR::f32)iY;
      vectPos.z = (GR::f32)iZ;
    }

};


class CFace
{
  public:

    math::vector3 vectNormal[3];

    GR::u32       m_dwVertex[3],
                  m_dwSpecularColor[3],
                  m_dwDiffuseColor[3],
                  m_dwFlags;

    GR::f32       m_fTextureX[3],
                  m_fTextureY[3];

    CFace( GR::u32 dwVertex1 = 0, GR::u32 dwVertex2 = 0, GR::u32 dwVertex3 = 0 )
    {
      vectNormal[0] = math::vector3( 0.0f, 0.0f, 0.0f );
      vectNormal[1] = math::vector3( 0.0f, 0.0f, 0.0f );
      vectNormal[2] = math::vector3( 0.0f, 0.0f, 0.0f );
      m_dwVertex[0] = dwVertex1;
      m_dwVertex[1] = dwVertex2;
      m_dwVertex[2] = dwVertex3;
      m_dwDiffuseColor[0] = 0xffffffff;
      m_dwDiffuseColor[1] = 0xffffffff;
      m_dwDiffuseColor[2] = 0xffffffff;
      m_dwSpecularColor[0] = 0xffffffff;
      m_dwSpecularColor[1] = 0xffffffff;
      m_dwSpecularColor[2] = 0xffffffff;
      m_dwFlags = 0;
      m_fTextureX[0] = 0.0f;
      m_fTextureY[0] = 0.0f;
      m_fTextureX[1] = 1.0f;
      m_fTextureY[1] = 0.0f;
      m_fTextureX[2] = 1.0f;
      m_fTextureY[2] = 1.0f;
    }

    void CalculateNormals( CMesh& Mesh, bool bFlip = false );

};


class CMesh
{

  public:

    typedef std::vector<CVertex>  tVectVertices;

    typedef std::vector<CFace>    tVectFaces;


    tVectVertices               m_vectVertices;

    tVectFaces                  m_vectFaces;

    CBoundingBox                m_BoundingBox;

    CVertexBuffer*              m_pVertexBuffer;



    CMesh() :
      m_pVertexBuffer( NULL )
    {
      m_vectVertices.clear();
    }

    virtual ~CMesh();

    // Vertices
    size_t              AddVertex( GR::f32 fX, GR::f32 fY, GR::f32 fZ );
    size_t              AddVertex( CVertex& newVertex );

    void                SetVertex( GR::u32 dwNr, GR::f32 fX, GR::f32 fY, GR::f32 fZ );
    void                RemoveVertex( int iNr );

    // Faces
    size_t              AddFace( CFace& newFace );
    void                RemoveFace( CFace* pFace );

    void                Clear();

    void                CalculateNormals();
    void                CalculateBoundingBox();

    CVertexBuffer*      CreateVertexBuffer( CD3DViewer& aViewer, GR::u32 dwFVF = D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ );

};


#endif // MESH_OBJECT_H