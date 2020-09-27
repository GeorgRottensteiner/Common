#include <Debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUIScrollBar.h"

GUI_IMPLEMENT_CLONEABLE( GUIScrollBar, "Scrollbar" )



GUIScrollBar::GUIScrollBar( int NewX, int NewY, int NewWidth, int NewHeight, ScrollbarFlagType Type, GR::u32 Id ) :
  AbstractScrollbar<GUIComponent, GUIButton, GUISlider>( NewX, NewY, NewWidth, NewHeight, Type, Id )
{
  XTextureSection   tsUL;
  XTextureSection   tsDR;

  if ( Type & GUIScrollBar::SBFT_HORIZONTAL )
  {
    tsUL = CustomTextureSection( GUI::CTS_ARROW_LEFT );
    tsDR = CustomTextureSection( GUI::CTS_ARROW_RIGHT );
  }
  else
  {
    tsUL = CustomTextureSection( GUI::CTS_ARROW_UP );
    tsDR = CustomTextureSection( GUI::CTS_ARROW_DOWN );
  }

  if ( tsUL.m_pTexture )
  {
    m_pButtonLeftUp->SetCustomTextureSection( GUIButton::CTS_IMAGE, tsUL );
  }
  if ( tsDR.m_pTexture )
  {
    m_pButtonRightDown->SetCustomTextureSection( GUIButton::CTS_IMAGE, tsDR );
  }
}



void GUIScrollBar::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
}



bool GUIScrollBar::ProcessEvent( const GUI::ComponentEvent& Event )
{
  if ( Event.Type == CET_STYLE_CHANGED )
  {
    // adjust button sections
    XTextureSection     tsUL;
    XTextureSection     tsDR;

    if ( Style() & GUIScrollBar::SBFT_HORIZONTAL )
    {
      tsUL = CustomTextureSection( GUI::CTS_ARROW_LEFT );
      tsDR = CustomTextureSection( GUI::CTS_ARROW_RIGHT );
    }
    else
    {
      tsUL = CustomTextureSection( GUI::CTS_ARROW_UP );
      tsDR = CustomTextureSection( GUI::CTS_ARROW_DOWN );
    }

    if ( tsUL.m_pTexture )
    {
      m_pButtonLeftUp->SetCustomTextureSection( GUIButton::CTS_IMAGE, tsUL );
    }
    if ( tsDR.m_pTexture )
    {
      m_pButtonRightDown->SetCustomTextureSection( GUIButton::CTS_IMAGE, tsDR );
    }
  }
  return AbstractScrollbar<GUIComponent, GUIButton, GUISlider>::ProcessEvent( Event );
}