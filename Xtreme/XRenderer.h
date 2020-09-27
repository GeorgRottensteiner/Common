#pragma once

#include <GR/GRTypes.h>

#include <Math/matrix4.h>

#include <Lang/Service.h>

#include "XDisplayMode.h"
#include "XTexture.h"
#include "XFont.h"
#include "XLight.h"
#include "XMaterial.h"
#include "XViewport.h"
#include "XCamera.h"
#include "XTextureSection.h"
#include "XVertexBuffer.h"



namespace GR
{
  namespace Graphic
  {
    class ImageData;
  }
}



class XRenderer : public GR::IService
{

  public:

    enum eInitialisationFlags
    {
      IN_FULLSCREEN = 0x00000001,
      IN_VSYNC      = 0x00000002,
    };

    enum eShaderType
    {
      ST_INVALID      = 0,
      ST_FLAT,
      ST_FLAT_NO_TEXTURE,
      ST_ALPHA_TEST,
      ST_ALPHA_BLEND,
      ST_50_PERCENT_BLEND,
      ST_ALPHA_BLEND_AND_TEST,
      ST_ADDITIVE,
      ST_ALPHA_TEST_COLOR_FROM_DIFFUSE
    };

    enum eRenderState
    {
      RS_ZBUFFER,
      RS_ZWRITE,
      RS_LIGHTING,
      RS_ALPHABLENDING,
      RS_ALPHATEST,
      RS_SRC_BLEND,
      RS_DEST_BLEND,
      RS_COLOR_OP,
      RS_COLOR_ARG_1,
      RS_COLOR_ARG_2,
      RS_ALPHA_OP,
      RS_ALPHA_ARG_1,
      RS_ALPHA_ARG_2,
      RS_CULLMODE,
      RS_ALPHAREF,
      RS_ALPHAFUNC,
      RS_AMBIENT,
      RS_TEXTURE_FACTOR,

      RS_FILL_MODE,
      RS_SHADE_MODE,

      RS_NORMALIZE_NORMALS,

      RS_CLEAR_COLOR,

      RS_MINFILTER,
      RS_MAGFILTER,
      RS_MIPFILTER,

      RS_LIGHT,

      RS_SPECULAR,
      RS_DITHERING,

      RS_ADDRESSU,
      RS_ADDRESSV,

      RS_DIFFUSE_MATERIAL_SOURCE,
      RS_AMBIENT_MATERIAL_SOURCE,
      RS_SPECULAR_MATERIAL_SOURCE,
      RS_EMISSIVE_MATERIAL_SOURCE,

      RS_TEXTURE_TRANSFORM,

      RS_COLORMASK,

      RS_FOG_COLOR,
      RS_FOG_START,
      RS_FOG_END,
      RS_FOG_ENABLE,
      RS_FOG_DENSITY,
      RS_FOG_TABLEMODE,
      RS_FOG_VERTEXMODE,

      RS_SWAP_MODE
    };

    enum eRenderStateValue
    {
      RSV_ENABLE,
      RSV_DISABLE,
      RSV_ONE,
      RSV_ZERO,
      RSV_SRC_ALPHA,
      RSV_INV_SRC_ALPHA,

      RSV_CULL_NONE,
      RSV_CULL_CCW,
      RSV_CULL_CW,

      RSV_CURRENT,
      RSV_DIFFUSE,
      RSV_TEXTURE,
      RSV_MODULATE,
      RSV_SELECT_ARG_1,
      RSV_SELECT_ARG_2,
      RSV_TEXTURE_FACTOR,
      RSV_BLENDFACTORALPHA,

      RSV_COMPARE_GREATEREQUAL,
      RSV_COMPARE_LESSEQUAL,
      RSV_COMPARE_EQUAL,
      RSV_COMPARE_GREATER,
      RSV_COMPARE_NOTEQUAL,
      RSV_COMPARE_LESS,
      RSV_COMPARE_NEVER,
      RSV_COMPARE_ALWAYS,

