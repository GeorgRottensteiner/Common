#include "QuadCache.h"
#include "OpenGLShaderRenderClass.h"
#include "OpenGLShaderVertexBuffer.h"



void QuadCache::VertexEntry::SetColor( GR::u32 Color )
{
  Diffuse = Color;
}



QuadCache::~QuadCache()
{
  if ( m_pBuffer != NULL )
  {
    m_pRenderer->DestroyVertexBuffer( m_pBuffer );
    m_pBuffer = NULL;
  }
}



QuadCache::QuadCache( OpenGLShaderRenderClass* pRenderer, GR::u32 VertexFormat ) :
  m_pBuffer( NULL ),
  m_pRenderer( pRenderer ),
  m_VertexFormat( VertexFormat ),
  m_UsedVertices( 0 ),
  m_MatricesChanged( false )
{
  QuadInfo.resize( 128 );
  Vertices.resize( 128 * 6 );
}



void QuadCache::AddEntry( const XTextureSection& Section, const GR::tVector& Pos, const GR::tVector& Size, GR::u32 Color,
                              XRenderer::eShaderType Shader )
{
  GR::tFPoint     tu1, tu2, tu3, tu4;

  Section.GetTrueUV( tu1, tu2, tu3, tu4 );

  AddEntry( Section.m_pTexture,
            Pos, tu1.x, tu1.y,
            GR::tVector( Pos.x + Size.x, Pos.y, Pos.z ), tu2.x, tu2.y,
            GR::tVector( Pos.x, Pos.y + Size.y, Pos.z ), tu3.x, tu3.y,
            GR::tVector( Pos.x + Size.x, Pos.y + Size.y, Pos.z ), tu4.x, tu4.y,
            Color, Color, Color, Color,
            Shader );
}



void QuadCache::AddEntry( XTexture* pTexture,
                              const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                              const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                              const GR::tVector& Pos3, GR::f32 TU3, GR::f32 TV3,
                              const GR::tVector& Pos4, GR::f32 TU4, GR::f32 TV4,
                              GR::u32 Color1,
                              GR::u32 Color2,
                              GR::u32 Color3,
                              GR::u32 Color4,
                              XRenderer::eShaderType Shader )
{
  if ( ( Vertices.size() >= 6 * 512 )
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
  auto&   v4 = Vertices[m_UsedVertices + 3];
  auto&   v5 = Vertices[m_UsedVertices + 4];
  auto&   v6 = Vertices[m_UsedVertices + 5];

  v1.Pos      = Pos1;
  v1.TextureCoord.set( TU1, TV1 );
  v1.SetColor( Color1 );
  v2.Pos = Pos2;
  v2.TextureCoord.set( TU2, TV2 );
  v2.SetColor( Color2 );
  v3.Pos = Pos3;
  v3.TextureCoord.set( TU3, TV3 );
  v3.SetColor( Color3 );

  v4 = v3;
  v5 = v2;

  v6.Pos = Pos4;
  v6.TextureCoord.set( TU4, TV4 );
  v6.SetColor( Color4 );

  math::vector3     vectDummy1,
                    vectDummy2;

  vectDummy1 = v1.Pos - v2.Pos;
  vectDummy2 = v1.Pos - v3.Pos;

  vectDummy1 = vectDummy1.cross( vectDummy2 );
  vectDummy1.normalize();

  v1.Normal = vectDummy1;
  v2.Normal = vectDummy1;
  v3.Normal = vectDummy1;
  v4.Normal = vectDummy1;
  v5.Normal = vectDummy1;
  v6.Normal = vectDummy1;

  auto&  entry = QuadInfo[m_UsedVertices / 6];

  entry.pTexture = pTexture;
  entry.Shader = Shader;

  if ( m_MatricesChanged )
  {
    entry.MatricesChanged = true;
    entry.Projection  = m_CurrentProjection;
    entry.View        = m_CurrentView;
    entry.World       = m_CurrentWorld;
    m_MatricesChanged = false;
  }

  if ( Shader == XRenderer::ST_FLAT_NO_TEXTURE )
  {
    entry.pTexture = NULL;
  }
  m_UsedVertices += 6;
}



void QuadCache::EnsureBuffer()
{
  if ( m_pBuffer != NULL )
  {
    return;
  }
  m_pBuffer = (OpenGLShaderVertexBuffer*)m_pRenderer->CreateVertexBuffer( (GR::u32)Vertices.size() / 3, m_VertexFormat, XVertexBuffer::PT_TRIANGLE );
}



void QuadCache::GrowCache()
{
  QuadInfo.resize( QuadInfo.size() * 2 );
  Vertices.resize( Vertices.size() * 2 );

  if ( m_pBuffer != NULL )
  {
    m_pRenderer->DestroyVertexBuffer( m_pBuffer );
    m_pBuffer = NULL;
  }
  EnsureBuffer();
}



void QuadCache::FlushCache()
{
  m_MatricesChanged = false;

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

  while ( curPrimitiveIndex < m_UsedVertices / 6 )
  {
    auto& entry( QuadInfo[curPrimitiveIndex] );

    // no stored settings yet?
    if ( currentShader == XRenderer::ST_INVALID )
    {
      currentShader   = entry.Shader;
      pCurrentTexture = entry.pTexture;
    }
    else if ( ( currentShader != entry.Shader )
    ||        ( entry.MatricesChanged )
    ||        ( pCurrentTexture != entry.pTexture ) )
    {
      // something changes, flush
      DrawPart( startPrimitive, curPrimitiveIndex - startPrimitive );
      startPrimitive = curPrimitiveIndex;
      currentShader = entry.Shader;
      pCurrentTexture = entry.pTexture;

      if ( entry.MatricesChanged )
      {
        m_pRenderer->SetTransform( XRenderer::TT_PROJECTION, entry.Projection );
        m_pRenderer->SetTransform( XRenderer::TT_VIEW, entry.View );
        m_pRenderer->SetTransform( XRenderer::TT_WORLD, entry.World );
      }
    }

    ++curPrimitiveIndex;
  }
  if ( startPrimitive < m_UsedVertices / 6 )
  {
    DrawPart( startPrimitive, m_UsedVertices / 6 - startPrimitive );
  }
  m_UsedVertices = 0;

  // restore last set texture
  m_pRenderer->SetTexture( 0, pNewTexture );

  // restore last set shader
  m_pRenderer->SetShader( currentRendererShader );
}



void QuadCache::DrawPart( int StartPrimitive, int PrimitiveCount )
{
  m_pRenderer->SetShader( QuadInfo[StartPrimitive].Shader );
  m_pRenderer->SetTexture( 0, QuadInfo[StartPrimitive].pTexture );
  //dh::Log( "Draw Part Quad %d %x", StartPrimitive, PrimitiveCount );
  // direct rendering, do not call RenderVertexBuffer
  m_pRenderer->ChooseShaders( m_pBuffer->VertexFormat() );
  m_pBuffer->Display( 2 * StartPrimitive, 2 * PrimitiveCount );
}



void QuadCache::TransformChanged( XRenderer::eTransformType TransformType )
{
  // TODO - store transforms!
  /*
  m_CurrentProjection = m_pRenderer->Matrix( XRenderer::TT_PROJECTION );
  m_CurrentView       = m_pRenderer->Matrix( XRenderer::TT_VIEW );
  m_CurrentWorld      = m_pRenderer->Matrix( XRenderer::TT_WORLD );
  m_MatricesChanged   = true;*/

  FlushCache();
}