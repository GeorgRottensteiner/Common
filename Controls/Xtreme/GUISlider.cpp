#include "GUIComponentDisplayer.h"
#include "GUISlider.h"

GUI_IMPLEMENT_CLONEABLE( GUISlider, "Slider" )



GUISlider::GUISlider( int NewX, int NewY, int NewWidth, int NewHeight, SliderFlagType Flags, GR::u32 Id ) :
  AbstractSlider<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Flags, Id )
{

}



void GUISlider::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  GetSliderRect( rc );

  XTextureSection     TSSlider = CustomTextureSection( GUI::CTS_SLIDER );
  if ( TSSlider.m_pTexture )
  {
    //SetSizes( m_FullLength, TSSlider.m_Height );
    if ( ( TSSlider.m_pTexture->m_ImageFormat == GR::Graphic::IF_A4R4G4B4 )
    ||   ( TSSlider.m_pTexture->m_ImageFormat == GR::Graphic::IF_A8R8G8B8 ) )
    {
      Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );
    }
    Displayer.DrawTextureSection( rc.Left, rc.Top, TSSlider );
    return;
  }

  if ( Style() & AbstractSlider::SFT_HORIZONTAL )
  {
    XTextureSection     TSSlider1 = CustomTextureSection( GUI::CTS_SLIDER_LEFT );
    XTextureSection     TSSlider2 = CustomTextureSection( GUI::CTS_SLIDER_HCENTER );
    XTextureSection     TSSlider3 = CustomTextureSection( GUI::CTS_SLIDER_RIGHT );

    if ( ( TSSlider1.m_pTexture )
    &&   ( TSSlider2.m_pTexture )
    &&   ( TSSlider3.m_pTexture ) )
    {
      Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT );

      Displayer.DrawTextureSection( rc.Left, rc.Top, TSSlider1 );

      Displayer.DrawTextureSectionHRepeat( rc.Left + TSSlider1.VisualWidth(), rc.Top,
                             rc.width() - TSSlider3.VisualWidth() - TSSlider1.VisualWidth(),
                             TSSlider2,
                             0xffffffff );
      Displayer.DrawTextureSection( rc.Right - TSSlider3.VisualWidth(), rc.Top, TSSlider3 );
      return;
    }
  }
  else
  {
    XTextureSection     TSSlider1 = CustomTextureSection( GUI::CTS_SLIDER_TOP );
    XTextureSection     TSSlider2 = CustomTextureSection( GUI::CTS_SLIDER_VCENTER );
    XTextureSection     TSSlider3 = CustomTextureSection( GUI::CTS_SLIDER_BOTTOM );

    if ( ( TSSlider1.m_pTexture )
    &&   ( TSSlider2.m_pTexture )
    &&   ( TSSlider3.m_pTexture ) )
    {
      Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT );

      Displayer.DrawTextureSection( rc.Left, rc.Top, TSSlider1 );

      Displayer.DrawTextureSectionVRepeat( rc.Left, rc.Top + TSSlider1.VisualHeight(),
                             rc.height() - TSSlider3.VisualHeight() - TSSlider1.VisualHeight(),
                             TSSlider2,
                             0xffffffff );
      
      Displayer.DrawTextureSection( rc.Left, rc.Top + rc.height() - TSSlider3.VisualHeight(), TSSlider3 );
      return;
    }
  }

  Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetColor( GUI::COL_HIGHLIGHT ) );
}



bool GUISlider::ProcessEvent( const GUI::ComponentEvent& Event )
{
  switch ( Event.Type )
  {
    case CET_CUSTOM_SECTION_CHANGED:
      if ( Event.Value == GUI::CTS_SLIDER )
      {
        // slider image changed, adjust size
        int     sliderSize = CustomTextureSection( (GR::u32)Event.Value ).m_Width;
        if ( !( Style() & SFT_HORIZONTAL ) )
        {
          sliderSize = CustomTextureSection( (GR::u32)Event.Value ).m_Height;
        }
        SetSizes( m_FullLength, sliderSize );
      }
      break;
  }

  return AbstractSlider<GUIComponent>::ProcessEvent( Event );
}



bool GUISlider::IsSliderResizeable()
{
  if ( CustomTextureSection( GUI::CTS_SLIDER ).m_Width != 0 )
  {
    return false;
  }
  return AbstractSlider<GUIComponent>::IsSliderResizeable();
}
