#include ".\VulkanPixelShader.h"
#include ".\VulkanRenderer.h"

#include <IO/FileUtil.h>

#include <Memory/ByteBuffer.h>

#include <Xtreme/XBasicVertexBuffer.h>



VulkanPixelShader::VulkanPixelShader( VulkanRenderer* pRenderer ) :
  m_pRenderer( pRenderer )
{
}



void VulkanPixelShader::Release()
{
}



bool VulkanPixelShader::CreateFromBuffers( const ByteBuffer& CompiledShader )
{
  Release();

  if ( CompiledShader.Empty() )
  {
    return false;
  }
  /*
  if ( FAILED( m_pRenderer->Device()->CreatePixelShader( CompiledShader.Data(), CompiledShader.Size(), nullptr, &m_pShader ) ) )
  {
    Release();
    return false;
  }*/
  m_CompiledShader = CompiledShader;
  return true;
}



bool VulkanPixelShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( CompiledShaderFile.empty() )
  {
    return false;
  }
  ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( CompiledShaderFile.c_str() );

  /*
  if ( FAILED( m_pRenderer->Device()->CreatePixelShader( shaderData.Data(), shaderData.Size(), nullptr, &m_pShader ) ) )
  {
    Release();
    return false;
  }*/
  m_CompiledShader = shaderData;
  return true;
}



