#include "DX11Font.h"
#include "DX11Renderer.h"
#include "DX11PixelShader.h"
#include "DX11Texture.h"
#include "DX11VertexShader.h"
#include "DX11VertexBuffer.h"

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Xtreme/Environment/XWindow.h>
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
#include <Xtreme/Environment/XWindowUniversalApp.h>
#endif

#include <IO/FileUtil.h>

#include <String/StringUtil.h>

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP )
#include <dxgi1_4.h>
#elif  ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
#include <dxgi1_3.h>
#else
#include <WinSys/SubclassManager.h>
#endif



#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )



DX11Renderer::DX11Renderer( HINSTANCE hInstance ) :
  m_Width( 0 ),
  m_Height( 0 ),
  m_VSyncEnabled( false ), 
  m_pDevice( NULL ),
  m_pDeviceContext( NULL ),
  m_pMatrixBuffer( NULL ),
  m_pLightBuffer( NULL ),
  m_pMaterialBuffer( NULL ),
  m_pFogBuffer( NULL ),
  m_pSwapChain( NULL ),
  m_pTargetBackBuffer( NULL ),
  m_pTempBuffer( NULL ),
  m_pSamplerStateLinear( NULL ),
  m_pSamplerStatePoint( NULL ),
  m_pRasterizerStateCullBack( NULL ),
  m_pRasterizerStateCullFront( NULL ),
  m_pRasterizerStateCullNone( NULL ),
  m_pRasterizerStateCullBackWireframe( NULL ),
  m_pRasterizerStateCullFrontWireframe( NULL ),
  m_pRasterizerStateCullNoneWireframe( NULL ),
  m_pBlendStateAdditive( NULL ),
  m_pBlendStateNoBlend( NULL ),
  m_pBlendStateAlphaBlend( NULL ),
  m_CurrentShaderType( ST_INVALID ),
  m_NumActiveLights( 0 ),
  m_LightingEnabled( false ),
  m_depthStencilBuffer( NULL ),
  m_depthStencilStateZBufferCheckAndWriteEnabled( NULL ),
  m_depthStencilStateZBufferCheckAndWriteDisabled( NULL ),
  m_depthStencilStateZBufferCheckEnabledNoWrite( NULL ),
  m_depthStencilStateZBufferWriteEnabledNoCheck( NULL ),
  m_depthStencilView( NULL ),
  m_pCurrentlySetRenderTargetTexture( NULL ),
  m_QuadCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_QuadCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ ),
  m_LineCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_LineCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ ),
  m_TriangleCache( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZRHW ),
  m_TriangleCache3d( this, XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_XYZ ),
  m_Windowed( true ),
  m_CacheMode( CM_QUAD ),
  m_LightsChanged( false ),
  m_ForceToWindowedMode( false ),
  m_TogglingFullScreen( false ),
  m_Ready( false )
{
  m_ClearColor[0] = 0.0f;
  m_ClearColor[1] = 0.0f;
  m_ClearColor[2] = 0.0f;
  m_ClearColor[3] = 1.0f;

  for ( int i = 0; i < 8; ++i )
  {
    m_SetTextures[i]      = NULL;
    m_SetLights[i].m_Type = XLight::LT_INVALID;
    m_LightEnabled[i]     = false;
  }

  m_Material.Ambient  = ColorValue( 0xffffffff );
  m_Material.Emissive = ColorValue();
  m_Material.Diffuse  = ColorValue( 0xffffffff );
  m_Material.Specular = ColorValue( 0xffffffff );
  m_Material.SpecularPower = 1.0f;
}



DX11Renderer::~DX11Renderer()
{ 
  Release();
}



bool DX11Renderer::AddBasicVertexShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat )
{ 
  GR::String     basePath = "D:/projekte/Xtreme/DX11Renderer/shaders/";
  //GR::String     basePath = "D:/privat/projekte/Xtreme/DX11Renderer/shaders/";

  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( ( basePath + Filename ).c_str() );

  if ( shaderData.Empty() )
  {
    return false;
  }

  auto pVShader = AddBasicVertexShader( Desc, shaderData.ToHexString(), VertexFormat );
  if ( pVShader == NULL )
  {
    return false;
  }
  pVShader->m_LoadedFromFile = Filename;
  return true;
}



bool DX11Renderer::AddBasicPixelShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat )
{
  GR::String     basePath = "D:/projekte/Xtreme/DX11Renderer/shaders/";
  //GR::String     basePath = "D:/privat/projekte/Xtreme/DX11Renderer/shaders/";

  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( ( basePath + Filename ).c_str() );

  if ( shaderData.Empty() )
  {
    return false;
  }

  return AddBasicPixelShader( Desc, shaderData.ToHexString(), VertexFormat );
}



bool DX11Renderer::InitialiseBasicShaders()
{
  bool            hadError = false;

#include "ShaderBinary.inl"

  return !hadError;
  return true;
}



DX11VertexShader* DX11Renderer::AddBasicVertexShader( const GR::String& Desc, const GR::String& HexVS, GR::u32 VertexFormat )
{
  ByteBuffer      vsFlat( HexVS );

  auto  shader = new DX11VertexShader( this, VertexFormat );
  if ( !shader->CreateFromBuffers( vsFlat ) )
  {
    delete shader;
    return NULL;
  }
  m_BasicVertexShaders[Desc] = shader;
  m_VertexShaders.push_back( shader );

  m_VSInputLayout[VertexFormat] = shader;
  return shader;
}



bool DX11Renderer::AddBasicPixelShader( const GR::String& Desc, const GR::String& HexPS, GR::u32 VertexFormat )
{
  ByteBuffer      psFlat( HexPS );

  auto  shader = new DX11PixelShader( this );
  if ( !shader->CreateFromBuffers( psFlat ) )
  {
    delete shader;
    return false;
  }
  m_BasicPixelShaders[Desc] = shader;
  m_PixelShaders.push_back( shader );
  return true;
}



bool DX11Renderer::Initialize( GR::u32 Width,
                               GR::u32 Height,
                               GR::u32 Depth,
                               GR::u32 Flags,
                               GR::IEnvironment& Environment )
{
  if ( m_Ready )
  {
    return true;
  }
  m_pEnvironment = &Environment;

  m_Windowed      = !( Flags & XRenderer::IN_FULLSCREEN );
  m_VSyncEnabled  = !!( Flags & XRenderer::IN_VSYNC );

  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
#if defined(_DEBUG)
  //if ( DX::SdkLayersAvailable() )
  {
    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
  }
#endif
#endif

#if OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE
  //creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1
  };

  m_FeatureLevel = D3D_FEATURE_LEVEL_9_1;

  ID3D11DeviceContext*    pDC;

  ID3D11Device*   pDevice;

  HRESULT hr = D3D11CreateDevice( nullptr,					// Specify nullptr to use the default adapter.
                                  D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the harare graphics driver.
                                  0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
                                  creationFlags,				// Set debug and Direct2D compatibility flags.
                                  featureLevels,				// List of feature levels this app can support.
                                  ARRAYSIZE( featureLevels ),	// Size of the list above.
                                  D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
                                  &pDevice,					// Returns the Direct3D device created.
                                  &m_FeatureLevel,			// Returns feature level of device created.
                                  &pDC					// Returns the device immediate context.
                                  );
  if ( FAILED( hr ) )
  {
    // fallback to WARP device
    if ( FAILED( D3D11CreateDevice( nullptr,
                                    D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a harare device.
                                    0,
                                    creationFlags,
                                    featureLevels,
                                    ARRAYSIZE( featureLevels ),
                                    D3D11_SDK_VERSION,
                                    &pDevice,
                                    &m_FeatureLevel,
                                    &pDC ) ) )
    {
      return false;
    }
  }

  Xtreme::IAppWindow* pWindowService = ( Xtreme::IAppWindow* )Environment.Service( "Window" );
  HWND      hWnd = NULL;
  if ( pWindowService != NULL )
  {
    hWnd = (HWND)pWindowService->Handle();
  }
  else
  {
    dh::Log( "No Window service found in environment" );
  }


  m_hwndViewport = hWnd;
  m_DisplayOffset.Clear();

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  //RECT      windowRect;
  //GetWindowRect( m_hwndViewport, &windowRect );
  //m_WindowedPlacement.set( windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );
  m_WindowedPlacement.length = sizeof( WINDOWPLACEMENT );
  GetWindowPlacement( m_hwndViewport, &m_WindowedPlacement );
#endif

  //pDC->QueryInterface( __uuidof( ID3D11DeviceContext3 ), (void**)&m_pDeviceContext );
  hr = pDC->QueryInterface( __uuidof( ID3D11DeviceContext ), (void**)&m_pDeviceContext );
  pDC->Release();
  if ( FAILED( hr ) )
  {
	  return false;
  }

  //pDevice->QueryInterface( __uuidof( ID3D11Device3 ), (void**)&m_pDevice );
  hr = pDevice->QueryInterface( __uuidof( ID3D11Device ), (void**)&m_pDevice );
  pDevice->Release();
  if ( FAILED( hr ) )
  {
	  return false;
  }

  if ( !InitialiseBasicShaders() )
  {
    Release();
    return false;
  }

  m_Width = Width;
  m_Height = Height;

  m_Canvas.Set( 0, 0, m_Width, m_Height );

  SetVertexShader( "position_color" );
  SetPixelShader( "position_color" );

  m_VirtualSize.Set( m_Width, m_Height );

  CD3D11_BUFFER_DESC  constantBufferDesc( sizeof( ModelViewProjectionConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
  if ( FAILED( m_pDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_pMatrixBuffer ) ) )
  {
    Release();
    return false;
  }

  constantBufferDesc = CD3D11_BUFFER_DESC( sizeof( LightsConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
  if ( FAILED( m_pDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_pLightBuffer ) ) )
  {
    Release();
    return false;
  }

  constantBufferDesc = CD3D11_BUFFER_DESC( sizeof( MaterialConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
  if ( FAILED( m_pDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_pMaterialBuffer ) ) )
  {
    Release();
    return false;
  }
  
  constantBufferDesc = CD3D11_BUFFER_DESC( sizeof( FogConstantBuffer ), D3D11_BIND_CONSTANT_BUFFER );
  if ( FAILED( m_pDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_pFogBuffer ) ) )
  {
    Release();
    return false;
  }


  // Create a texture sampler state description.
  D3D11_SAMPLER_DESC      samplerDesc;

  samplerDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias      = 0.0f;
  samplerDesc.MaxAnisotropy   = 1;
  samplerDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0]  = 0;
  samplerDesc.BorderColor[1]  = 0;
  samplerDesc.BorderColor[2]  = 0;
  samplerDesc.BorderColor[3]  = 0;
  samplerDesc.MinLOD          = 0;
  samplerDesc.MaxLOD          = D3D11_FLOAT32_MAX;

  // Create the texture sampler state.
  if ( FAILED( m_pDevice->CreateSamplerState( &samplerDesc, &m_pSamplerStateLinear ) ) )
  {
    Release();
    return false;
  }
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  if ( FAILED( m_pDevice->CreateSamplerState( &samplerDesc, &m_pSamplerStatePoint ) ) )
  {
    Release();
    return false;
  }

  D3D11_BLEND_DESC blendState;
  ZeroMemory( &blendState, sizeof( blendState ) );

  blendState.RenderTarget[0].BlendEnable = FALSE;
  blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  if ( FAILED( m_pDevice->CreateBlendState( &blendState, &m_pBlendStateNoBlend ) ) )
  {
    Release();
    return false;
  }

  ZeroMemory( &blendState, sizeof( blendState ) );

  blendState.RenderTarget[0].BlendEnable            = TRUE;
  blendState.RenderTarget[0].BlendOp                = D3D11_BLEND_OP_ADD;
  blendState.RenderTarget[0].SrcBlend               = D3D11_BLEND_SRC_ALPHA;
  blendState.RenderTarget[0].DestBlend              = D3D11_BLEND_INV_SRC_ALPHA;
  blendState.RenderTarget[0].BlendOpAlpha           = D3D11_BLEND_OP_ADD;
  blendState.RenderTarget[0].SrcBlendAlpha          = D3D11_BLEND_SRC_ALPHA;
  blendState.RenderTarget[0].DestBlendAlpha         = D3D11_BLEND_INV_SRC_ALPHA;
  blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  if ( FAILED( m_pDevice->CreateBlendState( &blendState, &m_pBlendStateAlphaBlend ) ) )
  {
    Release();
    return false;
  }

  ZeroMemory( &blendState, sizeof( blendState ) );

  blendState.RenderTarget[0].BlendEnable = TRUE;
  blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  //blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_OP_ADD;
  blendState.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
  blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  //blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
  blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  if ( FAILED( m_pDevice->CreateBlendState( &blendState, &m_pBlendStateAdditive ) ) )
  {
    Release();
    return false;
  }

  // Initialize the description of the depth buffer.
  D3D11_TEXTURE2D_DESC          depthBufferDesc;
  D3D11_DEPTH_STENCIL_DESC      depthStencilDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

  ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

  // Set up the description of the depth buffer.
  depthBufferDesc.Width     = m_Width;
  depthBufferDesc.Height    = m_Height;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  // Create the texture for the depth buffer using the filled out description.
  HRESULT result = m_pDevice->CreateTexture2D( &depthBufferDesc, NULL, &m_depthStencilBuffer );
  if ( FAILED( result ) )
  {
    return false;
  }

  // Initialize the description of the stencil state.
  ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

  // Set up the description of the stencil state.
  depthStencilDesc.DepthEnable      = true;
  depthStencilDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc        = D3D11_COMPARISON_LESS_EQUAL;

  depthStencilDesc.StencilEnable    = false;
  depthStencilDesc.StencilReadMask  = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  // Stencil operations if pixel is front-facing.
  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // Stencil operations if pixel is back-facing.
  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // Create the depth stencil state.
  result = m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStateZBufferCheckAndWriteEnabled );
  if ( FAILED( result ) )
  {
    return false;
  }

  depthStencilDesc.DepthEnable = false;
  result = m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStateZBufferWriteEnabledNoCheck );
  if ( FAILED( result ) )
  {
    return false;
  }

  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  result = m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStateZBufferCheckAndWriteDisabled );
  if ( FAILED( result ) )
  {
    return false;
  }

  depthStencilDesc.DepthEnable = true;
  result = m_pDevice->CreateDepthStencilState( &depthStencilDesc, &m_depthStencilStateZBufferCheckEnabledNoWrite );
  if ( FAILED( result ) )
  {
    return false;
  }

  // Set the depth stencil state.
  m_pDeviceContext->OMSetDepthStencilState( m_depthStencilStateZBufferCheckAndWriteDisabled, 1 );

  // Initialize the depth stencil view.
  ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

  // Set up the depth stencil view description.
  depthStencilViewDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view.
  result = m_pDevice->CreateDepthStencilView( m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView );
  if ( FAILED( result ) )
  {
    return false;
  }

  // Bind the render target view and depth stencil buffer to the output render pipeline.
  m_pDeviceContext->OMSetRenderTargets( 1, &m_pTargetBackBuffer, m_depthStencilView );

  m_RasterizerDescCullBack.FillMode               = D3D11_FILL_SOLID;
  m_RasterizerDescCullBack.CullMode               = D3D11_CULL_BACK;
  m_RasterizerDescCullBack.DepthBias              = D3D11_DEFAULT_DEPTH_BIAS;
  m_RasterizerDescCullBack.DepthBiasClamp         = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
  m_RasterizerDescCullBack.FrontCounterClockwise  = FALSE;
  m_RasterizerDescCullBack.SlopeScaledDepthBias   = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  m_RasterizerDescCullBack.DepthClipEnable        = TRUE;
  m_RasterizerDescCullBack.ScissorEnable          = FALSE;
  m_RasterizerDescCullBack.MultisampleEnable      = FALSE;
  m_RasterizerDescCullBack.AntialiasedLineEnable  = FALSE;

  m_RasterizerDescCullBackWireframe = m_RasterizerDescCullBack;
  m_RasterizerDescCullBackWireframe.FillMode = D3D11_FILL_WIREFRAME;

  m_RasterizerDescCullFront = m_RasterizerDescCullBack;
  m_RasterizerDescCullFront.CullMode = D3D11_CULL_FRONT;
  m_RasterizerDescCullFrontWireframe = m_RasterizerDescCullFront;
  m_RasterizerDescCullFrontWireframe.FillMode = D3D11_FILL_WIREFRAME;

  m_RasterizerDescCullNone = m_RasterizerDescCullBack;
  m_RasterizerDescCullNone.CullMode = D3D11_CULL_NONE;
  m_RasterizerDescCullNoneWireframe = m_RasterizerDescCullNone;
  m_RasterizerDescCullNoneWireframe.FillMode = D3D11_FILL_WIREFRAME;

  if ( ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullBack, &m_pRasterizerStateCullBack ) ) )
  ||   ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullFront, &m_pRasterizerStateCullFront ) ) )
  ||   ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullNone, &m_pRasterizerStateCullNone ) ) )
  ||   ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullBackWireframe, &m_pRasterizerStateCullBackWireframe ) ) )
  ||   ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullFrontWireframe, &m_pRasterizerStateCullFrontWireframe ) ) )
  ||   ( FAILED( m_pDevice->CreateRasterizerState( &m_RasterizerDescCullNoneWireframe, &m_pRasterizerStateCullNoneWireframe ) ) ) )
  {
    Release();
    return false;
  }

  m_pDeviceContext->VSSetSamplers( 0, 1, &m_pSamplerStateLinear );
  m_pDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerStateLinear );

  XViewport   vp;

  vp.X = 0;
  vp.Y = 0;
  vp.Width = m_Width;
  vp.Height = m_Height;
  vp.MinZ = 0.0f;
  vp.MaxZ = 1.0f;

  SetViewport( vp );

  m_Matrices.ScreenCoord2d.OrthoOffCenterLH( 0.0f, 
                                             ( GR::f32 )m_Width, 
                                             ( GR::f32 )m_Height, 
                                             0.0f, 
                                             0.0f, 
                                             1.0f );
  m_Matrices.ScreenCoord2d.Transpose();
  m_Matrices.Model.Identity();
  m_Matrices.TextureTransform.Identity();
  m_StoredTextureTransform.Identity();

  //m_LightInfo.GlobalAmbient = ColorValue( 0x00000000 );
  m_LightInfo.Ambient = ColorValue( 0x00000000 );

  SetTransform( XRenderer::TT_WORLD, m_Matrices.Model );

  if ( !CreateSwapChain() )
  {
    Release();
    return false;
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );
  if ( pManager )
  {
    pManager->AddHandler( "DX11Renderer", fastdelegate::MakeDelegate( this, &DX11Renderer::WindowProc ) );
  }
