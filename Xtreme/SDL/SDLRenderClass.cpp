#include <debug/debugclient.h>

#include <Grafik/ContextDescriptor.h>
#include <Grafik/ImageFormate/ImageFormatManager.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/Environment/XWindowSDL.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <String/XML.h>

#include <Misc/Misc.h>

#include <IO/FileStream.h>

#include "SDLRenderClass.h"
#include "SDLTexture.h"
#include "SDLVertexBuffer.h"



SDLRenderClass::SDLRenderClass() :
  m_IsReady( false ),
  m_Windowed( true ),
  m_VSyncEnabled( true )
{
}



SDLRenderClass::~SDLRenderClass()
{
  Release();
}



bool SDLRenderClass::Initialize( GR::u32 dwWidth, GR::u32 dwHeight, GR::u32 dwDepth, GR::u32 Flags, GR::IEnvironment& Environment )
{
  m_pEnvironment = &Environment;

  m_Transform[TT_WORLD].Identity();
  m_Transform[TT_VIEW].Identity();
  m_Transform[TT_PROJECTION].Identity();

  m_IsReady = true;

  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
    if ( pLoader )
    {
      // es gibt Assets zu laden
      GR::up    imageCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE );
      for ( GR::up i = 0; i < imageCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE, i );

        GR::u32         dwColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ColorKey" ), 16 );

        GR::u32         dwForcedFormat = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "ForcedFormat" ) );

        GR::String      fileName = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "File" );

        std::list<GR::String>    listImages;

        GR::u32     dwMipMaps = 0;

        while ( true )
        {
          if ( dwMipMaps == 0 )
          {
            listImages.push_back( AppPath( fileName ) );
          }
          else
          {
            GR::String   strPath = pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", dwMipMaps ) );

            if ( strPath.empty() )
            {
              break;
            }
            listImages.push_back( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, CMisc::printf( "MipMap%d", dwMipMaps ) ) ) );
          }
          dwMipMaps++;
        }

        if ( listImages.empty() )
        {
          Log( "Renderer.General", "DX9Renderer: Asset Image has no images!" );
        }
        else
        {
          XTexture* pTexture = LoadTexture( listImages.front().c_str(), (GR::Graphic::eImageFormat)dwForcedFormat, dwColorKey, dwMipMaps, 0xff000000 );

          if ( pTexture )
          {
            if ( pAsset == NULL )
            {
              pAsset = new Xtreme::Asset::XAssetImage( pTexture );
            }
            else
            {
              ( ( Xtreme::Asset::XAssetImage* )pAsset )->m_pTexture = pTexture;
            }
            pLoader->SetAsset( Xtreme::Asset::XA_IMAGE, i, pAsset );

            // MipMaps laden
            std::list<GR::String>::iterator    it( listImages.begin() );
            it++;
            GR::u32   dwLevel = 1;

            while ( it != listImages.end() )
            {
              GR::String&  strPath( *it );

              GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)dwForcedFormat, dwColorKey, 0xff000000 );
              if ( pData == NULL )
              {
                Log( "Renderer.General", CMisc::printf( "DX9Renderer:: Asset MipMap failed to load (%s)", strPath.c_str() ) );
              }
              else
              {
                CopyDataToTexture( pTexture, *pData, dwColorKey, dwLevel );
                delete pData;
              }

              ( (SDLTexture*)pTexture )->m_FileNames.push_back( strPath );

              ++dwLevel;
              ++it;
            }
          }
          else
          {
            Log( "Renderer.General", CMisc::printf( "DX9Renderer: Failed to load image %s", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE, i, "Name" ) ) );
          }
        }
      }

      GR::up    imageSectionCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_IMAGE_SECTION );
      for ( GR::up i = 0; i < imageSectionCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, i );

        Xtreme::Asset::XAssetImage* pTexture = (Xtreme::Asset::XAssetImage*)pLoader->Asset( Xtreme::Asset::XA_IMAGE, pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) );
        if ( pTexture == NULL )
        {
          Log( "Renderer.General", CMisc::printf( "DX9Renderer: ImageSection Asset, Image %s not found", pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Image" ) ) );
        }
        else
        {
          XTextureSection   tsSection;

          tsSection.m_XOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "X" ) );
          tsSection.m_YOffset = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Y" ) );
          tsSection.m_Width   = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "W" ) );
          tsSection.m_Height  = GR::Convert::ToI32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "H" ) );
          tsSection.m_Flags   = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_IMAGE_SECTION, i, "Flags" ) );

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
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_FONT, i );

        GR::u32     dwColorKey = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "ColorKey" ), 16 );

        GR::u32     dwLoadFlags = GR::Convert::ToU32( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "LoadFlags" ), 16 );

        XFont*      pFont = NULL;

        if ( ( dwLoadFlags & XFont::FLF_SQUARED )
        ||   ( dwLoadFlags & XFont::FLF_SQUARED_ONE_FONT ) )
        {
          pFont = LoadFontSquare( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(),
                                  dwLoadFlags,
                                  dwColorKey );
        }
        else
        {
          pFont = LoadFont( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "File" ) ).c_str(), dwColorKey );
        }

        if ( pFont )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX9Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
          }
          delete pAsset;

          pAsset = new Xtreme::Asset::XAssetFont( pFont );
          pLoader->SetAsset( Xtreme::Asset::XA_FONT, i, pAsset );
        }
        else
        {
          Log( "Renderer.General", "DX9Renderer: Failed to load font %s", pLoader->AssetAttribute( Xtreme::Asset::XA_FONT, i, "Name" ) );
        }
      }

      GR::up    meshCount = pLoader->AssetTypeCount( Xtreme::Asset::XA_MESH );
      for ( GR::up i = 0; i < meshCount; ++i )
      {
        Xtreme::Asset::XAsset*    pAsset = pLoader->Asset( Xtreme::Asset::XA_MESH, i );

        XMesh*       pMesh = NULL;

        pMesh = CT3DMeshLoader::Load( AppPath( pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "File" ) ).c_str() );
        if ( pMesh )
        {
          if ( pAsset )
          {
            Log( "Renderer.General", "DX9Renderer: Asset already loaded! %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
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
          Log( "Renderer.General", "DX9Renderer: Failed to load Mesh %s", pLoader->AssetAttribute( Xtreme::Asset::XA_MESH, i, "Name" ) );
        }
      }
      pLoader->NotifyService( "GUI", "AssetsLoaded" );
    }
  }
  NotifyService( "Renderer", "Initialised" );
  return true;
}



