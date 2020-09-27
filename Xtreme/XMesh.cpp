#include "XMesh.h"
#include "XTextureSection.h"

#include <IO\FileStream.h>

#include <Debug\debugclient.h>

#include <math/mathutil.h>

#include <vector>



namespace Mesh
{

  void Face::CalculateNormals( XMesh& Object, bool bFlip )
  {

    if ( ( m_Vertex[0] >= Object.m_Vertices.size() )
    ||   ( m_Vertex[1] >= Object.m_Vertices.size() )
    ||   ( m_Vertex[2] >= Object.m_Vertices.size() ) )
    {
      dh::Log( "Vertex Index out of bounds %d,%d,%d > %d",
               m_Vertex[0], m_Vertex[1], m_Vertex[2],
               Object.m_Vertices.size() );
      return;
    }


    math::vector3     vectDummy1,
                      vectDummy2;

    vectDummy1 = Object.m_Vertices[m_Vertex[0]].Position - Object.m_Vertices[m_Vertex[1]].Position;
    vectDummy2 = Object.m_Vertices[m_Vertex[0]].Position - Object.m_Vertices[m_Vertex[2]].Position;

    vectDummy1 = vectDummy1.cross( vectDummy2 );

    if ( bFlip )
    {
      vectDummy1 = -vectDummy1;
    }
    vectDummy1.normalize();

    m_Normal[0] = vectDummy1;
    m_Normal[1] = vectDummy1;
    m_Normal[2] = vectDummy1;

  }

}



void XMesh::CalculateBoundingBox()
{
  m_BoundingBox.Clear();

  tVertices::iterator   itVertex( m_Vertices.begin() );
  while ( itVertex != m_Vertices.end() )
  {
    Mesh::Vertex&   Vertex = *itVertex;

    m_BoundingBox.AddVertex( Vertex.Position );

    ++itVertex;
  }
}



void XMesh::CalculateNormals()
{

  tFaces::iterator   itFace( m_Faces.begin() );
  while ( itFace != m_Faces.end() )
  {
    Mesh::Face&   Face = *itFace;

    Face.CalculateNormals( *this );

    ++itFace;
  }

}



void XMesh::Clear()
{
  m_Faces.clear();
  m_Vertices.clear();
}



size_t XMesh::AddVertex( GR::f32 X, GR::f32 Y, GR::f32 Z )
{
  m_Vertices.push_back( Mesh::Vertex( X, Y, Z ) );
  return m_Vertices.size() - 1;
}



size_t XMesh::AddVertex( const Mesh::Vertex& Vertex )
{
  m_Vertices.push_back( Vertex );
  return m_Vertices.size() - 1;
}



size_t XMesh::AddVertex( const GR::tVector& Vect )
{
  m_Vertices.push_back( Mesh::Vertex( Vect.x, Vect.y, Vect.z ) );
  return m_Vertices.size() - 1;
}



void XMesh::InsertVertex( size_t Index, GR::f32 X, GR::f32 Y, GR::f32 Z )
{
  if ( Index >= m_Vertices.size() )
  {
    return;
  }
  m_Vertices.insert( m_Vertices.begin() + Index, Mesh::Vertex( X, Y, Z ) );
  // adjust vertex indices
  for ( size_t i = 0; i < m_Faces.size(); ++i )
  {
    Mesh::Face&    Face( m_Faces[i] );

    for ( int j = 0; j < 3; ++j )
    {
      if ( Face.m_Vertex[j] >= Index )
      {
        Face.m_Vertex[j]++;
      }
    }
  }
}



void XMesh::InsertVertex( size_t Index, const Mesh::Vertex& Vertex )
{
  if ( Index >= m_Vertices.size() )
  {
    return;
  }
  m_Vertices.insert( m_Vertices.begin() + Index, Vertex );
  // adjust vertex indices
  for ( size_t i = 0; i < m_Faces.size(); ++i )
  {
    Mesh::Face&    Face( m_Faces[i] );

    for ( int j = 0; j < 3; ++j )
    {
      if ( Face.m_Vertex[j] >= Index )
      {
        Face.m_Vertex[j]++;
      }
    }
  }
}



