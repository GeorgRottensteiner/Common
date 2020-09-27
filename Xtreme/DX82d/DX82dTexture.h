#ifndef XTREME_DX8_TEXTURE_H_
#define XTREME_DX8_TEXTURE_H_



#include <d3d8.h>

#include <Xtreme/XTexture.h>

#include <Grafik/ImageData.h>



class CDX8Texture : public XTexture
{

  public:

    DX82dRenderer*                    m_pRenderer;

    D3DFORMAT                         m_PixelFormat;

    LPDIRECT3DTEXTURE8                m_Surface;

    DWORD                             m_TransparentColor;

    GR::String                        m_FileName;

    GR::Graphic::ImageData            m_ImageData;


    CDX8Texture( DX82dRenderer* pRenderer );
    virtual ~CDX8Texture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME_DX8_TEXTURE_H_