#ifndef XCESS_H
#define XCESS_H


#include <Interface/IResourceManager.h>

#include "XSObject.h"
#include "XSMesh.h"
#include "XSMeshResource.h"

#include "SceneGraph.h"



class CXCess
{

  public:

    IResourceManager<GR::String,CXSMeshResource>     m_MeshManager;


    CSceneGraph                 m_Scene;

    CRenderManager              m_RenderManager;


    virtual ~CXCess()
    {
    }

    static CXCess& Instance()
    {
      static CXCess  g_Instance;

      return g_Instance;
    }


  protected:

    CXCess()
    {
    }



};

#endif// XCESS_H

