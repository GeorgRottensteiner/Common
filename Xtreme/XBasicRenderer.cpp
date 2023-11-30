#include "XBasicRenderer.h"
#include "XBasicFont.h"

#include <Xtreme/MeshFormate/T3DLoader.h>
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
#include <Grafik/GDIPage.h>
#endif

#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Interface/IValueStorage.h>
#include <Interface/IFileLocator.h>

#include <debug/DebugService.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/XTextureBase.h>



XBasicRenderer::XBasicRenderer() :
  m_ClearColor( 0xff000000 ),
  m_pDebugger( NULL ),
  m_DirectTexelMappingOffset( 0.0f, 0.0f )
{
  memset( &m_Material, 0, sizeof( m_Material ) );
  memset( &m_Light,    0, sizeof( m_Light ) );
}



XBasicRenderer::~XBasicRenderer()
{
}



void XBasicRenderer::VirtualSize( const GR::tPoint& NewSize )
{
  m_VirtualSize = NewSize;
}



GR::tPoint XBasicRenderer::VirtualSize() const
{
  return m_VirtualSize;
}



void XBasicRenderer::AddTexture( XTexture* pTexture )
{
  m_Textures.push_back( pTexture );
}



void XBasicRenderer::AddVertexBuffer( XVertexBuffer* pVBBuffer )
{
  m_VertexBuffers.push_back( pVBBuffer );
}



void XBasicRenderer::AddFont( XFont* pFont )
{
  m_Fonts.push_back( pFont );
}



void XBasicRenderer::UnloadAssets( Xtreme::Asset::eXAssetType Type )
{
  Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader )
  {
    GR::up    assetCount = pLoader->AssetTypeCount( Type );
    for ( GR::up i = 0; i < assetCount; ++i )
    {
      Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Type, i );
      delete pAsset;
      pLoader->SetAsset( Type, i, NULL );
    }
  }
}



void XBasicRenderer::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = ( Xtreme::Asset::XAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu entladen
      UnloadAssets( Xtreme::Asset::XA_IMAGE );
      UnloadAssets( Xtreme::Asset::XA_IMAGE_SECTION );
      UnloadAssets( Xtreme::Asset::XA_FONT );
      UnloadAssets( Xtreme::Asset::XA_MESH );
      pLoader->NotifyService( "GUI", "AssetsUnloaded" );
    }
  }
}



void XBasicRenderer::ReleaseAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    pTexture->Release();

    ++it;
  }
}



void XBasicRenderer::DestroyTexture( XTexture* pTexture )
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pThisTexture = *it;

    if ( pThisTexture == pTexture )
    {
      pTexture->Release();
      delete pTexture;

      m_Textures.erase( it );

      return;
    }

    ++it;
  }
}



void XBasicRenderer::DestroyAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    pTexture->Release();
    delete pTexture;

    ++it;
  }
  m_Textures.clear();
}



void XBasicRenderer::RestoreAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    //pTexture->>ReRelease();

    ++it;
  }
}



void XBasicRenderer::RestoreStates()
{
  tMapRenderStates    tempStates = m_RenderStates;

  m_RenderStates.clear();
  tMapRenderStates::iterator    itState( tempStates.begin() );
  while ( itState != tempStates.end() )
  {
    SetState( itState->first.first, itState->second, itState->first.second );

    ++itState;
  }

  for ( GR::u32 i = 0; i < TT_LAST_ENTRY; ++i )
  {
    SetTransform( (XBasicRenderer::eTransformType)i, m_Transform[i] );
  }

  SetMaterial( m_Material );

  for ( GR::u32 i = 0; i < 8; ++i )
  {
    if ( m_Light[i].m_Type != XLight::LT_INVALID )
    {
      SetLight( i, m_Light[i] );
    }
  }
}



void XBasicRenderer::DestroyAllVertexBuffers()
{
  tListVertexBuffers::iterator   it( m_VertexBuffers.begin() );
  while ( it != m_VertexBuffers.end() )
  {
    XVertexBuffer*   pVBBuffer = *it;

    pVBBuffer->Release();
    delete pVBBuffer;

    ++it;
  }
  m_VertexBuffers.clear();
}



void XBasicRenderer::ReleaseAllVertexBuffers()
{
  tListVertexBuffers::iterator   it( m_VertexBuffers.begin() );
  while ( it != m_VertexBuffers.end() )
  {
    XVertexBuffer*   pVBBuffer = *it;

    pVBBuffer->Release();

    ++it;
  }
}



void XBasicRenderer::RestoreAllVertexBuffers()
{
  tListVertexBuffers::iterator   it( m_VertexBuffers.begin() );
  while ( it != m_VertexBuffers.end() )
  {
    XVertexBuffer*   pVBBuffer = *it;

    pVBBuffer->Restore();

    ++it;
  }
}



void XBasicRenderer::RestoreAllFonts()
{
  tListFonts::iterator   it( m_Fonts.begin() );
  while ( it != m_Fonts.end() )
  {
    XFont*   pFont = *it;

    pFont->Recreate();

    ++it;
  }
}



void XBasicRenderer::DestroyFont( XFont* pFont )
{
  tListFonts::iterator   it( m_Fonts.begin() );
  while ( it != m_Fonts.end() )
  {
    XFont*   pOtherFont = *it;

    if ( pFont == pOtherFont )
    {
      pFont->Release();
      delete pFont;
      m_Fonts.erase( it );
      return;
    }
    ++it;
  }
}



void XBasicRenderer::DestroyAllFonts()
{
  tListFonts::iterator   it( m_Fonts.begin() );
  while ( it != m_Fonts.end() )
  {
    XFont*   pFont = *it;

    pFont->Release();
    delete pFont;

    ++it;
  }
  m_Fonts.clear();
}



