#ifndef _gfx_util_h
#define _gfx_util_h

#include <dc_stack.h>

namespace gfx
{
  inline COLORREF RGB2COLORREF( unsigned long color )
  {
    return RGB( ( color & 0xff0000 ) >> 16, ( color & 0x00ff00 ) >> 8, color & 0x0000ff ); 
  }

  /////////////////////////////////////////////////////////////////////
  //-    gefüllte Polygone auf GFXPage malen
  /////////////////////////////////////////////////////////////////////
  template<typename IT> 
  void FillPolygonXY( CGfxPage& page, IT first, IT last, unsigned long color ) 
  { 
    COLORREF refColor = RGB2COLORREF( color ); 
    HDC      hdc      = page.GetDC(); 
    POINT    pt; 
    std::vector<POINT>    vectPoints; 
    dc_stack dcs( hdc );
    dcs.push( CreatePen( PS_SOLID, 1, refColor ) );
    dcs.push( CreateSolidBrush( refColor ) );
    for ( ; first != last; ++first ) 
    { 
      pt.x = first->x; 
      pt.y = first->y; 
      vectPoints.push_back( pt ); 
    } 
    Polygon( hdc, &vectPoints[0], vectPoints.size() ); 
    dcs.pop(2);
    page.ReleaseDC(); 
  } 
 
  template<typename IT> 
  void FillPolygonXZ( CGfxPage& page, IT first, IT last, unsigned long color ) 
  { 
    COLORREF refColor = RGB2COLORREF( color ); 
    HDC      hdc      = page.GetDC(); 
    POINT   pt; 
    std::vector<POINT> vectPoints; 
    dc_stack dcs( hdc );
    dcs.push( CreatePen( PS_SOLID, 1, refColor ) );
    dcs.push( CreateSolidBrush( refColor ) );
    for ( ; first != last; ++first ) 
    { 
      pt.x = first->x; 
      pt.y = first->z; 
      vectPoints.push_back( pt ); 
    } 
    Polygon( hdc, &vectPoints[0], vectPoints.size() ); 
    dcs.pop(2);
    page.ReleaseDC(); 
  } 

  template<typename IT> 
  void FillPolygonXY( CGfxPage& page, IT first, IT last, unsigned long color, float scale ) 
  { 
    COLORREF refColor = RGB2COLORREF( color ); 
    HDC      hdc      = page.GetDC(); 
    POINT    pt; 
    std::vector<POINT>    vectPoints; 
    dc_stack dcs( hdc );
    dcs.push( CreatePen( PS_SOLID, 1, refColor ) );
    dcs.push( CreateSolidBrush( refColor ) );
    for ( ; first != last; ++first ) 
    { 
      pt.x = first->x * scale; 
      pt.y = first->y * scale; 
      vectPoints.push_back( pt ); 
    } 
    Polygon( hdc, &vectPoints[0], vectPoints.size() ); 
    dcs.pop(2);
    page.ReleaseDC(); 
  } 

  template<typename IT> 
  void FillPolygonXZ( CGfxPage& page, IT first, IT last, unsigned long color, float scale ) 
  { 
    COLORREF refColor = RGB2COLORREF( color ); 
    HDC      hdc      = page.GetDC(); 
    POINT    pt; 
    std::vector<POINT>    vectPoints;  
    dc_stack dcs( hdc );
    dcs.push( CreatePen( PS_SOLID, 1, refColor ) );
    dcs.push( CreateSolidBrush( refColor ) );
    for ( ; first != last; ++first ) 
    { 
      pt.x = first->x * scale; 
      pt.y = first->z * scale; 
      vectPoints.push_back( pt ); 
    } 
    Polygon( hdc, &vectPoints[0], vectPoints.size() ); 
    dcs.pop(2);
    page.ReleaseDC(); 
  } 

  /////////////////////////////////////////////////////////////////////
  //-    nicht gefüllte Polygone auf GFXPage malen
  /////////////////////////////////////////////////////////////////////
  template < typename IT >
  void DrawPolygonXY( CGfxPage& page, IT first, IT last, unsigned long color )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
    {
      if ( first2 == last ) first2 = first0;
      page.Line( first->x, first->y, first2->x, first2->y, color );  
    }
  }

  template < typename IT >
  void DrawPolygonXY( CGfxPage& page, IT first, IT last, unsigned long color, float scale )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
    {
      if ( first2 == last ) first2 = first0;
      page.Line( first->x * scale, first->y * scale, first2->x * scale, first2->y * scale, color );  
    }
  }

  template < typename IT >
  void DrawPolygonXZ( CGfxPage& page, IT first, IT last, unsigned long color )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
    {
      if ( first2 == last ) first2 = first0;
      page.Line( first->x, first->z, first2->x, first2->z, color );  
    }
  }

  template < typename IT >
  void DrawPolygonXZ( CGfxPage& page, IT first, IT last, unsigned long color, float scale )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
    {
      if ( first2 == last ) first2 = first0;
      page.Line( first->x * scale, first->z * scale, first2->x * scale, first2->z * scale, color );  
    }
  }



  /////////////////////////////////////////////////////////////////////
  //-    Linienzuege auf GFXPage malen
  /////////////////////////////////////////////////////////////////////
  
  template < typename IT >
  void DrawLineXY( CGfxPage& page, IT first, IT last, unsigned long color )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
      if ( first2 != last )
        page.Line( first->x, first->y, first2->x, first2->y, color );  
  }

  template < typename IT >
  void DrawLineXY( CGfxPage& page, IT first, IT last, unsigned long color, float scale )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
      if ( first2 != last )
        page.Line( first->x * scale, first->y * scale, first2->x * scale, first2->y * scale, color );  
  }

  template < typename IT >
  void DrawLineXZ( CGfxPage& page, IT first, IT last, unsigned long color )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
      if ( first2 != last )
        page.Line( first->x, first->z, first2->x, first2->z, color );  
  }

  template < typename IT >
  void DrawLineXZ( CGfxPage& page, IT first, IT last, unsigned long color, float scale )
  {
    if ( first == last ) return;
    IT first0 = first, first2 = first;
    ++first2;
    for ( ; first != last; ++first, ++first2 )
      if ( first2 != last )
        page.Line( first->x * scale, first->z * scale, first2->x * scale, first2->z * scale, color );  
  }
}

#endif// _gfx_util_h
