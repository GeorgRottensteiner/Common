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
  GR::tRect   rc;

  if ( GetCaptionRect( rc ) )
  {
    Displayer.m_pActualRenderer->SetShader( XRenderer::ST_FLAT_NO_TEXTURE );
    GR::u32   col1 = GetColor( GUI::COL_ACTIVECAPTION );
    GR::u32   col2 = GetColor( GUI::COL_GRADIENTACTIVECAPTION );
    Displayer.DrawQuad( rc.Left, rc.Top, rc.Width(), rc.Height(), 
                        col1, 
                        col2, 
                        col1, 
                        col2 );
  }

  Displayer.m_pActualRenderer->SetShader( XRenderer::ST_ALPHA_TEST );
  Displayer.DrawText( m_pFont, 0, 0, m_Caption, 
                      GUI::AF_DEFAULT, 
                      GetColor( GUI::COL_CAPTIONTEXT ), &rc );

  GUIComponent::DisplayNonClientOnPage( Displayer );
}




