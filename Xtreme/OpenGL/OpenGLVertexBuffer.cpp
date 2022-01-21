#include <debug/debugclient.h>

#include "OpenGLVertexBuffer.h"

#include <Xtreme/XRenderer.h>
#include "OpenGLRenderClass.h"



#if ( OPERATING_SYSTEM == OS_WEB )
#include <emscripten.h>
#else
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#endif


typedef void ( APIENTRY* PFNGLBINDBUFFERPROC ) ( GLenum target, GLuint buffer );
typedef void ( APIENTRY* PFNGLDELETEBUFFERSPROC ) ( GLsizei n, const GLuint* buffers );
typedef void ( APIENTRY* PFNGLGENBUFFERSPROC ) ( GLsizei n, GLuint* buffers );

#if ( OPERATING_SYSTEM != OS_WEB )
typedef void ( APIENTRY* PFNGLBUFFERDATAPROC ) ( GLenum target, int size, const GLvoid* data, GLenum usage );
#endif
extern PFNGLBUFFERDATAPROC    glBufferData;					// VBO Data Loading Procedure

// VBO Extension Function Pointers
extern PFNGLGENBUFFERSPROC    glGenBuffers;					// VBO Name Generation Procedure
extern PFNGLBINDBUFFERPROC    glBindBuffer;					// VBO Bind Procedure
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;			// VBO Deletion Procedure




OpenGLVertexBuffer::OpenGLVertexBuffer( XRenderer* pRenderClass ) :
  XBasicVertexBuffer( pRenderClass ),
  m_PrimitivePartCount( 0 ),
  m_VertexBufferID( 0 ),
  m_TextureCoordinatesBufferID( 0 )
{
}



OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
  Release();
}



bool OpenGLVertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



bool OpenGLVertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( !XBasicVertexBuffer::Create( PrimitiveCount, VertexFormat, Type ) )
  {
    return false;
  }

  return CreateBuffers();
}



bool OpenGLVertexBuffer::Restore()
{
  OpenGLRenderClass*     pRenderer = (OpenGLRenderClass*)m_pRenderClass;

  Release();

  if ( !CreateBuffers() )
  {
    return false;
  }

  UpdateData();

  return true;
}



void OpenGLVertexBuffer::Release()
{
  if ( m_VertexBufferID != 0 )
  {
    glDeleteBuffers( 1, &m_VertexBufferID );
    m_VertexBufferID = 0;
  }
  if ( m_TextureCoordinatesBufferID != 0 )
  {
    glDeleteBuffers( 1, &m_TextureCoordinatesBufferID );
    m_TextureCoordinatesBufferID = 0;
  }
}



bool OpenGLVertexBuffer::CreateBuffers()
{
  Invalidate();


  OpenGLRenderClass*     pRenderer = (OpenGLRenderClass*)m_pRenderClass;

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
  return true;
}



void OpenGLVertexBuffer::Invalidate()
{
  Release();
}



bool OpenGLVertexBuffer::Display( GR::u32 Index, GR::u32 Count )
{
  OpenGLRenderClass*           pRenderer = (OpenGLRenderClass*)m_pRenderClass;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );

  if ( ( m_iOffsetXYZ != -1 )
  &&   ( m_iOffsetRHW != -1 ) )
  {
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetXYZ != -1 )
  {
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetXYZ );
  }
  else if ( m_iOffsetRHW != -1 )
  {
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 1, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetRHW );
  }
  if ( m_iOffsetNormal != -1 )
  {
    glEnableClientState( GL_NORMAL_ARRAY );
    glNormalPointer( GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetNormal );
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    glEnableClientState( GL_COLOR_ARRAY );
    glColorPointer( 4, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetDiffuse );
  }
  if ( m_iOffsetTU != -1 )
  {
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer( 2, GL_FLOAT, m_PrimitiveSize, (void*)m_iOffsetTU );
  }

  pRenderer->CheckError( "OpenGLVertexBuffer::Display" );
  glDrawArrays( GL_TRIANGLES, 0, Count * 3 );

  pRenderer->CheckError( "OpenGLVertexBuffer::Display 2" );
  glDisableClientState( GL_VERTEX_ARRAY );
  glDisableClientState( GL_TEXTURE_COORD_ARRAY );
  glDisableClientState( GL_COLOR_ARRAY );
  glDisableClientState( GL_NORMAL_ARRAY );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  pRenderer->CheckError( "OpenGLVertexBuffer::Display 3" );
  return true;
}