bool SDLRenderClass::Release()
{
  ReleaseAssets();

  m_IsReady = false;

  NotifyService( "Renderer", "Released" );
  return true;
}



bool SDLRenderClass::IsReady() const
{
  return m_IsReady;
}



bool SDLRenderClass::InitDeviceObjects()
{
  RestoreDeviceObjects();
  return true;
}



bool SDLRenderClass::RestoreDeviceObjects()
{
  RestoreStates();

  RestoreAllVertexBuffers();
  RestoreAllTextures();
  RestoreAllFonts();

  return true;
}



bool SDLRenderClass::InvalidateDeviceObjects()
{
  return true;
}



bool SDLRenderClass::DeleteDeviceObjects()
{
  return true;
}



bool SDLRenderClass::BeginScene()
{
  return true;
  /*
  HRESULT hr = S_OK;

  if ( !m_pd3dDevice )
  {
    return false;
  }
  if ( !m_bReady )
  {
    return false;
  }

  // Test the cooperative level to see if it's okay to render
  if ( FAILED( hr = m_pd3dDevice->TestCooperativeLevel() ) )
  {

    // If the device was lost, do not render until we get it back
    if ( D3DERR_DEVICELOST == hr )
    {
      return false;
    }

    // Check if the device needs to be resized.
    if ( D3DERR_DEVICENOTRESET == hr )
    {
      // If we are windowed, read the desktop mode and use the same format for
      // the back buffer
      if ( m_bWindowed )
      {
        tD3DAdapterInfo* pAdapterInfo = &m_vectAdapters[m_dwAdapter];
        m_pD3D->GetAdapterDisplayMode( m_dwAdapter, &pAdapterInfo->d3ddmDesktop );
        m_d3dpp.BackBufferFormat = pAdapterInfo->d3ddmDesktop.Format;
      }

      if ( FAILED( hr = Resize3DEnvironment() ) )
      {
        return false;
      }
    }
  }

  return !!SUCCEEDED( m_pd3dDevice->BeginScene() );
  */
}



void SDLRenderClass::EndScene()
{
}



void SDLRenderClass::PresentScene( GR::tRect* rectSrc, GR::tRect* rectDest )
{
  Xtreme::SDLWindow*   pApp = ( Xtreme::SDLWindow*)m_pEnvironment->Service( "Window" );

  SDL_UpdateWindowSurface( pApp->m_pMainWindow );
  /*
  if ( !m_strSaveScreenShotFileName.empty() )
  {
    DoSaveSnapShot( m_strSaveScreenShotFileName.c_str() );
    m_strSaveScreenShotFileName.clear();
  }*/

  // Show the frame on the primary surface.
  //m_pd3dDevice->Present( rectSrc, rectDest, NULL, NULL );
}



void SDLRenderClass::Clear( bool bClearColor, bool bClearZ )
{
}



