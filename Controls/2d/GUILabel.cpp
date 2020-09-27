#include "GUIComponentDisplayer.h"
#include "GUILabel.h"



GUI_IMPLEMENT_CLONEABLE( GUILabel, "Label" )



GUILabel::GUILabel( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::UTF8String& strCaption, GR::u32 lfType, GR::u32 dwId ) :
  AbstractLabel<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, lfType, dwId )
{
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
}



void GUILabel::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  DrawText( pPage, m_Caption.c_str(), m_ClientRect, m_textAlignment );
}