void XBasicRenderer::SetTransform( eTransformType Type, const math::matrix4& Matrix )
{
  m_Transform[Type] = Matrix;
}



void XBasicRenderer::AdjustWindowForMode( bool Windowed )
{
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  if ( Windowed )
  {
    m_WindowedModeExStyles &= ~WS_EX_TOPMOST;

    SetWindowPlacement( m_hwndViewport, &m_wPlacement );

    Log( "Renderer.Full", "AdjustWindowForMode Windowed" );
    Log( "Renderer.Full", "Actual Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
    Log( "Renderer.Full", "Actual Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );
    Log( "Renderer.Full", "Stored Window Styles %x", m_WindowedModeStyles );
    Log( "Renderer.Full", "Stored Window ExStyles %x", m_WindowedModeExStyles );

    SetWindowLong( m_hwndViewport, GWL_STYLE, m_WindowedModeStyles );
    SetWindowLong( m_hwndViewport, GWL_EXSTYLE, m_WindowedModeExStyles );

    Log( "Renderer.Full", "AdjustWindowForMode Windowed after setting styles" );
    Log( "Renderer.Full", "Actual Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
    Log( "Renderer.Full", "Actual Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );

    SetWindowPos( m_hwndViewport, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED );

    SetMenu( m_hwndViewport, m_hMenu );

    Log( "Renderer.Full", "AdjustWindowForMode Windowed done" );
    Log( "Renderer.Full", "Actual Window Styles %x", GetWindowLong( m_hwndViewport, GWL_STYLE ) );
    Log( "Renderer.Full", "Actual Window ExStyles %x", GetWindowLong( m_hwndViewport, GWL_EXSTYLE ) );
  }
  else
  {
    m_hMenu = GetMenu( m_hwndViewport );
    SetMenu( m_hwndViewport, NULL );

    m_wPlacement.length = sizeof( m_wPlacement );
    GetWindowPlacement( m_hwndViewport, &m_wPlacement );
    m_WindowedModeStyles    = GetWindowLong( m_hwndViewport, GWL_STYLE );
    m_WindowedModeExStyles  = GetWindowLong( m_hwndViewport, GWL_EXSTYLE );

    m_WindowedModeExStyles &= ~WS_EX_TOPMOST;

    GR::u32   dwStyles = m_WindowedModeStyles;

    //dwStyles &= ~( WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_OVERLAPPED | WS_CHILD | WS_DLGFRAME | WS_THICKFRAME );
    //dwStyles |= WS_POPUP;
    //m_hwndWindowedModeParent = SetParent( m_hwndViewport, NULL );
    dwStyles = WS_POPUP | WS_VISIBLE;
    SetWindowLong( m_hwndViewport, GWL_STYLE, dwStyles );

    SetWindowPos( m_hwndViewport, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED );
  }
#endif
}



XViewport XBasicRenderer::Viewport()
{
  return m_ViewPort;
}



XViewport XBasicRenderer::TrueViewport()
{
  return m_VirtualViewport;
}



math::matrix4 XBasicRenderer::Matrix( XBasicRenderer::eTransformType Type )
{
  return m_Transform[Type];
}



void XBasicRenderer::TransformToScreen( GR::tVector& Pos )
{
  math::matrix4     matComplete = m_Transform[TT_WORLD] * m_Transform[TT_VIEW] * m_Transform[TT_PROJECTION];

  matComplete.TransformCoord( Pos );

  Pos.x = ( Pos.x + 1.0f ) * m_ViewPort.Width * 0.5f + m_ViewPort.X;
  Pos.y = ( 1.0f - Pos.y ) * m_ViewPort.Height * 0.5f + m_ViewPort.Y;
  Pos.z = ( Pos.z + 1.0f ) * 0.5f;
}



GR::tVector& XBasicRenderer::Project( GR::tVector& Pos,
                                      const XViewport& ViewPort,
                                      const math::matrix4& Projection,
                                      const math::matrix4& View,
                                      const math::matrix4& World )
{
  math::matrix4     matComplete = World * View * Projection;

  matComplete.TransformCoord( Pos );

  Pos.x = ( Pos.x + 1.0f ) * ViewPort.Width * 0.5f + ViewPort.X;
  Pos.y = ( 1.0f - Pos.y ) * ViewPort.Height * 0.5f + ViewPort.Y;
  Pos.z = ( Pos.z + 1.0f ) * 0.5f;

  return Pos;
}



GR::tVector& XBasicRenderer::Unproject( GR::tVector& Pos,
                                   const XViewport& ViewPort,
                                   const math::matrix4& Projection,
                                   const math::matrix4& View,
                                   const math::matrix4& World )
{
  GR::tVector   temp;

  temp.x = ( ( Pos.x - ViewPort.X ) * 2.0f / ViewPort.Width - 1.0f );
  temp.y = 1.0f - ( Pos.y - ViewPort.Y ) * 2.0f / ViewPort.Height;
  temp.z = 2 * Pos.z - 1.0f;

  math::matrix4     matComplete;

  matComplete = World * View * Projection;

  matComplete.Inverse();

  matComplete.TransformCoord( temp );

  Pos = temp;
  return Pos;
}



void XBasicRenderer::CastRayFromScreen( int X, int Y, GR::tVector& Pos1, GR::tVector& Pos2, float Z1Value )
{
  GR::tVector vectOrig1 = GR::tVector( float( X ), float( Y ), Z1Value ),
              vectOrig2 = GR::tVector( float( X ), float( Y ), 1.0f );

  Pos1 = Unproject( vectOrig1, m_ViewPort, m_Transform[TT_PROJECTION], m_Transform[TT_VIEW], m_Transform[TT_WORLD] );
  Pos2 = Unproject( vectOrig2, m_ViewPort, m_Transform[TT_PROJECTION], m_Transform[TT_VIEW], m_Transform[TT_WORLD] );
}



void XBasicRenderer::RenderQuad( const GR::tVector& Pos,
                                 const GR::tVector& Size,
                                 GR::f32 TU1, GR::f32 TPos1,
                                 GR::f32 TU2, GR::f32 TPos2,
                                 GR::u32 Color1, GR::u32 Color2,
                                 GR::u32 Color3, GR::u32 Color4 )
{
  RenderQuad( Pos,
              Pos + GR::tVector( Size.x, 0.0f, 0.0f ),
              Pos + GR::tVector( 0.0f, Size.y, 0.0f ),
              Pos + GR::tVector( Size.x, Size.y, 0.0f ),
              TU1, TPos1,
              TU2, TPos2,
              Color1, Color2, Color3, Color4 );
}



void XBasicRenderer::RenderQuad( const GR::tVector& Pos,
                                 const GR::tVector& Size,
                                 GR::u32 Color1, GR::u32 Color2,
                                 GR::u32 Color3, GR::u32 Color4 )
{
  RenderQuad( Pos,
              Pos + GR::tVector( Size.x, 0.0f, 0.0f ),
              Pos + GR::tVector( 0.0f, Size.y, 0.0f ),
              Pos + GR::tVector( Size.x, Size.y, 0.0f ),
              0.0f, 0.0f, 0.0f, 0.0f,
              Color1, Color2, Color3, Color4 );
}



void XBasicRenderer::RenderQuad( const GR::tVector& Pos1,
                            const GR::tVector& Pos2,
                            const GR::tVector& Pos3,
                            const GR::tVector& Pos4,
                            GR::u32 Color1, GR::u32 Color2,
                            GR::u32 Color3, GR::u32 Color4 )
{
  RenderQuad( Pos1, Pos2, Pos3, Pos4,
              0.0f, 0.0f, 1.0f, 1.0f,
              Color1, Color2, Color3, Color4 );
}



void XBasicRenderer::RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                              GR::u32 Color1, GR::u32 Color2,
                              GR::u32 Color3, GR::u32 Color4, float Z )
{
  RenderQuad2d( X, Y, Width, Height,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                Color1, Color2, Color3, Color4, Z );
}



void XBasicRenderer::RenderTextureSection( const GR::tVector& Pos1,
                                           const GR::tVector& Pos2,
                                           const GR::tVector& Pos3,
                                           const GR::tVector& Pos4,
                                           const XTextureSection& TexSection,
                                           GR::u32 Color1, GR::u32 Color2,
                                           GR::u32 Color3, GR::u32 Color4,
                                           GR::u32 AlternativeFlags )
{
  SetTexture( 0, TexSection.m_pTexture );

  static GR::tFPoint     UPos1;
  static GR::tFPoint     UPos2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  if ( AlternativeFlags == -1 )
  {
    AlternativeFlags = TexSection.m_Flags;
  }
  TexSection.GetTrueUV( UPos1, UPos2, UV3, UV4, AlternativeFlags );

  RenderQuad( Pos1, Pos2, Pos3, Pos4,
              UPos1.x, UPos1.y,
              UPos2.x, UPos2.y,
              UV3.x, UV3.y,
              UV4.x, UV4.y,
              Color1, Color2, Color3, Color4 );
}



void XBasicRenderer::RenderTextureSection2d( GR::i32 X, GR::i32 Y,
                                             const XTextureSection& TexSection,
                                             GR::u32 Color,
                                             GR::i32 Width, GR::i32 Height,
                                             GR::u32 AlternativeFlags,
                                             float Z )
{
  SetTexture( 0, TexSection.m_pTexture );

  if ( Width == -1 )
  {
    Width  = TexSection.m_Width;
  }
  if ( Height == -1 )
  {
    Height = TexSection.m_Height;
  }

  static GR::tFPoint     UPos1;
  static GR::tFPoint     UPos2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  if ( AlternativeFlags == -1 )
  {
    AlternativeFlags = TexSection.m_Flags;
  }
  TexSection.GetTrueUV( UPos1, UPos2, UV3, UV4, AlternativeFlags );

  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( Width, Height );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= Width / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= Height / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= Height - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= Width - 1;
  }
  RenderQuad2d( X, Y,
                Width, Height,
                UPos1.x, UPos1.y,
                UPos2.x, UPos2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color,
                Color,
                Color,
                Color,
                Z );
}



void XBasicRenderer::RenderTextureSection2d( GR::i32 X, GR::i32 Y,
                                        const XTextureSection& TexSection,
                                        GR::u32 Color1, GR::u32 Color2,
                                        GR::u32 Color3, GR::u32 Color4,
                                        GR::i32 Width, GR::i32 Height,
                                        GR::u32 AlternativeFlags, float Z )
{

  SetTexture( 0, TexSection.m_pTexture );

  if ( Width == -1 )
  {
    Width  = TexSection.m_Width;
  }
  if ( Height == -1 )
  {
    Height = TexSection.m_Height;
  }

  static GR::tFPoint     UPos1;
  static GR::tFPoint     UPos2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  if ( AlternativeFlags == -1 )
  {
    AlternativeFlags = TexSection.m_Flags;
  }
  TexSection.GetTrueUV( UPos1, UPos2, UV3, UV4, AlternativeFlags );
  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( Width, Height );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= Width / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= Height / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= Height - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= Width - 1;
  }

  RenderQuad2d( X, Y,
                Width, Height,
                UPos1.x, UPos1.y,
                UPos2.x, UPos2.y,
                UV3.x, UV3.y,
                UV4.x, UV4.y,
                Color1, Color2, Color3, Color4,
                Z );
}



void XBasicRenderer::RenderTextureSectionRotated2d( GR::i32 X, GR::i32 Y,
                                                    const XTextureSection& TexSection,
                                                    GR::f32 Angle,
                                                    GR::f32 ZoomFactorX, GR::f32 ZoomFactorY,
                                                    GR::u32 Color, GR::u32 Color2,
                                                    GR::u32 Color3, GR::u32 Color4,
                                                    GR::i32 Width, GR::i32 Height,
                                                    GR::u32 AlternativeFlags, float Z )
{
  SetTexture( 0, TexSection.m_pTexture );

  if ( Width == -1 )
  {
    Width = TexSection.m_Width;
  }
  if ( Height == -1 )
  {
    Height = TexSection.m_Height;
  }

  static GR::tFPoint     UPos1;
  static GR::tFPoint     UPos2;
  static GR::tFPoint     UV3;
  static GR::tFPoint     UV4;

  if ( AlternativeFlags == -1 )
  {
    AlternativeFlags = TexSection.m_Flags;
  }
  TexSection.GetTrueUV( UPos1, UPos2, UV3, UV4, AlternativeFlags );
  if ( ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_270 )
  ||   ( ( AlternativeFlags & XTextureSection::TSF_ROTATE_270 ) == XTextureSection::TSF_ROTATE_90 ) )
  {
    std::swap( Width, Height );
  }

  if ( AlternativeFlags & XTextureSection::TSF_HCENTER )
  {
    X -= Width / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_VCENTER )
  {
    Y -= Height / 2;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_BOTTOM )
  {
    Y -= Height - 1;
  }
  if ( AlternativeFlags & XTextureSection::TSF_ALIGN_RIGHT )
  {
    X -= Width - 1;
  }

  GR::tVector   delta( -Width * 0.5f * ZoomFactorX,
                        -Height * 0.5f * ZoomFactorY,
                        0.0f );
  GR::tVector   delta2( Width * 0.5f * ZoomFactorX,
                         -Height * 0.5f * ZoomFactorY,
                         0.0f );
  GR::tVector   delta3( -Width * 0.5f * ZoomFactorX,
                         Height * 0.5f * ZoomFactorY,
                         0.0f );
  GR::tVector   delta4( Width * 0.5f * ZoomFactorX,
                         Height * 0.5f * ZoomFactorY,
                         0.0f );

  delta.RotateZ( -Angle );
  delta2.RotateZ( -Angle );
  delta3.RotateZ( -Angle );
  delta4.RotateZ( -Angle );

  GR::tVector   pos1 = GR::tVector( (float)X, (float)Y, 0 ) + delta;
  GR::tVector   pos2 = GR::tVector( (float)X, (float)Y, 0 ) + delta2;
  GR::tVector   pos3 = GR::tVector( (float)X, (float)Y, 0 ) + delta3;
  GR::tVector   pos4 = GR::tVector( (float)X, (float)Y, 0 ) + delta4;

  RenderQuadDetail2d( pos1.x, pos1.y,
                      pos2.x, pos2.y,
                      pos3.x, pos3.y,
                      pos4.x, pos4.y,
                      UPos1.x, UPos1.y,
                      UPos2.x, UPos2.y,
                      UV3.x, UV3.y,
                      UV4.x, UV4.y,
                      Color, Color2, Color3, Color4, Z );
}



