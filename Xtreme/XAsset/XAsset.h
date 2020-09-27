#ifndef XASSET_H
#define XASSET_H


#include <GR/GRTypes.h>

#include <Interface/IIOStream.h>


namespace Xtreme
{

namespace Asset
{

  class XAsset
  {

    public:

      virtual bool                Release() = 0;

      virtual GR::up              Handle( const char* Name ) = 0;


      virtual ~XAsset()
      {
      }

  };

};

};


#endif // XASSET_H