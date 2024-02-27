#include "DX11LineCache.h"
#include "DX11Renderer.h"
#include "DX11VertexBuffer.h"



void DX11LineCache::VertexEntry::SetColor( GR::u32 Color )
{
  // R
  Diffuse[2] = ( ( Color >> 0 ) & 0xff ) / 255.0f;
  // G
  Diffuse[1] = ( ( Color >> 8 ) & 0xff ) / 255.0f;
  // B
  Diffuse[0] = ( ( Color >> 16 ) & 0xff ) / 255.0f;
  // A
  Diffuse[3] = ( Color >> 24 ) / 255.0f;
}



DX11LineCache::DX11LineCache( DX11Renderer* pRenderer, GR::u32 VertexFormat ) :
  m_pBuffer( NULL ),
  m_pRenderer( pRenderer ),
  m_VertexFormat( VertexFormat ),
  m_UsedVertices( 0 )
{
  LineInfo.resize( 128 );
  Vertices.resize( 128 * 2 );
}



void DX11LineCache::AddEntry( XTexture* pTexture, const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color,
                              XRenderer::eShaderType Shader )
{
  AddEntry( pTexture,
            Pos1, 0.0f, 0.0f,
            Pos2, 1.0f, 1.0f,
            Color, Color,
            Shader );
}



void DX11LineCache::AddEntry( XTexture* pTexture,
                              const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                              const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                              GR::u32 Color1,
                              GR::u32 Color2,
                              XRenderer::eShaderType Shader )
{
  if ( ( Vertices.size() >= 2 * 512 )
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

  v1.Pos      = Pos1;
  v1.TextureCoord.Set( TU1, TV1 );
  v1.SetColor( Color1 );
  v2.Pos = Pos2;
  v2.TextureCoord.Set( TU2, TV2 );
  v2.SetColor( Color2 );

  auto&  entry = LineInfo[m_UsedVertices / 2];

  entry.pTexture = pTexture;
  entry.Shader = Shader;

  if ( Shader == XRenderer::ST_FLAT_NO_TEXTURE )
  {
    entry.pTexture = NULL;
  }
  m_UsedVertices += 2;
}



void DX11LineCache::EnsureBuffer()
{
  if ( m_pBuffer != NULL )
  {
    return;
  }
  m_pBuffer = (DX11VertexBuffer*)m_pRenderer->CreateVertexBuffer( (GR::u32)Vertices.size() / 2, m_VertexFormat, XVertexBuffer::PT_LINE );
}



void DX11LineCache::GrowCache()
{
  LineInfo.resize( LineInfo.size() * 2 );
  Vertices.resize( Vertices.size() * 2 );

  if ( m_pBuffer != NULL )
  {
    m_pRenderer->DestroyVertexBuffer( m_pBuffer );
    m_pBuffer = NULL;
  }
  EnsureBuffer();
}



void DX11LineCache::FlushCache()
{
  if ( m_UsedVertices == 0 )
  {
    return;
  }

  XTexture*   pNewTexture = m_pRenderer->m_SetTextures[0];

  EnsureBuffer();
  m_pBuffer->SetData( &Vertices[0], m_UsedVertices );

  // draw 
  int                       startPrimitive = 0;
  XTexture*                 pCurrentTexture = (XTexture*)-1;
  XRenderer::eShaderType    currentShader = XRenderer::ST_INVALID;
  XRenderer::eShaderType    currentRendererShader = m_pRenderer->m_CurrentShaderType;

  int     curPrimitiveIndex = 0;

  while ( curPrimitiveIndex < m_UsedVertices / 2 )
  {
    auto& entry( LineInfo[curPrimitiveIndex] );

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
  if ( startPrimitive < m_UsedVertices / 2 )
  {
    DrawPart( startPrimitive, m_UsedVertices / 2 - startPrimitive );
  }
  m_UsedVertices = 0;

  // restore last set texture
  m_pRenderer->SetTexture( 0, pNewTexture );

  // restore last set shader
  m_pRenderer->SetShader( currentRendererShader );
}



void DX11LineCache::DrawPart( int StartPrimitive, int PrimitiveCount )
{
  m_pRenderer->SetShader( LineInfo[StartPrimitive].Shader );
  m_pRenderer->SetTexture( 0, LineInfo[StartPrimitive].pTexture );
  //dh::Log( "Draw Part Quad %d %x", StartPrimitive, PrimitiveCount );
  // direct rendering, do not call RenderVertexBuffer
  m_pRenderer->ChooseShaders( m_pBuffer->VertexFormat() );
  m_pBuffer->DisplayLine( StartPrimitive, PrimitiveCount );
}



void DX11LineCache::TransformChanged( XRenderer::eTransformType TransformType )
{
  // TODO - store transforms!
  FlushCache();
}