#endif

  ID3D11Texture2D*  pBackBuffer;

  if ( FAILED( m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer ) ) )
  {
    Release();
    return false;
  }

  // use the back buffer address to create the render target
  if ( FAILED( m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pTargetBackBuffer ) ) )
  {
    pBackBuffer->Release();
    Release();
    return false;
  }
  pBackBuffer->Release();

  for ( int i = 0; i < 8; ++i )
  {
    m_SetTextures[i] = NULL;
  }

  RecreateBuffers();

  // init material
  m_pDeviceContext->UpdateSubresource( m_pMaterialBuffer, 0, NULL, &m_Material, 0, 0 );
  m_pDeviceContext->PSSetConstantBuffers( 2, 1, &m_pMaterialBuffer );


  m_Fog.FogType     = 0; // FOG_TYPE_NONE;
  m_Fog.FogColor    = ColorValue( 0x00000000 );
  m_Fog.FogStart    = 10.0f;
  m_Fog.FogEnd      = 25.0f;
  m_Fog.FogDensity  = 0.02f;
  m_pDeviceContext->UpdateSubresource( m_pFogBuffer, 0, NULL, &m_Fog, 0, 0 );
  m_pDeviceContext->VSSetConstantBuffers( 3, 1, &m_pFogBuffer );
  m_pDeviceContext->PSSetConstantBuffers( 3, 1, &m_pFogBuffer );

  // assets
  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      LoadImageAssets();
      LoadImageSectionAssets();
      LoadFontAssets();
      LoadMeshAssets();

      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }

  m_RenderStates[std::make_pair( RS_CULLMODE, 0 )] = RSV_CULL_CCW;
  m_RenderStates[std::make_pair( RS_FILL_MODE, 0 )] = RSV_FILL_SOLID;

  // apply common settings
  SetState( XRenderer::RS_NORMALIZE_NORMALS, XRenderer::RSV_ENABLE ); // seems to be default on DX9/DX11
  SetState( XRenderer::RS_MINFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MAGFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_MIPFILTER, XRenderer::RSV_FILTER_LINEAR );
  SetState( XRenderer::RS_FOG_ENABLE, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_FOG_TABLEMODE, XRenderer::RSV_FOG_LINEAR );
  SetState( XRenderer::RS_LIGHTING, XRenderer::RSV_DISABLE );
  SetState( XRenderer::RS_AMBIENT, 0 );

  m_Ready = true;
  return true;
}



#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
BOOL DX11Renderer::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch ( uMsg )
  {
    case WM_SIZE:
      {
        int     width = LOWORD( lParam );
        int     height = HIWORD( lParam );

        //dh::Log( "WM_SIZE Size changed to %dx%d", width, height );
      }
      break;
    case WM_ACTIVATEAPP:
      if ( !wParam )
      {
        // we're being deactivated
        //dh::Log( "I'm being app deactivated" );
        if ( ( !m_TogglingFullScreen )
        &&   ( IsFullscreen() ) )
        {
          // switch off fullscreen
          //dh::Log( "-turning off fullscreen due to WM_ACTIVATEAPP" );
          ToggleFullscreen();
        }
      }
      else
      {
        //dh::Log( "I'm being app activated" );
      }
      break;
  }

  if ( m_pEnvironment != NULL )
  {
    ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );
    if ( pManager )
    {
      return pManager->CallNext( hWnd, uMsg, wParam, lParam );
    }
  }
  return (BOOL)CallWindowProc( NULL, hWnd, uMsg, wParam, lParam );
}
#endif



void DX11Renderer::SetVertexShader( const GR::String& Desc )
{
  if ( m_CurrentVertexShader == Desc )
  {
    return;
  }

  std::map<GR::String, DX11VertexShader*>::iterator    itVS( m_BasicVertexShaders.find( Desc ) );
  if ( itVS == m_BasicVertexShaders.end() )
  {
    dh::Log( "tried to set Unknown vertex shader %s", Desc.c_str() );
    return;
  }
  m_pDeviceContext->VSSetShader( itVS->second->m_pShader, NULL, 0 );

  m_CurrentVertexShader = Desc;

  //dh::Log( "Set vertex shader %s", Desc.c_str() );
}



void DX11Renderer::SetPixelShader( const GR::String& Desc )
{
  if ( m_CurrentPixelShader == Desc )
  {
    return;
  }

  std::map<GR::String, DX11PixelShader*>::iterator    itPS( m_BasicPixelShaders.find( Desc ) );
  if ( itPS == m_BasicPixelShaders.end() )
  {
    dh::Log( "tried to set Unknown pixel shader %s", Desc.c_str() );
    return;
  }
  m_pDeviceContext->PSSetShader( itPS->second->m_pShader, NULL, 0 );
  m_CurrentPixelShader = Desc;

  //dh::Log( "Set pixel shader %s", Desc.c_str() );
}



void DX11Renderer::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{
  FlushAllCaches();

  XBasicRenderer::SetTransform( tType, matTrix );

  switch ( tType )
  {
    case TT_PROJECTION:
      m_Matrices.Projection = matTrix;
      m_Matrices.Projection.Transpose();
      break;
    case TT_WORLD:
      m_Matrices.Model = matTrix;
      m_Matrices.Model.Transpose();

      //m_Matrices.InverseTransposeWorldMatrix = matTrix;
      //m_Matrices.InverseTransposeWorldMatrix.Inverse();
      //m_Matrices.InverseTransposeWorldMatrix.Transpose();
      break;
    case TT_VIEW:
      m_Matrices.View = matTrix;
      m_Matrices.View.Transpose();

      m_Matrices.ViewIT = matTrix;
      m_Matrices.ViewIT.Inverse();
      m_Matrices.ViewIT.Transpose();
      break;
    case TT_TEXTURE_STAGE_0:
      m_StoredTextureTransform = matTrix;
      // my shader hack uses these fields
      m_StoredTextureTransform.ms._14 = matTrix.ms._31;
      m_StoredTextureTransform.ms._24 = matTrix.ms._32;

      if ( m_RenderStates[std::make_pair( XRenderer::RS_TEXTURE_TRANSFORM, 0 )] == XRenderer::RSV_DISABLE )
      {
        // only store, do not set
        return;
      }
      m_Matrices.TextureTransform = m_StoredTextureTransform;
      break;
    default:
      dh::Log( "DX11Renderer::SetTransform unsupported transform type %d", tType );
      break;
  }

  m_QuadCache.TransformChanged( tType );
  m_QuadCache3d.TransformChanged( tType );
  m_LineCache.TransformChanged( tType );
  m_LineCache3d.TransformChanged( tType );
  m_TriangleCache.TransformChanged( tType );
  m_TriangleCache3d.TransformChanged( tType );


  // Prepare the constant buffer to send it to the graphics device.
  m_pDeviceContext->UpdateSubresource( m_pMatrixBuffer, 0, NULL, &m_Matrices, 0, 0 );
  m_pDeviceContext->VSSetConstantBuffers( 0, 1, &m_pMatrixBuffer );

  // camera pos is required for lights
  if ( m_NumActiveLights > 0 )
  {
    if ( tType == TT_WORLD )
    {
      //m_LightInfo.EyePosition.set( matTrix.ms._41, matTrix.ms._42, matTrix.ms._43 );
      m_LightInfo.EyePos.set( matTrix.ms._41, matTrix.ms._42, matTrix.ms._43 );
      m_pDeviceContext->UpdateSubresource( m_pLightBuffer, 0, NULL, &m_LightInfo, 0, 0 );
      //m_pDeviceContext->PSSetConstantBuffers( 1, 1, &m_pLightBuffer );
      m_pDeviceContext->VSSetConstantBuffers( 1, 1, &m_pLightBuffer );
      m_LightsChanged = false;
    }
  }
}



void DX11Renderer::FlushLightChanges()
{
  if ( m_LightsChanged )
  {
    m_LightsChanged = false;
    m_pDeviceContext->UpdateSubresource( m_pLightBuffer, 0, NULL, &m_LightInfo, 0, 0 );
    //m_pDeviceContext->PSSetConstantBuffers( 1, 1, &m_pLightBuffer );
    m_pDeviceContext->VSSetConstantBuffers( 1, 1, &m_pLightBuffer );
  }
}



void DX11Renderer::FlushAllCaches()
{
  FlushLightChanges();
  m_QuadCache3d.FlushCache();
  m_QuadCache.FlushCache();
  m_LineCache.FlushCache();
  m_LineCache3d.FlushCache();
  m_TriangleCache.FlushCache();
  m_TriangleCache3d.FlushCache();
}



bool DX11Renderer::Release()
{
  if ( !m_Ready )
  {
    return true;
  }
  //OutputDebugStringA( "DX11Renderer::Release called\n" );
  if ( m_pDevice == NULL )
  {
    return true;
  }

  if ( IsFullscreen() )
  {
    ToggleFullscreen();
  }

  #if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( m_pEnvironment != NULL )
  {
    ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );
    if ( pManager )
    {
      pManager->RemoveHandler( "DX11Renderer" );
    }
  }
  #endif

  // unbind all bound objects
  m_pDeviceContext->IASetInputLayout( NULL );
  m_pDeviceContext->VSSetShader( NULL, NULL, 0 );
  m_pDeviceContext->PSSetShader( NULL, NULL, 0 );
  m_pDeviceContext->RSSetState( NULL );
  m_pDeviceContext->OMSetBlendState( NULL, NULL, 0xffffffff );
  m_pDeviceContext->OMSetRenderTargets( 0, NULL, NULL );

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  if ( m_pSwapChain != NULL )
  {
    //m_pSwapChain->SetFullscreenState( FALSE, NULL );
  }
