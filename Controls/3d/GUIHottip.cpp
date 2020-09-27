#include <Grafik\ContextDescriptor.h>
#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIHottip.h"



GUI_IMPLEMENT_CLONEABLE( CGUIHottip, "Hottip" )



CGUIHottip::CGUIHottip( const char* szCaption, DWORD dwId ) :
  CAbstractHottip<CGUIComponent>( 0, 0, 0, 0, szCaption, dwId )
{

  ModifyEdge( GUI::GET_TRANSPARENT_BKGND );

}



void CGUIHottip::DisplayOnPage( CD3DViewer* pViewer, int iOffsetX, int iOffsetY )
{
  /*
  pRenderer->SetShader( XRenderer::ST_ALPHA_BLEND );

  pViewer->SetTexture( 0, NULL );
  pViewer->RenderQuad2d( iXOffset, iYOffset, m_ClientRect.width(), m_ClientRect.height(), 0xc0ffffff );

  pRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );

  pRenderer->RenderQuad2d( iXOffset, iYOffset, m_ClientRect.width(), 1, 0xff000000 );
  pRenderer->RenderQuad2d( iXOffset, iYOffset + m_ClientRect.height() - 1, m_ClientRect.width(), 1, 0xff000000 );
  pRenderer->RenderQuad2d( iXOffset, iYOffset, 1, m_ClientRect.height(), 0xff000000 );
  pRenderer->RenderQuad2d( iXOffset + m_ClientRect.width() - 1, iYOffset, 1, m_ClientRect.height(), 0xff000000 );

  std::vector<GR::String>    vectText;

  GR::tRect   rcClient;

  GetClientRect( rcClient );

  WrapText( m_Caption, rcClient, vectText );

  rcClient.offset( 0, 2 );

  std::vector<GR::String>::iterator    it( vectText.begin() );
  while ( it != vectText.end() )
  {
    rcClient.Bottom = rcClient.Top + m_pFont->TextHeight( it->c_str() ) + 2;

    DisplayText( pRenderer, iXOffset, iYOffset, it->c_str(), GUI::AF_CENTER, GetColor( GUI::COL_WINDOWTEXT ), &rcClient );

    rcClient.offset( 0, m_pFont->TextHeight( it->c_str() ) );

    ++it;
  }
  */
}



