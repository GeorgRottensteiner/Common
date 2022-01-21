#pragma once


#if ( OPERATING_SYSTEM == OS_WEB )
#define GL_GLEXT_PROTOTYPES
#include <emscripten.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

const GLubyte* APIENTRY gluErrorString( GLenum   errCode );
#elif OPERATING_SUB_SYSTEM == OS_SUB_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>
#include <gl\glu.h>
#else
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <gl\wglext.h>
#endif



#include <Xtreme/XBasicRenderer.h>

#include "QuadCache.h"
#include "LineCache.h"
#include "TriangleCache.h"


class OpenGLShaderTexture;



namespace InternalShader
{
  enum Value
  {
    POSITION_COLOR,
    POSITIONRHW_COLOR,
    POSITIONRHW_COLOR_ALPHATEST,
    POSITION_TEXTURE_COLOR,
    POSITIONRHW_TEXTURE_COLOR,
    POSITIONRHW_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED,
    POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT,
    POSITIONRHW_TEXTURE_COLOR_NORMAL_ONELIGHT,
    POSITIONRHW_TEXTURE_COLOR_NORMAL_TWOLIGHTS,

    POSITIONRHW_TEXTURE_COLOR_ALPHATEST,
    POSITIONRHW_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST,
    POSITIONRHW_NOTEXTURE_COLOR,
    POSITIONRHW_TEXTURE_COLOR_ALPHATEST_COLOR_FROM_DIFFUSE,

    POSITION_NOTEXTURE_COLOR_NORMAL_LIGHTSDISABLED,
    POSITION_NOTEXTURE_COLOR_NORMAL_NOLIGHT,
    POSITION_NOTEXTURE_COLOR_NORMAL_ONELIGHT,
    POSITION_NOTEXTURE_COLOR_NORMAL_TWOLIGHTS,
    POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED,
    POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT,
    POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT,
    POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS,

    POSITION_TEXTURE_COLOR_NORMAL_LIGHTSDISABLED_ALPHATEST,
    POSITION_TEXTURE_COLOR_NORMAL_NOLIGHT_ALPHATEST,
    POSITION_TEXTURE_COLOR_NORMAL_ONELIGHT_ALPHATEST,
    POSITION_TEXTURE_COLOR_NORMAL_TWOLIGHTS_ALPHATEST
  };
}



class OpenGLShaderRenderClass : public XBasicRenderer
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
                 && ( G == RHS.G )
                 && ( B == RHS.B )
                 && ( A == RHS.A ) );
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

    ModelViewProjectionConstantBuffer     m_Matrices;
    LightsConstantBuffer                  m_LightInfo;
    MaterialConstantBuffer                m_Material;
    FogConstantBuffer                     m_Fog;


    QuadCache                     m_QuadCache;
    QuadCache                     m_QuadCache3d;
    LineCache                     m_LineCache;
    LineCache                     m_LineCache3d;
    TriangleCache                 m_TriangleCache;
    TriangleCache                 m_TriangleCache3d;

    GLuint                        m_UBOGlobalMatrices;
    GLuint                        m_UBOLights;
    GLuint                        m_UBOMaterial;
    GLuint                        m_UBOFog;

    GLuint                        m_UBOGlobalMatricesBindingID;
    GLuint                        m_UBOLightsBindingID;
    GLuint                        m_UBOMaterialBindingID;
    GLuint                        m_UBOFogBindingID;

    XLight                        m_SetLights[8];
    bool                          m_LightEnabled[8];
    bool                          m_LightingEnabled;

    bool                          m_LightsChanged;

    math::matrix4                 m_StoredTextureTransform;


#if OPERATING_SUB_SYSTEM == OS_SUB_SDL
    SDL_GLContext                 m_GLContext;
    SDL_Window*                   m_pMainWindow;
#else
    HGLRC                         m_RC;

    HDC                           m_DC;

    PIXELFORMATDESCRIPTOR         m_pfd;
