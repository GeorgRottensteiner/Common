#include "Layer.h"

#include <IO/FileStream.h>
#include <IO/FileChunk.h>
#include <Memory/MemoryStream.h>



namespace GR
{
  namespace Gamebase
  {
    Layer::Layer() :
      RelativeSpeed( false ),
      m_pMap( NULL ),
      Rotation( 0.0f ),
      Colorkeyed( false ),
      ColorKey( 0xffff00ff ),
      Tinted( false ),
      TintColor( 0xffffffff ),
      Visibility( LV_OPAQUE )
    {
    }

    Layer::~Layer()
    {
    }



    GR::i32 Layer::DisplayWidth()
    {
      return 0;
    }



    GR::i32 Layer::DisplayHeight()
    {
      return 0;
    }

  }
}
