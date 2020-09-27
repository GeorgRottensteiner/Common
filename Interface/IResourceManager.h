#ifndef IRESOURCEMANAGER_H
#define IRESOURCEMANAGER_H



#include <map>
#include <string>

#include <Interface/IResource.h>



template <class K,class T> class IResourceManager
{

  public:

    typedef std::map<K,T>   tResourceMap;


  protected:
    

    tResourceMap      m_mapResourcen;


  public:


    virtual ~IResourceManager()
    {
      Clear();
    }

    virtual T&            Request( const K& strName )
    {
      typename tResourceMap::iterator   it( m_mapResourcen.find( strName ) );
      if ( it == m_mapResourcen.end() )
      {
        static T    aT;
        return aT;
      }
      T& pResource = it->second;

      return pResource;
    }

    virtual const T&            RequestRef( const K& strName ) const
    {
      typename tResourceMap::const_iterator   it( m_mapResourcen.find( strName ) );
      if ( it == m_mapResourcen.end() )
      {
        static T    aT;

        return aT;
      }
      const T& pResource = it->second;

      return pResource;
    }

    tResourceMap& GetMap()
    {
      return m_mapResourcen;
    }

    const tResourceMap& GetMap() const
    {
      return m_mapResourcen;
    }

    T& operator[] ( const K& strName )
    {
      return Request( strName );
    }

    const T& operator[] ( const K& strName ) const
    {
      return RequestRef( strName );
    }

    void                  Insert( const K& Name, T pResource )
    {
      typename tResourceMap::iterator   it( m_mapResourcen.find( Name ) );
      if ( it != m_mapResourcen.end() )
      {
        dh::Log( "Overriding double Resource entry!" );
      }
      m_mapResourcen[Name] = pResource;
    }

    void                  Remove( T pResource )
    {
      typename tResourceMap::iterator   it( m_mapResourcen.begin() );
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

    bool                  Exists( const K& Key )
    {
      return m_mapResourcen.find( Key ) != m_mapResourcen.end();
    }

    void                  Clear()
    {
      m_mapResourcen.clear();
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



#endif // IRESOURCEMANAGER_H