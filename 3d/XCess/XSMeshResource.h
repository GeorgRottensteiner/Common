#ifndef XSMESH_RESOURCE_H
#define XSMESH_RESOURCE_H


#include <Interface/IResource.h>

#include <3d/MeshObjectLoader.h>

#include "XSMesh.h"



class CXSMeshResource : public IResource
{

  protected:

    CMesh*                        m_pMesh;

    GR::String                   m_strFileName;


  public:


    CXSMeshResource( const char* szFileName ) :
      m_strFileName( szFileName ),
      m_pMesh( NULL )
    {
    }

    virtual bool Load()
    {
      if ( m_pMesh )
      {
        return true;
      }
      m_pMesh = CMeshObjectLoader::LoadDX8Object( m_strFileName.c_str() );
      return ( m_pMesh != NULL );
    }

    virtual bool Release()
    {
      if ( m_pMesh )
      {
        delete m_pMesh;
        m_pMesh = NULL;
      }
      return true;
    }

    virtual bool        Destroy()
    {
      Release();
      return true;
    }

    CMesh* Mesh() const
    {
      return m_pMesh;
    }

};

#endif // XSMESH_RESOURCE_H

