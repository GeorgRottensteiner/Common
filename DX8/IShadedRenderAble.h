#ifndef _ISHADEDRENDERABLE_H
#define _ISHADEDRENDERABLE_H


#include <DX8/IRenderAble.h>

#include <DX8/DX8Shader.h>



struct IShadedRenderAble : public IRenderAble
{

  public:

    GR::eShaderType           m_eShader;


    IShadedRenderAble() :
      m_eShader( GR::ST_NONE )
    {
    }

    virtual ~IShadedRenderAble() 
    {
    }

};

#endif// _ISHADEDRENDERABLE_H

