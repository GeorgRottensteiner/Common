#include "FlagLayer.h"



namespace GR
{
  namespace Gamebase
  {
    FlagLayer::FlagLayer() :
      TileSpacingX( 32 ),
      TileSpacingY( 32 )
    {
      Type = LT_FLAG_LAYER;
    }



    FlagLayer::~FlagLayer()
    {
    }



    GR::i32 FlagLayer::DisplayWidth()
    {
      return GR::Gamebase::TileLayer2D<GR::u32>::Width() * TileSpacingX;
    }



    GR::i32 FlagLayer::DisplayHeight()
    {
      return GR::Gamebase::TileLayer2D<GR::u32>::Height() * TileSpacingY;
    }

  }
}