      // Filter
      RSV_FILTER_POINT,
      RSV_FILTER_LINEAR,

      RSV_ADDRESS_WRAP,
      RSV_ADDRESS_CLAMP,

      // Texture Transform
      RSV_TEXTURE_TRANSFORM_COUNT2,

      // Color-Material-Source
      RSV_CMS_MATERIAL,
      RSV_CMS_COLOR_DIFFUSE,
      RSV_CMS_COLOR_SPECULAR,

      // Fill-Mode
      RSV_FILL_SOLID,
      RSV_FILL_WIREFRAME,
      RSV_FILL_POINTS,

      // Shade-Mode
      RSV_SHADE_FLAT,
      RSV_SHADE_GOURAUD,

      // Fog-Flags
      RSV_FOG_NONE,
      RSV_FOG_EXP,
      RSV_FOG_EXP_SQUARED,
      RSV_FOG_LINEAR,

      // Colorwrite-Flags
      RSV_COLORMASK_RED   = 0x00000001,
      RSV_COLORMASK_GREEN = 0x00000002,
      RSV_COLORMASK_BLUE  = 0x00000004,
      RSV_COLORMASK_ALPHA = 0x00000008,

      // Swap Modes
      RSV_SWAP_DISCARD    = 0x00000001,
      RSV_SWAP_FLIP       = 0x00000002,
      RSV_SWAP_COPY       = 0x00000003,
      RSV_SWAP_COPY_VSYNC = 0x00000004
    };

    enum eTransformType
    {
      TT_WORLD,
      TT_PROJECTION,
      TT_VIEW,
      TT_TEXTURE_STAGE_0,
      TT_TEXTURE_STAGE_1,

      TT_LAST_ENTRY,
    };


  public:

    XRenderer()
    {
    }

    virtual ~XRenderer()
    {
    }


    virtual void              AddTexture( XTexture* pTexture ) = 0;
    virtual void              AddVertexBuffer( XVertexBuffer* pVBBuffer ) = 0;
    virtual void              AddFont( XFont* pFont ) = 0;

    virtual void              ReleaseAllTextures() = 0;
    virtual void              DestroyTexture( XTexture* pTexture ) = 0;
    virtual void              DestroyAllTextures() = 0;

    virtual void              RestoreAllTextures() = 0;
    virtual void              RestoreStates() = 0;

    virtual void              DestroyAllVertexBuffers() = 0;
    virtual void              ReleaseAllVertexBuffers() = 0;
    virtual void              RestoreAllVertexBuffers() = 0;

    virtual void              RestoreAllFonts() = 0;

    virtual void              DestroyFont( XFont* pFont ) = 0;
    virtual void              DestroyAllFonts() = 0;


    virtual bool              Initialize( GR::u32 Width,
                                          GR::u32 Height,
                                          GR::u32 Depth,
                                          GR::u32 Flags,
                                          GR::IEnvironment& Environment ) = 0;

    virtual bool              Release() = 0;

    virtual void              ReleaseAssets() = 0;

    virtual bool              OnResized() = 0;

    virtual bool              IsReady() const = 0;

    virtual void              SetTransform( eTransformType tType, const math::matrix4& matTrix ) = 0;

