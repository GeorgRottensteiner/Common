#ifndef GUI_RADIOBUTTON_H
#define GUI_RADIOBUTTON_H



#include <string>
#include <vector>

#include <Controls\AbstractRadioButton.h>
#include "GUIComponent.h"



class GUIRadioButton : public AbstractRadioButton<GUIComponent>
{

  protected:

    GR::u32                   m_PushedColor,
                              m_MouseOverColor;


  public:

    DECLARE_CLONEABLE( GUIRadioButton, "RadioButton" )

    GUIRadioButton( int NewX = 0, int NewY = 0, int NewWidth = 20, int NewHeight = 20, GR::u32 Id = 0 );
    GUIRadioButton( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id = 0 );

    virtual void DisplayOnPage( GUIComponentDisplayer& Displayer );

};


#endif // __GUI_RADIOBUTTON_H__



