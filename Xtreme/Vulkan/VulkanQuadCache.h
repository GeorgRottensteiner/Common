#ifndef XTREME_Vulkan_QUADCACHE_H
#define XTREME_Vulkan_QUADCACHE_H



#include <Xtreme/XTextureSection.h>

#include <vector>

#include <Xtreme/XMesh.h>
#include <Xtreme/XRenderer.h>



class VulkanVertexBuffer;
class VulkanRenderer;

class VulkanQuadCache
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

    struct QuadCacheEntry
    {
      XTexture*                 pTexture;
      XRenderer::eShaderType    Shader;
      bool                      MatricesChanged;
      math::matrix4             World;
      math::matrix4             View;
      math::matrix4             Projection;

      QuadCacheEntry() :
        Shader( XRenderer::ST_INVALID ),
        pTexture( NULL ),
        MatricesChanged( false )
      {
      }
    };

    std::vector<QuadCacheEntry>     QuadInfo;

    std::vector<VertexEntry>        Vertices;

    int                             m_UsedVertices;

    GR::u32                         m_VertexFormat;

    math::matrix4                   m_CurrentWorld;
    math::matrix4                   m_CurrentView;
    math::matrix4                   m_CurrentProjection;
    bool                            m_MatricesChanged;

    VulkanVertexBuffer*               m_pBuffer;
    VulkanRenderer*                   m_pRenderer;


    VulkanQuadCache( VulkanRenderer* pRenderer, GR::u32 VertexFormat );


    void AddEntry( const XTextureSection& Section, const GR::tVector& Pos, const GR::tVector& Size, GR::u32 Color,
                   XRenderer::eShaderType Shader );
    void AddEntry( XTexture* pTexture, 
                   const GR::tVector& Pos1, GR::f32 TU1, GR::f32 TV1,
                   const GR::tVector& Pos2, GR::f32 TU2, GR::f32 TV2,
                   const GR::tVector& Pos3, GR::f32 TU3, GR::f32 TV3,
                   const GR::tVector& Pos4, GR::f32 TU4, GR::f32 TV4,
                   GR::u32 Color1,
                   GR::u32 Color2,
                   GR::u32 Color3,
                   GR::u32 Color4,
                   XRenderer::eShaderType Shader );

    void EnsureBuffer();
    void GrowCache();
    void FlushCache();


    void DrawPart( int StartPrimitive, int PrimitiveCount );

    void TransformChanged( XRenderer::eTransformType TransformType );

};



#endif // XTREME_Vulkan_QUADCACHE_H
