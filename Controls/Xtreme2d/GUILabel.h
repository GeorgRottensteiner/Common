#ifndef GUI_LABEL_H
#define GUI_LABEL_H



#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



class GUILabel : public AbstractLabel<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUILabel, "Label" )


    GUILabel( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, const GR::String& Caption = GR::String(), GR::u32 ID = 0 );
    GUILabel( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 dwFlags, const GR::String& Caption, GR::u32 ID = 0 );


    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_LABEL_H



