#ifndef XASSET_FONT_H
#define XASSET_FONT_H

#include <ctype.h>

#include <Xtreme/XFont.h>

#include "XAsset.h"



namespace Xtreme
{

  namespace Asset
  {

    class XAssetFont : public XAsset
    {

      public:

        XFont*                      m_pFont;

        XAssetFont( XFont* pFont ) :
          m_pFont( pFont )
        {
        }

        XFont*                      Font()
        {
          return m_pFont;
        }

        virtual bool                Release()
        {
          return false;
        }

        virtual GR::up              Handle( const char* Name )
        {
          if ( ( Name != NULL )
          &&   ( strlen( Name ) == 4 )
          &&   ( toupper( Name[0] ) == 'F' )
          &&   ( toupper( Name[1] ) == 'O' )
          &&   ( toupper( Name[2] ) == 'N' )
          &&   ( toupper( Name[3] ) == 'T' ) )
          {
            return (GR::up)m_pFont;
          }
          return 0;
        }

    };

  }

}


#endif // XASSET_FONT_H