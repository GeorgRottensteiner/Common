#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H



#include "GUIComponent.h"

#include <Controls\AbstractSlider.h>




class GUISlider : public AbstractSlider<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUISlider, "Slider" )


    GUISlider( int X = 0, int Y = 0, int NewWidth = 20, int NewHeight = 20, SliderFlagType Flags = SFT_VERTICAL, GR::u32 Id = 0 );

    virtual void DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual bool IsSliderResizeable();

    virtual bool ProcessEvent( const GUI::ComponentEvent& Event );

};


#endif // GUI_SLIDER_H