void XMesh::InsertVertex( size_t Index, const GR::tVector& Vect )
{
  if ( Index >= m_Vertices.size() )
  {
    return;
  }
  m_Vertices.insert( m_Vertices.begin() + Index, Mesh::Vertex( Vect.x, Vect.y, Vect.z ) );
  // adjust vertex indices
  for ( size_t i = 0; i < m_Faces.size(); ++i )
  {
    Mesh::Face&    Face( m_Faces[i] );

    for ( int j = 0; j < 3; ++j )
    {
      if ( Face.m_Vertex[j] >= Index )
      {
        Face.m_Vertex[j]++;
      }
    }
  }
}



void XMesh::SetVertex( GR::u32 Nr, GR::f32 X, GR::f32 Y, GR::f32 Z )
{

  while ( m_Vertices.size() <= Nr )
  {
    m_Vertices.push_back( Mesh::Vertex( X, Y, Z ) );
  }
  m_Vertices[Nr].Position.set( X, Y, Z );

}



void XMesh::RemoveVertex( int Nr )
{

  tVertices::iterator itV( m_Vertices.begin() );
  std::advance( itV, Nr );
  if ( itV != m_Vertices.end() )
  {
    m_Vertices.erase( itV );
    // adjust vertex indices
    for ( size_t i = 0; i < m_Faces.size(); ++i )
    {
      Mesh::Face&    Face( m_Faces[i] );

      for ( int j = 0; j < 3; ++j )
      {
        if ( Face.m_Vertex[j] > (GR::u32)Nr )
        {
          Face.m_Vertex[j]--;
        }
        else if ( Face.m_Vertex[j] == Nr )
        {
          // this face has an invalid vertex now!
          Face.m_Vertex[j] = -1;
        }
      }
    }

  }

}



size_t XMesh::AddFace( Mesh::Face& Face )
{

  m_Faces.push_back( Face );

  for ( int i = 0; i < 3; ++i )
  {
    if ( Face.m_Vertex[i] >= m_Vertices.size() )
    {
      dh::Log( "XMesh::AddFace Vertex Index out of bounds Nr %d - %d > %d", i, Face.m_Vertex[i], m_Vertices.size() );
    }
  }

  return m_Faces.size() - 1;

}



void XMesh::InsertFace( size_t FaceIndex, Mesh::Face& Face )
{

  if ( FaceIndex >= m_Faces.size() )
  {
    return;
  }

  m_Faces.insert( m_Faces.begin() + FaceIndex, Face );

  for ( int i = 0; i < 3; ++i )
  {
    if ( Face.m_Vertex[i] >= m_Vertices.size() )
    {
      dh::Log( "XMesh::AddFace Vertex Index out of bounds Nr %d - %d > %d", i, Face.m_Vertex[i], m_Vertices.size() );
    }
  }

}



void XMesh::RemoveFace( Mesh::Face* pFace )
{

  if ( pFace == NULL )
  {
    return;
  }
  tFaces::iterator    it( m_Faces.begin() );
  while ( it != m_Faces.end() )
  {
    if ( &(*it) == pFace )
    {
      m_Faces.erase( it );
      return;
    }

    ++it;
  }

}



bool XMesh::IntersectWithRay( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal )
{

  float         fClosestDistanceSquared = 6000000000.0f;
  GR::tVector   vectFaceHit;
  bool          bHit = false;

  tFaces::iterator    it( m_Faces.begin() );
  while ( it != m_Faces.end() )
  {
    Mesh::Face&    Face = *it;

    if ( math::IntersectTriangleWithLine( vect1, vect2,
                                          m_Vertices[Face.m_Vertex[0]].Position,
                                          m_Vertices[Face.m_Vertex[1]].Position,
                                          m_Vertices[Face.m_Vertex[2]].Position,
                                          vectFaceHit ) )
    {
      float   fDistanceSquared = ( vect1 - vectFaceHit ).length_squared();

      if ( fDistanceSquared < fClosestDistanceSquared )
      {
        fClosestDistanceSquared = fDistanceSquared;
        vectHit = vectFaceHit;
        bHit = true;
        if ( pVectNormal )
        {
          *pVectNormal = Face.m_Normal[0];
        }
      }
    }

    ++it;
  }

  return bHit;

}



