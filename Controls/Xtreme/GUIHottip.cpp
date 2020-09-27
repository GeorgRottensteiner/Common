#include <Grafik\ContextDescriptor.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIHottip.h"



GUI_IMPLEMENT_CLONEABLE( GUIHottip, "Hottip" )



GUIHottip::GUIHottip( const GR::String& Caption, GR::u32 Id ) :
  AbstractHottip<GUIComponent>( 0, 0, 0, 0, Caption, Id )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUIHottip::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

  Displayer.DrawQuad( 0, 0, m_ClientRect.width(), m_ClientRect.height(), GetColor( GUI::COL_HOTTIP_BACKGROUND ) );

  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );

  Displayer.DrawQuad( 0, 0, m_ClientRect.width(), 1, 0xff000000 );
  Displayer.DrawQuad( 0, m_ClientRect.height() - 1, m_ClientRect.width(), 1, 0xff000000 );
  Displayer.DrawQuad( 0, 0, 1, m_ClientRect.height(), 0xff000000 );
  Displayer.DrawQuad( m_ClientRect.width() - 1, 0, 1, m_ClientRect.height(), 0xff000000 );

  std::vector<GR::String>    vectText;

  GR::tRect   rcClient;

  GetClientRect( rcClient );

  GUI::WrapText( m_pFont, m_Caption, rcClient, vectText );

  rcClient.offset( 0, 2 );


  std::vector<GR::String>::iterator    it( vectText.begin() );
  while ( it != vectText.end() )
  {
    rcClient.Bottom = rcClient.Top + m_pFont->TextHeight( it->c_str() ) + 2;

    Displayer.DrawText( m_pFont, 0, 0, it->c_str(), GUI::AF_CENTER, GetColor( GUI::COL_HOTTIP_TEXT ), &rcClient );

    rcClient.offset( 0, m_pFont->TextHeight( it->c_str() ) );

    ++it;
  }
}