XTexture* SDLRenderClass::LoadTexture( const char* szFileName, GR::Graphic::eImageFormat imgFormatToConvert, GR::u32 dwColorKey, const GR::u32 dwMipMapLevels, GR::u32 ColorKeyReplacementColor )
{
  GR::String path = szFileName;

  GR::Graphic::ImageData*   pData = LoadAndConvert( path.c_str(), imgFormatToConvert, dwColorKey, ColorKeyReplacementColor );
  if ( pData == NULL )
  {
    return NULL;
  }

  XTexture*   pTexture = CreateTexture( *pData, dwMipMapLevels );
  if ( pTexture == NULL )
  {
    delete pData;
    return NULL;
  }

  pTexture->m_ColorKey        = dwColorKey;
  pTexture->m_LoadedFromFile  = path;

  delete pData;
  return pTexture;

}



XTexture* SDLRenderClass::CreateTexture( const GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevels, bool AllowUsageAsRenderTarget )
{
  XTexture*   pTexture = CreateTexture( ImageData.Width(), ImageData.Height(), ImageData.ImageFormat(), dwMipMapLevels );
  if ( pTexture == NULL )
  {
    return NULL;
  }

  CopyDataToTexture( pTexture, ImageData );
  return pTexture;
}



XTexture* SDLRenderClass::CreateTexture( const GR::u32 dwWidth, const GR::u32 dwHeight, const GR::Graphic::eImageFormat imgFormatArg, const GR::u32 dwMipMapLevels, bool AllowUsageAsRenderTarget )
{
  if ( ( dwWidth == 0 )
  ||   ( dwHeight == 0 ) )
  {
    return NULL;
  }

  GR::Graphic::eImageFormat    imgFormat( imgFormatArg );


  if ( !IsTextureFormatOK( imgFormat, AllowUsageAsRenderTarget ) )
  {
    return NULL;
  }


  SDLTexture*   pTexture = new SDLTexture();

  AddTexture( pTexture );

  return pTexture;

}



bool SDLRenderClass::IsTextureFormatOK( GR::Graphic::eImageFormat imgFormat, bool AllowUsageAsRenderTarget )
{
  return true;
}



void SDLRenderClass::RenderQuad2d( GR::i32 iX, GR::i32 iY, GR::i32 iWidth, GR::i32 iHeight,
                                    GR::f32 fTU1, GR::f32 fTV1,
                                    GR::f32 fTU2, GR::f32 fTV2,
                                    GR::f32 fTU3, GR::f32 fTV3,
                                    GR::f32 fTU4, GR::f32 fTV4,
                                    GR::u32 dwColor1, GR::u32 dwColor2,
                                    GR::u32 dwColor3, GR::u32 dwColor4, float fZ )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == dwColor4 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
}



void SDLRenderClass::RenderQuadDetail2d( GR::f32 fX, GR::f32 fY, GR::f32 fWidth, GR::f32 fHeight,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::f32 fTU3, GR::f32 fTV3,
                                          GR::f32 fTU4, GR::f32 fTV4,
                                          GR::u32 dwColor1, GR::u32 dwColor2,
                                          GR::u32 dwColor3, GR::u32 dwColor4, float fZ )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == dwColor4 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }
}



void SDLRenderClass::RenderQuadDetail2d( GR::f32 fX1, GR::f32 fY1,
                                          GR::f32 fX2, GR::f32 fY2,
                                          GR::f32 fX3, GR::f32 fY3,
                                          GR::f32 fX4, GR::f32 fY4,
                                          GR::f32 fTU1, GR::f32 fTV1,
                                          GR::f32 fTU2, GR::f32 fTV2,
                                          GR::f32 fTU3, GR::f32 fTV3,
                                          GR::f32 fTU4, GR::f32 fTV4,
                                          GR::u32 dwColor1, GR::u32 dwColor2,
                                          GR::u32 dwColor3, GR::u32 dwColor4, float fZ )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == dwColor4 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }

}



void SDLRenderClass::RenderTriangle2d( const GR::tPoint& pt1, const GR::tPoint& pt2, const GR::tPoint& pt3,
                                        GR::f32 fTU1, GR::f32 fTV1,
                                        GR::f32 fTU2, GR::f32 fTV2,
                                        GR::f32 fTU3, GR::f32 fTV3,
                                        GR::u32 dwColor1, GR::u32 dwColor2,
                                        GR::u32 dwColor3, float fZ )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor1;
  }

}



void SDLRenderClass::RenderTriangle( const GR::tVector& ptPos1,
                                      const GR::tVector& ptPos2,
                                      const GR::tVector& ptPos3,
                                      GR::f32 fTU1, GR::f32 fTV1,
                                      GR::f32 fTU2, GR::f32 fTV2,
                                      GR::f32 fTU3, GR::f32 fTV3,
                                      GR::u32 dwColor1, GR::u32 dwColor2,
                                      GR::u32 dwColor3 )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor1;
  }

}



