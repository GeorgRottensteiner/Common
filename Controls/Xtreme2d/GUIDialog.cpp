#include "GUIComponentDisplayer.h"
#include "GUIDialog.h"



GUI_IMPLEMENT_CLONEABLE( GUIDialog, "Dialog" )



GUIDialog::GUIDialog( int NewX, int NewY, int NewWidth, int NewHeight, const GR::String& Caption, GR::u32 Type, GR::u32 Id ) :
  AbstractDialog<GUIComponent>( NewX, NewY, NewWidth, NewHeight, Caption, Id, Type )
{
  ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
  AbstractDialog<GUIComponent>::RecalcClientRect();
}



void GUIDialog::DisplayOnPage( GUIComponentDisplayer& Displayer )
{

}



void GUIDialog::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{

  GUIComponent::DisplayNonClientOnPage( Displayer );

  GR::tRect   rc;

  if ( GetCaptionRect( rc ) )
  {
    Displayer.DrawQuad( rc.Left, rc.Top, rc.width(), rc.height(), GetSysColor( GUI::COL_ACTIVECAPTION ) );

    Displayer.DrawText( m_pFont, 0, 0, m_Caption, GUI::AF_DEFAULT, GetSysColor( GUI::COL_CAPTIONTEXT ), &rc );
  }
}