bool OpenGLVertexBuffer::DisplayLine( GR::u32 Index, GR::u32 Count )
{
  OpenGLRenderClass*           pRenderer = (OpenGLRenderClass*)m_pRenderClass;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  /*
  auto inputLayout = pRenderer->InputLayout( m_VertexFormat );
  if ( inputLayout == NULL )
  {
    dh::Log( "OpenGLVertexBuffer::Display missing input layout for vertex format %x", m_VertexFormat );
    return false;
  }
  pDeviceContext->IASetInputLayout( inputLayout );
  pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

  UINT stride = (UINT)m_PrimitiveSize;
  UINT offset = 0;

  DWORD     Primitives = m_PrimitiveCount,
            PrimCopied = 0;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    DWORD   PrimitivesToCopy = Primitives;
    if ( PrimitivesToCopy > m_PrimitivePartCount )
    {
      PrimitivesToCopy = m_PrimitivePartCount;
    }

    if ( ( Index >= 2 * PrimCopied )
      && ( Index < 2 * ( PrimCopied + PrimitivesToCopy ) ) )
    {
      DWORD   CopyNow = Count;
      if ( CopyNow > PrimitivesToCopy )
      {
        CopyNow = PrimitivesToCopy;
      }

      pDeviceContext->IASetVertexBuffers( 0,
        1,
        &( *it ),
        &stride,
        &offset );

      pDeviceContext->Draw( 2 * CopyNow, 2 * ( Index - PrimCopied ) );

      Count -= CopyNow;
      PrimCopied += CopyNow;
    }

    if ( Count == 0 )
    {
      break;
    }

    ++it;
  }*/
  return true;
}



void OpenGLVertexBuffer::UpdateData()
{
  OpenGLRenderClass*           pRenderer = (OpenGLRenderClass*)m_pRenderClass;

  GR::u8*   pData = (GR::u8*)m_pData;

  glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferID );
  pRenderer->CheckError( "UpdateData glBindBuffer" );

  glBufferData( GL_ARRAY_BUFFER, m_PrimitiveSize * m_NumVerticesPerPrimitive * m_PrimitiveCount, m_pData, GL_STATIC_DRAW );
  pRenderer->CheckError( "UpdateData glBufferData" );

  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  pRenderer->CheckError( "UpdateData glBindBuffer 2" );
}



void OpenGLVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, Color, vectTextureCoord );

  CreateBuffers();
  UpdateData();
}



void OpenGLVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::f32 fRHW,
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, fRHW, Color, vectTextureCoord );


  CreateBuffers();
  UpdateData();
}



bool OpenGLVertexBuffer::SetData( void* pData, GR::u32 NumVertices )
{
  if ( NumVertices > m_VertexCount )
  {
    dh::Log( "CDX8VertexBuffer::SetData setting too many vertices! (%d > %d)", NumVertices, m_VertexCount );
    return false;
  }

  memcpy( m_pData, pData, NumVertices * m_PrimitiveSize );
  UpdateData();
  return true;
}



GR::up OpenGLVertexBuffer::VertexSize( GR::u32 VertexFormat )
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
  }
  else if ( VertexFormat & VFF_XYZRHW )
  {
    m_iOffsetXYZ = ( GR::i32 )VertexSize;
    VertexSize += 3 * sizeof( GR::f32 );

    //m_iOffsetRHW = ( GR::i32 )VertexSize;
    //VertexSize += sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_NORMAL )
  {
    m_iOffsetNormal = ( GR::i32 )VertexSize;
    VertexSize += 3 * sizeof( GR::f32 );
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



void OpenGLVertexBuffer::SetVertex( size_t iVertexIndex,
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