void SDLRenderClass::RenderQuad( const GR::tVector& ptPos1,
                                  const GR::tVector& ptPos2,
                                  const GR::tVector& ptPos3,
                                  const GR::tVector& ptPos4,
                                  GR::f32 fTU1, GR::f32 fTV1,
                                  GR::f32 fTU2, GR::f32 fTV2,
                                  GR::u32 dwColor1, GR::u32 dwColor2,
                                  GR::u32 dwColor3, GR::u32 dwColor4 )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == dwColor4 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }

}



void SDLRenderClass::RenderQuad( const GR::tVector& ptPos1,
                                  const GR::tVector& ptPos2,
                                  const GR::tVector& ptPos3,
                                  const GR::tVector& ptPos4,
                                  GR::f32 fTU1, GR::f32 fTV1,
                                  GR::f32 fTU2, GR::f32 fTV2,
                                  GR::f32 fTU3, GR::f32 fTV3,
                                  GR::f32 fTU4, GR::f32 fTV4,
                                  GR::u32 dwColor1, GR::u32 dwColor2,
                                  GR::u32 dwColor3, GR::u32 dwColor4 )
{
  if ( ( dwColor2 == dwColor3 )
  &&   ( dwColor3 == dwColor4 )
  &&   ( dwColor4 == 0 ) )
  {
    dwColor2 = dwColor3 = dwColor4 = dwColor1;
  }

}



