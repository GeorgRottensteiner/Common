#pragma once


#include <SDL.h>

#include <vector>

#include <Xtreme/XBasicRenderer.h>
#include <Xtreme/XAsset/XAssetLoader.h>



class SDLRenderClass : public XBasicRenderer
{

  public:


    SDLRenderClass();
    virtual ~SDLRenderClass();


    virtual bool                  Initialize( GR::u32 Width, GR::u32 Height, GR::u32 Depth, GR::u32 Flags, GR::IEnvironment& Environment );
    virtual bool                  Release();

    virtual void                  ReleaseAssets();
    void                          UnloadAssets( Xtreme::Asset::eXAssetType Type );

    virtual bool                  OnResized();
    virtual GR::u32               Width();
    virtual GR::u32               Height();
    virtual GR::Graphic::eImageFormat   ImageFormat();

    virtual bool                  ToggleFullscreen();
    virtual bool                  SetMode( XRendererDisplayMode& DisplayMode );
    virtual bool                  IsFullscreen();

    virtual bool                  VSyncEnabled();
    virtual void                  EnableVSync( bool Enable );

    virtual bool                  SaveScreenShot( const char* szFileName );

    virtual bool                  IsReady() const;

    virtual void                  Clear( bool bClearColor = true, bool bClearZ = true );

    virtual bool                  SetState( eRenderState rState, GR::u32 rValue, GR::u32 dwStage = 0 );
    virtual void                  SetTransform( eTransformType tType, const math::matrix4& matTrix );

    virtual bool                  SetViewport( const XViewport& Viewport );
    virtual bool                  SetTrueViewport( const XViewport& Viewport );

    virtual XTexture*             LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert = GR::Graphic::IF_UNKNOWN, GR::u32 dwColorKey = 0, const GR::u32 dwMipMapLevels = 0, GR::u32 ColorKeyReplacementColor = 0 );
    virtual XTexture*             CreateTexture( const GR::u32 dwWidth, const GR::u32 dwHeight, const GR::Graphic::eImageFormat imgFormat, const GR::u32 dwMipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual XTexture*             CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevels = 0, bool AllowUsageAsRenderTarget = false );
    virtual void                  SetTexture( GR::u32 dwStage, XTexture* pTexture );

    virtual XVertexBuffer*        CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*        CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type );
    virtual XVertexBuffer*        CreateVertexBuffer( XVertexBuffer* pCloneSource );
    virtual XVertexBuffer*        CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat = XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_TEXTURECOORD );

    virtual void                  DestroyVertexBuffer( XVertexBuffer* );
    virtual bool                  RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 dwIndex = 0, GR::u32 dwCount = 0 );
    virtual bool                  RenderMesh( const Mesh::IMesh& Mesh );

    virtual bool                  SetLight( GR::u32 dwLightIndex, XLight& Light );

    virtual bool                  SetMaterial( const XMaterial& Material );

    virtual void                  RenderLine2d( const GR::tPoint& pt1, 
                                                const GR::tPoint& pt2, 
                                                GR::u32 dwColor1, 
                                                GR::u32 dwColor2 = 0, float fZ = 0.0f );
    virtual void                  RenderLine( const GR::tVector& vect1, 
                                              const GR::tVector& vect2, 
                                              GR::u32 dwColor1, 
                                              GR::u32 dwColor2 = 0 );

    virtual void                  RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                                GR::f32 fTU1, GR::f32 fTV1,
                                                GR::f32 fTU2, GR::f32 fTV2,
                                                GR::f32 fTU3, GR::f32 fTV3,
                                                GR::f32 fTU4, GR::f32 fTV4,
                                                GR::u32 dwColor1, GR::u32 dwColor2, 
                                                GR::u32 dwColor3, GR::u32 dwColor4, float fZ = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                                GR::f32 fTU1, GR::f32 fTV1,
                                                GR::f32 fTU2, GR::f32 fTV2,
                                                GR::f32 fTU3, GR::f32 fTV3,
                                                GR::f32 fTU4, GR::f32 fTV4,
                                                GR::u32 dwColor1, GR::u32 dwColor2, 
                                                GR::u32 dwColor3, GR::u32 dwColor4, float fZ = 0.0f );
    virtual void                  RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1, 
                                                  GR::f32 fX2, GR::f32 fY2, 
                                                  GR::f32 fX3, GR::f32 fY3, 
                                                  GR::f32 fX4, GR::f32 fY4, 
                                                  GR::f32 fTU1, GR::f32 fTV1,
                                                  GR::f32 fTU2, GR::f32 fTV2,
                                                  GR::f32 fTU3, GR::f32 fTV3,
                                                  GR::f32 fTU4, GR::f32 fTV4,
                                                  GR::u32 dwColor1, GR::u32 dwColor2, 
                                                  GR::u32 dwColor3, GR::u32 dwColor4, float fZ = 0.0f );
    virtual void                  RenderQuad( const GR::tVector& ptPos1,
                                              const GR::tVector& ptPos2,
                                              const GR::tVector& ptPos3,
                                              const GR::tVector& ptPos4,
                                              GR::f32 fTU1, GR::f32 fTV1,
                                              GR::f32 fTU2, GR::f32 fTV2,
                                              GR::u32 dwColor1, GR::u32 dwColor2, 
                                              GR::u32 dwColor3, GR::u32 dwColor4 );
    virtual void                  RenderQuad( const GR::tVector& ptPos1,
                                              const GR::tVector& ptPos2,
                                              const GR::tVector& ptPos3,
                                              const GR::tVector& ptPos4,
                                              GR::f32 fTU1, GR::f32 fTV1,
                                              GR::f32 fTU2, GR::f32 fTV2,
                                              GR::f32 fTU3, GR::f32 fTV3,
                                              GR::f32 fTU4, GR::f32 fTV4,
                                              GR::u32 dwColor1, GR::u32 dwColor2, 
                                              GR::u32 dwColor3, GR::u32 dwColor4 );
    
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

    // Set NULL to return to previous back buffer
    virtual void              SetRenderTarget( XTexture* pTexture );

  protected:

    virtual bool                  InitDeviceObjects();
    virtual bool                  InvalidateDeviceObjects();
    virtual bool                  RestoreDeviceObjects();
    virtual bool                  DeleteDeviceObjects();



  protected:

    bool                          m_IsReady;
    bool                          m_Windowed;
    bool                          m_VSyncEnabled;
    


    virtual bool                  BeginScene();
    virtual void                  EndScene();
    virtual void                  PresentScene( GR::tRect* rectSrc = NULL, GR::tRect* rectDest = NULL );

    virtual bool                  CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 dwColorKey = 0, const GR::u32 dwMipMapLevel = 0 );
    virtual bool                  ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevel = 0 );
    virtual void                  RestoreAllTextures();

    virtual bool                  IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget = false );

    void                          DoSaveSnapShot( const char* szFile );

};
