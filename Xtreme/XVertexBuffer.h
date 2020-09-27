#pragma once

#include <GR/GRTypes.h>

#include "XMesh.h"



class XRenderer;

class XVertexBuffer
{

  public:

    enum PrimitiveType
    {
      PT_TRIANGLE,
      PT_LINE
    };

    enum eVertexFormatFlags
    {
      VFF_NONE          = 0,
      VFF_XYZ           = 0x00000002,
      VFF_XYZRHW        = 0x00000004,
      VFF_NORMAL        = 0x00000010,
      VFF_DIFFUSE       = 0x00000040,
      VFF_SPECULAR      = 0x00000080,
      VFF_TEXTURECOORD  = 0x00000100,
    };

    virtual ~XVertexBuffer()
    {
    }

    virtual bool            Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type ) = 0;
    virtual bool            Create( GR::u32 VertexFormat, PrimitiveType Type ) = 0;

    virtual bool            CloneFrom( XVertexBuffer* pCloneSource ) = 0;

    virtual void            Release() = 0;
    virtual bool            Restore() = 0;

    virtual void            FillFromMesh( const Mesh::IMesh& Mesh ) = 0;

    virtual void            AddVertex( const GR::tVector& vectPos, 
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord ) = 0;
    virtual void            AddVertex( const GR::tVector& vectPos, 
                                       const GR::f32 fRHW,
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord ) = 0;

    virtual void            SetVertex( size_t iVertexIndex,
                                       const GR::tVector& vectPos, 
                                       const GR::tVector& vectNormal, 
                                       const GR::f32 fRHW,
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord ) = 0;

    virtual GR::up          VertexSize( GR::u32 VertexFormat = 0 ) = 0;
    virtual GR::up          VertexCount() const = 0;
    virtual GR::u32         PrimitiveCount() const = 0;
    virtual GR::u32         VertexFormat() const = 0;
    virtual PrimitiveType   Type() const = 0;

    virtual GR::tVector     VertexPos( const size_t iVertexIndex ) = 0;
    virtual GR::tVector     VertexNormal( const size_t iVertexIndex ) = 0;
    virtual GR::f32         VertexRHW( const size_t iVertexIndex ) = 0;
    virtual GR::u32         VertexColor( const size_t iVertexIndex ) = 0;
    virtual GR::tFPoint     VertexTU( const size_t iVertexIndex ) = 0;

    virtual void            UpdateData() = 0;
    virtual const void*     Data() = 0;

};
