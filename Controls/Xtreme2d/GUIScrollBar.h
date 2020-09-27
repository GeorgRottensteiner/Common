#ifndef GUI_SCROLLBAR_H
#define GUI_SCROLLBAR_H



#include <Controls\AbstractScrollBar.h>
#include "GUIComponent.h"
#include "GUIButton.h"
#include "GUISlider.h"



class GUIScrollBar : public AbstractScrollbar<GUIComponent, GUIButton, GUISlider>
{

  public:

    DECLARE_CLONEABLE( GUIScrollBar, "Scrollbar" )


    GUIScrollBar( int X = 0, int Y = 0, int Width = 20, int Height = 20, ScrollbarFlagType Type = SBFT_INVALID, GR::u32 Id = 0 );

    virtual void              DisplayOnPage( GUIComponentDisplayer& Displayer );

    virtual void              SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 ColorKey );

};


#endif // GUI_SCROLLBAR_H



