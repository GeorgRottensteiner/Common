#include "SDLVertexBuffer.h"


SDLVertexBuffer::SDLVertexBuffer()
{
}



SDLVertexBuffer::~SDLVertexBuffer()
{
}



bool SDLVertexBuffer::Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type )
{
  return false;
}



bool SDLVertexBuffer::Create( GR::u32 VertexFormat, PrimitiveType Type )
{
  return false;
}



bool SDLVertexBuffer::CloneFrom( XVertexBuffer* pCloneSource )
{
  return false;
}



void SDLVertexBuffer::Release()
{
}



bool SDLVertexBuffer::Restore()
{
  return false;
}



void SDLVertexBuffer::FillFromMesh( const Mesh::IMesh& Mesh )
{
}



void SDLVertexBuffer::AddVertex( const GR::tVector& vectPos,
                            const GR::u32 dwColor,
                            const GR::tFPoint& vectTextureCoord )
{
}



void SDLVertexBuffer::AddVertex( const GR::tVector& vectPos,
                            const GR::f32 fRHW,
                            const GR::u32 dwColor,
                            const GR::tFPoint& vectTextureCoord )
{
}



void SDLVertexBuffer::SetVertex( size_t iVertexIndex,
                            const GR::tVector& vectPos,
                            const GR::tVector& vectNormal,
                            const GR::f32 fRHW,
                            const GR::u32 dwColor,
                            const GR::tFPoint& vectTextureCoord )
{
}



GR::up SDLVertexBuffer::VertexSize( GR::u32 dwVertexFormat )
{
  return 0;
}



GR::up SDLVertexBuffer::VertexCount() const
{
  return 0;
}



GR::u32 SDLVertexBuffer::PrimitiveCount() const
{
  return 0;
}



GR::u32 SDLVertexBuffer::VertexFormat() const
{
  return 0;
}

GR::tVector SDLVertexBuffer::VertexPos( const size_t iVertexIndex )
{
  return GR::tVector();
}



GR::tVector SDLVertexBuffer::VertexNormal( const size_t iVertexIndex )
{
  return GR::tVector();
}



GR::f32 SDLVertexBuffer::VertexRHW( const size_t iVertexIndex )
{
  return 0.0f;
}



GR::u32 SDLVertexBuffer::VertexColor( const size_t iVertexIndex )
{
  return 0;
}



GR::tFPoint SDLVertexBuffer::VertexTU( const size_t iVertexIndex )
{
  return GR::tFPoint();
}



void SDLVertexBuffer::UpdateData()
{
}



const void* SDLVertexBuffer::Data()
{
  return NULL;
}



XVertexBuffer::PrimitiveType SDLVertexBuffer::Type() const
{
  return XVertexBuffer::PrimitiveType::PT_TRIANGLE;
}