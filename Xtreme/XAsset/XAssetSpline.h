#ifndef XASSET_SPLINE_H
#define XASSET_SPLINE_H

#include <Math/Spline.h>

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetSpline : public XAsset
  {

    public:

      CSpline                     m_Spline;



      XAssetSpline( const CSpline& Spline = CSpline() ) :
        m_Spline( Spline )
      {
      }

      const CSpline&  Spline()
      {
        return m_Spline;
      }

      virtual bool                Release()
      {
        return false;
      }

      virtual GR::up              Handle( const char* Name )
      {
        if ( GR::Strings::CompareCaseInsensitive( Name, "Spline" ) == 0 )
        {
          return (GR::up)&m_Spline;
        }
        return 0;
      }


  };

};

};


#endif // XASSET_SPLINE_H