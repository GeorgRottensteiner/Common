#include "VulkanFont.h"
#include "VulkanRenderer.h"
#include "VulkanPixelShader.h"
#include "VulkanTexture.h"
#include "VulkanVertexShader.h"
#include "VulkanVertexBuffer.h"

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Xtreme/Environment/XWindow.h>

#include <IO/FileUtil.h>

#include <String/StringUtil.h>

#include <WinSys/SubclassManager.h>

#include <set>

#pragma comment( lib, "vulkan-1.lib" )



VulkanRenderer::VulkanRenderer( HINSTANCE hInstance ) :
  m_Width( 0 ),
  m_Height( 0 ),
  m_VSyncEnabled( false ), 
  m_pTempBuffer( NULL ),
  m_CurrentShaderType( ST_INVALID ),
  m_NumActiveLights( 0 ),
  m_LightingEnabled( false ),
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
  m_Ready( false ),
  m_VulkanInstance( VK_NULL_HANDLE ),
  m_VulkanDevice( VK_NULL_HANDLE ),
  m_PhysicalDevice( VK_NULL_HANDLE )
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



VulkanRenderer::~VulkanRenderer()
{ 
  Release();
}



bool VulkanRenderer::AddBasicVertexShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat )
{ 
  GR::String     basePath = "D:/projekte/Xtreme/VulkanRenderer/shaders/";
  //GR::String     basePath = "D:/privat/projekte/Xtreme/VulkanRenderer/shaders/";

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



bool VulkanRenderer::AddBasicPixelShaderFromFile( const GR::String& Filename, const GR::String& Desc, GR::u32 VertexFormat )
{
  GR::String     basePath = "D:/projekte/Xtreme/VulkanRenderer/shaders/";
  //GR::String     basePath = "D:/privat/projekte/Xtreme/VulkanRenderer/shaders/";

  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( ( basePath + Filename ).c_str() );

  if ( shaderData.Empty() )
  {
    return false;
  }

  return AddBasicPixelShader( Desc, shaderData.ToHexString(), VertexFormat );
}



bool VulkanRenderer::InitialiseBasicShaders()
{
  /*
  std::list<GR::String>    precompiledShaders;

  GR::IO::FileUtil::EnumFilesInDirectory( "D:/projekte/Xtreme/VulkanRenderer/shaders/*.cso", precompiledShaders );

  auto  it( precompiledShaders.begin() );

  while ( it != precompiledShaders.end() )
  {
    const GR::String& fileName( *it );

    if ( GR::Strings::StartsWith( fileName, "VS_" ) )
    {
      ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( fileName.c_str() );
      if ( !shaderData.Empty() )
      {
        if ( !AddBasicVertexShader( Path::FileNameWithoutExtension( fileName.substr( 3 ) ), shaderData.ToHexString(), 
      }
    }

    ++it;
  }*/

  bool            hadError = false;

  /*
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_color.cso", "positionrhw_color", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_color.cso", "position_color", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_texture_color.cso", "position_texture_color", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_notexture_color.cso", "position_notexture_color", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_texture_color.cso", "positionrhw_texture_color", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_notexture_color.cso", "positionrhw_notexture_color", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_texture_color_normal_nolight.cso", "position_texture_color_normal_nolight", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_notexture_color_normal_nolight.cso", "position_notexture_color_normal_nolight", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_texture_color_normal_light1.cso", "position_texture_color_normal_light1", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_notexture_color_normal_light1.cso", "position_notexture_color_normal_light1", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_position_notexture_color_normal_light2.cso", "position_notexture_color_normal_light2", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_texture_color_normal_nolight.cso", "positionrhw_texture_color_normal_nolight", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_notexture_color_normal_nolight.cso", "positionrhw_notexture_color_normal_nolight", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_notexture_color_normal_light1.cso", "positionrhw_notexture_color_normal_light1", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_notexture_color_normal_light2.cso", "positionrhw_notexture_color_normal_light2", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_texture_color_normal_light1.cso", "positionrhw_texture_color_normal_light1", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicVertexShaderFromFile( "VS_positionrhw_texture_color_normal_light2.cso", "positionrhw_texture_color_normal_light2", XVertexBuffer::VFF_XYZRHW | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD | XVertexBuffer::VFF_NORMAL );

  hadError |= !AddBasicPixelShaderFromFile( "PS_position_color.cso", "position_color", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE );
  hadError |= !AddBasicPixelShaderFromFile( "PS_position_texture_color.cso", "position_texture_color", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  hadError |= !AddBasicPixelShaderFromFile( "PS_position_color_normal_light1.cso", "position_color_normal_light1", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL );
  hadError |= !AddBasicPixelShaderFromFile( "PS_position_color_alphatest.cso", "position_color_alphatest", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE );
  hadError |= !AddBasicPixelShaderFromFile( "PS_position_texture_color_alphatest.cso", "position_texture_color_alphatest", XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD );
  */

//#include "ShaderBinary.inl"

  return !hadError;

  /*
  if ( ( !AddBasicVertexShader( "positionrhw_color", 
    "xx"
    "xx"
    "xx"
    "xx"
    "xx"
    , XBasicVertexBuffer::VFF_XYZ | XBasicVertexBuffer::VFF_DIFFUSE ) )
  ||   ( !AddBasicVertexShader( "position_color",  

  "xx"
  "xx"
  "xx"
  "xx"
  "xx"
  "xx"

  , XBasicVertexBuffer::VFF_XYZ | XBasicVertexBuffer::VFF_DIFFUSE ) )
  ||   ( !AddBasicPixelShader( "position_color", 
  "xx"
  "xx"
  "xx"
  "xx"
, XBasicVertexBuffer::VFF_XYZ | XBasicVertexBuffer::VFF_DIFFUSE ) )

|| ( !AddBasicVertexShader( "position_texture_color",
"xx"
"xx"
"xx"
"xx"
"xx"
"xx"
, XBasicVertexBuffer::VFF_XYZ | XBasicVertexBuffer::VFF_TEXTURECOORD | XBasicVertexBuffer::VFF_DIFFUSE ) )
|| ( !AddBasicPixelShader( "position_texture_color",
"xx"
"xx"
"xx"
"xx"
"xx"
, XBasicVertexBuffer::VFF_XYZ | XBasicVertexBuffer::VFF_TEXTURECOORD | XBasicVertexBuffer::VFF_DIFFUSE ) )

)
  {
    return false;
  }*/
  return true;
}



VulkanVertexShader* VulkanRenderer::AddBasicVertexShader( const GR::String& Desc, const GR::String& HexVS, GR::u32 VertexFormat )
{
  //ByteBuffer      vsFlat( "4458424380C7F76D99C5F2910E1A16C276134F5E01000000CC0200000600000038000000D000000054010000D00100002802000078020000416F6E3990000000900000000002FEFF68000000280000000000240000002400000024000000240001002400000000000002FEFF5100000501000FA00000803F0000000000000000000000001F0000020500008000000F901F0000020500018001000F9002000003000003C00000E4900000E4A00400000400000CC00000AA90010044A0010014A001000002000007E00100E490FFFF0000534844527C000000400001001F0000005F00000372101000000000005F000003721010000100000067000004F220100000000000010000006500000372201000010000003600000572201000000000004612100000000000360000058220100000000000014000000000803F36000005722010000100000046121000010000003E0000015354415474000000040000000000000000000000040000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000000000000000000052444546500000000000000000000000000000001C0000000004FEFF000100001C0000004D6963726F736F66742028522920484C534C2053686164657220436F6D70696C657220392E33302E393230302E323037383900AB4953474E480000000200000008000000380000000000000000000000030000000000000007070000410000000000000000000000030000000100000007070000504F534954494F4E00434F4C4F5200AB4F53474E4C000000020000000800000038000000000000000100000003000000000000000F00000044000000000000000000000003000000010000000708000053565F504F534954494F4E00434F4C4F5200ABAB" );
  ByteBuffer      vsFlat( HexVS );

  auto  shader = new VulkanVertexShader( this, VertexFormat );
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



bool VulkanRenderer::AddBasicPixelShader( const GR::String& Desc, const GR::String& HexPS, GR::u32 VertexFormat )
{
  ByteBuffer      psFlat( HexPS );

  auto  shader = new VulkanPixelShader( this );
  if ( !shader->CreateFromBuffers( psFlat ) )
  {
    delete shader;
    return false;
  }
  m_BasicPixelShaders[Desc] = shader;
  m_PixelShaders.push_back( shader );
  return true;
}



bool VulkanRenderer::Failed( const GR::String& Function, VkResult ResultCode )
{
  if ( ResultCode != VK_SUCCESS )
  {
    dh::Log( "%s failed with VkResult %d", Function.c_str(), ResultCode );
    return true;
  }
  return false;
}



bool VulkanRenderer::Initialize( GR::u32 Width,
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
  m_DisplayOffset.clear();

  uint32_t extensionCount = 0;
  if ( Failed( "vkEnumerateInstanceExtensionProperties", vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr ) ) )
  {
    Release();
    return false;
  }

  VkApplicationInfo appInfo = {};

  appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext              = NULL;
  appInfo.pApplicationName   = "Xtreme Renderer";
  appInfo.applicationVersion = 1;
  appInfo.pEngineName        = "Xtreme";
  appInfo.engineVersion      = 1;
  appInfo.apiVersion         = VK_API_VERSION_1_0;


  VkInstanceCreateInfo instanceInfo = {};
  instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceInfo.pNext                   = NULL;
  instanceInfo.flags                   = 0;
  instanceInfo.pApplicationInfo        = &appInfo;
  instanceInfo.enabledExtensionCount   = 0;
  instanceInfo.ppEnabledExtensionNames = NULL;
  instanceInfo.enabledLayerCount       = 0;
  instanceInfo.ppEnabledLayerNames     = NULL;


  if ( Failed( "vkCreateInstance", vkCreateInstance( &instanceInfo, NULL, &m_VulkanInstance ) ) )
  {
    Release();
    return false;
  }

  uint32_t gpuCount = 1;

  if ( Failed( "vkEnumeratePhysicalDevices", vkEnumeratePhysicalDevices( m_VulkanInstance, &gpuCount, NULL ) ) )
  {
    Release();
    return false;
  }
  m_GPUs.resize( gpuCount );
  if ( Failed( "vkEnumeratePhysicalDevices", vkEnumeratePhysicalDevices( m_VulkanInstance, &gpuCount, m_GPUs.data() ) ) )
  {
    Release();
    return false;
  }

  if ( !pickPhysicalDevice() )
  {
    Release();
    return false;
  }

  pickPhysicalDevice();
  createLogicalDevice();
  createImageViews();
  CreateSwapChain();
  createImageViews();

  VkDeviceQueueCreateInfo queue_info = {};
  uint32_t    queue_family_count = 0;

  vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queue_family_count, NULL );
  m_QueueProperties.resize( queue_family_count );

  vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queue_family_count, m_QueueProperties.data() );

  bool found = false;
  for ( unsigned int i = 0; i < queue_family_count; i++ )
  {
    if ( m_QueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
    {
      queue_info.queueFamilyIndex = i;
      found = true;
      break;
    }
  }

  float queue_priorities[1] = { 0.0 };
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;

  /*
  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = 0;
  device_info.ppEnabledExtensionNames = NULL;
  device_info.enabledLayerCount = 0;
  device_info.ppEnabledLayerNames = NULL;
  device_info.pEnabledFeatures = NULL;

  if ( Failed( "vkCreateDevice", vkCreateDevice( m_PhysicalDevice, &device_info, NULL, &m_VulkanDevice ) ) )
  {
    Release();
    return false;
  }*/

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  RECT      windowRect;
  GetWindowRect( m_hwndViewport, &windowRect );
  m_WindowedPlacement.set( windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );
#endif

  if ( !InitialiseBasicShaders() )
  {
    Release();
    return false;
  }

  m_Width = Width;
  m_Height = Height;

  m_Canvas.set( 0, 0, m_Width, m_Height );

  SetVertexShader( "position_color" );
  SetPixelShader( "position_color" );

  m_VirtualSize.set( m_Width, m_Height );

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

  m_LightInfo.Ambient = ColorValue( 0x00000000 );

  SetTransform( XRenderer::TT_WORLD, m_Matrices.Model );

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  ISubclassManager* pManager = (ISubclassManager*)m_pEnvironment->Service( "SubclassManager" );
  if ( pManager )
  {
    pManager->AddHandler( "VulkanRenderer", fastdelegate::MakeDelegate( this, &VulkanRenderer::WindowProc ) );
  }
#endif

  for ( int i = 0; i < 8; ++i )
  {
    m_SetTextures[i] = NULL;
  }

  RecreateBuffers();

  // init material
  m_Fog.FogType     = 0; // FOG_TYPE_NONE;
  m_Fog.FogColor    = ColorValue( 0x00000000 );
  m_Fog.FogStart    = 10.0f;
  m_Fog.FogEnd      = 25.0f;
  m_Fog.FogDensity  = 0.02f;

  // assets
  if ( m_pEnvironment )
  {
    Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu laden
      GR::up    imageCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE );
      for ( GR::up i = 0; i < imageCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );

        GR::u32         ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );
        GR::u32         ColorKeyReplacementColor = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKeyReplacementColor" ), 16 );

        GR::u32         ForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        GR::String  fileName = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" );

        std::list<GR::String>    listImages;

        GR::u32     MipMaps = 0;

