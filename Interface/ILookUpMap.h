#ifndef _ILOOKUPMAP_H_
#define _ILOOKUPMAP_H_

/*----------------------------------------------------------------------------+
 | Programmname       : FormatManager                                         |
 +----------------------------------------------------------------------------+
 | Autor              :                                                       |
 | Datum              :                                                       |
 | Version            :                                                       |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#pragma warning ( disable : 4786 )
#include <map>
#include <string>

#include <Interface/IResource.h>
#include <time.h>



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

template <class T> class ILookUpMap
{

  protected:


    typedef std::map<GR::String,T>   tResourceMap;


    tResourceMap      m_mapResourcen;


  public:

    virtual T            Request( const GR::String& strName )
    {
      tResourceMap::iterator   it( m_mapResourcen.find( strName ) );
      if ( it == m_mapResourcen.end() )
      {
        return T();
      }
      return it->second;
    }

    void                  Insert( const GR::String& strName, T pResource )
    {
      m_mapResourcen[strName] = pResource;
    }

    void                  Remove( T pResource )
    {
      tResourceMap::iterator   it( m_mapResourcen.begin() );
      while ( it != m_mapResourcen.end() )
      {
        if ( it->second == pResource )
        {
          m_mapResourcen.erase( it );
          return;
        }
        ++it;
      }
    }

    void                  DeleteAllEntries()
    {
      tResourceMap::iterator   it( m_mapResourcen.begin() );
      while ( it != m_mapResourcen.end() )
      {
        T    pEntry = it->second;

        delete pEntry;

        ++it;
      }
      m_mapResourcen.clear();
    }

    // sucht den nächsten passenden Eintrag nach diesem Key, zum "Durchiterieren"
    GR::String FindNextEntry( const GR::String& strKey )
    {
      if ( m_mapResourcen.empty() )
      {
        return GR::String( "" );
      }
      tResourceMap::iterator   it( m_mapResourcen.find( strKey ) );
      if ( it != m_mapResourcen.end() )
      {
        ++it;
        if ( it != m_mapResourcen.end() )
        {
          return it->first;
        }
      }
      return m_mapResourcen.begin()->first;
    }

    bool              empty() const
    {
      return m_mapResourcen.empty();
    }
    int               size() const
    {
      return m_mapResourcen.size();
    }

};



#endif //_ILOOKUPMAP_H_