void XBasicRenderer::RenderBoundingBox( const XBoundingBox& BBox, GR::u32 Color )
{
  if ( !BBox.m_Transformed )
  {
    RenderBoundingBox( BBox.UpperLeftCorner(), BBox.LowerRightCorner(), Color );
    return;
  }

  RenderLine( BBox.TransformedCorner( 0 ),
              BBox.TransformedCorner( 1 ),
              Color );
  RenderLine( BBox.TransformedCorner( 1 ),
                BBox.TransformedCorner( 2 ),
                Color );
  RenderLine( BBox.TransformedCorner( 2 ),
                BBox.TransformedCorner( 3 ),
                Color );
  RenderLine( BBox.TransformedCorner( 3 ),
                BBox.TransformedCorner( 0 ),
                Color );

  RenderLine( BBox.TransformedCorner( 4 ),
                BBox.TransformedCorner( 5 ),
                Color );
  RenderLine( BBox.TransformedCorner( 5 ),
                BBox.TransformedCorner( 6 ),
                Color );
  RenderLine( BBox.TransformedCorner( 6 ),
                BBox.TransformedCorner( 7 ),
                Color );
  RenderLine( BBox.TransformedCorner( 7 ),
                BBox.TransformedCorner( 4 ),
                Color );

  RenderLine( BBox.TransformedCorner( 0 ),
                BBox.TransformedCorner( 4 ),
                Color );
  RenderLine( BBox.TransformedCorner( 1 ),
                BBox.TransformedCorner( 5 ),
                Color );
  RenderLine( BBox.TransformedCorner( 2 ),
                BBox.TransformedCorner( 6 ),
                Color );
  RenderLine( BBox.TransformedCorner( 3 ),
                BBox.TransformedCorner( 7 ),
                Color );
}



