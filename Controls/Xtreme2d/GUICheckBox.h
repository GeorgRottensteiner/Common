#ifndef GUI_CHECKBOX_H
#define GUI_CHECKBOX_H



#include <Controls\AbstractCheckbox.h>
#include "GUIComponent.h"



class GUICheckBox : public AbstractCheckBox<GUIComponent>
{

  protected:


  public:

    DECLARE_CLONEABLE( GUICheckBox, "CheckBox" )

    GUICheckBox( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 dwId = 0 );
    GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 Flags = CS_DEFAULT, GR::u32 dwId = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );


};


#endif // GUI_CHECKBOX_H



