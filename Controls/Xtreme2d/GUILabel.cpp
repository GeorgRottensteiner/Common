#include "GUIComponentDisplayer.h"
#include "GUILabel.h"



GUI_IMPLEMENT_CLONEABLE( GUILabel, "Label" )



GUILabel::GUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 dwID ) :
  AbstractLabel<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, GUI::AF_DEFAULT, dwID )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



GUILabel::GUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwFlags, const GR::String& strCaption, GR::u32 dwID ) :
  AbstractLabel<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, dwFlags, dwID )
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
    rc.Right  -= m_TextPos.width();
    rc.Bottom -= m_TextPos.height();

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



