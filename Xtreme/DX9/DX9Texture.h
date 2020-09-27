#ifndef XTREME_DX9_TEXTURE_H_
#define XTREME_DX9_TEXTURE_H_



#include <d3d9.h>

#include <Xtreme/XTextureBase.h>


class CDX9Texture : public XTextureBase
{

  public:

    D3DFORMAT                         m_d3dfPixelFormat;

    LPDIRECT3DTEXTURE9                m_Surface;

    DWORD                             m_TransparentColor;

    GR::u32                           m_MipMapLevels;

    std::list<GR::Graphic::ImageData> m_StoredImageData;



    CDX9Texture();
    virtual ~CDX9Texture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_DX9_TEXTURE_H_