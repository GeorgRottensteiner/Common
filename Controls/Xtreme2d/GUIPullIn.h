#ifndef GUI_PULLIN_H
#define GUI_PULLIN_H



#include <Controls\AbstractPullIn.h>

#include "GUIComponent.h"



class GUIPullIn : public AbstractPullIn<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIPullIn, "PullIn" )


    GUIPullIn( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, const char* szCaption = NULL, GR::u32 dfType = 0, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_PULLIN_H



