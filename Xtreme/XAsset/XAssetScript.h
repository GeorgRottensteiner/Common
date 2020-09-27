#ifndef XASSET_SCRIPT_H
#define XASSET_SCRIPT_H

#include <Math/Spline.h>

#include "XAsset.h"



namespace Xtreme
{

  namespace Asset
  {

    class XAssetScript : public XAsset
    {

      public:

        GR::String                 m_Filename;



        XAssetScript()
        {
        }



        const GR::String Filename()
        {
          return m_Filename;
        }

        virtual bool Release()
        {
          return false;
        }

        virtual GR::up Handle( const char* Name )
        {
          return 0;
        }

    };

  }

}


#endif // XASSET_SCRIPT_H