#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
#endif
        while ( true )
        {
          if ( MipMaps == 0 )
          {
            listImages.push_back( AppPath( fileName ) );
          }
          else
          {
            GR::String   strPath = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", MipMaps ) );

            if ( strPath.empty() )
            {
              break;
            }
            //TODO - NOT APP PATH!
            listImages.push_back( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", MipMaps ) ) ) );
          }
          MipMaps++;
        }

        if ( listImages.empty() )
        {
          Log( "Renderer.General", "VulkanRenderer: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front().c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey, MipMaps, ColorKeyReplacementColor );

          if ( pTexture )
          {
            if ( pAsset == NULL )
            {
              //Log( "Renderer.General", CMisc::printf( "VulkanRenderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
              pAsset = new Xtreme::Asset::XAssetImage( pTexture );
              pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pAsset );
            }
            else
            {
              ( ( Xtreme::Asset::XAssetImage* )pAsset )->m_pTexture = pTexture;
            }
            //delete pAsset;

            //pAsset = new Xtreme::Asset::XAssetImage( pTexture );
            //pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pAsset );

            // MipMaps laden
            std::list<GR::String>::iterator    it( listImages.begin() );
            it++;
            GR::u32   Level = 1;

            while ( it != listImages.end() )
            {
              GR::String&  strPath( *it );

              GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey, ColorKeyReplacementColor );
              if ( pData == NULL )
              {
                Log( "Renderer.General", CMisc::printf( "VulkanRenderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, ColorKey, Level );
                delete pData;
              }

              ( (VulkanTexture*)pTexture )->m_listFileNames.push_back( strPath );

              ++Level;
              ++it;
            }
          }
          else
          {
            Log( "Renderer.General", CMisc::printf( "VulkanRenderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
          }
        }
      }

      GR::up    imageSectionCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < imageSectionCount; ++i )
      {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
#endif

        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = ( Xtreme::Asset::XAssetImage* )pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          Log( "Renderer.General", CMisc::printf( "VulkanRenderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
        }
        else
        {
          XTextureSection   tsSection;

          tsSection.m_XOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "X" ) );
          tsSection.m_YOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Y" ) );
          tsSection.m_Width = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "W" ) );
          tsSection.m_Height = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "H" ) );
          tsSection.m_Flags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Flags" ) );

          tsSection.m_pTexture = pTexture->Texture();

          if ( pAsset == NULL )
          {
            pAsset = new Xtreme::Asset::XAssetImageSection( tsSection );
          }
          else
          {
            ( ( Xtreme::Asset::XAssetImageSection* )pAsset )->m_tsImage = tsSection;
          }
          pLoader->SetAsset( Xtreme::Asset::XA_IMAGE_SECTION, i, pAsset );
        }
      }

      GR::up    fontCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_FONT );
      for ( GR::up i = 0; i < fontCount; ++i )
      {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
#endif

        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_FONT, i );

        GR::u32     ColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     LoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        XFont*      pFont = NULL;

        if ( ( LoadFlags & XFont::FLF_SQUARED )
          || ( LoadFlags & XFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
            LoadFlags,
            ColorKey );
        }
        else
        {
          pFont = LoadFont( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), ColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "VulkanRenderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          Log( "Renderer.General", "VulkanRenderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }

      GR::up    meshCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_MESH );
      for ( GR::up i = 0; i < meshCount; ++i )
      {
#if ( OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP ) || ( OPERATING_SUB_SYSTEM == OS_SUB_WINDOWS_PHONE )
        Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents( Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent );
#endif

        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_MESH, i );

        XMesh*       pMesh = NULL;

        pMesh = CT3DMeshLoader::Load( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "File" ) ).c_str() );
        if ( pMesh )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "VulkanRenderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
          }
          delete pAsset;

          Xtreme::Asset::XAssetMesh*    pAssetMesh = new Xtreme::Asset::XAssetMesh( *pMesh );

          pAssetMesh->m_Mesh.CalculateBoundingBox();

          pAssetMesh->m_pVertexBuffer = CreateVertexBuffer( pAssetMesh->m_Mesh );

          pAsset = pAssetMesh;
          pLoader->SetAsset( Xtreme::Asset::XA_MESH, i, pAsset );

          delete pMesh;
        }
        else
        {
          Log( "Renderer.General", "VulkanRenderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
        }
      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }

  m_RenderStates[std::make_pair( RS_CULLMODE, 0 )] = RSV_CULL_CCW;
  m_RenderStates[std::make_pair( RS_FILL_MODE, 0 )] = RSV_FILL_SOLID;

  m_Ready = true;
  return true;
}



