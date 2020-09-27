#ifndef XRENDERER_UNIVERSAL_H
#define XRENDERER_UNIVERSAL_H

#include <OS/OS.h>

#include <Xtreme/XBasicRenderer.h>

#include <Xtreme/XAsset/XAssetLoader.h>

#include <d3d11_2.h>

#include "DX11LineCache.h"
#include "DX11QuadCache.h"
#include "DX11TriangleCache.h"

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
#include <agile.h>

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP )
#include <d3d11_3.h>
#else
#include <d3d11_2.h>
#endif

#endif



class DX11VertexShader;
class DX11PixelShader;
class DX11VertexBuffer;
class DX11Texture;
 
class DX11Renderer : public XBasicRenderer
{
  private:

    struct ColorValue
    {
      GR::f32       R;
      GR::f32       G;
      GR::f32       B;
      GR::f32       A;

      ColorValue( GR::u32 Color = 0 )
      {
        R = ( ( Color >> 16 ) & 0xff ) / 255.0f;
        G = ( ( Color >> 8 ) & 0xff ) / 255.0f;
        B = ( ( Color >> 0 ) & 0xff ) / 255.0f;
        A = ( Color >> 24 ) / 255.0f;
      }

      bool operator== ( const ColorValue& RHS )
      {
        return ( ( R == RHS.R )
        &&       ( G == RHS.G )
        &&       ( B == RHS.B )
        &&       ( A == RHS.A ) );
      }
    };

    struct ModelViewProjectionConstantBuffer
    {
      math::matrix4     Model;
      math::matrix4     View;
      math::matrix4     ViewIT;
      math::matrix4     Projection;
      math::matrix4     ScreenCoord2d;
      math::matrix4     TextureTransform;
    };

    struct DirectionLight
    {
      GR::tVector   Direction;
      GR::f32       PaddingL1;
      ColorValue    Ambient;
      ColorValue    Diffuse;
      ColorValue    Specular;
    };

    struct LightsConstantBuffer
    {
      ColorValue        Ambient;
      GR::tVector       EyePos;
      GR::f32           PaddingLC1;
      DirectionLight    Light[8];

      LightsConstantBuffer()
      {
      }
    };

    struct MaterialConstantBuffer
    {
      ColorValue    Emissive;     
      ColorValue    Ambient;      
      ColorValue    Diffuse;      
      ColorValue    Specular;     
      GR::f32       SpecularPower;
      GR::f32       Padding[3];
    };

    struct FogConstantBuffer
    {
      int         FogType;    //  = 0; // FOG_TYPE_NONE;
      GR::f32     FogStart = 10.f;
      GR::f32     FogEnd = 25.f;
      GR::f32     FogDensity = .02f;
      ColorValue  FogColor;
      GR::f32     PaddingF[4];
    };

    


    GR::u32                   m_Width;
    GR::u32                   m_Height;

    ID3D11Device*             m_pDevice;
    ID3D11DeviceContext*      m_pDeviceContext;

    D3D_FEATURE_LEVEL         m_FeatureLevel;

    DX11QuadCache                         m_QuadCache;
    DX11QuadCache                         m_QuadCache3d;
    DX11LineCache                         m_LineCache;
    DX11LineCache                         m_LineCache3d;
    DX11TriangleCache                     m_TriangleCache;
    DX11TriangleCache                     m_TriangleCache3d;

    ID3D11RasterizerState*                m_pRasterizerStateCullBack;
    ID3D11RasterizerState*                m_pRasterizerStateCullFront;
    ID3D11RasterizerState*                m_pRasterizerStateCullNone;
    ID3D11RasterizerState*                m_pRasterizerStateCullBackWireframe;
    ID3D11RasterizerState*                m_pRasterizerStateCullFrontWireframe;
    ID3D11RasterizerState*                m_pRasterizerStateCullNoneWireframe;

    DX11Texture*                          m_pCurrentlySetRenderTargetTexture;

    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullBack;
    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullFront;
    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullNone;
    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullBackWireframe;
    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullFrontWireframe;
    D3D11_RASTERIZER_DESC                 m_RasterizerDescCullNoneWireframe;

    eShaderType                           m_CurrentShaderType;

