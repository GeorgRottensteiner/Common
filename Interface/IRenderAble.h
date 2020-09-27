#ifndef _IRENDERABLE_H
#define _IRENDERABLE_H

class XRenderer;

struct IRenderAble
{

  public:

    virtual ~IRenderAble() 
    {
    }

    virtual void          Render( XRenderer& Renderer ) = 0;

};

#endif// _IRENDERABLE_H

