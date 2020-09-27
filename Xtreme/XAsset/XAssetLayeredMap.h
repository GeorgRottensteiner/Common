#ifndef XASSET_LAYERED_MAP_H
#define XASSET_LAYERED_MAP_H

#include "XAsset.h"

#include <Interface/IAnimationManager.h>

#include <vector>
#include <string>


namespace Xtreme
{

namespace Asset
{

  class XAssetLayeredMap : public XAsset
  {

    public:

      GR::String        Name;
      GR::String        File;



      XAssetLayeredMap()
      {
      }



      const GR::String& MapName()
      {
        return Name;
      }



      const GR::String& MapFile()
      {
        return File;
      }



      virtual GR::up Handle( const char* Name )
      {
        return 0;
      }



      virtual bool Release()
      {
        return true;
      }

  };

}

}


#endif // XASSET_LAYERED_MAP_H