#endif

  if ( m_pBlendStateAdditive != NULL )
  {
    m_pBlendStateAdditive->Release();
    m_pBlendStateAdditive = NULL;
  }
  if ( m_pBlendStateAlphaBlend != NULL )
  {
    m_pBlendStateAlphaBlend->Release();
    m_pBlendStateAlphaBlend = NULL;
  }
  if ( m_pBlendStateNoBlend != NULL )
  {
    m_pBlendStateNoBlend->Release();
    m_pBlendStateNoBlend = NULL;
  }

  if ( m_pRasterizerStateCullBack != NULL )
  {
    m_pRasterizerStateCullBack->Release();
    m_pRasterizerStateCullBack = NULL;
  }
  if ( m_pRasterizerStateCullFront != NULL )
  {
    m_pRasterizerStateCullFront->Release();
    m_pRasterizerStateCullFront = NULL;
  }
  if ( m_pRasterizerStateCullNone != NULL )
  {
    m_pRasterizerStateCullNone->Release();
    m_pRasterizerStateCullNone = NULL;
  }
  if ( m_pRasterizerStateCullBackWireframe != NULL )
  {
    m_pRasterizerStateCullBackWireframe->Release();
    m_pRasterizerStateCullBackWireframe = NULL;
  }
  if ( m_pRasterizerStateCullFrontWireframe != NULL )
  {
    m_pRasterizerStateCullFrontWireframe->Release();
    m_pRasterizerStateCullFrontWireframe = NULL;
  }
  if ( m_pRasterizerStateCullNoneWireframe != NULL )
  {
    m_pRasterizerStateCullNoneWireframe->Release();
    m_pRasterizerStateCullNoneWireframe = NULL;
  }

  if ( m_depthStencilBuffer != NULL )
  {
    m_depthStencilBuffer->Release();
    m_depthStencilBuffer = NULL;
  }
  if ( m_depthStencilStateZBufferCheckAndWriteEnabled != NULL )
  {
    m_depthStencilStateZBufferCheckAndWriteEnabled->Release();
    m_depthStencilStateZBufferCheckAndWriteEnabled = NULL;
  }
  if ( m_depthStencilStateZBufferCheckAndWriteDisabled != NULL )
  {
    m_depthStencilStateZBufferCheckAndWriteDisabled->Release();
    m_depthStencilStateZBufferCheckAndWriteDisabled = NULL;
  }
  if ( m_depthStencilStateZBufferCheckEnabledNoWrite != NULL )
  {
    m_depthStencilStateZBufferCheckEnabledNoWrite->Release();
    m_depthStencilStateZBufferCheckEnabledNoWrite = NULL;
  }
  if ( m_depthStencilStateZBufferWriteEnabledNoCheck != NULL )
  {
    m_depthStencilStateZBufferWriteEnabledNoCheck->Release();
    m_depthStencilStateZBufferWriteEnabledNoCheck = NULL;
  }
  if ( m_depthStencilView != NULL )
  {
    m_depthStencilView->Release();
    m_depthStencilView = NULL;
  }

  if ( m_pSamplerStatePoint != NULL )
  {
    m_pSamplerStatePoint->Release();
    m_pSamplerStatePoint = NULL;
  }
  if ( m_pSamplerStateLinear != NULL )
  {
    m_pSamplerStateLinear->Release();
    m_pSamplerStateLinear = NULL;
  }

  if ( m_pFogBuffer != NULL )
  {
    m_pFogBuffer->Release();
    m_pFogBuffer = NULL;
  }
  if ( m_pMaterialBuffer != NULL )
  {
    m_pMaterialBuffer->Release();
    m_pMaterialBuffer = NULL;
  }
  if ( m_pLightBuffer != NULL )
  {
    m_pLightBuffer->Release();
    m_pLightBuffer = NULL;
  }
  if ( m_pMatrixBuffer != NULL )
  {
    m_pMatrixBuffer->Release();
    m_pMatrixBuffer = NULL;
  }

  if ( m_pTargetBackBuffer != NULL )
  {
    m_pTargetBackBuffer->Release();
    m_pTargetBackBuffer = NULL;
  }
  DestroyAllTextures();
  DestroyAllVertexBuffers();
  m_pCurrentlySetRenderTargetTexture = NULL;
  m_BasicVertexBuffers.clear();
  std::list<DX11VertexShader*>::iterator    itVS( m_VertexShaders.begin() );
  while ( itVS != m_VertexShaders.end() )
  {
    DX11VertexShader*   pShader = *itVS;

    pShader->Release();

    ++itVS;
  }
  std::list<DX11PixelShader*>::iterator    itPS( m_PixelShaders.begin() );
  while ( itPS != m_PixelShaders.end() )
  {
    DX11PixelShader*   pShader = *itPS;

    pShader->Release();

    ++itPS;
  }
  m_VertexShaders.clear();
  m_PixelShaders.clear();
  m_BasicPixelShaders.clear();
  m_BasicVertexShaders.clear();

  if ( m_pSwapChain != NULL )
  {
    m_pSwapChain->Release();
    m_pSwapChain = NULL;
  }

  if ( m_pDeviceContext != NULL )
  {
    m_pDeviceContext->Release();
    m_pDeviceContext = NULL;
  }

  /*
  // display alive objects
  ID3D11Debug*    pDebug = NULL;
  m_pDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &pDebug ) );
  pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
  pDebug->Release();
  */

  m_pDevice->Release();
  m_pDevice = NULL;
  m_hwndViewport = NULL;
  m_pEnvironment = NULL;
  m_Ready = false;
  return true;
}



bool DX11Renderer::OnResized()
{
  //dh::Log( "OnResized" );
  ReleaseBuffers();
  /*
  if ( m_pDeviceContext != NULL )
  {
    m_pDeviceContext->OMSetRenderTargets( 0, 0, 0 );
  }*/

  if ( m_pSwapChain == NULL )
  {
    //dh::Log( "OnResized SwapChain== NULL" );
    return false;
  }

  // Release all outstanding references to the swap chain's buffers.
  if ( m_pTargetBackBuffer != NULL )
  {
    m_pTargetBackBuffer->Release();
    m_pTargetBackBuffer = NULL;
  }

  HRESULT hr;


  // Preserve the existing buffer count and format.
  // Automatically choose the width and height to match the client rect for HWNDs.
  //if ( FAILED( hr = m_pSwapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 ) ) )
  //if ( FAILED( hr = m_pSwapChain->ResizeBuffers( 0, m_Width, m_Height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) ) )
  if ( FAILED( hr = m_pSwapChain->ResizeBuffers( 2, m_Width, m_Height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) ) )
  {
    // Perform error handling here!
    dh::Log( "m_pSwapChain->ResizeBuffer failed, %x", hr );
  }

  DXGI_SWAP_CHAIN_DESC    desc;
  if ( SUCCEEDED( m_pSwapChain->GetDesc( &desc ) ) )
  {
    // early return when the client size changed
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    RECT  rc;

    ::GetClientRect( desc.OutputWindow, &rc );

    /*
    dh::Log( "OnResized - buffer now %dx%d, thought to be %dx%d, window client size is %dx%d",
      desc.BufferDesc.Width,
      desc.BufferDesc.Height,
      m_Width, m_Height,
      rc.right - rc.left, rc.bottom - rc.top );*/
    if ( ( rc.right - rc.left != m_Width )
    ||   ( rc.bottom - rc.top != m_Height ) )
    {
      m_Width = rc.right - rc.left;
      m_Height = rc.bottom - rc.top;

      m_ViewPort.Width = m_Width;
      m_ViewPort.Height = m_Height;

      //dh::Log( "OnResized: ================ New Size set %dx%d", m_Width, m_Height );

      SetViewport( m_ViewPort );
      return OnResized();
    }
#endif 
  }

  

  // Get buffer and create a render-target-view.
  ID3D11Texture2D* pBuffer;
  if ( FAILED( hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBuffer ) ) )
  {
    // Perform error handling here!
    dh::Log( "m_pSwapChain->GetBuffer failed, %x", hr );
    return false;
  }

  if ( FAILED( hr = m_pDevice->CreateRenderTargetView( pBuffer, NULL, &m_pTargetBackBuffer ) ) )
  {
    // Perform error handling here!
    dh::Log( "m_pDevice->CreateRenderTargetView failed, %x", hr );
    pBuffer->Release();
    return false;
  }
  pBuffer->Release();

  if ( !RecreateBuffers() )
  {
    dh::Log( "OnResized RecreateBuffers failed" );
    return false;
  }

  m_pDeviceContext->OMSetRenderTargets( 1, &m_pTargetBackBuffer, m_depthStencilView );

  //dh::Log( "OnResized OK" );
  return true;
}



bool DX11Renderer::IsReady() const
{
  return m_Ready;
}



bool DX11Renderer::BeginScene()
{
  if ( !m_Ready )
  {
    return false;
  }

  BOOL    fullScreen = FALSE;

  if ( ( m_pSwapChain != NULL )
  &&   ( SUCCEEDED( m_pSwapChain->GetFullscreenState( &fullScreen, NULL ) ) )
  &&   ( !fullScreen )
  &&   ( !m_Windowed ) )
  {
    //dh::Log( "lost device detected?" );
    //OutputDebugStringA( "Lost device detected\n" );
    // lost device -> switch to windowed mode
    m_ForceToWindowedMode = true;
    ToggleFullscreen();
    //OutputDebugStringA( "---Lost device togglefullscreen done\n" );
  }
  if ( m_pTargetBackBuffer )
  {
    // set the render target as the back buffer
    m_pDeviceContext->OMSetRenderTargets( 1, &m_pTargetBackBuffer, m_depthStencilView );
  }

  // viewport has to be set every frame
  SetViewport( m_ViewPort );

  return true;
}



void DX11Renderer::EndScene()
{
  FlushAllCaches();
}



bool DX11Renderer::CreateSwapChain()
{
  IDXGIDevice*    pDXGIDevice = NULL;

  if ( FAILED( m_pDevice->QueryInterface( __uuidof( IDXGIDevice ), (void **)&pDXGIDevice ) ) )
  {
    return false;
  }

  IDXGIAdapter * pDXGIAdapter;
  if ( FAILED( pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), (void **)&pDXGIAdapter ) ) )
  {
    pDXGIDevice->Release();
    return false;
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  IDXGIFactory4* pIDXGIFactory;
  if ( FAILED( pDXGIAdapter->GetParent( IID_PPV_ARGS( &pIDXGIFactory ) ) ) )
  {
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }
#else
  IDXGIFactory* pIDXGIFactory;
  if ( FAILED( pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), (void **)&pIDXGIFactory ) ) )
  {
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }
#endif
  /*
  // Create a DirectX graphics interface factory.
  IDXGIFactory*   pIDXGIFactory = NULL;
  HRESULT result = CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**)&pIDXGIFactory );
  if ( FAILED( result ) )
  {
  return false;
  }
  */

  IDXGIAdapter*   adapter = NULL;
  // Use the factory to create an adapter for the primary graphics interface (video card).
  HRESULT result = pIDXGIFactory->EnumAdapters( 0, &adapter );
  if ( FAILED( result ) )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }

  // Enumerate the primary adapter output (monitor).
  IDXGIOutput*      adapterOutput = NULL;
  result = adapter->EnumOutputs( 0, &adapterOutput );
  if ( FAILED( result ) )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }

  // Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
  UINT    numModes = 0;
  result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &numModes, NULL );
  if ( FAILED( result ) )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }

  // Create a list to hold all the possible display modes for this monitor/video card combination.
  DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
  if ( !displayModeList )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }

  // Now fill the display mode list structures.
  result = adapterOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList );
  if ( FAILED( result ) )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }

  // Now go through all the display modes and find the one that matches the screen width and height.
  // When a match is found store the numerator and denominator of the refresh rate for that monitor.
  int     numerator = 0;
  int     denominator = 1;

  for ( UINT i = 0; i < numModes; i++ )
  {
    const auto&  displayMode( displayModeList[i] );

    m_DisplayModes.push_back( XRendererDisplayMode( displayMode.Width, displayMode.Height, MapFormat( displayMode.Format ) ) );

    if ( displayMode.Width == (unsigned int)m_Width )
    {
      if ( displayMode.Height == (unsigned int)m_Height )
      {
        numerator = displayModeList[i].RefreshRate.Numerator;
        denominator = displayModeList[i].RefreshRate.Denominator;
      }
    }
  }

  /*
  // Get the adapter (video card) description.
  result = adapter->GetDesc( &adapterDesc );
  if ( FAILED( result ) )
  {
  return false;
  }

  // Store the dedicated video card memory in megabytes.
  m_videoCardMemory = (int)( adapterDesc.DedicatedVideoMemory / 1024 / 1024 );

  // Convert the name of the video card to a character array and store it.
  error = wcstombs_s( &stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128 );
  if ( error != 0 )
  {
  return false;
  }*/

  // Release the display mode list.
  delete[] displayModeList;
  displayModeList = 0;

  // Release the adapter output.
  adapterOutput->Release();
  adapterOutput = 0;

  // Release the adapter.
  adapter->Release();
  adapter = 0;


#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc0;
  // Initialize the swap chain description.
  ZeroMemory( &swapChainDesc0, sizeof( swapChainDesc0 ) );

  // Set the width and height of the back buffer.
  swapChainDesc0.Width = m_Width;
  swapChainDesc0.Height = m_Height;

  // Set regular 32-bit surface for the back buffer.
  //swapChainDesc0.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc0.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  swapChainDesc0.Stereo = false;

  swapChainDesc0.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

  // Set the scan line ordering and scaling to unspecified.
  //swapChainDesc0.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc0.Scaling = DXGI_SCALING_STRETCH;

#else
  DXGI_SWAP_CHAIN_DESC swapChainDesc0;
  // Initialize the swap chain description.
  ZeroMemory( &swapChainDesc0, sizeof( swapChainDesc0 ) );

  // Set the width and height of the back buffer.
  swapChainDesc0.BufferDesc.Width = m_Width;
  swapChainDesc0.BufferDesc.Height = m_Height;

  // Set regular 32-bit surface for the back buffer.
  swapChainDesc0.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  // Set the refresh rate of the back buffer.
  if ( m_VSyncEnabled )
  {
    swapChainDesc0.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc0.BufferDesc.RefreshRate.Denominator = denominator;
  }
  else
  {
    swapChainDesc0.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc0.BufferDesc.RefreshRate.Denominator = 1;
  }
  // Set the scan line ordering and scaling to unspecified.
  swapChainDesc0.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc0.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
#endif
  // Set the usage of the back buffer.
  swapChainDesc0.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  // Set the handle for the window to render to.
  swapChainDesc0.OutputWindow = m_hwndViewport;
  // Set to full screen or windowed mode.
  if ( !m_Windowed )
  {
    swapChainDesc0.Windowed = FALSE;
  }
  else
  {
    swapChainDesc0.Windowed = TRUE;
  }
#endif

  // Turn multisampling off.
  swapChainDesc0.SampleDesc.Count = 1;
  swapChainDesc0.SampleDesc.Quality = 0;


#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  // Discard the back buffer contents after presenting.
  swapChainDesc0.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD does not exist in this SDK yet
  // Set to a single back buffer.
  swapChainDesc0.BufferCount = 1;
  // Don't set the advanced flags.
  swapChainDesc0.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#elif ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  swapChainDesc0.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
  swapChainDesc0.BufferCount = 2; // Use double-buffering to minimize latency.
  swapChainDesc0.Flags = 0;

