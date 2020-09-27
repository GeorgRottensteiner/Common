#include <debug/debugclient.h>

#include "DX11VertexBuffer.h"

#include <d3d11_2.h>

#include <Xtreme/XRenderer.h>
#include "DX11Renderer.h"



DX11VertexBuffer::DX11VertexBuffer( XRenderer* pRenderClass ) :
  XBasicVertexBuffer( pRenderClass ),
  m_PrimitivePartCount( 0 )
{
}



DX11VertexBuffer::~DX11VertexBuffer()
{
  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    ID3D11Buffer*   pD3DVB = (ID3D11Buffer*)*it;

    if ( pD3DVB )
    {
      pD3DVB->Release();
      pD3DVB = NULL;
    }

    ++it;
  }
  m_vectVertexBuffers.clear();
}



bool DX11VertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



bool DX11VertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( !XBasicVertexBuffer::Create( PrimitiveCount, VertexFormat, Type ) )
  {
    return false;
  }

  DX11Renderer*     pRenderer = (DX11Renderer*)m_pRenderClass;
  ID3D11Device2*    pDevice = (ID3D11Device2*)pRenderer->Device();
  if ( pDevice == NULL )
  {
    return false;
  }
  return CreateBuffers( pDevice );
}



bool DX11VertexBuffer::Restore()
{
  DX11Renderer*     pRenderer = (DX11Renderer*)m_pRenderClass;
  ID3D11Device2*    pDevice = (ID3D11Device2*)pRenderer->Device();
  if ( pDevice == NULL )
  {
    return false;
  }

  Release();

  if ( !CreateBuffers( pDevice ) )
  {
    return false;
  }

  UpdateData();

  return true;
}



void DX11VertexBuffer::Release()
{
  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    auto   pVertexBuffer = *it;

    if ( pVertexBuffer )
    {
      pVertexBuffer->Release();
    }

    ++it;
  }
  m_vectVertexBuffers.clear();
}



bool DX11VertexBuffer::CreateBuffers( ID3D11Device* pDevice )
{
  if ( pDevice == NULL )
  {
    return false;
  }

  Invalidate();


  DX11Renderer*     pRenderer = (DX11Renderer*)m_pRenderClass;

  int               maxPrimitiveCount = pRenderer->MaxPrimitiveCount();

  m_PrimitivePartCount = maxPrimitiveCount;

  // TODO - zu viele Vertices? does not exist anymore?
  /*
  if ( ( d3dCaps.MaxVertexIndex < m_PrimitiveCount * m_NumVerticesPerPrimitive )
  &&   ( d3dCaps.MaxVertexIndex != 0 ) )
  {
    m_PrimitivePartCount = d3dCaps.MaxVertexIndex / m_NumVerticesPerPrimitive;
  }*/

  DWORD   Primitives = m_PrimitiveCount;

  GR::u8* pData = (GR::u8*)m_pData;

  while ( Primitives )
  {
    DWORD     primitivesThisBuffer = Primitives;

    if ( primitivesThisBuffer > m_PrimitivePartCount )
    {
      primitivesThisBuffer = m_PrimitivePartCount;
    }

    D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
    vertexBufferData.pSysMem          = pData;
    vertexBufferData.SysMemPitch      = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC vertexBufferDesc( (UINT)( primitivesThisBuffer * m_PrimitiveSize * m_NumVerticesPerPrimitive ), D3D11_BIND_VERTEX_BUFFER );

    // both flags so we can write new data to the existing buffer
    vertexBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer*             pVertexBuffer = NULL;

    HRESULT     hr = S_OK;
    if ( FAILED( hr = pDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &pVertexBuffer ) ) )
    {
      dh::Log( "DX11VertexBuffer::CreateVertexBuffer: CreateVertexBuffer failed" );
      dh::Log( "Tried with %d primitives, format %x", primitivesThisBuffer, m_VertexFormat );
      Invalidate();
      return false;
    }
    m_vectVertexBuffers.push_back( pVertexBuffer );

    pData += m_PrimitiveCount * m_PrimitiveSize * m_NumVerticesPerPrimitive;
    Primitives -= primitivesThisBuffer;
  }

  /*
  // TODO - is there a limit on primitive count?
  D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
  vertexBufferData.pSysMem          = m_pData;
  vertexBufferData.SysMemPitch      = 0;
  vertexBufferData.SysMemSlicePitch = 0;

  CD3D11_BUFFER_DESC vertexBufferDesc( m_PrimitiveCount * m_PrimitiveSize * m_NumVerticesPerPrimitive, D3D11_BIND_VERTEX_BUFFER );

  // both flags so we can write new data to the existing buffer
  vertexBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
  vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  ID3D11Buffer*             pVertexBuffer = NULL;

  Invalidate();
  HRESULT     hr = S_OK;
  if ( FAILED( hr = pDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &pVertexBuffer ) ) )
  {
    return false;
  }
  m_vectVertexBuffers.push_back( pVertexBuffer );
  */
  return true;
}



