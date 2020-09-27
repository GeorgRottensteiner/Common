#ifndef GR_GAMEBASE_LAYER_H
#define GR_GAMEBASE_LAYER_H

#include <string>

#include <GR/GRTypes.h>



namespace GR
{
  namespace Gamebase
  {
    class LayeredMap;

    class Layer
    {
      public:

        enum LayerType
        {
          LT_TILE_LAYER,
          LT_COLLISION_LAYER,
          LT_FREE_SECTION_LAYER,
          LT_FLAG_LAYER
        };

        enum LayerVisibility
        {
          LV_OPAQUE         = 0,        // abdeckend
          LV_ALPHA_BIT      = 1,        // alpha bit (color key)
          LV_ALPHA          = 2,        // full alpha
          LV_INVISIBLE      = 3         // not shown (collision layer)
        };


        GR::String              Name;

        LayerType               Type;

        bool                    RelativeSpeed;
        bool                    Colorkeyed;
        GR::u32                 ColorKey;
        bool                    Tinted;
        GR::u32                 TintColor;

        GR::tPoint              Offset;
        GR::f32                 Rotation;
        GR::tPoint              RotationOffset;

        LayerVisibility         Visibility;

        LayeredMap*             m_pMap;


        Layer();
        virtual ~Layer();


        virtual GR::i32         DisplayWidth();
        virtual GR::i32         DisplayHeight();

    };
  }
}


#endif // GR_GAMEBASE_LAYER_H
