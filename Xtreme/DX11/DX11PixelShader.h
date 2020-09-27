#ifndef XTREME_DX11PIXELSHADER_RENDERER
#define XTREME_DX11PIXELSHADER_RENDERER

#include <d3d11.h>

#include <string>
#include <vector>

#include <GR/GRTypes.h>

#include <Memory/ByteBuffer.h>



class DX11Renderer;

class DX11PixelShader
{

  public:

    ID3D11PixelShader*                  m_pShader;

    DX11Renderer*                       m_pRenderer;

    GR::u32                             m_VertexFormat;

    ByteBuffer                          m_CompiledShader;



    DX11PixelShader( DX11Renderer* pRenderer );


    void                                Release();


    bool CreateFromFile( const GR::String& CompiledShaderFile );
    bool CreateFromBuffers( const ByteBuffer& CompiledShader );

};


#endif // XTREME_DX11PIXELSHADER_RENDERER
