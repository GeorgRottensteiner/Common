#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H



#include <vector>

#include <GR/GRTypes.h>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Lang/GlobalAllocator.h>
#endif

#include "XBoundingBox.h"



class XMesh;
struct XTextureSection;

namespace Mesh
{

  class Vertex
  {
    public:

      GR::tVector     Position;

      GR::u32         NumReferences;


      Vertex() :
        NumReferences( 0 )
      {
      }

      Vertex( GR::f32 X, GR::f32 Y, GR::f32 Z )
      {
        NumReferences = 0;
        Position.set( X, Y, Z );
      }

      Vertex( int X, int Y, int Z )
      {
        NumReferences = 0;
        Position.set( (GR::f32)X, (GR::f32)Y, (GR::f32)Z );
      }

  };


  class Face
  {
    public:

      GR::tVector   m_Normal[3];

      GR::u32       m_Vertex[3],
                    m_SpecularColor[3],
                    m_DiffuseColor[3],
                    m_Flags;

      GR::f32       m_TextureX[3],
                    m_TextureY[3];

      Face( GR::u32 Vertex1 = 0, GR::u32 Vertex2 = 1, GR::u32 Vertex3 = 2 )
      {
        SetDefault();
        m_Vertex[0] = Vertex1;
        m_Vertex[1] = Vertex2;
        m_Vertex[2] = Vertex3;
      }

      Face( GR::u32 Vertex1, GR::u32 Color1,
            GR::u32 Vertex2, GR::u32 Color2,
            GR::u32 Vertex3, GR::u32 Color3 )
      {
        SetDefault();
        m_Vertex[0] = Vertex1;
        m_Vertex[1] = Vertex2;
        m_Vertex[2] = Vertex3;

        m_DiffuseColor[0] = Color1;
        m_DiffuseColor[1] = Color2;
        m_DiffuseColor[2] = Color3;
      }

      void SetDefault()
      {
        m_Normal[0] = GR::tVector( 0.0f, 0.0f, 0.0f );
        m_Normal[1] = GR::tVector( 0.0f, 0.0f, 0.0f );
        m_Normal[2] = GR::tVector( 0.0f, 0.0f, 0.0f );
        m_Vertex[0] = 0;
        m_Vertex[1] = 1;
        m_Vertex[2] = 2;
        m_DiffuseColor[0] = 0xffffffff;
        m_DiffuseColor[1] = 0xffffffff;
        m_DiffuseColor[2] = 0xffffffff;
        m_SpecularColor[0] = 0xffffffff;
        m_SpecularColor[1] = 0xffffffff;
        m_SpecularColor[2] = 0xffffffff;
        m_Flags = 0;
        m_TextureX[0] = 0.0f;
        m_TextureY[0] = 0.0f;
        m_TextureX[1] = 1.0f;
        m_TextureY[1] = 0.0f;
        m_TextureX[2] = 1.0f;
        m_TextureY[2] = 1.0f;
      }

      void CalculateNormals( XMesh& Mesh, bool Flip = false );

  };

  class IMesh
  {
    public:


      virtual ~IMesh()
      {
      }

      // Vertices
      virtual size_t              AddVertex( GR::f32 X, GR::f32 Y, GR::f32 Z ) = 0;
      virtual size_t              AddVertex( const Mesh::Vertex& Vertex ) = 0;
      virtual size_t              AddVertex( const GR::tVector& Vertex ) = 0;
      virtual void                InsertVertex( size_t Index, GR::f32 X, GR::f32 Y, GR::f32 Z ) = 0;
      virtual void                InsertVertex( size_t Index, const Mesh::Vertex& Vertex ) = 0;
      virtual void                InsertVertex( size_t Index, const GR::tVector& Vertex ) = 0;

      virtual void                SetVertex( GR::u32 Nr, GR::f32 X, GR::f32 Y, GR::f32 Z ) = 0;
      virtual void                RemoveVertex( int Nr ) = 0;

      // Faces
      virtual size_t              AddFace( Mesh::Face& Face ) = 0;
      virtual void                InsertFace( size_t FaceIndex, Mesh::Face& Face ) = 0;
      virtual void                RemoveFace( Mesh::Face* pFace ) = 0;

      virtual size_t              AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad ) = 0;
      virtual size_t              AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad,
                                           GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;
      virtual size_t              AddQuad( const GR::tVector& CornerUL, const GR::tVector& CornerUR, const GR::tVector& CornerLL, const GR::tVector& CornerLR ) = 0;
      virtual size_t              AddQuad( const GR::tVector& CornerUL, const GR::tVector& CornerUR, const GR::tVector& CornerLL, const GR::tVector& CornerLR,
                                           XTextureSection& tsQuad ) = 0;
      virtual size_t              AddQuad( const GR::tVector& CornerUL, GR::f32 TU1, GR::f32 TV1,
                                   const GR::tVector& CornerUR, GR::f32 TU2, GR::f32 TV2,
                                   const GR::tVector& CornerLL, GR::f32 TU3, GR::f32 TV3,
                                   const GR::tVector& CornerLR, GR::f32 TU4, GR::f32 TV4 ) = 0;
      virtual size_t              AddTriangle( const GR::tVector& V1, GR::f32 TU1, GR::f32 TV1,
                                       const GR::tVector& V2, GR::f32 TU2, GR::f32 TV2,
                                       const GR::tVector& V3, GR::f32 TU3, GR::f32 TV3 ) = 0;
      virtual size_t              AddTriangle( const GR::tVector& P1, 
                                       const GR::tVector& P2, 
                                       const GR::tVector& P3,
                                       XTextureSection& Section,
                                       int texSecCorner1, 
                                       int texSecCorner2,
                                       int texSecCorner3 ) = 0;

