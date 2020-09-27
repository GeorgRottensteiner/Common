#pragma once


#include <Grafik/GFXPage.h>
#include <Grafik/GDIPage.h>
#include <Grafik/DDrawPage.h>
#include <Grafik/Image.h>
#include <Grafik/RLEList.h>
#include <Grafik/ContextDescriptor.h>

#include <Xtreme/XBasic2dRenderer.h>
#include <Xtreme/Xtreme2d/XMultiTexture2d.h>



class CDDrawTexture;

class CDDrawRenderClass : public XBasic2dRenderer
{

  public:


    CDDrawRenderClass( HINSTANCE hInstance = NULL );
    virtual ~CDDrawRenderClass();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual void                  ReleaseAssets();

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::u32               Depth();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  SaveScreenShot( const GR::String& FileName );

    virtual bool                  IsReady() const;

    virtual bool                  SetViewport( XViewport& Viewport );

    virtual XTexture*             LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );
    virtual XTexture*             CreateTexture( const GR::Graphic::ImageData& ImageData );
    virtual XTexture*             CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat Format );

    virtual XMultiTexture*        LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0 );

    virtual X2dFont*              LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor = 0xff000000 );
    virtual X2dFont*              LoadFontSquare( const GR::String& FileName, GR::u32 Flags = X2dFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    virtual void                  DestroyFont( X2dFont* pFont );
    virtual void                  DestroyAllFonts();


    virtual void                  RenderLine( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color );

    virtual void                  RenderQuad( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                              GR::u32 Color );
    virtual void                  RenderTriangle( GR::i32 X1, GR::i32 Y1,
                                                  GR::i32 X2, GR::i32 Y2,
                                                  GR::i32 X3, GR::i32 Y3,
                                                  GR::u32 Color );

    virtual void                  RenderQuadMasked( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                                    GR::i32 MaskX, GR::i32 MaskY, 
                                                    const XTextureSection& TexSectionMask, 
                                                    const GR::u32 Color = 0xffffff,
                                                    const GR::u32 MaskColorKey = 0xff000000 );

    virtual void                  RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y, const XMultiTexture* pTexture );
    virtual void                  RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                                 const XMultiTexture* pTexture,
                                                                 GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                    const XMultiTexture* pTexture,
                                                                    GR::i32 AlphaValue );

    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void                  RenderTextureSectionWithAlphaLayer( GR::i32 X, GR::i32 Y, 
                                                                      const XTextureSection& TexSection,
                                                                      GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSectionWithAlphaLayerRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                      const XTextureSection& TexSection,
                                      GR::u32 Color,
                                      float Angle, float ZoomX, float ZoomY );
    virtual void                  RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSectionColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                  const XTextureSection& TexSection,
                                                                  GR::u32 ColorKey = 0,
                                                                  bool DoNotCreateNewIfNeeded = true );
    virtual void                  RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                                           const XTextureSection& TexSection,
                                                                           GR::u32 ColorKey = 0, GR::u32 Color = 0xffffffff,
                                                                           bool DoNotCreateNewIfNeeded = true );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                    const XTextureSection& TexSection,
                                                                    GR::i32 AlphaValue );
    virtual void                  RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue,
                                                                GR::u32 ColorKey = 0 );
    virtual void                  RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void                  RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection );
    virtual void                  RenderTextureSectionRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY );
    virtual void                  RenderTextureSectionRotatedZoomedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY,
                                                                 GR::u32 ColorKey = 0 );
    virtual void                  RenderTextureSectionRotatedZoomedColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                                 const XTextureSection& TexSection,
                                                                 float Angle, float ZoomX, float ZoomY,
                                                                 GR::u32 ColorKey = 0,
                                                                 GR::u32 Color = 0xffffffff );

    virtual void                  RenderTextureSectionAdditive( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection );
    virtual void                  RenderTextureSectionAdditiveColorKeyed( GR::i32 X, GR::i32 Y, const GR::u32 ColorKey,
                                                                          const XTextureSection& TexSection, GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSectionAdditiveColorKeyedRotatedZoomed( GR::i32 X, GR::i32 Y, 
                                                                          float Angle, float ZoomX, float ZoomY,
                                                                          const GR::u32 ColorKey,
                                                                          const XTextureSection& TexSection, GR::u32 Color = 0xffffffff );

    virtual void                  RenderText( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color );
    virtual void                  RenderText( X2dFont* Font, int X, int Y, const GR::String& Text, 
                                              float ScaleX, float ScaleY, GR::u32 Color );

    virtual void                  RenderTextureSectionMasked( GR::i32 X, GR::i32 Y, 
                                                              const XTextureSection& TexSection,
                                                              GR::i32 MaskX, GR::i32 MaskY, 
                                                              const XTextureSection& TexSectionMask, 
                                                              const GR::u32 Color = 0xffffff,
                                                              const GR::u32 MaskColorKey = 0xff000000 );

  private:

    struct tRLEEntry
    {
      GR::Graphic::RLEList            rleList;
      GR::Graphic::ContextDescriptor  cdAlphaMask;
    };

    struct tFontRLEKey
    {
      GR::u32         Letter;
      X2dFont*        pFont;
      bool operator< ( const tFontRLEKey& right ) const
      {
        if ( pFont != right.pFont )
        {
          return ( pFont < right.pFont );
        }
        if ( Letter != right.Letter )
        {
          return ( Letter < right.Letter );
        }
        return false;
      }
    };

    struct tColorKeyEntry
    {
      GR::u32         ColorKey;
      XTextureSection Section;

      bool operator< ( const tColorKeyEntry& right ) const
      {
        if ( ColorKey != right.ColorKey )
        {
          return ( ColorKey < right.ColorKey );
        }
        if ( Section.m_pTexture != right.Section.m_pTexture )
        {
          return ( Section.m_pTexture < right.Section.m_pTexture );
        }
        if ( Section.m_XOffset != right.Section.m_XOffset )
        {
          return ( Section.m_XOffset < right.Section.m_XOffset );
        }
        if ( Section.m_YOffset != right.Section.m_YOffset )
        {
          return ( Section.m_YOffset < right.Section.m_YOffset );
        }
        if ( Section.m_Width != right.Section.m_Width )
        {
          return ( Section.m_Width < right.Section.m_Width );
        }
        if ( Section.m_Height != right.Section.m_Height )
        {
          return ( Section.m_Height < right.Section.m_Height );
        }
        return false;
      }

    };


    typedef std::map<tColorKeyEntry,GR::Graphic::RLEList>   tRLELists;
    typedef std::map<tFontRLEKey,tRLEEntry>     tFontRLELists;
    typedef std::list<XMultiTexture2d*>         tMultiTextures;
    typedef std::map<tColorKeyEntry,XTexture*>  tColorkeyedTextures;;

    tRLELists                     m_RLELists;
    tFontRLELists                 m_FontRLELists;
    tColorkeyedTextures           m_ColorkeyedTextures;

    tMultiTextures                m_MultiTextures;

    HINSTANCE                     m_hInstance;

    GR::Graphic::GDIPage*		      m_pWinGPage;

    GR::Graphic::DirectXPage*     m_pDirectXPage;

    DWORD                         m_CreationWidth,
                                  m_CreationHeight,
                                  m_CreationDepth;

    bool                          m_Ready,
                                  m_Windowed,
                                  m_ForceWindowSize;

    GR::i32                       m_ForcedWindowWidth,
                                  m_ForcedWindowHeight,
                                  m_RenderWidth,
                                  m_RenderHeight,
                                  m_Depth;

    WNDPROC                       m_OldWndProc;

    GR::String                    m_SaveScreenShotFileName;

    XRendererDisplayMode          m_FullscreenDisplayMode;


    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );
    virtual bool                  CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData );

    virtual void                  RestoreAllTextures();

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat Format );

    bool                          CreatePage();
    bool                          ReleasePage();

    void                          ConvertTextureFormat( XTexture* pTexture, GR::Graphic::eImageFormat Format );

    void                          PreCacheFont( X2dFont* pFont, GR::u32 TransparentColor = 0xff000000 );

    GR::Graphic::GFXPage*         CurrentPage();

    bool                          ResizeSurface();

    void                          DoSaveSnapShot( const GR::String& FileName );

    static LRESULT CALLBACK       DDrawWndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
    LRESULT CALLBACK              WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

    virtual void                  Offset( const GR::tPoint& NewOffset );

    bool                          ConvertFontFormat( X2dFont* pFont, GR::Graphic::eImageFormat Format );

    bool                          IsBitCompatible( const GR::Graphic::eImageFormat Format1, const GR::Graphic::eImageFormat Format2 );


    void                          AdjustPosBySectionFlags( int& X, int& Y, const XTextureSection& Section );
};
