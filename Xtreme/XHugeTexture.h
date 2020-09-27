#pragma once

#include <Xtreme/XTextureSection.h>
#include <Xtreme/XRenderer.h>

#include <GR/Gamebase/XFrameEvent.h>

#include <Interface/IEventProducer.h>

#include <vector>



class XHugeTexture : public EventListener<GR::Gamebase::tXFrameEvent>
{

  public:

    typedef std::vector<XTextureSection>      tTextures;
    
    
    tTextures                 m_Textures;

    XRenderer*                m_pRenderer;

    IEventProducer<GR::Gamebase::tXFrameEvent>*   m_pEventProducer;

    GR::String                m_Filename;

    int                       m_SplitWidth,
                              m_SplitHeight,
                              m_SplitTilesX,
                              m_SplitTilesY;


    XHugeTexture( XRenderer* pRenderer = NULL, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer = NULL );
    ~XHugeTexture();

    void                      Initialize( XRenderer* pRenderer = NULL, IEventProducer<GR::Gamebase::tXFrameEvent>* pEventProducer = NULL );

    bool                      LoadImage( const char* FileName, int SplitWidth = 256, int SplitHeight = 256 );
    bool                      SetImage( GR::Graphic::ImageData& Image, int SplitWidth = 256, int iSplitHeight = 256 );
    void                      Clear();

    bool                      Restore( GR::Graphic::ImageData* pImage );


    void                      Render2d( XRenderer& Renderer, int X = 0, int Y = 0, GR::u32 Color = 0xffffffff );

    virtual bool              ProcessEvent( const GR::Gamebase::tXFrameEvent& Event );

};
