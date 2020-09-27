#ifndef _RESOURCEMANAGER_H_
#define _RESOURCEMANAGER_H_

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

#include <Interface/IResourceManager.h>
#include <Interface/IService.h>



/*-Typedefs-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

class CMasterResourceManager : public GR::IService
{

  protected:


    typedef std::map<GR::String,IResourceManager*>   tMapManagers;


    tMapManagers                  m_mapManager;


  public:


    static CMasterResourceManager&      Instance();

    IResource*                    Request( const GR::String& strName );
    void                          AddManager( const GR::String& strName, IResourceManager* pResourceManager );

    virtual void                  OnSet();

    void                          Clear();


    bool                          empty() const
    {
      return m_mapResourcen.empty();
    }

    int                           size() const
    {
      return (int)m_mapResourcen.size();
    }

};



#endif //_RESOURCEMANAGER_H_