bool VulkanRenderer::isDeviceSuitable( VkPhysicalDevice device )
{
  QueueFamilyIndices indices = FindQueueFamilies( device );

  bool extensionsSupported = checkDeviceExtensionSupport( device );

  bool swapChainAdequate = false;
  if ( extensionsSupported )
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport( device );
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}



bool VulkanRenderer::checkDeviceExtensionSupport( VkPhysicalDevice device )
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

  std::vector<VkExtensionProperties> availableExtensions( extensionCount );
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

  std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

  for ( const auto& extension : availableExtensions )
  {
    requiredExtensions.erase( extension.extensionName );
  }

  return requiredExtensions.empty();
}



void VulkanRenderer::createImageViews()
{
  m_SwapChainImageViews.resize( m_SwapChainImages.size() );

  for ( size_t i = 0; i < m_SwapChainImages.size(); i++ )
  {
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = m_SwapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = m_SwapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if ( vkCreateImageView( m_VulkanDevice, &createInfo, nullptr, &m_SwapChainImageViews[i] ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create image views!" );
    }
  }
}



bool VulkanRenderer::pickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices( m_VulkanInstance, &deviceCount, nullptr );

  if ( deviceCount == 0 )
  {
    Log( "Error", "vkEnumeratePhysicalDevices found no devices with Vulkan support" );
    return false;
  }

  std::vector<VkPhysicalDevice> devices( deviceCount );
  vkEnumeratePhysicalDevices( m_VulkanInstance, &deviceCount, devices.data() );

  for ( const auto& device : devices )
  {
    if ( isDeviceSuitable( device ) )
    {
      m_PhysicalDevice = device;
      break;
    }
  }

  if ( m_PhysicalDevice == VK_NULL_HANDLE )
  {
    Log( "Error", "Failed to find suitable GPU" );
    return false;
  }
  return true;
}



