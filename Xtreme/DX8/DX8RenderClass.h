#pragma once


#include <D3D8.h>

#include <vector>

#include <Xtreme/XBasicRenderer.h>
#include <Xtreme/XAsset/XAssetLoader.h>



struct tD3DModeInfo
{
  DWORD       Width;
  DWORD       Height;
  DWORD       Behaviour;          // Hardware / Software / Mixed vertex processing
  D3DFORMAT   PixelFormat;        // Pixel format in this mode
  D3DFORMAT   DepthStencilFormat; // Which depth/stencil format to use with this mode
};



struct tD3DDeviceInfo
{
  // Device data
  D3DDEVTYPE                  deviceType;      // Reference, HAL, etc.

  D3DCAPS8                    d3dCaps;         // Capabilities of this device

  GR::String                 strName;

  bool                        bCanDoWindowed;  // Whether this device can work in windowed mode

  // Modes for this device
  std::vector<tD3DModeInfo>   m_vectMode;

  // Current state

  DWORD                       CurrentMode;

  bool                        bWindowed;

  D3DMULTISAMPLE_TYPE         MultiSampleType;
};



struct tD3DAdapterInfo
{
  // Adapter data
  D3DADAPTER_IDENTIFIER8      d3dAdapterIdentifier;

  D3DDISPLAYMODE              d3ddmDesktop;      // Desktop display mode for this adapter

  // Devices for this adapter
  std::vector<tD3DDeviceInfo> vectDevices;

  // Current state
  DWORD                       CurrentDevice;
};



class CDX8RenderClass : public XBasicRenderer
{

  public:


    CDX8RenderClass( HINSTANCE hInstance = NULL );
    virtual ~CDX8RenderClass();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::Graphic::eImageFormat   ImageFormat();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  VSyncEnabled();
    virtual void                  EnableVSync( bool Enable = true );

    virtual bool                  SaveScreenShot( const char* szFileName );

    virtual bool                  IsReady() const;
    LPDIRECT3DDEVICE8             Device();

    virtual void                  Clear( bool bClearColor = true, bool bClearZ = true );

    virtual bool                  SetState( eRenderState rState, GR::u32 rValue, GR::u32 State = 0 );
    virtual void                  SetTransform( eTransformType tType, const math::matrix4& matTrix );

    virtual bool                  SetViewport( const XViewport& Viewport );
    virtual bool                  SetTrueViewport( const XViewport& Viewport );

    virtual XTexture*             LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual XTexture*             CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*             CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual void                  SetTexture( GR::u32 Stage, XTexture* pTexture );

