#include "GUIComponentDisplayer.h"
#include "GUIPullIn.h"



GUI_IMPLEMENT_CLONEABLE( GUIPullIn, "PullIn" )



GUIPullIn::GUIPullIn( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dftType, GR::u32 dwId ) :
  AbstractPullIn<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId, dftType )
{
  ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  SetColor( GUI::COL_WINDOW, GetSysColor( GUI::COL_BTNFACE ) );
}



void GUIPullIn::DisplayOnPage( GUIComponentDisplayer& Displayer )
{

}



void GUIPullIn::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  GUIComponent::DisplayNonClientOnPage( Displayer );

  GR::tRect   rc;
  GR::tRect   rcClient;

  if ( GetCaptionRect( rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetSysColor( GUI::COL_ACTIVECAPTION ) );
  }

  GetClientRect( rcClient );
  Displayer.DrawText( m_pFont, rc.Left, rc.Top, m_Caption, GUI::AF_DEFAULT, GetSysColor( GUI::COL_CAPTIONTEXT ), &rcClient );
}