#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
BOOL VulkanRenderer::WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
        //dh::Log( "I'm being deactivated" );
        if ( ( !m_TogglingFullScreen )
        &&   ( IsFullscreen() ) )
        {
          // switch off fullscreen
          ToggleFullscreen();
        }
      }
      else
      {
        //dh::Log( "I'm being activated" );
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



void VulkanRenderer::SetVertexShader( const GR::String& Desc )
{
  if ( m_CurrentVertexShader == Desc )
  {
    return;
  }

  std::map<GR::String, VulkanVertexShader*>::iterator    itVS( m_BasicVertexShaders.find( Desc ) );
  if ( itVS == m_BasicVertexShaders.end() )
  {
    dh::Log( "tried to set Unknown vertex shader %s", Desc.c_str() );
    return;
  }
  m_CurrentVertexShader = Desc;

  //dh::Log( "Set vertex shader %s", Desc.c_str() );
}



void VulkanRenderer::SetPixelShader( const GR::String& Desc )
{
  if ( m_CurrentPixelShader == Desc )
  {
    return;
  }

  std::map<GR::String, VulkanPixelShader*>::iterator    itPS( m_BasicPixelShaders.find( Desc ) );
  if ( itPS == m_BasicPixelShaders.end() )
  {
    dh::Log( "tried to set Unknown pixel shader %s", Desc.c_str() );
    return;
  }
  m_CurrentPixelShader = Desc;

  //dh::Log( "Set pixel shader %s", Desc.c_str() );
}



void VulkanRenderer::SetTransform( eTransformType tType, const math::matrix4& matTrix )
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
      dh::Log( "VulkanRenderer::SetTransform unsupported transform type %d", tType );
      break;
  }

  m_QuadCache.TransformChanged( tType );
  m_QuadCache3d.TransformChanged( tType );
  m_LineCache.TransformChanged( tType );
  m_LineCache3d.TransformChanged( tType );
  m_TriangleCache.TransformChanged( tType );
  m_TriangleCache3d.TransformChanged( tType );


  // Prepare the constant buffer to send it to the graphics device.
  // camera pos is required for lights
  if ( m_NumActiveLights > 0 )
  {
    if ( tType == TT_WORLD )
    {
      m_LightInfo.EyePos.set( matTrix.ms._41, matTrix.ms._42, matTrix.ms._43 );
      m_LightsChanged = false;
    }
  }
}



void VulkanRenderer::FlushLightChanges()
{
  if ( m_LightsChanged )
  {
    m_LightsChanged = false;
  }
}



void VulkanRenderer::FlushAllCaches()
{
  FlushLightChanges();
  m_QuadCache3d.FlushCache();
  m_QuadCache.FlushCache();
  m_LineCache.FlushCache();
  m_LineCache3d.FlushCache();
  m_TriangleCache.FlushCache();
  m_TriangleCache3d.FlushCache();
}



bool VulkanRenderer::Release()
{
  if ( !m_Ready )
  {
    return true;
  }

  for ( auto imageView : m_SwapChainImageViews )
  {
    vkDestroyImageView( m_VulkanDevice, imageView, nullptr );
  }
  vkDestroySwapchainKHR( m_VulkanDevice, m_SwapChain, nullptr );

  vkDestroyCommandPool( m_VulkanDevice, m_CommandPool, nullptr );

  if ( m_VulkanDevice != VK_NULL_HANDLE )
  {
    vkDestroyDevice( m_VulkanDevice, NULL );
    m_VulkanDevice = VK_NULL_HANDLE;
  }

  if ( m_Surface != VK_NULL_HANDLE )
  {
    vkDestroySurfaceKHR( m_VulkanInstance, m_Surface, nullptr );
    m_Surface = VK_NULL_HANDLE;
  }
  if ( m_VulkanInstance != VK_NULL_HANDLE )
  {
    vkDestroyInstance( m_VulkanInstance, NULL );
    m_VulkanInstance = VK_NULL_HANDLE;
  }

  //OutputDebugStringA( "VulkanRenderer::Release called\n" );

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
      pManager->RemoveHandler( "VulkanRenderer" );
    }
  }
  #endif

  // unbind all bound objects

#if OPERATING_SUB_SYSTEM == OS_SUB_UNIVERSAL_APP
  if ( m_pSwapChain != NULL )
  {
    //m_pSwapChain->SetFullscreenState( FALSE, NULL );
  }
#endif

  DestroyAllTextures();
  DestroyAllVertexBuffers();
  m_pCurrentlySetRenderTargetTexture = NULL;
  m_BasicVertexBuffers.clear();
  std::list<VulkanVertexShader*>::iterator    itVS( m_VertexShaders.begin() );
  while ( itVS != m_VertexShaders.end() )
  {
    VulkanVertexShader*   pShader = *itVS;

    pShader->Release();

    ++itVS;
  }
  std::list<VulkanPixelShader*>::iterator    itPS( m_PixelShaders.begin() );
  while ( itPS != m_PixelShaders.end() )
  {
    VulkanPixelShader*   pShader = *itPS;

    pShader->Release();

    ++itPS;
  }
  m_VertexShaders.clear();
  m_PixelShaders.clear();
  m_BasicPixelShaders.clear();
  m_BasicVertexShaders.clear();

  /*
  // display alive objects
  ID3D11Debug*    pDebug = NULL;
  m_pDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &pDebug ) );
  pDebug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
  pDebug->Release();
  */

  m_hwndViewport = NULL;
  m_pEnvironment = NULL;
  m_Ready = false;
  return true;
}



bool VulkanRenderer::OnResized()
{
  //dh::Log( "OnResized" );
  ReleaseBuffers();

  //dh::Log( "OnResized OK" );
  return true;
}



bool VulkanRenderer::IsReady() const
{
  return m_Ready;
}



bool VulkanRenderer::BeginScene()
{
  BOOL    fullScreen = FALSE;

  // viewport has to be set every frame
  SetViewport( m_ViewPort );

  return true;
}



void VulkanRenderer::EndScene()
{
  FlushAllCaches();
}



void VulkanRenderer::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  FlushAllCaches();

  HRESULT     hr = S_OK;

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
  /*
  if ( ( hr == DXGI_ERROR_DEVICE_REMOVED )
  ||   ( hr == DXGI_ERROR_DEVICE_RESET ) )
  {
    dh::Log( "TODO - device lost" );
    //HandleDeviceLost();
  }
  else if ( FAILED( hr ) )
  {
    dh::Log( "Present had an error!" );
  }*/
}



bool VulkanRenderer::ToggleFullscreen()
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

  m_TogglingToFullScreen = !fullScreen;

  // temporarely set m_Windowed to target
  m_Windowed = !fullScreen;


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
    RECT      windowRect;
    GetWindowRect( m_hwndViewport, &windowRect );

    m_WindowedPlacement.set( windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );

    //dh::Log( "Store m_WindowedPlacement as %d,%d %dx%d", windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );
