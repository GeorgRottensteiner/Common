#include "GUIComponentDisplayer.h"
#include "GUIScrollBar.h"



GUI_IMPLEMENT_CLONEABLE( GUIScrollBar, "Scrollbar" )



GUIScrollBar::GUIScrollBar( int iNewX, int iNewY, int iNewWidth, int iNewHeight, ScrollbarFlagType sfType, GR::u32 dwId ) :
  AbstractScrollbar<GUIComponent, GUIButton, GUISlider>( iNewX, iNewY, iNewWidth, iNewHeight, sfType, dwId )
{

}



void GUIScrollBar::DisplayOnPage( GUIComponentDisplayer& Displayer )
{

}



void GUIScrollBar::SetCustomTextureSection( const GR::u32 dwType, const XTextureSection& TexSection, GR::u32 dwColorKey )
{
  AbstractScrollbar<GUIComponent, GUIButton, GUISlider>::SetCustomTextureSection( dwType, TexSection, dwColorKey );

  if ( dwType == GUI::CTS_SLIDER )
  {
    m_pSlider->SetCustomTextureSection( dwType, TexSection, dwColorKey );

    // force refresh on slider
    SetSize( Width(), Height() );
  }
}