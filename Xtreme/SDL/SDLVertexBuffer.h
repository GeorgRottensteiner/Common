#pragma once


#include <SDL.h>

#include <vector>

#include <Xtreme/XVertexBuffer.h>


class SDLVertexBuffer : public XVertexBuffer
{

  public:


    SDLVertexBuffer();
    virtual ~SDLVertexBuffer();


    virtual bool            Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type );
    virtual bool            Create( GR::u32 VertexFormat, PrimitiveType Type );

    virtual bool            CloneFrom( XVertexBuffer* pCloneSource );

    virtual void            Release();
    virtual bool            Restore();

    virtual void            FillFromMesh( const Mesh::IMesh& Mesh );

    virtual void            AddVertex( const GR::tVector& vectPos,
                                       const GR::u32 dwColor,
                                       const GR::tFPoint& vectTextureCoord );
    virtual void            AddVertex( const GR::tVector& vectPos,
                                       const GR::f32 fRHW,
                                       const GR::u32 dwColor,
                                       const GR::tFPoint& vectTextureCoord );

    virtual void            SetVertex( size_t iVertexIndex,
                                       const GR::tVector& vectPos,
                                       const GR::tVector& vectNormal,
                                       const GR::f32 fRHW,
                                       const GR::u32 dwColor,
                                       const GR::tFPoint& vectTextureCoord );

    virtual GR::up          VertexSize( GR::u32 dwVertexFormat = 0 );
    virtual GR::up          VertexCount() const;
    virtual GR::u32         PrimitiveCount() const;
    virtual GR::u32         VertexFormat() const;

    virtual GR::tVector     VertexPos( const size_t iVertexIndex );
    virtual GR::tVector     VertexNormal( const size_t iVertexIndex );
    virtual GR::f32         VertexRHW( const size_t iVertexIndex );
    virtual GR::u32         VertexColor( const size_t iVertexIndex );
    virtual GR::tFPoint     VertexTU( const size_t iVertexIndex );

    virtual void            UpdateData();
    virtual const void*     Data();

    virtual PrimitiveType   Type() const;
};
