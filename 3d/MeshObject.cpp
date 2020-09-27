#include "MeshObject.h"

#include <IO\FileStream.h>
#include <IO\FileChunk.h>

#include <DX8/DX8Viewer.h>

#include <Debug\debugclient.h>

#include <Misc/Misc.h>

#include <vector>



void CFace::CalculateNormals( CMesh& Object, bool bFlip )
{

  math::vector3     vectDummy1,
                    vectDummy2;

  vectDummy1 = Object.m_vectVertices[m_dwVertex[0]].vectPos - Object.m_vectVertices[m_dwVertex[1]].vectPos;
  vectDummy2 = Object.m_vectVertices[m_dwVertex[0]].vectPos - Object.m_vectVertices[m_dwVertex[2]].vectPos;

  vectDummy1.cross( vectDummy2 );
  vectDummy1.normalize();

  if ( bFlip )
  {
    vectDummy1[0] = -vectDummy1[0];
  }
  vectDummy1[1] = vectDummy1[0];
  vectDummy1[2] = vectDummy1[0];

}



/*-Destructor-----------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CMesh::~CMesh()
{
  
  m_vectFaces.clear();

}



/*-CalculateBoundingBox-------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMesh::CalculateBoundingBox()
{

  m_BoundingBox.Clear();
  
  std::vector<CVertex>::iterator   itVertex( m_vectVertices.begin() );
  while ( itVertex != m_vectVertices.end() )
  {
    CVertex&   Vertex = *itVertex;

    m_BoundingBox.ErweitereUmVertex( Vertex.vectPos );

    ++itVertex;
  }

}



/*-CreateVertexBuffer---------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

CVertexBuffer* CMesh::CreateVertexBuffer( CD3DViewer& aViewer, GR::u32 dwFVF )
{

  if ( m_vectFaces.empty() )
  {
    dh::Log( "CMeshObject::CreateVertexBuffer no Faces\n" );
    return NULL;
  }

  GR::u32   dwVertexSize = 0;

  GR::i32   iOffsetXYZ = -1,
            iOffsetRHW = -1,
            iOffsetNormal = -1,
            iOffsetDiffuse = -1,
            iOffsetSpecular = -1,
            iOffsetTexCoord = -1;


  if ( dwFVF & D3DFVF_XYZ )
  {
    iOffsetXYZ = dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );
  }
  else if ( dwFVF & D3DFVF_XYZRHW )
  {
    iOffsetXYZ = dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );
    iOffsetRHW = dwVertexSize;
    dwVertexSize += sizeof( GR::f32 );
  }
  else if ( ( dwFVF & D3DFVF_XYZB1 )
  ||        ( dwFVF & D3DFVF_XYZB2 )
  ||        ( dwFVF & D3DFVF_XYZB3 )
  ||        ( dwFVF & D3DFVF_XYZB4 )
  ||        ( dwFVF & D3DFVF_XYZB5 ) )
  {
    dh::Log( "CMeshObject::CreateVertexBuffer Blend Weights not supported yet.\n" );
  }
  if ( dwFVF & D3DFVF_NORMAL )
  {
    iOffsetNormal = dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );
  }
  if ( dwFVF & D3DFVF_PSIZE )
  {
    dh::Log( "CMeshObject::CreateVertexBuffer Point Sizes not supported yet.\n" );
  }
  if ( dwFVF & D3DFVF_DIFFUSE )
  {
    iOffsetDiffuse = dwVertexSize;
    dwVertexSize += sizeof( GR::u32 );
  }
  if ( dwFVF & D3DFVF_SPECULAR )
  {
    iOffsetSpecular = dwVertexSize;
    dwVertexSize += sizeof( GR::u32 );
  }
  // BAUSTELLE - Texture Koordinaten sollten die dwFVF-Flags richtig parsen
  if ( dwFVF & D3DFVF_TEX1 )
  {
    iOffsetTexCoord = dwVertexSize;
    dwVertexSize += 2 * sizeof( GR::f32 );
  }

  if ( dwVertexSize == 0 )
  {
    dh::Log( "CMeshObject::CreateVertexBuffer Created without content!\n" );
    return NULL;
  }

  GR::u8*       pVertexCache;

  CVertexBuffer* pBuffer = aViewer.CreateVertexBuffer( (GR::u32)m_vectFaces.size(),
                                      dwVertexSize,
                                      (GR::u32)( m_vectFaces.size() * dwVertexSize * 3 ),
                                      D3DPT_TRIANGLELIST,
                                      D3DUSAGE_WRITEONLY,//D3DUSAGE_DYNAMIC,
                                      dwFVF,
                                      D3DPOOL_DEFAULT );

  pVertexCache = (GR::u8*)pBuffer->m_pData;

  GR::u8*       pVertexCacheStart = pVertexCache;

  for ( size_t i = 0; i < m_vectFaces.size(); i++ )
  {
    for ( int j = 0; j < 3; j++ )
    {
      if ( iOffsetXYZ != -1 )
      {
        *(math::vector3*)( pVertexCache + iOffsetXYZ ) = m_vectVertices[m_vectFaces[i].m_dwVertex[j]].vectPos;
      }
      if ( iOffsetNormal != -1 )
      {
        *(math::vector3*)( pVertexCache + iOffsetNormal ) = m_vectFaces[i].vectNormal[j];
      }
      if ( iOffsetDiffuse != -1 )
      {
        *(GR::u32*)( pVertexCache + iOffsetDiffuse ) = m_vectFaces[i].m_dwDiffuseColor[j];
      }
      if ( iOffsetSpecular != -1 )
      {
        *(GR::u32*)( pVertexCache + iOffsetSpecular ) = 0xffffffff;//m_vectFaces[i].m_dwSpecularColor[j];
      }
      if ( iOffsetTexCoord != -1 )
      {
        (*(GR::f32*)( pVertexCache + iOffsetTexCoord ) ) = m_vectFaces[i].m_fTextureX[j];
        (*(GR::f32*)( pVertexCache + iOffsetTexCoord + sizeof( GR::f32 ) ) ) = m_vectFaces[i].m_fTextureY[j];
      }
      pVertexCache += dwVertexSize;
    }
  }

  pBuffer->UpdateData();

  m_pVertexBuffer = pBuffer;

  return m_pVertexBuffer;

}



/*-Clear----------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMesh::Clear()
{

  m_vectFaces.clear();

}



/*-AddVertex------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CMesh::AddVertex( GR::f32 fX, GR::f32 fY, GR::f32 fZ )
{
  m_vectVertices.push_back( CVertex( fX, fY, fZ ) );
  return m_vectVertices.size() - 1;
}

size_t CMesh::AddVertex( CVertex& newVertex )
{
  m_vectVertices.push_back( newVertex );
  return m_vectVertices.size() - 1;
}



void CMesh::SetVertex( GR::u32 dwNr, GR::f32 fX, GR::f32 fY, GR::f32 fZ )
{

  while ( m_vectVertices.size() <= dwNr )
  {
    m_vectVertices.push_back( CVertex( fX, fY, fZ ) );
  }
  m_vectVertices[dwNr].vectPos.x = fX;
  m_vectVertices[dwNr].vectPos.y = fY;
  m_vectVertices[dwNr].vectPos.z = fZ;

}


/*-RemoveVertex---------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

void CMesh::RemoveVertex( int iNr )
{

  tVectVertices::iterator itV( m_vectVertices.begin() );
  std::advance( itV, iNr );
  if ( itV != m_vectVertices.end() )
  {
    m_vectVertices.erase( itV );
  }

}



/*-AddFace--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

size_t CMesh::AddFace( CFace& Face )
{

  m_vectFaces.push_back( Face );

  return m_vectFaces.size() - 1;

}



void CMesh::RemoveFace( CFace* pFace )
{

  if ( pFace == NULL )
  {
    return;
  }
  tVectFaces::iterator    it( m_vectFaces.begin() );
  while ( it != m_vectFaces.end() )
  {
    if ( &(*it) == pFace )
    {
      m_vectFaces.erase( it );
      return;
    }

    ++it;
  }

}



