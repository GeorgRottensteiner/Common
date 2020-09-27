#include "TileLayer.h"



namespace GR
{
  namespace Gamebase
  {
    TileLayer::TileLayer() :
      TileSpacingX( 32 ),
      TileSpacingY( 32 )
    {
      Type = LT_TILE_LAYER;

      Clamp();
    }



    TileLayer::~TileLayer()
    {
    }



    GR::i32 TileLayer::DisplayWidth()
    {
      return GR::Gamebase::TileLayer2D<GR::u16>::Width() * TileSpacingX;
    }



    GR::i32 TileLayer::DisplayHeight()
    {
      return GR::Gamebase::TileLayer2D<GR::u16>::Height() * TileSpacingY;
    }

  }
}