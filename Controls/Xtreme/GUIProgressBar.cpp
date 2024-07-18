#include "GUIComponentDisplayer.h"
#include "GUIProgressBar.h"



IMPLEMENT_CLONEABLE( GUIProgressBar, "GUI.ProgressBar" )

GUIProgressBar::GUIProgressBar( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 ID ) :
  AbstractProgressBar<GUIComponent>( NewX, NewY, NewWidth, NewHeight, ID )
{
}



void GUIProgressBar::DisplayOnPage( XRenderer* pRenderer )
{
  pRenderer->SetShader( XRenderer::ST_ALPHA_BLEND_AND_TEST );

  GR::u32   trueWidth = 0;

  if ( m_FullLength )
  {
    trueWidth = ( m_ClientRect.Width() * m_CurrentPos / m_FullLength );
  }

  if ( trueWidth )
  {
    if ( m_tsFillImage.m_Width )
    {
      GR::u32   pos = 0;

      while ( pos < trueWidth )
      {
        if ( trueWidth - pos >= ( GR::u32 )m_tsFillImage.m_Width )
        {
          pRenderer->RenderTextureSection2d( pos, 0, m_tsFillImage, GetColor( GUI::COL_BTNFACE ) );
          pos += m_tsFillImage.m_Width;
        }
        else
        {
          XTextureSection   tsTemp( m_tsFillImage );

          tsTemp.m_Width = trueWidth - pos;
          pRenderer->RenderTextureSection2d( pos, 0, tsTemp, GetColor( GUI::COL_BTNFACE ) );
          pos = trueWidth;
        }
      }
    }
    else
    {
      pRenderer->SetTexture( 0, NULL );
      pRenderer->RenderQuad2d( 0, 0, trueWidth, m_ClientRect.Height(), GetColor( GUI::COL_BTNFACE ) );
    }
  }
}



void GUIProgressBar::SetImage( const XTextureSection& tsImage )
{
  m_tsFillImage = tsImage;
}