#endif
  }
  //dh::Log( "SwapChain says fullscreen state is %d", fullScreen );
  //OutputDebugStringA( CMisc::printf( "SwapChain says fullscreen state is %d, calling ReleaseBuffers\n", fullScreen ) );

  //dh::Log( "calling ReleaseBuffers in ToggleFullscreen" );
  //ReleaseBuffers();

  m_Windowed = !fullScreen;
  //dh::Log( "SwapChain says fullscreen state is now %d", fullScreen );

  if ( !fullScreen )
  {
    // restore window placement
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    //dh::Log( "restore m_WindowedPlacement as %d,%d %dx%d", m_WindowedPlacement.Left, m_WindowedPlacement.Top, m_WindowedPlacement.width(), m_WindowedPlacement.height() );
    SetWindowPos( m_hwndViewport, NULL, m_WindowedPlacement.Left, m_WindowedPlacement.Top, m_WindowedPlacement.width(), m_WindowedPlacement.height(), SWP_NOACTIVATE );

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



bool VulkanRenderer::ReleaseBuffers()
{
  // invalidate all buffers

  // Release all vidmem objects
  SetRenderTarget( NULL );
  for ( int i = 0; i < 8; ++i )
  {
    SetTexture( i, NULL );
  }

  return true;
}



bool VulkanRenderer::SetMode( XRendererDisplayMode& DisplayMode )
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



bool VulkanRenderer::RecreateBuffers()
{
  return true;
}



void VulkanRenderer::RestoreAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    if ( pTexture->m_LoadedFromFile.empty() )
    {
      // eine bloss erzeugte Textur
      VulkanTexture*    pVulkanTexture = (VulkanTexture*)pTexture;

      // sicher gehen
      pVulkanTexture->Release();

      if ( !CreateNativeTextureResources( pVulkanTexture ) )
      {
        Log( "Renderer.General", "CreateTexture failed" );
      }

      GR::u32     MipMapLevel = 0;
      std::list<GR::Graphic::ImageData>::iterator   itID( pVulkanTexture->m_StoredImageData.begin() );
      while ( itID != pVulkanTexture->m_StoredImageData.end() )
      {
        CopyDataToTexture( pVulkanTexture, *itID, 0, MipMapLevel );

        ++itID;
        ++MipMapLevel;
      }
      if ( pVulkanTexture->AllowUsageAsRenderTarget )
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

        VulkanTexture*    pVulkanTexture = (VulkanTexture*)pTexture;

        // sicher gehen
        pVulkanTexture->Release();

        if ( CreateNativeTextureResources( pVulkanTexture ) )
        {
          CopyDataToTexture( pTexture, *pData );

          // Mipmap-Levels einlesen
          std::list<GR::String>::iterator    it( pVulkanTexture->m_listFileNames.begin() );

          GR::u32     Level = 1;

          while ( it != pVulkanTexture->m_listFileNames.end() )
          {
            GR::String&  strPath( *it );

            GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), ( GR::Graphic::eImageFormat )pVulkanTexture->m_ImageFormat, pVulkanTexture->m_ColorKey, pVulkanTexture->m_ColorKeyReplacementColor );
            if ( pData == NULL )
            {
              Log( "Renderer.General", CMisc::printf( "DX8Renderer:: RestoreAllTextures failed to load MipMap (%s)", strPath.c_str() ) );
            }
            else
            {
              CopyDataToTexture( pTexture, *pData, pVulkanTexture->m_ColorKey, Level );
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



bool VulkanRenderer::IsFullscreen()
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



void VulkanRenderer::SetCullFillMode( GR::u32 CullMode, GR::u32 FillMode )
{
  switch ( CullMode )
  {
    case XRenderer::RSV_CULL_CCW:
      break;
    case XRenderer::RSV_CULL_CW:
      break;
    case XRenderer::RSV_CULL_NONE:
      break;
  }
}



bool VulkanRenderer::SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage )
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
      }
      else
      {
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



bool VulkanRenderer::SetViewport( const XViewport& Viewport )
{
  FlushAllCaches();

  m_ViewPort = Viewport;

  /*
  CD3D11_VIEWPORT    viewPort;

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

  viewPort.Width    = (GR::f32)virtualX * Viewport.Width;
  viewPort.Height   = (GR::f32)virtualY * Viewport.Height;
  viewPort.TopLeftX = m_Canvas.Left + (GR::f32)virtualX * Viewport.X;
  viewPort.TopLeftY = m_Canvas.Top + (GR::f32)virtualY * Viewport.Y;
  viewPort.MinDepth = Viewport.MinZ;
  viewPort.MaxDepth = Viewport.MaxZ;

  if ( viewPort.TopLeftX >= m_Canvas.Left + m_Canvas.width() )
  {
    viewPort.TopLeftX = (GR::f32)m_Canvas.Left + m_Canvas.width();
    viewPort.Width = 0;
  }
  if ( viewPort.TopLeftY >= m_Canvas.Top + m_Canvas.height() )
  {
    viewPort.TopLeftY = (GR::f32)m_Canvas.Top + m_Canvas.height();
    viewPort.Height = 0;
  }
  if ( viewPort.TopLeftX + viewPort.Width > m_Canvas.Left + m_Canvas.width() )
  {
    viewPort.Width = m_Canvas.Left + m_Canvas.width() - viewPort.TopLeftX;
  }
  if ( viewPort.TopLeftY + viewPort.Height> m_Canvas.Top + m_Canvas.height() )
  {
    viewPort.Height = m_Canvas.Top + m_Canvas.height() - viewPort.TopLeftY;
  }

  // adjust ortho matrix?
  m_Matrices.ScreenCoord2d.OrthoOffCenterLH( viewPort.TopLeftX, 
                                             viewPort.TopLeftX + viewPort.Width,
                                             viewPort.TopLeftY + viewPort.Height,
                                             viewPort.TopLeftY, 
                                             0.0f, 
                                             1.0f );
  */
  //dh::Log( "Set viewport %f,%f  %fx%f", viewPort.TopLeftX, viewPort.TopLeftY, viewPort.Width, viewPort.Height );

  m_Matrices.ScreenCoord2d.Transpose();
  return true;
}



bool VulkanRenderer::SetTrueViewport( const XViewport& Viewport )
{ 
  FlushAllCaches();

  m_VirtualViewport = Viewport;

  /*
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
  }*/

  return true;
}



GR::u32 VulkanRenderer::Width()
{
  return m_Width;
}



GR::u32 VulkanRenderer::Height()
{
  return m_Height;
}



void VulkanRenderer::Clear( bool ClearColor, bool ClearZ )
{
  FlushAllCaches();
  if ( ClearColor )
  {
  }
  if ( ClearZ )
  {
  }
}



VkFormat VulkanRenderer::FindSupportedFormat( const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features ) 
{
  for ( VkFormat format : candidates ) 
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties( m_PhysicalDevice, format, &props );

    if ( ( tiling == VK_IMAGE_TILING_LINEAR )
    &&   ( ( props.linearTilingFeatures & features ) == features ) )
    {
      return format;
    }
    else if ( ( tiling == VK_IMAGE_TILING_OPTIMAL )
    &&        ( ( props.optimalTilingFeatures & features ) == features ) ) 
    {
      return format;
    }
  }

  dh::Log( "FindSupportedFormat failed for VkImageTiling %d, VkFormatFeatureFlags %d", tiling, features );
  return VK_FORMAT_UNDEFINED;
}


bool VulkanRenderer::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{

  /*
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
  }*/
  return true;
}



uint32_t VulkanRenderer::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties ) 
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties( m_PhysicalDevice, &memProperties );

  for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ ) 
  {
    if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties ) 
    {
      return i;
    }
  }
  dh::Log( "failed to find suitable memory type!" );
  return -1;
}



