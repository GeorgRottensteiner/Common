#ifndef __DX8_OBJECT_H__
#define __DX8_OBJECT_H__
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

#pragma warning( disable:4786 )
#include <windows.h>
#include <d3dx8.h>
#include <vector>
#include <map>
#include <string>

#include <DX8\VertexBuffer.h>
#include <DX8\BoundingBox.h>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

namespace TAO
{
  namespace CHUNK
  {
    const DWORD INVALID               = 0;
    const DWORD MAGIC_NUMBER          = 0xABCD;
    const DWORD OBJECT_HEAD           = 0xA000;
    const DWORD OBJECT_POSITION       = 0xA100;
    const DWORD VERTEX_LIST           = 0xA200;
    const DWORD FACE_LIST             = 0xA300;
    const DWORD MATERIAL_LIST         = 0xA400;
    const DWORD TEXTURE_LIST          = 0xA500;
    const DWORD FRAME_HEAD            = 0xA600;
    const DWORD JOINT_LIST            = 0xA700;
    const DWORD FACE_ALPHA            = 0xF000;
    const DWORD FACE_TEXTURE          = 0xF001;
    const DWORD FACE_DIFFUSE_RGB      = 0xF002;
    const DWORD FACE_SPECULAR_RGB     = 0xF003;
    const DWORD FACE_NORMALS          = 0xF004;
    const DWORD FACE_MATERIAL         = 0xF005;
    const DWORD FACE_TEXTURE_UV       = 0xF006;
    const DWORD END_OF_FILE           = 0xFFFF;
  };
};


typedef struct tDX8Vertex
{
  D3DXVECTOR3 position;
  D3DXVECTOR3 normal;
  D3DCOLOR    colorDiffuse;    // The color
  float       fU,
              fV;
} tDX8Vertex;

class CDX8Vertex
{
  public:

    D3DXVECTOR3   vectPos;

    DWORD         m_dwReferenzen;


    CDX8Vertex() :
      vectPos( 0, 0, 0 ),
      m_dwReferenzen( 0 )
      {}
    CDX8Vertex( float fX, float fY, float fZ )
    {
      m_dwReferenzen = 0;
      vectPos.x = fX;
      vectPos.y = fY;
      vectPos.z = fZ;
    }
    CDX8Vertex( int iX, int iY, int iZ )
    {
      m_dwReferenzen = 0;
      vectPos.x = (float)iX;
      vectPos.y = (float)iY;
      vectPos.z = (float)iZ;
    }

};

typedef std::vector<CDX8Vertex>   tVectVertices;

class CDX8Joint
{
  public:

    D3DXVECTOR3   m_vectPos;

    float         m_fYaw,
                  m_fPitch,
                  m_fRoll;

    GR::String   m_strName;

    CDX8Joint() :
      m_vectPos( 0, 0, 0 ),
      m_fYaw( 0.0f ),
      m_fPitch( 0.0f ),
      m_fRoll( 0.0f )
      {}
    CDX8Joint( float fX, float fY, float fZ ) :
      m_fYaw( 0.0f ),
      m_fPitch( 0.0f ),
      m_fRoll( 0.0f )
    {
      m_vectPos.x = fX;
      m_vectPos.y = fY;
      m_vectPos.z = fZ;
    }

};

typedef std::vector<CDX8Joint>    tVectJoints;

class CDX8Object;
class CDX8Frame;

class CDX8Face
{
  public:

    D3DXVECTOR3   vectNormal[3];

    DWORD         m_dwVertex[3],
                  m_dwSpecularColor[3],
                  m_dwDiffuseColor[3],
                  m_dwFlags;

    float         m_fTextureX[3],
                  m_fTextureY[3];

