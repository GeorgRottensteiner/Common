#ifndef XTREME_Vulkan_LINECACHE_H
#define XTREME_Vulkan_LINECACHE_H



#include <Xtreme/XTextureSection.h>

#include <vector>

#include <Xtreme/XMesh.h>
#include <Xtreme/XRenderer.h>



class VulkanVertexBuffer;
class VulkanRenderer;

class VulkanLineCache
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

    std::vector<CacheEntry>         LineInfo;

    std::vector<VertexEntry>        Vertices;

    int                             m_UsedVertices;

    GR::u32                         m_VertexFormat;


    VulkanVertexBuffer*               m_pBuffer;
    VulkanRenderer*                   m_pRenderer;


    VulkanLineCache( VulkanRenderer* pRenderer, GR::u32 VertexFormat );


    void AddEntry( XTexture* pTexture,
                   const GR::tVector& Pos1, const GR::tVector& Pos2, GR::u32 Color,
                   XRenderer::eShaderType Shader );
    void AddEntry( XTexture* pTexture, 
                   const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                   const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                   GR::u32 Color1,
                   GR::u32 Color2,
                   XRenderer::eShaderType Shader );

    void EnsureBuffer();
    void GrowCache();
    void FlushCache();


    void DrawPart( int StartPrimitive, int PrimitiveCount );
    void TransformChanged( XRenderer::eTransformType TransformType );

};



#endif // XTREME_Vulkan_LINECACHE_H