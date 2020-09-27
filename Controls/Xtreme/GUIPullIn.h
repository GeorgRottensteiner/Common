#ifndef GUI_PULLIN_H
#define GUI_PULLIN_H



#include <Controls\AbstractPullIn.h>

#include "GUIComponent.h"



class GUIPullIn : public AbstractPullIn<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUIPullIn, "PullIn" )


    GUIPullIn( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, const GR::String& Caption = GR::String(), GR::u32 Type = 0, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );
    virtual void              DisplayNonClientOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_PULLIN_H



