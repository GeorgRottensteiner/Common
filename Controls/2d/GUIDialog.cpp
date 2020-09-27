#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIDialog.h"



CGUIDialog::CGUIDialog( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::UTF8String& strCaption, GR::u32 dwId, GR::u32 dfType ) :
  CAbstractDialog<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, dwId, dfType )
{

  ModifyEdge( GUI::GET_RAISED_BORDER );

}



void CGUIDialog::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
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



void CGUIDialog::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

}


