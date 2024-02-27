#include "GUIComponentDisplayer.h"
#include "GUIButton.h"



GUI_IMPLEMENT_CLONEABLE( GUIButton, "Button" )



GUIButton::GUIButton( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id, GR::u32 StyleFlags ) :
  AbstractButton<GUIComponent>( X, Y, Width, Height, Caption, Id, StyleFlags ),
  m_PushedTextOffset( 1, 1 )
{
  if ( !( Style() & BCS_NO_BORDER ) )
  {
    ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  }
  SetColor( GUI::COL_WINDOW, GetSysColor( GUI::COL_BTNFACE ) );
}



GUIButton::GUIButton( int X, int Y, int Width, int Height, GR::u32 Id, GR::u32 StyleFlags ) :
  AbstractButton<GUIComponent>( X, Y, Width, Height, Id, StyleFlags ),
  m_PushedTextOffset( 1, 1 )
{
  if ( !( Style() & BCS_NO_BORDER ) )
  {
    ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  }
  SetColor( GUI::COL_WINDOW, GetSysColor( GUI::COL_BTNFACE ) );
}



void GUIButton::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  GetClientRect( rc );

  XTextureSection   tsNormal( CustomTextureSection( CTS_NORMAL ) );
  XTextureSection   tsMouseOver( CustomTextureSection( CTS_MOUSE_OVER ) );
  XTextureSection   tsPushed( CustomTextureSection( CTS_PUSHED ) );

  if ( tsNormal.m_pTexture )
  {
    GR::u32 colorKey = CustomTSColorKey( CTS_NORMAL );

    if ( colorKey )
    {
      if ( IsPushed() )
      {
        Displayer.DrawTextureSectionColorKeyed( 0, 0, tsPushed, colorKey );
      }
      else if ( IsMouseInside() )
      {
        Displayer.DrawTextureSectionColorKeyed( 0, 0, tsMouseOver, colorKey );
      }
      else
      {
        Displayer.DrawTextureSectionColorKeyed( 0, 0, tsNormal, colorKey );
      }
    }
  }
  else
  {
    XTextureSection   tsImage( CustomTextureSection( CTS_IMAGE ) );
    if ( tsImage.m_pTexture )
    {
      GR::u32 colorKey = CustomTSColorKey( CTS_IMAGE );

      if ( colorKey )
      {
        if ( IsPushed() )
        {
          Displayer.DrawTextureSectionColorKeyed( 1, 1, tsImage, colorKey );
        }
        else
        {
          Displayer.DrawTextureSectionColorKeyed( 0, 0, tsImage, colorKey );
        }
      }
      else
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
    }
    else if ( CustomTextureSection( GUI::CTS_BUTTON ).m_pTexture )
    {
      if ( IsPushed() )
      {
        Displayer.DrawTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON_PUSHED ) );
      }
      else if ( IsMouseInside() )
      {
        Displayer.DrawTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON_MOUSEOVER ) );
      }
      else
      {
        Displayer.DrawTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON ) );
      }
    }
  }
  if ( m_pFont )
  {
    if ( IsPushed() )
    {
      rc.Offset( m_PushedTextOffset );
    }
    Displayer.DrawText( m_pFont, m_Caption, rc, GUI::AF_DEFAULT, GetColor( GUI::COL_WINDOWTEXT ) );
  }

  if ( IsFocused() )
  {
    GetClientRect( rc );
    rc.Inflate( -1, -1 );
    Displayer.DrawFocusRect( rc, VisualStyle() );
  }
}



void GUIButton::SetPushedTextOffset( const GR::tPoint& ptOffset )
{
  m_PushedTextOffset = ptOffset;
}



void GUIButton::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  if ( !( Style() & BCS_NO_BORDER ) )
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


  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    int   dx = 0;
    int   dy = 0;

    if ( IsPushed() )
    {
      dx = 1;
      dy = 1;
    }
    if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
    {
      if ( VisualStyle() & GUI::VFT_REPEAT_BKGND )
      {
        Displayer.DrawTextureSectionHVRepeat( dx, dy, Width(), Height(), TextureSection( GUI::BT_BACKGROUND ) );
      }
      else
      {
        Displayer.DrawTextureSection( dx, dy, TextureSection( GUI::BT_BACKGROUND ) );
      }
    }
    else
    {
      Displayer.DrawQuad( 0, 0, Width(), Height(), GetColor( GUI::COL_WINDOW ) );
    }
  }

  GR::tRect   rectComp;
  GetComponentRect( rectComp );
  Displayer.DrawEdge( m_VisualStyle, rectComp, m_TextureSection );
}



