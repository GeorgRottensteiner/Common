#ifndef _ISHADEDRENDERABLE_H
#define _ISHADEDRENDERABLE_H


#include <Interface/IRenderAble.h>

#include "XRenderer.h"



struct IShadedRenderAble : public IRenderAble
{

  public:

    XRenderer::eShaderType           m_eShader;


    IShadedRenderAble() :
      m_eShader( XRenderer::ST_INVALID )
    {
    }

    virtual ~IShadedRenderAble() 
    {
    }

};

#endif// _ISHADEDRENDERABLE_H

