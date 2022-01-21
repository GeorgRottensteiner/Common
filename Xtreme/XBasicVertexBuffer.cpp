#include "XBasicVertexBuffer.h"
#include "XRenderer.h"



XBasicVertexBuffer::XBasicVertexBuffer( XRenderer* pClass ) :
  m_pRenderClass( pClass ),
  m_VertexFormat( VFF_NONE ),
  m_iOffsetXYZ( -1 ),
  m_iOffsetRHW( -1 ),
  m_iOffsetNormal( -1 ),
  m_iOffsetDiffuse( -1 ),
  m_iOffsetSpecular( -1 ),
  m_iOffsetTU( -1 ),
  m_PrimitiveCount( 0 ),
  m_VertexCount( 0 ),
  m_PrimitiveSize( 0 ),
  m_CompleteBufferSize( 0 ),
  m_pData( NULL ),
  m_PrimitiveType( PT_TRIANGLE ),
  m_NumVerticesPerPrimitive( 3 )
{
}



XBasicVertexBuffer::~XBasicVertexBuffer()
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }
}



bool XBasicVertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }

  if ( Type == PT_TRIANGLE )
  {
    m_NumVerticesPerPrimitive = 3;
  }
  else if ( Type == PT_LINE )
  {
    m_NumVerticesPerPrimitive = 2;
  }
  else
  {
    dh::Log( "XBasicVertexBuffer::Create called with unsupported primitivetype %d", Type );
    return false;
  }

  m_PrimitiveSize       = VertexSize( VertexFormat );
  m_VertexCount         = PrimitiveCount * m_NumVerticesPerPrimitive;
  m_PrimitiveCount      = PrimitiveCount;
  m_CompleteBufferSize  = m_VertexCount * m_PrimitiveSize;
  m_VertexFormat        = VertexFormat;
  m_PrimitiveType         = Type;

  m_pData = new( std::nothrow )GR::u8[m_CompleteBufferSize];
  if ( m_pData == NULL )
  {
    return false;
  }
  memset( m_pData, 0, m_CompleteBufferSize );

  //dh::Log( "XBasicVertexBuffer::Create Created VertexBuffer with %d bytes", m_CompleteBufferSize );
  return true;

}



bool XBasicVertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return Create( 0, VertexFormat, Type );
}



void XBasicVertexBuffer::FillFromMesh( const Mesh::IMesh& Mesh )
{
  size_t            iVertexIndex = 0;

  if ( m_PrimitiveType != PT_TRIANGLE )
  {
    dh::Log( "XBasicVertexBuffer::FillFromMesh called with primitive type LINE!" );
    return;
  }

  // Experiment
  //memset( m_pData, 0xcc, m_CompleteBufferSize );
  //dh::Log( "XBasicVertexBuffer::Create Created VertexBuffer with %d bytes", m_CompleteBufferSize );

  //XMesh::tVectFaces::const_iterator   itFaces( Mesh.m_vectFaces.begin() );
  //while ( itFaces != Mesh.m_vectFaces.end() )
  GR::u32     faceCount = Mesh.FaceCount();
  for ( GR::u32 face = 0; face < faceCount; ++face )
  {
    //const CFace&    Face = *itFaces;
    const Mesh::Face&    Face( Mesh.Face( face ) );

    for ( int i = 0; i < 3; ++i )
    {
      if ( Face.m_Vertex[i] >= Mesh.VertexCount() )
      {
        dh::Log( "XBasicVertexBuffer::FillFromMesh Vertex index out of bounds! (%d >= %d)",
              Face.m_Vertex[i], Mesh.VertexCount() );
        ++iVertexIndex;
        continue;
      }
      SetVertex( iVertexIndex, 
                 Mesh.Vertex( Face.m_Vertex[i] ).Position,
                 Face.m_Normal[i],
                 1.0f,
                 Face.m_DiffuseColor[i],
                 GR::tFPoint( Face.m_TextureX[i], Face.m_TextureY[i] ) );
      ++iVertexIndex;
    }

    //++itFaces;
  }

  /*
  for ( GR::up i = 0; i < m_CompleteBufferSize; ++i )
  {
    if ( ( (GR::u8*)m_pData )[i] == 0xcc )
    {
      dh::Log( "XBasicVertexBuffer::FillFromMesh unfilled vertex data at %d!!", i );
    }
  }
  */
  UpdateData();

}



void XBasicVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                               const GR::u32 Color,
                               const GR::tFPoint& vectTextureCoord )
{

  if ( m_PrimitiveSize == 0 )
  {
    return;
  }
  m_VertexCount++;
  m_PrimitiveCount      = m_VertexCount / m_NumVerticesPerPrimitive;
  m_CompleteBufferSize  = m_VertexCount * m_PrimitiveSize;

  if ( m_pData )
  {
    GR::u8*   pTemp = new GR::u8[m_CompleteBufferSize];

    memcpy( pTemp, m_pData, m_CompleteBufferSize - m_PrimitiveSize );

    delete[] m_pData;

    m_pData = pTemp;
  }
  else
  {
    m_pData = new GR::u8[m_CompleteBufferSize];
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + ( m_VertexCount - 1 ) * m_PrimitiveSize;

  if ( m_iOffsetXYZ != -1 )
  {
    *(GR::tVector*)( pNewVertex + m_iOffsetXYZ ) = vectPos;
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    *(GR::u32*)( pNewVertex + m_iOffsetDiffuse ) = Color;
  }
  if ( m_iOffsetTU != -1 )
  {
    (*(GR::tFPoint*)( pNewVertex + m_iOffsetTU ) ) = vectTextureCoord;
  }
}



void XBasicVertexBuffer::AddVertex( const GR::tVector& vectPos, 
                               const GR::f32 fRHW,
                               const GR::u32 Color,
                               const GR::tFPoint& vectTextureCoord )
{

  if ( m_PrimitiveSize == 0 )
  {
    return;
  }
  m_VertexCount++;
  m_PrimitiveCount      = m_VertexCount / m_NumVerticesPerPrimitive;
  m_CompleteBufferSize  = m_VertexCount * m_PrimitiveSize;

  if ( m_pData )
  {
    GR::u8*   pTemp = new GR::u8[m_CompleteBufferSize];

    memcpy( pTemp, m_pData, m_CompleteBufferSize - m_PrimitiveSize );

    delete[] m_pData;

    m_pData = pTemp;
  }
  else
  {
    m_pData = new GR::u8[m_CompleteBufferSize];
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + ( m_VertexCount - 1 ) * m_PrimitiveSize;

  if ( m_iOffsetXYZ != -1 )
  {
    *(GR::tVector*)( pNewVertex + m_iOffsetXYZ ) = vectPos;
  }
  if ( m_iOffsetRHW != -1 )
  {
    *(GR::f32*)( pNewVertex + m_iOffsetRHW ) = fRHW;
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    *(GR::u32*)( pNewVertex + m_iOffsetDiffuse ) = Color;
  }
  if ( m_iOffsetTU != -1 )
  {
    (*(GR::tFPoint*)( pNewVertex + m_iOffsetTU ) ) = vectTextureCoord;
  }
}



void XBasicVertexBuffer::SetVertex( size_t iVertexIndex,
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

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetXYZ != -1 )
  {
    *(GR::tVector*)( pNewVertex + m_iOffsetXYZ ) = vectPos;
  }
  if ( m_iOffsetNormal != -1 )
  {
    *(GR::tVector*)( pNewVertex + m_iOffsetNormal ) = vectNormal;
  }
  if ( m_iOffsetRHW != -1 )
  {
    *(GR::f32*)( pNewVertex + m_iOffsetRHW ) = fRHW;
  }
  if ( m_iOffsetDiffuse != -1 )
  {
    *(GR::u32*)( pNewVertex + m_iOffsetDiffuse ) = Color;
  }
  if ( m_iOffsetTU != -1 )
  {
    (*(GR::tFPoint*)( pNewVertex + m_iOffsetTU ) ) = vectTextureCoord;
  }
}



GR::up XBasicVertexBuffer::VertexCount() const
{
  return m_VertexCount;
}



GR::up XBasicVertexBuffer::VertexSize( GR::u32 VertexFormat )
{
  if ( VertexFormat == 0 )
  {
    VertexFormat = m_VertexFormat;
  }

  GR::up   dwVertexSize = 0;

  if ( VertexFormat & VFF_XYZ )
  {
    m_iOffsetXYZ = (GR::i32)dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );
  }
  else if ( VertexFormat & VFF_XYZRHW )
  {
    m_iOffsetXYZ = (GR::i32)dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );

    m_iOffsetRHW = (GR::i32)dwVertexSize;
    dwVertexSize += sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_NORMAL )
  {
    m_iOffsetNormal = (GR::i32)dwVertexSize;
    dwVertexSize += 3 * sizeof( GR::f32 );
  }
  if ( VertexFormat & VFF_DIFFUSE )
  {
    m_iOffsetDiffuse = (GR::i32)dwVertexSize;
    dwVertexSize += sizeof( GR::u32 );
  }
  if ( VertexFormat & VFF_SPECULAR )
  {
    m_iOffsetSpecular = (GR::i32)dwVertexSize;
    dwVertexSize += sizeof( GR::u32 );
  }
  if ( VertexFormat & VFF_TEXTURECOORD )
  {
    m_iOffsetTU = (GR::i32)dwVertexSize;
    dwVertexSize += 2 * sizeof( GR::f32 );
  }

  return dwVertexSize;
}



