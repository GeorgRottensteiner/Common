#ifndef GFX_HELPER_H
#define GFX_HELPER_H



#include <GR/GRTypes.h>



namespace GFX
{

  GR::u32           ColorFromRGB( GR::u8 R, GR::u8 G, GR::u8 B );
  void              GetTriplets( GR::u32 Source, int& R, int& G, int& B );
  GR::u32           ColorAdjustBrightness( GR::u32 Source, GR::u32 Factor = 100 );
  GR::u32           ColorGradient( GR::u32 Color1, GR::u32 Color2, int Percentage );

  // Farbkonvertierer

  // Fast bedeutet ungenau (Bits werden vergessen)
  GR::u32           ColorConvertShiftApart555( GR::u16 Color );
  GR::u32           ColorConvertShiftApart565( GR::u16 Color );

  GR::u32           ColorConvert32to555( GR::u32 Color );
  GR::u32           ColorConvert32to565( GR::u32 Color );

  GR::u32           ColorConvert555to32( GR::u16 Color );
  GR::u32           ColorConvert565to32( GR::u16 Color );

  GR::u32           AlphaFade( GR::u32 Color1, GR::u32 Color2, int Alpha );

  GR::u32           Modulate( const GR::u32 Color1, const GR::u32 Color2 );

  GR::u32           RGBToBGR( GR::u32 Color );
}


#endif // GFX_HELPER_H