#if ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  swapChainDesc0.Windowed = TRUE;
  swapChainDesc0.OutputWindow = m_hwndViewport;
#endif

#endif


#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  auto pWindowService = (Xtreme::IAppWindow*)m_pEnvironment->Service( "Window" );
  if ( pWindowService == NULL )
  {
    dh::Log( "No Window service found" );
    return false;
  }

  Xtreme::UniversalAppWindow*  pWnd = (Xtreme::UniversalAppWindow*)pWindowService;

  auto coreWindow = pWnd->CoreWindow;// Windows::UI::Core::CoreWindow::GetForCurrentThread();
  /*
  result = pIDXGIFactory->CreateSwapChainForComposition(
    m_pDevice,
    &swapChainDesc0,
    nullptr,
    &m_pSwapChain
  );*/
#if ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
  result = pIDXGIFactory->CreateSwapChain( m_pDevice, &swapChainDesc0, &m_pSwapChain );
#else
  result = pIDXGIFactory->CreateSwapChainForCoreWindow(
    m_pDevice,
    reinterpret_cast<IUnknown*>( coreWindow.Get() ),
    &swapChainDesc0,
    nullptr,
    &m_pSwapChain
  );
#endif

#else

  // Create the swap chain, Direct3D device, and Direct3D device context.
  result = pIDXGIFactory->CreateSwapChain( m_pDevice, &swapChainDesc0, &m_pSwapChain );
#endif
  if ( FAILED( result ) )
  {
    // Release the factory.
    pIDXGIFactory->Release();
    pIDXGIFactory = 0;

    return false;
  }

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  //result = pIDXGIFactory->MakeWindowAssociation( m_hwndViewport, DXGI_MWA_NO_ALT_ENTER );
  //result = pIDXGIFactory->MakeWindowAssociation( m_hwndViewport, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN );
  result = pIDXGIFactory->MakeWindowAssociation( m_hwndViewport, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN );
  if ( FAILED( result ) )
  {
    pIDXGIFactory->Release();
    pDXGIAdapter->Release();
    pDXGIDevice->Release();
    return false;
  }
#endif

  // Release the factory.
  pIDXGIFactory->Release();
  pIDXGIFactory = 0;
  pDXGIAdapter->Release();
  pDXGIDevice->Release();

  return true;
}



void DX11Renderer::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  FlushAllCaches();

  if ( m_pSwapChain == NULL )
  {
    return;
  }
  // The first argument instructs DXGI to block until VSync, putting the application
  // to sleep until the next VSync. This ensures we don't waste any cycles rendering
  // frames that will never be displayed to the screen.
  HRESULT hr = m_pSwapChain->Present( m_VSyncEnabled ? 1 : 0, 0 );

  if ( hr != S_OK )
  {
    //OutputDebugStringA( "Present returned not S_OK\n" );
  }

  // Discard the contents of the render target.
  // This is a valid operation only when the existing contents will be entirely
  // overwritten. If dirty or scroll rects are used, this call should be removed.
  //m_pDeviceContext->DiscardView( m_pTargetBackBuffer );

  /*
  // Discard the contents of the depth stencil.
  m_pDeviceContext->DiscardView( m_d3dDepthStencilView.Get() );
  */

  // If the device was removed either by a disconnection or a driver upgrade, we 
  // must recreate all device resources.
  if ( ( hr == DXGI_ERROR_DEVICE_REMOVED )
  ||   ( hr == DXGI_ERROR_DEVICE_RESET ) )
  {
    dh::Log( "TODO - device lost" );
    //HandleDeviceLost();
  }
  else if ( FAILED( hr ) )
  {
    dh::Log( "Present had an error!" );
  }
}



bool DX11Renderer::ToggleFullscreen()
{
  //OutputDebugStringA( "ToggleFullscreen called\n" );

  if ( m_TogglingFullScreen )
  {
    return true;
  }
  m_TogglingFullScreen = true;

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  auto view = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
  if ( view->IsFullScreenMode )
  {
    view->ExitFullScreenMode();
    Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode = Windows::UI::ViewManagement::ApplicationViewWindowingMode::Auto;
    // The SizeChanged event will be raised when the exit from full-screen mode is complete.
    m_TogglingFullScreen = false;
    return true;
  }
  else
  {
    if ( view->TryEnterFullScreenMode() )
    {
      Windows::UI::ViewManagement::ApplicationView::PreferredLaunchWindowingMode = Windows::UI::ViewManagement::ApplicationViewWindowingMode::FullScreen;
      // The SizeChanged event will be raised when the entry to full-screen mode is complete.
      m_TogglingFullScreen = false;
      return true;
    }
  }
  m_TogglingFullScreen = false;
  return false;
#endif


  // regular win32 desktop from here on
  BOOL    fullScreen = FALSE;

  HRESULT hr = S_OK;

  DXGI_MODE_DESC      modeDesc;
  modeDesc.Width                    = m_Width;
  modeDesc.Height                   = m_Height;
  modeDesc.Format                   = DXGI_FORMAT_UNKNOWN;
  modeDesc.RefreshRate.Denominator  = 0;
  modeDesc.RefreshRate.Numerator    = 0;
  modeDesc.ScanlineOrdering         = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  modeDesc.Scaling                  = DXGI_MODE_SCALING_UNSPECIFIED;

  if ( FAILED( hr = m_pSwapChain->GetFullscreenState( &fullScreen, NULL ) ) )
  {
    dh::Log( "GetFullscreenState failed (%x)", hr );
    m_TogglingFullScreen = false;
    return false;
  }

  m_TogglingToFullScreen = !fullScreen;

  // temporarely set m_Windowed to target
  m_Windowed = !fullScreen;
  if ( FAILED( hr = m_pSwapChain->ResizeTarget( &modeDesc ) ) )
  {
    dh::Log( "ResizeTarget failed (%x)", hr );
    m_TogglingFullScreen = false;
    return false;
  }


  if ( m_ForceToWindowedMode )
  {
    //dh::Log( "Forced window mode" );
    m_ForceToWindowedMode = false;
    fullScreen = true;
    m_Windowed = false;
  }

  // store window placement
  if ( !fullScreen )
  {
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    //RECT      windowRect;
    //GetWindowRect( m_hwndViewport, &windowRect );
    //m_WindowedPlacement.set( windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );

    m_WindowedPlacement.length = sizeof( WINDOWPLACEMENT );
    GetWindowPlacement( m_hwndViewport, &m_WindowedPlacement );
    //dh::Log( "Store m_WindowedPlacement as %d,%d %dx%d", windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );
#endif
  }
  //dh::Log( "SwapChain says fullscreen state is %d", fullScreen );
  //OutputDebugStringA( CMisc::printf( "SwapChain says fullscreen state is %d, calling ReleaseBuffers\n", fullScreen ) );

  //dh::Log( "calling ReleaseBuffers in ToggleFullscreen" );
  //ReleaseBuffers();

  //OnResized();
  hr = m_pSwapChain->SetFullscreenState( !fullScreen, NULL );
  if ( FAILED( hr ) )
  {
    dh::Log( "SetFullscreenState failed (%x)", hr );
    m_TogglingFullScreen = false;
    m_Windowed = !!fullScreen;
    return false;
  }
  //dh::Log( "SwapChain tried fullscreen state set to %d", !fullScreen );
  if ( FAILED( hr = m_pSwapChain->GetFullscreenState( &fullScreen, NULL ) ) )
  {
    dh::Log( "GetFullscreenState failed (%x)", hr );
    m_TogglingFullScreen = false;
    m_Windowed = !!fullScreen;
    return false;
  }
  m_Windowed = !fullScreen;
  //dh::Log( "SwapChain says fullscreen state is now %d", fullScreen );

  if ( !fullScreen )
  {
    // restore window placement
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    //dh::Log( "restore m_WindowedPlacement as %d,%d %dx%d", m_WindowedPlacement.Left, m_WindowedPlacement.Top, m_WindowedPlacement.width(), m_WindowedPlacement.height() );
    //SetWindowPos( m_hwndViewport, NULL, m_WindowedPlacement.Left, m_WindowedPlacement.Top, m_WindowedPlacement.width(), m_WindowedPlacement.height(), SWP_NOACTIVATE );
    SetWindowPlacement( m_hwndViewport, &m_WindowedPlacement );

    //RecreateBuffers();

    /*
    ID3D11Debug*      pDebug = NULL;
    if ( SUCCEEDED( m_pDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &pDebug ) ) ) )
    {
      OutputDebugStringA( "===================Before ResizeBuffers after back to window mode=================" );
      pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
      pDebug->Release();
    }*/

    //m_pSwapChain->ResizeBuffers( 2, m_WindowedPlacement.width(), m_WindowedPlacement.height(), DXGI_FORMAT_UNKNOWN, 0 );
#endif
  }

  //m_Windowed = !fullScreen;
  m_TogglingFullScreen = false;

  return true;
}



bool DX11Renderer::ReleaseBuffers()
{
  if ( m_pDeviceContext == NULL )
  {
    //dh::Log( "DX11Renderer::SetMode called with m_pDeviceContext = NULL" );
    return false;
  }
  if ( m_pDevice == NULL )
  {
    //dh::Log( "DX11Renderer::SetMode called with m_pDevice = NULL" );
    return false;
  }

  // invalidate all buffers

  // Release all vidmem objects
  SetRenderTarget( NULL );
  for ( int i = 0; i < 8; ++i )
  {
    SetTexture( i, NULL );
  }

  // unbind all bound objects
  //m_pDeviceContext->IASetInputLayout( NULL );
  //m_pDeviceContext->VSSetShader( NULL, NULL, 0 );
  //m_pDeviceContext->PSSetShader( NULL, NULL, 0 );
  //m_pDeviceContext->RSSetState( NULL );
  //m_pDeviceContext->OMSetBlendState( NULL, NULL, 0xffffffff );
  m_pDeviceContext->OMSetRenderTargets( 0, NULL, NULL );

  if ( m_depthStencilBuffer != NULL )
  {
    m_depthStencilBuffer->Release();
    m_depthStencilBuffer = NULL;
  }
  if ( m_depthStencilView != NULL )
  {
    m_depthStencilView->Release();
    m_depthStencilView = NULL;
  }
  return true;
}



bool DX11Renderer::SetMode( XRendererDisplayMode& DisplayMode )
{
  //dh::Log( "SetMode called" );
  if ( m_Windowed == !DisplayMode.FullScreen )
  {
    //dh::Log( "m_Windowed == !DisplayMode.FullScreen" );
    //ReleaseBuffers();

    bool    olindowMode = m_Windowed;

    // still try?
    m_Width = DisplayMode.Width;
    m_Height = DisplayMode.Height;

    if ( DisplayMode.FullScreen )
    {
      DXGI_MODE_DESC      modeDesc;
      modeDesc.Width = m_Width;
      modeDesc.Height = m_Height;
      modeDesc.Format = DXGI_FORMAT_UNKNOWN;
      modeDesc.RefreshRate.Denominator = 0;
      modeDesc.RefreshRate.Numerator = 0;
      modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

      m_pSwapChain->ResizeTarget( &modeDesc );
    }

    return OnResized();
    //return RecreateBuffers();
  }
  if ( m_Windowed != !DisplayMode.FullScreen )
  {
    //dh::Log( "m_Windowed != !DisplayMode.FullScreen" );
    m_Width   = DisplayMode.Width;
    m_Height  = DisplayMode.Height;

    if ( !ToggleFullscreen() )
    {
      //dh::Log( "-failed" );
      return false;
    }
  }
  return true;
}



bool DX11Renderer::RecreateBuffers()
{
  //dh::Log( "Recreating buffers for size %dx%d", m_Width, m_Height );
  //OutputDebugStringA( CMisc::printf( "Recreating buffers for size %dx%d\n", m_Width, m_Height ) );

  if ( m_depthStencilBuffer == NULL )
  {
    // rebuild released buffers
    D3D11_TEXTURE2D_DESC          depthBufferDesc;

    ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = m_Width;
    depthBufferDesc.Height = m_Height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    HRESULT result = m_pDevice->CreateTexture2D( &depthBufferDesc, NULL, &m_depthStencilBuffer );
    if ( FAILED( result ) )
    {
      dh::Log( "CreateTexture2D m_depthStencilBuffer failed (%x)\n", result );
      return false;
    }
  }
  if ( m_depthStencilView == NULL )
  {
    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

    ZeroMemory( &depthStencilViewDesc, sizeof( depthStencilViewDesc ) );

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    HRESULT result = m_pDevice->CreateDepthStencilView( m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView );
    if ( FAILED( result ) )
    {
      dh::Log( "CreateDepthStencilView m_depthStencilView failed (%x)", result );
      return false;
    }
  }
  m_pDeviceContext->OMSetRenderTargets( 1, &m_pTargetBackBuffer, m_depthStencilView );
  return true;
}



void DX11Renderer::RestoreAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    if ( pTexture->m_LoadedFromFile.empty() )
    {
      // eine bloss erzeugte Textur
      DX11Texture*    pDX11Texture = (DX11Texture*)pTexture;

      // sicher gehen
      pDX11Texture->Release();

      if ( !CreateNativeTextureResources( pDX11Texture ) )
      {
        Log( "Renderer.General", "CreateTexture failed" );
      }

      GR::u32     MipMapLevel = 0;
      std::list<GR::Graphic::ImageData>::iterator   itID( pDX11Texture->m_StoredImageData.begin() );
      while ( itID != pDX11Texture->m_StoredImageData.end() )
      {
        CopyDataToTexture( pDX11Texture, *itID, 0, MipMapLevel );

        ++itID;
        ++MipMapLevel;
      }
      if ( pDX11Texture->AllowUsageAsRenderTarget )
      {
        // TODO
      }
    }
    else
    {
      if ( m_pEnvironment == NULL )
      {
        return;
      }
      ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
      if ( pManager == NULL )
      {
        return;
      }
      GR::Graphic::ImageData*   pData = pManager->LoadData( pTexture->m_LoadedFromFile.c_str() );
      if ( pData )
      {
        pData->ConvertSelfTo( pTexture->m_ImageFormat, 0, pTexture->m_ColorKey != 0, pTexture->m_ColorKey, 0, 0, 0, 0, pTexture->m_ColorKeyReplacementColor );

        XTextureBase*    pDX11Texture = (XTextureBase*)pTexture;

        // sicher gehen
        pDX11Texture->Release();

        if ( CreateNativeTextureResources( pDX11Texture ) )
        {
          CopyDataToTexture( pTexture, *pData );

          // Mipmap-Levels einlesen
          std::list<GR::String>::iterator    it( pDX11Texture->FileNames.begin() );

          GR::u32     Level = 1;

          while ( it != pDX11Texture->FileNames.end() )
          {
            GR::String&  strPath( *it );

            GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), ( GR::Graphic::eImageFormat )pDX11Texture->m_ImageFormat, pDX11Texture->m_ColorKey, pDX11Texture->m_ColorKeyReplacementColor );
            if ( pData == NULL )
            {
              Log( "Renderer.General", CMisc::printf( "DX8Renderer:: RestoreAllTextures failed to load MipMap (%s)", strPath.c_str() ) );
            }
            else
            {
              CopyDataToTexture( pTexture, *pData, pDX11Texture->m_ColorKey, Level );
              delete pData;
            }
            ++it;
            ++Level;
          }
        }
        else
        {
          Log( "Renderer.General", "Create Texture Failed" );
        }
        delete pData;
      }
    }
    ++it;
  }
}



