#include "GUIComponentDisplayer.h"
#include "GUIButton.h"

GUI_IMPLEMENT_CLONEABLE( GUIButton, "Button" )



GUIButton::GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id, GR::u32 StyleArg ) :
  AbstractButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Id, StyleArg ),
  m_PushedTextOffset( 1, 1 )
{
  if ( !( Style() & BCS_NO_BORDER ) )
  {
    ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  }
  SetColor( GUI::COL_HIGHLIGHTTEXT, GetColor( GUI::COL_WINDOWTEXT ), true );
}



GUIButton::GUIButton( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id, GR::u32 StyleArg ) :
  AbstractButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Id, StyleArg ),
  m_PushedTextOffset( 1, 1 )
{
  if ( !( Style() & BCS_NO_BORDER ) )
  {
    ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  }
  SetColor( GUI::COL_HIGHLIGHTTEXT, GetColor( GUI::COL_WINDOWTEXT ), true );
}



void GUIButton::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  GetClientRect( rc );

  if ( !( Style() & BCS_NO_BORDER ) )
  {
    if ( !( VisualStyle() & GUI::VFT_FLAT_BORDER ) )
    {
      if ( IsPushed() )
      {
        ModifyVisualStyle( GUI::VFT_SUNKEN_BORDER, GUI::VFT_RAISED_BORDER );
      }
      else
      {
        ModifyVisualStyle( GUI::VFT_RAISED_BORDER, GUI::VFT_SUNKEN_BORDER );
      }
    }
  }

  XTextureSection   tsImage1( CustomTextureSection( GUI::CTS_BUTTON ) );
  XTextureSection   tsImage2( CustomTextureSection( GUI::CTS_BUTTON_MOUSEOVER ) );
  XTextureSection   tsImage3( CustomTextureSection( GUI::CTS_BUTTON_PUSHED ) );
  XTextureSection   tsImage4( CustomTextureSection( GUI::CTS_BUTTON_DISABLED ) );

  if ( ( !IsEnabled() )
  &&   ( tsImage4.m_pTexture ) )
  {
    Displayer.DrawTextureSection( 0, 0, tsImage4 );
  }
  else
  {
    if ( ( tsImage1.m_pTexture )
    &&   ( tsImage2.m_pTexture )
    &&   ( tsImage3.m_pTexture ) )
    {
      if ( IsPushed() )
      {
        Displayer.DrawTextureSection( 0, 0, tsImage3 );
      }
      else if ( IsMouseInside() )
      {
        Displayer.DrawTextureSection( 0, 0, tsImage2 );
      }
      else
      {
        Displayer.DrawTextureSection( 0, 0, tsImage1 );
      }
    }
    else if ( tsImage1.m_pTexture )
    {
      Displayer.DrawTextureSection( 0, 0, tsImage1 );
    }
  }

  if ( m_pFont )
  {
    if ( IsPushed() )
    {
      rc.offset( m_PushedTextOffset );
    }
    GR::u32       Color = GetColor( GUI::COL_BTNTEXT );
    if ( ( IsMouseInside() )
    ||   ( IsFocused() ) )
    {
      Color = GetColor( GUI::COL_BTNHIGHLIGHT );
    }
    if ( !IsEnabled() )
    {
      Color = GetColor( GUI::COL_GRAYTEXT );
    }

    Displayer.DrawText( m_pFont, 0, 0, m_Caption, GUI::AF_DEFAULT | TextAlignment(), Color, &rc );
  }

  XTextureSection   tsImage( CustomTextureSection( CTS_IMAGE ) );
  if ( tsImage.m_pTexture )
  {
    if ( IsPushed() )
    {
      Displayer.DrawTextureSection( 1, 1, tsImage );
    }
    else
    {
      Displayer.DrawTextureSection( 0, 0, tsImage );
    }
  }
  
  if ( IsFocused() )
  {
    GetClientRect( rc );
    rc.inflate( -1, -1 );
    Displayer.DrawFocusRect( VisualStyle(), rc );
  }
}



void GUIButton::SetPushedTextOffset( const GR::tPoint& Offset )
{
  m_PushedTextOffset = Offset;
}



