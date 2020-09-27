#include <debug/debugclient.h>

#include "VulkanVertexBuffer.h"

#include <Xtreme/XRenderer.h>
#include "VulkanRenderer.h"



VulkanVertexBuffer::VulkanVertexBuffer( XRenderer* pRenderClass ) :
  XBasicVertexBuffer( pRenderClass ),
  m_PrimitivePartCount( 0 )
{
}



VulkanVertexBuffer::~VulkanVertexBuffer()
{
  /*
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
  */
}



bool VulkanVertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



bool VulkanVertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( !XBasicVertexBuffer::Create( PrimitiveCount, VertexFormat, Type ) )
  {
    return false;
  }

  VulkanRenderer*     pRenderer = (VulkanRenderer*)m_pRenderClass;
    /*
  ID3D11Device2*    pDevice = (ID3D11Device2*)pRenderer->Device();
  if ( pDevice == NULL )
  {
    return false;
  }*/
  //return CreateBuffers( pDevice );
  return false;
}



bool VulkanVertexBuffer::Restore()
{
  VulkanRenderer*     pRenderer = (VulkanRenderer*)m_pRenderClass;
  /* ID3D11Device2*    pDevice = (ID3D11Device2*)pRenderer->Device();
  if ( pDevice == NULL )
  {
    return false;
  }

  Release();

  if ( !CreateBuffers( pDevice ) )
  {
    return false;
  }*/

  UpdateData();

  return true;
}



void VulkanVertexBuffer::Release()
{
  /*
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
  */
}



/*
bool VulkanVertexBuffer::CreateBuffers( ID3D11Device* pDevice )
{
  if ( pDevice == NULL )
  {
    return false;
  }

  Invalidate();


  VulkanRenderer*     pRenderer = (VulkanRenderer*)m_pRenderClass;

  int               maxPrimitiveCount = pRenderer->MaxPrimitiveCount();

  m_PrimitivePartCount = maxPrimitiveCount;

  // TODO - zu viele Vertices? does not exist anymore?

  DWORD   dwPrimitives = m_PrimitiveCount;

  GR::u8* pData = (GR::u8*)m_pData;

  while ( dwPrimitives )
  {
    DWORD     primitivesThisBuffer = dwPrimitives;

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
      dh::Log( "VulkanVertexBuffer::CreateVertexBuffer: CreateVertexBuffer failed" );
      dh::Log( "Tried with %d primitives, format %x", primitivesThisBuffer, m_VertexFormat );
      Invalidate();
      return false;
    }
    m_vectVertexBuffers.push_back( pVertexBuffer );

    pData += m_PrimitiveCount * m_PrimitiveSize * m_NumVerticesPerPrimitive;
    dwPrimitives -= primitivesThisBuffer;
  }
  return true;
}
*/



void VulkanVertexBuffer::Invalidate()
{
  /*
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
  */
}



bool VulkanVertexBuffer::Display( GR::u32 Index, GR::u32 Count )
{
  VulkanRenderer*           pRenderer = (VulkanRenderer*)m_pRenderClass;
  /*
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
    dh::Log( "VulkanVertexBuffer::Display missing input layout for vertex format %x", m_VertexFormat );
    return false;
  }
  pDeviceContext->IASetInputLayout( inputLayout );
  pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

  UINT stride = (UINT)m_PrimitiveSize;
  UINT offset = 0;

  DWORD     dwPrimitives = m_PrimitiveCount,
            dwPrimCopied = 0;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    DWORD   dwPrimitivesToCopy = dwPrimitives;
    if ( dwPrimitivesToCopy > m_PrimitivePartCount )
    {
      dwPrimitivesToCopy = m_PrimitivePartCount;
    }

    if ( ( Index >= m_NumVerticesPerPrimitive * dwPrimCopied )
    &&   ( Index < m_NumVerticesPerPrimitive * ( dwPrimCopied + dwPrimitivesToCopy ) ) )
    {
      DWORD   dwCopyNow = Count;
      if ( dwCopyNow > dwPrimitivesToCopy )
      {
        dwCopyNow = dwPrimitivesToCopy;
      }

      pDeviceContext->IASetVertexBuffers( 0,
                                          1,
                                          &( *it ),
                                          &stride,
                                          &offset );

      pDeviceContext->Draw( m_NumVerticesPerPrimitive * dwCopyNow, m_NumVerticesPerPrimitive * ( Index - dwPrimCopied ) );

      Count        -= dwCopyNow;
      dwPrimCopied += dwCopyNow;
    }

    if ( Count == 0 )
    {
      break;
    }

    ++it;
  }*/
  return true;
}



