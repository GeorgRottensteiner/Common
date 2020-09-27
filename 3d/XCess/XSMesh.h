#ifndef XSMESH_H
#define XSMESH_H


#include "XSObject.h"

#include <3d/MeshObject.h>


class CXSMesh : public CXSObject
{

  protected:

    CMesh*                              m_pMesh;

    CVertexBuffer*                      m_pVertexBuffer;


  public:


    CXSMesh( GR::u32 xsFlags = XS_DEFAULT );
    CXSMesh( const char* szMeshName, GR::u32 xsFlags = XS_DEFAULT );
    CXSMesh( CMesh* pMesh, GR::u32 xsFlags = XS_DEFAULT );
    virtual ~CXSMesh();

    virtual void                Render( CD3DViewer& Viewer );

    const CBoundingBox&         BoundingBox();

    void                        Mesh( CMesh* pMesh );
    void                        Mesh( const GR::String& strMeshName );
    virtual void                Update( const float fElapsedTime );

};

#endif // XSMESH_H

