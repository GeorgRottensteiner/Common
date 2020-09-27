#ifndef XTREME_DX11VERTEXSHADER_RENDERER
#define XTREME_DX11VERTEXSHADER_RENDERER

#include <d3d11.h>

#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>



class DX11Renderer;

class DX11VertexShader
{

  public:

    ID3D11VertexShader*                 m_pShader;
    ID3D11InputLayout*                  m_pInputLayout;

    DX11Renderer*                       m_pRenderer;

    GR::u32                             m_VertexFormat;

    ByteBuffer                          m_CompiledShader;

    GR::String                          m_LoadedFromFile;



    DX11VertexShader( DX11Renderer* pRenderer, GR::u32 VertexFormat );


    void                                Release();


    bool CreateFromFile( const GR::String& CompiledShaderFile );
    bool CreateFromBuffers( const ByteBuffer& CompiledShader );


    void CreateElementDescription( std::vector<D3D11_INPUT_ELEMENT_DESC>& InputDesc );
    bool CreateInputLayout();


};


#endif // XTREME_DX11VERTEXSHADER_RENDERER
