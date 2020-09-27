#pragma once

#include <GR/GRTypes.h>

#include <list>
#include <map>

#include "X2dRenderer.h"
#include "X2dFont.h"
#include "XViewport.h"
#include "XTextureSection.h"


#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#undef DrawText
#endif


namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}

class IDebugService;



struct XBasic2dRenderer : public GR::Service::ServiceImpl<X2dRenderer>
{

  protected:

    typedef std::list<XTexture*>                tTextures;
    typedef std::list<X2dFont*>                 tFonts;
    typedef std::list<XRendererDisplayMode>     tDisplayModes;

    GR::IEnvironment*         m_pEnvironment;

    GR::tPoint                m_DisplayOffset;
    GR::tPoint                m_VirtualSize;

    XViewport                 m_ViewPort;

    tTextures                 m_Textures;

    tFonts                    m_Fonts;

    tDisplayModes             m_DisplayModes;

#if OPERATING_SYSTEM == OS_WINDOWS
    // Window-spezifics
    HWND                      m_hwndViewport;

    HMENU                     m_hMenu;

    WINDOWPLACEMENT           m_wPlacement;
#endif

    GR::u32                   m_WindowedModeStyles;
    GR::u32                   m_WindowedModeExStyles;

    GR::tFPoint               m_DirectTexelMappingOffset;

    IDebugService*            m_pDebugger;



    void                      SortAndCleanDisplayModes();
    GR::String                AppPath( const GR::String& Path );

    void                      Log( const GR::String& System, const char* Format, ... );


  public:

    XBasic2dRenderer();
    virtual ~XBasic2dRenderer();


    virtual void              AddTexture( XTexture* pTexture );
    virtual void              AddFont( X2dFont* pFont );

    virtual void              ReleaseAllTextures();
    virtual void              DestroyTexture( XTexture* pTexture );
    virtual void              DestroyAllTextures();

    virtual void              RestoreAllTextures();

    virtual void              RestoreAllFonts();
    virtual void              DestroyAllFonts();


    virtual bool              Initialize( GR::u32 Width,
                                          GR::u32 Height,
                                          GR::u32 Depth,
                                          bool FullScreen,
                                          GR::IEnvironment& Environment ) = 0;

    virtual bool              Release() = 0;

    virtual bool              OnResized() = 0;

    virtual bool              IsReady() const = 0;

    virtual bool              BeginScene() = 0;
    virtual void              EndScene() = 0;
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL ) = 0;

    virtual bool              ToggleFullscreen() = 0;
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode ) = 0;
    virtual bool              IsFullscreen() = 0;

    virtual void              AdjustWindowForMode( bool Windowed );

    virtual bool              SetViewport( XViewport& Viewport ) = 0;
    virtual XViewport         Viewport();

    virtual GR::u32           Width() = 0;
    virtual GR::u32           Height() = 0;
    virtual GR::u32           Depth() = 0;

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat Format ) = 0;
    virtual XTexture*         CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat Format ) = 0;
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData ) = 0;
    virtual XTexture*         LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 ) = 0;
    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 ) = 0;

    virtual X2dFont*          LoadFont( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor = 0xff000000 );
    virtual X2dFont*          LoadFontSquare( const GR::String& FileName, GR::u32 Flags = X2dFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    virtual X2dFont*          CreateFontSquare();
    virtual void              DestroyFont( X2dFont* pFont );

    virtual void              RenderText( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void              RenderTextCentered( X2dFont* Font, int CenterX, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void              RenderTextRightAligned( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void              RenderText( X2dFont* Font, int X, int Y, const GR::String& Text,
                                          float ScaleX, float ScaleY, GR::u32 Color = 0xffffffff );

    virtual GR::Graphic::ImageData*   LoadAndConvert( const GR::String& FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor );

    virtual void              RenderRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color );

    virtual bool              SaveScreenShot( const GR::String& FileName ) = 0;

    virtual void              Offset( const GR::tPoint& NewOffset );
    GR::tPoint                Offset() const;

    virtual void              VirtualSize( const GR::tPoint& NewSize );
    virtual GR::tPoint        VirtualSize() const;

};
