#include "PixelShader.h"
#include "OpenGLShaderRenderClass.h"

#include <IO/FileUtil.h>

#include <Memory/ByteBuffer.h>

#include <Xtreme/XBasicVertexBuffer.h>



extern PFNGLATTACHSHADERPROC         glAttachShader;
extern PFNGLCOMPILESHADERPROC        glCompileShader;
extern PFNGLCREATEPROGRAMPROC        glCreateProgram;
extern PFNGLCREATESHADERPROC         glCreateShader;
extern PFNGLDELETEPROGRAMPROC        glDeleteProgram;
extern PFNGLDELETESHADERPROC         glDeleteShader;
extern PFNGLDETACHSHADERPROC         glDetachShader;
extern PFNGLSHADERSOURCEPROC         glShaderSource;
extern PFNGLGETSHADERIVPROC          glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC     glGetShaderInfoLog;



PixelShader::PixelShader( OpenGLShaderRenderClass* pRenderer ) :
  m_pRenderer( pRenderer ),
  m_ShaderID( 0 )
{
}



PixelShader::~PixelShader()
{
  Release();
}



void PixelShader::Release()
{
  if ( m_ShaderID != 0 )
  {
    glDeleteShader( m_ShaderID );
    m_ShaderID = 0;
  }
}



bool PixelShader::CreateFromString( const GR::String& ShaderContent )
{
  Release();

  m_ShaderID = glCreateShader( GL_FRAGMENT_SHADER );

  // Compile vertex Shader
  GLint Result = GL_FALSE;
  int InfoLogLength;

  char const* FragmentSourcePointer = (const char*)ShaderContent.c_str();
  glShaderSource( m_ShaderID, 1, &FragmentSourcePointer, NULL );
  glCompileShader( m_ShaderID );

  // Check Fragment Shader
  glGetShaderiv( m_ShaderID, GL_COMPILE_STATUS, &Result );
  glGetShaderiv( m_ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );
  // driver return length 1 with included 0
  if ( InfoLogLength > 1 )
  {
    std::vector<char> shaderErrorMessage( InfoLogLength + 1 );
    glGetShaderInfoLog( m_ShaderID, InfoLogLength, NULL, &shaderErrorMessage[0] );
    dh::Log( "PixelShader Error: %s", &shaderErrorMessage[0] );
  }
  return true;
}



bool PixelShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( CompiledShaderFile.empty() )
  {
    return false;
  }

  m_ShaderScript = GR::IO::FileUtil::ReadFileAsString( CompiledShaderFile );

  return CreateFromString( m_ShaderScript );
}



