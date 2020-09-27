#include "XSObject.h"
#include "SceneNode.h"



CXSObject::CXSObject( GR::u32 xsFlags ) :
  m_pNode( NULL ),
  m_pTexture( NULL ),
  m_Flags( xsFlags ),
  m_dwItemData( NULL )
{
}



CXSObject::~CXSObject() 
{
}



void CXSObject::Texture( CDX8Texture* pTexture )
{
  m_pTexture = pTexture;
}



math::matrix4 CXSObject::Transformation()
{

  CSceneNode*       pParent = m_pNode;

  math::matrix4     matComplete;

  matComplete.Identity();

  while ( pParent )
  {
    matComplete *= pParent->Transform();
    
    pParent = pParent->m_pParent;
  }

  return matComplete;

}



CBoundingBox CXSObject::TransformedBoundingBox()
{

  CBoundingBox      BB = m_BoundingBox;

  BB.Transform( Transformation() );

  return BB;

}



CBoundingBox CXSObject::BoundingBox()
{

  CBoundingBox      BB = m_BoundingBox;

  CSceneNode*       pParent = m_pNode;

  math::matrix4     matComplete;

  matComplete.Identity();

  /*
  while ( pParent )
  {
    matComplete *= pParent->Transform().Inverse();
    //matComplete = pParent->Transform().Inverse() * matComplete;
    
    pParent = pParent->m_pParent;
  }
  */

  BB.Transform( matComplete );

  return BB;

}



void CXSObject::Update( const float fElapsedTime )
{
}



void CXSObject::ItemData( const GR::up dwItemData )
{

  m_dwItemData = dwItemData;

}



GR::u32 CXSObject::ItemData() const
{

  return m_dwItemData;

}



void CXSObject::ID( const CXSID& ID )
{

  m_ID = ID;

}



const CXSID& CXSObject::ID() const
{

  return m_ID;

}



void CXSObject::ModifyFlags( const GR::u32 dwAdd, const GR::u32 dwRemove )
{

  m_Flags &= ~dwRemove;
  m_Flags |= dwAdd;

}



GR::u32 CXSObject::Flags() const
{

  return m_Flags;

}



void CXSObject::ToQueue( CRenderManager& RManager, CFrustum& Frustum )
{

  RManager.AddToQueue( this );

}



void CXSObject::Render( CD3DViewer& Viewer )
{

  m_pNode->PreRender( Viewer );

}