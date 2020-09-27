#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

/*----------------------------------------------------------------------------+
 | Programmname       :3d-Engine über Direct3D (Textures)                     |
 +----------------------------------------------------------------------------+
 | Autor              :Georg Rottensteiner                                    |
 +----------------------------------------------------------------------------*/



/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <DX8\DX8Texture.h>

#include <Interface/IResourceManager.h>


class CD3DViewer;

class CTextureManager : public IResourceManager<GR::String,CDX8Texture*>
{

  public:


    static CTextureManager&   Instance();

    CDX8Texture*              Create( CD3DViewer& Viewer,
                                      const GR::String& strName,
                                      DWORD dwFlags = DX8::TF_DEFAULT,
                                      DWORD dwTransparentColor = 0xff000000,
                                      DWORD dwMipmapLevels = 1 );

    void                      RecreateTextures();
    void                      ReleaseTextures();


  private:

    CTextureManager();

};



#endif //_TEXTURE_MANAGER_H_