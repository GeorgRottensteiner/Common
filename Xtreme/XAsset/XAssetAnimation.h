#ifndef XASSET_ANIMATION_H
#define XASSET_ANIMATION_H

#include <Interface/IAnimationManager.h>

#include <String/StringUtil.h>

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetAnimation : public XAsset
  {

    public:

      IAnimationManager<GR::String>::tAnimType        m_Anim;

      GR::u32                                         m_AnimID;



      XAssetAnimation( const IAnimationManager<GR::String>::tAnimType& Anim = IAnimationManager<GR::String>::tAnimType() ) :
        m_Anim( Anim ),
        m_AnimID( 0 )
      {
      }

      const IAnimationManager<GR::String>::tAnimType&  Animation()
      {
        return m_Anim;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( GR::Strings::CompareCaseInsensitive( Name, "Animation" ) == 0 )
        {
          return (GR::up)&m_Anim;
        }
        else if ( GR::Strings::CompareCaseInsensitive( Name, "AnimationID" ) == 0 )
        {
          return (GR::up)m_AnimID;
        }
        return 0;
      }

  };

}

}


#endif // XASSET_ANIMATION_H