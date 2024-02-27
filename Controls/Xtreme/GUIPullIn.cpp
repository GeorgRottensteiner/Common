#include "GUIComponentDisplayer.h"
#include "GUIPullIn.h"

GUI_IMPLEMENT_CLONEABLE( GUIPullIn, "PullIn" )



GUIPullIn::GUIPullIn( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Type, GR::u32 Id ) :
  AbstractPullIn<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Id, Type )
{
  ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
}



void GUIPullIn::DisplayOnPage( GUIComponentDisplayer& )
{
}



void GUIPullIn::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  GR::tRect   rc;

  if ( GetCaptionRect( rc ) )
  {
    GR::u32 col1 = GetSysColor( GUI::COL_ACTIVECAPTION );
    GR::u32 col2 = GetSysColor( GUI::COL_GRADIENTACTIVECAPTION );

    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 
                        col1, col2, col1, col2 );
  }

  Displayer.DrawText( m_pFont, 0, 0, m_Caption, 
                      GUI::AF_DEFAULT, 
                      GetSysColor( GUI::COL_CAPTIONTEXT ), &rc );

  GUIComponent::DisplayNonClientOnPage( Displayer );
}




