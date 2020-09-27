#include ".\dx9shader.h"



DX9Shader::DX9Shader() :
  m_pShader( NULL )
{
}



void DX9Shader::Release()
{

  if ( m_pShader )
  {
    m_pShader->Release();
    m_pShader = NULL;
  }

}