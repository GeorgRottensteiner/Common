#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIPullIn.h"



IMPLEMENT_CLONEABLE( GUIPullIn, "GUI.PullIn" )



GUIPullIn::GUIPullIn( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwId, GR::u32 dfType ) :
  AbstractPullIn<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId, dfType )
{
  ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
}



void GUIPullIn::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
{
  GUIComponent::DisplayNonClientOnPage( pPage );

  GR::tRect   rectCaption;
  if ( GetCaptionRect( rectCaption ) )
  {
    pPage->Box( rectCaption.Left, rectCaption.Top,
                rectCaption.Right - 1, rectCaption.Bottom - 1,
                pPage->GetRGB256( GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_ACTIVECAPTION ) ) );
    
    rectCaption.inflate( 1, 1 );
    DrawText( pPage, m_Caption.c_str(), rectCaption );
  }
}



void GUIPullIn::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
}



