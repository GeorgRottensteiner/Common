#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUICheckBox.h"



GUICheckBox::GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID ) :
  AbstractCheckBox<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, GUICheckBox::CS_DEFAULT, dwID )
{
}



GUICheckBox::GUICheckBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwID ) :
  AbstractCheckBox<GUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwID )
{
}



void GUICheckBox::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{
  int   iX = 2;
  int   iY = ( m_ClientRect.height() - 16 ) / 2;

  if ( Style() & CS_CHECKBOX_RIGHT )
  {
    iX = m_ClientRect.width() - 16 - 2;
  }

  pPage->Box( iX, iY, iX + 15, iY + 15, pPage->GetRGB256( GetSysColor( GUI::COL_WINDOW ) ) );
  DrawEdge( pPage, GUI::VFT_SUNKEN_BORDER, GR::tRect( iX, iY, 16, 16 ) );

  if ( IsChecked() )
  {
    pPage->Line( iX + 2, iY + 2,  iX + 13, iY + 13, pPage->GetRGB256( GetSysColor( GUI::COL_WINDOWTEXT ) ) );
    pPage->Line( iX + 2, iY + 13, iX + 13, iY + 2, pPage->GetRGB256( GetSysColor( GUI::COL_WINDOWTEXT ) ) );
  }

  
  if ( m_pFont )
  {
    GR::tRect   rc( 20, 0, m_ClientRect.width() - 20, m_ClientRect.height() );

    if ( Style() & CS_CHECKBOX_RIGHT )
    {
      rc.Left = 0;
      rc.Right -= 20;
    }

    DrawText( pPage, m_Caption.c_str(), rc, m_textAlignment );
  }
}



