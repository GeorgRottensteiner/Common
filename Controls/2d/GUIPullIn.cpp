#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIPullIn.h"



IMPLEMENT_CLONEABLE( CGUIPullIn, "GUI.PullIn" )



CGUIPullIn::CGUIPullIn( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwId, GR::u32 dfType ) :
  CAbstractPullIn<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwId, dfType )
{

  ModifyEdge( GUI::GET_RAISED_BORDER );

}



void CGUIPullIn::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
{

  CGUIComponent::DisplayNonClientOnPage( pPage );

  GR::tRect   rectCaption;
  if ( GetCaptionRect( rectCaption ) )
  {
    pPage->Box( rectCaption.Left, rectCaption.Top,
                rectCaption.Right - 1, rectCaption.Bottom - 1,
                pPage->GetRGB256( CGUIComponentDisplayer::Instance().GetSysColor( GUI::COL_ACTIVECAPTION ) ) );
    
    rectCaption.inflate( 1, 1 );
    DrawText( pPage, m_Caption.c_str(), rectCaption );
  }

}



void CGUIPullIn::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

}



