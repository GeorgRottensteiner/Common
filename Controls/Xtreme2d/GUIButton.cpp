#include "GUIComponentDisplayer.h"
#include "GUIButton.h"



GUI_IMPLEMENT_CLONEABLE( GUIButton, "Button" )



GUIButton::GUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 dwId, GR::u32 dwStyle ) :
  AbstractButton<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, dwId, dwStyle ),
  m_ptPushedTextOffset( 1, 1 )
{
  if ( !( Style() & BCS_NO_BORDER ) )
  {
    ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  }
  SetColor( GUI::COL_WINDOW, GetSysColor( GUI::COL_BTNFACE ) );
}



GUIButton::GUIButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId, GR::u32 dwStyle ) :
  AbstractButton<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwId, dwStyle ),
  m_ptPushedTextOffset( 1, 1 )
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
        Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( 0, 0, tsPushed, colorKey );
      }
      else if ( IsMouseInside() )
      {
        Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( 0, 0, tsMouseOver, colorKey );
      }
      else
      {
        Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( 0, 0, tsNormal, colorKey );
      }
    }
  }
  else
  {
    XTextureSection   tsImage( CustomTextureSection( CTS_IMAGE ) );
    if ( tsImage.m_pTexture )
    {
      GR::u32 dwColorKey = CustomTSColorKey( CTS_IMAGE );

      if ( dwColorKey )
      {
        if ( IsPushed() )
        {
          Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( 1, 1, tsImage, dwColorKey );
        }
        else
        {
          Displayer.m_pActualRenderer->RenderTextureSectionColorKeyed( 0, 0, tsImage, dwColorKey );
        }
      }
      else
      {
        if ( IsPushed() )
        {
          Displayer.m_pActualRenderer->RenderTextureSection( 1, 1, tsImage );
        }
        else
        {
          Displayer.m_pActualRenderer->RenderTextureSection( 0, 0, tsImage );
        }
      }
    }
    else if ( CustomTextureSection( GUI::CTS_BUTTON ).m_pTexture )
    {
      if ( IsPushed() )
      {
        Displayer.m_pActualRenderer->RenderTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON_PUSHED ) );
      }
      else if ( IsMouseInside() )
      {
        Displayer.m_pActualRenderer->RenderTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON_MOUSEOVER ) );
      }
      else
      {
        Displayer.m_pActualRenderer->RenderTextureSection( 0, 0, CustomTextureSection( GUI::CTS_BUTTON ) );
      }
    }
  }
  if ( m_pFont )
  {
    if ( IsPushed() )
    {
      rc.offset( m_ptPushedTextOffset );
    }
    Displayer.DrawText( m_pFont, m_Caption, rc, GUI::AF_DEFAULT, GetColor( GUI::COL_WINDOWTEXT ) );
  }

  if ( IsFocused() )
  {
    GetClientRect( rc );
    rc.inflate( -1, -1 );
    Displayer.DrawFocusRect( rc, VisualStyle() );
  }
}



void GUIButton::SetPushedTextOffset( const GR::tPoint& ptOffset )
{
  m_ptPushedTextOffset = ptOffset;
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
    int   iDX = 0;
    int   iDY = 0;

    if ( IsPushed() )
    {
      iDX = 1;
      iDY = 1;
    }
    if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
    {
      if ( VisualStyle() & GUI::VFT_REPEAT_BKGND )
      {
        Displayer.DrawTextureSectionHVRepeat( iDX, iDY, Width(), Height(), TextureSection( GUI::BT_BACKGROUND ) );
      }
      else
      {
        Displayer.DrawTextureSection( iDX, iDY, TextureSection( GUI::BT_BACKGROUND ) );
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



