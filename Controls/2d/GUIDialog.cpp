#include <Grafik/Image.h>

#include "GUIComponentDisplayer.h"
#include "GUIDialog.h"



GUIDialog::GUIDialog( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const GR::String& strCaption, GR::u32 dwId, GR::u32 dfType ) :
  AbstractDialog<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, strCaption, dwId, dfType )
{
  ModifyVisualStyle( GUI::VFT_RAISED_BORDER );
}



void GUIDialog::DisplayNonClientOnPage( GR::Graphic::GFXPage* pPage )
{
  GUIComponent::DisplayNonClientOnPage( pPage );

  GR::tRect   rectCaption;
  if ( GetCaptionRect( rectCaption ) )
  {
    pPage->Box( rectCaption.Left, rectCaption.Top,
                rectCaption.Right - 1, rectCaption.Bottom - 1,
                pPage->GetRGB256( GUIComponentDisplayer::Instance().GetSysColor( GUI::COL_ACTIVECAPTION ) ) );
    
    rectCaption.Inflate( 1, 1 );
    DrawText( pPage, m_Caption.c_str(), rectCaption );
  }
}



void GUIDialog::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
}