    std::list<DX11VertexShader*>          m_VertexShaders;
    std::list<DX11PixelShader*>           m_PixelShaders;

    std::map<GR::String, DX11VertexShader*>  m_BasicVertexShaders;
    std::map<GR::String, DX11PixelShader*>   m_BasicPixelShaders;

    std::map<GR::u32, DX11VertexShader*>  m_VSInputLayout;

    ID3D11Buffer*                         m_pMatrixBuffer;
    ID3D11Buffer*                         m_pLightBuffer;
    ID3D11Buffer*                         m_pMaterialBuffer;
    ID3D11Buffer*                         m_pFogBuffer;
    int                                   m_CachedFontType;

    GR::tRect                             m_WindowedPlacement;

    bool                                  m_Ready;


#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP )
    IDXGISwapChain1*                      m_pSwapChain;
#else
    IDXGISwapChain*                       m_pSwapChain;
#endif

  public:
    Platform::Agile<Windows::UI::Core::CoreWindow> m_window;
  private:
#else
    IDXGISwapChain*                       m_pSwapChain;
#endif

    ModelViewProjectionConstantBuffer     m_Matrices;
    LightsConstantBuffer                  m_LightInfo;
    MaterialConstantBuffer                m_Material;
    FogConstantBuffer                     m_Fog;

    ID3D11RenderTargetView*               m_pTargetBackBuffer;
    ID3D11SamplerState*                   m_pSamplerStateLinear;
    ID3D11SamplerState*                   m_pSamplerStatePoint;

    bool                                  m_Windowed;
    bool                                  m_VSyncEnabled;
    bool                                  m_LightsChanged;

    math::matrix4                         m_StoredTextureTransform;

    enum CacheMode
    {
      CM_LINE,
      CM_QUAD,
      CM_TRIANGLE
    };

    CacheMode                             m_CacheMode;

    GR::f32                               m_ClearColor[4];

    DX11VertexBuffer*                     m_pTempBuffer;

    GR::String                            m_CurrentVertexShader;
    GR::String                            m_CurrentPixelShader;

    ID3D11BlendState*                     m_pBlendStateNoBlend;
    ID3D11BlendState*                     m_pBlendStateAlphaBlend;
    ID3D11BlendState*                     m_pBlendStateAdditive;

    ID3D11Texture2D*                      m_depthStencilBuffer;
    ID3D11DepthStencilState*              m_depthStencilStateZBufferCheckAndWriteEnabled;
    ID3D11DepthStencilState*              m_depthStencilStateZBufferCheckAndWriteDisabled;
    ID3D11DepthStencilState*              m_depthStencilStateZBufferCheckEnabledNoWrite;
    ID3D11DepthStencilState*              m_depthStencilStateZBufferWriteEnabledNoCheck;
    ID3D11DepthStencilView*               m_depthStencilView;

    XTexture*                             m_SetTextures[8];

    XLight                                m_SetLights[8];
    bool                                  m_LightEnabled[8];
    bool                                  m_LightingEnabled;

    GR::u32                               m_NumActiveLights;

    std::map<GR::u32, XVertexBuffer*>     m_BasicVertexBuffers;

    XRendererDisplayMode                  m_FullscreenDisplayMode;

    bool                                  m_ForceToWindowedMode;
    bool                                  m_TogglingFullScreen;
    bool                                  m_TogglingToFullScreen;



    bool SetFogInfo();



  public:

    DX11Renderer( HINSTANCE hInstance = NULL );

    virtual ~DX11Renderer();



    virtual bool              Initialize( GR::u32 Width,
                                          GR::u32 Height,
                                          GR::u32 Depth,
                                          GR::u32 Flags,
                                          GR::IEnvironment& Environment );

    virtual bool              Release();

    virtual bool              OnResized();

    virtual bool              IsReady() const;

    virtual bool              BeginScene();
    virtual void              EndScene();
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool              ToggleFullscreen();
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool              IsFullscreen();

