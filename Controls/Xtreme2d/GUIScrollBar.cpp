#include "GUIComponentDisplayer.h"
#include "GUIScrollBar.h"



GUI_IMPLEMENT_CLONEABLE( GUIScrollBar, "Scrollbar" )



GUIScrollBar::GUIScrollBar( int NewX, int NewY, int NewWidth, int NewHeight, ScrollbarFlagType ScrollbarFlags , GR::u32 Id ) :
  AbstractScrollbar<GUIComponent, GUIButton, GUISlider>( NewX, NewY, NewWidth, NewHeight, ScrollbarFlags, Id )
{

}



void GUIScrollBar::DisplayOnPage( GUIComponentDisplayer& Displayer )
{

}



void GUIScrollBar::SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 ColorKey )
{
  AbstractScrollbar<GUIComponent, GUIButton, GUISlider>::SetCustomTextureSection( Type, TexSection, ColorKey );

  if ( Type == GUI::CTS_SLIDER )
  {
    m_pSlider->SetCustomTextureSection( Type, TexSection, ColorKey );

    // force refresh on slider
    SetSize( Width(), Height() );
  }
}