bool XBasicVertexBuffer::CloneFrom( XVertexBuffer* pCloneSource )
{
  Release();
  if ( m_pData )
  {
    delete[] m_pData;
    m_pData = NULL;
  }

  if ( pCloneSource == NULL )
  {
    return false;
  }

  XBasicVertexBuffer*   pBasicVertexBuffer = (XBasicVertexBuffer*)pCloneSource;

  m_VertexFormat        = pBasicVertexBuffer->m_VertexFormat;
  m_PrimitiveSize       = VertexSize( m_VertexFormat );
  m_VertexCount         = pBasicVertexBuffer->m_PrimitiveCount * m_NumVerticesPerPrimitive;
  m_PrimitiveCount      = pBasicVertexBuffer->m_PrimitiveCount;
  m_CompleteBufferSize  = m_VertexCount * m_PrimitiveSize;
  m_PrimitiveType         = pBasicVertexBuffer->m_PrimitiveType;
  
  m_pData = new( std::nothrow )GR::u8[m_CompleteBufferSize];
  if ( m_pData == NULL )
  {
    return false;
  }

  memcpy( m_pData, pBasicVertexBuffer->m_pData, m_CompleteBufferSize );

  return Restore();

}



GR::tVector XBasicVertexBuffer::VertexPos( const size_t iVertexIndex )
{
  if ( iVertexIndex >= m_VertexCount )
  {
    return GR::tVector();
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetXYZ != -1 )
  {
    return *(GR::tVector*)( pNewVertex + m_iOffsetXYZ );
  }
  return GR::tVector();
}



GR::tVector XBasicVertexBuffer::VertexNormal( const size_t iVertexIndex )
{
  if ( iVertexIndex >= m_VertexCount )
  {
    return GR::tVector();
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetNormal != -1 )
  {
    return *(GR::tVector*)( pNewVertex + m_iOffsetNormal );
  }
  return GR::tVector();
}



GR::f32 XBasicVertexBuffer::VertexRHW( const size_t iVertexIndex )
{
  if ( iVertexIndex >= m_VertexCount )
  {
    return 0.0f;
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetRHW != -1 )
  {
    return *(GR::f32*)( pNewVertex + m_iOffsetRHW );
  }
  return 0.0f;
}




GR::u32 XBasicVertexBuffer::VertexColor( const size_t iVertexIndex )
{
  if ( iVertexIndex >= m_VertexCount )
  {
    return 0;
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetDiffuse != -1 )
  {
    return *(GR::u32*)( pNewVertex + m_iOffsetDiffuse );
  }
  return 0;
}



GR::tFPoint XBasicVertexBuffer::VertexTU( const size_t iVertexIndex )
{
  if ( iVertexIndex >= m_VertexCount )
  {
    return GR::tFPoint();
  }

  GR::u8*   pNewVertex = (GR::u8*)m_pData + iVertexIndex * m_PrimitiveSize;

  if ( m_iOffsetTU != -1 )
  {
    return *(GR::tFPoint*)( pNewVertex + m_iOffsetTU );
  }
  return GR::tFPoint();
}



GR::u32 XBasicVertexBuffer::PrimitiveCount() const
{
  return m_PrimitiveCount;
}



GR::u32 XBasicVertexBuffer::VertexFormat() const
{
  return m_VertexFormat;
}



const void* XBasicVertexBuffer::Data()
{
  return m_pData;
}



XBasicVertexBuffer::PrimitiveType XBasicVertexBuffer::Type() const
{
  return m_PrimitiveType;
}