#ifndef _pj_3d_util_h
#define _pj_3d_util_h

#include <3d/wavefront.h>
#include <DX8\DX8Object.h>
#include <debug/debugclient.h>

namespace d3
{

struct util
{

static CDX8Object *MakeDXObject( CWF& wf )
{
  CDX8Object* pObject = new CDX8Object();
  CDX8Frame*  pFrame = &pObject->m_mapFrames["Default"];

  int o = 0;

  pFrame->m_vectVertices.resize( wf.v_.size() - 1 );
  
  //- Anzahl der dreiecke noch nicht bekannt, da wf-faces beliebig viele ecken haben können
  pObject->m_vectFaces.reserve(   wf.f_.size() * 3 ); //- baustelle, muus optimiert werden
  
  {
    for ( int i = 0; i < wf.v_.size() - 1; ++i )
    {
      CDX8Vertex&  aVertex = pFrame->m_vectVertices[ i ];
      aVertex.vectPos = D3DXVECTOR3( wf.v_[i + 1].x, wf.v_[i + 1].y, wf.v_[i + 1].z );
    }

    for ( int j = 0; j < wf.f_.size(); ++j )
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
        
        for ( int p = 0; p < f.size() - 2; ++p )
        // for ( int p = 0; p < 1; ++p )
        {
          pObject->m_vectFaces.push_back( CDX8Face() );
          CDX8Face& aFace = pObject->m_vectFaces.back();
          aFace.m_dwVertex[0] = f[0  ].v_ - 1;
          aFace.m_dwVertex[1] = f[p+1].v_ - 1;
          aFace.m_dwVertex[2] = f[p+2].v_ - 1;
          
          CWF::vertex_t& v1 = wf.vn_[ f[0    ].vn_ ];
          CWF::vertex_t& v2 = wf.vn_[ f[p+1  ].vn_ ];
          CWF::vertex_t& v3 = wf.vn_[ f[p+2  ].vn_ ];
          aFace.vectNormal[0] = D3DXVECTOR3( v1.x, v1.y, v1.z );
          aFace.vectNormal[1] = D3DXVECTOR3( v2.x, v2.y, v2.z );
          aFace.vectNormal[2] = D3DXVECTOR3( v3.x, v3.y, v3.z );
    
          // D3DXVec3Normalize( &aFace.vectNormal[0], &aFace.vectNormal[0] );
          // D3DXVec3Normalize( &aFace.vectNormal[1], &aFace.vectNormal[1] );
          // D3DXVec3Normalize( &aFace.vectNormal[2], &aFace.vectNormal[2] );

          aFace.m_fTextureX[0] =     wf.vt_[ f[0  ].vt_ ].x;
          aFace.m_fTextureY[0] = 1 - wf.vt_[ f[0  ].vt_ ].y;
          aFace.m_fTextureX[1] =     wf.vt_[ f[p+1].vt_ ].x;
          aFace.m_fTextureY[1] = 1 - wf.vt_[ f[p+1].vt_ ].y;
          aFace.m_fTextureX[2] =     wf.vt_[ f[p+2].vt_ ].x;
          aFace.m_fTextureY[2] = 1 - wf.vt_[ f[p+2].vt_ ].y;
        }
      }
    }
  }

  return pObject;
}

};

} //end namespace d3

#endif