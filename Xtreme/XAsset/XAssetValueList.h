#ifndef XASSET_VALUELIST_H
#define XASSET_VALUELIST_H

#include "XAsset.h"



namespace Xtreme
{

namespace Asset
{

  class XAssetValueList : public XAsset
  {

    public:

      std::map<GR::String,GR::String> m_Values;


      XAssetValueList()
      {
      }



      const GR::String& Value( const GR::String& Key )
      {
        std::map<GR::String,GR::String>::const_iterator   itK( m_Values.find( Key ) );
        if ( itK == m_Values.end() )
        {
          static GR::String    empty;
          return empty;
        }
        return itK->second;
      }



      const GR::String& Value( int Index )
      {
        if ( ( Index < 0 )
        ||   ( Index >= (int)m_Values.size() ) )
        {
          static GR::String    empty;
          return empty;
        }
        std::map<GR::String,GR::String>::const_iterator   itK( m_Values.begin() );
        std::advance( itK, Index );
        return itK->second;
      }



      const GR::String& ParamName( int Index )
      {
        if ( ( Index < 0 )
        ||   ( Index >= (int)m_Values.size() ) )
        {
          static GR::String    empty;
          return empty;
        }
        std::map<GR::String,GR::String>::const_iterator   itK( m_Values.begin() );
        std::advance( itK, Index );
        return itK->first;
      }



      int ParamCount() const
      {
        return (int)m_Values.size();
      }



      void SetValue( const GR::String& Key, const GR::String& Value )
      {
        m_Values[Key] = Value;
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


#endif // XASSET_VALUELIST_H