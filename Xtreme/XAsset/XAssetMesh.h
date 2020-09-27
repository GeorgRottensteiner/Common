#ifndef XASSET_MESH_H
#define XASSET_MESH_H


#include <Xtreme/XMesh.h>

#include "XAsset.h"


class XVertexBuffer;

namespace Xtreme
{

namespace Asset
{

  class XAssetMesh : public XAsset
  {

    public:

      XMesh                       m_Mesh;

      XVertexBuffer*              m_pVertexBuffer;

      XAssetMesh( const XMesh& Mesh ) :
        m_Mesh( Mesh ),
        m_pVertexBuffer( NULL )
      {
      }

      XMesh& Mesh()
      {
        return m_Mesh;
      }

      XVertexBuffer* VertexBuffer()
      {
        return m_pVertexBuffer;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( GR::Strings::CompareCaseInsensitive( Name, "VertexBuffer" ) == 0 )
        {
          return (GR::up)m_pVertexBuffer;
        }
        else if ( GR::Strings::CompareCaseInsensitive( Name, "Mesh" ) == 0 )
        {
          return (GR::up)&m_Mesh;
        }
        else if ( GR::Strings::CompareCaseInsensitive( Name, "BoundingBox" ) == 0 )
        {
          return (GR::up)&m_Mesh.m_BoundingBox;
        }
        return 0;
      }

  };

};

};


#endif // XASSET_MESH_H