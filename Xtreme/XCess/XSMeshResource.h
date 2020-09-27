#ifndef XSMESH_RESOURCE_H
#define XSMESH_RESOURCE_H


#include <Interface/IResource.h>

#include <Xtreme/MeshFormate/Tao.h>

#include "XSMesh.h"



class CXSMeshResource : public IResource
{

  protected:

    XMesh*                        m_pMesh;

    GR::String                   m_strFileName;


  public:


    CXSMeshResource() :
      m_strFileName(),
      m_pMesh( NULL )
    {
    }

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
      m_pMesh = CMeshObjectLoader::LoadTaoObject( m_strFileName.c_str() );
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

    XMesh* Mesh() const
    {
      return m_pMesh;
    }

};

#endif // XSMESH_RESOURCE_H