void DX11VertexBuffer::Invalidate()
{
  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    auto   pD3DVB = (ID3D11Buffer*)*it;

    if ( pD3DVB )
    {
      pD3DVB->Release();
      pD3DVB = NULL;
    }

    ++it;
  }
  m_vectVertexBuffers.clear();
}



bool DX11VertexBuffer::Display( GR::u32 Index, GR::u32 Count )
{
  DX11Renderer*           pRenderer = (DX11Renderer*)m_pRenderClass;
  ID3D11Device*           pDevice = (ID3D11Device*)pRenderer->Device();
  ID3D11DeviceContext*    pDeviceContext = (ID3D11DeviceContext*)pRenderer->DeviceContext();
  if ( ( pDevice == NULL )
  ||   ( pDeviceContext == NULL ) )
  {
    return false;
  }

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  auto inputLayout = pRenderer->InputLayout( m_VertexFormat );
  if ( inputLayout == NULL )
  {
    dh::Log( "DX11VertexBuffer::Display missing input layout for vertex format %x", m_VertexFormat );
    return false;
  }
  pDeviceContext->IASetInputLayout( inputLayout );
  pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

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

    if ( ( Index >= m_NumVerticesPerPrimitive * PrimCopied )
    &&   ( Index < m_NumVerticesPerPrimitive * ( PrimCopied + PrimitivesToCopy ) ) )
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

      pDeviceContext->Draw( m_NumVerticesPerPrimitive * CopyNow, m_NumVerticesPerPrimitive * ( Index - PrimCopied ) );

      Count        -= CopyNow;
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



bool DX11VertexBuffer::DisplayLine( GR::u32 Index, GR::u32 Count )
{
  DX11Renderer*           pRenderer = (DX11Renderer*)m_pRenderClass;
  ID3D11Device*           pDevice = (ID3D11Device*)pRenderer->Device();
  ID3D11DeviceContext*    pDeviceContext = (ID3D11DeviceContext*)pRenderer->DeviceContext();
  if ( ( pDevice == NULL )
  ||   ( pDeviceContext == NULL ) )
  {
    return false;
  } 

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  auto inputLayout = pRenderer->InputLayout( m_VertexFormat );
  if ( inputLayout == NULL )
  {
    dh::Log( "DX11VertexBuffer::Display missing input layout for vertex format %x", m_VertexFormat );
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
  }
  return true;
}



void DX11VertexBuffer::UpdateData()
{
  DX11Renderer*           pRenderer = (DX11Renderer*)m_pRenderClass;
  ID3D11Device*           pDevice = (ID3D11Device*)pRenderer->Device();
  ID3D11DeviceContext*    pDeviceContext = (ID3D11DeviceContext*)pRenderer->DeviceContext();
  if ( ( pDevice == NULL )
  ||   ( pDeviceContext == NULL ) )
  {
    return;
  }

  GR::u8*   pData = (GR::u8*)m_pData;

  GR::u32   PrimitivesLeft = m_PrimitiveCount;

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    ID3D11Buffer*   pVB = *it;

    GR::u32   PrimitivesToCopy = PrimitivesLeft;
    if ( PrimitivesToCopy > m_PrimitivePartCount )
    {
      PrimitivesToCopy = m_PrimitivePartCount;
    }
    if ( m_PrimitivePartCount == 0 )
    {
      PrimitivesToCopy = PrimitivesLeft;
    }

    D3D11_MAPPED_SUBRESOURCE      subResource;

    if ( FAILED( pDeviceContext->Map( pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource ) ) )
    {
      dh::Log( "DX11VertexBuffer::UpdateData failed" );
      return;
    }

    GR::u8* pPos = ( GR::u8* )subResource.pData;

    // TODO - could be a problem, doesn't take pitch and stride in account!
    memcpy( pPos, pData, PrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive );

    pDeviceContext->Unmap( pVB, 0 );

    pData += PrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive;

    PrimitivesLeft -= PrimitivesToCopy;

    ++it;
  }
}



void DX11VertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, Color, vectTextureCoord );

  CreateBuffers( ( (DX11Renderer*)m_pRenderClass )->Device() );
  UpdateData();
}



void DX11VertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                  const GR::f32 fRHW,
                                  const GR::u32 Color,
                                  const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, fRHW, Color, vectTextureCoord );


  CreateBuffers( ( (DX11Renderer*)m_pRenderClass )->Device() );
  UpdateData();
}



bool DX11VertexBuffer::SetData( void* pData, GR::u32 NumVertices )
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



GR::up DX11VertexBuffer::VertexSize( GR::u32 VertexFormat )
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



void DX11VertexBuffer::SetVertex( size_t iVertexIndex,
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



