#ifndef GUI_LABEL_H
#define GUI_LABEL_H



#include <Controls\AbstractLabel.h>
#include "GUIComponent.h"



class GUILabel : public AbstractLabel<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUILabel, "Label" )


    GUILabel( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, const GR::String& Caption = GR::String(), GR::u32 ID = 0 );
    GUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwFlags, const GR::String& Caption, GR::u32 ID = 0 );


    virtual void        DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // GUI_LABEL_H



