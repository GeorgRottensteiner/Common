#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

/*----------------------------------------------------------------------------+
 | Programmname       :3d-Engine über Direct3D (Textures)                     |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <Sound/Sound.h>

#include <Interface/IResourceManager.h>

#include <Sound/SoundResource.h>


class CSoundManager : public IResourceManager<CSoundResource>
{

  public:


    static CSoundManager&     Instance();

    CSoundResource*           Create( const GR::String& strName,
                                      bool bLoadAs3d = false );


  private:

    CSoundManager();

};



#endif //_SOUND_MANAGER_H_