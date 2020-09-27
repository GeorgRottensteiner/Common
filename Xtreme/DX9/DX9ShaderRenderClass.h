#pragma once


#undef D3D_SDK_VERSION
#include <D3D9.h>

#include <string>
#include <map>

#include <Xtreme/XBasicRenderer.h>
#include "DX9RenderClass.h"
#include "DX9Shader.h"



class ByteBuffer;

class DX9ShaderRenderClass : public CDX9RenderClass
{

  public:

    DX9ShaderRenderClass( HINSTANCE hInstance = NULL );
    virtual ~DX9ShaderRenderClass();


    virtual HRESULT                 ConfirmDevice( D3DCAPS9* pCaps, DWORD Behaviour, D3DFORMAT Format );

    virtual bool                    InitDeviceObjects();
    virtual bool                    InvalidateDeviceObjects();
    virtual bool                    RestoreDeviceObjects();
    virtual bool                    DeleteDeviceObjects();


    virtual void                    SetShader( eShaderType sType );


  private:


    std::map<GR::String,DX9Shader> m_Shader;


    void                            DestroyAllShaders();


    DX9Shader                       CreatePixelShaderFromFile( const GR::String& PixelShaderKey, const char* Filename );
    DX9Shader                       CreatePixelShaderFromBuffer( const GR::String& PixelShaderKey, const ByteBuffer& ShaderData );

    bool                            SetPixelShader( const GR::String& PixelShaderKey );
    bool                            SetPixelShader( const DX9Shader& Shader );


};
