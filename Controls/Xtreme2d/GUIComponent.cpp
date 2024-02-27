#include <Grafik/ContextDescriptor.h>

#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"



GUIComponent::GUIComponent( GR::u32 Id ) :
  GUI::Component( Id ),
  m_pBackground( NULL )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int X, int Y, int Width, int Height, GR::u32 Id ) :
  GUI::Component( X, Y, Width, Height, Id ),
  m_pBackground( NULL )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



GUIComponent::GUIComponent( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id ) :
  GUI::Component( X, Y, Width, Height, Caption, Id ),
  m_pBackground( NULL )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
}



void GUIComponent::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    if ( m_pBackground )
    {
      Displayer.m_pActualRenderer->RenderTextureSection( 0, 0, m_pBackground );
    }
    else if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
    {
      if ( VisualStyle() & GUI::VFT_REPEAT_BKGND )
      {
        Displayer.DrawTextureSectionHVRepeat( 0, 0, Width(), Height(), TextureSection( GUI::BT_BACKGROUND ) );
      }
      else
      {
        Displayer.DrawTextureSection( 0, 0, TextureSection( GUI::BT_BACKGROUND ) );
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



void GUIComponent::Display( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;

  if ( Flags() & GUI::COMPFT_OWNER_DRAW )
  {
    GUI::OwnerDrawInfo    odInfo;

    odInfo.pDisplayer = pDisplayer;
    odInfo.pComponent = this;
    odInfo.ItemIndex  = -1;
    odInfo.ComponentOffset.Set( pCompDisplayer->m_OffsetX, pCompDisplayer->m_OffsetY );


    GenerateEvent( OET_OWNER_DRAW, (GR::up)&odInfo );
  }
  else
  {
    DisplayOnPage( *pCompDisplayer );
  }
}



void GUIComponent::DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer*   pCompDisplayer = (GUIComponentDisplayer*)pDisplayer;
  DisplayNonClientOnPage( *pCompDisplayer );
}



int GUIComponent::GetBorderWidth( const GUI::eBorderType eType ) const
{
  if ( eType >= GUI::BT_EDGE_LAST_ENTRY )
  {
    return 0;
  }
  if ( m_TextureSection[eType].first.m_pTexture )
  {
    if ( ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
    {
      return m_TextureSection[eType].first.m_Width;
    }
  }
  switch ( eType )
  {
    case GUI::BT_EDGE_LEFT:
    case GUI::BT_EDGE_RIGHT:
    case GUI::BT_EDGE_TOP_LEFT:
    case GUI::BT_EDGE_TOP_RIGHT:
    case GUI::BT_EDGE_BOTTOM_LEFT:
    case GUI::BT_EDGE_BOTTOM_RIGHT:
    case GUI::BT_SUNKEN_EDGE_LEFT:
    case GUI::BT_SUNKEN_EDGE_RIGHT:
    case GUI::BT_SUNKEN_EDGE_TOP_LEFT:
    case GUI::BT_SUNKEN_EDGE_TOP_RIGHT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT:
      if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
      {
        return 1;
      }
      if ( ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
      ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
      {
        return 2;
      }
      break;
  }
  return 0;
}



int GUIComponent::GetBorderHeight( const GUI::eBorderType eType ) const
{
  if ( eType >= GUI::BT_EDGE_LAST_ENTRY )
  {
    return 0;
  }
  if ( m_TextureSection[eType].first.m_pTexture )
  {
    if ( ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
    ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
    {
      return m_TextureSection[eType].first.m_Height;
    }
  }
  switch ( eType )
  {
    case GUI::BT_EDGE_TOP:
    case GUI::BT_EDGE_BOTTOM:
    case GUI::BT_EDGE_TOP_LEFT:
    case GUI::BT_EDGE_TOP_RIGHT:
    case GUI::BT_EDGE_BOTTOM_LEFT:
    case GUI::BT_EDGE_BOTTOM_RIGHT:
    case GUI::BT_SUNKEN_EDGE_TOP:
    case GUI::BT_SUNKEN_EDGE_BOTTOM:
    case GUI::BT_SUNKEN_EDGE_TOP_LEFT:
    case GUI::BT_SUNKEN_EDGE_TOP_RIGHT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT:
    case GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT:
      if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
      {
        return 1;
      }
      if ( ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
      ||   ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER ) )
      {
        return 2;
      }
      break;
  }
  return 0;
}




void GUIComponent::SetBackground( XMultiTexture* pBack )
{
  m_pBackground = pBack;
}



XTextureSection GUIComponent::Section( Xtreme::Asset::IAssetLoader* pLoader, const GR::String& Image )
{
  Xtreme::Asset::XAsset* pSection = pLoader->Asset( Xtreme::Asset::XA_IMAGE_SECTION, Image.c_str() );
  if ( pSection == NULL )
  {
    dh::Log( "GUIComponent::Section Section (%s) not found", Image.c_str() );
    return XTextureSection();
  }
  return *(XTextureSection*)pSection->Handle( "Section" );
}



void GUIComponent::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
{
  GUI::Component::ParseXML( pElement, Environment );

  if ( pElement == NULL )
  {
    return;
  }

  Xtreme::Asset::IAssetLoader* pLoader = (Xtreme::Asset::IAssetLoader*)Environment.Service( "AssetLoader" );
  if ( pLoader )
  {
    // default fields
    if ( pElement->HasAttribute( "Font" ) )
    {
      Xtreme::Asset::XAsset* pFont = pLoader->Asset( Xtreme::Asset::XA_FONT,  pElement->Attribute( "Font" ).c_str() );
      if ( pFont == NULL )
      {
        dh::Log( "GUIComponent::ParseXML Font (%s) not found", pElement->Attribute( "Font" ).c_str() );
      }
      else
      {
        SetFont( (XFont*)pFont->Handle( "Font" ) );
      }
    }

    if ( pElement->HasAttribute( "EdgeTopLeft" ) )
    {
      SetTextureSection( GUI::BT_EDGE_TOP_LEFT, Section( pLoader, pElement->Attribute( "EdgeTopLeft" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeTop" ) )
    {
      SetTextureSection( GUI::BT_EDGE_TOP, Section( pLoader, pElement->Attribute( "EdgeTop" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeTopRight" ) )
    {
      SetTextureSection( GUI::BT_EDGE_TOP_RIGHT, Section( pLoader, pElement->Attribute( "EdgeTopRight" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeLeft" ) )
    {
      SetTextureSection( GUI::BT_EDGE_LEFT, Section( pLoader, pElement->Attribute( "EdgeLeft" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeRight" ) )
    {
      SetTextureSection( GUI::BT_EDGE_RIGHT, Section( pLoader, pElement->Attribute( "EdgeRight" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeBottomLeft" ) )
    {
      SetTextureSection( GUI::BT_EDGE_BOTTOM_LEFT, Section( pLoader, pElement->Attribute( "EdgeBottomLeft" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeBottom" ) )
    {
      SetTextureSection( GUI::BT_EDGE_BOTTOM, Section( pLoader, pElement->Attribute( "EdgeBottom" ) ) );
    }
    if ( pElement->HasAttribute( "EdgeBottomRight" ) )
    {
      SetTextureSection( GUI::BT_EDGE_BOTTOM_RIGHT, Section( pLoader, pElement->Attribute( "EdgeBottomRight" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeTopLeft" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_TOP_LEFT, Section( pLoader, pElement->Attribute( "SunkenEdgeTopLeft" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeTop" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_TOP, Section( pLoader, pElement->Attribute( "SunkenEdgeTop" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeTopRight" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_TOP_RIGHT, Section( pLoader, pElement->Attribute( "SunkenEdgeTopRight" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeLeft" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_LEFT, Section( pLoader, pElement->Attribute( "SunkenEdgeLeft" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeRight" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_RIGHT, Section( pLoader, pElement->Attribute( "SunkenEdgeRight" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeBottomLeft" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_BOTTOM_LEFT, Section( pLoader, pElement->Attribute( "SunkenEdgeBottomLeft" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeBottom" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_BOTTOM, Section( pLoader, pElement->Attribute( "SunkenEdgeBottom" ) ) );
    }
    if ( pElement->HasAttribute( "SunkenEdgeBottomRight" ) )
    {
      SetTextureSection( GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT, Section( pLoader, pElement->Attribute( "SunkenEdgeBottomRight" ) ) );
    }

    if ( pElement->HasAttribute( "Background" ) )
    {
      SetTextureSection( GUI::BT_BACKGROUND, Section( pLoader, pElement->Attribute( "Background" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.Button" ) )
    {
      SetCustomTextureSection( GUI::CTS_BUTTON, Section( pLoader, pElement->Attribute( "CustomSection.Button" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ButtonMouseOver" ) )
    {
      SetCustomTextureSection( GUI::CTS_BUTTON_MOUSEOVER, Section( pLoader, pElement->Attribute( "CustomSection.ButtonMouseOver" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ButtonPushed" ) )
    {
      SetCustomTextureSection( GUI::CTS_BUTTON_PUSHED, Section( pLoader, pElement->Attribute( "CustomSection.ButtonPushed" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ButtonDisabled" ) )
    {
      SetCustomTextureSection( GUI::CTS_BUTTON_DISABLED, Section( pLoader, pElement->Attribute( "CustomSection.ButtonDisabled" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ToggleMinus" ) )
    {
      SetCustomTextureSection( GUI::CTS_TOGGLE_MINUS, Section( pLoader, pElement->Attribute( "CustomSection.ToggleMinus" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.TogglePlus" ) )
    {
      SetCustomTextureSection( GUI::CTS_TOGGLE_PLUS, Section( pLoader, pElement->Attribute( "CustomSection.TogglePlus" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowUp" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_UP, Section( pLoader, pElement->Attribute( "CustomSection.ArrowUp" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowUpMouseOver" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_UP_MOUSEOVER, Section( pLoader, pElement->Attribute( "CustomSection.ArrowUpMouseOver" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowUpPushed" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_UP_PUSHED, Section( pLoader, pElement->Attribute( "CustomSection.ArrowUpPushed" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowUpDisabled" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_UP_DISABLED, Section( pLoader, pElement->Attribute( "CustomSection.ArrowUpDisabled" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowDown" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_DOWN, Section( pLoader, pElement->Attribute( "CustomSection.ArrowDown" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowDownMouseOver" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_DOWN_MOUSEOVER, Section( pLoader, pElement->Attribute( "CustomSection.ArrowDownMouseOver" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowDownPushed" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_DOWN_PUSHED, Section( pLoader, pElement->Attribute( "CustomSection.ArrowDownPushed" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.ArrowDownDisabled" ) )
    {
      SetCustomTextureSection( GUI::CTS_ARROW_DOWN_DISABLED, Section( pLoader, pElement->Attribute( "CustomSection.ArrowDownDisabled" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.CheckboxChecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_CHECKBOX_CHECKED, Section( pLoader, pElement->Attribute( "CustomSection.CheckboxChecked" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.CheckboxUnchecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_CHECKBOX_UNCHECKED, Section( pLoader, pElement->Attribute( "CustomSection.CheckboxUnchecked" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.CheckboxCheck" ) )
    {
      SetCustomTextureSection( GUI::CTS_CHECKBOX_CHECK, Section( pLoader, pElement->Attribute( "CustomSection.CheckboxCheck" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.RadioChecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_RADIO_CHECKED, Section( pLoader, pElement->Attribute( "CustomSection.RadioChecked" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.RadioUnchecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_RADIO_UNCHECKED, Section( pLoader, pElement->Attribute( "CustomSection.RadioUnchecked" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.Slider" ) )
    {
      SetCustomTextureSection( GUI::CTS_SLIDER, Section( pLoader, pElement->Attribute( "CustomSection.Slider" ) ) );
    }
  }
}



void GUIComponent::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
}