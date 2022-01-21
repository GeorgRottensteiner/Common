#pragma once

#ifndef XTREME_OPENGL_VERTEXBUFFER_H
#define XTREME_OPENGL_VERTEXBUFFER_H


#include <Xtreme/XBasicVertexBuffer.h>

#include <vector>

#include <gl\gl.h>
#include <gl\glext.h>


class XRenderer;

class OpenGLVertexBuffer : public XBasicVertexBuffer
{

  protected:

    GR::u32                       m_PrimitivePartCount;

    GLuint                        m_VertexBufferID;
    GLuint                        m_TextureCoordinatesBufferID;


    bool                          CreateBuffers();
    void                          Invalidate();


  public:


    OpenGLVertexBuffer( XRenderer* pRenderClass );
    virtual ~OpenGLVertexBuffer();

    virtual bool                  Create( GR::u32 PrimitiveCount, GR::u32 VertexFormat, PrimitiveType Type );
    virtual bool                  Create( GR::u32 VertexFormat, PrimitiveType Type );
    virtual void                  Release();
    virtual bool                  Restore();

    bool                          Display( GR::u32 Index = 0, GR::u32 Count = 0 );
    bool                          DisplayLine( GR::u32 Index = 0, GR::u32 Count = 0 );

    bool                          SetData( void* pData, GR::u32 NumVertices );
    virtual void                  UpdateData();

    virtual void                  AddVertex( const GR::tVector& vectPos, 
                                             const GR::u32 Color,
                                             const GR::tFPoint& vectTextureCoord );
    virtual void                  AddVertex( const GR::tVector& vectPos, 
                                             const GR::f32 fRHW,
                                             const GR::u32 Color,
                                             const GR::tFPoint& vectTextureCoord );

    // overridden to use f32-colors
    virtual GR::up                VertexSize( GR::u32 VertexFormat );

    virtual void                  SetVertex( size_t iVertexIndex,
                                             const GR::tVector& vectPos,
                                             const GR::tVector& vectNormal,
                                             const GR::f32 fRHW,
                                             const GR::u32 Color,
                                             const GR::tFPoint& vectTextureCoord );

};


#endif // XTREME_OPENGL_VERTEXBUFFER_H