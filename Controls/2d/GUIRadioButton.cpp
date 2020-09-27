#include <Grafik/Font.h>

#include "GUIComponentDisplayer.h"
#include "GUIRadioButton.h"



CGUIRadioButton::CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, const char* szCaption, GR::u32 dwID ) :
  CAbstractRadioButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, szCaption, dwID )
{
}



CGUIRadioButton::CGUIRadioButton( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 dwID ) :
  CAbstractRadioButton<CGUIComponent>( iNewX, iNewY, iNewWidth, iNewHeight, dwID )
{
}



void CGUIRadioButton::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{

  if ( !IsVisible() )
  {
    return;
  }

  if ( IsPushed() )
  {
    pPage->Rectangle( 0, 0, m_Width - 1, m_Height - 1, pPage->GetRGB256( 0x0000ff ) );
  }
  else
  {
    pPage->Rectangle( 0, 0, m_Width - 1, m_Height - 1, pPage->GetRGB256( 0xffffff ) );
  }

  
  if ( m_pFont )
  {
    GR::tRect   rc( 20, 0, m_ClientRect.width() - 20, m_ClientRect.height() );

    DrawText( pPage, m_Caption.c_str(), rc, m_textAlignment );
  }

}



