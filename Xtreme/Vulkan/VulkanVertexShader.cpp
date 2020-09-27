#include ".\VulkanVertexShader.h"
#include ".\VulkanRenderer.h"

#include <IO/FileUtil.h>

#include <Memory/ByteBuffer.h>

#include <Xtreme/XBasicVertexBuffer.h>



VulkanVertexShader::VulkanVertexShader( VulkanRenderer* pRenderer, GR::u32 VertexFormat ) :
  m_pRenderer( pRenderer ),
  m_VertexFormat( VertexFormat )
{
}



void VulkanVertexShader::Release()
{
}



bool VulkanVertexShader::CreateFromBuffers( const ByteBuffer& CompiledShader )
{
  Release();

  if ( !CompiledShader.Empty() )
  {
    /*
    if ( FAILED( m_pRenderer->Device()->CreateVertexShader( CompiledShader.Data(), CompiledShader.Size(), nullptr, &m_pShader ) ) )
    {
      return false;
    }*/
    m_CompiledShader = CompiledShader;
  }
  if ( !CreateInputLayout() )
  {
    Release();
    return false;
  }
  return true;
}



bool VulkanVertexShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( !CompiledShaderFile.empty() )
  {
    ByteBuffer    shaderData = GR::IO::FileUtil::ReadFileAsBuffer( CompiledShaderFile.c_str() );

    /*
    if ( FAILED( m_pRenderer->Device()->CreateVertexShader( shaderData.Data(), shaderData.Size(), nullptr, &m_pShader ) ) )
    {
      return false;
    }*/
    m_CompiledShader = shaderData;
  }
  if ( !CreateInputLayout() )
  {
    Release();
    return false;
  }
  return true;
}



bool VulkanVertexShader::CreateInputLayout()
{
  return true;
}


