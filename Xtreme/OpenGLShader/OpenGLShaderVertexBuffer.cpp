#include <debug/debugclient.h>

#include "OpenGLShaderVertexBuffer.h"

#include <Xtreme/XRenderer.h>
#include "OpenGLShaderRenderClass.h"



#if ( OPERATING_SYSTEM == OS_WEB )
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#endif


#if ( OPERATING_SYSTEM != OS_WEB )
typedef void ( APIENTRY* PFNGLBINDBUFFERPROC ) ( GLenum target, GLuint buffer );
typedef void ( APIENTRY* PFNGLDELETEBUFFERSPROC ) ( GLsizei n, const GLuint* buffers );
typedef void ( APIENTRY* PFNGLGENBUFFERSPROC ) ( GLsizei n, GLuint* buffers );
typedef void ( APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC ) ( GLuint index );
typedef void ( APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC ) ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer );
typedef void ( APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC ) ( GLuint index );
typedef void ( APIENTRYP PFNGLGENVERTEXARRAYSPROC ) ( GLsizei n, GLuint* arrays );
typedef void ( APIENTRYP PFNGLBINDVERTEXARRAYPROC ) ( GLuint array );
typedef void ( APIENTRYP PFNGLDELETEVERTEXARRAYSPROC ) ( GLsizei n, const GLuint* arrays );

#endif

#if ( OPERATING_SYSTEM != OS_WEB )
typedef void ( APIENTRY* PFNGLBUFFERDATAPROC ) ( GLenum target, int size, const GLvoid* data, GLenum usage );
#endif

#if ( OPERATING_SYSTEM != OS_WEB )
extern PFNGLBUFFERDATAPROC    glBufferData;					// VBO Data Loading Procedure

// VBO Extension Function Pointers
extern PFNGLGENBUFFERSPROC    glGenBuffers;					// VBO Name Generation Procedure
extern PFNGLBINDBUFFERPROC    glBindBuffer;					// VBO Bind Procedure
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;			// VBO Deletion Procedure
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC  glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLGENVERTEXARRAYSPROC      glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC      glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC   glDeleteVertexArrays;
#endif



OpenGLShaderVertexBuffer::OpenGLShaderVertexBuffer( XRenderer* pRenderClass ) :
  XBasicVertexBuffer( pRenderClass ),
  m_PrimitivePartCount( 0 ),
  m_VertexBufferID( 0 ),
  m_VertexArrayObjectID( 0 )
{
}



OpenGLShaderVertexBuffer::~OpenGLShaderVertexBuffer()
{
  Release();
}



bool OpenGLShaderVertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



bool OpenGLShaderVertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( !XBasicVertexBuffer::Create( PrimitiveCount, VertexFormat, Type ) )
  {
    return false;
  }

  return CreateBuffers();
}



bool OpenGLShaderVertexBuffer::Restore()
{
  OpenGLShaderRenderClass*     pRenderer = (OpenGLShaderRenderClass*)m_pRenderClass;

  Release();

  if ( !CreateBuffers() )
  {
    return false;
  }

  UpdateData();

  return true;
}



void OpenGLShaderVertexBuffer::Release()
{
  if ( m_VertexBufferID != 0 )
  {
    glDeleteBuffers( 1, &m_VertexBufferID );
    m_VertexBufferID = 0;
  }
  if ( m_VertexArrayObjectID != 0 )
  {
    glDeleteVertexArrays( 1, &m_VertexArrayObjectID );
    m_VertexArrayObjectID = 0;
  }
}



bool OpenGLShaderVertexBuffer::CreateBuffers()
{
  Invalidate();


  OpenGLShaderRenderClass*     pRenderer = (OpenGLShaderRenderClass*)m_pRenderClass;

  pRenderer->CheckError( "pre glGenBuffers" );
  // Generate And Bind The Vertex Buffer
  glGenBuffers( 1, &m_VertexBufferID );                  // Get A Valid Name
  pRenderer->CheckError( "glGenBuffers" );
  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );         // Bind The Buffer
  pRenderer->CheckError( "glBindBuffer" );
  // Load The Data
  //GLenum target, int size, const GLvoid* data, GLenum usage
  glBufferData( GL_ARRAY_BUFFER, m_PrimitiveSize * m_NumVerticesPerPrimitive, m_pData, GL_STATIC_DRAW );
  pRenderer->CheckError( "glBufferData" );

  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glGenVertexArrays( 1, &m_VertexArrayObjectID );

  return true;
}



void OpenGLShaderVertexBuffer::Invalidate()
{
  Release();
}



