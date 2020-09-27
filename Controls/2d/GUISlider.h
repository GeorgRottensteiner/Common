#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H



#include <Controls\AbstractSlider.h>

#include "GUIComponent.h"



class GR::Graphic::GFXPage;

class GUISlider : public AbstractSlider<GUIComponent>
{

  protected:

    GR::Graphic::Image*          m_pImageSliderTop;
    GR::Graphic::Image*          m_pImageSliderMiddle;
    GR::Graphic::Image*          m_pImageSliderBottom;

  public:

    DECLARE_CLONEABLE( GUISlider, "Slider" )

    GUISlider( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, SliderFlagType sfType = SFT_VERTICAL, GR::u32 dwID = 0 );

    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

    void                SetImages( GR::Graphic::Image* pImageSliderTop,
                                   GR::Graphic::Image* pImageSliderMiddle,
                                   GR::Graphic::Image* pImageSliderBottom );

};


#endif // GUI_SLIDER_H