    virtual bool              SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage = 0 );
    virtual bool              SetViewport( const XViewport& Viewport );
    virtual bool              SetTrueViewport( const XViewport& Viewport );

    virtual GR::u32           Width();
    virtual GR::u32           Height();

    virtual void              Clear( bool bClearColor = true, bool bClearZ = true );

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual void              SetTexture( GR::u32 Stage, XTexture* pTexture );
    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, const GR::u32 MipMapLevel = 0 );

    // Lights
    virtual bool              SetLight( GR::u32 LightIndex, XLight& Light );

    virtual bool              SetMaterial( const XMaterial& Material );

    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*    CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD );

    virtual void              DestroyVertexBuffer( XVertexBuffer* );
    virtual bool              RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index = 0, GR::u32 Count = 0 );

    virtual void              RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2 = 0, float fZ = 0.0f );
    virtual void              RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 = 0 );

    virtual void              RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f );
    virtual void              RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f );

    virtual void              RenderQuad( const GR::tVector& ptPos1,
      const GR::tVector& ptPos2,
      const GR::tVector& ptPos3,
      const GR::tVector& ptPos4,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void              RenderQuad( const GR::tVector& ptPos1,
      const GR::tVector& ptPos2,
      const GR::tVector& ptPos3,
      const GR::tVector& ptPos4,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void              RenderTriangle( const GR::tVector& pt1,
      const GR::tVector& pt2,
      const GR::tVector& pt3,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0 );

    virtual void              RenderTriangle2d( const GR::tPoint& pt1,
      const GR::tPoint& pt2,
      const GR::tPoint& pt3,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::u32 Color1, GR::u32 Color2 = 0,
      GR::u32 Color3 = 0, float fZ = 0.0f );

    virtual bool              SaveScreenShot( const char* FileName );

    virtual bool VSyncEnabled();
    virtual void EnableVSync( bool Enable );
    virtual GR::Graphic::eImageFormat ImageFormat();
    virtual bool ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel );
    virtual bool RenderMesh( const Mesh::IMesh& );
    virtual void RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
      GR::f32 fX2, GR::f32 fY2,
      GR::f32 fX3, GR::f32 fY3,
      GR::f32 fX4, GR::f32 fY4,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 Color1, GR::u32 Color2,
      GR::u32 Color3, GR::u32 Color4, float fZ );
    virtual void SetRenderTarget( XTexture * );


    virtual void SetTransform( eTransformType tType, const math::matrix4& matTrix );

    virtual void SetShader( eShaderType sType );


    ID3D11Device* Device();
    ID3D11DeviceContext* DeviceContext();

    // these shaders are precompiled in memory and used to carry an input layout (because stupid vertexbuffer require a input layout which requires shaders)
    bool InitialiseBasicShaders();

    DX11VertexShader* AddBasicVertexShader( const GR::String& Desc, const GR::String& HexVS, GR::u32 VertexFormat );
    bool AddBasicPixelShader( const GR::String& Desc, const GR::String& HexPS, GR::u32 VertexFormat );

    bool AddBasicVertexShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat );
    bool AddBasicPixelShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat );

    void SetVertexShader( const GR::String& Desc );
    void SetPixelShader( const GR::String& Desc );

    bool CreateSwapChain();

    ID3D11InputLayout* InputLayout( GR::u32 VertexFormat );


    DXGI_FORMAT MapFormat( GR::Graphic::eImageFormat imgFormat );
    GR::Graphic::eImageFormat MapFormat( DXGI_FORMAT d3dFormat );

    // choose shaders according to set ShaderType and VertexFormat (urgh)
    void ChooseShaders( GR::u32 VertexFormat );

    bool RenderMesh2d( const Mesh::IMesh& );

    virtual void RenderBox( const GR::tVector& vPos, const GR::tVector& vSize, GR::u32 Color );

    virtual XFont* LoadFontSquare( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor );
    XFont* LoadFont( const char* FileName, GR::u32 TransparentColor );

    int   MaxPrimitiveCount();

    virtual void RestoreAllTextures();

    bool CreateNativeTextureResources( XTextureBase* pTexture );

    void SetLineMode();
    void SetQuadMode();
    void SetTriangleMode();

    void FlushAllCaches();
    void FlushLightChanges();

    bool ReleaseBuffers();
    bool RecreateBuffers();

    void SetCullFillMode( GR::u32 CullMode, GR::u32 FillMode );


    friend class DX11QuadCache;
    friend class DX11LineCache;
    friend class DX11TriangleCache;

    #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    BOOL WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    #endif

};




#endif// XRENDERER_UNIVERSAL_H