void XBasicRenderer::RenderBoundingBox( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color )
{
  RenderLine( GR::tVector( Pos1.x, Pos1.y, Pos1.z ),
              GR::tVector( Pos1.x, Pos2.y, Pos1.z ),
              Color );

  RenderLine( GR::tVector( Pos1.x,
                            Pos2.y,
                            Pos1.z ),
                GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos1.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos1.z ),
                GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos1.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos1.z ),
                GR::tVector( Pos1.x,
                            Pos1.y,
                            Pos1.z ),
                Color );

  // Seite 2
  RenderLine( GR::tVector( Pos1.x,
                            Pos1.y,
                            Pos2.z ),
                GR::tVector( Pos1.x,
                            Pos2.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos1.x,
                            Pos2.y,
                            Pos2.z ),
                GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos2.z ),
                GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos2.z ),
                GR::tVector( Pos1.x,
                            Pos1.y,
                            Pos2.z ),
                Color );

  // Seite 3
  RenderLine( GR::tVector( Pos1.x,
                            Pos1.y,
                            Pos1.z ),
                GR::tVector( Pos1.x,
                            Pos1.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos1.x,
                            Pos2.y,
                            Pos1.z ),
                GR::tVector( Pos1.x,
                            Pos2.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos1.z ),
                GR::tVector( Pos2.x,
                            Pos1.y,
                            Pos2.z ),
                Color );

  RenderLine( GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos1.z ),
                GR::tVector( Pos2.x,
                            Pos2.y,
                            Pos2.z ),
                Color );
}



