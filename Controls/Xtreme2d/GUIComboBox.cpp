#include <Xtreme/XFont.h>

#include "GUIComponentDisplayer.h"
#include "GUIComboBox.h"



GUI_IMPLEMENT_CLONEABLE( GUIComboBox, "ComboBox" )



GUIComboBox::GUIComboBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 cfType, DWORD dwId ) :
  AbstractComboBox<GUIComponent,GUIEdit,GUIButton,GUIListBox>( iNewX, iNewY, iNewWidth, iNewHeight, cfType, dwId )
{
  m_pEdit->SetLocation( 0, 0 );
  m_pEdit->SetSize( m_ClientRect.size().x - 20, 20 );
  m_pEdit->Enable( FALSE );

  m_pButtonDropDown->SetLocation( m_ClientRect.size().x - 20, 0 );
  m_pButtonDropDown->SetSize( 20, 20 );

  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP_RIGHT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM_RIGHT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_RIGHT, XTextureSection() );

  m_pListBox->ModifyVisualStyle( GUI::VFT_FLAT_BORDER, GUI::VFT_SUNKEN_BORDER );

  SetSize( Width(), m_pEdit->Height() );

  SetFont( (X2dFont*)m_pFont );

  RecalcClientRect();
}



void GUIComboBox::SetFont( X2dFont* pFont )
{
  GUIComponent::SetFont( pFont );

  m_pEdit->SetSize( m_pEdit->Width(), pFont->TextHeight( "ÄÖÜAOyg," ) );

  m_pListBox->SetFont( pFont );
  m_pListBox->SetItemSize( pFont->TextHeight( "ÄÖÜAOyg," ) );
  m_pEdit->SetFont( pFont );
  m_pButtonDropDown->SetFont( pFont );
  m_pButtonDropDown->SetSize( m_pButtonDropDown->Width(), pFont->TextHeight( "ÄÖÜAOyg," ) );

  SetSize( Width(), m_pEdit->Height() );

  RecalcClientRect();
}



void GUIComboBox::SetTextureSection( const GUI::eBorderType eType, const XTextureSection& TexSection )
{
  AbstractComboBox<GUIComponent,GUIEdit,GUIButton,GUIListBox>::SetTextureSection( eType, TexSection );

  m_pListBox->SetTextureSection( eType, TexSection );
}



void GUIComboBox::DisplayOnPage( GUIComponentDisplayer& Displayer )
{
}



