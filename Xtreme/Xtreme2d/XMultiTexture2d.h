#pragma once

#include <Xtreme/XMultiTexture.h>

#include <Xtreme/X2dRenderer.h>

#include <vector>



class XMultiTexture2d : public XMultiTexture
{

  public:

    typedef std::vector<XTextureSection>      tVectTextures;


    tVectTextures             m_Textures;

    X2dRenderer*              m_pRenderer;

    GR::String                m_Filename;

    int                       m_SplitWidth,
                              m_SplitHeight,

                              m_SplitTilesX,
                              m_SplitTilesY;


    XMultiTexture2d( X2dRenderer* pRenderer = NULL );
    virtual ~XMultiTexture2d();

    void                      Initialize( X2dRenderer* pRenderer = NULL );
    bool                      Empty() const;

    bool                      LoadImage( GR::IEnvironment& Environment, const GR::String& FileName, int SplitWidth = 256, int SplitHeight = 256 );
    void                      Clear();

    bool                      Restore( GR::Graphic::ImageData* pCDData );


    void                      Render2d( X2dRenderer& Renderer, int X = 0, int Y = 0, const GR::u32 Color = 0xffffffff ) const;

};
