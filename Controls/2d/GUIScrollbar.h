#ifndef GUI_SCROLLBAR_H
#define GUI_SCROLLBAR_H



#include <Controls\AbstractScrollbar.h>

#include "GUIComponent.h"
#include "GUIButton.h"
#include "GUISlider.h"



class GR::Graphic::GFXPage;

class GUIScrollbar : public AbstractScrollbar<GUIComponent,GUIButton,GUISlider>
{

  public:

    DECLARE_CLONEABLE( GUIScrollbar, "Scrollbar" )


    GUIScrollbar( int iNewX = 0, int iNewY = 0, int iNewWidth = 0, int iNewHeight = 0, GR::u32 sfType = SBFT_VERTICAL, GR::u32 dwId = 0 );

    virtual void        DisplayOnPage( GR::Graphic::GFXPage* pPage );

    void                SetSliderImages( GR::Graphic::Image* pImageSliderTop, GR::Graphic::Image* pImageSliderCenter, GR::Graphic::Image* pImageSliderBottom );

};


#endif // GUI_SCROLLBAR_H



