#include "XCess.h"
#include "XSMesh.h"

#include <3d/MeshObject.h>


CXSMesh::CXSMesh( GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( NULL )
{
  m_eShader = GR::ST_FLAT;
}



CXSMesh::CXSMesh( const char* szMeshName, GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( NULL )
{
  m_eShader = GR::ST_FLAT;

  Mesh( szMeshName );
}



CXSMesh::CXSMesh( CMesh* pMesh, GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( pMesh )
{
  if ( m_pMesh )
  {
    m_pMesh->CalculateBoundingBox();
    m_BoundingBox = m_pMesh->m_BoundingBox;
  }
  m_eShader = GR::ST_FLAT;
}



CXSMesh::~CXSMesh()
{
}



void CXSMesh::Render( CD3DViewer& Viewer )
{

  CXSObject::Render( Viewer );
  if ( ( m_pVertexBuffer == NULL )
  &&   ( m_pMesh ) )
  {
    m_pVertexBuffer = m_pMesh->CreateVertexBuffer( Viewer );
  }
  if ( m_pVertexBuffer )
  {
    Viewer.SetTexture( 0, m_pTexture );
    m_pVertexBuffer->Display();

    //Viewer.DisplayBoundingBox( BoundingBox(), 0xff00ff00 );
  }

}



const CBoundingBox& CXSMesh::BoundingBox()
{

  return m_BoundingBox;

}



void CXSMesh::Mesh( CMesh* pMesh )
{

  m_pMesh = pMesh;
  if ( m_pMesh )
  {
    m_pMesh->CalculateBoundingBox();
    m_BoundingBox = m_pMesh->m_BoundingBox;
  }

}



void CXSMesh::Mesh( const GR::String& strMeshName )
{

  CXSMeshResource    xsMesh = CXCess::Instance().m_MeshManager.Request( strMeshName );

  Mesh( xsMesh.Mesh() );

}



void CXSMesh::Update( const float fElapsedTime )
{
}


