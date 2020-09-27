#include <debug/debugclient.h>

#include ".\dx8vertexbuffer.h"

#include <d3d8.h>
#include <d3dx8.h>


#include <Xtreme/XRenderer.h>
#include <Xtreme/DX8/DX8RenderClass.h>



CDX8VertexBuffer::CDX8VertexBuffer( XRenderer* pRenderClass ) :
  XBasicVertexBuffer( pRenderClass ),
  m_d3dPrimType( D3DPT_TRIANGLELIST ),
  m_PrimitivePartCount( 0 )
{
}

CDX8VertexBuffer::~CDX8VertexBuffer()
{
  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   pD3DVB = (IDirect3DVertexBuffer8*)*it;

    if ( pD3DVB )
    {
      pD3DVB->Release();
      pD3DVB = NULL;
    }

    ++it;
  }
  m_vectVertexBuffers.clear();
}



bool CDX8VertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



bool CDX8VertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( !XBasicVertexBuffer::Create( PrimitiveCount, VertexFormat, Type ) )
  {
    return false;
  }

  m_d3dPool               = D3DPOOL_DEFAULT;
  m_Usage               = D3DUSAGE_WRITEONLY;

  return CreateBuffers( ( (CDX8RenderClass*)m_pRenderClass )->Device() );
}



bool CDX8VertexBuffer::Restore()
{
  CDX8RenderClass*    pDX8Render = (CDX8RenderClass*)m_pRenderClass;

  if ( !pDX8Render->Device() )
  {
    return false;
  }

  Release();

  if ( !CreateBuffers( pDX8Render->Device() ) )
  {
    return false;
  }

  UpdateData();

  return true;
}



void CDX8VertexBuffer::Release()
{
  if ( !m_vectVertexBuffers.empty() )
  {
    tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
    while ( it != m_vectVertexBuffers.end() )
    {
      IDirect3DVertexBuffer8*   pVertexBuffer = *it;

      if ( pVertexBuffer )
      {
        pVertexBuffer->Release();
      }

      ++it;
    }
    m_vectVertexBuffers.clear();
  }
}



bool CDX8VertexBuffer::CreateBuffers( LPDIRECT3DDEVICE8 pd3dDevice )
{
  if ( pd3dDevice == NULL )
  {
    return false;
  }

  D3DCAPS8    d3dCaps;
  
  if ( FAILED( pd3dDevice->GetDeviceCaps( &d3dCaps ) ) )
  {
    return false;
  }

  Invalidate();

  m_PrimitivePartCount = d3dCaps.MaxPrimitiveCount;
  // zu viele Vertices?
  if ( ( d3dCaps.MaxVertexIndex < m_PrimitiveCount * m_NumVerticesPerPrimitive )
  &&   ( d3dCaps.MaxVertexIndex != 0 ) )
  {
    m_PrimitivePartCount = d3dCaps.MaxVertexIndex / m_NumVerticesPerPrimitive;
  }

  DWORD   Primitives = m_PrimitiveCount;

  while ( Primitives )
  {
    DWORD     PrimitivesThisBuffer = Primitives;

    if ( PrimitivesThisBuffer > m_PrimitivePartCount )
    {
      PrimitivesThisBuffer = m_PrimitivePartCount;
    }

    IDirect3DVertexBuffer8*   pVertexBuffer = NULL;

    // HINT: m_VertexFormat is compatible to D3D VertexFormat flags
    HRESULT hr = pd3dDevice->CreateVertexBuffer( PrimitivesThisBuffer * m_PrimitiveSize * m_NumVerticesPerPrimitive,
                                                 m_Usage, 
                                                 m_VertexFormat, 
                                                 m_d3dPool, 
                                                 &pVertexBuffer );

    if ( hr != D3D_OK )
    {
      dh::Log( "CDX8VertexBuffer::CreateVertexBuffer: CreateVertexBuffer failed" );
      dh::Log( "Tried with %d primitives, format %x", PrimitivesThisBuffer, m_VertexFormat );
      Invalidate();
      return false;
    }

    m_vectVertexBuffers.push_back( pVertexBuffer );

    Primitives -= PrimitivesThisBuffer;
  }
  return true;
}



void CDX8VertexBuffer::Invalidate()
{
  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   pD3DVB = (IDirect3DVertexBuffer8*)*it;

    if ( pD3DVB )
    {
      pD3DVB->Release();
      pD3DVB = NULL;
    }

    ++it;
  }
  m_vectVertexBuffers.clear();
}



bool CDX8VertexBuffer::Display( GR::u32 Index, GR::u32 Count )
{
  CDX8RenderClass*    pDX8Render = (CDX8RenderClass*)m_pRenderClass;


  // HINT: m_VertexFormat is compatible to D3D VertexFormat flags
  HRESULT   hRes = pDX8Render->Device()->SetVertexShader( m_VertexFormat );
  if ( FAILED( hRes ) )
  {
    dh::Log( "SetVertexShader failed" );
  }

  DWORD     Primitives = m_PrimitiveCount,
            PrimCopied = 0;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   iVB = *it;

    DWORD   PrimitivesToCopy = Primitives;
    if ( PrimitivesToCopy > m_PrimitivePartCount )
    {
      PrimitivesToCopy = m_PrimitivePartCount;
    }

    if ( ( Index >= m_NumVerticesPerPrimitive * PrimCopied )
    &&   ( Index < m_NumVerticesPerPrimitive * ( PrimCopied + PrimitivesToCopy ) ) )
    {
      DWORD   CopyNow = Count;
      if ( CopyNow > PrimitivesToCopy )
      {
        CopyNow = PrimitivesToCopy;
      }
      HRESULT   hRes = pDX8Render->Device()->SetStreamSource( 0, iVB, (UINT)m_PrimitiveSize );
      if ( FAILED( hRes ) )
      {
        dh::Log( "Display failed %x", hRes );
      }

      hRes = pDX8Render->Device()->DrawPrimitive( m_d3dPrimType,
                                                  Index - PrimCopied,
                                                  CopyNow );
      if ( FAILED( hRes ) )
      {
        dh::Log( "Display failed %x", hRes );
      }

      Count -= CopyNow;
      PrimCopied += CopyNow;
    }

    if ( Count == 0 )
    {
      break;
    }

    ++it;
  }

  return true;

}



void CDX8VertexBuffer::UpdateData()
{
  GR::u8*   pData = (GR::u8*)m_pData;

  GR::u32   PrimitivesLeft = m_PrimitiveCount;

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    IDirect3DVertexBuffer8*   pVB = *it;

    GR::u32   PrimitivesToCopy = PrimitivesLeft;
    if ( PrimitivesToCopy > m_PrimitivePartCount )
    {
      PrimitivesToCopy = m_PrimitivePartCount;
    }

    GR::u8*   pPos;

    pVB->Lock( 0, 0, &pPos, 0 );

    memcpy( pPos, pData, PrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive );

    pVB->Unlock();

    pData += PrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive;

    PrimitivesLeft -= PrimitivesToCopy;

    ++it;
  }
}



void CDX8VertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, Color, vectTextureCoord );

  CreateBuffers( ( (CDX8RenderClass*)m_pRenderClass )->Device() );
  UpdateData();
}



void CDX8VertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::f32 fRHW,
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, fRHW, Color, vectTextureCoord );


  CreateBuffers( ( (CDX8RenderClass*)m_pRenderClass )->Device() );
  UpdateData();
}



bool CDX8VertexBuffer::SetData( void* pData, GR::u32 NumVertices )
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