bool VulkanRenderer::CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory ) 
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if ( vkCreateBuffer( m_VulkanDevice, &bufferInfo, nullptr, &buffer ) != VK_SUCCESS ) 
  {
    dh::Log( "vkCreateBuffer failed" );
    return false;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements( m_VulkanDevice, buffer, &memRequirements );

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType( memRequirements.memoryTypeBits, properties );

  if ( vkAllocateMemory( m_VulkanDevice, &allocInfo, nullptr, &bufferMemory ) != VK_SUCCESS ) 
  {
    dh::Log( "vkAllocateMemory failed" );
    vkDestroyBuffer( m_VulkanDevice, buffer, nullptr );
    return false;
  }
  vkBindBufferMemory( m_VulkanDevice, buffer, bufferMemory, 0 );
  return true;
}



VulkanRenderer::QueueFamilyIndices VulkanRenderer::FindQueueFamilies( VkPhysicalDevice device )
{
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

  std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
  vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

  int i = 0;
  for ( const auto& queueFamily : queueFamilies ) 
  {
    if ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
    {
      indices.graphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_Surface, &presentSupport );

    if ( presentSupport ) 
    {
      indices.presentFamily = i;
    }

    if ( indices.isComplete() ) 
    {
      break;
    }

    i++;
  }

  return indices;
}



void VulkanRenderer::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = FindQueueFamilies( m_PhysicalDevice );

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamilyHasValue;

  if ( vkCreateCommandPool( m_VulkanDevice, &poolInfo, nullptr, &m_CommandPool ) != VK_SUCCESS )
  {
    dh::Log( "vkCreateCommandPool failed" );
  }
}



VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats )
{
  for ( const auto& availableFormat : availableFormats )
  {
    if ( availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}



VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes )
{
  for ( const auto& availablePresentMode : availablePresentModes )
  {
    if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
    {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}



VkExtent2D VulkanRenderer::chooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
{
  if ( capabilities.currentExtent.width != UINT32_MAX )
  {
    return capabilities.currentExtent;
  }
  else
  {
    VkExtent2D actualExtent = { m_Width, m_Height };

    actualExtent.width = math::maxValue( capabilities.minImageExtent.width, math::minValue( capabilities.maxImageExtent.width, actualExtent.width ) );
    actualExtent.height = math::maxValue( capabilities.minImageExtent.height, math::minValue( capabilities.maxImageExtent.height, actualExtent.height ) );

    return actualExtent;
  }
}



VulkanRenderer::SwapChainSupportDetails VulkanRenderer::querySwapChainSupport( VkPhysicalDevice device )
{
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, m_Surface, &details.capabilities );

  uint32_t formatCount = 0;
  if ( Failed( "vkGetPhysicalDeviceSurfaceFormatsKHR", vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_Surface, &formatCount, nullptr ) ) )
  {
    return SwapChainSupportDetails();
  }

  if ( formatCount != 0 )
  {
    details.formats.resize( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_Surface, &formatCount, details.formats.data() );
  }

  uint32_t presentModeCount = 0;
  if ( Failed( "vkGetPhysicalDeviceSurfacePresentModesKHR", vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_Surface, &presentModeCount, nullptr ) ) )
  {
    return SwapChainSupportDetails();
  }

  if ( presentModeCount != 0 )
  {
    details.presentModes.resize( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_Surface, &presentModeCount, details.presentModes.data() );
  }

  return details;
}



void VulkanRenderer::createLogicalDevice()
{
  QueueFamilyIndices indices = FindQueueFamilies( m_PhysicalDevice );

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

  float queuePriority = 1.0f;
  for ( uint32_t queueFamily : uniqueQueueFamilies )
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back( queueCreateInfo );
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = static_cast<uint32_t>( queueCreateInfos.size() );
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensions.size() );
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  /*
  if ( enableValidationLayers )
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayers.size() );
    createInfo.ppEnabledLayerNames = validationLayers.data();
  }
  else*/
  {
    createInfo.enabledLayerCount = 0;
  }

  if ( vkCreateDevice( m_PhysicalDevice, &createInfo, nullptr, &m_VulkanDevice ) != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create logical device!" );
  }

  vkGetDeviceQueue( m_VulkanDevice, indices.graphicsFamily, 0, &m_GraphicsQueue );
  vkGetDeviceQueue( m_VulkanDevice, indices.presentFamily, 0, &m_PresentQueue );
}



bool VulkanRenderer::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport( m_PhysicalDevice );

  VkSurfaceFormatKHR surfaceFormat  = chooseSwapSurfaceFormat( swapChainSupport.formats );
  VkPresentModeKHR presentMode      = chooseSwapPresentMode( swapChainSupport.presentModes );
  VkExtent2D extent                 = chooseSwapExtent( swapChainSupport.capabilities );

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if ( swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount )
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface  = m_Surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies( m_PhysicalDevice );
  uint32_t queueFamilyIndices[] = 
  { 
    indices.graphicsFamily, 
    indices.presentFamily 
  };

  if ( indices.graphicsFamily != indices.presentFamily )
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if ( vkCreateSwapchainKHR( m_VulkanDevice, &createInfo, nullptr, &m_SwapChain ) != VK_SUCCESS )
  {
    throw std::runtime_error( "failed to create swap chain!" );
  }

  vkGetSwapchainImagesKHR( m_VulkanDevice, m_SwapChain, &imageCount, nullptr );
  m_SwapChainImages.resize( imageCount );
  vkGetSwapchainImagesKHR( m_VulkanDevice, m_SwapChain, &imageCount, m_SwapChainImages.data() );

  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent = extent;
  return false;
}



VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_CommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers( m_VulkanDevice, &allocInfo, &commandBuffer );

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer( commandBuffer, &beginInfo );

  return commandBuffer;
}



