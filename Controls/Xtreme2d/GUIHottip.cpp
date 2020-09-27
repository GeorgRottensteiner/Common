#include <Grafik\ContextDescriptor.h>
#include <Grafik/Font.h>

#include <String/StringUtil.h>

#include "GUIComponentDisplayer.h"
#include "GUIHottip.h"



GUI_IMPLEMENT_CLONEABLE( GUIHottip, "Hottip" )



GUIHottip::GUIHottip( const GR::String& strCaption, GR::u32 dwId ) :
  AbstractHottip<GUIComponent>( 0, 0, 0, 0, strCaption, dwId )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUIHottip::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.DrawQuad( 0, 0, m_ClientRect.width(), m_ClientRect.height(), 0xc0ffffff );

  Displayer.DrawRect( GR::tPoint(), m_ClientRect.size(), 0xff000000 );

  std::vector<GR::String>    vectText;

  GR::tRect   rcTemp;

  GetClientRect( rcTemp );

  GR::Strings::WrapText( m_pFont, m_Caption, rcTemp, vectText );

  rcTemp.offset( 2, 2 );

  std::vector<GR::String>::iterator    it( vectText.begin() );
  while ( it != vectText.end() )
  {
    rcTemp.Bottom = rcTemp.Top + m_pFont->TextHeight( *it ) + 2;

    Displayer.DrawText( m_pFont, 0, 0, *it, GUI::AF_CENTER, GetColor( GUI::COL_WINDOWTEXT ), &rcTemp );

    rcTemp.offset( 0, m_pFont->TextHeight( *it ) );

    ++it;
  }
}



