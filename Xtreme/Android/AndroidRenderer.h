#pragma once

#include <Xtreme/XRenderer.h>
#include <Xtreme/XBasicRenderer.h>

#include <EGL/egl.h>
#include <GLES/gl.h>



class AndroidRenderer : public XBasicRenderer
{
  private:


    EGLDisplay    m_Display;
    EGLSurface    m_Surface;
    EGLContext    m_Context;

    int           m_Width;
    int           m_Height;


  public:

    ANativeWindow*            m_Window;



    AndroidRenderer();

    virtual bool              Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment );

    virtual bool              Release();

    virtual bool              OnResized();

    virtual bool              IsReady() const;

    virtual bool              BeginScene();
    virtual void              EndScene();
    virtual void              PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool              ToggleFullscreen();
    virtual bool              SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool              IsFullscreen();

    virtual bool              SetState( eRenderState rState, GR::u32 rValue, GR::u32 dwStage = 0 );
    virtual bool              SetViewport( const XViewport& Viewport );
    virtual bool              SetTrueViewport( const XViewport& Viewport );

    virtual GR::u32           Width();
    virtual GR::u32           Height();

    virtual void              Clear( bool bClearColor = true, bool bClearZ = true );

    virtual bool              IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         CreateTexture( const GR::u32 dwWidth, const GR::u32 dwHeight, const GR::Graphic::eImageFormat imgFormat, const GR::u32 dwMipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*         LoadTexture( const char* FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 dwColorKey = 0, const GR::u32 dwMipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual XTexture*         LoadTexture( const GR::IString& FileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 dwColorKey = 0, const GR::u32 dwMipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual void              SetTexture( GR::u32 dwStage, XTexture* pTexture );
    virtual bool              CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 dwColorKey = 0, const GR::u32 dwMipMapLevel = 0 );

    // Lights
    virtual bool              SetLight( GR::u32 dwLightIndex, XLight& Light );

    virtual bool              SetMaterial( const XMaterial& Material );

    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 dwPrimitiveCount, GR::u32 dwVertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*    CreateVertexBuffer( GR::u32 dwVertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*    CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 dwVertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD );

    virtual void              DestroyVertexBuffer( XVertexBuffer* );
    virtual bool              RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 dwIndex = 0, GR::u32 dwCount = 0 );


    virtual void              RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 dwColor1, GR::u32 dwColor2 = 0, float fZ = 0.0f );
    virtual void              RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 dwColor1, GR::u32 dwColor2 = 0 );

    virtual void              RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0, GR::u32 dwColor4 = 0, float fZ = 0.0f );
    virtual void              RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0, GR::u32 dwColor4 = 0, float fZ = 0.0f );
    virtual void              RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
      GR::f32 fX2, GR::f32 fY2,
      GR::f32 fX3, GR::f32 fY3,
      GR::f32 fX4, GR::f32 fY4,
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
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0, GR::u32 dwColor4 = 0 );

    virtual void              RenderQuad( const GR::tVector& ptPos1,
      const GR::tVector& ptPos2,
      const GR::tVector& ptPos3,
      const GR::tVector& ptPos4,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::f32 fTU4, GR::f32 fTV4,
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0, GR::u32 dwColor4 = 0 );

    virtual void              RenderTriangle( const GR::tVector& pt1,
      const GR::tVector& pt2,
      const GR::tVector& pt3,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0 );

    virtual void              RenderTriangle2d( const GR::tPoint& pt1,
      const GR::tPoint& pt2,
      const GR::tPoint& pt3,
      GR::f32 fTU1, GR::f32 fTV1,
      GR::f32 fTU2, GR::f32 fTV2,
      GR::f32 fTU3, GR::f32 fTV3,
      GR::u32 dwColor1, GR::u32 dwColor2 = 0,
      GR::u32 dwColor3 = 0, float fZ = 0.0f );

    virtual bool              SaveScreenShot( const char* FileName );

    virtual void              ReleaseAssets();

    virtual bool              VSyncEnabled();
    virtual void              EnableVSync( bool Enable = true );

    virtual GR::Graphic::eImageFormat   ImageFormat();

    virtual bool              ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 MipMapLevel );

    virtual bool              RenderMesh( const Mesh::IMesh& Mesh );

    // Set NULL to return to previous back buffer
    virtual void              SetRenderTarget( XTexture* pTexture );

};

