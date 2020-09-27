#ifndef XTREME_VulkanVERTEXSHADER_RENDERER
#define XTREME_VulkanVERTEXSHADER_RENDERER

#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>



class VulkanRenderer;

class VulkanVertexShader
{

  public:

    VulkanRenderer*                       m_pRenderer;

    GR::u32                             m_VertexFormat;

    ByteBuffer                          m_CompiledShader;

    GR::String                          m_LoadedFromFile;



    VulkanVertexShader( VulkanRenderer* pRenderer, GR::u32 VertexFormat );


    void                                Release();


    bool CreateFromFile( const GR::String& CompiledShaderFile );
    bool CreateFromBuffers( const ByteBuffer& CompiledShader );


    bool CreateInputLayout();


};


#endif // XTREME_VulkanVERTEXSHADER_RENDERER
