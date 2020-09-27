#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H



#include <Controls\AbstractSlider.h>
#include "GUIComponent.h"



class GUISlider : public AbstractSlider<GUIComponent>
{

  public:

    DECLARE_CLONEABLE( GUISlider, "Slider" )


    GUISlider( int X = 0, int Y = 0, int Width = 20, int Height = 20, SliderFlagType Flags = SFT_VERTICAL, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual void              SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 ColorKey = 0 );


};


#endif // __GUI_SLIDER_H__



