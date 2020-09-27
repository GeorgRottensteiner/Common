#ifndef XTREME_DX11_TEXTURE_H
#define XTREME_DX11_TEXTURE_H



#include <d3d11.h>

#include <Xtreme/XTextureBase.h>


class DX11Texture : public XTextureBase
{

  public:

    DXGI_FORMAT                       m_D3DFormat;

    ID3D11Texture2D*                  m_pTexture;
    ID3D11ShaderResourceView*         m_pTextureShaderResourceView;
    ID3D11RenderTargetView*           m_pTextureRenderTargetView;

    DWORD                             m_TransparentColor;

    GR::u32                           m_MipMapLevels;

    std::list<GR::Graphic::ImageData> m_StoredImageData;



    DX11Texture();
    virtual ~DX11Texture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_DX11_TEXTURE_H