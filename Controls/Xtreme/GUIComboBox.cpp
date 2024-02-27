#include <Xtreme/XFont.h>

#include "GUIComponentDisplayer.h"
#include "GUIComboBox.h"

GUI_IMPLEMENT_CLONEABLE( GUIComboBox, "Combobox" )



GUIComboBox::GUIComboBox( int iNewX, int iNewY, int iNewWidth, int iNewHeight, GR::u32 cfType, GR::u32 Id ) :
  AbstractComboBox<GUIComponent,GUIEdit,GUIButton,GUIListBox>( iNewX, iNewY, iNewWidth, iNewHeight, cfType, Id )
{
  m_pEdit->SetLocation( 0, 0 );
  m_pEdit->SetSize( m_ClientRect.Width() - 20, 20 );
  m_pEdit->Enable( false );

  m_pButtonDropDown->SetLocation( m_ClientRect.Width() - 20, 0 );
  m_pButtonDropDown->SetSize( 20, 20 );

  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP_RIGHT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_TOP, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_BOTTOM_RIGHT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_LEFT, XTextureSection() );
  m_pListBox->SetTextureSection( GUI::BT_EDGE_RIGHT, XTextureSection() );

  XTextureSection   tsComboButton = CustomTextureSection( GUI::CTS_COMBO_BUTTON );
  if ( tsComboButton.m_pTexture )
  {
    m_pButtonDropDown->SetCustomTextureSection( GUIButton::CTS_IMAGE, tsComboButton );
    m_pButtonDropDown->SetCaption( "" );
  }

  SetSize( Width(), m_pEdit->Height() );

  SetFont( (XFont*)m_pFont );

  RecalcClientRect();
}



void GUIComboBox::SetFont( XFont* pFont )
{
  GUIComponent::SetFont( pFont );

  int   iNewHeight = 20;
  if ( pFont )
  {
    pFont->TextHeight( "AOyg,\xC4\xD6\xDC\xDF" );
  }
  if ( iNewHeight < 20 )
  {
    iNewHeight = 20;
  }

  m_pEdit->SetSize( m_pEdit->Width(), iNewHeight );

  m_pListBox->SetFont( pFont );
  m_pListBox->SetItemSize( iNewHeight );
  m_pEdit->SetFont( pFont );
  m_pButtonDropDown->SetFont( pFont );
  m_pButtonDropDown->SetSize( m_pButtonDropDown->Width(), iNewHeight );

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



