#ifndef GR_GAMEBASE_FLAG_LAYER_H
#define GR_GAMEBASE_FLAG_LAYER_H

#include <Game/2dLayer.h>

#include "ObjectLayer.h"

#include <map>
#include <set>
#include <string>


namespace GR
{
  namespace Gamebase
  {
    class FlagLayer : public ObjectLayer,
                      public GR::Gamebase::TileLayer2D<GR::u32>
    {
      public:

        GR::String           Tileset;

        GR::i32               TileSpacingX;
        GR::i32               TileSpacingY;



        FlagLayer();
        virtual ~FlagLayer();

        virtual GR::i32       DisplayWidth();
        virtual GR::i32       DisplayHeight();


    };
  }
}

#endif // GR_GAMEBASE_FLAG_LAYER_H
