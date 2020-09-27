#ifndef _DX8_VIEWER_H_
#define _DX8_VIEWER_H_
/*----------------------------------------------------------------------------+
 | Programmname       : D3DApp für DX8                                        |
 +----------------------------------------------------------------------------+
 | Autor              : Rottensteiner Georg                                   |
 | Datum              : 12.7.2000                                             |
 | Version            : 0.1                                                   |
 +----------------------------------------------------------------------------*/


/*-Includes-------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

#ifndef STRICT
#define STRICT
#endif


#include <D3D8.h>
#include <D3DX8.h>

#include <DX8/DX8Types.h>
#include <DX8/DX8RenderStateBlock.h>

#include <Math\vector3.h>

#include <DX8\DX8Object.h>
#include <DX8\VertexBuffer.h>

#include <Interface/IFont.h>

#include <Grafik\ImageFormate\ImageFormatManager.h>
#include <Grafik\ImageFormate\FormatTGA.h>
#include <Grafik\ImageFormate\FormatBMP.h>
#include <Grafik\ImageFormate\FormatIGF.h>

#include <DX8/TextureManager.h>

#include <vector>
#include <map>
#include <string>
#include <set>



/*-Defines--------------------------------------------------------------------+
 |                                                                            |
 +----------------------------------------------------------------------------*/

struct tD3DModeInfo
{
  DWORD       m_dwWidth,
              m_dwHeight,
              m_dwBehaviour;      // Hardware / Software / Mixed vertex processing
  D3DFORMAT   m_d3dfPixel,        // Pixel format in this mode
              m_d3dfDepthStencil; // Which depth/stencil format to use with this mode
};

struct tD3DDeviceInfo
{
    // Device data
    D3DDEVTYPE                  deviceType;      // Reference, HAL, etc.

    D3DCAPS8                    d3dCaps;         // Capabilities of this device

    GR::String                 strName;

    bool                        bCanDoWindowed;  // Whether this device can work in windowed mode

    // Modes for this device
    std::vector<tD3DModeInfo>   m_vectMode;

    // Current state

    DWORD                       dwCurrentMode;

    BOOL                        bWindowed;

    D3DMULTISAMPLE_TYPE         MultiSampleType;
};

struct tD3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8      d3dAdapterIdentifier;

    D3DDISPLAYMODE              d3ddmDesktop;      // Desktop display mode for this adapter

    // Devices for this adapter
    std::vector<tD3DDeviceInfo> vectDevices;

    // Current state
    DWORD                       dwCurrentDevice;
};

class CD3DViewer;

struct tTextureSection;


class CDX8MultiTexture
{
  public:

    CD3DViewer*                       m_pViewer;

    tSize                             m_vSize;

    int                               m_iTileWidth,
                                      m_iTileHeight,
                                      m_iTilesX,
                                      m_iTilesY;

    DWORD                             m_dwCreationFlags,
                                      m_dwTransparentColor,
                                      m_dwResourceID,
                                      m_dwMipmapLevels,

                                      m_dwReferenzen;

    GR::String                       m_strFileName,
                                      m_strResourceType;

    HINSTANCE                         m_hResourceInstance;

    std::vector<tTextureSection*>     m_vectSections;


    CDX8MultiTexture::CDX8MultiTexture():
      m_iTileWidth( 0 ),
      m_iTileHeight( 0 ),
      m_iTilesX( 1 ),
      m_iTilesY( 1 ),
      m_dwReferenzen( 1 ),
      m_pViewer( NULL )
    {
      m_vectSections.clear();
    }

    BOOL RecreateSections();
    void ReleaseSections();
};

typedef std::map<GR::String,CDX8MultiTexture*>   tMapMultiTextures;

class CDX8Font;

class CD3DViewer : public CDX8RenderStateBlock
{

  protected:

    // Internal variables for the state of the app
    std::vector<tD3DAdapterInfo>  m_vectAdapters;

    //DWORD                         m_dwVertexShader;

    DWORD                         m_dwAdapter;

    DWORD                         m_dwRenderWidth,
                                  m_dwRenderHeight;

    D3DLIGHT8                     m_Lights[8];

    bool                          m_bLightEnabled[8];

    bool                          m_bAutoMipMapGeneration,    // autom. Mipmap-Generierung
                                  m_bForceWindowSize,         // 3d-Surface-Größe erzwingen
                                  m_bVSyncEnabled,            // betrifft Fullscreen, VSync benutzen
                                  m_bMultiThreadSafe,         // MultiThreading enablen

                                  m_bWindowedModePossible;    // kann der gewünschte Modus im Fenster?

    BOOL                          m_bWindowed;

    BOOL                          m_bActive;

