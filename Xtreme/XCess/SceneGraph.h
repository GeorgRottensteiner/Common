#ifndef _CSCENEGRAPH_H
#define _CSCENEGRAPH_H


#include <Interface/IRenderAble.h>

#include <Xtreme/XFrustum.h>

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

    void Render( XRenderer& Renderer )
    {
      m_RenderManager.Render( Renderer );
    }

    void BuildQueue( XFrustum& Frustum )
    {
      m_RenderManager.ClearQueue();
      ToQueue( m_RenderManager, Frustum );
    }

    CXSObject*  PickObject( XRenderer& Renderer, int iX, int iY, GR::tVector& vPickPoint )
    {
      GR::tVector     v1,v2;

      math::matrix4     matIdentity;

      matIdentity.Identity();
      Renderer.SetTransform( XRenderer::TT_WORLD, matIdentity );

      Renderer.CastRayFromScreen( iX, iY, v1, v2 );

      math::vector3   vm1( v1.x, v1.y, v1.z );
      math::vector3   vm2( v2.x, v2.y, v2.z );

      return CSceneNode::PickObject( vm1, vm2, vPickPoint );
    }


};

#endif// _CSCENEGRAPH_H

