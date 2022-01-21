#include "TriangleCache.h"
#include "OpenGLShaderVertexBuffer.h"
#include "OpenGLShaderRenderClass.h"
#include "OpenGLShaderVertexBuffer.h"


void TriangleCache::VertexEntry::SetColor( GR::u32 Color )
{
  Diffuse = Color;
}



TriangleCache::TriangleCache( OpenGLShaderRenderClass* pRenderer, GR::u32 VertexFormat ) :
  m_pBuffer( NULL ),
  m_pRenderer( pRenderer ),
  m_VertexFormat( VertexFormat ),
  m_UsedVertices( 0 )
{
  PrimitiveInfo.resize( 128 );
  Vertices.resize( 128 * 3 );
}



TriangleCache::~TriangleCache()
{
  if ( m_pBuffer != NULL )
  {
    m_pRenderer->DestroyVertexBuffer( m_pBuffer );
    m_pBuffer = NULL;
  }
}



void TriangleCache::AddEntry( XTexture* pTexture,
                              const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                              const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                              const GR::tVector& Pos3, GR::f32 TU3, GR::f32 TV3,
                              GR::u32 Color1,
                              GR::u32 Color2,
                              GR::u32 Color3,
                              XRenderer::eShaderType Shader )
{
  if ( ( Vertices.size() >= 3 * 512 )
  &&   ( m_UsedVertices == Vertices.size() ) )
  {
    FlushCache();
  }

  if ( m_UsedVertices >= (int)Vertices.size() )
  {
    GrowCache();
  }
  EnsureBuffer();

  // still space in the cache
  auto&   v1 = Vertices[m_UsedVertices];
  auto&   v2 = Vertices[m_UsedVertices + 1];
  auto&   v3 = Vertices[m_UsedVertices + 2];

  v1.Pos      = Pos1;
  v1.TextureCoord.set( TU1, TV1 );
  v1.SetColor( Color1 );
  v2.Pos = Pos2;
  v2.TextureCoord.set( TU2, TV2 );
  v2.SetColor( Color2 );
  v3.Pos = Pos3;
  v3.TextureCoord.set( TU3, TV3 );
  v3.SetColor( Color3 );

  math::vector3     vectDummy1,
                    vectDummy2;

  vectDummy1 = v1.Pos - v2.Pos;
  vectDummy2 = v1.Pos - v3.Pos;

  vectDummy1 = vectDummy1.cross( vectDummy2 );
  vectDummy1.normalize();

  v1.Normal = vectDummy1;
  v2.Normal = vectDummy1;
  v3.Normal = vectDummy1;

  auto&  entry = PrimitiveInfo[m_UsedVertices / 3];

  entry.pTexture = pTexture;
  entry.Shader = Shader;

  if ( Shader == XRenderer::ST_FLAT_NO_TEXTURE )
  {
    entry.pTexture = NULL;
  }
  m_UsedVertices += 3;
}



void TriangleCache::EnsureBuffer()
{
  if ( m_pBuffer != NULL )
  {
    return;
  }
  m_pBuffer = (OpenGLShaderVertexBuffer*)m_pRenderer->CreateVertexBuffer( (GR::u32)Vertices.size() / 3, m_VertexFormat, XVertexBuffer::PT_TRIANGLE );
}



void TriangleCache::GrowCache()
{
  PrimitiveInfo.resize( PrimitiveInfo.size() * 2 );
  Vertices.resize( Vertices.size() * 2 );

  if ( m_pBuffer != NULL )
  {
    m_pRenderer->DestroyVertexBuffer( m_pBuffer );
    m_pBuffer = NULL;
  }
  EnsureBuffer();
}



void TriangleCache::FlushCache()
{
  if ( m_UsedVertices == 0 )
  {
    return;
  }

  XTexture*   pNewTexture = m_pRenderer->m_pSetTextures[0];

  EnsureBuffer();

  for ( int i = 0; i < m_UsedVertices; ++i )
  {
    auto& vertex( Vertices[i] );
    m_pBuffer->SetVertex( i, vertex.Pos, vertex.Normal, 0.0f, vertex.Diffuse, vertex.TextureCoord );
  }
  m_pBuffer->UpdateData();

  // TODO - optimize to allow direct copy of memory!
  //m_pBuffer->SetData( &Vertices[0], m_UsedVertices );

  // draw 
  int                       startPrimitive = 0;
  XTexture*                 pCurrentTexture = (XTexture*)-1;
  XRenderer::eShaderType    currentShader = XRenderer::ST_INVALID;
  XRenderer::eShaderType    currentRendererShader = m_pRenderer->m_CurrentShaderType;

  int     curPrimitiveIndex = 0;

  while ( curPrimitiveIndex < m_UsedVertices / 3 )
  {
    auto& entry( PrimitiveInfo[curPrimitiveIndex] );

    // no stored settings yet?
    if ( currentShader == XRenderer::ST_INVALID )
    {
      currentShader   = entry.Shader;
      pCurrentTexture = entry.pTexture;
    }
    else if ( ( currentShader != entry.Shader )
    ||        ( pCurrentTexture != entry.pTexture ) )
    {
      // something changes, flush
      DrawPart( startPrimitive, curPrimitiveIndex - startPrimitive );
      startPrimitive = curPrimitiveIndex;
      currentShader = entry.Shader;
      pCurrentTexture = entry.pTexture;
    }

    ++curPrimitiveIndex;
  }
  if ( startPrimitive < m_UsedVertices / 3 )
  {
    DrawPart( startPrimitive, m_UsedVertices / 3 - startPrimitive );
  }
  m_UsedVertices = 0;

  // restore last set texture
  m_pRenderer->SetTexture( 0, pNewTexture );

  // restore last set shader
  m_pRenderer->SetShader( currentRendererShader );
}



void TriangleCache::DrawPart( int StartPrimitive, int PrimitiveCount )
{
  m_pRenderer->SetShader( PrimitiveInfo[StartPrimitive].Shader );
  m_pRenderer->SetTexture( 0, PrimitiveInfo[StartPrimitive].pTexture );
  //dh::Log( "Draw Part Quad %d %x", StartPrimitive, PrimitiveCount );
  // direct rendering, do not call RenderVertexBuffer
  m_pRenderer->ChooseShaders( m_pBuffer->VertexFormat() );
  m_pBuffer->Display( StartPrimitive, PrimitiveCount );
}



void TriangleCache::TransformChanged( XRenderer::eTransformType TransformType )
{
  // TODO - store transforms!
  FlushCache();
}