bool DX11Renderer::IsFullscreen()
{
  if ( m_TogglingFullScreen )
  {
    return m_TogglingToFullScreen;
  }
  return !m_Windowed;
  /*
  BOOL    fullScreen = FALSE;

  if ( m_pSwapChain == NULL )
  {
    return false;
  }
  if ( FAILED( m_pSwapChain->GetFullscreenState( &fullScreen, NULL ) ) )
  {
    return false;
  }
  // during startup?
  if ( fullScreen != m_Windowed )
  {
    return !m_Windowed;
  }
  return !!fullScreen;*/
}



void DX11Renderer::SetCullFillMode( GR::u32 CullMode, GR::u32 FillMode )
{
  switch ( CullMode )
  {
    case XRenderer::RSV_CULL_CCW:
      if ( FillMode == RSV_FILL_SOLID )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullBack );
      }
      else if ( FillMode == RSV_FILL_WIREFRAME )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullBackWireframe );
      }
      break;
    case XRenderer::RSV_CULL_CW:
      if ( FillMode == RSV_FILL_SOLID )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullFront );
      }
      else if ( FillMode == RSV_FILL_WIREFRAME )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullFrontWireframe );
      }
      break;
    case XRenderer::RSV_CULL_NONE:
      if ( FillMode == RSV_FILL_SOLID )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullNone );
      }
      else if ( FillMode == RSV_FILL_WIREFRAME )
      {
        m_pDeviceContext->RSSetState( m_pRasterizerStateCullNoneWireframe );
      }
      break;
  }
}



bool DX11Renderer::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
{
  tMapRenderStates::iterator    it( m_RenderStates.find( std::make_pair( rState, Stage ) ) );
  if ( it != m_RenderStates.end() )
  {
    if ( it->second == rValue )
    {
      return true;
    }
  }

  switch ( rState )
  {
    case RS_AMBIENT:
      //m_LightInfo.GlobalAmbient = ColorValue( rValue );
      m_LightInfo.Ambient = ColorValue( rValue );
      m_LightsChanged = true;
      if ( m_LightingEnabled )
      {
        FlushAllCaches();
      }
      break;
    case RS_CLEAR_COLOR:
      m_ClearColor[2] = ( ( rValue >>  0 ) & 0xff ) / 255.0f;
      m_ClearColor[1] = ( ( rValue >>  8 ) & 0xff ) / 255.0f;
      m_ClearColor[0] = ( ( rValue >> 16 ) & 0xff ) / 255.0f;
      m_ClearColor[3] = ( ( rValue >> 24 ) & 0xff ) / 255.0f;
      break;
    case RS_LIGHTING:
      if ( rValue == XRenderer::RSV_ENABLE )
      {
        if ( !m_LightingEnabled )
        {
          FlushAllCaches();
          m_LightingEnabled = true;
        }
      }
      else if( rValue == XRenderer::RSV_DISABLE )
      {
        if ( m_LightingEnabled )
        {
          FlushAllCaches();
          m_LightingEnabled = false;
        }
      }
      break;
    case RS_LIGHT:
      if ( Stage < 8 )
      {
        if ( rValue == XRenderer::RSV_ENABLE )
        {
          if ( !m_LightEnabled[Stage] )
          {
            FlushAllCaches();
            //m_LightInfo.Lights[Stage].Enabled = true;
            m_LightEnabled[Stage] = true;
            ++m_NumActiveLights;
          }
        }
        else if ( rValue == XRenderer::RSV_DISABLE )
        {
          if ( m_LightEnabled[Stage] )
          {
            FlushAllCaches();
            m_LightEnabled[Stage] = false;
            //m_LightInfo.Lights[Stage].Enabled = false;
            --m_NumActiveLights;
          }
        }
      }
      break;
    case RS_CULLMODE:
      FlushAllCaches();
      SetCullFillMode( rValue, m_RenderStates[std::make_pair( RS_FILL_MODE, 0 )] );
      break;
    case RS_FILL_MODE:
      FlushAllCaches();
      SetCullFillMode( m_RenderStates[std::make_pair( RS_CULLMODE, 0 )], rValue );
      break;
    case RS_ZBUFFER:
      if ( Stage == 0 )
      {
        if ( ( rValue == XRenderer::RSV_ENABLE )
        ||   ( rValue == XRenderer::RSV_DISABLE ) )
        {
          FlushAllCaches();

          ID3D11DepthStencilState*    pStateToSet = m_depthStencilStateZBufferCheckAndWriteEnabled;

          if ( m_RenderStates[std::make_pair( XRenderer::RS_ZWRITE, 0 )] == XRenderer::RSV_ENABLE )
          {
            pStateToSet = ( rValue == XRenderer::RSV_ENABLE ) ? m_depthStencilStateZBufferCheckAndWriteEnabled : m_depthStencilStateZBufferWriteEnabledNoCheck;
          }
          else if ( m_RenderStates[std::make_pair( XRenderer::RS_ZWRITE, 0 )] == XRenderer::RSV_DISABLE )
          {
            pStateToSet = ( rValue == XRenderer::RSV_ENABLE ) ? m_depthStencilStateZBufferCheckEnabledNoWrite : m_depthStencilStateZBufferCheckAndWriteDisabled;
          }

          m_pDeviceContext->OMSetDepthStencilState( pStateToSet, 1 );
        }
      }
      break;
    case RS_ZWRITE:
      if ( Stage == 0 )
      {
        if ( ( rValue == XRenderer::RSV_ENABLE )
        ||   ( rValue == XRenderer::RSV_DISABLE ) )
        {
          FlushAllCaches();

          ID3D11DepthStencilState*    pStateToSet = m_depthStencilStateZBufferCheckAndWriteEnabled;

          if ( m_RenderStates[std::make_pair( XRenderer::RS_ZBUFFER, 0 )] == XRenderer::RSV_ENABLE )
          {
            pStateToSet = ( rValue == XRenderer::RSV_ENABLE ) ? m_depthStencilStateZBufferCheckAndWriteEnabled : m_depthStencilStateZBufferCheckEnabledNoWrite;
          }
          else if ( m_RenderStates[std::make_pair( XRenderer::RS_ZBUFFER, 0 )] == XRenderer::RSV_DISABLE )
          {
            pStateToSet = ( rValue == XRenderer::RSV_ENABLE ) ? m_depthStencilStateZBufferWriteEnabledNoCheck : m_depthStencilStateZBufferCheckAndWriteDisabled;
          }

          m_pDeviceContext->OMSetDepthStencilState( pStateToSet, 1 );
        }
      }
      break;
    case RS_MINFILTER:
    case RS_MAGFILTER:
    case RS_MIPFILTER:
      FlushAllCaches();
      // TODO - THAT'S A HACK! THERE OUGHT TO BE DIFFERENT SAMPLERS FOR ALL COMBINATIONS!
      if ( rValue == RSV_FILTER_POINT )
      {
        m_pDeviceContext->VSSetSamplers( 0, 1, &m_pSamplerStatePoint );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerStatePoint );
      }
      else
      {
        m_pDeviceContext->VSSetSamplers( 0, 1, &m_pSamplerStateLinear );
        m_pDeviceContext->PSSetSamplers( 0, 1, &m_pSamplerStateLinear );
      }
      break;
    case RS_TEXTURE_TRANSFORM:
      FlushAllCaches();
      if ( rValue == RSV_DISABLE )
      {
        math::matrix4     matTemp = m_StoredTextureTransform;

        SetTransform( XRenderer::TT_TEXTURE_STAGE_0, math::matrix4().Identity() );

        m_StoredTextureTransform = matTemp;
      }
      else if ( rValue == RSV_TEXTURE_TRANSFORM_COUNT2 )
      {
        SetTransform( XRenderer::TT_TEXTURE_STAGE_0, m_StoredTextureTransform );
      }
      break;
    case RS_FOG_COLOR:
      m_Fog.FogColor = ColorValue( rValue );
      if ( m_Fog.FogType != 0 )
      {
        SetFogInfo();
      }
      break;
    case RS_FOG_ENABLE:
      if ( rValue == RSV_ENABLE )
      {
        if ( m_Fog.FogType != m_CachedFontType )
        {
          m_Fog.FogType = m_CachedFontType;
        }
      }
      else if ( m_Fog.FogType != 0 )
      {
        m_CachedFontType = m_Fog.FogType;
        m_Fog.FogType = 0;
      }
      SetFogInfo();
      break;
    case RS_FOG_DENSITY:
      m_Fog.FogDensity = *( GR::f32* )&rValue;
      if ( m_Fog.FogType != 0 )
      {
        SetFogInfo();
      }
      break;
    case RS_FOG_START:
      m_Fog.FogStart = *( GR::f32* )&rValue;
      if ( m_Fog.FogType != 0 )
      {
        SetFogInfo();
      }
      break;
    case RS_FOG_END:
      m_Fog.FogEnd = *( GR::f32* )&rValue;
      if ( m_Fog.FogType != 0 )
      {
        SetFogInfo();
      }
      break;
    case RS_FOG_VERTEXMODE:
      if ( ( rValue >= RSV_FOG_EXP )
      &&   ( rValue <= RSV_FOG_LINEAR ) )
      {
        if ( m_Fog.FogType == 0 )
        {
          if ( m_RenderStates[std::make_pair( RS_FOG_ENABLE, Stage )] == RSV_ENABLE )
          {
            // fog is enabled!
            m_Fog.FogType = rValue - RSV_FOG_NONE;
            m_CachedFontType = rValue - RSV_FOG_NONE;
            SetFogInfo();
          }
        }
        else if ( m_Fog.FogType != rValue - RSV_FOG_NONE )
        {
          m_Fog.FogType = rValue - RSV_FOG_NONE;
          m_CachedFontType = rValue - RSV_FOG_NONE;
          SetFogInfo();
        }
      }
      break;
  }
  m_RenderStates[std::make_pair( rState, Stage )] = rValue;
  return true;
}



bool DX11Renderer::SetViewport( const XViewport& Viewport )
{
  if ( m_pDeviceContext == NULL )
  {
    return false;
  }
  FlushAllCaches();

  m_ViewPort = Viewport;

  CD3D11_VIEWPORT    viewPort;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  viewPort.Width    = (GR::f32)virtualX * Viewport.Width;
  viewPort.Height   = (GR::f32)virtualY * Viewport.Height;
  viewPort.TopLeftX = m_Canvas.Left + (GR::f32)virtualX * Viewport.X;
  viewPort.TopLeftY = m_Canvas.Top + (GR::f32)virtualY * Viewport.Y;
  viewPort.MinDepth = Viewport.MinZ;
  viewPort.MaxDepth = Viewport.MaxZ;

  if ( viewPort.TopLeftX >= m_Canvas.Left + m_Canvas.Width() )
  {
    viewPort.TopLeftX = (GR::f32)m_Canvas.Left + m_Canvas.Width();
    viewPort.Width = 0;
  }
  if ( viewPort.TopLeftY >= m_Canvas.Top + m_Canvas.Height() )
  {
    viewPort.TopLeftY = (GR::f32)m_Canvas.Top + m_Canvas.Height();
    viewPort.Height = 0;
  }
  if ( viewPort.TopLeftX + viewPort.Width > m_Canvas.Left + m_Canvas.Width() )
  {
    viewPort.Width = m_Canvas.Left + m_Canvas.Width() - viewPort.TopLeftX;
  }
  if ( viewPort.TopLeftY + viewPort.Height> m_Canvas.Top + m_Canvas.Height() )
  {
    viewPort.Height = m_Canvas.Top + m_Canvas.Height() - viewPort.TopLeftY;
  }

  // adjust ortho matrix?
  m_Matrices.ScreenCoord2d.OrthoOffCenterLH( viewPort.TopLeftX, 
                                             viewPort.TopLeftX + viewPort.Width,
                                             viewPort.TopLeftY + viewPort.Height,
                                             viewPort.TopLeftY, 
                                             0.0f, 
                                             1.0f );

  //dh::Log( "Set viewport %f,%f  %fx%f", viewPort.TopLeftX, viewPort.TopLeftY, viewPort.Width, viewPort.Height );

  m_Matrices.ScreenCoord2d.Transpose();
  // Prepare the constant buffer to send it to the graphics device.
  m_pDeviceContext->UpdateSubresource( m_pMatrixBuffer, 0, NULL, &m_Matrices, 0, 0 );
  m_pDeviceContext->VSSetConstantBuffers( 0, 1, &m_pMatrixBuffer );

  m_pDeviceContext->RSSetViewports( 1, &viewPort );
  return true;
}



bool DX11Renderer::SetTrueViewport( const XViewport& Viewport )
{ 
  FlushAllCaches();

  m_VirtualViewport = Viewport;

  CD3D11_VIEWPORT    viewPort;

  viewPort.Width = ( GR::f32 )Viewport.Width;
  viewPort.Height = ( GR::f32 )Viewport.Height;
  viewPort.TopLeftX = ( GR::f32 )Viewport.X;
  viewPort.TopLeftY = ( GR::f32 )Viewport.Y;
  viewPort.MinDepth = Viewport.MinZ;
  viewPort.MaxDepth = Viewport.MaxZ;

  if ( viewPort.TopLeftX >= Width() )
  {
    viewPort.TopLeftX = ( GR::f32 )Width();
    viewPort.Width = 0;
  }
  if ( viewPort.TopLeftY >= Height() )
  {
    viewPort.TopLeftY = ( GR::f32 )Height();
    viewPort.Height = 0;
  }
  if ( viewPort.TopLeftX + viewPort.Width > Width() )
  {
    viewPort.Width = Width() - viewPort.TopLeftX;
  }
  if ( viewPort.TopLeftY + viewPort.Height> Height() )
  {
    viewPort.Height = Height() - viewPort.TopLeftY;
  }

  m_pDeviceContext->RSSetViewports( 1, &viewPort );
  return true;
}



