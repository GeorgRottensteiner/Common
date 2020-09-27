#include "GUIComponentDisplayer.h"
#include "GUISlider.h"



IMPLEMENT_CLONEABLE( GUISlider, "Slider" )



GUISlider::GUISlider( int iNewX, int iNewY, int iNewWidth, int iNewHeight, SliderFlagType sfType, GR::u32 dwId ) :
  AbstractSlider<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, sfType, dwId ),
  m_pImageSliderTop( NULL ),
  m_pImageSliderMiddle( NULL ),
  m_pImageSliderBottom( NULL )
{
}



void GUISlider::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  GR::tRect   rcComp;

  GetClientRect( rcComp );


  // der Slider
  GR::tRect   rc;
  GetSliderRect( rc );

  if ( ( m_pImageSliderTop )
  &&   ( m_pImageSliderMiddle )
  &&   ( m_pImageSliderBottom ) )
  {
    m_pImageSliderTop->PutImage( pPage, rc.Left, rc.Top );

    int   iRestHeight = rc.height() - m_pImageSliderTop->GetHeight() - m_pImageSliderBottom->GetHeight();

    if ( iRestHeight % m_pImageSliderMiddle->GetHeight() )
    {
      iRestHeight = iRestHeight / m_pImageSliderMiddle->GetHeight() + 1;
    }
    else
    {
      iRestHeight = iRestHeight / m_pImageSliderMiddle->GetHeight();
    }
    for ( int i = 0; i < iRestHeight; ++i )
    {
      m_pImageSliderMiddle->PutImage( pPage, rc.Left, rc.Top + m_pImageSliderTop->GetHeight() + i * m_pImageSliderMiddle->GetHeight() );
    }
    m_pImageSliderBottom->PutImage( pPage, rc.Left, rc.Top + rc.height() - m_pImageSliderBottom->GetHeight() );
  }
  else if ( m_pImageSliderMiddle )
  {
    // nur ein einzelnes Slider-Image
    m_pImageSliderMiddle->PutImage( pPage, rc.Left, rc.Top );
  }
  else
  {
    DrawEdge( pPage, GUI::VFT_RAISED_BORDER, rc );
  }

}



void GUISlider::SetImages( GR::Graphic::Image* pImageSliderTop,
                            GR::Graphic::Image* pImageSliderMiddle,
                            GR::Graphic::Image* pImageSliderBottom )
{
  m_pImageSliderTop     = pImageSliderTop;
  m_pImageSliderMiddle  = pImageSliderMiddle;
  m_pImageSliderBottom  = pImageSliderBottom;

  if ( ( m_pImageSliderTop == NULL )
  &&   ( m_pImageSliderBottom == NULL )
  &&   ( m_pImageSliderMiddle ) )
  {
    // nur ein einzelnes Slider-Image
    if ( Style() & GUISlider::SFT_HORIZONTAL )
    {
      SetSizes( m_FullLength, m_pImageSliderMiddle->GetWidth() );
    }
    else
    {
      SetSizes( m_FullLength, m_pImageSliderMiddle->GetHeight() );
    }
  }
}


