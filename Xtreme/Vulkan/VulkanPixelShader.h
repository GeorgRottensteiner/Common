#ifndef XTREME_VulkanPIXELSHADER_RENDERER
#define XTREME_VulkanPIXELSHADER_RENDERER

#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>



class VulkanRenderer;

class VulkanPixelShader
{

  public:

    VulkanRenderer*                       m_pRenderer;

    GR::u32                             m_VertexFormat;

    ByteBuffer                          m_CompiledShader;



    VulkanPixelShader( VulkanRenderer* pRenderer );


    void                                Release();


    bool CreateFromFile( const GR::String& CompiledShaderFile );
    bool CreateFromBuffers( const ByteBuffer& CompiledShader );

};


#endif // XTREME_VulkanPIXELSHADER_RENDERER
