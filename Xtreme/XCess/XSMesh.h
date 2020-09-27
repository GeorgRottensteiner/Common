#ifndef XSMESH_H
#define XSMESH_H


#include "XSObject.h"

#include <Xtreme/XMesh.h>


class CXSMesh : public CXSObject
{

  protected:

    XMesh*                              m_pMesh;

    XVertexBuffer*                      m_pVertexBuffer;


  public:


    CXSMesh( GR::u32 xsFlags = XS_DEFAULT );
    CXSMesh( const char* szMeshName, GR::u32 xsFlags = XS_DEFAULT );
    CXSMesh( XMesh* pMesh, GR::u32 xsFlags = XS_DEFAULT );
    virtual ~CXSMesh();

    virtual void                Render( XRenderer& Renderer );

    const XBoundingBox&         BoundingBox();

    void                        Mesh( XMesh* pMesh );
    void                        Mesh( const GR::String& strMeshName );
    virtual void                Update( const float fElapsedTime );

};

#endif // XSMESH_H