    virtual bool              BeginScene() = 0;
    virtual void              EndScene() = 0;
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL ) = 0;

    virtual bool              ToggleFullscreen() = 0;
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode ) = 0;
    virtual bool              IsFullscreen() = 0;

    virtual bool              VSyncEnabled() = 0;
    virtual void              EnableVSync( bool Enable = true ) = 0;

    virtual void              AdjustWindowForMode( bool bWindowed ) = 0;

    virtual bool              SetState( eRenderState rState, GR::u32 rValue, GR::u32 Stage = 0 ) = 0;
    virtual bool              SetViewport( const XViewport& Viewport ) = 0;
    virtual bool              SetTrueViewport( const XViewport& Viewport ) = 0;
    virtual XViewport         Viewport() = 0;
    virtual XViewport         TrueViewport() = 0;


    virtual math::matrix4     Matrix( XRenderer::eTransformType ttType ) = 0;

    virtual GR::u32           Width() = 0;
    virtual GR::u32           Height() = 0;
    virtual GR::Graphic::eImageFormat   ImageFormat() = 0;

    virtual void              Clear( bool bClearColor = true, bool bClearZ = true ) = 0;

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat imgFormat, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false ) = 0;
    virtual XTexture*         LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 ) = 0;
    virtual XTexture*         LoadTexture( const GR::String& FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 ) = 0;
    virtual void              SetTexture( GR::u32 Stage, XTexture* pTexture ) = 0;
    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, const GR::u32 MipMapLevel = 0 ) = 0;
    virtual bool              ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel = 0 ) = 0;

    // Lights
    virtual bool              SetLight( GR::u32 LightIndex, XLight& Light ) = 0;

    virtual bool              SetMaterial( const XMaterial& Material ) = 0;

    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type ) = 0;
    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type ) = 0;
    virtual XVertexBuffer*    CreateVertexBuffer( XVertexBuffer* pCloneSource ) = 0;
    virtual XVertexBuffer*    CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD ) = 0;

    virtual void              DestroyVertexBuffer( XVertexBuffer* ) = 0;
    virtual bool              RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 Index = 0, GR::u32 Count = 0 ) = 0;
    virtual bool              RenderMesh( const Mesh::IMesh& Mesh ) = 0;

    virtual XFont*            LoadFontSquare( const char* FileName, GR::u32 Flags = XFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual XFont*            LoadFontSquare( const GR::String& FileName, GR::u32 Flags = XFont::FLF_SQUARED_ONE_FONT, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual XFont*            LoadFont( const char* FileName, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual XFont*            LoadFont( const GR::String& FileName, GR::u32 TransparentColor = 0xff000000 ) = 0;
    virtual XFont*            CreateFontSquare() = 0;

    virtual void              RenderText2d( XFont* pFont, int iX, int iY, const GR::Char* Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;
    virtual void              RenderText2d( XFont* pFont, int iX, int iY, const GR::String& Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;

    virtual void              RenderText2d( XFont* pFont, int iX, int iY, const GR::Char* Text,
                                            float fScaleX, float fScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderText2d( XFont* pFont, int iX, int iY, const GR::String& Text,
                                            float fScaleX, float fScaleY, GR::u32 Color1, GR::u32 Color2 = 0, GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;

    virtual void              RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;
    virtual void              RenderTextCentered2d( XFont* pFont, int CenterX, int Y, const GR::Char* Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;
    virtual void              RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::String& Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;
    virtual void              RenderTextRightAligned2d( XFont* pFont, int X, int Y, const GR::Char* Text, GR::u32 Color = 0xffffffff, float fZ = 0.0f ) = 0;


    virtual void              RenderText( XFont* pFont, const GR::tVector& vectPos,
                                          const GR::Char* Text,
                                          const GR::tVector& vectScale,
                                          GR::u32 Color = 0xffffffff ) = 0;

    virtual void              RenderText( XFont* pFont, const GR::tVector& vectPos,
                                          const GR::String& Text,
                                          const GR::tVector& vectScale,
                                          GR::u32 Color = 0xffffffff ) = 0;

    // Math Helpers
    virtual void              TransformToScreen( GR::tVector& vect ) = 0;

    virtual GR::tVector&      Project( GR::tVector& vect,
                                       const XViewport& viewPort,
                                       const math::matrix4& matProjection,
                                       const math::matrix4& matView,
                                       const math::matrix4& matWorld ) = 0;
    virtual GR::tVector&      Unproject( GR::tVector& vect,
                                         const XViewport& viewPort,
                                         const math::matrix4& matProjection,
                                         const math::matrix4& matView,
                                         const math::matrix4& matWorld ) = 0;

    virtual void              CastRayFromScreen( int iX, int iY,
                                                 GR::tVector& v1,
                                                 GR::tVector& v2,
                                                 float fZ1Value = 0.1f ) = 0;

    virtual void              RenderLine2d( const GR::tPoint& Point1, const GR::tPoint& Point2, GR::u32 Color1, GR::u32 Color2 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 Color1, GR::u32 Color2 = 0 ) = 0;

    virtual void              RenderRect2d( const GR::tPoint& Point1, const GR::tPoint& Size, GR::u32 Color1, float fZ = 0.0f ) = 0;
    virtual void              RenderRect( const GR::tVector& vect1,
                                          const GR::tVector& vect2,
                                          const GR::tVector& vect3,
                                          const GR::tVector& vect4,
                                          GR::u32 Color1 ) = 0;
    virtual void              RenderRect( const GR::tVector& vectPos,
                                          const GR::tVector& vectSize,
                                          GR::u32 Color1 ) = 0;
    virtual void              RenderTriangle( const GR::tVector& Point1,
                                              const GR::tVector& Point2,
                                              const GR::tVector& Point3,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0 ) = 0;
    virtual void              RenderTriangle( const GR::tVector& Point1,
                                              const GR::tVector& Point2,
                                              const GR::tVector& Point3,
                                              GR::f32 fTU1, GR::f32 fTV1,
                                              GR::f32 fTU2, GR::f32 fTV2,
                                              GR::f32 fTU3, GR::f32 fTV3,
                                              GR::u32 Color1, GR::u32 Color2 = 0,
                                              GR::u32 Color3 = 0 ) = 0;

    virtual void              RenderTriangle2d( const GR::tPoint& Point1,
                                                const GR::tPoint& Point2,
                                                const GR::tPoint& Point3,
                                                GR::u32 Color1, GR::u32 Color2 = 0,
                                                GR::u32 Color3 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderTriangle2d( const GR::tPoint& Point1,
                                                const GR::tPoint& Point2,
                                                const GR::tPoint& Point3,
                                                GR::f32 fTU1, GR::f32 fTV1,
                                                GR::f32 fTU2, GR::f32 fTV2,
                                                GR::f32 fTU3, GR::f32 fTV3,
                                                GR::u32 Color1, GR::u32 Color2 = 0,
                                                GR::u32 Color3 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                            GR::f32 fTU1, GR::f32 fTV1,
                                            GR::f32 fTU2, GR::f32 fTV2,
                                            GR::f32 fTU3, GR::f32 fTV3,
                                            GR::f32 fTU4, GR::f32 fTV4,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                            GR::f32 fTU1, GR::f32 fTV1,
                                            GR::f32 fTU2, GR::f32 fTV2,
                                            GR::f32 fTU3, GR::f32 fTV3,
                                            GR::f32 fTU4, GR::f32 fTV4,
                                            GR::u32 Color1, GR::u32 Color2 = 0,
                                            GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;
    virtual void              RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
                                                  GR::f32 fX2, GR::f32 fY2,
                                                  GR::f32 fX3, GR::f32 fY3,
                                                  GR::f32 fX4, GR::f32 fY4,
                                                  GR::f32 fTU1, GR::f32 fTV1,
                                                  GR::f32 fTU2, GR::f32 fTV2,
                                                  GR::f32 fTU3, GR::f32 fTV3,
                                                  GR::f32 fTU4, GR::f32 fTV4,
                                                  GR::u32 Color1, GR::u32 Color2 = 0,
                                                  GR::u32 Color3 = 0, GR::u32 Color4 = 0, float fZ = 0.0f ) = 0;

    virtual void              RenderQuad( const GR::tVector& Pos1,
                                          const GR::tVector& Pos2,
                                          const GR::tVector& Pos3,
                                          const GR::tVector& Pos4,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderQuad( const GR::tVector& Pos1,
                                          const GR::tVector& Pos2,
                                          const GR::tVector& Pos3,
                                          const GR::tVector& Pos4,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderQuad( const GR::tVector& Pos,
                                          const GR::tVector& Size,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderQuad( const GR::tVector& Pos,
                                          const GR::tVector& Size,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderQuad( const GR::tVector& Pos1,
                                          const GR::tVector& Pos2,
                                          const GR::tVector& Pos3,
                                          const GR::tVector& Pos4,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::f32 fTU3, GR::f32 fTV3,
                                          GR::f32 fTU4, GR::f32 fTV4,
                                          GR::u32 Color1, GR::u32 Color2 = 0,
                                          GR::u32 Color3 = 0, GR::u32 Color4 = 0 ) = 0;

    virtual void              RenderTextureSection( const GR::tVector& v1,
                                                    const GR::tVector& v2,
                                                    const GR::tVector& v3,
                                                    const GR::tVector& v4,
                                                    const XTextureSection& TexSection,
                                                    GR::u32 Color1, GR::u32 Color2 = 0,
                                                    GR::u32 Color3 = 0, GR::u32 Color4 = 0,
                                                    GR::u32 AlternativeFlags = -1 ) = 0;
    virtual void              RenderTextureSection2d( GR::i32 iX, GR::i32 iY,
                                                      const XTextureSection& TexSection,
                                                      GR::u32 Color = 0xffffffff,
                                                      GR::i32 iWidth = -1, GR::i32 iHeight = -1,
                                                      GR::u32 AlternativeFlags = -1, float fZ = 0.0f ) = 0;
    virtual void              RenderTextureSection2d( GR::i32 iX, GR::i32 iY,
                                                      const XTextureSection& TexSection,
                                                      GR::u32 Color, GR::u32 Color2,
                                                      GR::u32 Color3, GR::u32 Color4,
                                                      GR::i32 iWidth = -1, GR::i32 iHeight = -1,
                                                      GR::u32 AlternativeFlags = -1, float fZ = 0.0f ) = 0;
    virtual void              RenderTextureSectionRotated2d( GR::i32 iX, GR::i32 iY,
                                                             const XTextureSection& TexSection,
                                                             GR::f32 Angle, GR::f32 ZoomFactorX = 1.0f, GR::f32 ZoomFactorY = 1.0f,
                                                             GR::u32 Color = 0xffffffff, GR::u32 Color2 = 0,
                                                             GR::u32 Color3 = 0, GR::u32 Color4 = 0, 
                                                             GR::i32 iWidth = -1, GR::i32 iHeight = -1,
                                                             GR::u32 AlternativeFlags = -1, float fZ = 0.0f ) = 0;


    virtual void              RenderBoundingBox( const XBoundingBox& BBox, GR::u32 Color ) = 0;
    virtual void              RenderBoundingBox( const GR::tVector& v1, const GR::tVector& v2, GR::u32 Color ) = 0;

    virtual void              RenderBox( const GR::tVector& vPos, const GR::tVector& vSize, GR::u32 Color ) = 0;

    virtual void              SetShader( eShaderType sType ) = 0;

    virtual bool              SaveScreenShot( const char* FileName ) = 0;
    virtual bool              SaveScreenShot( const GR::String& FileName ) = 0;

    // Set NULL to return to previous back buffer
    virtual void              SetRenderTarget( XTexture* pTexture ) = 0;


    virtual size_t            NumberDisplayModes() = 0;
    virtual bool              ListDisplayModes( XRendererDisplayMode* pModeList, size_t SizeOfBuffer ) = 0;

    virtual GR::tFPoint       DirectTexelMappingOffset() = 0;

    virtual void              Offset( const GR::tPoint& NewOffset ) = 0;
    virtual GR::tPoint        Offset() const = 0;

    virtual void              VirtualSize( const GR::tPoint& NewSize ) = 0;
    virtual GR::tPoint        VirtualSize() const = 0;

    // use for "letterboxing"
    virtual void              Canvas( const GR::tRect& Canvas ) = 0;
    virtual GR::tRect         Canvas() const = 0;


};
