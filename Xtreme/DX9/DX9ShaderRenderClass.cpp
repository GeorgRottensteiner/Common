#include <debug/debugclient.h>

#include <Grafik/ImageFormate/ImageFormatManager.h>
#include <Grafik/ContextDescriptor.h>

#include <Xtreme/XAsset/XAssetLoader.h>
#include <Xtreme/XAsset/XAssetImage.h>
#include <Xtreme/XAsset/XAssetImageSection.h>
#include <Xtreme/XAsset/XAssetFont.h>
#include <Xtreme/XAsset/XAssetMesh.h>

#include <Xtreme/MeshFormate/T3DLoader.h>

#include <String/XML.h>

#include <Misc/Misc.h>

#include <IO/FileStream.h>
#include <IO/FileUtil.h>

#include "DX9ShaderRenderClass.h"
#include "DX9Texture.h"
#include "DX9VertexBuffer.h"

#include <d3DX9.h>


#pragma comment ( lib, "d3DX9.lib" )
#pragma comment ( lib, "d3d9.lib" )
#pragma comment ( lib, "dxguid.lib" )



DX9ShaderRenderClass::DX9ShaderRenderClass( HINSTANCE hInstance ) :
  CDX9RenderClass( hInstance )
{
}



DX9ShaderRenderClass::~DX9ShaderRenderClass()
{
}



HRESULT DX9ShaderRenderClass::ConfirmDevice( D3DCAPS9* pCaps, DWORD Behaviour, D3DFORMAT Format )
{

  if ( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
  {
    return E_FAIL;
  }

  return S_OK;

}



bool DX9ShaderRenderClass::InitDeviceObjects()
{

  if ( !CDX9RenderClass::InitDeviceObjects() )
  {
    return false;
  }

  CreatePixelShaderFromFile( "Flat.Light.0", AppPath( "flat_light_0.psc" ).c_str() );
  CreatePixelShaderFromFile( "Flat.Light.1", AppPath( "flat_light_1.psc" ).c_str() );

  CreatePixelShaderFromFile( "FlatNoTexture.Light.0", AppPath( "flat_notex_light_0.psc" ).c_str() );

  return true;

}



bool DX9ShaderRenderClass::InvalidateDeviceObjects()
{

  return CDX9RenderClass::InvalidateDeviceObjects();

}



bool DX9ShaderRenderClass::RestoreDeviceObjects()
{

  return CDX9RenderClass::RestoreDeviceObjects();

}



bool DX9ShaderRenderClass::DeleteDeviceObjects()
{

  DestroyAllShaders();

  return CDX9RenderClass::DeleteDeviceObjects();

}



void DX9ShaderRenderClass::DestroyAllShaders()
{

  std::map<GR::String,DX9Shader>::iterator   itShader( m_Shader.begin() );
  while ( itShader != m_Shader.end() )
  {
    DX9Shader&    shader( itShader->second );

    shader.Release();

    ++itShader;
  }
  m_Shader.clear();

}



DX9Shader DX9ShaderRenderClass::CreatePixelShaderFromBuffer( const GR::String& PixelShaderKey, const ByteBuffer& ShaderData )
{

  if ( ShaderData.Empty() )
  {
    return DX9Shader();
  }
  DX9Shader   shader;

  if ( FAILED( m_pd3dDevice->CreatePixelShader( (const DWORD*)ShaderData.Data(), &shader.m_pShader ) ) )
  {
    return DX9Shader();
  }
  m_Shader[PixelShaderKey] = shader;

  return shader;

}



DX9Shader DX9ShaderRenderClass::CreatePixelShaderFromFile( const GR::String& PixelShaderKey, const char* Filename )
{

  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( Filename );
  if ( shaderData.Empty() )
  {
    return DX9Shader();
  }

  return CreatePixelShaderFromBuffer( PixelShaderKey, shaderData );

}



bool DX9ShaderRenderClass::SetPixelShader( const GR::String& PixelShaderKey )
{

  std::map<GR::String,DX9Shader>::const_iterator   itSH( m_Shader.find( PixelShaderKey ) );
  if ( itSH == m_Shader.end() )
  {
    return false;
  }
  return SetPixelShader( itSH->second );

}



bool DX9ShaderRenderClass::SetPixelShader( const DX9Shader& Shader )
{

  if ( Shader.m_pShader == NULL )
  {
    return false;
  }

  return SUCCEEDED( m_pd3dDevice->SetPixelShader( Shader.m_pShader ) );

}



void DX9ShaderRenderClass::SetShader( eShaderType sType )
{

  switch ( sType )
  {
    case ST_FLAT:
      SetState( XRenderer::RS_ALPHATEST, XRenderer::RSV_DISABLE );
      SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_DISABLE );
      SetPixelShader( "Flat.Light.0" );
      return;
    case ST_FLAT_NO_TEXTURE:
      SetState( XRenderer::RS_ALPHATEST, XRenderer::RSV_DISABLE );
      SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_DISABLE );
      SetTexture( 0, NULL );
      SetPixelShader( "FlatNoTexture.Light.0" );
      break;
    case ST_ALPHA_BLEND:
      SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_ENABLE );
      SetPixelShader( "Flat.Light.0" );
      break;
    case ST_50_PERCENT_BLEND:
      break;
    case ST_ALPHA_TEST:
      SetState( XRenderer::RS_ALPHATEST, XRenderer::RSV_ENABLE );
      SetPixelShader( "Flat.Light.0" );
      break;
    case ST_ALPHA_BLEND_AND_TEST:
      SetState( XRenderer::RS_ALPHATEST, XRenderer::RSV_ENABLE );
      SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_ENABLE );
      SetPixelShader( "Flat.Light.0" );
      break;
    case ST_ADDITIVE:
      break;
  }
  XBasicRenderer::SetShader( sType );

}



