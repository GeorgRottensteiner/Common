#include "XCess.h"
#include "XSMesh.h"

#include <Xtreme/XMesh.h>


CXSMesh::CXSMesh( GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( NULL )
{
  m_eShader = XRenderer::ST_FLAT;
}



CXSMesh::CXSMesh( const char* szMeshName, GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( NULL )
{
  m_eShader = XRenderer::ST_FLAT;

  Mesh( szMeshName );
}



CXSMesh::CXSMesh( XMesh* pMesh, GR::u32 xsFlags ) :
  CXSObject( xsFlags ),
  m_pVertexBuffer( NULL ),
  m_pMesh( pMesh )
{
  if ( m_pMesh )
  {
    m_pMesh->CalculateBoundingBox();
    m_BoundingBox = m_pMesh->m_BoundingBox;
  }
  m_eShader = XRenderer::ST_FLAT;
}



CXSMesh::~CXSMesh()
{
}



void CXSMesh::Render( XRenderer& Renderer )
{

  CXSObject::Render( Renderer );
  if ( ( m_pVertexBuffer == NULL )
  &&   ( m_pMesh ) )
  {
    m_pVertexBuffer = Renderer.CreateVertexBuffer( *m_pMesh );
  }
  if ( m_pVertexBuffer )
  {
    Renderer.SetTexture( 0, m_pTexture );
    Renderer.RenderVertexBuffer( m_pVertexBuffer );
    //Viewer.DisplayBoundingBox( BoundingBox(), 0xff00ff00 );
  }

}



const XBoundingBox& CXSMesh::BoundingBox()
{

  return m_BoundingBox;

}



void CXSMesh::Mesh( XMesh* pMesh )
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

  CXSMeshResource&    xsMesh = CXCess::Instance().m_MeshManager.Request( strMeshName );

  Mesh( xsMesh.Mesh() );

}



void CXSMesh::Update( const float fElapsedTime )
{
}


