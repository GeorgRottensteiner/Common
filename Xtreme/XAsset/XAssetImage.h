#ifndef XASSET_IMAGE_H
#define XASSET_IMAGE_H


#include <Xtreme/XTexture.h>

#include <Grafik/ContextDescriptor.h>

#include <String/StringUtil.h>

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetImage : public XAsset
  {

    public:

      XTexture*                           m_pTexture;

      GR::Graphic::ContextDescriptor      m_Image;



      XAssetImage( XTexture* pTexture ) :
        m_pTexture( pTexture )
      {
      }

      XTexture*                   Texture()
      {
        return m_pTexture;
      }

      const GR::Graphic::ContextDescriptor& CD()
      {
        return m_Image;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( GR::Strings::CompareCaseInsensitive( Name, "Texture" ) == 0 )
        {
          return (GR::up)m_pTexture;
        }
        return 0;
      }

  };

};

};


#endif // XASSET_IMAGE_H