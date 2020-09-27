#include ".\DX11PixelShader.h"
#include ".\DX11Renderer.h"

#include <IO/FileUtil.h>

#include <Memory/ByteBuffer.h>

#include <Xtreme/XBasicVertexBuffer.h>



DX11PixelShader::DX11PixelShader( DX11Renderer* pRenderer ) :
  m_pShader( NULL ),
  m_pRenderer( pRenderer )
{
}



void DX11PixelShader::Release()
{
  if ( m_pShader )
  {
    m_pShader->Release();
    m_pShader = NULL;
  }
}



bool DX11PixelShader::CreateFromBuffers( const ByteBuffer& CompiledShader )
{
  Release();

  if ( CompiledShader.Empty() )
  {
    return false;
  }
  if ( FAILED( m_pRenderer->Device()->CreatePixelShader( CompiledShader.Data(), CompiledShader.Size(), nullptr, &m_pShader ) ) )
  {
    Release();
    return false;
  }
  m_CompiledShader = CompiledShader;
  return true;
}



bool DX11PixelShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( CompiledShaderFile.empty() )
  {
    return false;
  }
  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( CompiledShaderFile.c_str() );

  if ( FAILED( m_pRenderer->Device()->CreatePixelShader( shaderData.Data(), shaderData.Size(), nullptr, &m_pShader ) ) )
  {
    Release();
    return false;
  }
  m_CompiledShader = shaderData;
  return true;
}



