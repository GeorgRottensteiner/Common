#pragma once

#include <GR/GRTypes.h>

#include <Grafik/ImageData.h>

#include <string>




class XMultiTexture
{

  public:

    GR::Graphic::eImageFormat    m_ImageFormat;

    GR::String     m_strLoadedFromFile;

    GR::tPoint      m_ptSize;

    GR::u32         m_dwColorKey;



    XMultiTexture() :
      m_ImageFormat( GR::Graphic::IF_UNKNOWN ),
      m_ptSize( 0, 0 ),
      m_dwColorKey( 0 )
    {
    }

    virtual ~XMultiTexture()
    {
    }

};