      virtual void                Clear() = 0;

      virtual void                CalculateNormals() = 0;
      virtual void                CalculateBoundingBox() = 0;

      virtual bool                IntersectWithRay( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal = NULL ) = 0;
      virtual bool                IntersectWithLineSegment( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal = NULL ) const = 0;

      virtual GR::u32             FaceCount() const = 0;
      virtual const Mesh::Face&   Face( GR::u32 Index ) const = 0;
      virtual Mesh::Face&         Face( GR::u32 Index ) = 0;

      virtual GR::u32             VertexCount() const = 0;
      virtual const Mesh::Vertex& Vertex( GR::u32 Index ) const = 0;
      virtual Mesh::Vertex&       Vertex( GR::u32 Index ) = 0;

  };

}



class XMesh : public Mesh::IMesh
{

  public:

#if OPERATING_SYSTEM == OS_WINDOWS
    typedef std::vector<Mesh::Vertex,GR::STL::global_allocator<Mesh::Vertex> > tVertices;

    typedef std::vector<Mesh::Face,GR::STL::global_allocator<Mesh::Face> >     tFaces;
#else
    typedef std::vector<Mesh::Vertex>   tVertices;

    typedef std::vector<Mesh::Face>     tFaces;
#endif

    tVertices                   m_Vertices;
    tFaces                      m_Faces;

    XBoundingBox                m_BoundingBox;


    XMesh()
    {
    }

    // Vertices
    size_t              AddVertex( GR::f32 X, GR::f32 Y, GR::f32 Z );
    size_t              AddVertex( const Mesh::Vertex& Vertex );
    size_t              AddVertex( const GR::tVector& Vertex );
    void                InsertVertex( size_t Index, GR::f32 X, GR::f32 Y, GR::f32 Z );
    void                InsertVertex( size_t Index, const Mesh::Vertex& Vertex );
    void                InsertVertex( size_t Index, const GR::tVector& Vertex );

    void                SetVertex( GR::u32 Nr, GR::f32 X, GR::f32 Y, GR::f32 Z );
    void                RemoveVertex( int Nr );

    // Faces
    size_t              AddFace( Mesh::Face& Face );
    void                InsertFace( size_t FaceIndex, Mesh::Face& Face );
    void                RemoveFace( Mesh::Face* pFace );

    size_t              AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad );
    size_t              AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad,
                                 GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 );
    size_t              AddQuad( const GR::tVector& CornerUL, const GR::tVector& CornerUR, const GR::tVector& CornerLL, const GR::tVector& CornerLR );
    size_t              AddQuad( const GR::tVector& CornerUL, const GR::tVector& CornerUR, const GR::tVector& CornerLL, const GR::tVector& CornerLR,
                                 XTextureSection& tsQuad );
    size_t              AddQuad( const GR::tVector& CornerUL, GR::f32 TU1, GR::f32 TV1,
                                 const GR::tVector& CornerUR, GR::f32 TU2, GR::f32 TV2,
                                 const GR::tVector& CornerLL, GR::f32 TU3, GR::f32 TV3,
                                 const GR::tVector& CornerLR, GR::f32 TU4, GR::f32 TV4 );
    size_t              AddTriangle( const GR::tVector& V1, GR::f32 TU1, GR::f32 TV1,
                                     const GR::tVector& V2, GR::f32 TU2, GR::f32 TV2,
                                     const GR::tVector& V3, GR::f32 TU3, GR::f32 TV3 );
    size_t              AddTriangle( const GR::tVector& P1, 
                                     const GR::tVector& P2, 
                                     const GR::tVector& P3,
                                     XTextureSection& Section,
                                     int texSecCorner1, 
                                     int texSecCorner2,
                                     int texSecCorner3 );

    void                Clear();

    void                CalculateNormals();
    void                CalculateBoundingBox();

    bool                IntersectWithRay( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal = NULL );
    bool                IntersectWithLineSegment( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal = NULL ) const;

    GR::u32             FaceCount() const;
    const Mesh::Face&   Face( GR::u32 Index ) const;
    Mesh::Face&         Face( GR::u32 Index );

    GR::u32             VertexCount() const;
    const Mesh::Vertex& Vertex( GR::u32 Index ) const;
    Mesh::Vertex&       Vertex( GR::u32 Index );

};


#endif // MESH_OBJECT_H