    CDX8Face( DWORD dwVertex1 = 0, DWORD dwVertex2 = 0, DWORD dwVertex3 = 0 )
      {
        vectNormal[0] = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
        vectNormal[1] = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
        vectNormal[2] = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
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

    CDX8Face( const CDX8Face& rhs )
    {
      vectNormal[0]             = rhs.vectNormal[0];
      vectNormal[1]             = rhs.vectNormal[1];
      vectNormal[2]             = rhs.vectNormal[2];

      m_dwVertex[0]             = rhs.m_dwVertex[0];
      m_dwVertex[1]             = rhs.m_dwVertex[1];
      m_dwVertex[2]             = rhs.m_dwVertex[2];

      m_dwSpecularColor[0]      = rhs.m_dwSpecularColor[0];
      m_dwSpecularColor[1]      = rhs.m_dwSpecularColor[1];
      m_dwSpecularColor[2]      = rhs.m_dwSpecularColor[2];

      m_dwDiffuseColor[0]       = rhs.m_dwDiffuseColor[0];
      m_dwDiffuseColor[1]       = rhs.m_dwDiffuseColor[1];
      m_dwDiffuseColor[2]       = rhs.m_dwDiffuseColor[2];

      m_dwFlags                 = rhs.m_dwFlags;

      m_fTextureX[0]            = rhs.m_fTextureX[0];
      m_fTextureX[1]            = rhs.m_fTextureX[1];
      m_fTextureX[2]            = rhs.m_fTextureX[2];
      m_fTextureY[0]            = rhs.m_fTextureY[0];
      m_fTextureY[1]            = rhs.m_fTextureY[1];
      m_fTextureY[2]            = rhs.m_fTextureY[2];
    }
    CDX8Face& CDX8Face::operator= ( const CDX8Face& rhs )
    {

      if ( &rhs == this )
      {
        return *this;
      }

      vectNormal[0]             = rhs.vectNormal[0];
      vectNormal[1]             = rhs.vectNormal[1];
      vectNormal[2]             = rhs.vectNormal[2];

      m_dwVertex[0]             = rhs.m_dwVertex[0];
      m_dwVertex[1]             = rhs.m_dwVertex[1];
      m_dwVertex[2]             = rhs.m_dwVertex[2];

      m_dwSpecularColor[0]      = rhs.m_dwSpecularColor[0];
      m_dwSpecularColor[1]      = rhs.m_dwSpecularColor[1];
      m_dwSpecularColor[2]      = rhs.m_dwSpecularColor[2];

      m_dwDiffuseColor[0]       = rhs.m_dwDiffuseColor[0];
      m_dwDiffuseColor[1]       = rhs.m_dwDiffuseColor[1];
      m_dwDiffuseColor[2]       = rhs.m_dwDiffuseColor[2];

      m_dwFlags                 = rhs.m_dwFlags;

      m_fTextureX[0]            = rhs.m_fTextureX[0];
      m_fTextureX[1]            = rhs.m_fTextureX[1];
      m_fTextureX[2]            = rhs.m_fTextureX[2];
      m_fTextureY[0]            = rhs.m_fTextureY[0];
      m_fTextureY[1]            = rhs.m_fTextureY[1];
      m_fTextureY[2]            = rhs.m_fTextureY[2];

      return *this;
    }

    void CalcNormal( CDX8Frame& Frame, bool bFlip = false );
};

typedef std::vector<CDX8Face>   tVectFaces;

class CDX8Frame
{
  public:

    GR::String                 m_strName;

    tVectVertices               m_vectVertices;

    tVectJoints                 m_vectJoints;

    CVertexBuffer*              m_pVertexBuffer;

    CBoundingBox                m_BoundingBox;


    CDX8Frame() :
      m_pVertexBuffer( NULL ),
      m_strName( "" )
    {
        m_vectVertices.clear();
        m_vectJoints.clear();
    }
    virtual ~CDX8Frame()
    {
    }

    size_t AddVertex( float fX, float fY, float fZ )
    {
      m_vectVertices.push_back( CDX8Vertex( fX, fY, fZ ) );
      return m_vectVertices.size() - 1;
    }

    size_t AddVertex( CDX8Vertex& newVertex )
    {
      m_vectVertices.push_back( newVertex );
      return m_vectVertices.size() - 1;
    }

    size_t AddJoint( CDX8Joint& Joint )
    {
      m_vectJoints.push_back( Joint );
      return m_vectJoints.size() - 1;
    }

    void SetJoint( DWORD dwNr, CDX8Joint& Joint )
    {
      while ( m_vectJoints.size() <= dwNr )
      {
        m_vectJoints.push_back( CDX8Joint( Joint ) );
      }
      m_vectJoints[dwNr] = Joint;
    }

    void SetVertex( DWORD dwNr, float fX, float fY, float fZ )
    {
      while ( m_vectVertices.size() <= dwNr )
      {
        m_vectVertices.push_back( CDX8Vertex( fX, fY, fZ ) );
      }
      m_vectVertices[dwNr].vectPos.x = fX;
      m_vectVertices[dwNr].vectPos.y = fY;
      m_vectVertices[dwNr].vectPos.z = fZ;
    }

    void RemoveJoint( CDX8Joint* pJoint )
    {
      tVectJoints::iterator   itPos( m_vectJoints.begin() );
      while ( itPos != m_vectJoints.end() )
      {
        if ( &(*itPos) == pJoint )
        {
          m_vectJoints.erase( itPos );
          return;
        }
        itPos++;
      }
    }

    void CalculateNormals( CDX8Object* pObject );
    void Scale( const float fX, const float fY, const float fZ );
    void Align( const int iFlags, const math::vector3& vAlignTo );

};

typedef std::map<GR::String,CDX8Frame>    tMapFrames;




class CD3DViewer;

class CDX8Object
{

  public:


    tVectFaces                      m_vectFaces;

    tMapFrames                      m_mapFrames;

    CDX8Object()
    {
      m_vectFaces.clear();
      m_mapFrames.clear();
    }

    virtual   ~CDX8Object();

    void                CalculateBoundingBox();
    BOOL                CreateVertexBuffers( CD3DViewer& aViewer, DWORD dwFVF = D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_XYZ );

    void                Clear();


    // Vertices
    size_t              AddVertex( float fX, float fY, float fZ );
    void                RemoveVertex( int iNr );

    // Faces
    size_t              AddFace( CDX8Face& newFace );
    void                RemoveFace( CDX8Face* pFace );

    // Frames
    CDX8Frame*          AddFrame( const char *szName = "Default", CDX8Frame& Frame = CDX8Frame() );
    CDX8Frame*          GetFrame( const char* szName = NULL );

    // Joints
    size_t              AddJoint( float fX, float fY, float fZ, const char* szName );
    void                RemoveJoint( CDX8Joint* pJoint );

    void                CenterAbout( const math::vector3& vCenter );

};



/*-Prototypen-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CDX8Object *LoadObject( char *szName, BOOL bLoadTextures );
CDX8Object* LoadDX8Object( const char* szFileName );


#endif