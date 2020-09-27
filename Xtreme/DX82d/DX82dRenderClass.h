#pragma once


#include <D3D8.h>

#include <vector>

#include <Xtreme/XBasic2dRenderer.h>

#include <Xtreme/Xtreme2d/XMultiTexture2d.h>
#include <Xtreme/XAsset/XAssetLoader.h>


class IDebugService;

struct tD3DModeInfo
{
  DWORD       m_Width,
              m_Height,
              m_Behaviour;          // Hardware / Software / Mixed vertex processing
  D3DFORMAT   m_PixelFormat,        // Pixel format in this mode
              m_DepthStencilFormat; // Which depth/stencil format to use with this mode
};

struct tD3DDeviceInfo
{
    // Device data
    D3DDEVTYPE                  DeviceType;      // Reference, HAL, etc.

    D3DCAPS8                    d3dCaps;         // Capabilities of this device

    GR::String                  Name;

    bool                        CanDoWindowed;  // Whether this device can work in windowed mode

    // Modes for this device
    std::vector<tD3DModeInfo>   Modes;

    // Current state

    DWORD                       CurrentMode;

    bool                        Windowed;

    D3DMULTISAMPLE_TYPE         MultiSampleType;
};

struct tD3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8      d3dAdapterIdentifier;

    D3DDISPLAYMODE              d3ddmDesktop;      // Desktop display mode for this adapter

    // Devices for this adapter
    std::vector<tD3DDeviceInfo> Devices;

    // Current state
    DWORD                       CurrentDevice;
};



class DX82dRenderer : public XBasic2dRenderer
{

  public:


    DX82dRenderer( HINSTANCE hInstance = NULL );
    virtual ~DX82dRenderer();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool FullScreen, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual void                  ReleaseAssets();
    void                          UnloadAssets( Xtreme::Asset::eXAssetType Type );

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::u32               Depth();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  SaveScreenShot( const GR::String& FileName );

    virtual bool                  IsReady() const;
    LPDIRECT3DDEVICE8             Device();

    virtual bool                  SetViewport( XViewport& Viewport );

