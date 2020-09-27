#ifndef XTREME2D_DX8_TEXTURE_H_
#define XTREME2D_DX8_TEXTURE_H_



#include <Grafik/ContextDescriptor.h>

#include <Xtreme/XTexture.h>

#include <Grafik/RLEList.h>


class CDDrawTexture : public XTexture
{

  public:

    CDDrawRenderClass*                m_pRenderer;

    GR::Graphic::ContextDescriptor    m_cdImage;
    GR::Graphic::ContextDescriptor    m_cdAlphaLayer;

    GR::Graphic::RLEList              m_rleList;

    GR::u32                           m_TransparentColor;

    GR::String                       m_strFileName;


    CDDrawTexture( CDDrawRenderClass* pRenderer );
    virtual ~CDDrawTexture();

    virtual bool                      Release();
    virtual bool                      RequiresRebuild();

};



#endif // XTREME2D_DX8_TEXTURE_H_