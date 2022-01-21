#include "GUIComponentDisplayer.h"
#include "GUICheckBox.h"



GUI_IMPLEMENT_CLONEABLE( GUICheckBox, "Checkbox" )




GUICheckBox::GUICheckBox( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Flags, GR::u32 Id ) :
  AbstractCheckBox<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Flags, Id )
{
}



GUICheckBox::GUICheckBox( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 dwId ) :
  AbstractCheckBox<GUIComponent>( NewX, NewY, NewWidth, NewHeight, dwId )
{

}



void GUICheckBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

  int     boxSize = 20;

  if ( m_Height - 4 < boxSize )
  {
    boxSize = m_Height - 4;
  }

  XTextureSection   tsChecked = CustomTextureSection( GUI::CTS_CHECKBOX_CHECKED );
  XTextureSection   tsUnchecked = CustomTextureSection( GUI::CTS_CHECKBOX_UNCHECKED );

  if ( ( tsChecked.m_pTexture )
  &&   ( tsUnchecked.m_pTexture ) )
  {
    boxSize = tsChecked.m_Height;
  }
  
  if ( m_pFont )
  {
    if ( m_Style & GUICheckBox::CS_CHECKBOX_RIGHT )
    {
      GR::tRect   rcText( 0, 0,
                          m_ClientRect.width() - ( m_Height - boxSize ) / 2 - boxSize - 2, m_ClientRect.height() );

      Displayer.DrawText( m_pFont, 0, 0, m_Caption, m_TextAlignment, GetColor( GUI::COL_WINDOWTEXT ), &rcText );
    }
    else
    {
      GR::tRect   rcText( ( m_Height - boxSize ) / 2 + boxSize + 4, 0,
                          m_ClientRect.width() - ( m_Height - boxSize ) / 2 - boxSize - 2, m_ClientRect.height() );

      Displayer.DrawText( m_pFont, 0, 0, m_Caption, m_TextAlignment, GetColor( GUI::COL_WINDOWTEXT ), &rcText );
    }
  }

  if ( ( tsChecked.m_pTexture )
  &&   ( tsUnchecked.m_pTexture ) )
  {
    int   CheckPosX = 0;

    if ( m_Style & GUICheckBox::CS_CHECKBOX_RIGHT )
    {
      CheckPosX = m_Width - tsChecked.m_Width;
    }


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
    int   CheckPosX = 0;

    if ( m_Style & GUICheckBox::CS_CHECKBOX_RIGHT )
    {
      CheckPosX = m_Width - boxSize;
    }

    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
    if ( IsPushed() )
    {
      Displayer.DrawQuad( ( m_Height - boxSize ) / 2 + CheckPosX, ( m_Height - boxSize ) / 2,
                          boxSize, boxSize, GetColor( GUI::COL_BTNFACE ) );
    }
    else
    {
      Displayer.DrawQuad( ( m_Height - boxSize ) / 2 + CheckPosX, ( m_Height - boxSize ) / 2,
                          boxSize, boxSize, GetColor( GUI::COL_WINDOW ) );
    }
    Displayer.DrawEdge( GUI::VFT_SUNKEN_BORDER, 
                        GR::tRect( ( m_Height - boxSize ) / 2 + CheckPosX, ( m_Height - boxSize ) / 2, boxSize, boxSize ) );
    if ( IsChecked() )
    {
      XTextureSection   tsCheck = CustomTextureSection( GUI::CTS_CHECKBOX_CHECK );
      if ( tsCheck.m_pTexture )
      {
        Displayer.DrawTextureSection( ( m_Height - tsCheck.m_Height ) / 2 + CheckPosX,
                                      ( m_Height - tsCheck.m_Height ) / 2 + CheckPosX,
                                      tsCheck );
      }
      else
      {
        Displayer.DrawLine( GR::tPoint( ( m_Height - boxSize ) / 2 + CheckPosX,                ( m_Height - boxSize ) / 2 ),
                            GR::tPoint( ( m_Height - boxSize ) / 2 + CheckPosX + boxSize - 1, ( m_Height - boxSize ) / 2 + boxSize - 1 ),
                            GetColor( GUI::COL_WINDOWTEXT ) );
        Displayer.DrawLine( GR::tPoint( ( m_Height - boxSize ) / 2 + CheckPosX + boxSize - 1, ( m_Height - boxSize ) / 2 ),
                            GR::tPoint( ( m_Height - boxSize ) / 2 + CheckPosX,                ( m_Height - boxSize ) / 2 + boxSize - 1 ),
                            GetColor( GUI::COL_WINDOWTEXT ) );
      }
    }
  }
}



