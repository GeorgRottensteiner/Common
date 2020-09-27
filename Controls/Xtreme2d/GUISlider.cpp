#include <debug\debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUISlider.h"



GUI_IMPLEMENT_CLONEABLE( GUISlider, "Slider" )



GUISlider::GUISlider( int iNewX, int iNewY, int iNewWidth, int iNewHeight, SliderFlagType sftFlags, GR::u32 dwId ) :
  AbstractSlider<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, sftFlags, dwId )
{
}



void GUISlider::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::u32   dwColor     = 0xff606060;

  GR::tRect   rc;

  GetClientRect( rc );

  if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
  {
    if ( Style() & GUISlider::SFT_HORIZONTAL )
    {
      Displayer.DrawTextureSectionHRepeat( 0, 0, rc.width(), TextureSection( GUI::BT_BACKGROUND ) );
    }
    else
    {
      Displayer.DrawTextureSectionVRepeat( 0, 0, rc.height(), TextureSection( GUI::BT_BACKGROUND ) );
    }
  }
  else
  {
    Displayer.DrawQuad( 0, 0, rc.width(), rc.height(), dwColor );
  }

  GetSliderRect( rc );

  if ( CustomTextureSection( GUI::CTS_SLIDER ).m_pTexture )
  {
    Displayer.DrawTextureSection( rc.Left, rc.Top, CustomTextureSection( GUI::CTS_SLIDER ) );
  }
  else
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), 0xff808080 );
  }
}



void GUISlider::SetCustomTextureSection( const GR::u32 dwType, const XTextureSection& TexSection, GR::u32 dwColorKey )
{
  AbstractSlider<GUIComponent>::SetCustomTextureSection( dwType, TexSection, dwColorKey );

  if ( dwType == GUI::CTS_SLIDER )
  {
    SetSizes( m_FullLength, TexSection.m_Width );
  }
}