#pragma once

#include <GR/GRTypes.h>

#include <list>
#include <map>

#include <Interface/IAssetLoader.h>

#include <math/matrix4.h>

#include "XRenderer.h"
#include "XVertexBuffer.h"
#include "XFont.h"
#include "XLight.h"
#include "XMaterial.h"
#include "XViewport.h"
#include "XCamera.h"
#include "XTextureSection.h"



class IDebugService;

namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



class XTextureBase;

class XBasicRenderer : public GR::Service::ServiceImpl<XRenderer>
{

  protected:

    typedef std::list<XTexture*>        tListTextures;
    typedef std::list<XVertexBuffer*>   tListVertexBuffers;
    typedef std::map<std::pair<eRenderState,GR::u32>,GR::u32>    tMapRenderStates;
    typedef std::list<XFont*>           tListFonts;
    typedef std::list<XRendererDisplayMode>    tListDisplayModes;

    math::matrix4               m_Transform[TT_LAST_ENTRY];

    XViewport                   m_ViewPort;

    GR::u32                     m_ClearColor;

    tListTextures               m_Textures;

    tListVertexBuffers          m_VertexBuffers;

    tListFonts                  m_Fonts;

    tMapRenderStates            m_RenderStates;

    XMaterial                   m_Material;

    XLight                      m_Light[8];

    tListDisplayModes           m_DisplayModes;

    XViewport                   m_VirtualViewport;

    GR::tFPoint                 m_DirectTexelMappingOffset;

#if OPERATING_SYSTEM == OS_WINDOWS
    // Window-spezifics
    HWND                        m_hwndViewport;
    HWND                        m_hwndWindowedModeParent;

    HMENU                       m_hMenu;
#endif

#if OPERATING_SUB_SYSTEM == OS_SUB_DESKTOP
    WINDOWPLACEMENT             m_wPlacement;
#endif

    GR::u32                     m_WindowedModeStyles;
    GR::u32                     m_WindowedModeExStyles;

    IDebugService*              m_pDebugger;

    GR::tPoint                  m_DisplayOffset;
    GR::tPoint                  m_VirtualSize;

    GR::tRect                   m_Canvas;


    void                      LogDirect( const GR::String& strSystem, const GR::String& strText );
    void                      Log( const GR::String& strSystem, const char* szFormat, ... );


    void                      SortAndCleanDisplayModes();

    GR::String            AppPath( const GR::String& Path );


    void                      LoadImageAssets();
    void                      LoadImageSectionAssets();
    void                      LoadFontAssets();
    void                      LoadMeshAssets();

    void                      UnloadAssets( Xtreme::Asset::eXAssetType Type );



  public:

    XBasicRenderer();
    virtual ~XBasicRenderer();


    virtual void              AddTexture( XTexture* pTexture );
    virtual void              AddVertexBuffer( XVertexBuffer* pVBBuffer );
    virtual void              AddFont( XFont* pFont );

    virtual void              ReleaseAllTextures();
    virtual void              DestroyTexture( XTexture* pTexture );
    virtual void              DestroyAllTextures();

    virtual void              RestoreAllTextures();
    virtual void              RestoreStates();

    virtual void              DestroyAllVertexBuffers();
    virtual void              ReleaseAllVertexBuffers();
    virtual void              RestoreAllVertexBuffers();

    virtual void              RestoreAllFonts();
    virtual void              DestroyFont( XFont* pFont );
    virtual void              DestroyAllFonts();


    virtual bool              Initialize( GR::u32 Width,
                                          GR::u32 Height,
                                          GR::u32 Depth,
                                          GR::u32 Flags,
                                          GR::IEnvironment& Environment ) = 0;

    virtual bool              Release() = 0;
    virtual void              ReleaseAssets();

    virtual bool              OnResized() = 0;

    virtual bool              IsReady() const = 0;

    virtual void              SetTransform( eTransformType tType, const math::matrix4& matTrix );

