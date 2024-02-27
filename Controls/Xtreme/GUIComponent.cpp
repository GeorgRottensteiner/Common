#include <debug/debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUIComponent.h"



GUI_IMPLEMENT_CLONEABLE( GUIComponent, "GUI.Component" )



GUIComponent::GUIComponent( GR::u32 Id ) :
    GUI::Component( Id )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();

  m_CustomTextureSections = GUIComponentDisplayer::Instance().m_CustomTextureSections;
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );

  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
}



GUIComponent::GUIComponent( int X, int Y, int Width, int Height, GR::u32 Id ) :
    GUI::Component( X, Y, Width, Height, Id )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  m_CustomTextureSections = GUIComponentDisplayer::Instance().m_CustomTextureSections;
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );

  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
}



GUIComponent::GUIComponent( int X, int Y, int Width, int Height, const GR::String& Caption, GR::u32 Id ) :
    GUI::Component( X, Y, Width, Height, Caption, Id )
{
  m_pFont = GUIComponentDisplayer::Instance().DefaultFont();
  m_CustomTextureSections = GUIComponentDisplayer::Instance().m_CustomTextureSections;
  memcpy( m_Colors, GUIComponentDisplayer::Instance().m_Colors, sizeof( m_Colors ) );
  m_TextureSection.resize( GUI::BT_EDGE_LAST_ENTRY );
  for ( int i = 0; i < GUI::BT_EDGE_LAST_ENTRY; ++i )
  {
    m_TextureSection[i] = GUIComponentDisplayer::Instance().m_DefaultTextureSection[i];
  }
}



void GUIComponent::Display( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer& CompDisplayer( *(GUIComponentDisplayer*)pDisplayer );

  if ( Flags() & GUI::COMPFT_OWNER_DRAW )
  {
    GUI::OwnerDrawInfo    odInfo;

    odInfo.pDisplayer = pDisplayer;
    odInfo.pComponent = this;
    odInfo.ItemIndex  = -1;
    odInfo.ItemSize.Set( m_ClientRect.Width(), m_ClientRect.Height() );
    odInfo.ComponentOffset.Set( 0, 0 );
    

    GenerateEvent( OET_OWNER_DRAW, (GR::up)&odInfo );
  }
  else
  {
    DisplayOnPage( CompDisplayer );
  }
}



void GUIComponent::DisplayNonClient( GUI::ComponentDisplayerBase* pDisplayer )
{
  GUIComponentDisplayer&    guiDisplayer( *(GUIComponentDisplayer*)pDisplayer );

  DisplayNonClientOnPage( guiDisplayer );
}



void GUIComponent::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  if ( !( VisualStyle() & GUI::VFT_TRANSPARENT_BKGND ) )
  {
    GR::tRect  rcClient;

    GR::tPoint ptClientOffset = GetClientOffset();


    GetClientRect( rcClient );
    
    if ( m_TextureSection[GUI::BT_BACKGROUND].first.m_pTexture )
    {
      if ( ( VisualStyle() & GUI::VFT_REPEAT_BKGND ) != 0 )
      {
        Displayer.DrawTiled( ptClientOffset.x, ptClientOffset.y, rcClient, m_TextureSection[GUI::BT_BACKGROUND].first, GetColor( GUI::COL_BTNFACE ) );
      }
      else
      {
        if ( ( ( m_TextureSection[GUI::BT_BACKGROUND].first.m_Flags & XTextureSection::TSF_ROTATE_270 ) != 0 )
        &&   ( ( m_TextureSection[GUI::BT_BACKGROUND].first.m_Flags & XTextureSection::TSF_ROTATE_270 ) != XTextureSection::TSF_ROTATE_180 ) )
        {
          // zurückdrehen, texture section dreht selbst
          Displayer.DrawTextureSection( ptClientOffset.x, 
                                        ptClientOffset.y, 
                                        m_TextureSection[GUI::BT_BACKGROUND].first, 
                                        GetColor( GUI::COL_BTNFACE ),
                                        rcClient.Height(),
                                        rcClient.Width() );
        }
        else
        {
          Displayer.DrawTextureSection( ptClientOffset.x, 
                                        ptClientOffset.y, 
                                        m_TextureSection[GUI::BT_BACKGROUND].first,
                                        GetColor( GUI::COL_BTNFACE ),
                                        rcClient.Width(),
                                        rcClient.Height() );
        }
      }
    }
    else
    {
      GR::u32 Color = GetColor( GUI::COL_BTNFACE );

      if ( ( Color & 0xff000000 ) != 0xff000000 )
      {
        Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );
        Displayer.m_pActualRenderer->SetTexture( 0, NULL );
      }
      else
      {
        Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
      }

      Displayer.DrawQuad( ptClientOffset.x, 
                          ptClientOffset.y, 
                          rcClient.Width(), rcClient.Height(),
                          Color );
    }
  }

  GR::tRect   rectComp;
  GetComponentRect( rectComp );

  Displayer.DrawEdge( VisualStyle(), rectComp, m_TextureSection );
}



void GUIComponent::SetFont( Interface::IFont* pFont )
{
  m_pFont = pFont;
}



void GUIComponent::PrepareStatesForAlpha( GUIComponentDisplayer& Displayer, GR::u32 Color )
{
  if ( ( Color & 0xff000000 ) == 0xff000000 )
  {
    // volles Alpha, braucht kein Alphablending
    Displayer.m_pActualRenderer->SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_DISABLE );
  }
  else
  {
    Displayer.m_pActualRenderer->SetState( XRenderer::RS_ALPHABLENDING, XRenderer::RSV_ENABLE );
    Displayer.m_pActualRenderer->SetState( XRenderer::RS_SRC_BLEND,   XRenderer::RSV_SRC_ALPHA );
    Displayer.m_pActualRenderer->SetState( XRenderer::RS_DEST_BLEND,  XRenderer::RSV_INV_SRC_ALPHA );
  }
}