    BOOL                          m_bShowingBackBuffer,   // wird gerade der 2. Backbuffer angezeigt? // BAUSTELLE - Triple Buffering?
                                  m_bSaveSnapShot;

    GR::String                   m_strSnapShotFile;

    WNDPROC                       m_pOldProc;


    // Internal functions to manage and render the 3D scene
    HRESULT                       BuildDeviceList();
    BOOL                          FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
                                                        D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );

    HRESULT                       Initialize3DEnvironment();
    HRESULT                       Resize3DEnvironment();

    HRESULT                       ForceWindowed();
    HRESULT                       UserSelectNewDevice();

    virtual HRESULT               AdjustWindowForChange();

    void                          DoSaveSnapShot( const char *szFile );

    // Main objects used for creating and rendering the 3D scene
    D3DPRESENT_PARAMETERS m_d3dpp;         // Parameters for CreateDevice/Reset

    HWND                          m_hWnd;              // The main app window
    HWND                          m_hWndFocus;         // The D3D focus window (usually same as m_hWnd)

    D3DCAPS8                      m_d3dCaps;           // Caps for the device
    D3DSURFACE_DESC               m_d3dsdBackBuffer;   // Surface desc of the backbuffer

    DWORD                         m_dwCreateFlags;     // Indicate sw or hw vertex processing
    DWORD                         m_dwWindowStyle,     // Saved window style for mode switches
                                  m_dwWindowStyleEx;

    int                           m_iForcedWindowWidth,
                                  m_iForcedWindowHeight;

    WINDOWPLACEMENT               m_WindowPlacement;    // für Windowed-Modus aufbewahren

    ID3DXFont*                    m_pFont;

    HFONT                         m_hFont;

    // Overridable variables for the app
    BOOL                          m_bUseDepthBuffer;   // Whether to autocreate depthbuffer
    DWORD                         m_dwMinDepthBits;    // Minimum number of bits needed in depth buffer
    DWORD                         m_dwMinStencilBits;  // Minimum number of bits needed in stencil buffer
    DWORD                         m_dwCreationWidth;   // Width used to create window
    DWORD                         m_dwCreationHeight;  // Height used to create window
    DWORD                         m_dwCreationDepth;   // Depth used to create window

    static CD3DViewer*            m_pViewer;


    // Overridable functions for the 3D scene created by the app
    virtual HRESULT               ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT )   { return S_OK; }

    virtual void                  RestoreAppDeviceObjects();
    virtual void                  InvalidateAppDeviceObjects();
    virtual void                  DeleteAppDeviceObjects();

    // diese Funktionen müssen überschrieben werden!
    virtual HRESULT               InitDeviceObjects();
    virtual HRESULT               RestoreDeviceObjects();
    virtual HRESULT               InvalidateDeviceObjects();
    virtual HRESULT               DeleteDeviceObjects();


    // diese nicht!
    virtual void                  SelectDefaultMode( int iAdapterNr, tD3DAdapterInfo* pAdapter, tD3DDeviceInfo* pDevice );



    // Texture-spezifisch
    BOOL IsTextureFormatOk( D3DFORMAT TextureFormat );
    BOOL LoadSurface( CDX8Texture *pNewTexture, DWORD dwFlags, DWORD dwTransparentColor );
    BOOL LoadSurfaces( CDX8MultiTexture *pNewTexture, DWORD dwFlags, DWORD dwTransparentColor );
    HRESULT CopyImageToSplitSurfaces( CDX8MultiTexture *pNewTexture, GR::Graphic::ImageData* pData, DWORD dwFlags, DWORD dwTransparentColor );
    HRESULT CopyImageToSingleSurface( GR::Graphic::ImageData* pData, LPDIRECT3DTEXTURE8 resultingSurface,
                                      int iWidth, int iHeight,
                                      DWORD dwTransparentColor,
                                      DWORD dwFlags,
                                      int iCopyX1, int iCopyY1, int iCopyX2, int iCopyY2,
                                      int iTargetMipMapLevel = -1 );


  public:

    LPDIRECT3D8                   m_pD3D;              // The main D3D object

    LPDIRECT3DDEVICE8             m_pd3dDevice;        // The D3D rendering device

    tD3DModeInfo*                 m_pCurrentModeInfo;

    DWORD                         m_dwCurrentAdapter,
                                  m_dwCurrentDevice,
                                  m_dwCurrentMode;

    BOOL                          m_bReady;

    // Cachen der Matrizen
    D3DXMATRIX                    m_matWorld,
                                  m_matView,
                                  m_matProjection;

    D3DVIEWPORT8                  m_viewPort;


    CD3DViewer();
    ~CD3DViewer();


    virtual BOOL                  Create( HINSTANCE hInstance, HWND hwnd );
    void                          Cleanup3DEnvironment();

    HWND                          GetSafeHwnd() const;

    static CD3DViewer&            Instance();


    BOOL                          ResizeSurface();

    BOOL                          RenderBegin();
    void                          RenderDone( RECT* rectSrc = NULL, RECT* rectDest = NULL );

    void                          ShowFrontBuffer();     // für Videos

    // Screen-Funktionen
    virtual HRESULT               ToggleFullscreen();
    virtual void                  ToggleVSync();

    int                           Width() const;
    int                           Height() const;

    void                          SaveSnapShot( const char *szFile );


    HRESULT                       SetViewport( CONST D3DVIEWPORT8* pViewport );

    // Matrix-spezifisch
    HRESULT                       SetTransform( D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix );
    HRESULT                       SetTransform( D3DTRANSFORMSTATETYPE State, CONST math::matrix4& matTransform );

    void                          SetWorldMatrixForBillBoardingAt( const D3DXVECTOR3& vecPos );

    // Lights
    void                          SetLight( size_t iIndex, D3DLIGHT8& Light );
    void                          EnableLight( size_t iIndex, bool bEnable = true );


    // Darstellungs-Funktionen
    void                          DrawLine( const math::vector3& posAnfang, const math::vector3& posEnde, DWORD dwColor );
    void                          DrawLine( D3DXVECTOR3 posAnfang, D3DXVECTOR3 posEnde, DWORD dwColor );
    void                          DrawLine( int iX1, int iY1, int iX2, int iY2, DWORD dwColor );

    void                          DrawFreeLine( const D3DXVECTOR3& posAnfang, const D3DXVECTOR3& posEnde, DWORD dwColor1, DWORD dwColor2 = 0 );
    void                          DrawRect( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor );
    void                          DrawRect( const int iX, const int iY, const int iWidth, const int iHeight, DWORD dwColor );
    void                          DrawFreeRect( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor );

    void                          DrawBox( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize, DWORD dwColor1,
                                           DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawBox( int iX, int iY, int iWidth, int iHeight, DWORD dwColor1,
                                           DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawFreeBox( D3DXVECTOR3& vPos, D3DXVECTOR3& vSize, DWORD dwColor = 0xffffffff );

    void                          DrawFreeTriangle( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                            D3DXVECTOR3& v3,
                                            DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0 );
    void                          DrawQuad( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize,
                                            DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawQuad( int fX, int fY, int iWidth, int iHeight,
                                            DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );

    void                          DrawFreeQuad( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                                D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                                DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawFreeQuad( D3DXVECTOR3& vPos, D3DXVECTOR3& vSize,
                                                DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawFreeQuadDetail( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                              D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                              float fTU1, float fTV1, float fTU2, float fTV2,
                                              DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawFreeQuadDetail( D3DXVECTOR3& v1, D3DXVECTOR3& v2,
                                              D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                              float fTU1, float fTV1, float fTU2, float fTV2,
                                              float fTU3, float fTV3, float fTU4, float fTV4,
                                              DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawQuadDetail( D3DXVECTOR3& posAnfang, D3DXVECTOR3& vectSize,
                                          float fTU1, float fTV1, float fTU2, float fTV2,
                                          DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawQuadDetail( const int iX, const int iY, const int iWidth, const int iHeight,
                                                  float fTU1, float fTV1, float fTU2, float fTV2,
                                                  DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawQuadDetail( const int iX, const int iY, const int iWidth, const int iHeight,
                                                  float fTU1, float fTV1, float fTU2, float fTV2,
                                                  float fTU3, float fTV3, float fTU4, float fTV4,
                                                  DWORD dwColor1, DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0 );
    void                          DrawTextureSection( int iX, int iY, const tTextureSection& TexSection,
                                                  DWORD dwColor = 0xffffffff, int iWidth = -1, int iHeight = -1,
                                                  DWORD dwAlternativeFlags = 0 );
    void                          DrawFreeTextureSection( D3DXVECTOR3& v1, D3DXVECTOR3& v2, D3DXVECTOR3& v3, D3DXVECTOR3& v4,
                                                  const tTextureSection& TexSection, DWORD dwColor1,
                                                  DWORD dwColor2 = 0, DWORD dwColor3 = 0, DWORD dwColor4 = 0,
                                                  DWORD dwAlternativeFlags = 0 );

    void                          DisplayBoundingBox( const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, DWORD dwColor = 0xffffffff );
    void                          DisplayBoundingBox( const CBoundingBox& BBox, DWORD dwColor = 0xffffffff );

    virtual void                  Print( int iX, int iY, const char *szText, DWORD dwColor = 0xffffffff );
    virtual void                  DrawText( CDX8Font *pFont, int iX, int iY, const char *szText, float fScaleX, float fScaleY, DWORD dwColor1 = 0xffffffff, DWORD dwColor2 = 0xffffffff, DWORD dwColor3 = 0xffffffff, DWORD dwColor4 = 0xffffffff );


    void                          CastRayFromScreen( int iX, int iY, D3DXVECTOR3& v1, D3DXVECTOR3& v2, float fZ1Value = 0.0f );
    void                          TransformToScreen( D3DXVECTOR3& vect );


    // State-spezifisch
    HRESULT                       SetRenderState( D3DRENDERSTATETYPE dwState, GR::u32 dwValue );
    HRESULT                       SetTextureStageState( GR::u32 dwStage, D3DTEXTURESTAGESTATETYPE dwState, GR::u32 dwValue );

    HRESULT                       SetVertexShader( GR::u32 Handle );

    // Texture-spezifisch
    tMapMultiTextures                         m_mapMultiTextures;


    CDX8Texture*          CreateTexture( DWORD dwWidth,
                                         DWORD dwHeight,
                                         D3DFORMAT formatSurface,
                                         DWORD dwMipMapLevels = 1 );

    CDX8Texture*          LoadTexture( const char *szFileName,
                                               DWORD dwFlags = DX8::TF_DEFAULT,
                              DWORD dwTransparentColor = 0xff000000,
                              DWORD dwMipmapLevels = 1 );
    BOOL                  LoadMipMapTexture( CDX8Texture *pNewTexture,
                                         const char* szFileName,
                                         DWORD dwMipmapLevel,
                                                     DWORD dwFlags = DX8::TF_DEFAULT,
                                         DWORD dwTransparentColor = 0xff000000 );

    CDX8MultiTexture*     LoadMultiTexture( const char *szFileName,
                                                    DWORD dwFlags = DX8::TF_DEFAULT,
                              DWORD dwTransparentColor = 0xff000000,
                              DWORD dwMipmapLevels = 1 );
    CDX8MultiTexture*     LoadMultiTextureFromResource( HINSTANCE hInstance,
                                          DWORD dwResourceID,
                                          const char *szResourceType,
                                                    DWORD dwFlags = DX8::TF_DEFAULT | DX8::TF_FROM_RESOURCE,
                                          DWORD dwTransparentColor = 0xff000000,
                                          DWORD dwMipmapLevels = 1 );
    CDX8MultiTexture*     FindMultiTexture( const char *szFileName );

    void                  SetTexture( DWORD dwStage, CDX8Texture *pTexture );
    void                  SetTexture( DWORD dwStage, CDX8MultiTexture *pTexture, DWORD dwSection );

    void                  ReleaseTextures();
    BOOL                  RecreateTextures();

    void                  DestroyTexture( CDX8Texture *pTextureEntry );
    void                  DestroyTexture( CDX8MultiTexture *pTextureEntry );
    void                  DestroyTextures();

    void                  DumpTextures();

    // Font-spezifisch
    std::vector<CDX8Font*>      m_vectFonts;

    BOOL                  RecreateFonts();
    void                  ReleaseFonts();
    void                  DestroyFont( CDX8Font *pFont );
    void                  DestroyAllFonts();
    virtual int           DrawLetter( CDX8Font *pFont, int iX, int iY, char cLetter, DWORD dwColor = 0xffffffff );
    virtual float         DrawLetterDetail( CDX8Font *pFont, int iX, int iY, char cLetter, float fScaleX, float fScaleY, DWORD dwColor1, DWORD dwColor2, DWORD dwColor3, DWORD dwColor4 );
    virtual void          DrawText( CDX8Font *pFont, int iX, int iY, const char *szText, DWORD dwColor = 0xffffffff );
    void                  DrawFreeText( CDX8Font *pFont, D3DXVECTOR3& vectPos, const char *szText, D3DXVECTOR3& vectPlaneAndScale, DWORD dwColor = 0xffffffff );


    // VertexBuffer-spezifisch
    std::set<CVertexBuffer*>      m_setVBs;

    CVertexBuffer*      CreateVertexBuffer( DWORD dwPrimitiveCount,
                                          DWORD dwPrimitiveSize,
                                          DWORD dwStride,
                                          D3DPRIMITIVETYPE dwPrimitiveType,
                                          DWORD dwUsage,
                                          DWORD dwFVF,
                                          D3DPOOL d3dPool );
    void                RemoveVertexBuffer( CVertexBuffer *pVB );

    friend class CDX8Font;
    friend class CDX8Texture;
    friend class CDX8MultiTexture;
    friend class CTextureManager;
    friend class CVertexBuffer;
    friend LRESULT CALLBACK My3dAppProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    // Diverses
    void                DumpModes();

};



#endif // _DX8_VIEWER_H_



