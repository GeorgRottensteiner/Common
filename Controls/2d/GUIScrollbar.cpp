#include "GUIComponentDisplayer.h"
#include "GUIScrollbar.h"
#include "GUIButton.h"



IMPLEMENT_CLONEABLE( GUIScrollbar, "Scrollbar" )



GUIScrollbar::GUIScrollbar( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 sfType, GR::u32 dwId ) :
  AbstractScrollbar<GUIComponent,GUIButton,GUISlider>( iNewX, iNewY, iNewWidth, iNewHeight, sfType, dwId )
{
}



void GUIScrollbar::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
}



void GUIScrollbar::SetSliderImages( GR::Graphic::Image* pImageSliderTop, GR::Graphic::Image* pImageSliderCenter, GR::Graphic::Image* pImageSliderBottom )
{
  m_pSlider->SetImages( pImageSliderTop, pImageSliderCenter, pImageSliderBottom );
}