bool XMesh::IntersectWithLineSegment( const GR::tVector& vect1, const GR::tVector& vect2, GR::tVector& vectHit, GR::tVector* pVectNormal ) const
{

  float         fClosestDistanceSquared = 6000000000.0f;
  GR::tVector   vectFaceHit;
  bool          bHit = false;

  tFaces::const_iterator    it( m_Faces.begin() );
  while ( it != m_Faces.end() )
  {
    const Mesh::Face&    Face = *it;

    if ( math::IntersectTriangleWithLineSegment( vect1, vect2,
                                                 m_Vertices[Face.m_Vertex[0]].Position,
                                                 m_Vertices[Face.m_Vertex[1]].Position,
                                                 m_Vertices[Face.m_Vertex[2]].Position,
                                                 vectFaceHit ) )
    {
      float   fDistanceSquared = ( vect1 - vectFaceHit ).length_squared();

      if ( fDistanceSquared < fClosestDistanceSquared )
      {
        fClosestDistanceSquared = fDistanceSquared;
        vectHit = vectFaceHit;
        bHit = true;

        if ( pVectNormal )
        {
          *pVectNormal = Face.m_Normal[0];
        }
      }
    }

    ++it;
  }

  return bHit;

}



size_t XMesh::AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad )
{
  if ( ( Vertex1 < 0 )
  ||   ( Vertex1 >= (int)m_Vertices.size() )
  ||   ( Vertex2 < 0 )
  ||   ( Vertex2 >= (int)m_Vertices.size() )
  ||   ( Vertex3 < 0 )
  ||   ( Vertex3 >= (int)m_Vertices.size() )
  ||   ( Vertex4 < 0 )
  ||   ( Vertex4 >= (int)m_Vertices.size() ) )
  {
    return (size_t)-1;
  }

  GR::tFPoint     TUV1;
  GR::tFPoint     TUV2;
  GR::tFPoint     TUV3;
  GR::tFPoint     TUV4;

  tsQuad.GetTrueUV( TUV1, TUV2, TUV3, TUV4 );

  Mesh::Face   Face1( Vertex1, Vertex2, Vertex3 );
  Face1.m_TextureX[0] = TUV1.x;
  Face1.m_TextureY[0] = TUV1.y;
  Face1.m_TextureX[1] = TUV2.x;
  Face1.m_TextureY[1] = TUV2.y;
  Face1.m_TextureX[2] = TUV3.x;
  Face1.m_TextureY[2] = TUV3.y;

  Mesh::Face   Face2( Vertex3, Vertex2, Vertex4 );
  Face2.m_TextureX[0] = TUV3.x;
  Face2.m_TextureY[0] = TUV3.y;
  Face2.m_TextureX[1] = TUV2.x;
  Face2.m_TextureY[1] = TUV2.y;
  Face2.m_TextureX[2] = TUV4.x;
  Face2.m_TextureY[2] = TUV4.y;

  AddFace( Face1 );
  AddFace( Face2 );

  return Vertex1;
}



size_t XMesh::AddQuad( int Vertex1, int Vertex2, int Vertex3, int Vertex4, XTextureSection& tsQuad,
                       GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4 )
{
  if ( ( Vertex1 < 0 )
  ||   ( Vertex1 >= (int)m_Vertices.size() )
  ||   ( Vertex2 < 0 )
  ||   ( Vertex2 >= (int)m_Vertices.size() )
  ||   ( Vertex3 < 0 )
  ||   ( Vertex3 >= (int)m_Vertices.size() )
  ||   ( Vertex4 < 0 )
  ||   ( Vertex4 >= (int)m_Vertices.size() ) )
  {
    return (size_t)-1;
  }

  if ( Color2 == 0 )
  {
    Color2 = Color1;
    if ( Color3 == 0 )
    {
      Color3 = Color2;
      if ( Color4 == 0 )
      {
        Color4 = Color3;
      }
    }
  }

  GR::tFPoint     TUV1;
  GR::tFPoint     TUV2;
  GR::tFPoint     TUV3;
  GR::tFPoint     TUV4;

  tsQuad.GetTrueUV( TUV1, TUV2, TUV3, TUV4 );

  Mesh::Face   Face1( Vertex1, Vertex2, Vertex3 );
  Face1.m_TextureX[0] = TUV1.x;
  Face1.m_TextureY[0] = TUV1.y;
  Face1.m_TextureX[1] = TUV2.x;
  Face1.m_TextureY[1] = TUV2.y;
  Face1.m_TextureX[2] = TUV3.x;
  Face1.m_TextureY[2] = TUV3.y;
  Face1.m_DiffuseColor[0] = Color1;
  Face1.m_DiffuseColor[1] = Color2;
  Face1.m_DiffuseColor[2] = Color3;

  Mesh::Face   Face2( Vertex3, Vertex2, Vertex4 );
  Face2.m_TextureX[0] = TUV3.x;
  Face2.m_TextureY[0] = TUV3.y;
  Face2.m_TextureX[1] = TUV2.x;
  Face2.m_TextureY[1] = TUV2.y;
  Face2.m_TextureX[2] = TUV4.x;
  Face2.m_TextureY[2] = TUV4.y;
  Face2.m_DiffuseColor[0] = Color3;
  Face2.m_DiffuseColor[1] = Color2;
  Face2.m_DiffuseColor[2] = Color4;

  AddFace( Face1 );
  AddFace( Face2 );

  return Vertex1;
}



