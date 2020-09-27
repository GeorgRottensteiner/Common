#ifndef pj_d3dxutil_h
#define pj_d3dxutil_h

#pragma warning ( disable : 4786 ) //- nutzlose Warnung ausschalten
#include <GR/GRTypes.h>
#include <math/rectt.h>


namespace d3dxutil
{
  typedef math::rectt< math::vector3t<float> > rect3;
  
  template< class BOX >
  void drawbox( CD3DViewer& Viewer, const BOX& rect, DWORD color )
  {
    typedef D3DXVECTOR3 dxv3;
    BOX::vector_type size = rect.size();
    BOX::vector_type p1   = rect.position();
    BOX::vector_type p2   = rect.position() + rect.size();

    //- vertikale
    Viewer.DrawFreeLine( dxv3( p1.x , p1.y, p1.z ), dxv3( p1.x, p2.y, p1.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p1.x , p1.y, p2.z ), dxv3( p1.x, p2.y, p2.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p1.y, p1.z ), dxv3( p2.x, p2.y, p1.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p1.y, p2.z ), dxv3( p2.x, p2.y, p2.z  ), color, color );
    //- oben
    Viewer.DrawFreeLine( dxv3( p1.x , p2.y, p1.z ), dxv3( p1.x, p2.y, p2.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p1.x , p2.y, p2.z ), dxv3( p2.x, p2.y, p2.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p2.y, p2.z ), dxv3( p2.x, p2.y, p1.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p2.y, p1.z ), dxv3( p1.x, p2.y, p1.z  ), color, color );
    //- unten
    Viewer.DrawFreeLine( dxv3( p1.x , p1.y, p1.z ), dxv3( p1.x, p1.y, p2.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p1.x , p1.y, p2.z ), dxv3( p2.x, p1.y, p2.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p1.y, p2.z ), dxv3( p2.x, p1.y, p1.z  ), color, color );
    Viewer.DrawFreeLine( dxv3( p2.x , p1.y, p1.z ), dxv3( p1.x, p1.y, p1.z  ), color, color );
  }
}

#endif//pj_d3dxutil_h



