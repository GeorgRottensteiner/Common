#ifndef _DX8_TEXTURE_H_
#define _DX8_TEXTURE_H_
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#include <d3d8.h>

#include <DX8/DX8Types.h>

#include <Interface/IResourceManager.h>



class CD3DViewer;

class CDX8Texture : public IResource
{

  public:

    CD3DViewer*                       m_pViewer;

    D3DFORMAT                         m_d3dfPixelFormat;

    LPDIRECT3DTEXTURE8                m_Surface;

    tSize                             m_vSize;

    DWORD                             m_dwCreationFlags,
                                      m_dwTransparentColor,
                                      m_dwResourceID,
                                      m_dwMipmapLevels,

                                      m_dwReferenzen;

    GR::String                       m_strFileName,
                                      m_strResourceType;

    HINSTANCE                         m_hResourceInstance;


    CDX8Texture( const GR::String& strFileName = "",
                 DWORD dwCreationFlags = DX8::TF_DEFAULT,
                 DWORD dwTransparentColor = 0xff000000,
                 DWORD dwMipMapLevels = 1 );
    ~CDX8Texture();

    bool                              RecreateSurface();

    bool                              HasAlpha() const;

    virtual bool                      Load();
    virtual bool                      Release();
    virtual bool                      Destroy();

};



#endif // _DX8_TEXTURE_H_