GR::u32 DX11Renderer::Width()
{
  return m_Width;
}



GR::u32 DX11Renderer::Height()
{
  return m_Height;
}



void DX11Renderer::Clear( bool ClearColor, bool ClearZ )
{
  FlushAllCaches();
  if ( ClearColor )
  {
    if ( m_pCurrentlySetRenderTargetTexture != NULL )
    {
      m_pDeviceContext->ClearRenderTargetView( m_pCurrentlySetRenderTargetTexture->m_pTextureRenderTargetView, m_ClearColor );
    }
    else if ( m_pTargetBackBuffer != NULL )
    {
      m_pDeviceContext->ClearRenderTargetView( m_pTargetBackBuffer, m_ClearColor );
    }
  }
  if ( ClearZ )
  {
    if ( m_depthStencilView == NULL )
    {
      dh::Log( "Trying to clear NULL m_depthStencilView!" );
    }
    else
    {
      m_pDeviceContext->ClearDepthStencilView( m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
    }
  }
}



bool DX11Renderer::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  auto    mappedFormat = MapFormat( imgFormat );

  switch ( mappedFormat )
  {
    case DXGI_FORMAT_P8:
      // no indexed formats!
      return false;
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_B5G6R5_UNORM:
      // Requires DXGI 1.2 or later. DXGI 1.2 types are only supported on systems with Direct3D 11.1 or later (but also says >= 9.1, go figure)
      if ( m_FeatureLevel >= D3D_FEATURE_LEVEL_11_1 )
      {
        return true;
      }
      return false;
  }
  return true;
}



DXGI_FORMAT DX11Renderer::MapFormat( GR::Graphic::eImageFormat imgFormat )
{
  DXGI_FORMAT     d3dFmt = DXGI_FORMAT_UNKNOWN;

  switch ( imgFormat )
  {
    case GR::Graphic::IF_A1R5G5B5:
    case GR::Graphic::IF_X1R5G5B5:
      d3dFmt = DXGI_FORMAT_B5G5R5A1_UNORM;
      break;
    case GR::Graphic::IF_A4R4G4B4:
      d3dFmt = DXGI_FORMAT_B4G4R4A4_UNORM;
      break;
    case GR::Graphic::IF_A8:
      d3dFmt = DXGI_FORMAT_A8_UNORM;
      break;
    case GR::Graphic::IF_A8R8G8B8:
      d3dFmt = DXGI_FORMAT_B8G8R8A8_UNORM;
      break;
    case GR::Graphic::IF_INDEX8:
      d3dFmt = DXGI_FORMAT_P8;
      break;
    case GR::Graphic::IF_R5G6B5:
      d3dFmt = DXGI_FORMAT_B5G6R5_UNORM;
      break;
    case GR::Graphic::IF_R8G8B8:
    case GR::Graphic::IF_X8R8G8B8:
      d3dFmt = DXGI_FORMAT_B8G8R8X8_UNORM;
      break;
    default:
      dh::Log( "DX11Renderer::MapFormat Unsupported format %d", imgFormat );
      break;
  }
  return d3dFmt;
}



GR::Graphic::eImageFormat DX11Renderer::MapFormat( DXGI_FORMAT Format )
{
  GR::Graphic::eImageFormat  imgFmt = GR::Graphic::IF_UNKNOWN;

  switch ( Format )
  {
    case DXGI_FORMAT_B5G5R5A1_UNORM:
      imgFmt = GR::Graphic::IF_A1R5G5B5;
      break;
    case DXGI_FORMAT_B4G4R4A4_UNORM:
      imgFmt = GR::Graphic::IF_A4R4G4B4;
      break;
    case DXGI_FORMAT_A8_UNORM:
      imgFmt = GR::Graphic::IF_A8;
      break;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
      // TODO - not correct!
      imgFmt = GR::Graphic::IF_A8R8G8B8;
      break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      imgFmt = GR::Graphic::IF_A8R8G8B8;
      break;
    case DXGI_FORMAT_P8:
      imgFmt = GR::Graphic::IF_INDEX8;
      break;
    case DXGI_FORMAT_B5G6R5_UNORM:
      imgFmt = GR::Graphic::IF_R5G6B5;
      break;
    case DXGI_FORMAT_B8G8R8X8_UNORM:
      imgFmt = GR::Graphic::IF_X8R8G8B8;
      break;
    default:
      dh::Log( "DX11Renderer::MapFormat Unsupported D3D format %d", Format );
      break;
  }
  return imgFmt;
}



XTexture* DX11Renderer::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormatArg, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 )
  ||   ( m_pDevice == NULL ) )
  {
    return NULL;
  }

  GR::Graphic::eImageFormat    imgFormat( imgFormatArg );


  if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
  {
    // unterstützte Wechsel
    if ( imgFormat == GR::Graphic::IF_PALETTED )
    {
      imgFormat = GR::Graphic::IF_X8R8G8B8;
    }
    else if ( imgFormat == GR::Graphic::IF_R8G8B8 )
    {
      imgFormat = GR::Graphic::IF_X8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        return NULL;
      }
    }
    else if ( imgFormat == GR::Graphic::IF_X1R5G5B5 )
    {
      imgFormat = GR::Graphic::IF_R5G6B5;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        imgFormat = GR::Graphic::IF_X8R8G8B8;
        if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
        {
          return NULL;
        }
      }
    }
    else if ( imgFormat == GR::Graphic::IF_A1R5G5B5 )
    {
      imgFormat = GR::Graphic::IF_A8R8G8B8;
      if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
      {
        return NULL;
      }
    }
    else
    {
      return NULL;
    }
  }

  int   iTWidth = Width,
        iTHeight = Height;

  int iDummy = 1;
  for ( int i = 0; i < 32; i++ )
  {
    if ( iTWidth <= iDummy )
    {
      iTWidth = iDummy;
      break;
    }
    iDummy <<= 1;
  }
  iDummy = 1;
  for ( int i = 0; i < 32; i++ )
  {
    if ( iTHeight <= iDummy )
    {
      iTHeight = iDummy;
      break;
    }
    iDummy <<= 1;
  }


  DX11Texture*          pTexture = NULL;

  pTexture = new( std::nothrow )DX11Texture();
  if ( pTexture == NULL )
  {
    return NULL;
  }

  pTexture->m_MipMapLevels = MipMapLevels;
  if ( pTexture->m_MipMapLevels == 0 )
  {
    pTexture->m_MipMapLevels = 1;
  }
  pTexture->AllowUsageAsRenderTarget = AllowUsageAsRenderTarget;
  pTexture->m_ImageSourceSize.Set( Width, Height );
  pTexture->m_ImageFormat = imgFormat;

  if ( !CreateNativeTextureResources( pTexture ) )
  {
    delete pTexture;
    return false;
  }

  AddTexture( pTexture );

  return pTexture;
}



bool DX11Renderer::CreateNativeTextureResources( XTextureBase* pTextureBase )
{
  DX11Texture*    pTexture = (DX11Texture*)pTextureBase;

  D3D11_TEXTURE2D_DESC desc;

  ZeroMemory( &desc, sizeof( desc ) );

  desc.Width      = pTexture->m_SurfaceSize.x;
  desc.Height     = pTexture->m_SurfaceSize.y;
  if ( pTexture->m_SurfaceSize.x == 0 )
  {
    desc.Width = pTexture->m_ImageSourceSize.x;
    desc.Height = pTexture->m_ImageSourceSize.y;
  }
  desc.MipLevels  = pTexture->m_MipMapLevels;
  desc.ArraySize  = pTexture->m_MipMapLevels;
  desc.Format = MapFormat( pTexture->m_ImageFormat );
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.MiscFlags = 0;

  if ( pTexture->AllowUsageAsRenderTarget )
  {
    // allow CPU read/write
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  }

  ID3D11Texture2D*    pTexture2d = NULL;

  if ( FAILED( m_pDevice->CreateTexture2D( &desc, NULL, &pTexture2d ) ) )
  {
    dh::Log( "CreateTexture: CreateTexture failed" );
    return NULL;
  }

  D3D11_TEXTURE2D_DESC    texDesc;
  pTexture2d->GetDesc( &texDesc );

  pTexture->m_SurfaceSize.Set( texDesc.Width, texDesc.Height );
  pTexture->m_D3DFormat = texDesc.Format;

  // re-map since a few formats do not exist anymore (e.g. GR::Graphic::IF_X1R5G5B5)
  pTexture->m_ImageFormat = MapFormat( pTexture->m_D3DFormat );//imgFormat;
  pTexture->m_pTexture = pTexture2d;


  // render target view required if wanted as render target
  if ( pTexture->AllowUsageAsRenderTarget )
  {
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

    ZeroMemory( &renderTargetViewDesc, sizeof( renderTargetViewDesc ) );

    renderTargetViewDesc.Format = texDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    // Create the render target view
    if ( FAILED( m_pDevice->CreateRenderTargetView( pTexture2d, &renderTargetViewDesc, &pTexture->m_pTextureRenderTargetView ) ) )
    {
      pTexture->Release();
      delete pTexture;
      return false;
    }
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC   shaderResDesc;

  ZeroMemory( &shaderResDesc, sizeof( shaderResDesc ) );

  shaderResDesc.Format = texDesc.Format;
  shaderResDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResDesc.Texture2D.MostDetailedMip = 0;
  if ( pTexture->AllowUsageAsRenderTarget )
  {
    shaderResDesc.Texture2D.MipLevels = 1;
  }
  else
  {
    shaderResDesc.Texture2D.MipLevels = -1;   // -1 = consider all mip map levels
  }

  if ( FAILED( m_pDevice->CreateShaderResourceView( pTexture2d, &shaderResDesc, &pTexture->m_pTextureShaderResourceView ) ) )
  {
    pTexture->Release();
    return false;
  }
  return true;
}



XTexture* DX11Renderer::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), MipMapLevels );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



XTexture* DX11Renderer::LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  GR::String path = FileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), imgFormatToConvert, ColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    dh::Log( "Failed to load texture %s", FileName );
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData, MipMapLevels );
  if ( pTexture == NULL )
  {
    delete pData;
    dh::Log( "Failed to create texture for %s", FileName );
    return NULL;
  }
  pTexture->m_ColorKey        = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;
}



void DX11Renderer::SetTexture( GR::u32 Stage, XTexture* pTexture )
{
  if ( Stage >= 8 )
  {
    return;
  }

  if ( m_SetTextures[Stage] == pTexture )
  {
    return;
  }

  m_SetTextures[Stage] = pTexture;
  if ( pTexture == NULL )
  {
    // TODO - need flags if texture is set in stage?
    return;
  }
  // only set on pixel shader (for now)
  DX11Texture*    pDXTexture = (DX11Texture*)pTexture;
  m_pDeviceContext->PSSetShaderResources( Stage, 1, &pDXTexture->m_pTextureShaderResourceView );
}



