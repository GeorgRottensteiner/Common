#ifndef _IRENDERABLE_H
#define _IRENDERABLE_H

class CD3DViewer;



struct IRenderAble
{

  public:

    virtual ~IRenderAble() 
    {
    }

    virtual void          Render( CD3DViewer& Viewer ) = 0;

};

#endif// _IRENDERABLE_H