XVertexBuffer* SDLRenderClass::CreateVertexBuffer( XVertexBuffer* pCloneSource )
{
  SDLVertexBuffer*   pBuffer = new SDLVertexBuffer();

  if ( !pBuffer->Create( pCloneSource->PrimitiveCount(), pCloneSource->VertexFormat(), pCloneSource->Type() ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* SDLRenderClass::CreateVertexBuffer( GR::u32 PrimitiveCount, GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  SDLVertexBuffer*   pBuffer = new SDLVertexBuffer();

  if ( !pBuffer->Create( PrimitiveCount, VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* SDLRenderClass::CreateVertexBuffer( GR::u32 VertexFormat, XVertexBuffer::PrimitiveType Type )
{
  SDLVertexBuffer*   pBuffer = new SDLVertexBuffer();
  if ( !pBuffer->Create( VertexFormat, Type ) )
  {
    delete pBuffer;
    return NULL;
  }

  AddVertexBuffer( pBuffer );

  return pBuffer;
}



XVertexBuffer* SDLRenderClass::CreateVertexBuffer( const Mesh::IMesh& MeshObject, GR::u32 VertexFormat )
{
  XVertexBuffer*    pBuffer = CreateVertexBuffer( MeshObject.FaceCount(), VertexFormat, XVertexBuffer::PrimitiveType::PT_TRIANGLE );
  if ( pBuffer == NULL )
  {
    return NULL;
  }

  pBuffer->FillFromMesh( MeshObject );

  return pBuffer;
}



void SDLRenderClass::DestroyVertexBuffer( XVertexBuffer* pVBBuffer )
{
  if ( pVBBuffer == NULL )
  {
    return;
  }

  pVBBuffer->Release();

  m_VertexBuffers.remove( pVBBuffer );
  delete pVBBuffer;
}



bool SDLRenderClass::RenderVertexBuffer( XVertexBuffer* pBuffer, GR::u32 dwIndex, GR::u32 dwCount )
{
  if ( !m_IsReady )
  {
    return false;
  }
  if ( pBuffer == NULL )
  {
    return false;
  }
  //return ( (CDX9VertexBuffer*)pBuffer )->Display( dwIndex, dwCount );

  return false;
}



void SDLRenderClass::SetTexture( GR::u32 dwStage, XTexture* pTexture )
{
  //m_pd3dDevice->SetTexture( dwStage, pTexture ? ( (CDX9Texture*)pTexture )->m_Surface : NULL );
}



bool SDLRenderClass::SetState( eRenderState rState, GR::u32 rValue, GR::u32 dwStage )
{
  tMapRenderStates::iterator    it( m_RenderStates.find( std::make_pair( rState, dwStage ) ) );
  if ( it != m_RenderStates.end() )
  {
    if ( it->second == rValue )
    {
      return true;
    }
  }

  /*
  HRESULT   hRes = E_FAIL;

  switch ( rState )
  {
    case RS_FOG_COLOR:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGCOLOR, rValue );
      break;
    case RS_FOG_DENSITY:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGDENSITY, rValue );
      break;
    case RS_FOG_START:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGSTART, rValue );
      break;
    case RS_FOG_END:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGEND, rValue );
      break;
    case RS_FOG_ENABLE:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
      }
      break;
    case RS_FOG_TABLEMODE:
      if ( rValue == RSV_FOG_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_NONE );
      }
      else if ( rValue == RSV_FOG_EXP )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_EXP );
      }
      else if ( rValue == RSV_FOG_EXP_SQUARED )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_EXP2 );
      }
      else if ( rValue == RSV_FOG_LINEAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
      }
      break;
    case RS_FOG_VERTEXMODE:
      if ( rValue == RSV_FOG_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_NONE );
      }
      else if ( rValue == RSV_FOG_EXP )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_EXP );
      }
      else if ( rValue == RSV_FOG_EXP_SQUARED )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_EXP2 );
      }
      else if ( rValue == RSV_FOG_LINEAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR );
      }
      break;
    case RS_COLORMASK:
      {
        DWORD   dwFlags = 0;

        if ( rValue & RSV_COLORMASK_RED )
        {
          dwFlags |= D3DCOLORWRITEENABLE_RED;
        }
        if ( rValue & RSV_COLORMASK_GREEN )
        {
          dwFlags |= D3DCOLORWRITEENABLE_GREEN;
        }
        if ( rValue & RSV_COLORMASK_BLUE )
        {
          dwFlags |= D3DCOLORWRITEENABLE_BLUE;
        }
        if ( rValue & RSV_COLORMASK_ALPHA )
        {
          dwFlags |= D3DCOLORWRITEENABLE_ALPHA;
        }
        hRes = m_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, dwFlags );
      }
      break;
    case RS_FILL_MODE:
      if ( rValue == RSV_FILL_SOLID )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
      }
      else if ( rValue == RSV_FILL_POINTS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_POINT );
      }
      else if ( rValue == RSV_FILL_WIREFRAME )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
      }
      break;
    case RS_SHADE_MODE:
      if ( rValue == RSV_SHADE_FLAT )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
      }
      else if ( rValue == RSV_SHADE_GOURAUD )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
      }
      break;
    case RS_TEXTURE_FACTOR:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, rValue );
      break;
    case RS_NORMALIZE_NORMALS:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, FALSE );
      }
      break;
    case RS_DIFFUSE_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_EMISSIVE_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_AMBIENT_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_SPECULAR_MATERIAL_SOURCE:
      if ( rValue == RSV_CMS_MATERIAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
      }
      else if ( rValue == RSV_CMS_COLOR_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1 );
      }
      else if ( rValue == RSV_CMS_COLOR_SPECULAR )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2 );
      }
      break;
    case RS_TEXTURE_TRANSFORM:
      if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
      }
      else if ( rValue == RSV_TEXTURE_TRANSFORM_COUNT2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
      }
      break;
    case RS_ADDRESSU:
      if ( rValue == RSV_ADDRESS_WRAP )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
      }
      break;
    case RS_ADDRESSV:
      if ( rValue == RSV_ADDRESS_WRAP )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
      }
      else if ( rValue == RSV_ADDRESS_CLAMP )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
      }
      break;
    case RS_ZWRITE:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
      }
      break;
    case RS_SPECULAR:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
      }
      break;
    case RS_DITHERING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );
      }
      break;
    case RS_CLEAR_COLOR:
      m_dwClearColor = rValue;
      hRes = S_OK;
      break;
    case RS_LIGHT:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->LightEnable( dwStage, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->LightEnable( dwStage, FALSE );
      }
      break;
    case RS_MINFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MINFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MAGFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_MIPFILTER:
      if ( rValue == RSV_FILTER_LINEAR )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
      }
      else if ( rValue == RSV_FILTER_POINT )
      {
        hRes = m_pd3dDevice->SetSamplerState( dwStage, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
      }
      break;
    case RS_ZBUFFER:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
      }
      break;
    case RS_LIGHTING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
      }
      break;
    case RS_AMBIENT:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, rValue );
      break;
    case RS_COLOR_OP:
      if ( rValue == RSV_MODULATE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLOROP, D3DTOP_MODULATE );
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_BLENDFACTORALPHA )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLOROP, D3DTOP_DISABLE );
      }
      break;
    case RS_COLOR_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG1, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG1, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG1, D3DTA_TFACTOR );
      }
      break;
    case RS_COLOR_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG2, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG2, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_COLORARG2, D3DTA_TFACTOR );
      }
      break;
    case RS_ALPHA_OP:
      if ( rValue == RSV_MODULATE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
      }
      else if ( rValue == RSV_SELECT_ARG_1 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
      }
      else if ( rValue == RSV_SELECT_ARG_2 )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
      }
      else if ( rValue == RSV_BLENDFACTORALPHA )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, D3DTOP_BLENDFACTORALPHA );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
      }
      break;
    case RS_ALPHA_ARG_1:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
      }
      break;
    case RS_ALPHA_ARG_2:
      if ( rValue == RSV_CURRENT )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
      }
      else if ( rValue == RSV_DIFFUSE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
      }
      else if ( rValue == RSV_TEXTURE )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
      }
      else if ( rValue == RSV_TEXTURE_FACTOR )
      {
        hRes = m_pd3dDevice->SetTextureStageState( dwStage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
      }
      break;
    case RS_SRC_BLEND:
      if ( rValue == RSV_SRC_ALPHA )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
      }
      else if ( rValue == RSV_ONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
      }
      else if ( rValue == RSV_ZERO )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
      }
      break;
    case RS_DEST_BLEND:
      if ( rValue == RSV_INV_SRC_ALPHA)
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
      }
      else if ( rValue == RSV_ONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
      }
      else if ( rValue == RSV_ZERO )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
      }
      break;
    case RS_CULLMODE:
      if ( rValue == RSV_CULL_NONE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
      }
      else if ( rValue == RSV_CULL_CCW )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
      }
      else if ( rValue == RSV_CULL_CW )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
      }
      break;
    case RS_ALPHATEST:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
      }
      break;
    case RS_ALPHABLENDING:
      if ( rValue == RSV_ENABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
      }
      else if ( rValue == RSV_DISABLE )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
      }
      break;
    case RS_ALPHAREF:
      hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, rValue );
      break;
    case RS_ALPHAFUNC:
      if ( rValue == RSV_COMPARE_GREATEREQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
      }
      else if ( rValue == RSV_COMPARE_GREATER )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
      }
      else if ( rValue == RSV_COMPARE_EQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_EQUAL );
      }
      else if ( rValue == RSV_COMPARE_ALWAYS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_ALWAYS );
      }
      else if ( rValue == RSV_COMPARE_LESS )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESS );
      }
      else if ( rValue == RSV_COMPARE_LESSEQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );
      }
      else if ( rValue == RSV_COMPARE_NEVER )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_NEVER );
      }
      else if ( rValue == RSV_COMPARE_NOTEQUAL )
      {
        hRes = m_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
      }
      break;
  }

  m_mapRenderStates[std::make_pair( rState, dwStage )] = rValue;
  if ( hRes == E_FAIL )
  {
    dh::Log( "SDLRenderClass::SetState not supported (%d - %d - Stage %d)", rState, rValue, dwStage );
  }
  return SUCCEEDED( hRes );*/
  return true;
}



void SDLRenderClass::SetTransform( eTransformType tType, const math::matrix4& matTrix )
{
  XBasicRenderer::SetTransform( tType, matTrix );

  switch ( tType )
  {
    case TT_WORLD:
      break;
    case TT_VIEW:
      break;
    case TT_PROJECTION:
      break;
    case TT_TEXTURE_STAGE_0:
      break;
    case TT_TEXTURE_STAGE_1:
      break;
  }

}



bool SDLRenderClass::ImageDataFromTexture( XTexture* pTexture, GR::Graphic::ImageData& ImageData, const GR::u32 dwMipMapLevel )
{
  if ( pTexture == NULL )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: Texture was NULL" );
    return false;
  }

  SDLTexture*    pNativeTexture = (SDLTexture*)pTexture;

  /*
  if ( dwMipMapLevel >= pNativeTexture->m_StoredImageData.size() )
  {
    dh::Error( "Renderer.General", "ImageDataFromTexture: MipMapLevel out of bounds %d >= %d", dwMipMapLevel, pNativeTexture->m_dwMipMapLevels );
    return false;
  }

  std::list<GR::Graphic::ImageData>::iterator   it( pNativeTexture->m_StoredImageData.begin() );
  std::advance( it, dwMipMapLevel );

  ImageData = *it;
  return true;*/
  return false;
}



bool SDLRenderClass::CopyDataToTexture( XTexture* pTexture, const GR::Graphic::ImageData& ImageData, GR::u32 dwColorKey, const GR::u32 dwMipMapLevel )
{

  if ( pTexture == NULL )
  {
    dh::Error( "CopyDataToTexture: Texture was NULL" );
    return false;
  }

  SDLTexture*    pNativeTexture = (SDLTexture*)pTexture;

  /*
  if ( dwMipMapLevel >= pNativeTexture->m_dwMipMapLevels )
  {
    dh::Error( "CopyDataToTexture: MipMapLevel out of bounds %d >= %d", dwMipMapLevel, pNativeTexture->m_dwMipMapLevels );
    return false;
  }*/

  /*
  if ( ( ImageData.Width() > (int)dwWidth )
  ||   ( ImageData.Height() > (int)dwHeight ) )
  {
    pDX9Texture->m_Surface->UnlockRect( dwMipMapLevel );
    dh::Error( "CopyDataToTexture: Sizes mismatching %dx%d != %dx%d", ImageData.Width(), ImageData.Height(), dwWidth, dwHeight );
    return false;
  }

  GR::Graphic::ContextDescriptor      cdTexture;

  cdTexture.Attach( dwWidth, dwHeight, lockRect.Pitch, pTexture->m_ImageFormat, lockRect.pBits );

  if ( !ImageData.ConvertInto( &cdTexture, dwColorKey != 0, dwColorKey ) )
  {
    pDX9Texture->m_Surface->UnlockRect( dwMipMapLevel );
    dh::Error( "CopyDataToTexture ConvertInto failed" );
    return false;
  }

  pDX9Texture->m_Surface->UnlockRect( dwMipMapLevel );

  // store copied data
  while ( pDX9Texture->m_StoredImageData.size() <= dwMipMapLevel )
  {
    pDX9Texture->m_StoredImageData.push_back( GR::Graphic::ImageData() );
  }
  pDX9Texture->m_StoredImageData.back() = ImageData;*/
  return true;
}



void SDLRenderClass::RestoreAllTextures()
{
  tListTextures::iterator   it( m_Textures.begin() );
  while ( it != m_Textures.end() )
  {
    XTexture*   pTexture = *it;

    if ( pTexture->m_LoadedFromFile.empty() )
    {
      // eine bloss erzeugte Textur
      SDLTexture*    pNativeTexture = (SDLTexture*)pTexture;

      // sicher gehen
      pNativeTexture->Release();
        /*
      if ( SUCCEEDED( m_pd3dDevice->CreateTexture(
                      pTexture->m_SurfaceSize.x,
                      pTexture->m_SurfaceSize.y,
                      pDX9Texture->m_dwMipMapLevels,
                      0,
                      ( (CDX9Texture*)pTexture )->m_d3dfPixelFormat,
                      D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                      &( (CDX9Texture*)pTexture )->m_Surface,
                      NULL ) ) )
      {
      }
      else
      {
        dh::Log( "CreateTexture failed" );
      }
      GR::u32     MipMapLevel = 0;
      std::list<GR::Graphic::ImageData>::iterator   itID( pDX9Texture->m_StoredImageData.begin() );
      while ( itID != pDX9Texture->m_StoredImageData.end() )
      {
        CopyDataToTexture( pDX9Texture, *itID, 0, MipMapLevel );

        ++itID;
        ++MipMapLevel;
      }*/
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
        pData->ConvertSelfTo( pTexture->m_ImageFormat, 0, pTexture->m_ColorKey != 0, pTexture->m_ColorKey );

        pTexture->Release();

        /*
        if ( SUCCEEDED( m_pd3dDevice->CreateTexture(
                        pTexture->m_SurfaceSize.x,
                        pTexture->m_SurfaceSize.y,
                        ( (CDX9Texture*)pTexture )->m_dwMipMapLevels,
                        0,
                        ( (CDX9Texture*)pTexture )->m_d3dfPixelFormat,
                        D3DPOOL_MANAGED,//D3DPOOL_DEFAULT,//D3DPOOL_MANAGED,
                        &( (CDX9Texture*)pTexture )->m_Surface,
                        NULL ) ) )
        {
          CopyDataToTexture( pTexture, *pData );

          // Mipmap-Levels einlesen
          std::list<GR::String>::iterator    it( ( (CDX9Texture*)pTexture )->m_listFileNames.begin() );

          GR::u32     dwLevel = 1;

          while ( it != ( (CDX9Texture*)pTexture )->m_listFileNames.end() )
          {
            GR::String&  strPath( *it );

            GR::Graphic::ImageData*   pData = LoadAndConvert( strPath.c_str(), (GR::Graphic::eImageFormat)( (CDX9Texture*)pTexture )->m_ImageFormat, ( (CDX9Texture*)pTexture )->m_ColorKey );
            if ( pData == NULL )
            {
              dh::Error( "DX9Renderer:: RestoreAllTextures failed to load MipMap (%s)", strPath.c_str() );
            }
            else
            {
              CopyDataToTexture( pTexture, *pData, ( (CDX9Texture*)pTexture )->m_ColorKey, dwLevel );
              delete pData;
            }
            ++it;
            ++dwLevel;
          }
        }
        delete pData;*/
      }
    }

    ++it;
  }
}



bool SDLRenderClass::OnResized()
{
  //return ResizeSurface();
  return true;
}



bool SDLRenderClass::SetViewport( const XViewport& Viewport )
{
  /*
  D3DVIEWPORT9    viewPort;

  viewPort.Width    = Viewport.Width;
  viewPort.Height   = Viewport.Height;
  viewPort.X        = Viewport.X;
  viewPort.Y        = Viewport.Y;
  viewPort.MinZ     = Viewport.MinZ;
  viewPort.MaxZ     = Viewport.MaxZ;

  if ( viewPort.X >= Width() )
  {
    viewPort.X = Width();
    viewPort.Width = 0;
  }
  if ( viewPort.Y >= Height() )
  {
    viewPort.Y = Height();
    viewPort.Height = 0;
  }
  if ( viewPort.X + viewPort.Width > Width() )
  {
    viewPort.Width = Width() - viewPort.X;
  }
  if ( viewPort.Y + viewPort.Height> Height() )
  {
    viewPort.Height = Height() - viewPort.Y;
  }

  if ( FAILED( m_pd3dDevice->SetViewport( &viewPort ) ) )
  {
    return false;
  }*/
  m_ViewPort.X        = Viewport.X;
  m_ViewPort.Y        = Viewport.Y;
  m_ViewPort.Width    = Viewport.Width;
  m_ViewPort.Height   = Viewport.Height;
  m_ViewPort.MinZ     = Viewport.MinZ;
  m_ViewPort.MaxZ     = Viewport.MaxZ;
  return true;
}



bool SDLRenderClass::SetTrueViewport( const XViewport& Viewport )
{
  m_ViewPort.X        = Viewport.X;
  m_ViewPort.Y        = Viewport.Y;
  m_ViewPort.Width    = Viewport.Width;
  m_ViewPort.Height   = Viewport.Height;
  m_ViewPort.MinZ     = Viewport.MinZ;
  m_ViewPort.MaxZ     = Viewport.MaxZ;
  return true;
}



GR::u32 SDLRenderClass::Width()
{
  //return m_dwRenderWidth;
  return 0;
}



GR::u32 SDLRenderClass::Height()
{
  //return m_dwRenderHeight;
  return 0;
}



GR::Graphic::eImageFormat SDLRenderClass::ImageFormat()
{
  //return MapFormat( m_d3dpp.BackBufferFormat );
  return GR::Graphic::IF_A8R8G8B8;
}



bool SDLRenderClass::IsFullscreen()
{
  return !m_Windowed;
}



bool SDLRenderClass::ToggleFullscreen()
{
  m_Windowed = !m_Windowed;

  //return SUCCEEDED( Resize3DEnvironment() );
  return true;
}



bool SDLRenderClass::SetMode( XRendererDisplayMode& DisplayMode )
{
  tListDisplayModes::iterator   it( m_DisplayModes.begin() );
  while ( it != m_DisplayModes.end() )
  {
    XRendererDisplayMode&   DispMode = *it;
    if ( DispMode.FormatIsEqualTo( DisplayMode ) )
    {
      // der Mode existiert
      m_Windowed = !DisplayMode.FullScreen;
      if ( !m_Windowed )
      {
        //m_FullscreenDisplayMode = DisplayMode;
      }

      return false;
      //return SUCCEEDED( Resize3DEnvironment() );
    }

    ++it;
  }
  dh::Log( "SDLRenderClass::SetMode Try to set invalid mode %dx%d,%d", DisplayMode.Width, DisplayMode.Height, DisplayMode.ImageFormat );
  return false;
}



void SDLRenderClass::RenderLine2d( const GR::tPoint& pt1, const GR::tPoint& pt2, GR::u32 dwColor1, GR::u32 dwColor2, float fZ )
{
  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
  }
}



void SDLRenderClass::RenderLine( const GR::tVector& vect1, const GR::tVector& vect2, GR::u32 dwColor1, GR::u32 dwColor2 )
{
  if ( dwColor2 == 0 )
  {
    dwColor2 = dwColor1;
  }
}



bool SDLRenderClass::SetLight( GR::u32 dwLightIndex, XLight& Light )
{
  if ( dwLightIndex >= 8 )
  {
    return false;
  }

  m_Light[dwLightIndex] = Light;

  return true;
}



bool SDLRenderClass::SetMaterial( const XMaterial& Material )
{
  m_Material = Material;

  return true;
}



void SDLRenderClass::DoSaveSnapShot( const char* szFile )
{
}



bool SDLRenderClass::SaveScreenShot( const char* szFileName )
{
  if ( szFileName )
  {
    //m_strSaveScreenShotFileName = szFileName;
    return true;
  }
  return false;
}



bool SDLRenderClass::RenderMesh( const Mesh::IMesh& Mesh )
{
  SDLVertexBuffer   vbTemp;

  vbTemp.Create( Mesh.FaceCount() * 3, XVertexBuffer::VFF_XYZ | XVertexBuffer::VFF_NORMAL | XVertexBuffer::VFF_DIFFUSE | XVertexBuffer::VFF_TEXTURECOORD, XVertexBuffer::PrimitiveType::PT_TRIANGLE );

  vbTemp.FillFromMesh( Mesh );

  return RenderVertexBuffer( &vbTemp );
}



void SDLRenderClass::UnloadAssets( Xtreme::Asset::eXAssetType Type )
{
  Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
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



void SDLRenderClass::ReleaseAssets()
{
  if ( m_pEnvironment )
  {
    Xtreme::Asset::XAssetLoader* pLoader = (Xtreme::Asset::XAssetLoader*)m_pEnvironment->Service( "AssetLoader" );
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



bool SDLRenderClass::VSyncEnabled()
{
  return m_VSyncEnabled;
}



void SDLRenderClass::EnableVSync( bool Enable )
{
  if ( m_VSyncEnabled != Enable )
  {
    m_VSyncEnabled = Enable;
    //Resize3DEnvironment();
  }
}



void SDLRenderClass::SetRenderTarget( XTexture* pTexture )
{
}

