#include <debug/debugclient.h>

#include "GUIComponentDisplayer.h"
#include "GUIRadioButton.h"



GUI_IMPLEMENT_CLONEABLE( GUIRadioButton, "RadioButton" )



GUIRadioButton::GUIRadioButton( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Id ) :
  AbstractRadioButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Id ),
  m_PushedColor( 0xffffffff ),
  m_MouseOverColor( 0xffffa000 )
{
  SetColor( GUI::COL_WINDOWTEXT, 0xff000000 );
}



GUIRadioButton::GUIRadioButton( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id ) :
  AbstractRadioButton<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Id ),
  m_PushedColor( 0xffffffff ),
  m_MouseOverColor( 0xffffa000 )
{
  SetColor( GUI::COL_WINDOWTEXT, 0xff000000 );
}



void GUIRadioButton::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

  int     boxSize = 20;

  if ( m_ClientRect.height() < boxSize )
  {
    boxSize = m_ClientRect.height();
  }
  
  XTextureSection   tsChecked = CustomTextureSection( GUI::CTS_RADIO_CHECKED );
  XTextureSection   tsUnchecked = CustomTextureSection( GUI::CTS_RADIO_UNCHECKED );

  if ( ( tsChecked.m_pTexture )
  &&   ( tsUnchecked.m_pTexture ) )
  {
    boxSize = tsChecked.m_Height;
  }

  if ( m_pFont )
  {
    GR::tRect   rcText( ( m_ClientRect.height() - boxSize ) / 2 + boxSize + 4, 0,
                          m_ClientRect.width() - ( m_ClientRect.height() - boxSize ) / 2 - boxSize - 2, m_ClientRect.height() );
                        
    Displayer.DrawText( m_pFont, 0, 0, m_Caption, GUI::AF_LEFT | GUI::AF_VCENTER, GetColor( GUI::COL_WINDOWTEXT ), &rcText );
  }

  if ( ( tsChecked.m_pTexture )
  &&   ( tsUnchecked.m_pTexture ) )
  {
    int   CheckPosX = 0;

    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

    if ( IsChecked() )
    {
      Displayer.DrawTextureSection( ( m_Height - boxSize ) / 2 + CheckPosX, ( m_Height - boxSize ) / 2, tsChecked );
    }
    else
    {
      Displayer.DrawTextureSection( ( m_Height - boxSize ) / 2 + CheckPosX, ( m_Height - boxSize ) / 2, tsUnchecked );
    }
  }
  else
  {
    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
    Displayer.DrawQuad( ( m_ClientRect.height() - boxSize ) / 2, ( m_ClientRect.height() - boxSize ) / 2,
                        boxSize, boxSize, GetSysColor( GUI::COL_WINDOW ) );
    if ( IsChecked() )
    {
      Displayer.DrawLine( GR::tPoint( ( m_ClientRect.height() - boxSize ) / 2, ( m_ClientRect.height() - boxSize ) / 2 ),
                          GR::tPoint( ( m_ClientRect.height() - boxSize ) / 2 + boxSize - 1, ( m_ClientRect.height() - boxSize ) / 2 + boxSize - 1 ),
                          GetColor( GUI::COL_WINDOWTEXT ) );
      Displayer.DrawLine( GR::tPoint( ( m_ClientRect.height() - boxSize ) / 2 + boxSize - 1, ( m_ClientRect.height() - boxSize ) / 2 ),
                          GR::tPoint( ( m_ClientRect.height() - boxSize ) / 2, ( m_ClientRect.height() - boxSize ) / 2 + boxSize - 1 ),
                          GetColor( GUI::COL_WINDOWTEXT ) );
    }
  }
}