void XBasicRenderer::RenderBox( const GR::tVector& Pos, const GR::tVector& Size, GR::u32 Color )
{
  // top
  RenderQuad( GR::tVector( Pos.x, Pos.y, Pos.z + Size.z ),
              GR::tVector( Pos.x + Size.x, Pos.y, Pos.z + Size.z ),
              GR::tVector( Pos.x, Pos.y + Size.y, Pos.z + Size.z ),
              GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z + Size.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );

  // right side
  RenderQuad( GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z ),
              GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z + Size.z ),
              GR::tVector( Pos.x + Size.x, Pos.y, Pos.z ),
              GR::tVector( Pos.x + Size.x, Pos.y, Pos.z + Size.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );

  // bottom?
  RenderQuad( GR::tVector( Pos.x + Size.x, Pos.y, Pos.z ),
              GR::tVector( Pos.x, Pos.y, Pos.z ),
              GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z ),
              GR::tVector( Pos.x, Pos.y + Size.y, Pos.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );

  // left
  RenderQuad( GR::tVector( Pos.x, Pos.y, Pos.z + Size.z ),
              GR::tVector( Pos.x, Pos.y + Size.y, Pos.z + Size.z ),
              GR::tVector( Pos.x, Pos.y, Pos.z ),
              GR::tVector( Pos.x, Pos.y + Size.y, Pos.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );

  // front side
  RenderQuad( GR::tVector( Pos.x, Pos.y + Size.y, Pos.z + Size.z ),
              GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z + Size.z ),
              GR::tVector( Pos.x, Pos.y + Size.y, Pos.z ),
              GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );

  // back side
  RenderQuad( GR::tVector( Pos.x + Size.x, Pos.y, Pos.z + Size.z ),
              GR::tVector( Pos.x + Size.x, Pos.y, Pos.z + Size.z ),
              GR::tVector( Pos.x, Pos.y, Pos.z ),
              GR::tVector( Pos.x, Pos.y, Pos.z ),
              0.0f, 0.0f, 1.0f, 0.0f,
              0.0f, 1.0f, 1.0f, 1.0f,
              Color );
}



void XBasicRenderer::SetShader( eShaderType Type )
{
  switch ( Type )
  {
    case ST_FLAT:
      SetState( RS_ALPHATEST, RSV_DISABLE );
      SetState( RS_ALPHABLENDING, RSV_DISABLE );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_SELECT_ARG_1 );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );
      break;
    case ST_FLAT_NO_TEXTURE:
      SetState( RS_ALPHATEST, RSV_DISABLE );
      SetState( RS_ALPHABLENDING, RSV_DISABLE );

      SetState( RS_COLOR_OP,    RSV_SELECT_ARG_1 );
      SetState( RS_COLOR_ARG_1, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_SELECT_ARG_1 );
      SetState( RS_ALPHA_ARG_1, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );

      SetTexture( 0, NULL );
      break;
    case ST_ALPHA_BLEND:
      SetState( RS_ALPHATEST, RSV_DISABLE );
      SetState( RS_ALPHABLENDING, RSV_ENABLE );

      SetState( RS_SRC_BLEND, RSV_SRC_ALPHA );
      SetState( RS_DEST_BLEND, RSV_INV_SRC_ALPHA );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );
      break;
    case ST_50_PERCENT_BLEND:
      SetState( RS_ALPHATEST, RSV_DISABLE );
      SetState( RS_ALPHABLENDING, RSV_ENABLE );

      SetState( RS_SRC_BLEND, RSV_ONE );
      SetState( RS_DEST_BLEND, RSV_ONE );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );
      break;
    case ST_ALPHA_TEST:
      SetState( RS_ALPHATEST, RSV_ENABLE );
      SetState( RS_ALPHABLENDING, RSV_DISABLE );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );

      SetState( RS_ALPHAFUNC,   RSV_COMPARE_GREATEREQUAL );
      SetState( RS_ALPHAREF,    8 );
      break;
    case ST_ALPHA_BLEND_AND_TEST:
      SetState( RS_ALPHATEST, RSV_ENABLE );
      SetState( RS_ALPHABLENDING, RSV_ENABLE );

      SetState( RS_SRC_BLEND, RSV_SRC_ALPHA );
      SetState( RS_DEST_BLEND, RSV_INV_SRC_ALPHA );

      SetState( RS_ALPHAFUNC,   RSV_COMPARE_GREATEREQUAL );
      SetState( RS_ALPHAREF,    8 );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );
      break;
    case ST_ADDITIVE:
      SetState( RS_ALPHATEST, RSV_DISABLE );
      SetState( RS_ALPHABLENDING, RSV_ENABLE );

      SetState( RS_SRC_BLEND, RSV_SRC_ALPHA );
      SetState( RS_DEST_BLEND, RSV_ONE );

      SetState( RS_COLOR_OP,    RSV_MODULATE );
      SetState( RS_COLOR_ARG_1, RSV_TEXTURE );
      SetState( RS_COLOR_ARG_2, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP,    RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP,    RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP,    RSV_DISABLE, 1 );
      break;
    case ST_ALPHA_TEST_COLOR_FROM_DIFFUSE:
      SetState( RS_ALPHATEST, RSV_ENABLE );
      SetState( RS_ALPHABLENDING, RSV_DISABLE );

      SetState( RS_COLOR_OP, RSV_SELECT_ARG_1 );
      SetState( RS_COLOR_ARG_1, RSV_DIFFUSE );

      SetState( RS_ALPHA_OP, RSV_MODULATE );
      SetState( RS_ALPHA_ARG_1, RSV_TEXTURE );
      SetState( RS_ALPHA_ARG_2, RSV_DIFFUSE );

      SetState( RS_COLOR_OP, RSV_DISABLE, 1 );
      SetState( RS_ALPHA_OP, RSV_DISABLE, 1 );

      SetState( RS_ALPHAFUNC, RSV_COMPARE_GREATEREQUAL );
      SetState( RS_ALPHAREF, 8 );
      break;
  }
}



