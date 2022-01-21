#include "VertexShader.h"
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



VertexShader::VertexShader( OpenGLShaderRenderClass* pRenderer ) : //, GR::u32 VertexFormat ) :
  m_pRenderer( pRenderer ),
  m_ShaderID( 0 )
  //, m_VertexFormat( VertexFormat )
{
}



VertexShader::~VertexShader()
{
  Release();
}



void VertexShader::Release()
{
  if ( m_ShaderID != 0 )
  {
    glDeleteShader( m_ShaderID );
    m_ShaderID = 0;
  }
}



bool VertexShader::CreateFromString( const GR::String& ShaderContent )
{
  Release();

  m_ShaderID = glCreateShader( GL_VERTEX_SHADER );

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
    dh::Log( "VertexShader Error: %s", &shaderErrorMessage[0] );
  }
  return true;
}



bool VertexShader::CreateFromFile( const GR::String& CompiledShaderFile )
{
  Release();

  if ( !CompiledShaderFile.empty() )
  {
    m_ShaderScript = GR::IO::FileUtil::ReadFileAsString( CompiledShaderFile );

    return CreateFromString( m_ShaderScript );
  }
  return true;
}



