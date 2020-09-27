#ifndef XASSET_IMAGESECTION_H
#define XASSET_IMAGESECTION_H


#include <Xtreme/XTextureSection.h>

#include <Grafik/ContextDescriptor.h>

#include "XAsset.h"

#include <String/StringUtil.h>



namespace Xtreme
{

  namespace Asset
  {

    class XAssetImageSection : public XAsset
    {

      public:

        XTextureSection             m_tsImage;

        GR::Graphic::ContextDescriptor    m_Image;


        XAssetImageSection( XTextureSection& tsImage ) :
          m_tsImage( tsImage )
        {
        }

        XTextureSection             Section()
        {
          return m_tsImage;
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
          if ( GR::Strings::CompareCaseInsensitive( Name, "Section" ) == 0 )
          {
            return (GR::up)&m_tsImage;
          }
          return 0;
        }

    };

  }

}


#endif // XASSET_IMAGESECTION_H