#ifndef _PJ_RESSOURCEMANAGER_H_INCLUDED
#define _PJ_RESSOURCEMANAGER_H_INCLUDED
#pragma warning (disable:4786)

#include <map>
#include <string>
#pragma warning (disable:4786)

namespace PJ
{
  typedef std::map< GR::String, void* >           generic_map;
  typedef std::map< GR::String, void* >::iterator generic_map_it;

  template < class T >
  class CGenericRessourceManager
  {
  private:

    generic_map m_theRessourceMap;

  public:

    generic_map& _GetMap() { return m_theRessourceMap; }

    GR::String _DumpMap()
    {
      GR::String strDump;
      generic_map_it it( m_theRessourceMap.begin() );
      for( ; it != m_theRessourceMap.end(); ++it )
      {
        strDump += it->first;
        strDump += "\n";
      }
      return strDump
    }

    T* Store( const char* szName, T* pPointer )
    {
      //- name schon vorhanden? ressource bereits geladen?
      generic_map_it it( m_theRessourceMap.find( GR::String( szName ) ) );
      if ( it != m_theRessourceMap.end() )
      {
        if ( (T*)(it->second) != pPointer )
        { //- gibts schon - alte ressource verwerfen
          delete (T*)(it->second);
          m_theRessourceMap.erase( it );
        }
      }

      //- neue Ressource aufnehmen
      m_theRessourceMap[ GR::String( szName ) ] = (void*)(pPointer);
      return pPointer;
    }

    T* Recall( const char* szName )
    {
      T* pPointer = NULL;
      //- name vorhanden? ressource geladen?
      generic_map_it it( m_theRessourceMap.find( GR::String( szName ) ) );
      if ( it != m_theRessourceMap.end() )
      {
        //- gefunden
        pPointer = (T*)(it->second);
      }

      return pPointer;
    }


    void Erase( const char* szName )
    {
      //- name vorhanden? ressource geladen?
      generic_map_it it( m_theRessourceMap.find( GR::String( szName ) ) );
      if ( it != m_theRessourceMap.end() )
      {
        //- gefunden
        if ( (T*)(it->second) )
        {
          delete (T*)(it->second);
        }
        m_theRessourceMap.erase( it );
      }
    }


    //- Ressourcen freigeben
    ~CGenericRessourceManager()
    {
      generic_map_it it( m_theRessourceMap.begin() );
      for( ; it != m_theRessourceMap.end(); ++it )
      {
        if ( (T*)(it->second) )
        {
          delete (T*)(it->second);
        }
      }
    }
  };

  template < class K, class T >
  class CRessourceManager
  {
  private:

    std::map< K, T* > m_theRessourceMap;

  public:

    std::map< K, T* >& _GetMap() { return m_theRessourceMap; }


    T* Store( const K& id, T* pPointer )
    {
      //- name schon vorhanden? ressource bereits geladen?
      std::map< K, T* >::iterator it( m_theRessourceMap.find( id ) );
      if ( it != m_theRessourceMap.end() )
      {
        if ( it->second != pPointer )
        { //- gibts schon - alte ressource verwerfen
          delete it->second;
          m_theRessourceMap.erase( it );
        }
      }

      //- neue Ressource aufnehmen
      m_theRessourceMap[ id ] = pPointer;
      return pPointer;
    }

    T* Recall( const K& id )
    {
      T* pPointer = NULL;
      //- name vorhanden? ressource geladen?
      std::map< K, T* >::iterator it( m_theRessourceMap.find( id ) );
      if ( it != m_theRessourceMap.end() )
      {
        //- gefunden
        pPointer = it->second;
      }

      return pPointer;
    }


    void Erase( const K& id )
    {
      //- name vorhanden? ressource geladen?
      std::map< K, T* >::iterator it( m_theRessourceMap.find( id ) );
      if ( it != m_theRessourceMap.end() )
      {
        //- gefunden
        delete it->second;
        m_theRessourceMap.erase( it );
      }
    }


    //- Ressourcen freigeben
    ~CRessourceManager()
    {
      std::map< K, T* >::iterator it( m_theRessourceMap.begin() );
      for( ; it != m_theRessourceMap.end(); ++it )
      {
        delete it->second;
      }
    }
  };
};



/*
Beispiel:

#include <PJLib\PJ_RessourceManager.h>
#pragma warning (disable:4786)

using namespace PJ;

class CDummyClass
{
public:
  char* pChar;
  CDummyClass()
  {
    pChar = new char[100];
  }
  ~CDummyClass()
  {
    delete[] pChar;
  }
};

class CDoofiClass
{
  int bla;
};


#pragma warning (disable:4786)



CRessourceManager<CDummyClass> DummyMan;
CRessourceManager<CDoofiClass> DoofiMan;
CRessourceManager<int> IntMan;


void test()
{
  char szName[] = "blabla";

  CDummyClass* pDummy = new CDummyClass;
  DummyMan.Store( szName, pDummy );
  CDummyClass* pDummy2 = DummyMan.Recall( szName );

  CDoofiClass* pDoofi = new CDoofiClass;
  DoofiMan.Store( szName, pDoofi );
  CDoofiClass* pDoofi2 = DoofiMan.Recall( szName );

  int* pInt   = new int;
  IntMan.Store( szName, pInt );
  int* pInt2 = IntMan.Recall( szName );
}

*/

#endif// _PJ_RESSOURCEMANAGER_H_INCLUDED