void XBasicRenderer::RenderText2d( XFont* pFont, int X, int Y, const GR::Char* Text, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, Color, Z );
  }
}



void XBasicRenderer::RenderText2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, Color, Z );
  }
}



void XBasicRenderer::RenderText2d( XFont* pFont, int X, int Y, const GR::Char* Text,
                               float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4,
                               float Z )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, ScaleX, ScaleY, Color1, Color2, Color3, Color4, Z );
  }
}



void XBasicRenderer::RenderText2d( XFont* pFont, int X, int Y, const GR::String& Text,
                                   float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2, GR::u32 Color3, GR::u32 Color4,
                                   float Z )
{
  if ( pFont )
  {
    pFont->DrawText( X, Y, Text, ScaleX, ScaleY, Color1, Color2, Color3, Color4, Z );
  }
}



void XBasicRenderer::RenderText( XFont* pFont, const GR::tVector& Pos, const GR::Char* Text,
                                 const GR::tVector& Scale,
                                 GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawFreeText( Pos, Text, Scale, Color );
  }
}



void XBasicRenderer::RenderText( XFont* pFont, const GR::tVector& vectPos, const GR::String& Text,
                                 const GR::tVector& vectScale,
                                 GR::u32 Color )
{
  if ( pFont )
  {
    pFont->DrawFreeText( vectPos, Text, vectScale, Color );
  }
}



void XBasicRenderer::RenderRect2d( const GR::tPoint& Pos, const GR::tPoint& Size, GR::u32 Color1, float Z )
{
  RenderLine2d( Pos,
                GR::tPoint( Pos.x + Size.x - 1, Pos.y ), Color1, Color1,
                Z );
  RenderLine2d( GR::tPoint( Pos.x + Size.x - 1, Pos.y ),
                GR::tPoint( Pos.x + Size.x - 1, Pos.y + Size.y - 1 ), Color1, Color1,
                Z );
  RenderLine2d( Pos, GR::tPoint( Pos.x, Pos.y + Size.y - 1 ), Color1, Color1, Z );
  RenderLine2d( GR::tPoint( Pos.x, Pos.y + Size.y - 1 ),
                GR::tPoint( Pos.x + Size.x - 1, Pos.y + Size.y - 1 ), Color1, Color1,
                Z );
}



void XBasicRenderer::RenderRect( const GR::tVector& Pos1, const GR::tVector& Pos2,
                                 const GR::tVector& Pos3, const GR::tVector& Pos4, GR::u32 Color1 )
{
  RenderLine( Pos1, Pos2, Color1 );
  RenderLine( Pos2, Pos3, Color1 );
  RenderLine( Pos3, Pos4, Color1 );
  RenderLine( Pos4, Pos1, Color1 );
}



void XBasicRenderer::RenderRect( const GR::tVector& Pos,
                                 const GR::tVector& Size,
                                 GR::u32 Color1 )
{
  RenderLine( Pos, Pos + GR::tVector( Size.x, 0.0f, 0.0f ), Color1 );
  RenderLine( Pos + GR::tVector( Size.x, 0.0f, 0.0f ),
              Pos + GR::tVector( Size.x, Size.y, 0.0f ),
              Color1 );
  RenderLine( Pos + GR::tVector( Size.x, Size.y, 0.0f ),
              Pos + GR::tVector( 0.0f, Size.y, 0.0f ),
              Color1 );
  RenderLine( Pos + GR::tVector( 0.0f, Size.y, 0.0f ),
              Pos,
              Color1 );
}



