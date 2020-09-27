#pragma once

#include <GR/GRTypes.h>

#include <Grafik/ImageData.h>

#include <string>
#include <String/GRstring.h>



class XTexture
{

  public:

    GR::Graphic::eImageFormat   m_ImageFormat;

    GR::string                  m_LoadedFromFile;

    GR::tPoint                  m_ImageSourceSize;
    GR::tPoint                  m_SurfaceSize;

    GR::u32                     m_ColorKey;
    GR::u32                     m_ColorKeyReplacementColor;


    virtual bool                Release() = 0;

    // used for render target textures
    virtual bool                RequiresRebuild() = 0;



    XTexture() :
      m_ImageFormat( GR::Graphic::IF_UNKNOWN ),
      m_ImageSourceSize( 0, 0 ),
      m_SurfaceSize( 0, 0 ),
      m_ColorKey( 0 ),
      m_ColorKeyReplacementColor( 0 )
    {
    }

    virtual ~XTexture()
    {
    }

    

};