void VulkanRenderer::endSingleTimeCommands( VkCommandBuffer commandBuffer ) 
{
  vkEndCommandBuffer( commandBuffer );

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit( m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
  vkQueueWaitIdle( m_GraphicsQueue );

  vkFreeCommandBuffers( m_VulkanDevice, m_CommandPool, 1, &commandBuffer );
}



XTexture* VulkanRenderer::CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormatArg, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  if ( ( Width == 0 )
  ||   ( Height == 0 )
  ||   ( m_VulkanDevice == NULL ) )
  {
    return NULL;
  }

  VkBuffer        stagingBuffer;
  VkDeviceMemory  stagingBufferMemory;

  VkDeviceSize    imageSize = Width * Height * GR::Graphic::ImageData::BytesProPixel( imgFormatArg );

  if ( !CreateBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory ) )
  {
    return NULL;
  }

  /*
  void*   pData = NULL;
  vkMapMemory( m_VulkanDevice, stagingBufferMemory, 0, imageSize, 0, &pData );
  memcpy( pData, pixels, static_cast<size_t>( imageSize ) );
  vkUnmapMemory( device, stagingBufferMemory );*/

  VkImageCreateInfo imageInfo = {};
  imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType     = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width  = static_cast<uint32_t>( Width );
  imageInfo.extent.height = static_cast<uint32_t>( Height );
  imageInfo.extent.depth  = 1;
  imageInfo.mipLevels     = 1;
  imageInfo.arrayLayers   = 1;

  // TODO - choose format from list
  imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags         = 0; // Optional

  VkImage         textureImage;
  VkDeviceMemory  textureImageMemory;

  if ( Failed( "vkCreateImage", vkCreateImage( m_VulkanDevice, &imageInfo, nullptr, &textureImage ) ) )
  {
    // TODO - destroy buffer
    return NULL;
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements( m_VulkanDevice, textureImage, &memRequirements );

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize  = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

  if ( Failed( "vkAllocateMemory", vkAllocateMemory( m_VulkanDevice, &allocInfo, nullptr, &textureImageMemory ) ) )
  {
    // TODO - destroy buffer
    // TODO - destroy image
    return NULL;
  }
  if ( Failed( "vkBindImageMemory", vkBindImageMemory( m_VulkanDevice, textureImage, textureImageMemory, 0 ) ) )
  {
    // TODO - destroy buffer
    // TODO - destroy image
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

    if ( imgFormat == GR::Graphic::IF_R8G8B8 )
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

  VulkanTexture* pTexture = new( std::nothrow )VulkanTexture();
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
  pTexture->m_ImageSourceSize.set( Width, Height );
  pTexture->m_ImageFormat         = imgFormat;

  pTexture->m_Device              = m_VulkanDevice;
  pTexture->m_TextureImage        = textureImage;
  pTexture->m_TextureImageMemory  = textureImageMemory;

  if ( !CreateNativeTextureResources( pTexture ) )
  {
    delete pTexture;
    return false;
  }

  AddTexture( pTexture );

  return pTexture;
}



bool VulkanRenderer::CreateNativeTextureResources( VulkanTexture* pTexture )
{
  return true;
}



XTexture* VulkanRenderer::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels, bool AllowUsageAsRenderTarget )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), MipMapLevels );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



XTexture* VulkanRenderer::LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  GR::String path = szFileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), imgFormatToConvert, ColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    dh::Log( "Failed to load texture %s", szFileName );
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData, MipMapLevels );
  if ( pTexture == NULL )
  {
    delete pData;
    return NULL;
  }
  pTexture->m_ColorKey        = ColorKey;
  pTexture->m_ColorKeyReplacementColor = ColorKeyReplacementColor;
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;
}



void VulkanRenderer::SetTexture( GR::u32 Stage, XTexture* pTexture )
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
  VulkanTexture*    pDXTexture = (VulkanTexture*)pTexture;
}



bool VulkanRenderer::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey, const GR::u32 MipMapLevel )
{
  FlushAllCaches();
  if ( pTexture == NULL )
  {
    dh::Error( "CopyDataToTexture: Texture was NULL" );
    return false;
  }

  VulkanTexture*    pDXTexture = (VulkanTexture*)pTexture;

  if ( MipMapLevel >= pDXTexture->m_MipMapLevels )
  {
    dh::Error( "CopyDataToTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, ( (VulkanTexture*)pTexture )->m_MipMapLevels );
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

  // store copied data
  while ( pDXTexture->m_StoredImageData.size() <= MipMapLevel )
  {
    pDXTexture->m_StoredImageData.push_back( GR::Graphic::ImageData() );
  }
  pDXTexture->m_StoredImageData.back() = ImageData;
  return true;
}



// Lights
bool VulkanRenderer::SetLight( GR::u32 LightIndex, XLight& Light )
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
    m_LightInfo.Light[LightIndex].Diffuse   = ColorValue( Light.m_Diffuse );
    m_LightInfo.Light[LightIndex].Ambient   = ColorValue( Light.m_Ambient );
    m_LightInfo.Light[LightIndex].Direction = Light.m_Direction;
    m_LightInfo.Light[LightIndex].Specular  = ColorValue( Light.m_Specular );
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
    m_LightsChanged = false;
  }
  else
  {
    m_LightsChanged = true;
  }
  return true;
}



bool VulkanRenderer::SetMaterial( const XMaterial& Material )
{
  FlushAllCaches();

  m_Material.Ambient        = ColorValue( Material.Ambient );
  m_Material.Emissive       = ColorValue( Material.Emissive );
  m_Material.Diffuse        = ColorValue( Material.Diffuse );
  m_Material.Specular       = ColorValue( Material.Specular );
  m_Material.SpecularPower  = Material.Power;

  if ( m_Material.Diffuse == ColorValue( 0 ) )
  {
    m_Material.Diffuse = ColorValue( 0xffffffff );
  }

  return true;
}



bool VulkanRenderer::SetFogInfo()
{
  FlushAllCaches();

  return true;
}



XVertexBuffer* VulkanRenderer::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  VulkanVertexBuffer*   pBuffer = new VulkanVertexBuffer( this );

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* VulkanRenderer::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  VulkanVertexBuffer*   pBuffer = new VulkanVertexBuffer( this );

  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* VulkanRenderer::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat )
{
  XVertexBuffer*    pBuffer = CreateVertexBuffer( MeshObject.FaceCount(), VertexFormat, XVertexBuffer::PT_TRIANGLE );

  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( MeshObject );

  return pBuffer;
}



void VulkanRenderer::DestroyVertexBuffer( XVertexBuffer* pVB )
{
  if ( pVB != NULL )
  {
    pVB->Release();
    delete pVB;

    m_VertexBuffers.remove( pVB );
  }
}



bool VulkanRenderer::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index, GR::u32 Count )
{
  VulkanVertexBuffer*   pVB = (VulkanVertexBuffer*)pBuffer;

  if ( pVB == NULL )
  {
    return false;
  }
  ChooseShaders( pVB->VertexFormat() );

  FlushAllCaches();
  return pVB->Display( Index, Count );
}



