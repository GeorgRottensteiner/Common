#pragma once

#include <winsock.h>
#include <map>

#include <GR/GRTypes.h>



namespace GR
{

  class ProxyDetect
  {

    public:


      static GR::String             DetectFromURL( const GR::String& URL );

  };


}
