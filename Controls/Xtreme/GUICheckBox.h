#ifndef GUI_CHECKBOX_H
#define GUI_CHECKBOX_H



#include <string>
#include <vector>

#include <Controls\AbstractCheckbox.h>
#include "GUIComponent.h"



class GUICheckBox : public AbstractCheckBox<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUICheckBox, "Checkbox" )

    GUICheckBox( int NewX = 0, int NewY = 0, int NewWidth = 0, int NewHeight = 0, GR::u32 Id = 0 );
    GUICheckBox( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Flags = AbstractCheckBox<GUIComponent>::CS_DEFAULT, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


};


#endif // GUI_CHECKBOX_H



