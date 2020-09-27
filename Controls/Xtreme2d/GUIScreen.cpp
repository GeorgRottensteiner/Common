#include "GUIComponentDisplayer.h"
#include "GUIScreen.h"


GUI_IMPLEMENT_CLONEABLE( GUIScreen, "Screen" )



GUIScreen::GUIScreen( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwId ) :
  GUIComponent( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
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



