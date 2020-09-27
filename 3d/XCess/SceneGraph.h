#ifndef _CSCENEGRAPH_H
#define _CSCENEGRAPH_H


#include <Interface/IRenderAble.h>

#include <DX8/Frustum.h>

#include "RenderManager.h"
#include "SceneNode.h"



class CSceneGraph : public CSceneNode
{

  public:

    CRenderManager              m_RenderManager;


    CSceneGraph() :
      CSceneNode( "Scene" )
    {
    }

    virtual ~CSceneGraph() 
    {
    }

    void Render( CD3DViewer& Viewer )
    {
      m_RenderManager.Render( Viewer );
    }

    void BuildQueue( CFrustum& Frustum )
    {
      m_RenderManager.ClearQueue();
      ToQueue( m_RenderManager, Frustum );
    }

    CXSObject*  PickObject( CD3DViewer& Viewer, int iX, int iY, math::vector3& vPickPoint )
    {
      D3DXVECTOR3     v1,v2;

      D3DXMATRIX    matIdentity;
      D3DXMatrixIdentity( &matIdentity );
      Viewer.SetTransform( D3DTS_WORLD, &matIdentity );

      Viewer.CastRayFromScreen( iX, iY, v1, v2 );

      math::vector3   vm1( v1.x, v1.y, v1.z );
      math::vector3   vm2( v2.x, v2.y, v2.z );

      return CSceneNode::PickObject( vm1, vm2, vPickPoint );
    }


};

#endif// _CSCENEGRAPH_H