bool OpenGLShaderVertexBuffer::Display( GR::u32 Index, GR::u32 Count )
{
  OpenGLShaderRenderClass*           pRenderer = (OpenGLShaderRenderClass*)m_pRenderClass;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  int attributeIndex = 0;
  
  pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display a" );

  glBindVertexArray( m_VertexArrayObjectID );

  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display b" );

  if ( ( m_iOffsetXYZ != -1 )
  &&   ( m_iOffsetRHW != -1 ) )
  {
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( attributeIndex, 4, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetXYZ );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetXYZ != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c2a" );
    glVertexAttribPointer( attributeIndex, 3, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetXYZ );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c2" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 3, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetRHW != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 1, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetRHW );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c3" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 1, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetRHW );
  }
  if ( m_iOffsetNormal != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 3, GL_FLOAT, GL_TRUE, m_PrimitiveSize, (void*)m_iOffsetNormal );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c4" );

    //glEnableClientState( GL_NORMAL_ARRAY );
    //glNormalPointer( GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetNormal );
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 4, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetDiffuse );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c5" );

    //glEnableClientState( GL_COLOR_ARRAY );
    //glColorPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetDiffuse );
  }
  if ( m_iOffsetTU != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 2, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetTU );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display c6" );

    //glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    //glTexCoordPointer( 2, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetTU );
  }

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display" );
  glDrawArrays( GL_TRIANGLES, 0, Count * 3 );

  for ( int i = 0; i < attributeIndex; ++i )
  {
    glDisableVertexAttribArray( i );
  }

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display 2" );
  //glDisableClientState( GL_VERTEX_ARRAY );
  //glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  //glDisableClientState( GL_COLOR_ARRAY );
  //glDisableClientState( GL_NORMAL_ARRAY );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
  pRenderer->CheckError( "OpenGLShaderVertexBuffer::Display 3" );
  return true;
}



bool OpenGLShaderVertexBuffer::DisplayLine( GR::u32 Index, GR::u32 Count )
{
  OpenGLShaderRenderClass*           pRenderer = (OpenGLShaderRenderClass*)m_pRenderClass;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  int attributeIndex = 0;

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine a" );

  glBindVertexArray( m_VertexArrayObjectID );

  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine b" );

  if ( ( m_iOffsetXYZ != -1 )
       && ( m_iOffsetRHW != -1 ) )
  {
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( attributeIndex, 4, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetXYZ );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetXYZ != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c2a" );
    glVertexAttribPointer( attributeIndex, 3, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetXYZ );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c2" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 3, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetRHW != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 1, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetRHW );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c3" );

    //glEnableClientState( GL_VERTEX_ARRAY );
    //glVertexPointer( 1, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetRHW );
  }
  if ( m_iOffsetNormal != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 3, GL_FLOAT, GL_TRUE, m_PrimitiveSize, (void*)m_iOffsetNormal );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c4" );

    //glEnableClientState( GL_NORMAL_ARRAY );
    //glNormalPointer( GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetNormal );
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 4, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetDiffuse );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c5" );

    //glEnableClientState( GL_COLOR_ARRAY );
    //glColorPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetDiffuse );
  }
  if ( m_iOffsetTU != -1 )
  {
    glEnableVertexAttribArray( attributeIndex );
    glVertexAttribPointer( attributeIndex, 2, GL_FLOAT, GL_FALSE, m_PrimitiveSize, (void*)m_iOffsetTU );
    ++attributeIndex;

    pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine c6" );

    //glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    //glTexCoordPointer( 2, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetTU );
  }

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine" );
  glDrawArrays( GL_LINES, 0, Count * 2 );

  for ( int i = 0; i < attributeIndex; ++i )
  {
    glDisableVertexAttribArray( i );
  }

  pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine 2" );
  //glDisableClientState( GL_VERTEX_ARRAY );
  //glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  //glDisableClientState( GL_COLOR_ARRAY );
  //glDisableClientState( GL_NORMAL_ARRAY );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
  pRenderer->CheckError( "OpenGLShaderVertexBuffer::DisplayLine 3" );
  return true;
}



void OpenGLShaderVertexBuffer::UpdateData()
{
  OpenGLShaderRenderClass*           pRenderer = (OpenGLShaderRenderClass*)m_pRenderClass;

  GR::u8*   pData = (GR::u8*)m_pData;

  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );
  pRenderer->CheckError( "UpdateData glBindBuffer" );

  glBufferData( GL_ARRAY_BUFFER, m_PrimitiveSize * m_NumVerticesPerPrimitive * m_PrimitiveCount, m_pData, GL_STATIC_DRAW );
  pRenderer->CheckError( "UpdateData glBufferData" );

  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  pRenderer->CheckError( "UpdateData glBindBuffer 2" );
}



void OpenGLShaderVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, Color, vectTextureCoord );

  CreateBuffers();
  UpdateData();
}



void OpenGLShaderVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::f32 fRHW,
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, fRHW, Color, vectTextureCoord );


  CreateBuffers();
  UpdateData();
}



bool OpenGLShaderVertexBuffer::SetData( void* pData, GR::u32 NumVertices )
{
  if ( NumVertices > m_VertexCount )
  {
    dh::Log( "OpenGLShaderVertexBuffer::SetData setting too many vertices! (%d > %d)", NumVertices, m_VertexCount );
    return false;
  }

  memcpy( m_pData, pData, NumVertices * m_PrimitiveSize );
  UpdateData();
  return true;
}



GR::up OpenGLShaderVertexBuffer::VertexSize( GR::u32 VertexFormat )
{
  if ( VertexFormat == 0 )
  {
    VertexFormat = m_VertexFormat;
  }

  GR::up   VertexSize = 0;

  if ( VertexFormat & VFF_XYZ )
  {
    m_iOffsetXYZ = ( GR::i32 )VertexSize;
    VertexSize += 3 * sizeof( GR::f32 );

    // vec3 is padded to vec4
    VertexSize += sizeof( GR::f32 );
  }
  else if ( VertexFormat & VFF_XYZRHW )
  {
    m_iOffsetXYZ = ( GR::i32 )VertexSize;
    VertexSize += 3 * sizeof( GR::f32 );

    // vec3 is padded to vec4
    m_iOffsetRHW = ( GR::i32 )VertexSize;
    VertexSize += sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_NORMAL )
  {
    m_iOffsetNormal = ( GR::i32 )VertexSize;
    VertexSize += 3 * sizeof( GR::f32 );

    // vec3 is padded to vec4
    VertexSize += sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_DIFFUSE )
  {
    // colors are now float[4]
    m_iOffsetDiffuse = ( GR::i32 )VertexSize;
    VertexSize += 4 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_SPECULAR )
  {
    m_iOffsetSpecular = ( GR::i32 )VertexSize;
    VertexSize += 4 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_TEXTURECOORD )
  {
    m_iOffsetTU = ( GR::i32 )VertexSize;
    VertexSize += 2 * sizeof( GR::f32 );
  }

  return VertexSize;
}



void OpenGLShaderVertexBuffer::SetVertex( size_t iVertexIndex,
                                  const GR::tVector& vectPos,
                                  const GR::tVector& vectNormal,
                                  const GR::f32 fRHW,
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  if ( m_PrimitiveSize == 0 )
  {
    return;
  }
  if ( iVertexIndex >= m_VertexCount )
  {
    dh::Log( "XBasicVertexBuffer::SetVertex Vertex out of bounds! (%d > %d)", iVertexIndex, m_VertexCount );
    return;
  }

  GR::u8*   pNewVertex = ( GR::u8* )m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetXYZ != -1 )
  {
    *( GR::tVector* )( pNewVertex + m_iOffsetXYZ ) = vectPos;
  }
  if ( m_iOffsetNormal != -1 )
  {
    *( GR::tVector* )( pNewVertex + m_iOffsetNormal ) = vectNormal;
  }
  /*
  if ( m_iOffsetRHW != -1 )
  {
    *( GR::f32* )( pNewVertex + m_iOffsetRHW ) = fRHW;
  }*/
  if ( m_iOffsetDiffuse != -1 )
  {
    // R
    ( ( GR::f32* )( pNewVertex + m_iOffsetDiffuse ) )[2] = ( ( Color >> 0 ) & 0xff ) / 255.0f;
    // G
    ( ( GR::f32* )( pNewVertex + m_iOffsetDiffuse ) )[1] = ( ( Color >> 8 ) & 0xff ) / 255.0f;
    // B
    ( ( GR::f32* )( pNewVertex + m_iOffsetDiffuse ) )[0] = ( ( Color >> 16 ) & 0xff ) / 255.0f;
    // A
    ( ( GR::f32* )( pNewVertex + m_iOffsetDiffuse ) )[3] = ( Color >> 24 ) / 255.0f;
  }
  if ( m_iOffsetTU != -1 )
  {
    ( *( GR::tFPoint* )( pNewVertex + m_iOffsetTU ) ) = vectTextureCoord;
  }
}