    virtual XVertexBuffer*        CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*        CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*        CreateVertexBuffer( const Mesh::IMesh& Mesh, GR::u32 VertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD );
    virtual void                  DestroyVertexBuffer( XVertexBuffer* );
    virtual bool                  RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index = 0, GR::u32 Count = 0 );
    virtual bool                  RenderMesh( const Mesh::IMesh& Mesh );

    virtual bool                  SetLight( GR::u32 LightIndex, XLight& Light );

    virtual bool                  SetMaterial( const XMaterial& Material );

    virtual void                  RenderLine2d( const GR::tPoint& pt1,
                                                const GR::tPoint& pt2,
                                                GR::u32 Color1,
                                                GR::u32 Color2, float fZ = 0.0f );
    virtual void                  RenderLine( const GR::tVector& vect1,
                                              const GR::tVector& vect2,
                                              GR::u32 Color1,
                                              GR::u32 Color2 );

    virtual void                  RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                                GR::f32 fTU1, GR::f32 fTV1,
                                                GR::f32 fTU2, GR::f32 fTV2,
                                                GR::f32 fTU3, GR::f32 fTV3,
                                                GR::f32 fTU4, GR::f32 fTV4,
                                                GR::u32 Color1, GR::u32 Color2,
                                                GR::u32 Color3, GR::u32 Color4, float fZ = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                                GR::f32 fTU1, GR::f32 fTV1,
                                                GR::f32 fTU2, GR::f32 fTV2,
                                                GR::f32 fTU3, GR::f32 fTV3,
                                                GR::f32 fTU4, GR::f32 fTV4,
                                                GR::u32 Color1, GR::u32 Color2 = 0,
                                                GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
                                                  GR::f32 fX2, GR::f32 fY2,
                                                  GR::f32 fX3, GR::f32 fY3,
                                                  GR::f32 fX4, GR::f32 fY4,
                                                  GR::f32 fTU1, GR::f32 fTV1,
                                                  GR::f32 fTU2, GR::f32 fTV2,
                                                  GR::f32 fTU3, GR::f32 fTV3,
                                                  GR::f32 fTU4, GR::f32 fTV4,
                                                  GR::u32 Color1, GR::u32 Color2 = 0,
                                                  GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f );
    virtual void                  RenderQuad( const GR::tVector& ptPos1,
                                              const GR::tVector& ptPos2,
                                              const GR::tVector& ptPos3,
                                              const GR::tVector& ptPos4,
                                              GR::f32 fTU1, GR::f32 fTV1,
                                              GR::f32 fTU2, GR::f32 fTV2,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0, GR::u32 Color4 = 0 );
    virtual void                  RenderQuad( const GR::tVector& ptPos1,
                                              const GR::tVector& ptPos2,
                                              const GR::tVector& ptPos3,
                                              const GR::tVector& ptPos4,
                                              GR::f32 fTU1, GR::f32 fTV1,
                                              GR::f32 fTU2, GR::f32 fTV2,
                                              GR::f32 fTU3, GR::f32 fTV3,
                                              GR::f32 fTU4, GR::f32 fTV4,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void                  RenderTriangle( const GR::tVector& pt1,
                                                  const GR::tVector& pt2,
                                                  const GR::tVector& pt3,
                                                  GR::f32 fTU1, GR::f32 fTV1,
                                                  GR::f32 fTU2, GR::f32 fTV2,
                                                  GR::f32 fTU3, GR::f32 fTV3,
                                                  GR::u32 Color1, GR::u32 Color2 = 0,
                                                  GR::u32 Color3 = 0 );

    virtual void                  RenderTriangle2d( const GR::tPoint& pt1,
                                                    const GR::tPoint& pt2,
                                                    const GR::tPoint& pt3,
                                                    GR::f32 fTU1, GR::f32 fTV1,
                                                    GR::f32 fTU2, GR::f32 fTV2,
                                                    GR::f32 fTU3, GR::f32 fTV3,
                                                    GR::u32 Color1, GR::u32 Color2 = 0,
                                                    GR::u32 Color3 = 0, float fZ = 0.0f );


  private:

    WNDPROC                       m_pOldProc;

    HWND                          m_hWndFocus;

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

    bool                          m_bReady,
                                  m_bWindowedModePossible,
                                  m_bUseDepthBuffer,
                                  m_bWindowed,
                                  m_bActive,
                                  m_bForceWindowSize,
                                  m_VSyncEnabled,
                                  m_bMultiThreadSafe;

    GR::i32                       m_iForcedWindowWidth,
                                  m_iForcedWindowHeight,
                                  m_RenderWidth,
                                  m_RenderHeight;

    GR::tPoint                    m_FullScreenOffset;
    GR::tPoint                    m_FullScreenSize;
    GR::tPoint                    m_NativeMonitorSize;

    GR::String                   m_strSaveScreenShotFileName;

    std::vector<tD3DAdapterInfo>  m_vectAdapters;

    tD3DModeInfo*                 m_pCurrentModeInfo;

    XRendererDisplayMode          m_FullscreenDisplayMode;

    std::vector<XRendererDisplayMode>   m_DisplayModeList;

    IDirect3DSurface8*            m_pBackBufferTargetSurface;
    IDirect3DSurface8*            m_pBackBufferDepthStencilSurface;
    IDirect3DSurface8*            m_pCurrentlySetTargetSurface;

    XTexture*                     m_pCurrentlySetTargetTexture;



    bool                          FindSuitableMode( GR::u32 Width, GR::u32 Height, GR::u32 Depth, bool bWindowed );
    HRESULT                       BuildDeviceList();
    HRESULT                       Initialize3DEnvironment();
    void                          Cleanup3DEnvironment();


    virtual HRESULT               ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );
    virtual BOOL                  FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType, D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );

    virtual void                  SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice );

    virtual bool                  ResizeSurface();
    HRESULT                       Resize3DEnvironment();

    virtual bool                  InitDeviceObjects();
    virtual bool                  InvalidateDeviceObjects();
    virtual bool                  RestoreDeviceObjects();
    virtual bool                  DeleteDeviceObjects();

    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, const GR::u32 MipMapLevel = 0 );
    virtual bool                  ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel = 0 );
    virtual void                  RestoreAllTextures();

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false );

    void                          DoSaveSnapShot( const char* szFile );

    GR::Graphic::ImageData*       LoadAndConvert( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor );

    // Set NULL to return to previous back buffer
    virtual void                  SetRenderTarget( XTexture* pTexture );

    D3DFORMAT                     MapFormat( GR::Graphic::eImageFormat imgFormat );
    GR::Graphic::eImageFormat     MapFormat( D3DFORMAT d3dFormat );

    static CDX8RenderClass*       g_pDX8Instance;

};
