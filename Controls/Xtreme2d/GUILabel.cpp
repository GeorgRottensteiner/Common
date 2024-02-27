#include "GUIComponentDisplayer.h"
#include "GUILabel.h"



GUI_IMPLEMENT_CLONEABLE( GUILabel, "Label" )



GUILabel::GUILabel( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 ID ) :
  AbstractLabel<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, GUI::AF_DEFAULT, ID )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



GUILabel::GUILabel( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Flags, const GR::String& Caption, GR::u32 ID ) :
  AbstractLabel<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Flags, ID )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUILabel::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
  if ( m_pFont )
  {
    GR::tRect   rc;

    GetClientRect( rc );

    rc.Left   += m_TextPos.Left;
    rc.Top    += m_TextPos.Top;
    rc.Right  -= m_TextPos.Width();
    rc.Bottom -= m_TextPos.Height();

    /*
    if ( Style() & LS_SCALE_TEXT )
    {
      DisplayTextScaled( pRenderer, iXOffset, iYOffset, m_strCaption.c_str(),
                         m_textAlignment, GetSysColor( GUI::COL_WINDOWTEXT ),
                         &rc, m_fScaleFaktor );
    }
    else
    */
    {
      Displayer.DrawText( m_pFont, m_Caption, rc, m_TextAlignment, GetColor( GUI::COL_WINDOWTEXT ) );
    }
  }
}



