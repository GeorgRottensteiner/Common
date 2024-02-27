#include <debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUISlider.h"



GUI_IMPLEMENT_CLONEABLE( GUISlider, "Slider" )



GUISlider::GUISlider( int NewX, int NewY, int NewWidth, int NewHeight, SliderFlagType SliderFlags, GR::u32 Id ) :
  AbstractSlider<GUIComponent>( NewX, NewY, NewWidth, NewHeight, SliderFlags, Id )
{
}



void GUISlider::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::u32     color     = 0xff606060;
  GR::tRect   rc;

  GetClientRect( rc );

  if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
  {
    if ( Style() & GUISlider::SFT_HORIZONTAL )
    {
      Displayer.DrawTextureSectionHRepeat( 0, 0, rc.Width(), TextureSection( GUI::BT_BACKGROUND ) );
    }
    else
    {
      Displayer.DrawTextureSectionVRepeat( 0, 0, rc.Height(), TextureSection( GUI::BT_BACKGROUND ) );
    }
  }
  else
  {
    Displayer.DrawQuad( 0, 0, rc.Width(), rc.Height(), color );
  }

  GetSliderRect( rc );

  if ( CustomTextureSection( GUI::CTS_SLIDER ).m_pTexture )
  {
    Displayer.DrawTextureSection( rc.Left, rc.Top, CustomTextureSection( GUI::CTS_SLIDER ) );
  }
  else
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 0xff808080 );
  }
}



void GUISlider::SetCustomTextureSection( const GR::u32 Type, const XTextureSection& TexSection, GR::u32 ColorKey )
{
  AbstractSlider<GUIComponent>::SetCustomTextureSection( Type, TexSection, ColorKey );

  if ( Type == GUI::CTS_SLIDER )
  {
    SetSizes( m_FullLength, TexSection.m_Width );
  }
}