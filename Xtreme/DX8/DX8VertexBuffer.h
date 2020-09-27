#pragma once
#include <Xtreme/XBasicVertexbuffer.h>

#include <d3d8.h>
#include <vector>


class XRenderer;

class CDX8VertexBuffer : public XBasicVertexBuffer
{

  protected:

    typedef std::vector<IDirect3DVertexBuffer8*>    tVectVertexBuffers;

    tVectVertexBuffers            m_vectVertexBuffers;

    D3DPRIMITIVETYPE              m_d3dPrimType;

    GR::u32                       m_PrimitivePartCount,
                                  m_Usage;

    D3DPOOL                       m_d3dPool;


    bool                          CreateBuffers( LPDIRECT3DDEVICE8 pd3dDevice );
    void                          Invalidate();


  public:


    CDX8VertexBuffer( XRenderer* pRenderClass );
    virtual ~CDX8VertexBuffer();

    virtual bool                  Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type );
    virtual bool                  Create( GR::u32 VertexFormat, PrimitiveType Type );
    virtual void                  Release();
    virtual bool                  Restore();

    bool                          Display( GR::u32 Index = 0, GR::u32 Count = 0 );

    bool                          SetData( void* pData, GR::u32 NumVertices );
    virtual void                  UpdateData();

    virtual void                  AddVertex( const GR::tVector& vectPos, 
                                             const GR::u32 Color,
                                             const GR::tFPoint& vectTextureCoord );
    virtual void                  AddVertex( const GR::tVector& vectPos, 
                                             const GR::f32 fRHW,
                                             const GR::u32 Color,
                                             const GR::tFPoint& vectTextureCoord );

};