size_t XMesh::AddQuad( const GR::tVector& CornerUL, const GR::tVector& CornerUR, const GR::tVector& CornerLL, const GR::tVector& CornerLR,
                       XTextureSection& tsQuad )
{
  GR::u32     ResultIndex = (GR::u32)AddVertex( CornerUL );
  AddVertex( CornerUR );
  AddVertex( CornerLL );
  AddVertex( CornerLR );

  GR::tFPoint     TUV1;
  GR::tFPoint     TUV2;
  GR::tFPoint     TUV3;
  GR::tFPoint     TUV4;

  tsQuad.GetTrueUV( TUV1, TUV2, TUV3, TUV4 );

  Mesh::Face   Face1( ResultIndex, ResultIndex + 1, ResultIndex + 2 );
  Face1.m_TextureX[0] = TUV1.x;
  Face1.m_TextureY[0] = TUV1.y;
  Face1.m_TextureX[1] = TUV2.x;
  Face1.m_TextureY[1] = TUV2.y;
  Face1.m_TextureX[2] = TUV3.x;
  Face1.m_TextureY[2] = TUV3.y;

  Mesh::Face   Face2( ResultIndex + 2, ResultIndex + 1, ResultIndex + 3 );
  Face2.m_TextureX[0] = TUV3.x;
  Face2.m_TextureY[0] = TUV3.y;
  Face2.m_TextureX[1] = TUV2.x;
  Face2.m_TextureY[1] = TUV2.y;
  Face2.m_TextureX[2] = TUV4.x;
  Face2.m_TextureY[2] = TUV4.y;

  AddFace( Face1 );
  AddFace( Face2 );

  return ResultIndex;
}



size_t XMesh::AddQuad( const GR::tVector& CornerUL, GR::f32 TU1, GR::f32 TV1,
                       const GR::tVector& CornerUR, GR::f32 TU2, GR::f32 TV2,
                       const GR::tVector& CornerLL, GR::f32 TU3, GR::f32 TV3,
                       const GR::tVector& CornerLR, GR::f32 TU4, GR::f32 TV4 )
{

  GR::u32     ResultIndex = (GR::u32)AddVertex( CornerUL );
  AddVertex( CornerUR );
  AddVertex( CornerLL );
  AddVertex( CornerLR );

  Mesh::Face   Face1( ResultIndex, ResultIndex + 1, ResultIndex + 2 );
  Face1.m_TextureX[0] = TU1;
  Face1.m_TextureY[0] = TV1;
  Face1.m_TextureX[1] = TU2;
  Face1.m_TextureY[1] = TV2;
  Face1.m_TextureX[2] = TU3;
  Face1.m_TextureY[2] = TV3;

  Mesh::Face   Face2( ResultIndex + 2, ResultIndex + 1, ResultIndex + 3 );
  Face2.m_TextureX[0] = TU3;
  Face2.m_TextureY[0] = TV3;
  Face2.m_TextureX[1] = TU2;
  Face2.m_TextureY[1] = TV2;
  Face2.m_TextureX[2] = TU4;
  Face2.m_TextureY[2] = TV4;

  AddFace( Face1 );
  AddFace( Face2 );

  return ResultIndex;

}



