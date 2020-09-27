#ifndef XASSET_VALUETABLE_H
#define XASSET_VALUETABLE_H

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetValueTable : public XAsset
  {

    public:

      std::vector<GR::String>    m_Values;


      XAssetValueTable()
      {
      }



      const GR::String& Value( size_t Index )
      {
        if ( Index >= m_Values.size() )
        {
          static GR::String    empty;
          return empty;
        }
        return m_Values[Index];
      }



      void SetValue( size_t Index, const GR::String& Value )
      {
        if ( Index >= m_Values.size() )
        {
          m_Values.resize( Index + 1 );
        }
        m_Values[Index] = Value;
      }



      size_t ValueCount()
      {
        return m_Values.size();
      }



      virtual bool                Release()
      {
        m_Values.clear();
        return false;
      }



      virtual GR::up Handle( const char* Name )
      {
        return 0;
      }

  };

}

}


#endif // XASSET_VALUETABLE_H