#endif

    bool                          m_FullScreen;

    GR::u32                       m_Width,
                                  m_Height,
                                  m_OldPixelFormat,
                                  m_Depth;

    GR::f32                       m_AlphaRefValue;
    GR::u32                       m_AlphaFuncValue;
    GR::u32                       m_SrcBlend;
    GR::u32                       m_DestBlend;

    bool                          m_Ready;
    XTexture*                     m_pSetRenderTargetTexture;
    GR::tPoint                    m_SetRenderTargetTextureSize;

    XTexture*                     m_pSetTextures[8];

    eShaderType                   m_CurrentShaderType;

    bool                          m_SupportsVBO;
    bool                          m_2dModeActive;

    GR::u32                       m_NumActiveLights;

    std::map<int, GLuint>         m_StoredShaderPrograms;

    bool                          InitShader( InternalShader::Value Program, const GR::String& VertexShader, const GR::String& PixelShader );
    void                          SetGLShaderProgram( InternalShader::Value Program );



  public:


    OpenGLShaderRenderClass();
    virtual ~OpenGLShaderRenderClass();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual void                  ReleaseAssets();

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::Graphic::eImageFormat   ImageFormat();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  VSyncEnabled();
    virtual void                  EnableVSync( bool Enable );

    virtual bool                  SaveScreenShot( const char* FileName );

    virtual bool                  IsReady() const;

    virtual void                  Clear( bool ClearColor = true, bool ClearZ = true );

    virtual bool                  SetState( eRenderState State, GR::u32 Value, GR::u32 Stage = 0 );
    virtual void                  SetTransform( eTransformType Type, const math::matrix4& Matrix );

    virtual bool                  SetViewport( const XViewport& Viewport );
    virtual bool                  SetTrueViewport( const XViewport& Viewport );

    virtual XTexture*             LoadTexture( const char* zFileName, GR::Graphic::eImageFormat FormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 ColorKey = 0, const GR::u32 MipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual XTexture*             CreateTexture( const GR::u32 Width, const GR::u32 Height, const GR::Graphic::eImageFormat Format, const GR::u32 MipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
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

    virtual void                  RenderLine2d( const GR::tPoint& Pos1, const GR::tPoint& Pos2, GR::u32 Color1, GR::u32 Color2 = 0, float Z = 0.0f );
    virtual void                  RenderLine( const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color1, GR::u32 Color2 = 0 );

    virtual void                  RenderQuad2d( GR::i32 X, GR::i32 Y, GR::i32 Width, GR::i32 Height,
                                                GR::f32 TU1, GR::f32 TV1,
                                                GR::f32 TU2, GR::f32 TV2,
                                                GR::f32 TU3, GR::f32 TV3,
                                                GR::f32 TU4, GR::f32 TV4,
                                                GR::u32 Color1, GR::u32 Color2, 
                                                GR::u32 Color3, GR::u32 Color4, float Z = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 X, GR::f32 Y, GR::f32 Width, GR::f32 Height,
                                          GR::f32 TU1, GR::f32 TV1,
                                          GR::f32 TU2, GR::f32 TV2,
                                          GR::f32 TU3, GR::f32 TV3,
                                          GR::f32 TU4, GR::f32 TV4,
                                          GR::u32 Color1, GR::u32 Color2, 
                                          GR::u32 Color3, GR::u32 Color4, float Z = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 X1, GR::f32 Y1, 
                                                  GR::f32 X2, GR::f32 Y2, 
                                                  GR::f32 X3, GR::f32 Y3, 
                                                  GR::f32 X4, GR::f32 Y4, 
                                                  GR::f32 TU1, GR::f32 TV1,
                                                  GR::f32 TU2, GR::f32 TV2,
                                                  GR::f32 TU3, GR::f32 TV3,
                                                  GR::f32 TU4, GR::f32 TV4,
                                                  GR::u32 Color1, GR::u32 Color2, 
                                                  GR::u32 Color3, GR::u32 Color4, float Z = 0.0f );
    virtual void                  RenderQuad( const GR::tVector& ptPos1,
                                              const GR::tVector& ptPos2,
                                              const GR::tVector& ptPos3,
                                              const GR::tVector& ptPos4,
                                              GR::f32 TU1, GR::f32 TV1,
                                              GR::f32 TU2, GR::f32 TV2,
                                              GR::u32 Color1, GR::u32 Color2, 
                                              GR::u32 Color3, GR::u32 Color4 );
    virtual void                  RenderQuad( const GR::tVector& Pos1,
                                              const GR::tVector& Pos2,
                                              const GR::tVector& Pos3,
                                              const GR::tVector& Pos4,
                                              GR::f32 TU1, GR::f32 TV1,
                                              GR::f32 TU2, GR::f32 TV2,
                                              GR::f32 TU3, GR::f32 TV3,
                                              GR::f32 TU4, GR::f32 TV4,
                                              GR::u32 Color1, GR::u32 Color2, 
                                              GR::u32 Color3, GR::u32 Color4 );
    
    virtual void              RenderTriangle( const GR::tVector& Pos1,
                                              const GR::tVector& Pos2,
                                              const GR::tVector& Pos3,
                                              GR::f32 TU1, GR::f32 TV1,
                                              GR::f32 TU2, GR::f32 TV2,
                                              GR::f32 TU3, GR::f32 TV3,
                                              GR::u32 Color1, GR::u32 Color2 = 0, 
                                              GR::u32 Color3 = 0 );

    virtual void              RenderTriangle2d( const GR::tPoint& Pos1,
                                                const GR::tPoint& Pos2,
                                                const GR::tPoint& Pos3,
                                                GR::f32 TU1, GR::f32 TV1,
                                                GR::f32 TU2, GR::f32 TV2,
                                                GR::f32 TU3, GR::f32 TV3,
                                                GR::u32 Color1, GR::u32 Color2 = 0, 
                                                GR::u32 Color3 = 0, float fZ = 0.0f );

    // Set NULL to return to previous back buffer
    virtual void              SetRenderTarget( XTexture* pTexture );

    virtual void                  SetShader( eShaderType sType );


  private:

    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* pRectSrc = NULL, GR::tRect* pRectDest = NULL );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 ColorKey = 0, const GR::u32 MipMapLevel = 0 );
    virtual bool                  ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel = 0 );
    virtual void                  RestoreAllTextures();

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat Format, bool AllowUsageAsRenderTarget = false );

    void                          SetAlpha( GR::u32 AlphaTest, GR::u32 AlphaBlend );

    bool                          IsExtensionSupported( const GR::String& TargetExtension );

    void                          CheckError( const GR::String& Function );

    int                           VisibleWidth();
    int                           VisibleHeight();

    void                          InvertCullMode();

    void                          ReleaseTexture( OpenGLShaderTexture* pTexture );

    void                          ChooseShaders( GR::u32 VertexFormat );

    void                          FlushLightChanges();
    void                          FlushAllCaches();

    void                          UpdateMatrixUBO();
    void                          UpdateMaterialUBO();
    void                          UpdateFogUBO();
    void                          UpdateLightsUBO();

    friend class OpenGLShaderVertexBuffer;
    friend class OpenGLShaderTexture;
    friend class QuadCache;
    friend class LineCache;
    friend class TriangleCache;

};
