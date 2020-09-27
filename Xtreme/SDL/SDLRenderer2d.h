#pragma once


#include <SDL.h>

#include <vector>

#include <Xtreme/XBasic2dRenderer.h>
#include <Xtreme/XAsset/XAssetLoader.h>



class SDLRenderer2d : public XBasic2dRenderer
{

  private:

    bool                          m_IsReady;
    bool                          m_Windowed;
    bool                          m_VSyncEnabled;

    SDL_Renderer*                 m_pRenderer;

    int                           m_Width;
    int                           m_Height;



    // applies section flipping flags
    void                          RenderSDLSection( const XTextureSection& TS, SDL_Rect& Source, SDL_Rect& Target );



  public:


    SDLRenderer2d();
    virtual ~SDLRenderer2d();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual void                  ReleaseAssets();
    void                          UnloadAssets( Xtreme::Asset::eXAssetType Type );

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::Graphic::eImageFormat   ImageFormat();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  VSyncEnabled();
    virtual void                  EnableVSync( bool Enable );

    virtual bool                  SaveScreenShot( const GR::String& FileName );

    virtual bool                  IsReady() const;

    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual void                  VirtualSize( const GR::tPoint& NewSize );

    virtual void                  AdjustWindowForMode( bool Windowed );
    
    virtual bool                  SetViewport( XViewport& Viewport );

    virtual GR::u32               Depth();

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat Format );
    virtual XTexture*             CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat Format );
    virtual XTexture*             CreateTexture( const GR::Graphic::ImageData& ImageData );
    virtual XTexture*             LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );
    virtual bool                  CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData );

    virtual XMultiTexture*        LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0 );

    virtual void                  RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height, GR::u32 Color );
    virtual void                  RenderLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color );
    virtual void                  RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                                  GR::i32 X2, GR::i32 Y2,
                                                  GR::i32 X3, GR::i32 Y3,
                                                  GR::u32 Color );

    virtual void                  RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color );

    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y,
                                                    const XTextureSection& TexSection );
    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture );
    virtual void                  RenderTextureSectionColorized( GR::i32 X, GR::i32 Y,
                                                             const XTextureSection& TexSection,
                                                             GR::u32 Color );
    virtual void                  RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y,
                                                                  const XTextureSection& TexSection,
                                                                  GR::u32 Color );
    virtual void                  RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                                               const XTextureSection& TexSection,
                                                                               GR::u32 Color,
                                                                               float Angle, float ZoomX, float ZoomY );
    virtual void                  RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y,
                                                              const XTextureSection& TexSection,
                                                              GR::u32 ColorKey,
                                                              bool DoNotCreateNewIfNeeded );
    virtual void                  RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y,
                                                                       const XTextureSection& TexSection,
                                                                       GR::u32 ColorKey, GR::u32 Color,
                                                                       bool DoNotCreateNewIfNeeded );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y,
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue );
    virtual void                  RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y,
                                                                          const XTextureSection& TexSection,
                                                                          GR::i32 AlphaValue,
                                                                          GR::u32 ColorKey );

    virtual void                  RenderTextureSectionColorized( GR::i32 X, GR::i32 Y,
                                                             const XMultiTexture* pTexture,
                                                             GR::u32 Color );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y,
                                                                const XMultiTexture* pTexture,
                                                                GR::i32 AlphaValue );


    virtual void              RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void              RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void              RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY );
    virtual void              RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y,
                                                                           const XTextureSection& TexSection,
                                                                           float Angle, float ZoomX, float ZoomY,
                                                                           GR::u32 ColorKey = 0 );
    virtual void              RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y,
                                                                                    const XTextureSection& TexSection,
                                                                                    float Angle, float ZoomX, float ZoomY,
                                                                                    GR::u32 ColorKey = 0,
                                                                                    GR::u32 Color = 0xffffffff );
    virtual void              RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void              RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                                      const XTextureSection& TexSection, GR::u32 Color = 0xffffffff );
    virtual void              RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y,
                                                                                   float Angle, float ZoomX, float ZoomY,
                                                                                   const GR::u32 ColorKey,
                                                                                   const XTextureSection& TexSection, GR::u32 Color = 0xffffffff );
    virtual void              RenderTextureSectionMasked( GR::i32 X, GR::i32 Y,
                                                          const XTextureSection& TexSection,
                                                          GR::i32 MaskX, GR::i32 MaskY,
                                                          const XTextureSection& TexSectionMask,
                                                          const GR::u32 Color = 0xffffff,
                                                          const GR::u32 MaskColorKey = 0xff000000 );
    virtual void              RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                                GR::i32 MaskX, GR::i32 MaskY,
                                                const XTextureSection& TexSectionMask,
                                                const GR::u32 Color = 0xffffff,
                                                const GR::u32 MaskColorKey = 0xff000000 );


    int                       MapTextureFormat( GR::Graphic::eImageFormat Format );

};
