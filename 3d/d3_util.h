#ifndef _pj_3d_util_h
#define _pj_3d_util_h

#include <3d/wavefront.h>
#include <Xtreme/XMesh.h>
#include <debug/debugclient.h>

namespace d3
{

struct util
{

static XMesh MakeDXObject( CWF& wf )
{
  XMesh   object;

  int o = 0;

  object.m_Vertices.resize( wf._v.size() - 1 );
  
  //- Anzahl der dreiecke noch nicht bekannt, da wf-faces beliebig viele ecken haben können
  object.m_Faces.reserve(   wf.f_.size() * 3 ); //- baustelle, muus optimiert werden
  
  {
    for ( size_t i = 0; i < wf._v.size() - 1; ++i )
    {
      Mesh::Vertex&  aVertex( object.m_Vertices[ i ] );
      aVertex.Position = GR::tVector( wf._v[i + 1].x, wf._v[i + 1].y, wf._v[i + 1].z );
    }

    for ( size_t j = 0; j < wf.f_.size(); ++j )
    {
      CWF::face_t&  f     = wf.f_[j];

      if ( f.size() < 3 )
      {
        //- bischen wenig ecken!
        //dh() << dh::error << "wf-face " << j << " hat nur " << f.size() << " (wenig) ecken!\n";
      }
      else //- f.size() > 3
      {
        //- polygon mit mehr als 3 ecken - einen trianglefan erzeugen
        
        for ( size_t p = 0; p < f.size() - 2; ++p )
        // for ( int p = 0; p < 1; ++p )
        {
          object.m_Faces.push_back( Mesh::Face() );
          Mesh::Face& aFace = object.m_Faces.back();
          aFace.m_Vertex[0] = f[0  ]._v - 1;
          aFace.m_Vertex[1] = f[p+1]._v - 1;
          aFace.m_Vertex[2] = f[p+2]._v - 1;
          
          CWF::vertex_t& v1 = wf.vn_[ f[0    ].vn_ ];
          CWF::vertex_t& v2 = wf.vn_[ f[p+1  ].vn_ ];
          CWF::vertex_t& v3 = wf.vn_[ f[p+2  ].vn_ ];
          aFace.m_Normal[0] = GR::tVector( v1.x, v1.y, v1.z );
          aFace.m_Normal[1] = GR::tVector( v2.x, v2.y, v2.z );
          aFace.m_Normal[2] = GR::tVector( v3.x, v3.y, v3.z );
    
          // D3DXVec3Normalize( &aFace.vectNormal[0], &aFace.vectNormal[0] );
          // D3DXVec3Normalize( &aFace.vectNormal[1], &aFace.vectNormal[1] );
          // D3DXVec3Normalize( &aFace.vectNormal[2], &aFace.vectNormal[2] );

          aFace.m_TextureX[0] =     wf.vt_[ f[0  ].vt_ ].x;
          aFace.m_TextureY[0] = 1 - wf.vt_[ f[0  ].vt_ ].y;
          aFace.m_TextureX[1] =     wf.vt_[ f[p+1].vt_ ].x;
          aFace.m_TextureY[1] = 1 - wf.vt_[ f[p+1].vt_ ].y;
          aFace.m_TextureX[2] =     wf.vt_[ f[p+2].vt_ ].x;
          aFace.m_TextureY[2] = 1 - wf.vt_[ f[p+2].vt_ ].y;
        }
      }
    }
  }

  return object;
}

};

} //end namespace d3

#endif