bool VulkanVertexBuffer::DisplayLine( GR::u32 Index, GR::u32 Count )
{
  /*
  VulkanRenderer*           pRenderer = (VulkanRenderer*)m_pRenderClass;
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
    dh::Log( "VulkanVertexBuffer::Display missing input layout for vertex format %x", m_VertexFormat );
    return false;
  }
  pDeviceContext->IASetInputLayout( inputLayout );
  pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

  UINT stride = (UINT)m_PrimitiveSize;
  UINT offset = 0;

  DWORD     dwPrimitives = m_PrimitiveCount,
            dwPrimCopied = 0;

  if ( Count == 0 )
  {
    Count = m_PrimitiveCount;
  }

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    DWORD   dwPrimitivesToCopy = dwPrimitives;
    if ( dwPrimitivesToCopy > m_PrimitivePartCount )
    {
      dwPrimitivesToCopy = m_PrimitivePartCount;
    }

    if ( ( Index >= 2 * dwPrimCopied )
      && ( Index < 2 * ( dwPrimCopied + dwPrimitivesToCopy ) ) )
    {
      DWORD   dwCopyNow = Count;
      if ( dwCopyNow > dwPrimitivesToCopy )
      {
        dwCopyNow = dwPrimitivesToCopy;
      }

      pDeviceContext->IASetVertexBuffers( 0,
        1,
        &( *it ),
        &stride,
        &offset );

      pDeviceContext->Draw( 2 * dwCopyNow, 2 * ( Index - dwPrimCopied ) );

      Count -= dwCopyNow;
      dwPrimCopied += dwCopyNow;
    }

    if ( Count == 0 )
    {
      break;
    }

    ++it;
  }*/
  return true;
}



void VulkanVertexBuffer::UpdateData()
{
  VulkanRenderer*           pRenderer = (VulkanRenderer*)m_pRenderClass;
  /* ID3D11Device*           pDevice = (ID3D11Device*)pRenderer->Device();
  ID3D11DeviceContext*    pDeviceContext = (ID3D11DeviceContext*)pRenderer->DeviceContext();
  if ( ( pDevice == NULL )
  ||   ( pDeviceContext == NULL ) )
  {
    return;
  }

  GR::u8*   pData = (GR::u8*)m_pData;

  GR::u32   dwPrimitivesLeft = m_PrimitiveCount;

  tVectVertexBuffers::iterator    it( m_vectVertexBuffers.begin() );
  while ( it != m_vectVertexBuffers.end() )
  {
    ID3D11Buffer*   pVB = *it;

    GR::u32   dwPrimitivesToCopy = dwPrimitivesLeft;
    if ( dwPrimitivesToCopy > m_PrimitivePartCount )
    {
      dwPrimitivesToCopy = m_PrimitivePartCount;
    }
    if ( m_PrimitivePartCount == 0 )
    {
      dwPrimitivesToCopy = dwPrimitivesLeft;
    }

    D3D11_MAPPED_SUBRESOURCE      subResource;

    if ( FAILED( pDeviceContext->Map( pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource ) ) )
    {
      dh::Log( "VulkanVertexBuffer::UpdateData failed" );
      return;
    }

    GR::u8* pPos = ( GR::u8* )subResource.pData;

    // TODO - could be a problem, doesn't take pitch and stride in account!
    memcpy( pPos, pData, dwPrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive );

    pDeviceContext->Unmap( pVB, 0 );

    pData += dwPrimitivesToCopy * m_PrimitiveSize * m_NumVerticesPerPrimitive;

    dwPrimitivesLeft -= dwPrimitivesToCopy;

    ++it;
  }*/
}



void VulkanVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                    const GR::u32 Color,
                                    const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, Color, vectTextureCoord );

  //CreateBuffers( ( (VulkanRenderer*)m_pRenderClass )->Device() );
  UpdateData();
}



void VulkanVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                                    const GR::f32 fRHW,
                                    const GR::u32 Color,
                                    const GR::tFPoint& vectTextureCoord )
{
  XBasicVertexBuffer::AddVertex( vectPos, fRHW, Color, vectTextureCoord );


  //CreateBuffers( ( (VulkanRenderer*)m_pRenderClass )->Device() );
  UpdateData();
}



bool VulkanVertexBuffer::SetData( void* pData, GR::u32 NumVertices )
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



GR::up VulkanVertexBuffer::VertexSize( GR::u32 VertexFormat )
{
  if ( VertexFormat == 0 )
  {
    VertexFormat = m_VertexFormat;
  }

  GR::up   vertexSize = 0;

  if ( VertexFormat & VFF_XYZ )
  {
    m_iOffsetXYZ = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );
  }
  else if ( VertexFormat & VFF_XYZRHW )
  {
    m_iOffsetXYZ = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );

    //m_iOffsetRHW = ( GR::i32 )vertexSize;
    //vertexSize += sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_NORMAL )
  {
    m_iOffsetNormal = ( GR::i32 )vertexSize;
    vertexSize += 3 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_DIFFUSE )
  {
    // colors are now float[4]
    m_iOffsetDiffuse = ( GR::i32 )vertexSize;
    vertexSize += 4 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_SPECULAR )
  {
    m_iOffsetSpecular = ( GR::i32 )vertexSize;
    vertexSize += 4 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_TEXTURECOORD )
  {
    m_iOffsetTU = ( GR::i32 )vertexSize;
    vertexSize += 2 * sizeof( GR::f32 );
  }

  return vertexSize;
}



void VulkanVertexBuffer::SetVertex( size_t iVertexIndex,
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