    virtual bool              BeginScene() = 0;
    virtual void              EndScene() = 0;
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL ) = 0;

    virtual bool              ToggleFullscreen() = 0;
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode ) = 0;
    virtual bool              IsFullscreen() = 0;

    virtual void              AdjustWindowForMode( bool bWindowed );

    virtual bool              SetState( eRenderState State, GR::u32 Value, GR::u32 Stage = 0 ) = 0;
    virtual bool              SetViewport( const XViewport& Viewport ) = 0;
    virtual bool              SetTrueViewport( const XViewport& Viewport ) = 0;
    virtual XViewport         Viewport();
    virtual XViewport         TrueViewport();

    virtual math::matrix4     Matrix( XRenderer::eTransformType ttType );

    virtual GR::u32           Width() = 0;
    virtual GR::u32           Height() = 0;

    virtual void              Clear( bool ClearColor = true, bool ClearZ = true ) = 0;

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 ) = 0;
    virtual XTexture*         LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual void              SetTexture( GR::u32 Stage, XTexture* pTexture ) = 0;
    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, const GR::u32 MipMapLevel = 0 ) = 0;

    // Lights
    virtual bool              SetLight( GR::u32 LightIndex, XLight& Light ) = 0;

    virtual bool              SetMaterial( const XMaterial& Material ) = 0;

    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type ) = 0;
    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type ) = 0;
    virtual XVertexBuffer*    CreateVertexBuffer( XVertexBuffer* pCloneSource );
    virtual XVertexBuffer*    CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD ) = 0;

    virtual void              DestroyVertexBuffer( XVertexBuffer* ) = 0;
    virtual bool              RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index = 0, GR::u32 Count = 0 ) = 0;

    virtual XFont*            LoadFontSquare( const char* FileName, GR::u32 Flags = XFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    virtual XFont*            LoadFontSquare( const GR::String& FileName, GR::u32 Flags = XFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 );
    virtual XFont*            LoadFont( const char* FileName, GR::u32 TransparentColor = 0xff000000 );
    virtual XFont*            LoadFont( const GR::String& FileName, GR::u32 TransparentColor = 0xff000000 );
    virtual XFont*            CreateFontSquare();

    virtual void              RenderText2d( XFont* pFont, int X, int Y, const GR::Char* Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    //virtual void              RenderText2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderText2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::Char* Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::Char* Text, GR::u32 Color = 0xffffffff, float Z = 0.0f );
    virtual void              RenderText2d( XFont* pFont, int X, int Y, const GR::Char* Text,
                                            float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f );
    virtual void              RenderText2d( XFont* pFont, int X, int Y, const GR::String& Text,
                                            float ScaleX, float ScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f );
    virtual void              RenderText( XFont* pFont, const GR::tVector& Pos,
                                          const GR::Char* Text,
                                          const GR::tVector& Scale,
                                          GR::u32 Color = 0xffffffff );
    virtual void              RenderText( XFont* pFont, const GR::tVector& Pos,
                                          const GR::String& Text,
                                          const GR::tVector& Scale,
                                          GR::u32 Color = 0xffffffff );

    // Math Helpers
    virtual void              TransformToScreen( GR::tVector&  );

    virtual GR::tVector&      Project( GR::tVector& Pos,
                                       const XViewport& viewPort,
                                       const math::matrix4& matProjection,
                                       const math::matrix4& matView,
                                       const math::matrix4& matWorld );
    virtual GR::tVector&      Unproject( GR::tVector& Pos,
                                         const XViewport& viewPort,
                                         const math::matrix4& matProjection,
                                         const math::matrix4& matView,
                                         const math::matrix4& matWorld );

    virtual void              CastRayFromScreen( int X, int Y,
                                                 GR::tVector& v1,
                                                 GR::tVector& v2,
                                                 float Z1Value = 0.1f );

    virtual void              RenderLine2d( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color1, GR::u32 Color2 = 0, float Z = 0.0f ) = 0;
    virtual void              RenderLine( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color1, GR::u32 Color2 = 0 ) = 0;

    virtual void              RenderRect2d( const GR::tPoint& Pos1, const GR::tPoint& Size, GR::u32 Color1, float Z = 0.0f );
    virtual void              RenderRect( const GR::tVector& Pos1,
                                          const GR::tVector& Pos2,
                                          const GR::tVector& Pos3,
                                          const GR::tVector& Pos4,
                                          GR::u32 Color1 );
    virtual void              RenderRect( const GR::tVector& Pos,
                                          const GR::tVector& Size,
                                          GR::u32 Color1 );

    virtual void              RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                            GR::f32 TU1, GR::f32 TV1,
                                            GR::f32 TU2, GR::f32 TV2,
                                            GR::f32 TU3, GR::f32 TV3,
                                            GR::f32 TU4, GR::f32 TV4,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f ) = 0;
    virtual void              RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f );
    virtual void              RenderQuadDetail2d( GR::f32 X, GR::f32 Y, GR::f32 Width, GR::f32 Height,
                                            GR::f32 TU1, GR::f32 TV1,
                                            GR::f32 TU2, GR::f32 TV2,
                                            GR::f32 TU3, GR::f32 TV3,
                                            GR::f32 TU4, GR::f32 TV4,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f ) = 0;
    virtual void              RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1,
                                                  GR::f32 X2, GR::f32 Y2,
                                                  GR::f32 X3, GR::f32 Y3,
                                                  GR::f32 X4, GR::f32 Y4,
                                                  GR::f32 TU1, GR::f32 TV1,
                                                  GR::f32 TU2, GR::f32 TV2,
                                                  GR::f32 TU3, GR::f32 TV3,
                                                  GR::f32 TU4, GR::f32 TV4,
                                                  GR::u32 Color1, GR::u32 Color2 = 0,
                                                  GR::u32 Color3 = 0, GR::u32 Color4 = 0, float Z = 0.0f ) = 0;

    virtual void              RenderQuad( const GR::tVector& ptPos,
                                          const GR::tVector& ptSize,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void              RenderQuad( const GR::tVector& ptPos,
                                          const GR::tVector& ptSize,
                                          GR::f32 TU1, GR::f32 TV1,
                                          GR::f32 TU2, GR::f32 TV2,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void              RenderQuad( const GR::tVector& ptPos1,
                                          const GR::tVector& ptPos2,
                                          const GR::tVector& ptPos3,
                                          const GR::tVector& ptPos4,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 );

    virtual void              RenderQuad( const GR::tVector& ptPos1,
                                          const GR::tVector& ptPos2,
                                          const GR::tVector& ptPos3,
                                          const GR::tVector& ptPos4,
                                          GR::f32 TU1, GR::f32 TV1,
                                          GR::f32 TU2, GR::f32 TV2,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderQuad( const GR::tVector& ptPos1,
                                          const GR::tVector& ptPos2,
                                          const GR::tVector& ptPos3,
                                          const GR::tVector& ptPos4,
                                          GR::f32 TU1, GR::f32 TV1,
                                          GR::f32 TU2, GR::f32 TV2,
                                          GR::f32 TU3, GR::f32 TV3,
                                          GR::f32 TU4, GR::f32 TV4,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderTriangle( const GR::tVector& pt1,
                                              const GR::tVector& pt2,
                                              const GR::tVector& pt3,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0 );
    virtual void              RenderTriangle( const GR::tVector& pt1,
                                              const GR::tVector& pt2,
                                              const GR::tVector& pt3,
                                              GR::f32 TU1, GR::f32 TV1,
                                              GR::f32 TU2, GR::f32 TV2,
                                              GR::f32 TU3, GR::f32 TV3,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0 ) = 0;

    virtual void              RenderTriangle2d( const GR::tPoint& pt1,
                                                const GR::tPoint& pt2,
                                                const GR::tPoint& pt3,
                                                GR::u32 Color1, GR::u32 Color2 = 0,
                                                GR::u32 Color3 = 0, float Z = 0.0f );
    virtual void              RenderTriangle2d( const GR::tPoint& pt1,
                                                const GR::tPoint& pt2,
                                                const GR::tPoint& pt3,
                                                GR::f32 TU1, GR::f32 TV1,
                                                GR::f32 TU2, GR::f32 TV2,
                                                GR::f32 TU3, GR::f32 TV3,
                                                GR::u32 Color1, GR::u32 Color2 = 0,
                                                GR::u32 Color3 = 0, float Z = 0.0f ) = 0;

    virtual void              RenderTextureSection( const GR::tVector& v1,
                                                    const GR::tVector& v2,
                                                    const GR::tVector& v3,
                                                    const GR::tVector& v4,
                                                    const XTextureSection& TexSection,
                                                    GR::u32 Color1, GR::u32 Color2 = 0,
                                                    GR::u32 Color3 = 0, GR::u32 Color4 = 0,
                                                    GR::u32 AlternativeFlags = -1 );
    virtual void              RenderTextureSection2d( GR::i32 X, GR::i32 Y,
                                                      const XTextureSection& TexSection,
                                                      GR::u32 Color = 0xffffffff,
                                                      GR::i32 Width = -1, GR::i32 Height = -1,
                                                      GR::u32 AlternativeFlags = -1, float Z = 0.0f );
    virtual void              RenderTextureSection2d( GR::i32 X, GR::i32 Y,
                                                      const XTextureSection& TexSection,
                                                      GR::u32 Color, GR::u32 Color2,
                                                      GR::u32 Color3, GR::u32 Color4,
                                                      GR::i32 Width = -1, GR::i32 Height = -1,
                                                      GR::u32 AlternativeFlags = -1, float Z = 0.0f );
    virtual void              RenderTextureSectionRotated2d( GR::i32 X, GR::i32 Y,
                                                             const XTextureSection& TexSection,
                                                             GR::f32 Angle, GR::f32 ZoomFactorX = 1.0f, GR::f32 ZoomFactorY = 1.0f,
                                                             GR::u32 Color = 0xffffffff, GR::u32 Color2 = 0,
                                                             GR::u32 Color3 = 0, GR::u32 Color4 = 0,
                                                             GR::i32 Width = -1, GR::i32 Height = -1,
                                                             GR::u32 AlternativeFlags = -1, float Z = 0.0f );


    virtual void              RenderBoundingBox( const XBoundingBox& BBox, GR::u32 Color );
    virtual void              RenderBoundingBox( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color );

    virtual void              RenderBox( const GR::tVector& Pos, const GR::tVector& Size, GR::u32 Color );

    virtual void              SetShader( eShaderType Type );

    virtual bool              SaveScreenShot( const char* FileName ) = 0;
    virtual bool              SaveScreenShot( const GR::String& FileName );

    virtual void              VirtualSize( const GR::tPoint& NewSize );
    virtual GR::tPoint        VirtualSize() const;


    virtual size_t            NumberDisplayModes();
    virtual bool              ListDisplayModes( XRendererDisplayMode* pModeList, size_t SizeOfBuffer );

    GR::Graphic::ImageData*   LoadAndConvert( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 ColorKey, GR::u32 ColorKeyReplacementColor );

    virtual GR::tFPoint       DirectTexelMappingOffset();

    virtual void              Offset( const GR::tPoint& NewOffset );
    GR::tPoint                Offset() const;

    // use for "letterboxing"
    virtual void              Canvas( const GR::tRect& Canvas );
    virtual GR::tRect         Canvas() const;


};
