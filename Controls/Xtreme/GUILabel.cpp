#include "GUIComponentDisplayer.h"
#include "GUILabel.h"

GUI_IMPLEMENT_CLONEABLE( GUILabel, "Label" )



GUILabel::GUILabel( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 ID ) :
  AbstractLabel<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, GUI::AF_DEFAULT, ID )
{
}

GUILabel::GUILabel( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Flags, const GR::String& Caption, GR::u32 ID ) :
  AbstractLabel<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Flags, ID )
{
}



void GUILabel::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );

  if ( m_pFont )
  {
    GR::tRect   rc;

    GetClientRect( rc );

    rc.Left   += m_TextPos.Left;
    rc.Top    += m_TextPos.Top;
    rc.Right  -= m_TextPos.Right;
    rc.Bottom -= m_TextPos.Bottom;

    if ( Style() & LS_SCALE_TEXT )
    {
      Displayer.DrawTextScaled( m_pFont, 0, 0, m_Caption,
                                m_TextAlignment,
                                GetColor( GUI::COL_WINDOWTEXT ), 
                                &rc, m_ScaleFaktor );
    }
    else
    {
      Displayer.DrawText( m_pFont, 0, 0, m_Caption,
                          m_TextAlignment,
                          GetColor( GUI::COL_WINDOWTEXT ), 
                          &rc );
    }
  }
}



