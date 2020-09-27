#ifndef XTREME_DX11_TRIANGLECACHE_H
#define XTREME_DX11_TRIANGLECACHE_H



#include <Xtreme/XTextureSection.h>

#include <vector>

#include <Xtreme/XMesh.h>
#include <Xtreme/XRenderer.h>



class DX11VertexBuffer;
class DX11Renderer;

class DX11TriangleCache
{

  public:

    struct VertexEntry
    {
      GR::tVector     Pos;
      GR::tVector     Normal;
      GR::f32         Diffuse[4];
      GR::tFPoint     TextureCoord;

      void SetColor( GR::u32 Color );
    };

    struct CacheEntry
    {
      XTexture*                 pTexture;
      XRenderer::eShaderType    Shader;

      CacheEntry() :
        Shader( XRenderer::ST_INVALID ),
        pTexture( NULL )
      {
      }
    };

    std::vector<CacheEntry>         PrimitiveInfo;

    std::vector<VertexEntry>        Vertices;

    int                             m_UsedVertices;

    GR::u32                         m_VertexFormat;


    DX11VertexBuffer*               m_pBuffer;
    DX11Renderer*                   m_pRenderer;


    DX11TriangleCache( DX11Renderer* pRenderer, GR::u32 VertexFormat );


    void AddEntry( XTexture* pTexture, 
                   const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                   const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                   const GR::tVector& Pos3, GR::f32 TU3, GR::f32 TV3,
                   GR::u32 Color1,
                   GR::u32 Color2,
                   GR::u32 Color3,
                   XRenderer::eShaderType Shader );

    void EnsureBuffer();
    void GrowCache();
    void FlushCache();


    void DrawPart( int StartPrimitive, int PrimitiveCount );

    void TransformChanged( XRenderer::eTransformType TransformType );

};



#endif // XTREME_DX11_TRIANGLECACHE_H