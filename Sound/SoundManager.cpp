/*----------------------------------------------------------------------------+
 | Programmname       :3d-Engine über Direct3D (Textures)                     |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 +----------------------------------------------------------------------------*/



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include "SoundManager.h"

#include <DX8/DX8Viewer.h>



CSoundManager::CSoundManager()
{
}



CSoundManager& CSoundManager::Instance()
{

  static CSoundManager      g_Instance;


  return g_Instance;

}



CSoundResource* CSoundManager::Create( const GR::String& strName, bool bLoadAs3d )
{

  CSoundResource*   pResource = new CSoundResource( strName, bLoadAs3d );

  Insert( strName, pResource );

  return pResource;

}
