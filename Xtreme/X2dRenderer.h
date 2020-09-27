#pragma once

#include <GR/GRTypes.h>

#include <Lang/Service.h>

#include "XDisplayMode.h"
#include "XTexture.h"
#include "XMultiTexture.h"
#include "X2dFont.h"
#include "XViewport.h"
#include "XTextureSection.h"



namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



class X2dRenderer : public GR::IService
{

  public:

    enum eImageDisplayMethod
    {
      IMG_PLAIN,
      IMG_COLORKEY,
      IMG_ALPHABLEND,
      IMG_ADDITIVE,
    };



  public:

    X2dRenderer()
    {
    }

    virtual ~X2dRenderer()
    {
    }


    virtual void              AddTexture( XTexture* pTexture ) = 0;
    virtual void              AddFont( X2dFont* pFont ) = 0;

    virtual void              ReleaseAllTextures() = 0;
    virtual void              DestroyTexture( XTexture* pTexture ) = 0;
    virtual void              DestroyAllTextures() = 0;

    virtual void              RestoreAllTextures() = 0;

    virtual void              RestoreAllFonts() = 0;
    virtual void              DestroyAllFonts() = 0;


    virtual bool              Initialize( GR::u32 Width, 
                                          GR::u32 Height, 
                                          GR::u32 Depth, 
                                          bool FullScreen,
                                          GR::IEnvironment& Environment ) = 0;

    virtual bool              Release() = 0;

    virtual void              ReleaseAssets() = 0;

    virtual bool              OnResized() = 0;

    virtual bool              IsReady() const = 0;

    virtual bool              BeginScene() = 0;
    virtual void              EndScene() = 0;
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL ) = 0;

    virtual bool              ToggleFullscreen() = 0;
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode ) = 0;
    virtual bool              IsFullscreen() = 0;

    virtual void              AdjustWindowForMode( bool Windowed ) = 0;

    virtual bool              SetViewport( XViewport& Viewport ) = 0;
    virtual XViewport         Viewport() = 0;

    virtual GR::u32           Width() = 0;
    virtual GR::u32           Height() = 0;
    virtual GR::u32           Depth() = 0;

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat Format ) = 0;
    virtual XTexture*         CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat Format ) = 0;
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData ) = 0;
    virtual XTexture*         LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 ) = 0;
    virtual XMultiTexture*    LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0 ) = 0;

    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 ) = 0;
    virtual bool              CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData ) = 0;

    virtual X2dFont*          LoadFont( const GR::String& FileName, GR::u32 Flags = 0, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual X2dFont*          LoadFontSquare( const GR::String& FileName, GR::u32 Flags = X2dFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual X2dFont*          CreateFontSquare() = 0;
    virtual void              DestroyFont( X2dFont* pFont ) = 0;

    virtual void              RenderText( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextCentered( X2dFont* Font, int CenterX, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextRightAligned( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff ) = 0;

    virtual void              RenderText( X2dFont* Font, int X, int Y, const GR::String& Text,
                                          float ScaleX, float ScaleY, GR::u32 Color = 0xffffffff ) = 0;

    virtual void              RenderLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color ) = 0;
    virtual void              RenderRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color ) = 0;

    virtual void              RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                          GR::u32 Color ) = 0;
    virtual void              RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                              GR::i32 X2, GR::i32 Y2,
                                              GR::i32 X3, GR::i32 Y3,
                                              GR::u32 Color ) = 0;

    virtual void              RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color = 0xffffffff ) = 0;

    virtual void              RenderTextureSection( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture ) = 0;
    virtual void              RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                             const XMultiTexture* pTexture,
                                                             GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                const XMultiTexture* pTexture,
                                                                GR::i32 AlphaValue ) = 0;

    virtual void              RenderTextureSection( GR::i32 X, GR::i32 Y, 
                                                    const XTextureSection& TexSection ) = 0;
    virtual void              RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                             const XTextureSection& TexSection,
                                                             GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y, 
                                                                  const XTextureSection& TexSection,
                                                                  GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                               const XTextureSection& TexSection,
                                                                               GR::u32 Color = 0xffffffff,
                                                                               float Angle = 0.0f, float ZoomX = 1.0f, float ZoomY = 1.0f ) = 0;
    virtual void              RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y, 
                                                              const XTextureSection& TexSection,
                                                              GR::u32 ColorKey = 0,
                                                              bool DoNotCreateNewIfNeeded = true ) = 0;
    virtual void              RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                              const XTextureSection& TexSection,
                                                              GR::u32 ColorKey = 0, GR::u32 Color = 0xffffffff,
                                                              bool DoNotCreateNewIfNeeded = true ) = 0;
    virtual void              RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue ) = 0;
    virtual void              RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue,
                                                                GR::u32 ColorKey = 0 ) = 0;

    virtual void              RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection ) = 0;
    virtual void              RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection ) = 0;
    virtual void              RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY ) = 0;
    virtual void              RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY,
                                                                 GR::u32 ColorKey = 0 ) = 0;
    virtual void              RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY,
                                                                 GR::u32 ColorKey = 0,
                                                                 GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection ) = 0;
    virtual void              RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                                      const XTextureSection& TexSection, GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                      float Angle, float ZoomX, float ZoomY,
                                                                      const GR::u32 ColorKey,
                                                                      const XTextureSection& TexSection, GR::u32 Color = 0xffffffff ) = 0;
    virtual void              RenderTextureSectionMasked( GR::i32 X, GR::i32 Y, 
                                                          const XTextureSection& TexSection,
                                                          GR::i32 MaskX, GR::i32 MaskY, 
                                                          const XTextureSection& TexSectionMask, 
                                                          const GR::u32 Color = 0xffffff,
                                                          const GR::u32 MaskColorKey = 0xff000000 ) = 0;
    virtual void              RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                                GR::i32 MaskX, GR::i32 MaskY, 
                                                const XTextureSection& TexSectionMask, 
                                                const GR::u32 Color = 0xffffff,
                                                const GR::u32 MaskColorKey = 0xff000000 ) = 0;

    virtual bool              SaveScreenShot( const GR::String& FileName ) = 0;

    virtual void              Offset( const GR::tPoint& NewOffset ) = 0;
    virtual GR::tPoint        Offset() const = 0;

    virtual void              VirtualSize( const GR::tPoint& NewOffset ) = 0;
    virtual GR::tPoint        VirtualSize() const = 0;

};
