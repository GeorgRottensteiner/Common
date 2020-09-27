#include "GUIComponentDisplayer.h"
#include "GUIComboBox.h"



GUI_IMPLEMENT_CLONEABLE( CGUIComboBox, "ComboBox" )



CGUIComboBox::CGUIComboBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 cfType, GR::u32 dwId ) :
  CAbstractComboBox<CGUIComponent,CGUIEdit,CGUIButton,CGUIListBox>( iNewX, iNewY, iNewWidth, iNewHeight, cfType, dwId )
{

  RecalcClientRect();

  m_pEdit->SetLocation( 0, 0 );
  m_pEdit->SetSize( m_ClientRect.width() - 20, 20 );
  m_pEdit->Enable( FALSE );

  m_pButtonDropDown->SetLocation( m_ClientRect.width() - 20, 0 );
  m_pButtonDropDown->SetSize( 20, 20 );

}



void CGUIComboBox::DisplayOnPage( GR::Graphic::GFXPage* pPage )
{


}