    virtual XTexture*             LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );
    virtual XTexture*             CreateTexture( const GR::Graphic::ImageData& ImageData );
    virtual XTexture*             CreateTexture( GR::u32 Width, GR::u32 Height, GR::Graphic::eImageFormat imgFormat );
    virtual void                  SetTexture( GR::u32 Stage, XTexture* pTexture );

    virtual XMultiTexture*        LoadMultiTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0 );

    virtual X2dFont*              LoadFontSquare( const GR::String& FileName, GR::u32 Flags = X2dFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );


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

    virtual void                  RenderImage( GR::i32 X, GR::i32 Y, const XTextureSection& TexSection, GR::u32 Color );
    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y,
                                                    const XMultiTexture* pTexture );
    virtual void                  RenderTextureSectionColorized( GR::i32 X, GR::i32 Y, 
                                                             const XMultiTexture* pTexture,
                                                             GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                const XMultiTexture* pTexture,
                                                                GR::i32 AlphaValue );

    virtual void                  RenderTextureSection( GR::i32 X, GR::i32 Y, 
                                                        const XTextureSection& TexSection );
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
                                                              bool bDoNotCreateNewIfNeeded = true );
    virtual void                  RenderTextureSectionColorKeyedColorized( GR::i32 X, GR::i32 Y, 
                                                              const XTextureSection& TexSection,
                                                              GR::u32 ColorKey = 0, GR::u32 Color = 0xffffffff,
                                                              bool bDoNotCreateNewIfNeeded = true );
    virtual void                  RenderTextureSectionAlphaBlendedColorKeyed( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue,
                                                                GR::u32 ColorKey = 0 );
    virtual void                  RenderTextureSectionAlphaBlended( GR::i32 X, GR::i32 Y, 
                                                                const XTextureSection& TexSection,
                                                                GR::i32 AlphaValue );
    virtual void                  RenderTextureSectionHMirrored( GR::i32 X, GR::i32 Y, 
                                                             const XTextureSection& TexSection );
    virtual void                  RenderTextureSectionVMirrored( GR::i32 X, GR::i32 Y, 
                                                             const XTextureSection& TexSection );
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
    virtual void                  RenderTextureSectionMasked( GR::i32 X, GR::i32 Y, 
                                                              const XTextureSection& TexSection,
                                                              GR::i32 MaskX, GR::i32 MaskY, 
                                                              const XTextureSection& TexSectionMask, 
                                                              const GR::u32 Color = 0xffffff,
                                                              const GR::u32 MaskColorKey = 0xff000000 );

    virtual void                  RenderRect( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color1 );

    virtual void                  RenderText( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextCentered( X2dFont* Font, int CenterX, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void                  RenderTextRightAligned( X2dFont* Font, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff );
    virtual void                  RenderText( X2dFont* Font, int X, int Y, const GR::String& Text,
                                              float ScaleX, float ScaleY, GR::u32 Color = 0xffffffff );

  private:

    typedef std::map<GR::String,XTexture*>    tColorKeyedTextures;

    typedef std::map<GR::u32,GR::u32>         tRenderStates;

    typedef std::map<GR::u32,GR::u32>         tTextureStageStates;

    typedef std::list<XMultiTexture2d*>       tMultiTextures;

    tColorKeyedTextures           m_ColorKeyedTextures;

    tRenderStates                 m_RenderStates;

    tTextureStageStates           m_TextureStageStates[4];

    tMultiTextures                m_MultiTextures;

    XTexture*                     m_pCurTexture[8];

    WNDPROC                       m_pOldProc;

    HWND                          m_hWndFocus,
                                  m_hwndWindowedModeParent;

    HINSTANCE                     m_hInstance;

    LPDIRECT3D8                   m_pD3D;

    LPDIRECT3DDEVICE8             m_pd3dDevice;

    D3DPRESENT_PARAMETERS         m_d3dpp;

    D3DCAPS8                      m_DeviceCaps;
    D3DSURFACE_DESC               m_SurfaceDescBackBuffer;

    DWORD                         m_Adapter,
                                  m_CurrentAdapter,
                                  m_CurrentDevice,
                                  m_CurrentMode,
                                  m_CreateFlags,
                                  m_MinDepthBits,
                                  m_MinStencilBits,
                                  m_CreationWidth,
                                  m_CreationHeight,
                                  m_CreationDepth;

    bool                          m_Ready,
                                  m_WindowedModePossible,
                                  m_UseDepthBuffer,
                                  m_Windowed,
                                  m_Active,
                                  m_ForceWindowSize,
                                  m_VSyncEnabled,
                                  m_MultiThreadSafe;

    GR::i32                       m_ForcedWindowWidth,
                                  m_ForcedWindowHeight,
                                  m_RenderWidth,
                                  m_RenderHeight;

    GR::String                    m_SaveScreenShotFileName;

    std::vector<tD3DAdapterInfo>  m_Adapters;

    tD3DModeInfo*                 m_pCurrentModeInfo;

    XRendererDisplayMode          m_FullscreenDisplayMode;


    HRESULT                       BuildDeviceList();
    bool                          FindSuitableMode( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool bWindowed );
    HRESULT                       Initialize3DEnvironment();
    void                          Cleanup3DEnvironment();


    virtual HRESULT               ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );
    virtual BOOL                  FindDepthStencilFormat( int Adapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );

    virtual void                  SelectDefaultMode( int AdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice );

    virtual bool                  ResizeSurface();
    HRESULT                       Resize3DEnvironment();

    virtual bool                  InitDeviceObjects();
    virtual bool                  InvalidateDeviceObjects();
    virtual bool                  RestoreDeviceObjects();
    virtual bool                  DeleteDeviceObjects();

    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, GR::u32 ColorKeyReplacementColor = 0xff000000 );
    virtual bool                  CopyTextureToData( XTexture* pTexture, GR::Graphic::ImageData& ImageData );

    virtual void                  RestoreAllTextures();

    virtual void                  DestroyTexture( XTexture* pTexture );

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat );

    void                          DoSaveSnapShot( const GR::String& FileName );


    D3DFORMAT                     MapFormat( GR::Graphic::eImageFormat imgFormat );
    GR::Graphic::eImageFormat     MapFormat( D3DFORMAT d3dFormat );

    static DX82dRenderer*       g_pDX8Instance;

    static LRESULT CALLBACK       DX8Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    LRESULT CALLBACK              WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    void                          RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                        GR::f32 TU1, GR::f32 TV1,
                                        GR::f32 TU2, GR::f32 TV2,
                                        GR::f32 TU3, GR::f32 TV3,
                                        GR::f32 TU4, GR::f32 TV4,
                                        GR::u32 Color1, GR::u32 Color2 = 0, 
                                        GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    void                          RenderQuad2d( const GR::tVector& Pos1,
                                                const GR::tVector& Pos2,
                                                const GR::tVector& Pos3,
                                                const GR::tVector& Pos4,
                                                GR::f32 TU1, GR::f32 TV1,
                                                GR::f32 TU2, GR::f32 TV2,
                                                GR::f32 TU3, GR::f32 TV3,
                                                GR::f32 TU4, GR::f32 TV4,
                                                GR::u32 Color1, GR::u32 Color2 = 0, 
                                                GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    bool                          SetRenderState( GR::u32 State, GR::u32 Value );
    bool                          SetTextureStageState( GR::u32 State, GR::u32 Value, GR::u32 Stage = 0 );

    void                          Log( const GR::String& System, const char* Format, ... );

};
