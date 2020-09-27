#ifndef XHUGETEXTURE_2D_H
#define XHUGETEXTURE_2D_H

#include <Xtreme/XTextureSection.h>
#include <Xtreme/X2dRenderer.h>

#include <GR/Gamebase/XFrameEvent.h>

#include <Interface/IEventProducer.h>

#include <vector>



class XHugeTexture : public EventListener<GR::Gamebase::tXFrameEvent>
{

  public:

    typedef std::vector<XTextureSection>      tVectTextures;


    tVectTextures             m_Textures;

    X2dRenderer*              m_pRenderer;

    IEventProducer<GR::Gamebase::tXFrameEvent>*   m_pEventProducer;

    GR::String               m_Filename;
    GR::Graphic::ImageData    m_ImageSource;

    int                       m_SplitWidth,
                              m_SplitHeight,

                              m_SplitTilesX,
                              m_SplitTilesY;

    GR::u32                   m_ColorKey;

    bool                      m_ColorKeyed;

    GR::u32                   m_ColorizeColor;


    XHugeTexture( X2dRenderer* pRenderer = NULL, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer = NULL );
    ~XHugeTexture();

    void                      Initialize( X2dRenderer* pRenderer = NULL, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer = NULL );
    bool                      Empty() const;

    bool                      LoadImage( const char* szFileName, int iSplitWidth = 256, int iSplitHeight = 256, GR::u32 ColorKey = 0, GR::u32 ColorizeColor = -1 );
    bool                      LoadImageData( GR::Graphic::ImageData& ImageData, int iSplitWidth = 256, int iSplitHeight = 256, GR::u32 ColorKey = 0, GR::u32 ColorizeColor = -1 );
    void                      Clear();

    bool                      Restore( GR::Graphic::ImageData* pCDData );


    void                      Render2d( X2dRenderer& Renderer, int iX = 0, int iY = 0 );
    void                      Render2dColorKeyed( X2dRenderer& Renderer, GR::u32 ColorKey, int iX = 0, int iY = 0 );

    virtual bool              ProcessEvent( const GR::Gamebase::tXFrameEvent& Event );

};


#endif // XHUGETEXTURE_2D_H
