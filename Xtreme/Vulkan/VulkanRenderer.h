#ifndef XRENDERER_VULKAN_H
#define XRENDERER_VULKAN_H

#include <OS/OS.h>

#include <Xtreme/XBasicRenderer.h>

#include <Xtreme/XAsset/XAssetLoader.h>

#include "VulkanLineCache.h"
#include "VulkanQuadCache.h"
#include "VulkanTriangleCache.h"

#include <vulkan/vulkan.h>



class VulkanVertexShader;
class VulkanPixelShader;
class VulkanVertexBuffer;
class VulkanTexture;
 
class VulkanRenderer : public XBasicRenderer
{
  private:

    struct QueueFamilyIndices 
    {
      uint32_t    graphicsFamily;
      bool        graphicsFamilyHasValue;
      uint32_t    presentFamily;
      bool        presentFamilyHasValue;

      QueueFamilyIndices() :
        graphicsFamily( 0 ),
        graphicsFamilyHasValue( false ),
        presentFamily( 0 ),
        presentFamilyHasValue( false )
      {
      }



      bool isComplete() 
      {
        return graphicsFamilyHasValue && presentFamilyHasValue;
      }
    };

    struct SwapChainSupportDetails
    {
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
    };

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

    
    VkInstance                              m_VulkanInstance;
    VkDevice                                m_VulkanDevice;

    VkSurfaceKHR                            m_Surface;
    VkQueue                                 m_GraphicsQueue;
    VkQueue                                 m_PresentQueue;

    std::vector<VkPhysicalDevice>           m_GPUs;
    std::vector<VkQueueFamilyProperties>    m_QueueProperties;

    VkPhysicalDevice                        m_PhysicalDevice;


    GR::u32                   m_Width;
    GR::u32                   m_Height;

    VulkanQuadCache                         m_QuadCache;
    VulkanQuadCache                         m_QuadCache3d;
    VulkanLineCache                         m_LineCache;
    VulkanLineCache                         m_LineCache3d;
    VulkanTriangleCache                     m_TriangleCache;
    VulkanTriangleCache                     m_TriangleCache3d;

    VulkanTexture*                          m_pCurrentlySetRenderTargetTexture;

    eShaderType                             m_CurrentShaderType;

    std::list<VulkanVertexShader*>          m_VertexShaders;
    std::list<VulkanPixelShader*>           m_PixelShaders;

    std::map<GR::String, VulkanVertexShader*>  m_BasicVertexShaders;
    std::map<GR::String, VulkanPixelShader*>   m_BasicPixelShaders;

    std::map<GR::u32, VulkanVertexShader*>  m_VSInputLayout;

    int                                   m_CachedFontType;

    GR::tRect                             m_WindowedPlacement;

    VkCommandPool                         m_CommandPool;

    VkSwapchainKHR                        m_SwapChain;
    std::vector<VkImage>                  m_SwapChainImages;
    VkFormat                              m_SwapChainImageFormat;
    VkExtent2D                            m_SwapChainExtent;
    std::vector<VkImageView>              m_SwapChainImageViews;
    const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    bool                                  m_Ready;


    ModelViewProjectionConstantBuffer     m_Matrices;
    LightsConstantBuffer                  m_LightInfo;
    MaterialConstantBuffer                m_Material;
    FogConstantBuffer                     m_Fog;

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

    VulkanVertexBuffer*                     m_pTempBuffer;

    GR::String                            m_CurrentVertexShader;
    GR::String                            m_CurrentPixelShader;

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

    bool                      Failed( const GR::String& Function, VkResult ResultCode );

    QueueFamilyIndices        FindQueueFamilies( VkPhysicalDevice device );



  public:

    VulkanRenderer( HINSTANCE hInstance = NULL );

    virtual ~VulkanRenderer();



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
    virtual XTexture*         LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
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

    virtual bool              SaveScreenShot( const char* szFileName );

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

    // these shaders are precompiled in memory and used to carry an input layout (because stupid vertexbuffer require a input layout which requires shaders)
    bool InitialiseBasicShaders();

    VulkanVertexShader* AddBasicVertexShader( const GR::String& Desc, const GR::String& HexVS, GR::u32 VertexFormat );
    bool AddBasicPixelShader( const GR::String& Desc, const GR::String& HexPS, GR::u32 VertexFormat );

    bool AddBasicVertexShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat );
    bool AddBasicPixelShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat );

    void SetVertexShader( const GR::String& Desc );
    void SetPixelShader( const GR::String& Desc );

    uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
    bool CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory );
    VkFormat FindSupportedFormat( const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features );

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands( VkCommandBuffer commandBuffer );
    void createCommandPool();

    bool pickPhysicalDevice();
    bool isDeviceSuitable( VkPhysicalDevice device );
    bool checkDeviceExtensionSupport( VkPhysicalDevice device );
    VkSurfaceFormatKHR chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats );
    VkPresentModeKHR chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes );
    VkExtent2D chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities );
    bool CreateSwapChain();
    SwapChainSupportDetails querySwapChainSupport( VkPhysicalDevice device );
    void createImageViews();
    void createLogicalDevice();

    // choose shaders according to set ShaderType and VertexFormat (urgh)
    void ChooseShaders( GR::u32 VertexFormat );

    bool RenderMesh2d( const Mesh::IMesh& );

    virtual void RenderBox( const GR::tVector& vPos, const GR::tVector& vSize, GR::u32 Color );

    virtual XFont* LoadFontSquare( const char* szFileName, GR::u32 Flags, GR::u32 TransparentColor );
    XFont* LoadFont( const char* szFileName, GR::u32 TransparentColor );

    int   MaxPrimitiveCount();

    virtual void RestoreAllTextures();

    bool CreateNativeTextureResources( VulkanTexture* pTexture );

    void SetLineMode();
    void SetQuadMode();
    void SetTriangleMode();

    void FlushAllCaches();
    void FlushLightChanges();

    bool ReleaseBuffers();
    bool RecreateBuffers();

    void SetCullFillMode( GR::u32 CullMode, GR::u32 FillMode );


    friend class VulkanQuadCache;
    friend class VulkanLineCache;
    friend class VulkanTriangleCache;

    #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    BOOL WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    #endif

};




#endif// XRENDERER_VULKAN_H
