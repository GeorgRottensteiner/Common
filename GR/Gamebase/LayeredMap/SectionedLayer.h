#ifndef GR_GAMEBASE_SECTIONED_LAYER_H
#define GR_GAMEBASE_SECTIONED_LAYER_H

#include "ObjectLayer.h"



namespace GR
{
  namespace Gamebase
  {
    class SectionedLayer : public ObjectLayer
    {
      public:

        SectionedLayer();
        ~SectionedLayer();


        void Resize( int Width, int Height );
    };

  }
}

#endif // GR_GAMEBASE_SECTIONED_LAYER_H
