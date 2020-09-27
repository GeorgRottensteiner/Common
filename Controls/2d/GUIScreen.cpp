#include "GUIComponentDisplayer.h"
#include "GUIScreen.h"



GUI_IMPLEMENT_CLONEABLE( GUIScreen, "Screen" )


GUIScreen::GUIScreen( int iNewX, int iNewY, int iNewWidth, int iNewHeight, DWORD dwId ) :
  GUIComponent( iNewX, iNewY, iNewWidth, iNewHeight, dwId )
{

  m_ClassName = "Screen";
  ModifyVisualStyle( GUI::VFT_TRANSPARENT_BKGND );
  ModifyFlags( GUI::COMPFT_NOT_FOCUSABLE );

}



void GUIScreen::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

}



