#ifndef XASSET_SOUND_H
#define XASSET_SOUND_H


#include "XAsset.h"

#include <String/StringUtil.h>



namespace Xtreme
{

namespace Asset
{

  class XAssetSound : public XAsset
  {

    public:

      GR::u32                     m_SoundHandle;

      XAssetSound( const GR::u32 Handle = 0 ) :
        m_SoundHandle( Handle )
      {
      }

      GR::u32                     Handle()
      {
        return m_SoundHandle;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( GR::Strings::CompareCaseInsensitive( Name, "Sound" ) == 0 )
        {
          return m_SoundHandle;
        }
        return 0;
      }

  };

};

};


#endif // XASSET_SOUND_H