size_t XMesh::AddQuad( const GR::tVector& CornerUL,
                       const GR::tVector& CornerUR,
                       const GR::tVector& CornerLL,
                       const GR::tVector& CornerLR )
{
  GR::u32     ResultIndex = (GR::u32)AddVertex( CornerUL );
  AddVertex( CornerUR );
  AddVertex( CornerLL );
  AddVertex( CornerLR );

  Mesh::Face   Face1( ResultIndex, ResultIndex + 1, ResultIndex + 2 );
  Mesh::Face   Face2( ResultIndex + 2, ResultIndex + 1, ResultIndex + 3 );

  AddFace( Face1 );
  AddFace( Face2 );

  return ResultIndex;
}



size_t XMesh::AddTriangle( const GR::tVector& V1, GR::f32 TU1, GR::f32 TV1,
                           const GR::tVector& V2, GR::f32 TU2, GR::f32 TV2,
                           const GR::tVector& V3, GR::f32 TU3, GR::f32 TV3 )
{

  GR::u32     ResultIndex = (GR::u32)AddVertex( V1 );
  AddVertex( V2 );
  AddVertex( V3 );

  Mesh::Face   Face1( ResultIndex, ResultIndex + 1, ResultIndex + 2 );
  Face1.m_TextureX[0] = TU1;
  Face1.m_TextureY[0] = TV1;
  Face1.m_TextureX[1] = TU2;
  Face1.m_TextureY[1] = TV2;
  Face1.m_TextureX[2] = TU3;
  Face1.m_TextureY[2] = TV3;

  AddFace( Face1 );

  return ResultIndex;

}



size_t XMesh::AddTriangle( const GR::tVector& P1, 
                           const GR::tVector& P2, 
                           const GR::tVector& P3,
                           XTextureSection& tsQuad,
                           int texSecCorner1, 
                           int texSecCorner2,
                           int texSecCorner3 )
{

  GR::u32     ResultIndex = (GR::u32)AddVertex( P1 );
  AddVertex( P2 );
  AddVertex( P3 );

  GR::tFPoint     TUV[4];

  tsQuad.GetTrueUV( TUV[0], TUV[1], TUV[2], TUV[3] );

  Mesh::Face   Face1( ResultIndex, ResultIndex + 1, ResultIndex + 2 );
  Face1.m_TextureX[0] = TUV[texSecCorner1].x;
  Face1.m_TextureY[0] = TUV[texSecCorner1].y;
  Face1.m_TextureX[1] = TUV[texSecCorner2].x;
  Face1.m_TextureY[1] = TUV[texSecCorner2].y;
  Face1.m_TextureX[2] = TUV[texSecCorner3].x;
  Face1.m_TextureY[2] = TUV[texSecCorner3].y;

  AddFace( Face1 );

  return ResultIndex;

}



GR::u32 XMesh::FaceCount() const
{

  return (GR::u32)m_Faces.size();

}



Mesh::Face& XMesh::Face( GR::u32 Index )
{
  if ( Index >= (GR::u32)m_Faces.size() )
  {
    dh::Log( "XMesh::Face: Accessing Face index %d out of bounds %d", Index, m_Faces.size() );
    static Mesh::Face    face;

    return face;
  }
  return m_Faces[Index];
}



const Mesh::Face& XMesh::Face( GR::u32 Index ) const
{
  if ( Index >= (GR::u32)m_Faces.size() )
  {
    dh::Log( "XMesh::Face: Accessing Face index %d out of bounds %d", Index, m_Faces.size() );
    static Mesh::Face    face;

    return face;
  }
  return m_Faces[Index];
}



GR::u32 XMesh::VertexCount() const
{

  return (GR::u32)m_Vertices.size();

}



Mesh::Vertex& XMesh::Vertex( GR::u32 Index )
{
  if ( Index >= (GR::u32)m_Vertices.size() )
  {
    static Mesh::Vertex    vertex;

    return vertex;
  }
  return m_Vertices[Index];
}



const Mesh::Vertex& XMesh::Vertex( GR::u32 Index ) const
{
  if ( Index >= (GR::u32)m_Vertices.size() )
  {
    static Mesh::Vertex    vertex;

    return vertex;
  }
  return m_Vertices[Index];
}