XFont* XBasicRenderer::CreateFontSquare()
{
#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
  XTexture*     pTexFont = CreateTexture( 256, 256, GR::Graphic::IF_A1R5G5B5 );

  if ( pTexFont == NULL )
  {
    return NULL;
  }

  XFont*   pFont = new XBasicFont( this );

  GR::Graphic::GDIPage     pageTemp;

  pageTemp.Create( m_hwndViewport, 256, 256, 16 );

  HDC       hdcTemp = pageTemp.GetDC();

  HFONT     tempFont = CreateFontA( 0, 0, 0, 0, 300, 0, 0, 0, 0, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH, "Tahoma" );

  HFONT oldFont = (HFONT)SelectObject( hdcTemp, tempFont );
  SetTextColor( hdcTemp, RGB( 255, 255, 255 ) );
  SetBkColor( hdcTemp, RGB( 0, 0, 0 ) );
  SetBkMode( hdcTemp, OPAQUE );
  SelectObject( hdcTemp, oldFont );
  DeleteObject( oldFont );

  for ( int i = 0; i < 256; ++i )
  {
    RECT    rc;
    char    cTemp[2];

    cTemp[0] = (char)i;
    cTemp[1] = 0;

    SetRect( &rc, ( i % 16 ) * 16, ( i / 16 ) * 16,
                  ( i % 16 ) * 16 + 16, ( i / 16 ) * 16 + 16 );

    TextOutA( hdcTemp, rc.left, rc.top, cTemp, 1 );
  }

  GR::Graphic::ImageData    imageData;

  imageData.Attach( 256, 256, 512, GR::Graphic::IF_X1R5G5B5, pageTemp.GetData() );
  imageData.ConvertSelfTo( GR::Graphic::IF_A1R5G5B5, 0, true, 0x00000000 );

  ( (XBasicFont*)pFont )->m_TransparentColor = 0x00000000;
  CopyDataToTexture( pTexFont, imageData );

  pFont->FontFromTexture( XFont::FLF_SQUARED_ONE_FONT | XFont::FLF_ALPHA_BIT, pTexFont, &imageData );

  pageTemp.ReleaseDC();

  AddFont( pFont );

  return pFont;
#endif
  return NULL;
}




void XBasicRenderer::RenderTriangle( const GR::tVector& Pos1,
                                     const GR::tVector& Pos2,
                                     const GR::tVector& Pos3,
                                     GR::u32 Color1, GR::u32 Color2, GR::u32 Color3 )
{
  RenderTriangle( Pos1, Pos2, Pos3,
                  0.0f, 0.0f,
                  0.0f, 0.0f,
                  0.0f, 0.0f,
                  Color1, Color2, Color3 );
}



void XBasicRenderer::RenderTriangle2d( const GR::tPoint& Pos1,
                                       const GR::tPoint& Pos2,
                                       const GR::tPoint& Pos3,
                                       GR::u32 Color1, GR::u32 Color2, GR::u32 Color3,
                                       float Z )
{
  RenderTriangle2d( Pos1, Pos2, Pos3,
                    0.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 0.0f,
                    Color1, Color2, Color3,
                    Z );
}



XVertexBuffer* XBasicRenderer::CreateVertexBuffer( XVertexBuffer* pCloneSource )
{
  if ( pCloneSource == NULL )
  {
    return NULL;
  }

  XVertexBuffer*    pCopy = CreateVertexBuffer( pCloneSource->PrimitiveCount(),
                                                pCloneSource->VertexFormat(),
                                                pCloneSource->Type() );

  if ( pCopy == NULL )
  {
    return NULL;
  }
  pCopy->CloneFrom( pCloneSource );
  return pCopy;
}



XFont* XBasicRenderer::LoadFontSquare( const char* FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  XFont*   pFont = new XBasicFont( this, m_pEnvironment );

  pFont->LoadFontSquare( FileName, Flags, TransparentColor );

  AddFont( pFont );

  return pFont;
}



XFont* XBasicRenderer::LoadFont( const char* FileName, GR::u32 TransparentColor )
{
  XFont*    pFont = new XBasicFont( this, m_pEnvironment );

  if ( !pFont->LoadFont( FileName, 0, TransparentColor ) )
  {
    delete pFont;
    return NULL;
  }
  AddFont( pFont );
  return pFont;
}



void XBasicRenderer::LogDirect( const GR::String& System, const GR::String& Text )
{
  if ( m_pDebugger )
  {
    m_pDebugger->LogDirect( System.c_str(), Text.c_str() );
  }
}



void XBasicRenderer::Log( const GR::String& System, const char* Format, ... )
{
  if ( m_pDebugger )
  {
    static char    miscBuffer[5000];
#if OPERATING_SYSTEM == OS_WINDOWS
    vsprintf_s( miscBuffer, 5000, Format, (char *)( &Format + 1 ) );
#elif ( OPERATING_SYSTEM == OS_ANDROID )
    va_list args;
    va_start( args, Format );
    vsprintf( miscBuffer, Format, args );
    va_end( args );
#endif

    m_pDebugger->LogDirect( System.c_str(), miscBuffer );
  }
}



void XBasicRenderer::SortAndCleanDisplayModes()
{
  std::map<GR::String,XRendererDisplayMode>    sortedModes;


  tListDisplayModes     cleanList;


  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&     mode( *it );

    GR::String     key = Misc::Format( "%01:8%_%02:8%_%03:8%" ) << mode.Width << mode.Height << mode.ImageFormat;

    sortedModes[key] = mode;

    ++it;
  }

  m_DisplayModes.clear();

  std::map<GR::String,XRendererDisplayMode>::iterator    itM( sortedModes.begin() );
  while ( itM != sortedModes.end() )
  {
    m_DisplayModes.push_back( itM->second );

    ++itM;
  }
}



size_t XBasicRenderer::NumberDisplayModes()
{
  return m_DisplayModes.size();
}



bool XBasicRenderer::ListDisplayModes( XRendererDisplayMode* pModeList, size_t SizeOfBuffer )
{
  size_t      sizeUsed = 0;

  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    if ( ( sizeUsed + sizeof( XRendererDisplayMode ) ) > SizeOfBuffer )
    {
      return false;
    }
    memcpy( pModeList, &( *it ), sizeof( XRendererDisplayMode ) );
    ++pModeList;
    sizeUsed += sizeof( XRendererDisplayMode );

    ++it;
  }
  return true;
}



