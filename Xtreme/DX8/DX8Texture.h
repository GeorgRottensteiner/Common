#ifndef XTREME_DX8_TEXTURE_H_
#define XTREME_DX8_TEXTURE_H_



#include <d3d8.h>

#include <Xtreme/XTextureBase.h>


class CDX8Texture : public XTextureBase
{

  public:

    CDX8RenderClass*                  m_pRenderer;

    D3DFORMAT                         m_PixelFormat;

    LPDIRECT3DTEXTURE8                m_Surface;

    DWORD                             m_TransparentColor;

    GR::u32                           m_MipMapLevels;

    std::list<GR::String>         m_listFileNames;

    std::list<GR::Graphic::ImageData> m_StoredImageData;

    bool                              AllowUsageAsRenderTarget;
    bool                              RequiresRebuilding;


    CDX8Texture( CDX8RenderClass* pRenderer );
    virtual ~CDX8Texture();

    virtual bool                      Release();

    virtual bool                      RequiresRebuild();

};



#endif // XTREME_DX8_TEXTURE_H_