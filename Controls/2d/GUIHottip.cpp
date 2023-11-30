#include <Grafik\ContextDescriptor.h>
#include <Grafik/Font.h>

#include <String/StringUtil.h>

#include "GUIComponentDisplayer.h"
#include "GUIHottip.h"



GUIHottip::GUIHottip( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, DWORD dwId ) :
  AbstractHottip<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUIHottip::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  GR::Graphic::ContextDescriptor      cdPage( pPage );

  cdPage.AlphaBox( 0, 0, m_ClientRect.size().x, m_ClientRect.size().y, 0xffffff, 192 );
  pPage->Rectangle( 0, 0, m_ClientRect.size().x - 1, m_ClientRect.size().y - 1, 0 );

  std::vector<GR::String>    vectText;

  GR::tRect   rcClientOrig;

  GetClientRect( rcClientOrig );

  GR::tRect   rcClient( rcClientOrig );

  GR::Strings::WrapText( m_pFont, m_Caption, rcClient, vectText );

  rcClient.offset( 0, 2 );

  rcClient.Left = rcClientOrig.Left;
  rcClient.Right = rcClientOrig.Right;

  std::vector<GR::String>::iterator    it( vectText.begin() );
  while ( it != vectText.end() )
  {
    rcClient.Bottom = rcClient.Top + m_pFont->TextHeight( it->c_str() ) + 2;

    DrawText( pPage, it->c_str(), rcClient );

    rcClient.offset( 0, m_pFont->TextHeight( it->c_str() ) );

    ++it;
  }
}



ICloneAble* GUIHottip::Clone()
{
  return new GUIHottip( *this );
}