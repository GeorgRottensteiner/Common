#include ".\DX11VertexShader.h"
#include ".\DX11Renderer.h"

#include <IO/FileUtil.h>

#include <Memory/ByteBuffer.h>

#include <Xtreme/XBasicVertexBuffer.h>



DX11VertexShader::DX11VertexShader( DX11Renderer* pRenderer, GR::u32 VertexFormat ) :
  m_pShader( NULL ),
  m_pInputLayout( NULL ),
  m_pRenderer( pRenderer ),
  m_VertexFormat( VertexFormat )
{
}



void DX11VertexShader::Release()
{
  if ( m_pInputLayout )
  {
    m_pInputLayout->Release();
    m_pInputLayout = NULL;
  }
  if ( m_pShader )
  {
    m_pShader->Release();
    m_pShader = NULL;
  }
}



bool DX11VertexShader::CreateFromBuffers( const ByteBuffer& CompiledShader )
{
  Release();

  if ( !CompiledShader.Empty() )
  {
    if ( FAILED( m_pRenderer->Device()->CreateVertexShader( CompiledShader.Data(), CompiledShader.Size(), nullptr, &m_pShader ) ) )
    {
      return false;
    }
    m_CompiledShader = CompiledShader;
  }
  if ( !CreateInputLayout() )
  {
    Release();
    return false;
  }
  return true;
}



bool DX11VertexShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( !CompiledShaderFile.empty() )
  {
    ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( CompiledShaderFile.c_str() );

    if ( FAILED( m_pRenderer->Device()->CreateVertexShader( shaderData.Data(), shaderData.Size(), nullptr, &m_pShader ) ) )
    {
      return false;
    }
    m_CompiledShader = shaderData;
  }
  if ( !CreateInputLayout() )
  {
    Release();
    return false;
  }
  return true;
}



void DX11VertexShader::CreateElementDescription( std::vector<D3D11_INPUT_ELEMENT_DESC>& InputDesc )
{
  GR::up    vertexSize = 0;
  int       offsetXYZ = 0;
  int       offsetRHW = 0;
  int       offsetNormal = 0;
  int       offsetDiffuse = 0;
  int       offsetSpecular = 0;
  int       offsetTU = 0;

  if ( m_VertexFormat & XBasicVertexBuffer::VFF_XYZ )
  {
    offsetXYZ = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );
  }
  else if ( m_VertexFormat & XBasicVertexBuffer::VFF_XYZRHW )
  {
    offsetXYZ = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );
    /*
    offsetRHW = ( GR::i32 )vertexSize;
    vertexSize += sizeof( GR::f32 );*/
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_NORMAL )
  {
    offsetNormal = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_DIFFUSE )
  {
    offsetDiffuse = ( GR::i32 )vertexSize;
    //vertexSize += sizeof( GR::u32 );
    vertexSize += 4 * sizeof( GR::f32 );
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_SPECULAR )
  {
    offsetSpecular = ( GR::i32 )vertexSize;
    //vertexSize += sizeof( GR::u32 );
    vertexSize += 4 * sizeof( GR::f32 );
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_TEXTURECOORD )
  {
    offsetTU = ( GR::i32 )vertexSize;
    vertexSize += 2 * sizeof( GR::f32 );
  }

  if ( m_VertexFormat & XBasicVertexBuffer::VFF_XYZ )
  {
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)offsetXYZ, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }
  else if ( m_VertexFormat & XBasicVertexBuffer::VFF_XYZRHW )
  {
    // TODO
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)offsetXYZ, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
    //InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "RHW", 0, DXGI_FORMAT_R32_FLOAT, 0, offsetRHW, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }

  if ( m_VertexFormat & XBasicVertexBuffer::VFF_NORMAL )
  {
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, (UINT)offsetNormal, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_DIFFUSE )
  {
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetDiffuse, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }
  if ( m_VertexFormat & XBasicVertexBuffer::VFF_SPECULAR )
  {
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "SPECULAR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetSpecular, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }

  if ( m_VertexFormat & XBasicVertexBuffer::VFF_TEXTURECOORD )
  {
    InputDesc.push_back( D3D11_INPUT_ELEMENT_DESC() = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetTU, D3D11_INPUT_PER_VERTEX_DATA, 0 } );
  }
}



bool DX11VertexShader::CreateInputLayout()
{
  std::vector<D3D11_INPUT_ELEMENT_DESC>      inputDescs;

  CreateElementDescription( inputDescs );

  if ( FAILED( m_pRenderer->Device()->CreateInputLayout( &inputDescs[0],
                                                         (UINT)inputDescs.size(),
                                                         m_CompiledShader.Data(),
                                                         (UINT)m_CompiledShader.Size(),
                                                         &m_pInputLayout ) ) )
  {
    return false;
  }
  return true;
}


