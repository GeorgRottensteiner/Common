#ifndef XASSET_FONT2D_H
#define XASSET_FONT2D_H


#include <Xtreme/X2dFont.h>

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetFont2d : public XAsset
  {

    public:

      X2dFont*                    m_pFont;

      XAssetFont2d( X2dFont* pFont ) :
        m_pFont( pFont )
      {
      }

      X2dFont*                    Font()
      {
        return m_pFont;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( strcmp( Name, "Font" ) == 0 )
        {
          return (GR::up)m_pFont;
        }
        return 0;
      }

  };

};

};


#endif // XASSET_FONT2D_H