#include "GUIComponentDisplayer.h"
#include "GUIScrollableContainer.h"



GUI_IMPLEMENT_CLONEABLE( GUIScrollableContainer, "ScrollableContainer" )

 

GUIScrollableContainer::GUIScrollableContainer( int NewX, int NewY, int NewWidth, int NewHeight, GR::u32 Id ) :
  AbstractScrollableContainer<GUIComponent, GUIScrollBar>( NewX, NewY, NewWidth, NewHeight, Id )
{ 
  m_ClassName = "ScrollableContainer"; 
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
} 



void GUIScrollableContainer::ParseXML( GR::Strings::XMLElement* pElement, GR::IEnvironment& Environment )
{
  AbstractScrollableContainer<GUIComponent, GUIScrollBar>::ParseXML( pElement, Environment );
}



void GUIScrollableContainer::DisplayNonClientOnPage( GUIComponentDisplayer& Displayer )
{
  AbstractScrollableContainer<GUIComponent, GUIScrollBar>::DisplayNonClientOnPage( Displayer );
}