void VulkanRenderer::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 Color1, GR::u32 Color2, float fZ )
{
  if ( Color2 == 0 )
  {
    Color2 = Color1;
  }

  SetLineMode();
  m_LineCache3d.FlushCache();

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

  m_LineCache.AddEntry( m_SetTextures[0],
                        GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt1.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt1.y * virtualY, fZ ), 0.0f, 0.0f,
                        GR::tVector( m_Canvas.Left + m_DisplayOffset.x + ( GR::f32 )pt2.x * virtualX, m_Canvas.Top + m_DisplayOffset.y + ( GR::f32 )pt2.y * virtualY, fZ ), 0.0f, 0.0f,
                        Color1, Color2,
                        m_CurrentShaderType );
}



void VulkanRenderer::RenderLine( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color1, GR::u32 Color2 )
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



void VulkanRenderer::SetLineMode()
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



void VulkanRenderer::SetQuadMode()
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



void VulkanRenderer::SetTriangleMode()
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



void VulkanRenderer::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
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

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

  SetQuadMode();
  m_QuadCache3d.FlushCache();
  m_QuadCache.AddEntry( m_SetTextures[0],
    GR::tVector( m_Canvas.Left + iX * virtualX,              m_Canvas.Top + iY * virtualY, fZ ), fTU1, fTV1,
    GR::tVector( m_Canvas.Left + ( iX + iWidth ) * virtualX, m_Canvas.Top + iY * virtualY, fZ ), fTU2, fTV2,
    GR::tVector( m_Canvas.Left + iX * virtualX,              m_Canvas.Top + ( iY + iHeight ) * virtualY, fZ ), fTU3, fTV3,
    GR::tVector( m_Canvas.Left + ( iX + iWidth ) * virtualX, m_Canvas.Top + ( iY + iHeight ) * virtualY, fZ ), fTU4, fTV4,
    Color1, Color2, Color3, Color4, m_CurrentShaderType );
}



bool VulkanRenderer::RenderMesh2d( const Mesh::IMesh& Mesh )
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



void VulkanRenderer::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
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
  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

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



void VulkanRenderer::RenderQuad( const GR::tVector& ptPos1,
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



void VulkanRenderer::RenderQuad( const GR::tVector& ptPos1,
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



void VulkanRenderer::RenderTriangle( const GR::tVector& pt1,
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



void VulkanRenderer::RenderTriangle2d( const GR::tPoint& pt1,
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

  GR::f32     virtualX = (GR::f32)m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

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



bool VulkanRenderer::SaveScreenShot( const char* szFileName )
{
  FlushAllCaches();

  return true;
}



bool VulkanRenderer::VSyncEnabled()
{
  return m_VSyncEnabled;
}



void VulkanRenderer::EnableVSync( bool Enable )
{
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
  }
}



GR::Graphic::eImageFormat VulkanRenderer::ImageFormat()
{
  return GR::Graphic::IF_A8R8G8B8;
}



bool VulkanRenderer::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel )
{
  if ( pTexture == NULL )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: Texture was NULL" );
    return false;
  }

  VulkanTexture*    pVulkanTexture = (VulkanTexture*)pTexture;

  if ( MipMapLevel >= pVulkanTexture->m_StoredImageData.size() )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: MipMapLevel out of bounds %d >= %d", MipMapLevel, pVulkanTexture->m_MipMapLevels );
    return false;
  }

  std::list<GR::Graphic::ImageData>::iterator   it( pVulkanTexture->m_StoredImageData.begin() );
  std::advance( it, MipMapLevel );

  ImageData = *it;
  return true;
}



bool VulkanRenderer::RenderMesh( const Mesh::IMesh& Mesh )
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



void VulkanRenderer::RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1,
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

  GR::f32     virtualX = ( GR::f32 )m_Canvas.width() / m_VirtualSize.x;
  GR::f32     virtualY = ( GR::f32 )m_Canvas.height() / m_VirtualSize.y;

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



void VulkanRenderer::SetRenderTarget( XTexture* pTexture )
{
  if ( pTexture == m_pCurrentlySetRenderTargetTexture )
  {
    return;
  }
  FlushAllCaches();

  m_pCurrentlySetRenderTargetTexture = (VulkanTexture*)pTexture;
}



void VulkanRenderer::ChooseShaders( GR::u32 VertexFormat )
{
  eShaderType   shaderTypeToUse = m_CurrentShaderType;

  if ( shaderTypeToUse == ST_ALPHA_BLEND )
  {
    shaderTypeToUse = ST_FLAT;
  }
  else if ( shaderTypeToUse == ST_ALPHA_BLEND_AND_TEST )
  {
    shaderTypeToUse = ST_ALPHA_TEST;
  }
  else if ( shaderTypeToUse == ST_ADDITIVE )
  {
  }
  else
  {
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



void VulkanRenderer::RenderBox( const GR::tVector& vPos, const GR::tVector& vSize, GR::u32 Color )
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

  Mesh::Face      faceFront1( 2, Color, 3, Color, 6, Color );
  Mesh::Face      faceFront2( 6, Color, 3, Color, 7, Color );

  Mesh::Face      faceLeft1( 0, Color, 2, Color, 4, Color );
  Mesh::Face      faceLeft2( 4, Color, 2, Color, 6, Color );

  Mesh::Face      faceRight1( 3, Color, 1, Color, 7, Color );
  Mesh::Face      faceRight2( 7, Color, 1, Color, 5, Color );

  Mesh::Face      faceBack1( 1, Color, 0, Color, 5, Color );
  Mesh::Face      faceBack2( 5, Color, 0, Color, 4, Color );

  Mesh::Face      faceTop1( 0, Color, 1, Color, 2, Color );
  Mesh::Face      faceTop2( 2, Color, 1, Color, 3, Color );

  Mesh::Face      faceBottom1( 6, Color, 7, Color, 4, Color );
  Mesh::Face      faceBottom2( 4, Color, 7, Color, 5, Color );

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



void VulkanRenderer::SetShader( eShaderType sType )
{
  m_CurrentShaderType = sType;
}



XFont* VulkanRenderer::LoadFontSquare( const char* szFileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  XFont*   pFont = new VulkanFont( this, m_pEnvironment );

  pFont->LoadFontSquare( szFileName, Flags, TransparentColor );

  AddFont( pFont );

  return pFont;
}



XFont* VulkanRenderer::LoadFont( const char* szFileName, GR::u32 TransparentColor )
{
  XFont*    pFont = new VulkanFont( this, m_pEnvironment );

  if ( !pFont->LoadFont( szFileName, 0, TransparentColor ) )
  {
    delete pFont;
    return NULL;
  }
  AddFont( pFont );
  return pFont;
}



int VulkanRenderer::MaxPrimitiveCount()
{
  return 10000;
}