void GUIComponent::AdjustRect( GR::tPoint& Size, bool ClientToNC )
{
  int     deltaX = 0;
  int     deltaY = 0;

  if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_FLAT_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width == 0 )
    {
      deltaX = 4;
      deltaY = 4;
      if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
      {
        deltaX = 2;
        deltaY = 2;
      }
    }
    else
    {
      deltaX += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width + m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width;
      deltaY += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Height + m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height;
    }
  }

  if ( ClientToNC )
  {
    Size.x += deltaX;
    Size.y += deltaY;
  }
  else
  {
    Size.x -= deltaX;
    Size.y -= deltaY;
  }
}



void GUIComponent::RecalcClientRect()
{
  m_ClientRect.Set( 0, 0, m_Width, m_Height );

  if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_FLAT_BORDER ) )
  {
    if ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
    {
      if ( m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width == 0 )
      {
        int   iBorderWidth = 2;
        if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
        {
          iBorderWidth = 1;
        }
        m_ClientRect.Inflate( -iBorderWidth, -iBorderWidth );
      }
      else
      {
        m_ClientRect.Left += m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Width;
        m_ClientRect.Top += m_TextureSection[GUI::BT_SUNKEN_EDGE_TOP_LEFT].first.m_Height;

        m_ClientRect.Right -= m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Width;
        m_ClientRect.Bottom -= m_TextureSection[GUI::BT_SUNKEN_EDGE_BOTTOM_RIGHT].first.m_Height;
      }
    }
    else
    {
      if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width == 0 )
      {
        int   iBorderWidth = 2;
        if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
        {
          iBorderWidth = 1;
        }
        m_ClientRect.Inflate( -iBorderWidth, -iBorderWidth );
      }
      else
      {
        m_ClientRect.Left += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width;
        m_ClientRect.Top += m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Height;

        m_ClientRect.Right -= m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Width;
        m_ClientRect.Bottom -= m_TextureSection[GUI::BT_EDGE_BOTTOM_RIGHT].first.m_Height;
      }
    }
  }
}



int GUIComponent::GetBorderWidth( const GUI::eBorderType Type ) const
{
  if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_FLAT_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width == 0 )
    {
      // nichts gesetzt
      if ( Type != GUI::BT_BACKGROUND )
      {
        if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
        {
          return 1;
        }
        return 2;
      }
    }
  }
  return m_TextureSection[Type].first.m_Width;
}



int GUIComponent::GetBorderHeight( const GUI::eBorderType Type ) const
{
  if ( ( m_VisualStyle & GUI::VFT_SUNKEN_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_RAISED_BORDER )
  ||   ( m_VisualStyle & GUI::VFT_FLAT_BORDER ) )
  {
    if ( m_TextureSection[GUI::BT_EDGE_TOP_LEFT].first.m_Width == 0 )
    {
      // nichts gesetzt
      if ( Type != GUI::BT_BACKGROUND )
      {
        if ( m_VisualStyle & GUI::VFT_FLAT_BORDER )
        {
          return 1;
        }
        return 2;
      }
    }
  }
  return m_TextureSection[Type].first.m_Height;
}



GR::u32 GUIComponent::GetSysColor( GUI::eColorIndex eColor )
{
  return GUIComponentDisplayer::Instance().GetSysColor( eColor ) | 0xff000000;
}



void GUIComponent::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
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



void GUIComponent::SetSectionFromAttribute( Xtreme::Asset::IAssetLoader* pLoader, GR::Strings::XMLElement* pElement, const GR::String& Attribute, GUI::eBorderType Border )
{
  if ( !pElement->HasAttribute( Attribute ) )
  {
    return;
  }
  GR::String    attributeValue = pElement->Attribute( Attribute );
  if ( attributeValue.empty() )
  {
    // explicit none!
    SetTextureSection( Border, XTextureSection() );
  }
  else
  {
    SetTextureSection( Border, Section( pLoader, attributeValue ) );
  }
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

    SetSectionFromAttribute( pLoader, pElement, "EdgeTopLeft", GUI::BT_EDGE_TOP_LEFT );
    SetSectionFromAttribute( pLoader, pElement, "EdgeTop", GUI::BT_EDGE_TOP );
    SetSectionFromAttribute( pLoader, pElement, "EdgeTopRight", GUI::BT_EDGE_TOP_RIGHT );
    SetSectionFromAttribute( pLoader, pElement, "EdgeLeft", GUI::BT_EDGE_LEFT );
    SetSectionFromAttribute( pLoader, pElement, "EdgeRight", GUI::BT_EDGE_RIGHT );
    SetSectionFromAttribute( pLoader, pElement, "EdgeBottomLeft", GUI::BT_EDGE_BOTTOM_LEFT );
    SetSectionFromAttribute( pLoader, pElement, "EdgeBottom", GUI::BT_EDGE_BOTTOM );
    SetSectionFromAttribute( pLoader, pElement, "EdgeBottomRight", GUI::BT_EDGE_BOTTOM_RIGHT );
    /*
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
    }*/
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
    if ( pElement->HasAttribute( "CustomSection.RadioButtonChecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_RADIO_CHECKED, Section( pLoader, pElement->Attribute( "CustomSection.RadioButtonChecked" ) ) );
    }
    if ( pElement->HasAttribute( "CustomSection.RadioButtonUnchecked" ) )
    {
      SetCustomTextureSection( GUI::CTS_RADIO_UNCHECKED, Section( pLoader, pElement->Attribute( "CustomSection.RadioButtonUnchecked" ) ) );
    }
  }




}