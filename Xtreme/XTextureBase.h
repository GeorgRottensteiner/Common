#pragma once

#include <Xtreme/XTexture.h>

#include <Grafik/ImageData.h>

#include <string>
#include <String/GRstring.h>



class XTextureBase : public XTexture
{

  public:

    std::list<GR::String>             FileNames;

    bool                              AllowUsageAsRenderTarget;



    XTextureBase() :
      AllowUsageAsRenderTarget( false )
    {
    }

};
