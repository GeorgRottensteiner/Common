#pragma once

#include <GR/GRTypes.h>

#include "XMesh.h"
#include "XVertexBuffer.h"


class XRenderer;

class XBasicVertexBuffer : public XVertexBuffer
{

  protected:

    XRenderer*                    m_pRenderClass;

    GR::u32                       m_VertexFormat;

    GR::i32                       m_iOffsetXYZ,
                                  m_iOffsetRHW,
                                  m_iOffsetNormal,
                                  m_iOffsetDiffuse,
                                  m_iOffsetSpecular,
                                  m_iOffsetTU;

    GR::up                        m_CompleteBufferSize,
                                  m_PrimitiveSize;

    GR::u32                       m_VertexCount;

    GR::u32                       m_PrimitiveCount;

    int                           m_NumVerticesPerPrimitive;

    void*                         m_pData;

    PrimitiveType                 m_PrimitiveType;


  public:

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



    XBasicVertexBuffer( XRenderer* pClass );
    virtual ~XBasicVertexBuffer();

    virtual bool            Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type );
    virtual bool            Create( GR::u32 VertexFormat, PrimitiveType Type );

    virtual bool            CloneFrom( XVertexBuffer* pCloneSource );

    virtual void            Release() = 0;
    virtual bool            Restore() = 0;

    virtual void            FillFromMesh( const Mesh::IMesh& Mesh );

    virtual void            AddVertex( const GR::tVector& vectPos, 
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord );
    virtual void            AddVertex( const GR::tVector& vectPos, 
                                       const GR::f32 fRHW,
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord );

    virtual void            SetVertex( size_t iVertexIndex,
                                       const GR::tVector& vectPos, 
                                       const GR::tVector& vectNormal, 
                                       const GR::f32 fRHW,
                                       const GR::u32 Color,
                                       const GR::tFPoint& vectTextureCoord );

    virtual GR::up          VertexSize( GR::u32 VertexFormat = 0 );
    virtual GR::up          VertexCount() const;
    virtual GR::u32         PrimitiveCount() const;
    virtual GR::u32         VertexFormat() const;
    virtual PrimitiveType   Type() const;

    virtual GR::tVector     VertexPos( const size_t iVertexIndex );
    virtual GR::tVector     VertexNormal( const size_t iVertexIndex );
    virtual GR::f32         VertexRHW( const size_t iVertexIndex );
    virtual GR::u32         VertexColor( const size_t iVertexIndex );
    virtual GR::tFPoint     VertexTU( const size_t iVertexIndex );

    virtual void            UpdateData() = 0;

    virtual const void*     Data();

};
