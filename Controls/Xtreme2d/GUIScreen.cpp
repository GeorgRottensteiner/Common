#include "GUIComponentDisplayer.h"
#include "GUIScreen.h"


GUI_IMPLEMENT_CLONEABLE( GUIScreen, "Screen" )



GUIScreen::GUIScreen( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id ) :
  GUIComponent( NewX, NewY, NewWidth, NewHeight, Id )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );
}



void GUIScreen::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
}



void GUIScreen::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
{
  m_Image = pElement->Attribute( "Image" );
  GUIComponent::ParseXML( pElement, Environment );
}



