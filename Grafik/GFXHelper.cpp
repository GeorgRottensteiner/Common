#include <Grafik\GFXHelper.h>



namespace GFX
{
  GR::u32 ColorFromRGB( GR::u8 R, GR::u8 G, GR::u8 B )
  {
    return ( R << 16 ) | ( G << 8 ) | B;
  }



  void GetTriplets( GR::u32 Source, int& R, int& G, int& B )
  {
    R = ( ( Source & 0xff0000 ) >> 16 );
    G = ( ( Source & 0x00ff00 ) >> 8 );
    B =   ( Source & 0xff );
  }



  GR::u32 ColorAdjustBrightness( GR::u32 Source, GR::u32 Faktor )
  {
    int     r,
            g,
            b;

    GetTriplets( Source, r, g, b );

    r = ( ( Faktor * r ) / 100 );
    g = ( ( Faktor * g ) / 100 );
    b = ( ( Faktor * b ) / 100 );

    if ( r > 255 )
    {
      r = 255;
    }
    if ( g > 255 )
    {
      g = 255;
    }
    if ( b > 255 )
    {
      b = 255;
    }

    return ( r << 16 ) + ( g << 8 ) + b + ( Source & 0xff000000 );
  }



  GR::u32 ColorGradient( GR::u32 Color1, GR::u32 Color2, int Percentage )
  {
    int   r[3],
          g[3],
          b[3],
          a[3];


    a[0] = ( ( Color1 & 0xff000000 ) >> 24 );
    r[0] = ( ( Color1 & 0xff0000 ) >> 16 );
    g[0] = ( ( Color1 & 0x00ff00 ) >> 8 );
    b[0] =   ( Color1 & 0xff );
    a[1] = ( ( Color2 & 0xff000000 ) >> 24 );
    r[1] = ( ( Color2 & 0xff0000 ) >> 16 );
    g[1] = ( ( Color2 & 0x00ff00 ) >> 8 );
    b[1] =   ( Color2 & 0xff );

    r[2] = ( ( r[0] * ( 100 - Percentage ) ) + ( r[1] * Percentage ) ) / 100;
    g[2] = ( ( g[0] * ( 100 - Percentage ) ) + ( g[1] * Percentage ) ) / 100;
    b[2] = ( ( b[0] * ( 100 - Percentage ) ) + ( b[1] * Percentage ) ) / 100;
    a[2] = ( ( a[0] * ( 100 - Percentage ) ) + ( a[1] * Percentage ) ) / 100;

    return ( ( a[2] << 24 ) + ( r[2] << 16 ) + ( g[2] << 8 ) + b[2] );
  }



  GR::u32 ColorConvertShiftApart555( GR::u16 wColor )
  {
    return    ( (GR::u32)( wColor & 0x7c00 ) << 10 ) 
            + ( (GR::u32)( wColor & 0x03e0 ) << 5 ) 
            +   (GR::u32)( wColor & 0x001f );
  }



  GR::u32 ColorConvertShiftApart565( GR::u16 wColor )
  {
    return    ( (GR::u32)( wColor & 0xf800 ) << 9 ) 
            + ( (GR::u32)( wColor & 0x07c0 ) << 4 ) 
            +   (GR::u32)( wColor & 0x001f );
  }



  GR::u32 ColorConvert555to32( GR::u16 wColor )
  {
    return (GR::u32)( ( ( ( ( wColor & 0x7c00 ) >> 10 ) * 255 / 31 ) << 16 )
                 + ( ( ( ( wColor & 0x03e0 ) >>  5 ) * 255 / 31 ) << 8 )
                 + ( ( ( ( wColor & 0x001f )       ) * 255 / 31 ) ) );
  }



  GR::u32 ColorConvert565to32( GR::u16 wColor )
  {
    return (GR::u32)( ( ( ( ( wColor & 0xf800 ) >> 11 ) * 255 / 31 ) << 16 )
                 + ( ( ( ( wColor & 0x07e0 ) >>  5 ) * 255 / 63 ) << 8 )
                 + ( ( ( ( wColor & 0x001f )       ) * 255 / 31 ) ) );
  }



  GR::u32 AlphaFade( GR::u32 Color1, GR::u32 Color2, int Alpha )
  {
    GR::u32     Result = 0xff000000;

    GR::u8*     pSrc = (GR::u8*)&Color1;
    GR::u8*     pTgt = (GR::u8*)&Color2;
    GR::u8*     pRst = (GR::u8*)&Result;

    if ( ( Color1 == Color2 )
    ||   ( Alpha == 0 ) )
    {
      return Color2;
    }
    if ( Alpha == 255 )
    {
      return Color1;
    }

    // Alpha
    *(GR::u8*)pRst++ = (GR::u8)( ( ( *pSrc++ * Alpha ) + *pTgt++ * ( 255 - Alpha ) ) / 255 );
    // Rot
    *(GR::u8*)pRst++ = (GR::u8)( ( ( *pSrc++ * Alpha ) + *pTgt++ * ( 255 - Alpha ) ) / 255 );
    // Grün
    *(GR::u8*)pRst++ = (GR::u8)( ( ( *pSrc++ * Alpha ) + *pTgt++ * ( 255 - Alpha ) ) / 255 );
    // Blau
    *(GR::u8*)pRst++ = (GR::u8)( ( ( *pSrc++ * Alpha ) + *pTgt++ * ( 255 - Alpha ) ) / 255 );

    return Result;
  }



  GR::u32 Modulate( const GR::u32 Color1, const GR::u32 Color2 )
  {
    GR::u32     Result = 0xff000000;

    GR::u8*     pSrc = (GR::u8*)&Color1;
    GR::u8*     pTgt = (GR::u8*)&Color2;
    GR::u8*     pRst = (GR::u8*)&Result;

    if ( Color2 == 0xffffffff )
    {
      return Color1;
    }
    if ( Color1 == 0xffffffff )
    {
      return Color2;
    }

    // Alpha
    *(GR::u8*)pRst++ = (GR::u8)( ( *pSrc++ * *pTgt++ ) / 255 );
    // Rot
    *(GR::u8*)pRst++ = (GR::u8)( ( *pSrc++ * *pTgt++ ) / 255 );
    // Grün
    *(GR::u8*)pRst++ = (GR::u8)( ( *pSrc++ * *pTgt++ ) / 255 );
    // Blau
    *(GR::u8*)pRst++ = (GR::u8)( ( *pSrc++ * *pTgt++ ) / 255 );

    return Result;
  }



  GR::u32 RGBToBGR( GR::u32 Color )
  {
    return  ( ( Color & 0xff0000 ) >> 16 )  // swap R and B
          | ( ( Color & 0xff ) << 16 )
          | ( Color & 0xff00ff00 );         // keep alpha and G
  }



}