bool DX11Renderer::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{
  FlushAllCaches();
  if ( pTexture == NULL )
  {
    dh::Error( "CopyDataToTexture: Texture was NULL" );
    return false;
  }

  DX11Texture*    pDXTexture = (DX11Texture*)pTexture;

  if ( MipMapLevel >= pDXTexture->m_MipMapLevels )
  {
    dh::Error( "CopyDataToTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, ( (DX11Texture*)pTexture )->m_MipMapLevels );
    return false;
  }

  GR::u32       Width = pTexture->m_SurfaceSize.x;
  GR::u32       Height = pTexture->m_SurfaceSize.y;

  GR::u32       CurLevel = MipMapLevel;

  while ( CurLevel >= 1 )
  {
    Width /= 2;
    if ( Width == 0 )
    {
      Width = 1;
    }
    Height /= 2;
    if ( Height == 0 )
    {
      Height = 1;
    }
    --CurLevel;
  }

  if ( ( ImageData.Width() > (int)Width )
  ||   ( ImageData.Height() > (int)Height ) )
  {
    dh::Error( "CopyDataToTexture: Sizes mismatching %dx%d != %dx%d", ImageData.Width(), ImageData.Height(), Width, Height );
    return false;
  }

  GR::Graphic::ImageData      convertedData( ImageData );

  if ( !convertedData.ConvertSelfTo( pDXTexture->m_ImageFormat ) )
  {
    dh::Error( "CopyDataToTexture ConvertInto failed" );
    return false;
  }

  D3D11_BOX   updateRect;

  updateRect.left = 0;
  updateRect.top = 0;
  updateRect.right = convertedData.Width();
  updateRect.bottom = convertedData.Height();
  updateRect.front = 0;
  updateRect.back = 1;
  m_pDeviceContext->UpdateSubresource( pDXTexture->m_pTexture,
                                       D3D11CalcSubresource( MipMapLevel, 0, pDXTexture->m_MipMapLevels ),
                                       &updateRect,
                                       convertedData.Data(),
                                       convertedData.BytesPerLine(),
                                       0 );

  // store copied data
  while ( pDXTexture->m_StoredImageData.size() <= MipMapLevel )
  {
    pDXTexture->m_StoredImageData.push_back( GR::Graphic::ImageData() );
  }
  pDXTexture->m_StoredImageData.back() = ImageData;
  return true;
}



// Lights
bool DX11Renderer::SetLight( GR::u32 LightIndex, XLight& Light )
{
  if ( LightIndex >= 8 )
  {
    return false;
  }

  FlushAllCaches();

  bool    hadChange = false;
  if ( m_SetLights[LightIndex].m_Type != XLight::LT_INVALID )
  {
    if ( m_LightEnabled[LightIndex] )
    {
      --m_NumActiveLights;
      hadChange = true;
    }
  }
  m_SetLights[LightIndex] = Light;

  if ( Light.m_Type == XLight::LT_DIRECTIONAL )
  {
    //m_LightInfo.Lights[LightIndex].Color     = ColorValue( Light.m_Diffuse );
    m_LightInfo.Lights[LightIndex].Diffuse    = ColorValue( Light.m_Diffuse );
    m_LightInfo.Lights[LightIndex].Ambient    = ColorValue( Light.m_Ambient );
    m_LightInfo.Lights[LightIndex].Direction  = Light.m_Direction.normalized();
    m_LightInfo.Lights[LightIndex].Specular   = ColorValue( Light.m_Specular );
    //m_LightInfo.Lights[LightIndex].LightType  = Light.m_Type;
  }
   
  if ( m_SetLights[LightIndex].m_Type != XLight::LT_INVALID )
  {
    if ( m_LightEnabled[LightIndex] )
    {
      ++m_NumActiveLights;
      hadChange = true;
    }
  }
  if ( hadChange )
  {
    m_pDeviceContext->UpdateSubresource( m_pLightBuffer, 0, NULL, &m_LightInfo, 0, 0 );
    //m_pDeviceContext->PSSetConstantBuffers( 1, 1, &m_pLightBuffer );
    m_pDeviceContext->VSSetConstantBuffers( 1, 1, &m_pLightBuffer );
    m_LightsChanged = false;
  }
  else
  {
    m_LightsChanged = true;
  }
  return true;
}



bool DX11Renderer::SetMaterial( const XMaterial& Material )
{
  FlushAllCaches();

  m_Material.Ambient        = ColorValue( Material.Ambient );
  m_Material.Emissive       = ColorValue( Material.Emissive );
  m_Material.Diffuse        = ColorValue( Material.Diffuse );
  m_Material.Specular       = ColorValue( Material.Specular );
  m_Material.SpecularPower  = Material.Power;

  m_Material.Ambient.A  = 1.0f;
  m_Material.Emissive.A = 1.0f;
  m_Material.Diffuse.A  = 1.0f;
  m_Material.Specular.A = 1.0f;

  if ( m_Material.Diffuse == ColorValue( 0 ) )
  {
    m_Material.Diffuse = ColorValue( 0xffffffff );
  }

  m_pDeviceContext->UpdateSubresource( m_pMaterialBuffer, 0, NULL, &m_Material, 0, 0 );
  //m_pDeviceContext->PSSetConstantBuffers( 2, 1, &m_pMaterialBuffer );
  m_pDeviceContext->VSSetConstantBuffers( 2, 1, &m_pMaterialBuffer );
  return true;
}



bool DX11Renderer::SetFogInfo()
{
  FlushAllCaches();

  m_pDeviceContext->UpdateSubresource( m_pFogBuffer, 0, NULL, &m_Fog, 0, 0 );
  m_pDeviceContext->VSSetConstantBuffers( 3, 1, &m_pFogBuffer );
  m_pDeviceContext->PSSetConstantBuffers( 3, 1, &m_pFogBuffer );
  return true;
}



XVertexBuffer* DX11Renderer::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  if ( !m_pDevice )
  {
    dh::Log( "DX11Renderer::CreateVertexBuffer: no Device" );
    return NULL;
  }

  DX11VertexBuffer*   pBuffer = new DX11VertexBuffer( this );

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* DX11Renderer::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  if ( !m_pDevice )
  {
    dh::Log( "DX11Renderer::CreateVertexBuffer: no Device" );
    return NULL;
  }

  DX11VertexBuffer*   pBuffer = new DX11VertexBuffer( this );

  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* DX11Renderer::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat )
{
  XVertexBuffer*    pBuffer = CreateVertexBuffer( MeshObject.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( MeshObject );

  return pBuffer;
}



void DX11Renderer::DestroyVertexBuffer( XVertexBuffer* pVB )
{
  if ( pVB != NULL )
  {
    pVB->Release();
    delete pVB;

    m_VertexBuffers.remove( pVB );
  }
}



bool DX11Renderer::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  DX11VertexBuffer*   pVB = (DX11VertexBuffer*)pBuffer;

  if ( pVB == NULL )
  {
    return false;
  }
  ChooseShaders( pVB->VertexFormat() );

  FlushAllCaches();
  return pVB->Display( Index, Count );
}



void DX11Renderer::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float fZ )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }

  SetLineMode();
  m_LineCache3d.FlushCache();

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  m_LineCache.AddEntry( m_SetTextures[0],
                        GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt1.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt1.y * virtualY, fZ ), 0.0f, 0.0f,
                        GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt2.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt2.y * virtualY, fZ ), 0.0f, 0.0f,
                        Color1, Color2,
                        m_CurrentShaderType );
}



void DX11Renderer::RenderLine( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color1, GR::u32 Color2 )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }

  SetLineMode();
  m_LineCache.FlushCache();

  m_LineCache3d.AddEntry( m_SetTextures[0],
                          Pos1, 0.0f, 0.0f,
                          Pos2, 0.0f, 0.0f,
                          Color1, Color2,
                          m_CurrentShaderType );
}



void DX11Renderer::SetLineMode()
{
  if ( m_CacheMode == CM_LINE )
  {
    return;
  }
  m_CacheMode = CM_LINE;
  m_QuadCache3d.FlushCache();
  m_QuadCache.FlushCache();
  m_TriangleCache.FlushCache();
  m_TriangleCache3d.FlushCache();
}



void DX11Renderer::SetQuadMode()
{
  if ( m_CacheMode == CM_QUAD )
  {
    return;
  }
  m_CacheMode = CM_QUAD;
  m_LineCache.FlushCache();
  m_LineCache3d.FlushCache();
  m_TriangleCache.FlushCache();
  m_TriangleCache3d.FlushCache();
}



void DX11Renderer::SetTriangleMode()
{
  if ( m_CacheMode == CM_TRIANGLE )
  {
    return;
  }
  m_CacheMode = CM_TRIANGLE;
  m_LineCache.FlushCache();
  m_LineCache3d.FlushCache();
  m_QuadCache.FlushCache();
  m_QuadCache3d.FlushCache();
}



void DX11Renderer::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3, GR::u32 Color4, float fZ )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  iX += m_DisplayOffset.x;
  iY += m_DisplayOffset.y;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  SetQuadMode();
  m_QuadCache3d.FlushCache();
  m_QuadCache.AddEntry( m_SetTextures[0],
    GR::tVector( m_Canvas.Left + iX * virtualX,              m_Canvas.Top + iY * virtualY, fZ ), fTU1, fTV1,
    GR::tVector( m_Canvas.Left + ( iX + iWidth ) * virtualX, m_Canvas.Top + iY * virtualY, fZ ), fTU2, fTV2,
    GR::tVector( m_Canvas.Left + iX * virtualX,              m_Canvas.Top + ( iY + iHeight ) * virtualY, fZ ), fTU3, fTV3,
    GR::tVector( m_Canvas.Left + ( iX + iWidth ) * virtualX, m_Canvas.Top + ( iY + iHeight ) * virtualY, fZ ), fTU4, fTV4,
    Color1, Color2, Color3, Color4, m_CurrentShaderType );
}



bool DX11Renderer::RenderMesh2d( const Mesh::IMesh& Mesh )
{
  auto  pVB = CreateVertexBuffer( Mesh, XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_DIFFUSE );
  //auto  pVB = CreateVertexBuffer( Mesh, XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE );
  if ( pVB == NULL )
  {
    return false;
  }
  RenderVertexBuffer( pVB );
  DestroyVertexBuffer( pVB );
  return true;
}



void DX11Renderer::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3, GR::u32 Color4, float fZ )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }
  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  fX *= virtualX;
  fY *= virtualY;

  fX += m_DisplayOffset.x;
  fY += m_DisplayOffset.y;


  SetQuadMode();
  m_QuadCache3d.FlushCache();
  m_QuadCache.AddEntry( m_SetTextures[0],
    GR::tVector( m_Canvas.Left + fX,                      m_Canvas.Top + fY, fZ ), fTU1, fTV1,
    GR::tVector( m_Canvas.Left + fX + fWidth * virtualX,  m_Canvas.Top + fY, fZ ), fTU2, fTV2,
    GR::tVector( m_Canvas.Left + fX,                      m_Canvas.Top + fY + fHeight * virtualY, fZ ), fTU3, fTV3,
    GR::tVector( m_Canvas.Left + fX + fWidth * virtualX,  m_Canvas.Top + fY + fHeight * virtualY, fZ ), fTU4, fTV4,
    Color1, Color2, Color3, Color4, m_CurrentShaderType );
}



void DX11Renderer::RenderQuad( const GR::tVector& ptPos1,
  const GR::tVector& ptPos2,
  const GR::tVector& ptPos3,
  const GR::tVector& ptPos4,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3, GR::u32 Color4 )
{
  RenderQuad( ptPos1, ptPos2, ptPos3, ptPos4,
              fTU1, fTV1, fTU2, fTV1, fTU1, fTV2, fTU2, fTV2,
              Color1, Color2, Color3, Color4 );
}



void DX11Renderer::RenderQuad( const GR::tVector& ptPos1,
  const GR::tVector& ptPos2,
  const GR::tVector& ptPos3,
  const GR::tVector& ptPos4,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::f32 fTU4, GR::f32 fTV4,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3, GR::u32 Color4 )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }
  
  SetQuadMode();
  m_QuadCache.FlushCache();
  m_QuadCache3d.AddEntry( m_SetTextures[0],
    ptPos1, fTU1, fTV1,
    ptPos2, fTU2, fTV2,
    ptPos3, fTU3, fTV3,
    ptPos4, fTU4, fTV4,
    Color1, Color2, Color3, Color4, m_CurrentShaderType );
}



void DX11Renderer::RenderTriangle( const GR::tVector& pt1,
  const GR::tVector& pt2,
  const GR::tVector& pt3,
  GR::f32 fTU1, GR::f32 fTV1,
  GR::f32 fTU2, GR::f32 fTV2,
  GR::f32 fTU3, GR::f32 fTV3,
  GR::u32 Color1, GR::u32 Color2,
  GR::u32 Color3 )
{
  SetTriangleMode();
  FlushLightChanges();

  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  m_TriangleCache3d.AddEntry( m_SetTextures[0],
                              pt1, fTU1, fTV1,
                              pt2, fTU2, fTV2,
                              pt3, fTU3, fTV3,
                              Color1, Color2, Color3,
                              m_CurrentShaderType );
  /*
  XMesh       mesh;

  mesh.AddVertex( pt1 );
  mesh.AddVertex( pt2 );
  mesh.AddVertex( pt3 );

  Mesh::Face    face1( 0, Color1, 1, Color2, 2, Color3 );
  face1.m_TextureX[0] = fTU1;
  face1.m_TextureY[0] = fTV1;
  face1.m_TextureX[1] = fTU2;
  face1.m_TextureY[1] = fTV2;
  face1.m_TextureX[2] = fTU3;
  face1.m_TextureY[2] = fTV3;

  mesh.AddFace( face1 );
  mesh.CalculateNormals();

  RenderMesh( mesh );*/
}



void DX11Renderer::RenderTriangle2d( const GR::tPoint& pt1,
                                     const GR::tPoint& pt2,
                                     const GR::tPoint& pt3,
                                     GR::f32 fTU1, GR::f32 fTV1,
                                     GR::f32 fTU2, GR::f32 fTV2,
                                     GR::f32 fTU3, GR::f32 fTV3,
                                     GR::u32 Color1, GR::u32 Color2,
                                     GR::u32 Color3, float fZ )
{
  SetQuadMode();
  FlushLightChanges();
  m_QuadCache.FlushCache();

  if ( ( Color2 == Color3 )
  &&   ( Color3 == 0 ) )
  {
    Color2 = Color3 = Color1;
  }

  GR::f32     virtualX = (GR::f32)m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  m_TriangleCache.AddEntry( m_SetTextures[0],
                            GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt1.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt1.y * virtualY, fZ ), fTU1, fTV1,
                            GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt2.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt2.y * virtualY, fZ ), fTU2, fTV2,
                            GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt3.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt3.y * virtualY, fZ ), fTU3, fTV3,
                            Color1, Color2, Color3,
                            m_CurrentShaderType );

  /*
  XMesh       mesh;

  mesh.AddVertex( GR::tVector( ( GR::f32 )pt1.x, ( GR::f32 )pt1.y, fZ ) );
  mesh.AddVertex( GR::tVector( ( GR::f32 )pt2.x, ( GR::f32 )pt2.y, fZ ) );
  mesh.AddVertex( GR::tVector( ( GR::f32 )pt3.x, ( GR::f32 )pt3.y, fZ ) );

  Mesh::Face    face1( 0, Color1, 1, Color2, 2, Color3 );
  face1.m_TextureX[0] = fTU1;
  face1.m_TextureY[0] = fTV1;
  face1.m_TextureX[1] = fTU2;
  face1.m_TextureY[1] = fTV2;
  face1.m_TextureX[2] = fTU3;
  face1.m_TextureY[2] = fTV3;

  mesh.AddFace( face1 );

  RenderMesh2d( mesh );*/
}



bool DX11Renderer::SaveScreenShot( const char* FileName )
{
  FlushAllCaches();

  return true;
}



bool DX11Renderer::VSyncEnabled()
{
  return m_VSyncEnabled;
}



void DX11Renderer::EnableVSync( bool Enable )
{
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
  }
}



GR::Graphic::eImageFormat DX11Renderer::ImageFormat()
{
  return GR::Graphic::IF_A8R8G8B8;
}



bool DX11Renderer::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  if ( pTexture == NULL )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: Texture was NULL" );
    return false;
  }

  DX11Texture*    pDX11Texture = (DX11Texture*)pTexture;

  if ( MipMapLevel >= pDX11Texture->m_StoredImageData.size() )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, pDX11Texture->m_MipMapLevels );
    return false;
  }

  std::list<GR::Graphic::ImageData>::iterator   it( pDX11Texture->m_StoredImageData.begin() );
  std::advance( it, MipMapLevel );

  ImageData = *it;
  return true;
}



bool DX11Renderer::RenderMesh( const Mesh::IMesh& Mesh )
{
  SetQuadMode();
  FlushLightChanges();
  m_QuadCache.FlushCache();

  GR::u32 vertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL;
  if ( Mesh.FaceCount() == 2 )
  {
    // a quad, reuse stored vertex buffer
    std::map<GR::u32, XVertexBuffer*>::iterator   itMVB( m_BasicVertexBuffers.find( vertexFormat ) );
    if ( itMVB == m_BasicVertexBuffers.end() )
    {
      auto pVB = CreateVertexBuffer( Mesh, vertexFormat );

      m_BasicVertexBuffers[vertexFormat] = pVB;

      pVB->FillFromMesh( Mesh );
      RenderVertexBuffer( pVB );
      return true;
    }
    auto pVB = itMVB->second;

    pVB->FillFromMesh( Mesh );
    RenderVertexBuffer( pVB );
    return true;
  }

  // create temporary
  auto  pVB = CreateVertexBuffer( Mesh, vertexFormat );
  if ( pVB == NULL )
  {
    return false;
  }
  RenderVertexBuffer( pVB );
  DestroyVertexBuffer( pVB );
  return true;
}



