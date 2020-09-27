#ifndef XTREME_DX9SHADER_RENDERER
#define XTREME_DX9SHADER_RENDERER

#include <d3d9.h>



class DX9Shader
{

  public:

    IDirect3DPixelShader9*              m_pShader;



    DX9Shader();


    void                                Release();


};


#endif // XTREME_DX9SHADER_RENDERER
