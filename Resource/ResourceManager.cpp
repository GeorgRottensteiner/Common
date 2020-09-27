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

#include <map>
#include <string>

#include <Resource/ResourceManager.h>



CMasterResourceManager& CMasterResourceManager::Instance()
{

  static CMasterResourceManager    g_Instance;

  return g_Instance;

}



IResource* CMasterResourceManager::Request( const GR::String& strName )
{

  tMapManagers::iterator   it( m_mapManager.begin() );
  while ( it != m_mapManager.end() )
  {
    IResourceManager*   pManager = it->second;

    IResource*          pRes = pManager->Request( strName );
    if ( pRes )
    {
      return pRes;
    }

    ++it;
  }
  return NULL;

}



void CMasterResourceManager::Clear()
{

  m_mapManager.clear();

}



void CMasterResourceManager::AddManager( const GR::String& strName, IResourceManager* pResourceManager )
{

  m_mapManager[strName] = pResourceManager;

}



void CMasterResourceManager::OnSet()
{
}