GR::Graphic::ImageData* XBasicRenderer::LoadAndConvert( const char* FileName, GR::Graphic::eImageFormat FormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor )
{
  if ( m_pEnvironment == NULL )
  {
    return NULL;
  }
  ImageFormatManager*    pManager = (ImageFormatManager*)m_pEnvironment->Service( "ImageLoader" );
  if ( pManager == NULL )
  {
    return NULL;
  }
  GR::Graphic::ImageData*   pData = pManager->LoadData( FileName );
  if ( pData == NULL )
  {
    return NULL;
  }

  // convert format
  if ( FormatToConvert != GR::Graphic::IF_UNKNOWN )
  {
    pData->ConvertSelfTo( FormatToConvert, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
  }

  if ( ColorKey )
  {
    // bei Color-Key soll Alpha mit rein
    if ( pData->BitsProPixel() <= 16 )
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A1R5G5B5 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A1R5G5B5, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
      else if ( IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A8R8G8B8, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
    }
    else
    {
      if ( IsTextureFormatOK( GR::Graphic::IF_A8R8G8B8 ) )
      {
        pData->ConvertSelfTo( GR::Graphic::IF_A8R8G8B8, 0, ColorKey != 0, ColorKey, 0, 0, 0, 0, ColorKeyReplacementColor );
      }
    }
  }
  return pData;
}



GR::tFPoint XBasicRenderer::DirectTexelMappingOffset()
{
  return m_DirectTexelMappingOffset;
}



GR::String XBasicRenderer::AppPath( const GR::String& Path )
{
  if ( m_pEnvironment == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  GR::Gamebase::IValueStorage* pStorage = (GR::Gamebase::IValueStorage*)m_pEnvironment->Service( "ValueStorage" );
  if ( pStorage == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  return pStorage->AppPath( Path.c_str() );

  /*
  IFileLocator*   pLocator = (IFileLocator*)m_pEnvironment->Service( "FileLocator" );
  if ( pLocator == NULL )
  {
    return CMisc::AppPath( Path.c_str() );
  }
  return pLocator->LocateFile( Path.c_str() );*/
}



void XBasicRenderer::Offset( const GR::tPoint& NewOffset )
{
  m_DisplayOffset = NewOffset;
}



GR::tPoint XBasicRenderer::Offset() const
{
  return m_DisplayOffset;
}



bool XBasicRenderer::SaveScreenShot( const GR::String& FileName )
{
  return SaveScreenShot( FileName.c_str() );
}



XTexture* XBasicRenderer::LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, const GR::u32 MipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  return LoadTexture( FileName.c_str(), imgFormatToConvert, ColorKey, MipMapLevels, ColorKeyReplacementColor );
}



XFont* XBasicRenderer::LoadFontSquare( const GR::String& FileName, GR::u32 Flags, GR::u32 TransparentColor )
{
  return LoadFontSquare( FileName.c_str(), Flags, TransparentColor );
}



XFont* XBasicRenderer::LoadFont( const GR::String& FileName, GR::u32 TransparentColor )
{
  return LoadFont( FileName.c_str(), TransparentColor );
}



void XBasicRenderer::RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::String& Text, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    RenderText2d( pFont, CenterX - pFont->TextLength( Text ) / 2, Y, Text, Color, Z );
  }
}



void XBasicRenderer::RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::Char* pText, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    RenderText2d( pFont, CenterX - pFont->TextLength( pText ) / 2, Y, pText, Color, Z );
  }
}



void XBasicRenderer::RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    RenderText2d( pFont, X - pFont->TextLength( Text ), Y, Text, Color, Z );
  }
}



void XBasicRenderer::RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::Char* pText, GR::u32 Color, float Z )
{
  if ( pFont )
  {
    RenderText2d( pFont, X - pFont->TextLength( pText ), Y, pText, Color, Z );
  }
}



void XBasicRenderer::Canvas( const GR::tRect& Canvas )
{
  m_Canvas = Canvas;
}



GR::tRect XBasicRenderer::Canvas() const
{
  return m_Canvas;
}



void XBasicRenderer::LoadImageSectionAssets()
{
  if ( m_pEnvironment == NULL )
  {
    return;
  }
  Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
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
      Log( "Renderer.General", CMisc::printf( "DX11Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
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
}



void XBasicRenderer::LoadFontAssets()
{
  if ( m_pEnvironment == NULL )
  {
    return;
  }
  Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
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
        Log( "Renderer.General", "DX11Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
      }
      delete pAsset;

      pAsset = new Xtreme::Asset::XAssetFont( pFont );
      pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
    }
    else
    {
      Log( "Renderer.General", "DX11Renderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
    }
  }
}



void XBasicRenderer::LoadMeshAssets()
{
  if ( m_pEnvironment == NULL )
  {
    return;
  }
  Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
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
        Log( "Renderer.General", "DX11Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
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
      Log( "Renderer.General", "DX11Renderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
    }
  }
}



void XBasicRenderer::LoadImageAssets()
{
  if ( m_pEnvironment == NULL )
  {
    return;
  }
  Xtreme::Asset::IAssetLoader* pLoader = ( Xtreme::Asset::IAssetLoader* )m_pEnvironment->Service( "AssetLoader" );
  if ( pLoader == NULL )
  {
    return;
  }
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
      Log( "Renderer.General", "DX11Renderer: Asset Image has no images!" );
    }
    else
    {
      XTexture* pTexture = LoadTexture( listImages.front().c_str(), ( GR::Graphic::eImageFormat )ForcedFormat, ColorKey, MipMaps, ColorKeyReplacementColor );

      if ( pTexture )
      {
        if ( pAsset == NULL )
        {
          //Log( "Renderer.General", CMisc::printf( "DX11Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
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
            Log( "Renderer.General", CMisc::printf( "DX11Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
          }
          else
          {
            CopyDataToTexture( pTexture, *pData, ColorKey, Level );
            delete pData;
          }

          ( (XTextureBase*)pTexture )->FileNames.push_back( strPath );

          ++Level;
          ++it;
        }
      }
      else
      {
        Log( "Renderer.General", CMisc::printf( "DX11Renderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
      }
    }
  }
}