void DX11Renderer::RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1,
                                       GR::f32 X2, GR::f32 Y2,
                                       GR::f32 X3, GR::f32 Y3,
                                       GR::f32 X4, GR::f32 Y4,
                                       GR::f32 TU1, GR::f32 TV1,
                                       GR::f32 TU2, GR::f32 TV2,
                                       GR::f32 TU3, GR::f32 TV3,
                                       GR::f32 TU4, GR::f32 TV4,
                                       GR::u32 Color1, GR::u32 Color2,
                                       GR::u32 Color3, GR::u32 Color4, float fZ )
{
  if ( ( Color2 == Color3 )
  &&   ( Color3 == Color4 )
  &&   ( Color4 == 0 ) )
  {
    Color2 = Color3 = Color4 = Color1;
  }

  GR::f32     virtualX = ( GR::f32 )m_Canvas.Width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.Height() / m_VirtualSize.y;

  X1 *= virtualX;
  Y1 *= virtualY;
  X2 *= virtualX;
  Y2 *= virtualY;
  X3 *= virtualX;
  Y3 *= virtualY;
  X4 *= virtualX;
  Y4 *= virtualY;

  X1 += m_DisplayOffset.x;
  Y1 += m_DisplayOffset.y;
  X2 += m_DisplayOffset.x;
  Y2 += m_DisplayOffset.y;
  X3 += m_DisplayOffset.x;
  Y3 += m_DisplayOffset.y;
  X4 += m_DisplayOffset.x;
  Y4 += m_DisplayOffset.y;

  SetQuadMode();
  m_QuadCache3d.FlushCache();
  m_QuadCache.AddEntry( m_SetTextures[0],
                        GR::tVector( m_Canvas.Left + X1, m_Canvas.Top + Y1, fZ ), TU1, TV1,
                        GR::tVector( m_Canvas.Left + X2, m_Canvas.Top + Y2, fZ ), TU2, TV2,
                        GR::tVector( m_Canvas.Left + X3, m_Canvas.Top + Y3, fZ ), TU3, TV3,
                        GR::tVector( m_Canvas.Left + X4, m_Canvas.Top + Y4, fZ ), TU4, TV4,
                        Color1, Color2, Color3, Color4,
                        m_CurrentShaderType );
}



void DX11Renderer::SetRenderTarget( XTexture* pTexture )
{
  if ( pTexture == m_pCurrentlySetRenderTargetTexture )
  {
    return;
  }
  FlushAllCaches();

  m_pCurrentlySetRenderTargetTexture = (DX11Texture*)pTexture;
  if ( m_pCurrentlySetRenderTargetTexture )
  {
    m_pDeviceContext->OMSetRenderTargets( 1, &m_pCurrentlySetRenderTargetTexture->m_pTextureRenderTargetView, NULL );
  }
  else
  {
    // back to regular back buffer
    m_pDeviceContext->OMSetRenderTargets( 1, &m_pTargetBackBuffer, m_depthStencilView );
  }
}



ID3D11Device* DX11Renderer::Device()
{
  return m_pDevice;
}



ID3D11DeviceContext* DX11Renderer::DeviceContext()
{
  return m_pDeviceContext;
}



ID3D11InputLayout* DX11Renderer::InputLayout( GR::u32 VertexFormat )
{
  std::map<GR::u32, DX11VertexShader*>::iterator    it( m_VSInputLayout.find( VertexFormat ) );
  if ( it == m_VSInputLayout.end() )
  {
    return NULL;
  }

  return it->second->m_pInputLayout;
}



void DX11Renderer::ChooseShaders( GR::u32 VertexFormat )
{
  eShaderType   shaderTypeToUse = m_CurrentShaderType;

  if ( shaderTypeToUse == ST_ALPHA_BLEND )
  {
    m_pDeviceContext->OMSetBlendState( m_pBlendStateAlphaBlend, NULL, 0xffffffff );
    shaderTypeToUse = ST_FLAT;
  }
  else if ( shaderTypeToUse == ST_ALPHA_BLEND_AND_TEST )
  {
    m_pDeviceContext->OMSetBlendState( m_pBlendStateAlphaBlend, NULL, 0xffffffff );
    shaderTypeToUse = ST_ALPHA_TEST;
  }
  else if ( shaderTypeToUse == ST_ADDITIVE )
  {
    m_pDeviceContext->OMSetBlendState( m_pBlendStateAdditive, NULL, 0xffffffff );
  }
  else
  {
    m_pDeviceContext->OMSetBlendState( NULL, NULL, 0xffffffff );
  }

  // fall back to non-texture mode?
  if ( m_SetTextures[0] == NULL )
  {
    if ( shaderTypeToUse == ST_FLAT )
    {
      shaderTypeToUse = ST_FLAT_NO_TEXTURE;
    }
  }

  int     numLights = m_NumActiveLights;
  bool    lightDisabled = ( m_RenderStates[std::make_pair( XRenderer::RS_LIGHTING, 0 )] == XRenderer::RSV_DISABLE );

  switch ( shaderTypeToUse )
  {
    case ST_FLAT:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetVertexShader( "position_texture_color" );
        SetPixelShader( "position_texture_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetVertexShader( "positionrhw_texture_color" );
        SetPixelShader( "position_texture_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        SetVertexShader( "positionrhw_texture_color_normal_nolight" );
        SetPixelShader( "position_texture_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "position_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "positionrhw_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_texture_color_normal_lightdisabled" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_texture_color_normal_light1" );
          //SetPixelShader( "position_texture_color_normal_light1" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_texture_color_normal_light2" );
          SetPixelShader( "position_texture_color" );
          return;
        }
      }
      break;
    case ST_FLAT_NO_TEXTURE:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "position_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetVertexShader( "position_notexture_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_notexture_color_normal_lightdisabled" );
          //SetPixelShader( "position_texture_color" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_notexture_color_normal_nolight" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_notexture_color_normal_light1" );
          SetPixelShader( "position_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_notexture_color_normal_light2" );
          SetPixelShader( "position_color" );
          return;
        }
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
      {
        SetVertexShader( "positionrhw_color" );
        SetPixelShader( "position_color" );
        return;
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
      {
        SetVertexShader( "positionrhw_notexture_color" );
        SetPixelShader( "position_color" );
        return;
      }
      break;
    case ST_ALPHA_TEST:
      if ( m_SetTextures[0] == NULL )
      {
        // no texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no light with XYZRHW!
          SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_notexture_color_normal_lightdisabled" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_notexture_color_normal_nolight" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_notexture_color_normal_light1" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetVertexShader( "position_notexture_color_normal_light2" );
            SetPixelShader( "position_color_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }
      else
      {
        // has texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_texture_color" );
          SetPixelShader( "position_texture_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetVertexShader( "positionrhw_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_texture_color_normal_lightdisabled" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_texture_color_normal_nolight" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_texture_color_normal_light1" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetVertexShader( "position_texture_color_normal_light2" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_texture_color" );
          SetPixelShader( "position_texture_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }
      break;
    case ST_ADDITIVE:
      if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        if ( lightDisabled )
        {
          SetVertexShader( "position_texture_color_normal_lightdisabled" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 0 )
        {
          SetVertexShader( "position_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 1 )
        {
          SetVertexShader( "position_texture_color_normal_light1" );
          SetPixelShader( "position_texture_color" );
          return;
        }
        else if ( numLights == 2 )
        {
          SetVertexShader( "position_texture_color_normal_light2" );
          SetPixelShader( "position_texture_color" );
          return;
        }
      }
      else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
      {
        // no lights with xyzrhw
        SetVertexShader( "positionrhw_texture_color_normal_nolight" );
        SetPixelShader( "position_texture_color" );
        return;
      }
      break;
    case ST_ALPHA_TEST_COLOR_FROM_DIFFUSE:
      if ( m_SetTextures[0] == NULL )
      {
        // no texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_notexture_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetVertexShader( "positionrhw_notexture_color_normal_nolight" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_texture_color_normal_lightdisabled" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_texture_color_normal_nolight" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_texture_color_normal_light1" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
          else if ( numLights == 2 )
          {
            SetVertexShader( "position_texture_color_normal_light2" );
            SetPixelShader( "position_texture_color_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }
      else
      {
        // has texture set
        if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "position_texture_color" );
          SetPixelShader( "position_texture_colordiffuse_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          // no lights with xyzrhw
          SetVertexShader( "positionrhw_texture_color_normal_nolight" );
          SetPixelShader( "position_texture_colordiffuse_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL ) )
        {
          if ( lightDisabled )
          {
            SetVertexShader( "position_texture_color_normal_lightdisabled" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
          else if ( numLights == 0 )
          {
            SetVertexShader( "position_texture_color_normal_nolight" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
          else if ( numLights == 1 )
          {
            SetVertexShader( "position_texture_color_normal_light1" );
            SetPixelShader( "position_texture_colordiffuse_alphatest" );
            return;
          }
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD ) )
        {
          SetVertexShader( "positionrhw_texture_color" );
          SetPixelShader( "position_texture_colordiffuse_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "position_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
        else if ( VertexFormat == ( XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE ) )
        {
          SetVertexShader( "positionrhw_color" );
          SetPixelShader( "position_color_alphatest" );
          return;
        }
      }
      break;
  }
  dh::Log( "ChooseShaders unsupported shader for shader type %d, vertex format 0x%x, really active %d lights, texture set %x", shaderTypeToUse, VertexFormat, numLights, m_SetTextures[0] );
}



void DX11Renderer::RenderBox( const GR::tVector& vPos, const GR::tVector& vSize, GR::u32 Color )
{
  SetQuadMode();
  m_QuadCache.FlushCache();

  XMesh     meshBox;

  meshBox.AddVertex( vPos );
  meshBox.AddVertex( vPos + GR::tVector( vSize.x, 0.0f, 0.0f ) );
  meshBox.AddVertex( vPos + GR::tVector( 0.0f, vSize.y, 0.0f ) );
  meshBox.AddVertex( vPos + GR::tVector( vSize.x, vSize.y, 0.0f ) );
  meshBox.AddVertex( vPos + GR::tVector( 0.0f, 0.0f, vSize.z ) );
  meshBox.AddVertex( vPos + GR::tVector( vSize.x, 0.0f, vSize.z ) );
  meshBox.AddVertex( vPos + GR::tVector( 0.0f, vSize.y, vSize.z ) );
  meshBox.AddVertex( vPos + GR::tVector( vSize.x, vSize.y, vSize.z ) );

  Mesh::Face      faceFront1( 2, Color, 6, Color, 3, Color );
  Mesh::Face      faceFront2( 6, Color, 7, Color, 3, Color );

  Mesh::Face      faceLeft1( 0, Color, 4, Color, 2, Color );
  Mesh::Face      faceLeft2( 4, Color, 6, Color, 2, Color );

  Mesh::Face      faceRight1( 3, Color, 7, Color, 1, Color );
  Mesh::Face      faceRight2( 7, Color, 5, Color, 1, Color );

  Mesh::Face      faceBack1( 1, Color, 5, Color, 0, Color );
  Mesh::Face      faceBack2( 5, Color, 4, Color, 0, Color );

  Mesh::Face      faceTop1( 0, Color, 2, Color, 1, Color );
  Mesh::Face      faceTop2( 2, Color, 3, Color, 1, Color );

  Mesh::Face      faceBottom1( 6, Color, 4, Color, 7, Color );
  Mesh::Face      faceBottom2( 4, Color, 5, Color, 7, Color );

  meshBox.AddFace( faceFront1 );
  meshBox.AddFace( faceFront2 );
  meshBox.AddFace( faceLeft1 );
  meshBox.AddFace( faceLeft2 );
  meshBox.AddFace( faceRight1 );
  meshBox.AddFace( faceRight2 );
  meshBox.AddFace( faceBack1 );
  meshBox.AddFace( faceBack2 );
  meshBox.AddFace( faceTop1 );
  meshBox.AddFace( faceTop2 );
  meshBox.AddFace( faceBottom1 );
  meshBox.AddFace( faceBottom2 );

  for ( int i = 0; i < 12; ++i )
  {
    if ( i % 2 )
    {
      meshBox.m_Faces[i].m_TextureX[0] = 0.0f;
      meshBox.m_Faces[i].m_TextureY[0] = 1.0f;
      meshBox.m_Faces[i].m_TextureX[1] = 1.0f;
      meshBox.m_Faces[i].m_TextureY[1] = 0.0f;
      meshBox.m_Faces[i].m_TextureX[2] = 1.0f;
      meshBox.m_Faces[i].m_TextureY[2] = 1.0f;
    }
    else
    {
      meshBox.m_Faces[i].m_TextureX[0] = 0.0f;
      meshBox.m_Faces[i].m_TextureY[0] = 0.0f;
      meshBox.m_Faces[i].m_TextureX[1] = 1.0f;
      meshBox.m_Faces[i].m_TextureY[1] = 0.0f;
      meshBox.m_Faces[i].m_TextureX[2] = 0.0f;
      meshBox.m_Faces[i].m_TextureY[2] = 1.0f;
    }
  }

  meshBox.CalculateNormals();

  RenderMesh( meshBox );
}



void DX11Renderer::SetShader( eShaderType sType )
{
  m_CurrentShaderType = sType;
  if ( sType == ST_FLAT_NO_TEXTURE )
  {
    SetTexture( 0, NULL );
    SetTexture( 1, NULL );
  }
}



XFont* DX11Renderer::LoadFontSquare( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  XFont*   pFont = new DX11Font( this, m_pEnvironment );

  pFont->LoadFontSquare( FileName, Flags, TransparentColor );

  AddFont( pFont );

  return pFont;
}



XFont* DX11Renderer::LoadFont( const char* FileName, GR::u32 TransparentColor )
{
  XFont*    pFont = new DX11Font( this, m_pEnvironment );

  if ( !pFont->LoadFont( FileName, 0, TransparentColor ) )
  {
    delete pFont;
    return NULL;
  }
  AddFont( pFont );
  return pFont;
}



int DX11Renderer::MaxPrimitiveCount()
{
  switch ( m_FeatureLevel )
  {
    default:
    case D3D_FEATURE_LEVEL_9_1:
      return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_3:
    case D3D_FEATURE_LEVEL_10_0:
    case D3D_FEATURE_LEVEL_10_1:
    case D3D_FEATURE_LEVEL_11_0:
    case D3D_FEATURE_LEVEL_11_1:
      return D3D_FL9_2_IA_PRIMITIVE_MAX_COUNT;
  }
  return D3D_FL9_1_IA_PRIMITIVE_MAX_COUNT;
}


